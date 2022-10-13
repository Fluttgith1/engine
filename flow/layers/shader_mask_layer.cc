// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/flow/layers/shader_mask_layer.h"
#include "flutter/flow/raster_cache_util.h"

namespace flutter {

ShaderMaskLayer::ShaderMaskLayer(std::shared_ptr<DlColorSource> color_source,
                                 const SkRect& mask_rect,
                                 DlBlendMode blend_mode)
    : CacheableContainerLayer(
          RasterCacheUtil::kMinimumRendersBeforeCachingFilterLayer),
      color_source_(std::move(color_source)),
      mask_rect_(mask_rect),
      blend_mode_(blend_mode) {}

void ShaderMaskLayer::Diff(DiffContext* context, const Layer* old_layer) {
  DiffContext::AutoSubtreeRestore subtree(context);
  auto* prev = static_cast<const ShaderMaskLayer*>(old_layer);
  if (!context->IsSubtreeDirty()) {
    FML_DCHECK(prev);
    if (color_source_ != prev->color_source_ ||
        mask_rect_ != prev->mask_rect_ || blend_mode_ != prev->blend_mode_) {
      context->MarkSubtreeDirty(context->GetOldLayerPaintRegion(old_layer));
    }
  }
  if (context->has_raster_cache()) {
    context->SetTransform(
        RasterCacheUtil::GetIntegralTransCTM(context->GetTransform()));
  }
  DiffChildren(context, prev);

  context->SetLayerPaintRegion(this, context->CurrentSubtreeRegion());
}

void ShaderMaskLayer::Preroll(PrerollContext* context) {
  Layer::AutoPrerollSaveLayerState save =
      Layer::AutoPrerollSaveLayerState::Create(context);
  AutoCache cache = AutoCache(layer_raster_cache_item_.get(), context,
                              context->state_stack.transform());

  ContainerLayer::Preroll(context);
  // We always paint with a saveLayer (or a cached rendering),
  // so we can always apply opacity in any of those cases.
  context->renderable_state_flags = SAVE_LAYER_RENDER_FLAGS;
}

void ShaderMaskLayer::Paint(PaintContext& context) const {
  FML_DCHECK(needs_painting(context));

  auto mutator = context.state_stack.save();

  if (context.raster_cache) {
    mutator.integralTransform();

    SkPaint sk_paint;
    if (layer_raster_cache_item_->Draw(context,
                                       context.state_stack.fill(sk_paint))) {
      return;
    }
  }
  auto shader_rect = SkRect::MakeWH(mask_rect_.width(), mask_rect_.height());

  mutator.saveLayer(paint_bounds());
  if (context.builder) {
    PaintChildren(context);

    DlPaint dl_paint;
    dl_paint.setBlendMode(blend_mode_);
    if (color_source_) {
      dl_paint.setColorSource(color_source_.get());
    }
    context.builder->translate(mask_rect_.left(), mask_rect_.top());
    context.builder->drawRect(shader_rect, dl_paint);
  } else {
    PaintChildren(context);
    SkPaint paint;
    paint.setBlendMode(ToSk(blend_mode_));
    if (color_source_) {
      paint.setShader(color_source_->skia_object());
    }
    context.canvas->translate(mask_rect_.left(), mask_rect_.top());
    context.canvas->drawRect(shader_rect, paint);
  }
}

}  // namespace flutter
