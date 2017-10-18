// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/lib/ui/text/paragraph_builder.h"

#include "flutter/common/settings.h"
#include "flutter/common/threads.h"
#include "flutter/lib/ui/text/font_collection.h"
#include "flutter/lib/ui/ui_dart_state.h"
#include "flutter/third_party/txt/src/txt/font_style.h"
#include "flutter/third_party/txt/src/txt/font_weight.h"
#include "flutter/third_party/txt/src/txt/paragraph_style.h"
#include "flutter/third_party/txt/src/txt/text_decoration.h"
#include "flutter/third_party/txt/src/txt/text_style.h"
#include "lib/fxl/tasks/task_runner.h"
#include "lib/tonic/converter/dart_converter.h"
#include "lib/tonic/dart_args.h"
#include "lib/tonic/dart_binding_macros.h"
#include "lib/tonic/dart_library_natives.h"

namespace blink {
namespace {

// TextStyle

const int tsColorIndex = 1;
const int tsTextDecorationIndex = 2;
const int tsTextDecorationColorIndex = 3;
const int tsTextDecorationStyleIndex = 4;
const int tsFontWeightIndex = 5;
const int tsFontStyleIndex = 6;
const int tsTextBaselineIndex = 7;
const int tsFontFamilyIndex = 8;
const int tsFontSizeIndex = 9;
const int tsLetterSpacingIndex = 10;
const int tsWordSpacingIndex = 11;
const int tsHeightIndex = 12;

const int tsColorMask = 1 << tsColorIndex;
const int tsTextDecorationMask = 1 << tsTextDecorationIndex;
const int tsTextDecorationColorMask = 1 << tsTextDecorationColorIndex;
const int tsTextDecorationStyleMask = 1 << tsTextDecorationStyleIndex;
const int tsFontWeightMask = 1 << tsFontWeightIndex;
const int tsFontStyleMask = 1 << tsFontStyleIndex;
const int tsTextBaselineMask = 1 << tsTextBaselineIndex;
const int tsFontFamilyMask = 1 << tsFontFamilyIndex;
const int tsFontSizeMask = 1 << tsFontSizeIndex;
const int tsLetterSpacingMask = 1 << tsLetterSpacingIndex;
const int tsWordSpacingMask = 1 << tsWordSpacingIndex;
const int tsHeightMask = 1 << tsHeightIndex;

// ParagraphStyle

const int psTextAlignIndex = 1;
const int psTextDirectionIndex = 2;
const int psFontWeightIndex = 3;
const int psFontStyleIndex = 4;
const int psMaxLinesIndex = 5;
const int psFontFamilyIndex = 6;
const int psFontSizeIndex = 7;
const int psLineHeightIndex = 8;
const int psEllipsisIndex = 9;

const int psTextAlignMask = 1 << psTextAlignIndex;
const int psTextDirectionMask = 1 << psTextDirectionIndex;
const int psFontWeightMask = 1 << psFontWeightIndex;
const int psFontStyleMask = 1 << psFontStyleIndex;
const int psMaxLinesMask = 1 << psMaxLinesIndex;
const int psFontFamilyMask = 1 << psFontFamilyIndex;
const int psFontSizeMask = 1 << psFontSizeIndex;
const int psLineHeightMask = 1 << psLineHeightIndex;
const int psEllipsisMask = 1 << psEllipsisIndex;

}  // namespace

static void ParagraphBuilder_constructor(Dart_NativeArguments args) {
  DartCallConstructor(&ParagraphBuilder::create, args);
}

IMPLEMENT_WRAPPERTYPEINFO(ui, ParagraphBuilder);

#define FOR_EACH_BINDING(V)      \
  V(ParagraphBuilder, pushStyle) \
  V(ParagraphBuilder, pop)       \
  V(ParagraphBuilder, addText)   \
  V(ParagraphBuilder, build)

FOR_EACH_BINDING(DART_NATIVE_CALLBACK)

void ParagraphBuilder::RegisterNatives(tonic::DartLibraryNatives* natives) {
  natives->Register(
      {{"ParagraphBuilder_constructor", ParagraphBuilder_constructor, 6, true},
       FOR_EACH_BINDING(DART_REGISTER_NATIVE)});
}

fxl::RefPtr<ParagraphBuilder> ParagraphBuilder::create(
    tonic::Int32List& encoded,
    const std::string& fontFamily,
    double fontSize,
    double lineHeight,
    const std::u16string& ellipsis) {
  return fxl::MakeRefCounted<ParagraphBuilder>(encoded, fontFamily, fontSize,
                                               lineHeight, ellipsis);
}

ParagraphBuilder::ParagraphBuilder(tonic::Int32List& encoded,
                                   const std::string& fontFamily,
                                   double fontSize,
                                   double lineHeight,
                                   const std::u16string& ellipsis) {
  int32_t mask = encoded[0];
  txt::ParagraphStyle style;
  if (mask & psTextAlignMask)
    style.text_align = txt::TextAlign(encoded[psTextAlignIndex]);

  if (mask & psTextDirectionMask)
    style.text_direction = txt::TextDirection(encoded[psTextDirectionIndex]);

  if (mask & psFontWeightMask)
    style.font_weight =
        static_cast<txt::FontWeight>(encoded[psFontWeightIndex]);

  if (mask & psFontStyleMask)
    style.font_style = static_cast<txt::FontStyle>(encoded[psFontStyleIndex]);

  if (mask & psFontFamilyMask)
    style.font_family = fontFamily;

  if (mask & psFontSizeMask)
    style.font_size = fontSize;

  if (mask & psLineHeightMask)
    style.line_height = lineHeight;

  if (mask & psMaxLinesMask)
    style.max_lines = encoded[psMaxLinesIndex];

  if (mask & psEllipsisMask) {
    style.ellipsis = ellipsis;
  }

  m_paragraphBuilder = std::make_unique<txt::ParagraphBuilder>(
      style, blink::FontCollection::ForProcess().GetFontCollection());

}  // namespace blink

ParagraphBuilder::~ParagraphBuilder() = default;

void ParagraphBuilder::pushStyle(tonic::Int32List& encoded,
                                 const std::string& fontFamily,
                                 double fontSize,
                                 double letterSpacing,
                                 double wordSpacing,
                                 double height) {
  FXL_DCHECK(encoded.num_elements() == 8);

  int32_t mask = encoded[0];

  // Set to use the properties of the previous style if the property is not
  // explicitly given.
  txt::TextStyle style = m_paragraphBuilder->PeekStyle();

  if (mask & tsColorMask)
    style.color = encoded[tsColorIndex];

  if (mask & tsTextDecorationMask) {
    style.decoration =
        static_cast<txt::TextDecoration>(encoded[tsTextDecorationIndex]);
  }

  if (mask & tsTextDecorationColorMask)
    style.decoration_color = encoded[tsTextDecorationColorIndex];

  if (mask & tsTextDecorationStyleMask)
    style.decoration_style = static_cast<txt::TextDecorationStyle>(
        encoded[tsTextDecorationStyleIndex]);

  if (mask & tsTextBaselineMask) {
    // TODO(abarth): Implement TextBaseline. The CSS version of this
    // property wasn't wired up either.
  }

  if (mask & (tsFontWeightMask | tsFontStyleMask | tsFontFamilyMask |
              tsFontSizeMask | tsLetterSpacingMask | tsWordSpacingMask)) {
    if (mask & tsFontWeightMask)
      style.font_weight =
          static_cast<txt::FontWeight>(encoded[tsFontWeightIndex]);

    if (mask & tsFontStyleMask)
      style.font_style = static_cast<txt::FontStyle>(encoded[tsFontStyleIndex]);

    if (mask & tsFontFamilyMask)
      style.font_family = fontFamily;

    if (mask & tsFontSizeMask)
      style.font_size = fontSize;

    if (mask & tsLetterSpacingMask)
      style.letter_spacing = letterSpacing;

    if (mask & tsWordSpacingMask)
      style.word_spacing = wordSpacing;
  }

  if (mask & tsHeightMask) {
    style.height = height;
  }

  m_paragraphBuilder->PushStyle(style);
}

void ParagraphBuilder::pop() {
  m_paragraphBuilder->Pop();
}

void ParagraphBuilder::addText(const std::string& text) {
  m_paragraphBuilder->AddText(text);
}

fxl::RefPtr<Paragraph> ParagraphBuilder::build() {
  return Paragraph::Create(m_paragraphBuilder->Build());
}

}  // namespace blink
