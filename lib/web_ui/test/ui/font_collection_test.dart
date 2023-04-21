// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import 'dart:convert';
import 'dart:typed_data';

import 'package:test/bootstrap/browser.dart';
import 'package:test/test.dart';
import 'package:ui/src/engine.dart';

import '../common/test_initialization.dart';
import 'utils.dart';

void main() {
  internalBootstrapBrowserTest(() => testMain);
}

Future<void> testMain() async {
  setUpUnitTests();

  test('Loading valid font from data succeeds without family name', () async {
    final FlutterFontCollection collection = renderer.fontCollection;
    final ByteBuffer ahemData = await httpFetchByteBuffer('/assets/fonts/ahem.ttf');
    expect(
      await collection.loadFontFromList(ahemData.asUint8List()),
      true
    );
  }, skip: isHtml); // HtmlFontCollection requires family name

  test('Loading valid font from data succeeds with family name', () async {
    final FlutterFontCollection collection = renderer.fontCollection;
    final ByteBuffer ahemData = await httpFetchByteBuffer('/assets/fonts/ahem.ttf');
    expect(
      await collection.loadFontFromList(ahemData.asUint8List(), fontFamily: 'FamilyName'),
      true
    );
  });

  test('Loading invalid font from data throws', () async {
    final FlutterFontCollection collection = renderer.fontCollection;
    final List<int> invalidFontData = utf8.encode('This is not valid font data');
    expect(
      await collection.loadFontFromList(Uint8List.fromList(invalidFontData), fontFamily: 'FamilyName'),
      false
    );
  });
}
