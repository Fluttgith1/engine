// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import 'package:test/test.dart';
import 'package:ui/src/engine.dart';
import 'package:ui/ui.dart';
import 'package:web_engine_tester/golden_tester.dart';

import '../canvaskit/common.dart';

// TODO(hterkelsen): Support skwasm.

/// Initializes the renderer for this test.
void setUpUiTest() {
  setUpAll(() async {
    await webOnlyInitializePlatform();
    await renderer.fontCollection.debugDownloadTestFonts();
    renderer.fontCollection.registerDownloadedFonts();
  });
  if (isCanvasKit) {
    setUpCanvasKitTest();
  }
}

/// Draws the [Picture]. This is in preparation for a golden test.
void drawPictureUsingCurrentRenderer(Picture picture) {
  final SceneBuilder sb = SceneBuilder();
  sb.pushOffset(0, 0);
  sb.addPicture(Offset.zero, picture);
  renderer.renderScene(sb.build());
}

Future<void> matchGolden(String basename, {Rect? region}) async {
  await matchGoldenFile('canvas_lines_thickness.png', region: region);
}

/// Returns [true] if this test is running in the CanvasKit renderer.
bool get isCanvasKit => renderer is CanvasKitRenderer;

/// Returns [true] if this test is running in the HTML renderer.
bool get isHtml => renderer is HtmlRenderer;
