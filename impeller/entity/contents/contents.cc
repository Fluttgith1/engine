// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "impeller/entity/contents/contents.h"
#include <optional>

#include "fml/logging.h"
#include "impeller/base/strings.h"
#include "impeller/base/validation.h"
#include "impeller/core/formats.h"
#include "impeller/entity/contents/anonymous_contents.h"
#include "impeller/entity/contents/content_context.h"
#include "impeller/entity/contents/texture_contents.h"
#include "impeller/renderer/command_buffer.h"
#include "impeller/renderer/render_pass.h"

namespace impeller {

ContentContextOptions OptionsFromPass(const RenderPass& pass) {
  ContentContextOptions opts;
  opts.sample_count = pass.GetRenderTarget().GetSampleCount();
  opts.color_attachment_pixel_format =
      pass.GetRenderTarget().GetRenderTargetPixelFormat();
  opts.has_stencil_attachment =
      pass.GetRenderTarget().GetStencilAttachment().has_value();
  return opts;
}

ContentContextOptions OptionsFromPassAndEntity(const RenderPass& pass,
                                               const Entity& entity) {
  ContentContextOptions opts = OptionsFromPass(pass);
  opts.blend_mode = entity.GetBlendMode();
  return opts;
}

std::shared_ptr<Contents> Contents::MakeAnonymous(
    Contents::RenderProc render_proc,
    Contents::CoverageProc coverage_proc) {
  return AnonymousContents::Make(std::move(render_proc),
                                 std::move(coverage_proc));
}

Contents::Contents() = default;

Contents::~Contents() = default;

bool Contents::IsOpaque() const {
  return false;
}

Contents::StencilCoverage Contents::GetStencilCoverage(
    const Entity& entity,
    const std::optional<Rect>& current_stencil_coverage) const {
  return {.type = StencilCoverage::Type::kNoChange,
          .coverage = current_stencil_coverage};
}

std::optional<Snapshot> Contents::RenderToSnapshot(
    const ContentContext& renderer,
    const Entity& entity,
    std::optional<Rect> coverage_limit,
    const std::optional<SamplerDescriptor>& sampler_descriptor,
    bool msaa_enabled,
    const std::string& label) const {
  auto coverage = GetCoverage(entity);
  if (!coverage.has_value()) {
    return std::nullopt;
  }

  // Pad Contents snapshots with 1 pixel borders to ensure correct sampling
  // behavior. Not doing so results in a coverage leak for filters that support
  // customizing the input sampling mode. Snapshots of contents should be
  // theoretically treated as infinite size just like layers.
  coverage = coverage->Expand(1);

  if (coverage_limit.has_value()) {
    coverage = coverage->Intersection(*coverage_limit);
    if (!coverage.has_value()) {
      return std::nullopt;
    }
  }

  auto texture = renderer.MakeSubpass(
      label, ISize::Ceil(coverage->size),
      [&contents = *this, &entity, &coverage](const ContentContext& renderer,
                                              RenderPass& pass) -> bool {
        Entity sub_entity;
        sub_entity.SetBlendMode(BlendMode::kSourceOver);
        sub_entity.SetTransformation(
            Matrix::MakeTranslation(Vector3(-coverage->origin)) *
            entity.GetTransformation());
        return contents.Render(renderer, sub_entity, pass);
      },
      msaa_enabled);

  if (!texture) {
    return std::nullopt;
  }

  auto snapshot = Snapshot{
      .texture = texture,
      .transform = Matrix::MakeTranslation(coverage->origin),
  };
  if (sampler_descriptor.has_value()) {
    snapshot.sampler_descriptor = sampler_descriptor.value();
  }

  return snapshot;
}

bool Contents::CanInheritOpacity(const Entity& entity) const {
  return false;
}

void Contents::SetInheritedOpacity(Scalar opacity) {
  VALIDATION_LOG << "Contents::SetInheritedOpacity should never be called when "
                    "Contents::CanAcceptOpacity returns false.";
}

std::optional<Color> Contents::AsBackgroundColor(const Entity& entity,
                                                 ISize target_size) const {
  return {};
}

bool Contents::ApplyColorFilter(
    const Contents::ColorFilterProc& color_filter_proc) {
  return false;
}

bool Contents::ShouldRender(const Entity& entity,
                            const std::optional<Rect>& stencil_coverage) const {
  if (!stencil_coverage.has_value()) {
    return false;
  }

  auto coverage = GetCoverage(entity);
  if (!coverage.has_value()) {
    return false;
  }
  if (coverage == Rect::MakeMaximum()) {
    return true;
  }
  return stencil_coverage->IntersectsWithRect(coverage.value());
}

void Contents::SetCoverageHint(std::optional<Rect> coverage_hint) {
  coverage_hint_ = coverage_hint;
}

const std::optional<Rect>& Contents::GetCoverageHint() const {
  return coverage_hint_;
}

std::optional<Size> Contents::GetColorSourceSize() const {
  return color_source_size_;
};

void Contents::SetColorSourceSize(Size size) {
  color_source_size_ = size;
}

}  // namespace impeller
