// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/lib/ui/painting/image_generator.h"

#include <utility>

#include "flutter/fml/logging.h"
#include "third_party/skia/include/core/SkBitmap.h"
#include "third_party/skia/include/core/SkImage.h"

namespace flutter {

ImageGenerator::~ImageGenerator() = default;

sk_sp<SkImage> ImageGenerator::GetImage() {
  SkImageInfo info = GetInfo();

  SkBitmap bitmap;
  if (!bitmap.tryAllocPixels(info)) {
    FML_DLOG(ERROR) << "Failed to allocate memory for bitmap of size "
                    << info.computeMinByteSize() << "B";
    return nullptr;
  }

  const auto& pixmap = bitmap.pixmap();
  if (!GetPixels(pixmap.info(), pixmap.writable_addr(), pixmap.rowBytes())) {
    FML_DLOG(ERROR) << "Failed to get pixels for image.";
    return nullptr;
  }
  bitmap.setImmutable();
  return SkImages::RasterFromBitmap(bitmap);
}

BuiltinSkiaImageGenerator::~BuiltinSkiaImageGenerator() = default;

BuiltinSkiaImageGenerator::BuiltinSkiaImageGenerator(
    std::unique_ptr<SkImageGenerator> generator)
    : generator_(std::move(generator)) {}

const SkImageInfo& BuiltinSkiaImageGenerator::GetInfo() {
  return generator_->getInfo();
}

unsigned int BuiltinSkiaImageGenerator::GetFrameCount() const {
  return 1;
}

unsigned int BuiltinSkiaImageGenerator::GetPlayCount() const {
  return 1;
}

const ImageGenerator::FrameInfo BuiltinSkiaImageGenerator::GetFrameInfo(
    unsigned int frame_index) {
  return {.required_frame = std::nullopt,
          .duration = 0,
          .disposal_method = SkCodecAnimation::DisposalMethod::kKeep};
}

SkISize BuiltinSkiaImageGenerator::GetScaledDimensions(float desired_scale) {
  return generator_->getInfo().dimensions();
}

bool BuiltinSkiaImageGenerator::GetPixels(
    const SkImageInfo& info,
    void* pixels,
    size_t row_bytes,
    unsigned int frame_index,
    std::optional<unsigned int> prior_frame) {
  return generator_->getPixels(info, pixels, row_bytes);
}

std::unique_ptr<ImageGenerator> BuiltinSkiaImageGenerator::MakeFromGenerator(
    std::unique_ptr<SkImageGenerator> generator) {
  if (!generator) {
    return nullptr;
  }
  return std::make_unique<BuiltinSkiaImageGenerator>(std::move(generator));
}

BuiltinSkiaCodecImageGenerator::~BuiltinSkiaCodecImageGenerator() = default;

BuiltinSkiaCodecImageGenerator::BuiltinSkiaCodecImageGenerator(
    std::unique_ptr<SkCodec> codec)
    : codec_(std::move(codec)) {
  image_info_ = codec_->getInfo();
}

BuiltinSkiaCodecImageGenerator::BuiltinSkiaCodecImageGenerator(
    sk_sp<SkData> buffer)
    : codec_(SkCodec::MakeFromData(std::move(buffer)).release()) {
  image_info_ = codec_->getInfo();
}

const SkImageInfo& BuiltinSkiaCodecImageGenerator::GetInfo() {
  return image_info_;
}

unsigned int BuiltinSkiaCodecImageGenerator::GetFrameCount() const {
  return codec_->getFrameCount();
}

unsigned int BuiltinSkiaCodecImageGenerator::GetPlayCount() const {
  auto repetition_count = codec_->getRepetitionCount();
  return repetition_count < 0 ? kInfinitePlayCount : repetition_count + 1;
}

const ImageGenerator::FrameInfo BuiltinSkiaCodecImageGenerator::GetFrameInfo(
    unsigned int frame_index) {
  SkCodec::FrameInfo info = {};
  codec_->getFrameInfo(frame_index, &info);
  return {
      .required_frame = info.fRequiredFrame == SkCodec::kNoFrame
                            ? std::nullopt
                            : std::optional<unsigned int>(info.fRequiredFrame),
      .duration = static_cast<unsigned int>(info.fDuration),
      .disposal_method = info.fDisposalMethod};
}

SkISize BuiltinSkiaCodecImageGenerator::GetScaledDimensions(
    float desired_scale) {
  return codec_->getScaledDimensions(desired_scale);
}

bool BuiltinSkiaCodecImageGenerator::GetPixels(
    const SkImageInfo& info,
    void* pixels,
    size_t row_bytes,
    unsigned int frame_index,
    std::optional<unsigned int> prior_frame) {
  SkCodec::Options options;
  options.fFrameIndex = frame_index;
  if (prior_frame.has_value()) {
    options.fPriorFrame = prior_frame.value();
  }
  SkCodec::Result result = codec_->getPixels(info, pixels, row_bytes, &options);
  switch (result) {
    case SkCodec::kSuccess:
    case SkCodec::kIncompleteInput:
    case SkCodec::kErrorInInput:
      return true;
    default:
      return false;
  }
}

std::unique_ptr<ImageGenerator> BuiltinSkiaCodecImageGenerator::MakeFromData(
    sk_sp<SkData> data) {
  auto codec = SkCodec::MakeFromData(std::move(data));
  if (!codec) {
    return nullptr;
  }
  return std::make_unique<BuiltinSkiaCodecImageGenerator>(std::move(codec));
}

}  // namespace flutter
