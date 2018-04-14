// Copyright 2018 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import 'dart:async';
import 'dart:ui';
import 'dart:typed_data';
import 'dart:io';

import 'package:flutter_test/flutter_test.dart';
import 'package:path/path.dart' as path;

const int _kWidth = 10;
const int _kRadius = 2;

const Color _kBlack = const Color.fromRGBO(0, 0, 0, 1.0);
const Color _kGreen = const Color.fromRGBO(0, 255, 0, 1.0);

void main() {
  final Image testImage = createSquareTestImage();

  test('Encode with default arguments', () async {
    ByteData data = await testImage.toByteData();
    expect(new Uint8List.view(data.buffer), getExpectedBytes());
  });
}

Image createSquareTestImage() {
  double width = _kWidth.toDouble();
  double radius = _kRadius.toDouble();
  double innerWidth = (_kWidth - 2 * _kRadius).toDouble();

  PictureRecorder recorder = new PictureRecorder();
  Canvas canvas =
      new Canvas(recorder, new Rect.fromLTWH(0.0, 0.0, width, width));

  Paint black = new Paint()
    ..strokeWidth = 1.0
    ..color = _kBlack;
  Paint green = new Paint()
    ..strokeWidth = 1.0
    ..color = _kGreen;

  canvas.drawRect(new Rect.fromLTWH(0.0, 0.0, width, width), black);
  canvas.drawRect(
      new Rect.fromLTWH(radius, radius, innerWidth, innerWidth), green);
  return recorder.endRecording().toImage(_kWidth, _kWidth);
}

List<int> getExpectedBytes() {
  int bytesPerChannel = 4;
  List<int> result = new List<int>(_kWidth * _kWidth * bytesPerChannel);

  fillWithColor(Color color, int min, int max) {
    for (int i = min; i < max; i++) {
      for (int j = min; j < max; j++) {
        int offset = i * bytesPerChannel + j * _kWidth * bytesPerChannel;
        result[offset] = color.red;
        result[offset + 1] = color.green;
        result[offset + 2] = color.blue;
        result[offset + 3] = color.alpha;
      }
    }
  }

  fillWithColor(_kBlack, 0, _kWidth);
  fillWithColor(_kGreen, _kRadius, _kWidth - _kRadius);

  return result;
}
