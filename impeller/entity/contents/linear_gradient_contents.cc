// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "linear_gradient_contents.h"

#include "impeller/core/formats.h"
#include "impeller/entity/contents/content_context.h"
#include "impeller/entity/contents/gradient_generator.h"
#include "impeller/entity/entity.h"
#include "impeller/renderer/render_pass.h"
#include "impeller/renderer/vertex_buffer_builder.h"

namespace impeller {

LinearGradientContents::LinearGradientContents() = default;

LinearGradientContents::~LinearGradientContents() = default;

void LinearGradientContents::SetEndPoints(Point start_point, Point end_point) {
  start_point_ = start_point;
  end_point_ = end_point;
}

void LinearGradientContents::SetColors(std::vector<Color> colors) {
  colors_ = std::move(colors);
}

void LinearGradientContents::SetStops(std::vector<Scalar> stops) {
  stops_ = std::move(stops);
}

const std::vector<Color>& LinearGradientContents::GetColors() const {
  return colors_;
}

const std::vector<Scalar>& LinearGradientContents::GetStops() const {
  return stops_;
}

void LinearGradientContents::SetTileMode(Entity::TileMode tile_mode) {
  tile_mode_ = tile_mode;
}

bool LinearGradientContents::IsOpaque() const {
  if (GetOpacityFactor() < 1 || tile_mode_ == Entity::TileMode::kDecal) {
    return false;
  }
  for (auto color : colors_) {
    if (!color.IsOpaque()) {
      return false;
    }
  }
  return true;
}

// A much faster (in terms of ALU) linear gradient that uses vertex
// interpolation to perform all color computation. Requires that the geometry of
// the gradient is divided.
bool LinearGradientContents::FastLinearGradient(const ContentContext& renderer,
                                                const Entity& entity,
                                                RenderPass& pass) const {
  using VS = GradientPipeline::VertexShader;
  using FS = GradientPipeline::FragmentShader;

  auto options = OptionsFromPassAndEntity(pass, entity);
  Geometry& geometry = *GetGeometry();

  // We already know this is an axis aligned rectangle, so the coverage will
  // be approximately the same as the geometry. For non AARs, we can force
  // stencil then cover (not done here). We give an identity transform to
  // avoid double transforming the gradient.
  std::optional<Rect> maybe_rect = geometry.GetCoverage(Matrix());
  if (!maybe_rect.has_value()) {
    return false;
  }
  Rect rect = maybe_rect.value();

  VertexBufferBuilder<VS::PerVertexData> vtx_builder;
  bool horizontal_axis = start_point_.x == end_point_.x;

  // Step 1. Compute the locations of each breakpoint along the primary axis.
  if (stops_.size() == 2) {
    // If there are exactly two stops then we have a nearly trivial gradient.
    // Augment each vertex and submit. This is wrong if the gradient goes end ->
    // start, we could fix that via some normalization.
    vtx_builder.AddVertices(
        {{rect.GetLeftTop(), colors_[0]},
         {rect.GetRightTop(), horizontal_axis ? colors_[0] : colors_[1]},
         {rect.GetLeftBottom(), horizontal_axis ? colors_[1] : colors_[0]},
         {rect.GetRightBottom(), colors_[1]}});
    options.primitive_type = PrimitiveType::kTriangleStrip;
  } else {
    // Otherwise, we need to compute a point along the primary axis.
    std::vector<Point> points(stops_.size());
    for (auto i = 0u; i < stops_.size(); i++) {
      Scalar t = stops_[i];
      points[i] = (1.0 - t) * start_point_ + t * end_point_;
    }
    // Now create a rectangle that joins each segment. That will be two
    // triangles between each pair of points. For now just assume vertical but
    // we'll fix this later.
    options.primitive_type = PrimitiveType::kTriangle;
    vtx_builder.Reserve(6 * (stops_.size() - 1));
    for (auto i = 1u; i < points.size(); i++) {
      Rect section =
          Rect::MakeXYWH(rect.GetX(), points[i - 1].y, rect.GetWidth(),
                         abs(points[i].y - points[i - 1].y));
      vtx_builder.AddVertices({
          {section.GetLeftTop(), colors_[i - 1]},
          {section.GetRightTop(), colors_[i - 1]},
          {section.GetLeftBottom(), colors_[i]},
          {section.GetRightTop(), colors_[i - 1]},
          {section.GetLeftBottom(), colors_[i]},
          {section.GetRightBottom(), colors_[i]},
      });
    }
  }

  auto& host_buffer = renderer.GetTransientsBuffer();

  pass.SetLabel("LinearGradient");
  pass.SetVertexBuffer(vtx_builder.CreateVertexBuffer(host_buffer));
  pass.SetPipeline(renderer.GetGradientPipeline(options));
  pass.SetStencilReference(0);

  // Take the pre-populated vertex shader uniform struct and set managed
  // values.
  VS::FrameInfo frame_info;
  frame_info.mvp = entity.GetShaderTransform(pass);

  VS::BindFrameInfo(pass, host_buffer.EmplaceUniform(frame_info));

  FS::FragInfo frag_info;
  frag_info.alpha = GetOpacityFactor();

  FS::BindFragInfo(pass, host_buffer.EmplaceUniform(frag_info));

  return pass.Draw().ok();
}

bool LinearGradientContents::Render(const ContentContext& renderer,
                                    const Entity& entity,
                                    RenderPass& pass) const {
  if (GetGeometry()->IsAxisAlignedRect() &&
      (start_point_.x == end_point_.x || start_point_.y == end_point_.y)) {
    return FastLinearGradient(renderer, entity, pass);
  }
  if (renderer.GetDeviceCapabilities().SupportsSSBO()) {
    return RenderSSBO(renderer, entity, pass);
  }
  return RenderTexture(renderer, entity, pass);
}

bool LinearGradientContents::RenderTexture(const ContentContext& renderer,
                                           const Entity& entity,
                                           RenderPass& pass) const {
  using VS = LinearGradientFillPipeline::VertexShader;
  using FS = LinearGradientFillPipeline::FragmentShader;

  VS::FrameInfo frame_info;
  frame_info.matrix = GetInverseEffectTransform();

  PipelineBuilderCallback pipeline_callback =
      [&renderer](ContentContextOptions options) {
        return renderer.GetLinearGradientFillPipeline(options);
      };
  return ColorSourceContents::DrawGeometry<VS>(
      renderer, entity, pass, pipeline_callback, frame_info,
      [this, &renderer](RenderPass& pass) {
        auto gradient_data = CreateGradientBuffer(colors_, stops_);
        auto gradient_texture =
            CreateGradientTexture(gradient_data, renderer.GetContext());
        if (gradient_texture == nullptr) {
          return false;
        }

        FS::FragInfo frag_info;
        frag_info.start_point = start_point_;
        frag_info.end_point = end_point_;
        frag_info.tile_mode = static_cast<Scalar>(tile_mode_);
        frag_info.decal_border_color = decal_border_color_;
        frag_info.texture_sampler_y_coord_scale =
            gradient_texture->GetYCoordScale();
        frag_info.alpha = GetOpacityFactor();
        frag_info.half_texel =
            Vector2(0.5 / gradient_texture->GetSize().width,
                    0.5 / gradient_texture->GetSize().height);

        pass.SetCommandLabel("LinearGradientFill");

        SamplerDescriptor sampler_desc;
        sampler_desc.min_filter = MinMagFilter::kLinear;
        sampler_desc.mag_filter = MinMagFilter::kLinear;

        FS::BindTextureSampler(
            pass, std::move(gradient_texture),
            renderer.GetContext()->GetSamplerLibrary()->GetSampler(
                sampler_desc));
        FS::BindFragInfo(
            pass, renderer.GetTransientsBuffer().EmplaceUniform(frag_info));
        return true;
      });
}

bool LinearGradientContents::RenderSSBO(const ContentContext& renderer,
                                        const Entity& entity,
                                        RenderPass& pass) const {
  using VS = LinearGradientSSBOFillPipeline::VertexShader;
  using FS = LinearGradientSSBOFillPipeline::FragmentShader;

  VS::FrameInfo frame_info;
  frame_info.matrix = GetInverseEffectTransform();

  PipelineBuilderCallback pipeline_callback =
      [&renderer](ContentContextOptions options) {
        return renderer.GetLinearGradientSSBOFillPipeline(options);
      };
  return ColorSourceContents::DrawGeometry<VS>(
      renderer, entity, pass, pipeline_callback, frame_info,
      [this, &renderer](RenderPass& pass) {
        FS::FragInfo frag_info;
        frag_info.start_point = start_point_;
        frag_info.end_point = end_point_;
        frag_info.tile_mode = static_cast<Scalar>(tile_mode_);
        frag_info.decal_border_color = decal_border_color_;
        frag_info.alpha = GetOpacityFactor();

        auto& host_buffer = renderer.GetTransientsBuffer();
        auto colors = CreateGradientColors(colors_, stops_);

        frag_info.colors_length = colors.size();
        auto color_buffer =
            host_buffer.Emplace(colors.data(), colors.size() * sizeof(StopData),
                                DefaultUniformAlignment());

        pass.SetCommandLabel("LinearGradientSSBOFill");

        FS::BindFragInfo(
            pass, renderer.GetTransientsBuffer().EmplaceUniform(frag_info));
        FS::BindColorData(pass, color_buffer);

        return true;
      });
}

bool LinearGradientContents::ApplyColorFilter(
    const ColorFilterProc& color_filter_proc) {
  for (Color& color : colors_) {
    color = color_filter_proc(color);
  }
  decal_border_color_ = color_filter_proc(decal_border_color_);
  return true;
}

}  // namespace impeller
