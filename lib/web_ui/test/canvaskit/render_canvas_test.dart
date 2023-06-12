// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import 'package:test/bootstrap/browser.dart';
import 'package:test/test.dart';
import 'package:ui/src/engine.dart';
import 'package:ui/ui.dart' as ui;

import 'common.dart';

void main() {
  internalBootstrapBrowserTest(() => testMain);
}

void testMain() {
  group('CanvasKit', () {
    setUpCanvasKitTest();
    setUp(() {
      window.debugOverrideDevicePixelRatio(1.0);
    });

    // Regression test for https://github.com/flutter/flutter/issues/75286
    test('updates canvas logical size when device-pixel ratio changes', () {
      final RenderCanvas canvas = RenderCanvas();
      canvas.ensureSize(const ui.Size(10, 16));

      expect(canvas.canvasElement!.width, 10);
      expect(canvas.canvasElement!.height, 16);
      expect(canvas.canvasElement!.style.width, '10px');
      expect(canvas.canvasElement!.style.height, '16px');

      // Increase device-pixel ratio: this makes CSS pixels bigger, so we need
      // fewer of them to cover the browser window.
      window.debugOverrideDevicePixelRatio(2.0);
      canvas.ensureSize(const ui.Size(10, 16));
      expect(canvas.canvasElement!.width, 10);
      expect(canvas.canvasElement!.height, 16);
      expect(canvas.canvasElement!.style.width, '5px');
      expect(canvas.canvasElement!.style.height, '8px');

      // Decrease device-pixel ratio: this makes CSS pixels smaller, so we need
      // more of them to cover the browser window.
      window.debugOverrideDevicePixelRatio(0.5);
      canvas.ensureSize(const ui.Size(10, 16));
      expect(canvas.canvasElement!.width, 10);
      expect(canvas.canvasElement!.height, 16);
      expect(canvas.canvasElement!.style.width, '20px');
      expect(canvas.canvasElement!.style.height, '32px');

      // See https://github.com/flutter/flutter/issues/77084#issuecomment-1120151172
      window.debugOverrideDevicePixelRatio(2.0);
      canvas.ensureSize(const ui.Size(9.9, 15.9));
      expect(canvas.canvasElement!.width, 10);
      expect(canvas.canvasElement!.height, 16);
      expect(canvas.canvasElement!.style.width, '5px');
      expect(canvas.canvasElement!.style.height, '8px');
    });
  });
}
