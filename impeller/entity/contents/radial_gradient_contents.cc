// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "radial_gradient_contents.h"

#include "flutter/fml/logging.h"
#include "impeller/entity/contents/content_context.h"
#include "impeller/entity/entity.h"
#include "impeller/renderer/render_pass.h"
#include "impeller/tessellator/tessellator.h"

namespace impeller {

RadialGradientContents::RadialGradientContents() = default;

RadialGradientContents::~RadialGradientContents() = default;

void RadialGradientContents::SetPath(Path path) {
  path_ = std::move(path);
}

void RadialGradientContents::SetCenterAndRadius(Point centre, Scalar radius) {
  center_ = centre;
  radius_ = radius;
}

void RadialGradientContents::SetColors(std::vector<Color> colors) {
  colors_ = std::move(colors);
  if (colors_.empty()) {
    colors_.push_back(Color::Black());
    colors_.push_back(Color::Black());
  } else if (colors_.size() < 2u) {
    colors_.push_back(colors_.back());
  }
}

const std::vector<Color>& RadialGradientContents::GetColors() const {
  return colors_;
}

std::optional<Rect> RadialGradientContents::GetCoverage(
    const Entity& entity) const {
  return path_.GetTransformedBoundingBox(entity.GetTransformation());
};

bool RadialGradientContents::Render(const ContentContext& renderer,
                                    const Entity& entity,
                                    RenderPass& pass) const {
  using VS = RadialGradientFillPipeline::VertexShader;
  using FS = RadialGradientFillPipeline::FragmentShader;

  auto vertices_builder = VertexBufferBuilder<VS::PerVertexData>();
  {
    auto result =
        Tessellator{}.Tessellate(path_.GetFillType(), path_.CreatePolyline(),
                                 [&vertices_builder](Point point) {
                                   VS::PerVertexData vtx;
                                   vtx.vertices = point;
                                   vertices_builder.AppendVertex(vtx);
                                 });

    if (result == Tessellator::Result::kInputError) {
      return true;
    }
    if (result == Tessellator::Result::kTessellationError) {
      return false;
    }
  }

  VS::FrameInfo frame_info;
  frame_info.mvp = Matrix::MakeOrthographic(pass.GetRenderTargetSize()) *
                   entity.GetTransformation();

  FS::GradientInfo gradient_info;
  gradient_info.center = center_;
  gradient_info.radius = radius_;
  gradient_info.center_color = colors_[0].Premultiply();
  gradient_info.edge_color = colors_[1].Premultiply();

  Command cmd;
  cmd.label = "RadialGradientFill";
  cmd.pipeline = renderer.GetRadialGradientFillPipeline(
      OptionsFromPassAndEntity(pass, entity));
  cmd.stencil_reference = entity.GetStencilDepth();
  cmd.BindVertices(
      vertices_builder.CreateVertexBuffer(pass.GetTransientsBuffer()));
  cmd.primitive_type = PrimitiveType::kTriangle;
  FS::BindGradientInfo(
      cmd, pass.GetTransientsBuffer().EmplaceUniform(gradient_info));
  VS::BindFrameInfo(cmd, pass.GetTransientsBuffer().EmplaceUniform(frame_info));
  return pass.AddCommand(std::move(cmd));
}

}  // namespace impeller
