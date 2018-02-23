// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/flow/layers/clip_path_layer.h"

#if defined(OS_FUCHSIA)

#include "lib/ui/scenic/fidl_helpers.h"  // nogncheck

#endif  // defined(OS_FUCHSIA)

namespace flow {

ClipPathLayer::ClipPathLayer() = default;

ClipPathLayer::~ClipPathLayer() = default;

void ClipPathLayer::Preroll(PrerollContext* context, const SkMatrix& matrix) {
  SkRect child_paint_bounds = SkRect::MakeEmpty();
  PrerollChildren(context, matrix, &child_paint_bounds);

  if (child_paint_bounds.intersect(clip_path_.getBounds())) {
    set_paint_bounds(child_paint_bounds);
  }
}

void ClipPathLayer::UpdateScene(SystemCompositorContext& context) {
  FXL_DCHECK(needs_system_composite());
  context.PushLayer(paint_bounds());
  context.ClipFrame();

  context.SetClipPath(clip_path_);

  UpdateSceneChildren(context);
  context.PopLayer();

  // // TODO(MZ-140): Must be able to specify paths as shapes to nodes.
  // //               Treating the shape as a rectangle for now.
  // auto bounds = clip_path_.getBounds();
  // scenic_lib::Rectangle shape(context.session(),  // session
  //                             bounds.width(),     //  width
  //                             bounds.height()     //  height
  // );
}

void ClipPathLayer::Paint(PaintContext& context) const {
  TRACE_EVENT0("flutter", "ClipPathLayer::Paint");
  FXL_DCHECK(needs_painting());

  Layer::AutoSaveLayer save(context, paint_bounds(), nullptr);
  context.canvas.clipPath(clip_path_, true);
  PaintChildren(context);
}

}  // namespace flow
