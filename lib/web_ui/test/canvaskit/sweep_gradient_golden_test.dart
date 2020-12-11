// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// @dart = 2.12
import 'dart:html' as html;
import 'dart:math' as math;
import 'dart:typed_data';

import 'package:test/bootstrap/browser.dart';
import 'package:test/test.dart';
import 'package:ui/src/engine.dart';
import 'package:ui/ui.dart' as ui;

import 'package:web_engine_tester/golden_tester.dart';

import 'common.dart';

void main() {
  internalBootstrapBrowserTest(() => testMain);
}

const ui.Rect region = const ui.Rect.fromLTRB(0, 0, 500, 250);

Future<void> matchPictureGolden(String goldenFile, CkPicture picture,
    {bool write = false}) async {
  final EnginePlatformDispatcher dispatcher =
      ui.window.platformDispatcher as EnginePlatformDispatcher;
  final LayerSceneBuilder sb = LayerSceneBuilder();
  sb.pushOffset(0, 0);
  sb.addPicture(ui.Offset.zero, picture);
  dispatcher.rasterizer!.draw(sb.build().layerTree);
  await matchGoldenFile(goldenFile, region: region, write: write);
}

void testMain() {
  group('SweepGradient', () {
    setUpCanvasKitTest();

    test('is correctly rendered', () async {
      final CkPictureRecorder recorder = CkPictureRecorder();
      final CkCanvas canvas = recorder.beginRecording(region);

      final CkGradientSweep gradient = CkGradientSweep(
          ui.Offset(250, 125),
          <ui.Color>[
            ui.Color(0xFF4285F4),
            ui.Color(0xFF34A853),
            ui.Color(0xFFFBBC05),
            ui.Color(0xFFEA4335),
            ui.Color(0xFF4285F4),
          ],
          <double>[
            0.0,
            0.25,
            0.5,
            0.75,
            1.0,
          ],
          ui.TileMode.clamp,
          -(math.pi / 2),
          math.pi * 2 - (math.pi / 2),
          null);

      final CkPaint paint = CkPaint()..shader = gradient;

      canvas.drawRect(region, paint);

      await matchPictureGolden(
        'canvaskit_sweep_gradient.png',
        recorder.endRecording(),
      );
    });
  });
}
