// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_FLOW_RASTER_CACHE_H_
#define FLUTTER_FLOW_RASTER_CACHE_H_

#include <memory>
#include <unordered_map>

// #include "flutter/flow/layers/layer.h"
#include "flutter/flow/raster_cache_key.h"
#include "flutter/fml/macros.h"
#include "flutter/fml/memory/weak_ptr.h"
#include "flutter/fml/trace_event.h"
#include "third_party/skia/include/core/SkImage.h"
#include "third_party/skia/include/core/SkSize.h"

class SkColorSpace;

namespace flutter {

// enum class RasterCacheLayerStrategy { kLayer, kLayerChildren };

class RasterCacheResult {
 public:
  RasterCacheResult(sk_sp<SkImage> image,
                    const SkRect& logical_rect,
                    const char* type);

  virtual ~RasterCacheResult() = default;

  virtual void draw(SkCanvas& canvas, const SkPaint* paint) const;

  virtual SkISize image_dimensions() const {
    return image_ ? image_->dimensions() : SkISize::Make(0, 0);
  };

  virtual int64_t image_bytes() const {
    return image_ ? image_->imageInfo().computeMinByteSize() : 0;
  };

 private:
  sk_sp<SkImage> image_;
  SkRect logical_rect_;
  fml::tracing::TraceFlow flow_;
};

class Layer;
struct PrerollContext;

struct RasterCacheMetrics {
  /**
   * The number of cache entries with images evicted in this frame.
   */
  size_t eviction_count = 0;

  /**
   * The size of all of the images evicted in this frame.
   */
  size_t eviction_bytes = 0;

  /**
   * The number of cache entries with images used in this frame.
   */
  size_t in_use_count = 0;

  /**
   * The size of all of the images used in this frame.
   */
  size_t in_use_bytes = 0;

  /**
   * The total cache entries that had images during this frame whether
   * they were used in the frame or held memory during the frame and then
   * were evicted after it ended.
   */
  size_t total_count() const { return in_use_count + eviction_count; }

  /**
   * The size of all of the cached images during this frame whether
   * they were used in the frame or held memory during the frame and then
   * were evicted after it ended.
   */
  size_t total_bytes() const { return in_use_bytes + eviction_bytes; }
};

class RasterCache {
 public:
  struct Context {
    GrDirectContext* gr_context;
    SkColorSpace* dst_color_space;
    bool checkerboard;
  };

  // The default max number of picture and display list raster caches to be
  // generated per frame. Generating too many caches in one frame may cause jank
  // on that frame. This limit allows us to throttle the cache and distribute
  // the work across multiple frames.
  static constexpr int kDefaultPictureAndDispLayListCacheLimitPerFrame = 3;

  explicit RasterCache(size_t access_threshold = 3,
                       size_t picture_and_display_list_cache_limit_per_frame =
                           kDefaultPictureAndDispLayListCacheLimitPerFrame);

  virtual ~RasterCache() = default;

  static SkIRect GetDeviceBounds(const SkRect& rect, const SkMatrix& ctm) {
    SkRect device_rect;
    ctm.mapRect(&device_rect, rect);
    SkIRect bounds;
    device_rect.roundOut(&bounds);
    return bounds;
  }

  /**
   * @brief Snap the translation components of the matrix to integers.
   *
   * The snapping will only happen if the matrix only has scale and translation
   * transformations.
   *
   * @param ctm the current transformation matrix.
   * @return SkMatrix the snapped transformation matrix.
   */
  static SkMatrix GetIntegralTransCTM(const SkMatrix& ctm) {
    // Avoid integral snapping if the matrix has complex transformation to avoid
    // the artifact observed in https://github.com/flutter/flutter/issues/41654.
    if (!ctm.isScaleTranslate()) {
      return ctm;
    }
    SkMatrix result = ctm;
    result[SkMatrix::kMTransX] = SkScalarRoundToScalar(ctm.getTranslateX());
    result[SkMatrix::kMTransY] = SkScalarRoundToScalar(ctm.getTranslateY());
    return result;
  }

  static bool CanRasterizeRect(const SkRect& cull_rect);

  void PrepareNewFrame();
  void CleanupAfterFrame();

  // Marks an entry in the cache as having been seen during Preroll of
  // the current frame. Returns true if the entry is already populated.
  bool MarkSeen(const RasterCacheKeyID& id, const SkMatrix& matrix);

  // Returns true if this entry is already populated, or if it is
  // successfully populated during this call.
  bool UpdateCacheEntry(const RasterCacheKeyID& id,
                        const SkMatrix& matrix,
                        const Context& context,
                        const SkRect& logical_bounds,
                        const char* flow_type,
                        const std::function<void(SkCanvas*)>& renderer);

  // Draws this item if it should be rendered from the cache and returns
  // true iff it was successfully drawn. Typically this should only fail
  // if the item was disabled due to conditions discovered during |Preroll|
  // or if the attempt to populate the entry failed due to bounds overflow
  // conditions.
  bool Draw(const RasterCacheKeyID& id,
            SkCanvas& canvas,
            const SkPaint* paint) const;

  void Clear();

  void SetCheckboardCacheImages(bool checkerboard);

  const RasterCacheMetrics& picture_metrics() const { return picture_metrics_; }
  const RasterCacheMetrics& layer_metrics() const { return layer_metrics_; }

  size_t GetCachedEntriesCount() const;

  /**
   * Return the number of map entries in the layer cache regardless of whether
   * the entries have been populated with an image.
   */
  size_t GetLayerCachedEntriesCount() const;

  /**
   * Return the number of map entries in the picture caches (SkPicture and
   * DisplayList) regardless of whether the entries have been populated with
   * an image.
   */
  size_t GetPictureCachedEntriesCount() const;

  /**
   * @brief Estimate how much memory is used by picture raster cache entries in
   * bytes, including cache entries in the SkPicture cache and the DisplayList
   * cache.
   *
   * Only SkImage's memory usage is counted as other objects are often much
   * smaller compared to SkImage. SkImageInfo::computeMinByteSize is used to
   * estimate the SkImage memory usage.
   */
  size_t EstimatePictureCacheByteSize() const;

  /**
   * @brief Estimate how much memory is used by layer raster cache entries in
   * bytes.
   *
   * Only SkImage's memory usage is counted as other objects are often much
   * smaller compared to SkImage. SkImageInfo::computeMinByteSize is used to
   * estimate the SkImage memory usage.
   */
  size_t EstimateLayerCacheByteSize() const;

  /**
   * @brief Return the number of frames that a picture must be prepared
   * before it will be cached. If the number is 0, then no picture will
   * ever be cached.
   *
   * If the number is one, then it must be prepared and drawn on 1 frame
   * and it will then be cached on the next frame if it is prepared.
   */
  int access_threshold() const { return access_threshold_; }

 private:
  struct Entry {
    bool used_this_frame = false;
    size_t access_count = 0;
    std::unique_ptr<RasterCacheResult> image;
  };

  void SweepOneCacheAfterFrame(RasterCacheKey::Map<Entry>& cache,
                               RasterCacheMetrics& picture_metrics,
                               RasterCacheMetrics& layer_metrics);

  bool GenerateNewCacheInThisFrame() const {
    // Disabling caching when access_threshold is zero is historic behavior.
    return access_threshold_ != 0 &&
           picture_cached_this_frame_ + display_list_cached_this_frame_ <
               picture_and_display_list_cache_limit_per_frame_;
  }

  const size_t access_threshold_;
  const size_t picture_and_display_list_cache_limit_per_frame_;
  size_t picture_cached_this_frame_ = 0;
  size_t display_list_cached_this_frame_ = 0;
  RasterCacheMetrics layer_metrics_;
  RasterCacheMetrics picture_metrics_;
  mutable RasterCacheKey::Map<Entry> cache_;
  bool checkerboard_images_;

  void TraceStatsToTimeline() const;

  FML_DISALLOW_COPY_AND_ASSIGN(RasterCache);
};

}  // namespace flutter

#endif  // FLUTTER_FLOW_RASTER_CACHE_H_
