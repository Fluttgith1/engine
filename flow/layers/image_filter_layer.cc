// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/flow/layers/image_filter_layer.h"
#include "flutter/display_list/display_list_comparable.h"
#include "flutter/flow/layers/layer.h"
#include "flutter/flow/raster_cache_util.h"

namespace flutter {

ImageFilterLayer::ImageFilterLayer(std::shared_ptr<const DlImageFilter> filter)
    : CacheableContainerLayer(
          RasterCacheUtil::kMinimumRendersBeforeCachingFilterLayer),
      filter_(std::move(filter)),
      transformed_filter_(nullptr) {}

void ImageFilterLayer::Diff(DiffContext* context, const Layer* old_layer) {
  DiffContext::AutoSubtreeRestore subtree(context);
  auto* prev = static_cast<const ImageFilterLayer*>(old_layer);
  if (!context->IsSubtreeDirty()) {
    FML_DCHECK(prev);
    if (NotEquals(filter_, prev->filter_)) {
      context->MarkSubtreeDirty(context->GetOldLayerPaintRegion(old_layer));
    }
  }

  if (context->has_raster_cache()) {
    context->SetTransform(
        RasterCacheUtil::GetIntegralTransCTM(context->GetTransform()));
  }

  if (filter_) {
    auto filter = filter_->makeWithLocalMatrix(context->GetTransform());
    if (filter) {
      // This transform will be applied to every child rect in the subtree
      context->PushFilterBoundsAdjustment([filter](SkRect rect) {
        SkIRect filter_out_bounds;
        filter->map_device_bounds(rect.roundOut(), SkMatrix::I(),
                                  filter_out_bounds);
        return SkRect::Make(filter_out_bounds);
      });
    }
  }
  DiffChildren(context, prev);
  context->SetLayerPaintRegion(this, context->CurrentSubtreeRegion());
}

void ImageFilterLayer::Preroll(PrerollContext* context) {
  Layer::AutoPrerollSaveLayerState save =
      Layer::AutoPrerollSaveLayerState::Create(context);

  AutoCache cache = AutoCache(layer_raster_cache_item_.get(), context,
                              context->state_stack.transform());

  SkRect child_bounds = SkRect::MakeEmpty();

  PrerollChildren(context, &child_bounds);

  if (!filter_) {
    set_paint_bounds(child_bounds);
    return;
  }

  // Our saveLayer would apply any outstanding opacity or any outstanding
  // color filter after it applies our image filter. So we can apply either
  // of those attributes with our saveLayer.
  context->renderable_state_flags =
      (LayerStateStack::CALLER_CAN_APPLY_OPACITY |
       LayerStateStack::CALLER_CAN_APPLY_COLOR_FILTER);

  const SkIRect filter_in_bounds = child_bounds.roundOut();
  SkIRect filter_out_bounds;
  filter_->map_device_bounds(filter_in_bounds, SkMatrix::I(),
                             filter_out_bounds);
  child_bounds = SkRect::Make(filter_out_bounds);

  set_paint_bounds(child_bounds);

  // CacheChildren only when the transformed_filter_ doesn't equal null.
  // So in here we reset the LayerRasterCacheItem cache state.
  layer_raster_cache_item_->MarkNotCacheChildren();

  transformed_filter_ =
      filter_->makeWithLocalMatrix(context->state_stack.transform());
  if (transformed_filter_) {
    layer_raster_cache_item_->MarkCacheChildren();
  }
}

void ImageFilterLayer::Paint(PaintContext& context) const {
  FML_DCHECK(needs_painting(context));

  auto mutator = context.state_stack.save();

  if (context.raster_cache) {
    // Always apply the integral transform in the presence of a raster cache
    // whether or not we will draw from the cache
    mutator.integralTransform();

    // Try drawing the layer cache item from the cache before applying the
    // image filter if it was cached with the filter applied.
    if (!layer_raster_cache_item_->IsCacheChildren()) {
      SkPaint sk_paint;
      if (layer_raster_cache_item_->Draw(context,
                                         context.state_stack.fill(sk_paint))) {
        return;
      }
    }
  }

  // Now apply the image filter and then try rendering children either from
  // cache or directly.
  mutator.applyImageFilter(child_paint_bounds(), filter_);

  if (context.raster_cache && layer_raster_cache_item_->IsCacheChildren()) {
    SkPaint sk_paint;
    if (layer_raster_cache_item_->Draw(context,
                                       context.state_stack.fill(sk_paint))) {
      return;
    }
  }

  PaintChildren(context);
}

}  // namespace flutter
