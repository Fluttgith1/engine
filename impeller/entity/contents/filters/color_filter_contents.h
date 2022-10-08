// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#pragma once

#include "impeller/entity/contents/filters/filter_contents.h"

namespace impeller {

class ColorFilterContents : public FilterContents {
 public:
  static std::shared_ptr<ColorFilterContents> MakeBlend(
      BlendMode blend_mode,
      FilterInput::Vector inputs,
      std::optional<Color> foreground_color = std::nullopt);

  static std::shared_ptr<ColorFilterContents> MakeColorMatrix(
      FilterInput::Ref input,
      const ColorMatrix& color_matrix);

  static std::shared_ptr<ColorFilterContents> MakeLinearToSrgbFilter(
      FilterInput::Ref input);

  static std::shared_ptr<ColorFilterContents> MakeSrgbToLinearFilter(
      FilterInput::Ref input);

  ColorFilterContents();

  ~ColorFilterContents() override;

  void SetAbsorbOpacity(bool absorb_opacity);
  bool GetAbsorbOpacity() const;

 private:
  bool absorb_opacity_ = false;
  //   virtual std::optional<Rect> GetFilterCoverage(
  //       const FilterInput::Vector& inputs,
  //       const Entity& entity,
  //       const Matrix& effect_transform) const;

  //   /// @brief  Converts zero or more filter inputs into a new texture.
  //   virtual std::optional<Snapshot> RenderFilter(
  //       const FilterInput::Vector& inputs,
  //       const ContentContext& renderer,
  //       const Entity& entity,
  //       const Matrix& effect_transform,
  //       const Rect& coverage) const = 0;

  FML_DISALLOW_COPY_AND_ASSIGN(ColorFilterContents);
};

}  // namespace impeller
