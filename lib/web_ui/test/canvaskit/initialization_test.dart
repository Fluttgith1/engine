// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import 'dart:html' as html;

import 'package:test/bootstrap/browser.dart';
import 'package:test/test.dart';
import 'package:ui/src/engine.dart';

import 'common.dart';

void main() {
  internalBootstrapBrowserTest(() => testMain);
}

void testMain() {
  group('CanvasKit', () {
    setUpCanvasKitTest();

    test('populates flt-renderer and flt-build-mode', () {
      FlutterViewEmbedder();
      expect(html.document.body!.attributes['flt-renderer'],
          'canvaskit (requested explicitly)');
      expect(html.document.body!.attributes['flt-build-mode'], 'debug');
    });
    // TODO(hterkelsen): https://github.com/flutter/flutter/issues/60040

    test('does not include getH5vccSkSurface', () {
      expect(canvasKit.getH5vccSkSurface, isNull);
    }, testOn: 'chrome');
  }, skip: isIosSafari);

  group('Patched h5vcc CanvasKit', () {
    patchH5vccCanvasKit();
    setUpCanvasKitTest();

    test('includes patched getH5vccSkSurface', () {
      expect(canvasKit.getH5vccSkSurface, isNotNull);
    });
  }, testOn: 'chrome');
}
