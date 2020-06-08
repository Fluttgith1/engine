// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// @dart = 2.6
part of engine;

class ScreenMetrics {
  const ScreenMetrics(
    this.devicePixelRatio,
    this.physicalWidth,
    this.physicalHeight,
  );

  final double/*!*/ devicePixelRatio;
  final double/*!*/ physicalWidth;
  final double/*!*/ physicalHeight;
}
