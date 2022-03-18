// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/lib/ui/painting/image_filter.h"

#include "flutter/lib/ui/painting/matrix.h"
#include "third_party/skia/include/effects/SkImageFilters.h"
#include "third_party/tonic/converter/dart_converter.h"
#include "third_party/tonic/dart_args.h"
#include "third_party/tonic/dart_binding_macros.h"
#include "third_party/tonic/dart_library_natives.h"

namespace flutter {

IMPLEMENT_WRAPPERTYPEINFO(ui, ImageFilter);

void ImageFilter::Create(Dart_Handle wrapper) {
  UIDartState::ThrowIfUIOperationsProhibited();
  auto res = fml::MakeRefCounted<ImageFilter>();
  res->AssociateWithDartWrapper(wrapper);
}

static const std::array<SkSamplingOptions, 4> filter_qualities = {
    SkSamplingOptions(SkFilterMode::kNearest, SkMipmapMode::kNone),
    SkSamplingOptions(SkFilterMode::kLinear, SkMipmapMode::kNone),
    SkSamplingOptions(SkFilterMode::kLinear, SkMipmapMode::kLinear),
    SkSamplingOptions(SkCubicResampler{1 / 3.0f, 1 / 3.0f}),
};

SkSamplingOptions ImageFilter::SamplingFromIndex(int filterQualityIndex) {
  if (filterQualityIndex < 0) {
    return filter_qualities.front();
  } else if (static_cast<size_t>(filterQualityIndex) >=
             filter_qualities.size()) {
    return filter_qualities.back();
  } else {
    return filter_qualities[filterQualityIndex];
  }
}

SkFilterMode ImageFilter::FilterModeFromIndex(int filterQualityIndex) {
  if (filterQualityIndex <= 0) {
    return SkFilterMode::kNearest;
  }
  return SkFilterMode::kLinear;
}

ImageFilter::ImageFilter() {}

ImageFilter::~ImageFilter() {}

void ImageFilter::initBlur(double sigma_x,
                           double sigma_y,
                           SkTileMode tile_mode) {
  filter_ = SkImageFilters::Blur(sigma_x, sigma_y, tile_mode, nullptr, nullptr);
}

void ImageFilter::initMatrix(const tonic::Float64List& matrix4,
                             int filterQualityIndex) {
  auto sampling = ImageFilter::SamplingFromIndex(filterQualityIndex);
  filter_ =
      SkImageFilters::MatrixTransform(ToSkMatrix(matrix4), sampling, nullptr);
}

void ImageFilter::initColorFilter(ColorFilter* colorFilter) {
  filter_ = SkImageFilters::ColorFilter(
      colorFilter ? colorFilter->filter()->skia_object() : nullptr, nullptr);
}

void ImageFilter::initComposeFilter(ImageFilter* outer, ImageFilter* inner) {
  filter_ = SkImageFilters::Compose(outer ? outer->filter() : nullptr,
                                    inner ? inner->filter() : nullptr);
}

}  // namespace flutter
