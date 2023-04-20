// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#pragma once

#include "flutter/fml/macros.h"
#include "impeller/typographer/text_render_context.h"
#include "third_party/skia/include/core/SkBitmap.h"

namespace impeller {

class DeviceBuffer;
class Allocator;

class FontImpellerAllocator : public SkBitmap::Allocator {
 public:
  explicit FontImpellerAllocator(
      std::shared_ptr<impeller::Allocator> allocator);

  ~FontImpellerAllocator() = default;

  // |Allocator|
  bool allocPixelRef(SkBitmap* bitmap) override;

  std::optional<std::shared_ptr<DeviceBuffer>> GetDeviceBuffer() const;

 private:
  std::shared_ptr<impeller::Allocator> allocator_;
  std::optional<std::shared_ptr<DeviceBuffer>> buffer_;
};

class TextRenderContextSkia : public TextRenderContext {
 public:
  TextRenderContextSkia(std::shared_ptr<Context> context);

  ~TextRenderContextSkia() override;

  // |TextRenderContext|
  std::shared_ptr<GlyphAtlas> CreateGlyphAtlas(
      GlyphAtlas::Type type,
      std::shared_ptr<GlyphAtlasContext> atlas_context,
      const std::shared_ptr<const Capabilities>& capabilities,
      FrameIterator iterator) const override;

 private:
  FML_DISALLOW_COPY_AND_ASSIGN(TextRenderContextSkia);
};

}  // namespace impeller
