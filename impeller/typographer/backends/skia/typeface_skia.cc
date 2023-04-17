// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "impeller/typographer/backends/skia/typeface_skia.h"

namespace impeller {

TypefaceSkia::TypefaceSkia(sk_sp<SkTypeface> typeface)
    : typeface_(std::move(typeface)) {}

TypefaceSkia::~TypefaceSkia() = default;

bool TypefaceSkia::IsValid() const {
  return !!typeface_;
}

std::string TypefaceSkia::GetPostscriptName() const {
  SkString postscriptName;
  typeface_->getPostScriptName(&postscriptName);
  return std::string(postscriptName.c_str());
}

void* TypefaceSkia::GetCTFont() const {
  return typeface_->internal_private_getCTFontRef();
}

std::size_t TypefaceSkia::GetHash() const {
  if (!IsValid()) {
    return 0u;
  }

  return reinterpret_cast<size_t>(typeface_.get());
}

bool TypefaceSkia::IsEqual(const Typeface& other) const {
  auto sk_other = reinterpret_cast<const TypefaceSkia*>(&other);
  return sk_other->typeface_ == typeface_;
}

const sk_sp<SkTypeface>& TypefaceSkia::GetSkiaTypeface() const {
  return typeface_;
}

}  // namespace impeller
