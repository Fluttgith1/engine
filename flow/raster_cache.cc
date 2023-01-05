// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/flow/raster_cache.h"

#include <cstddef>
#include <vector>

#include "flutter/common/constants.h"
#include "flutter/flow/layers/container_layer.h"
#include "flutter/flow/layers/layer.h"
#include "flutter/flow/paint_utils.h"
#include "flutter/flow/raster_cache_util.h"
#include "flutter/fml/logging.h"
#include "flutter/fml/trace_event.h"
#include "third_party/skia/include/core/SkCanvas.h"
#include "third_party/skia/include/core/SkColorSpace.h"
#include "third_party/skia/include/core/SkImage.h"
#include "third_party/skia/include/core/SkPicture.h"
#include "third_party/skia/include/core/SkSurface.h"
#include "third_party/skia/include/gpu/GpuTypes.h"
#include "third_party/skia/include/gpu/GrDirectContext.h"

namespace flutter {

RasterCacheResult::RasterCacheResult(sk_sp<SkImage> image,
                                     const SkRect& logical_rect,
                                     const char* type)
    : image_(std::move(image)), logical_rect_(logical_rect), flow_(type) {}

void RasterCacheResult::draw(SkCanvas& canvas, const SkPaint* paint) const {
  SkAutoCanvasRestore auto_restore(&canvas, true);

  auto matrix = RasterCacheUtil::GetIntegralTransCTM(canvas.getTotalMatrix());
  SkRect bounds =
      RasterCacheUtil::GetRoundedOutDeviceBounds(logical_rect_, matrix);
  FML_DCHECK(std::abs(bounds.width() - image_->dimensions().width()) <= 1 &&
             std::abs(bounds.height() - image_->dimensions().height()) <= 1);
  canvas.resetMatrix();
  flow_.Step();
  canvas.drawImage(image_, bounds.fLeft, bounds.fTop, SkSamplingOptions(),
                   paint);
}

RasterCache::RasterCache(size_t access_threshold,
                         size_t display_list_cache_limit_per_frame)
    : access_threshold_(access_threshold),
      display_list_cache_limit_per_frame_(display_list_cache_limit_per_frame),
      checkerboard_images_(false) {}

/// @note Procedure doesn't copy all closures.
std::unique_ptr<RasterCacheResult> RasterCache::Rasterize(
    const RasterCache::Context& context,
    const std::function<void(SkCanvas*)>& draw_function,
    const std::function<void(SkCanvas*, const SkRect& rect)>& draw_checkerboard)
    const {
  auto matrix = RasterCacheUtil::GetIntegralTransCTM(context.matrix);
  SkRect dest_rect =
      RasterCacheUtil::GetRoundedOutDeviceBounds(context.logical_rect, matrix);

  const SkImageInfo image_info =
      SkImageInfo::MakeN32Premul(dest_rect.width(), dest_rect.height(),
                                 sk_ref_sp(context.dst_color_space));

  sk_sp<SkSurface> surface =
      context.gr_context ? SkSurface::MakeRenderTarget(
                               context.gr_context, skgpu::Budgeted::kYes, image_info)
                         : SkSurface::MakeRaster(image_info);

  if (!surface) {
    return nullptr;
  }

  SkCanvas* canvas = surface->getCanvas();
  canvas->clear(SK_ColorTRANSPARENT);
  canvas->translate(-dest_rect.left(), -dest_rect.top());
  canvas->concat(matrix);
  draw_function(canvas);

  if (checkerboard_images_) {
    draw_checkerboard(canvas, context.logical_rect);
  }

  return std::make_unique<RasterCacheResult>(
      surface->makeImageSnapshot(), context.logical_rect, context.flow_type);
}

bool RasterCache::UpdateCacheEntry(
    const RasterCacheKeyID& id,
    const Context& raster_cache_context,
    const std::function<void(SkCanvas*)>& render_function) const {
  RasterCacheKey key = RasterCacheKey(id, raster_cache_context.matrix);
  Entry& entry = cache_[key];
  if (!entry.image) {
    void (*func)(SkCanvas*, const SkRect& rect) = DrawCheckerboard;
    entry.image = Rasterize(raster_cache_context, render_function, func);
    if (entry.image != nullptr) {
      switch (id.type()) {
        case RasterCacheKeyType::kDisplayList: {
          display_list_cached_this_frame_++;
          break;
        }
        default:
          break;
      }
      return true;
    }
  }
  return entry.image != nullptr;
}

int RasterCache::MarkSeen(const RasterCacheKeyID& id,
                          const SkMatrix& matrix,
                          bool visible) const {
  RasterCacheKey key = RasterCacheKey(id, matrix);
  Entry& entry = cache_[key];
  entry.encountered_this_frame = true;
  entry.visible_this_frame = visible;
  if (visible || entry.accesses_since_visible > 0) {
    entry.accesses_since_visible++;
  }
  return entry.accesses_since_visible;
}

int RasterCache::GetAccessCount(const RasterCacheKeyID& id,
                                const SkMatrix& matrix) const {
  RasterCacheKey key = RasterCacheKey(id, matrix);
  auto entry = cache_.find(key);
  if (entry != cache_.cend()) {
    return entry->second.accesses_since_visible;
  }
  return -1;
}

bool RasterCache::HasEntry(const RasterCacheKeyID& id,
                           const SkMatrix& matrix) const {
  RasterCacheKey key = RasterCacheKey(id, matrix);
  if (cache_.find(key) != cache_.cend()) {
    return true;
  }
  return false;
}

bool RasterCache::Draw(const RasterCacheKeyID& id,
                       SkCanvas& canvas,
                       const SkPaint* paint) const {
  auto it = cache_.find(RasterCacheKey(id, canvas.getTotalMatrix()));
  if (it == cache_.end()) {
    return false;
  }

  Entry& entry = it->second;

  if (entry.image) {
    entry.image->draw(canvas, paint);
    return true;
  }

  return false;
}

void RasterCache::BeginFrame() {
  display_list_cached_this_frame_ = 0;
  picture_metrics_ = {};
  layer_metrics_ = {};
}

void RasterCache::UpdateMetrics() {
  for (auto it = cache_.begin(); it != cache_.end(); ++it) {
    Entry& entry = it->second;
    FML_DCHECK(entry.encountered_this_frame);
    if (entry.image) {
      RasterCacheMetrics& metrics = GetMetricsForKind(it->first.kind());
      metrics.in_use_count++;
      metrics.in_use_bytes += entry.image->image_bytes();
    }
    entry.encountered_this_frame = false;
  }
}

void RasterCache::EvictUnusedCacheEntries() {
  std::vector<RasterCacheKey::Map<Entry>::iterator> dead;

  for (auto it = cache_.begin(); it != cache_.end(); ++it) {
    Entry& entry = it->second;
    if (!entry.encountered_this_frame) {
      dead.push_back(it);
    }
  }

  for (auto it : dead) {
    if (it->second.image) {
      RasterCacheMetrics& metrics = GetMetricsForKind(it->first.kind());
      metrics.eviction_count++;
      metrics.eviction_bytes += it->second.image->image_bytes();
    }
    cache_.erase(it);
  }
}

void RasterCache::EndFrame() {
  UpdateMetrics();
  TraceStatsToTimeline();
}

void RasterCache::Clear() {
  cache_.clear();
  picture_metrics_ = {};
  layer_metrics_ = {};
}

size_t RasterCache::GetCachedEntriesCount() const {
  return cache_.size();
}

size_t RasterCache::GetLayerCachedEntriesCount() const {
  size_t layer_cached_entries_count = 0;
  for (const auto& item : cache_) {
    if (item.first.kind() == RasterCacheKeyKind::kLayerMetrics) {
      layer_cached_entries_count++;
    }
  }
  return layer_cached_entries_count;
}

size_t RasterCache::GetPictureCachedEntriesCount() const {
  size_t display_list_cached_entries_count = 0;
  for (const auto& item : cache_) {
    if (item.first.kind() == RasterCacheKeyKind::kDisplayListMetrics) {
      display_list_cached_entries_count++;
    }
  }
  return display_list_cached_entries_count;
}

void RasterCache::SetCheckboardCacheImages(bool checkerboard) {
  if (checkerboard_images_ == checkerboard) {
    return;
  }

  checkerboard_images_ = checkerboard;

  // Clear all existing entries so previously rasterized items (with or without
  // a checkerboard) will be refreshed in subsequent passes.
  Clear();
}

void RasterCache::TraceStatsToTimeline() const {
#if !FLUTTER_RELEASE
  FML_TRACE_COUNTER(
      "flutter",                                                           //
      "RasterCache", reinterpret_cast<int64_t>(this),                      //
      "LayerCount", layer_metrics_.total_count(),                          //
      "LayerMBytes", layer_metrics_.total_bytes() / kMegaByteSizeInBytes,  //
      "PictureCount", picture_metrics_.total_count(),                      //
      "PictureMBytes", picture_metrics_.total_bytes() / kMegaByteSizeInBytes);

#endif  // !FLUTTER_RELEASE
}

size_t RasterCache::EstimateLayerCacheByteSize() const {
  size_t layer_cache_bytes = 0;
  for (const auto& item : cache_) {
    if (item.first.kind() == RasterCacheKeyKind::kLayerMetrics &&
        item.second.image) {
      layer_cache_bytes += item.second.image->image_bytes();
    }
  }
  return layer_cache_bytes;
}

size_t RasterCache::EstimatePictureCacheByteSize() const {
  size_t picture_cache_bytes = 0;
  for (const auto& item : cache_) {
    if (item.first.kind() == RasterCacheKeyKind::kDisplayListMetrics &&
        item.second.image) {
      picture_cache_bytes += item.second.image->image_bytes();
    }
  }
  return picture_cache_bytes;
}

RasterCacheMetrics& RasterCache::GetMetricsForKind(RasterCacheKeyKind kind) {
  switch (kind) {
    case RasterCacheKeyKind::kDisplayListMetrics:
      return picture_metrics_;
    case RasterCacheKeyKind::kLayerMetrics:
      return layer_metrics_;
  }
}

}  // namespace flutter
