// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/flow/layers/physical_model_layer.h"

#include "flutter/flow/paint_utils.h"
#include "third_party/skia/include/utils/SkShadowUtils.h"

namespace flow {

PhysicalModelLayer::PhysicalModelLayer() : rrect_(SkRRect::MakeEmpty()) {}

PhysicalModelLayer::~PhysicalModelLayer() = default;

void PhysicalModelLayer::Preroll(PrerollContext* context,
                                 const SkMatrix& matrix) {
#if defined(OS_FUCHSIA)
  // Let the system compositor draw all shadows for us.
  if (elevation_ != 0)
    set_needs_system_composite(true);
#endif  // defined(OS_FUCHSIA)

  SkRect child_paint_bounds = SkRect::MakeEmpty();
  PrerollChildren(context, matrix, &child_paint_bounds);

  if (!needs_system_composite()) {
    // Add some margin to the paint bounds to leave space for the shadow.
    // The margin is hardcoded to an arbitrary maximum for now because Skia
    // doesn't provide a way to calculate it.
    SkRect bounds(rrect_.getBounds());
    bounds.outset(20.0, 20.0);
    set_paint_bounds(bounds);
  }
}

#if defined(OS_FUCHSIA)

void PhysicalModelLayer::UpdateScene(SceneUpdateContext& context,
                                     mozart::client::ContainerNode& container) {
  FTL_DCHECK(needs_system_composite());

  // TODO(MZ-137): Need to be able to express the radii as vectors.
  // TODO(MZ-138): Need to be able to specify an origin.
  mozart::client::RoundedRectangle layer_shape(
      context.session(),                              // session
      rrect_.width(),                                 //  width
      rrect_.height(),                                //  height
      rrect_.radii(SkRRect::kUpperLeft_Corner).x(),   //  top_left_radius
      rrect_.radii(SkRRect::kUpperRight_Corner).x(),  //  top_right_radius
      rrect_.radii(SkRRect::kLowerRight_Corner).x(),  //  bottom_right_radius
      rrect_.radii(SkRRect::kLowerLeft_Corner).x()    //  bottom_left_radius
      );
  mozart::client::Material layer_material(context.session());
  mozart::client::ShapeNode layer_shape_node(context.session());
  layer_shape_node.SetShape(layer_shape);
  layer_shape_node.SetMaterial(layer_material);
  layer_shape_node.SetTranslation(
      rrect_.width() * 0.5f + rrect_.getBounds().left(),
      rrect_.height() * 0.5f + rrect_.getBounds().top(), 0.f);

  mozart::client::EntityNode layer_entity_node(context.session());
  layer_entity_node.AddPart(layer_shape_node);
  layer_entity_node.SetClip(0u, true /* clip to self */);
  layer_entity_node.SetTranslation(0.f, 0.f, elevation_);
  container.AddChild(layer_entity_node);

  context.PushPhysicalModel(rrect_.getBounds(), color_);
  UpdateSceneChildren(context, layer_entity_node);
  context.PopPhysicalModel(layer_material);
}

#endif  // defined(OS_FUCHSIA)

void PhysicalModelLayer::Paint(PaintContext& context) {
  TRACE_EVENT0("flutter", "PhysicalModelLayer::Paint");
  FTL_DCHECK(!needs_system_composite());

  SkPath path;
  path.addRRect(rrect_);

  if (elevation_ != 0) {
    DrawShadow(&context.canvas, path, SK_ColorBLACK, elevation_,
               SkColorGetA(color_) != 0xff);
  }

  SkPaint paint;
  paint.setColor(color_);
  context.canvas.drawPath(path, paint);

  SkAutoCanvasRestore save(&context.canvas, false);
  if (rrect_.isRect()) {
    context.canvas.save();
  } else {
    context.canvas.saveLayer(&rrect_.getBounds(), nullptr);
  }
  context.canvas.clipRRect(rrect_, true);
  PaintChildren(context);
  if (context.checkerboard_offscreen_layers && !rrect_.isRect())
    DrawCheckerboard(&context.canvas, rrect_.getBounds());
}

void PhysicalModelLayer::DrawShadow(SkCanvas* canvas,
                                    const SkPath& path,
                                    SkColor color,
                                    double elevation,
                                    bool transparentOccluder) {
  SkShadowFlags flags = transparentOccluder
                            ? SkShadowFlags::kTransparentOccluder_ShadowFlag
                            : SkShadowFlags::kNone_ShadowFlag;
  const SkRect& bounds = path.getBounds();
  SkScalar shadow_x = (bounds.left() + bounds.right()) / 2;
  SkScalar shadow_y = bounds.top() - 600.0f;
  SkShadowUtils::DrawShadow(canvas, path, elevation,
                            SkPoint3::Make(shadow_x, shadow_y, 600.0f), 800.0f,
                            0.039f, 0.25f, color, flags);
}

}  // namespace flow
