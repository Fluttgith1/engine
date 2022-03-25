// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/flow/layers/picture_layer.h"

#include "flutter/flow/layers/offscreen_utils.h"
#include "flutter/fml/logging.h"
#include "third_party/skia/include/core/SkSerialProcs.h"

namespace flutter {

PictureLayer::PictureLayer(const SkPoint& offset,
                           SkiaGPUObject<SkPicture> picture,
                           bool is_complex,
                           bool will_change)
    : offset_(offset),
      picture_(std::move(picture)),
      is_complex_(is_complex),
      will_change_(will_change) {}

bool PictureLayer::IsReplacing(DiffContext* context, const Layer* layer) const {
  // Only return true for identical pictures; This way
  // ContainerLayer::DiffChildren can detect when a picture layer got inserted
  // between other picture layers
  auto picture_layer = layer->as_picture_layer();
  return picture_layer != nullptr && offset_ == picture_layer->offset_ &&
         Compare(context->statistics(), this, picture_layer);
}

void PictureLayer::Diff(DiffContext* context, const Layer* old_layer) {
  DiffContext::AutoSubtreeRestore subtree(context);
  if (!context->IsSubtreeDirty()) {
#ifndef NDEBUG
    FML_DCHECK(old_layer);
    auto prev = old_layer->as_picture_layer();
    DiffContext::Statistics dummy_statistics;
    // IsReplacing has already determined that the picture is same
    FML_DCHECK(prev->offset_ == offset_ &&
               Compare(dummy_statistics, this, prev));
#endif
  }
  context->PushTransform(SkMatrix::Translate(offset_.x(), offset_.y()));
#ifndef SUPPORT_FRACTIONAL_TRANSLATION
  context->SetTransform(
      RasterCache::GetIntegralTransCTM(context->GetTransform()));
#endif
  context->AddLayerBounds(picture()->cullRect());
  context->SetLayerPaintRegion(this, context->CurrentSubtreeRegion());
}

bool PictureLayer::Compare(DiffContext::Statistics& statistics,
                           const PictureLayer* l1,
                           const PictureLayer* l2) {
  const auto& pic1 = l1->picture_.skia_object();
  const auto& pic2 = l2->picture_.skia_object();

  if (pic1.get() == pic2.get()) {
    statistics.AddSameInstancePicture();
    return true;
  }
  auto op_cnt_1 = pic1->approximateOpCount();
  auto op_cnt_2 = pic2->approximateOpCount();
  if (op_cnt_1 != op_cnt_2 || pic1->cullRect() != pic2->cullRect()) {
    statistics.AddNewPicture();
    return false;
  }

  if (op_cnt_1 > 10) {
    statistics.AddPictureTooComplexToCompare();
    return false;
  }

  statistics.AddDeepComparePicture();

  // TODO(knopp) we don't actually need the data; this could be done without
  // allocations by implementing stream that calculates SHA hash and
  // comparing those hashes
  auto d1 = l1->SerializedPicture();
  auto d2 = l2->SerializedPicture();
  auto res = d1->equals(d2.get());
  if (res) {
    statistics.AddDifferentInstanceButEqualPicture();
  } else {
    statistics.AddNewPicture();
  }
  return res;
}

sk_sp<SkData> PictureLayer::SerializedPicture() const {
  if (!cached_serialized_picture_) {
    SkSerialProcs procs = {
        nullptr,
        nullptr,
        [](SkImage* i, void* ctx) {
          auto id = i->uniqueID();
          return SkData::MakeWithCopy(&id, sizeof(id));
        },
        nullptr,
        [](SkTypeface* tf, void* ctx) {
          auto id = tf->uniqueID();
          return SkData::MakeWithCopy(&id, sizeof(id));
        },
        nullptr,
    };
    cached_serialized_picture_ = picture_.skia_object()->serialize(&procs);
  }
  return cached_serialized_picture_;
}

void PictureLayer::Preroll(PrerollContext* context, const SkMatrix& matrix) {
  TRACE_EVENT0("flutter", "PictureLayer::Preroll");

  SkPicture* sk_picture = picture();

  SkRect bounds = sk_picture->cullRect().makeOffset(offset_.x(), offset_.y());

  if (auto* cache = context->raster_cache) {
    TRACE_EVENT0("flutter", "PictureLayer::RasterCache (Preroll)");
    if (context->cull_rect.intersects(bounds)) {
      if (cache->Prepare(context, sk_picture, is_complex_, will_change_, matrix,
                         offset_)) {
        context->subtree_can_inherit_opacity = true;
      }
    } else {
      // Don't evict raster cache entry during partial repaint
      cache->Touch(sk_picture, matrix);
    }
  }

  set_paint_bounds(bounds);
}

void PictureLayer::Paint(PaintContext& context) const {
  TRACE_EVENT0("flutter", "PictureLayer::Paint");
  FML_DCHECK(picture_.skia_object());
  FML_DCHECK(needs_painting(context));

  SkAutoCanvasRestore save(context.leaf_nodes_canvas, true);
  context.leaf_nodes_canvas->translate(offset_.x(), offset_.y());
#ifndef SUPPORT_FRACTIONAL_TRANSLATION
  context.leaf_nodes_canvas->setMatrix(RasterCache::GetIntegralTransCTM(
      context.leaf_nodes_canvas->getTotalMatrix()));
#endif

  if (context.raster_cache) {
    AutoCachePaint cache_paint(context);
    if (context.raster_cache->Draw(*picture(), *context.leaf_nodes_canvas,
                                   cache_paint.paint())) {
      TRACE_EVENT_INSTANT0("flutter", "raster cache hit");
      return;
    }
  }

  if (context.enable_leaf_layer_tracing) {
    const auto canvas_size = context.leaf_nodes_canvas->getBaseLayerSize();
    auto offscreen_surface = std::make_unique<OffscreenSurface>(canvas_size);

    {
      const auto now = fml::TimePoint::Now();

      // render to an offscreen canvas.
      auto* canvas = offscreen_surface->GetCanvas();
      canvas->clear(SK_ColorTRANSPARENT);
      picture()->playback(canvas);
      canvas->flush();

      const fml::TimeDelta offscreen_render_time = fml::TimePoint::Now() - now;
      // TODO (kaushikiska) record a trace event here.
      FML_LOG(ERROR) << "picture_layer: " << unique_id() << " took "
                     << offscreen_render_time.ToMicroseconds()
                     << " micro seconds";
    }

    {
      // Dump PNG to logs for now.
      sk_sp<SkData> b64_data = offscreen_surface->GetRasterDataAsBase64(true);
      FML_LOG(ERROR) << static_cast<const char*>(b64_data->data());
    }
  }

  FML_DCHECK(context.inherited_opacity == SK_Scalar1);
  picture()->playback(context.leaf_nodes_canvas);
}

}  // namespace flutter
