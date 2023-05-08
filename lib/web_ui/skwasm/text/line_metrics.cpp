// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "../export.h"
#include "third_party/skia/modules/skparagraph/include/Paragraph.h"

using namespace skia::textlayout;

SKWASM_EXPORT void lineMetrics_dispose(LineMetrics* metrics) {
  delete metrics;
}

SKWASM_EXPORT bool lineMetrics_getHardBreak(LineMetrics* metrics) {
  return metrics->fHardBreak;
}

SKWASM_EXPORT SkScalar lineMetrics_getAscent(LineMetrics* metrics) {
  return metrics->fAscent;
}

SKWASM_EXPORT SkScalar lineMetrics_getDescent(LineMetrics* metrics) {
  return metrics->fDescent;
}

SKWASM_EXPORT SkScalar lineMetrics_getUnscaledAscent(LineMetrics* metrics) {
  return metrics->fUnscaledAscent;
}

SKWASM_EXPORT SkScalar lineMetrics_getHeight(LineMetrics* metrics) {
  return metrics->fHeight;
}

SKWASM_EXPORT SkScalar lineMetrics_getWidth(LineMetrics* metrics) {
  return metrics->fWidth;
}

SKWASM_EXPORT SkScalar lineMetrics_getLeft(LineMetrics* metrics) {
  return metrics->fLeft;
}

SKWASM_EXPORT SkScalar lineMetrics_getBaseline(LineMetrics* metrics) {
  return metrics->fBaseline;
}

SKWASM_EXPORT int lineMetrics_getLineNumber(LineMetrics* metrics) {
  return metrics->fLineNumber;
}

SKWASM_EXPORT size_t lineMetrics_getStartIndex(LineMetrics* metrics) {
  return metrics->fStartIndex;
}

SKWASM_EXPORT size_t lineMetrics_getEndIndex(LineMetrics* metrics) {
  return metrics->fEndIndex;
}
