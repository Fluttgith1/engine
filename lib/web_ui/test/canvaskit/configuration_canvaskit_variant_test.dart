// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import 'package:test/bootstrap/browser.dart';
import 'package:test/test.dart';
import 'package:ui/src/engine.dart';

import 'common.dart';

void main() {
  internalBootstrapBrowserTest(() => testMain);
}

// This test exists to make sure we don't accidentally run the CanvasKit test suite
// in "auto" mode. The CanvasKit variant should always be deterministic.
void testMain() {
  setUpCanvasKitTest();

  test('CanvasKit tests always run with a specific variant', () {
    expect(
      configuration.canvasKitVariant,
      anyOf(CanvasKitVariant.chromium, CanvasKitVariant.full),
      reason: 'canvasKitVariant must be set to "chromium" or "full" in canvaskit tests!',
    );
  });
}
