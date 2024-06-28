// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import 'dart:typed_data';

/// A 1x1 fully transparent PNG image.
final Uint8List kTransparentImage = Uint8List.fromList(<int>[
  0x89,
  0x50,
  0x4e,
  0x47,
  0x0d,
  0x0a,
  0x1a,
  0x0a,
  0x00,
  0x00,
  0x00,
  0x0d,
  0x49,
  0x48,
  0x44,
  0x52,
  0x00,
  0x00,
  0x00,
  0x01,
  0x00,
  0x00,
  0x00,
  0x01,
  0x08,
  0x04,
  0x00,
  0x00,
  0x00,
  0xb5,
  0x1c,
  0x0c,
  0x02,
  0x00,
  0x00,
  0x00,
  0x0b,
  0x49,
  0x44,
  0x41,
  0x54,
  0x78,
  0xda,
  0x63,
  0x64,
  0x60,
  0x00,
  0x00,
  0x00,
  0x06,
  0x00,
  0x02,
  0x30,
  0x81,
  0xd0,
  0x2f,
  0x00,
  0x00,
  0x00,
  0x00,
  0x49,
  0x45,
  0x4e,
  0x44,
  0xae,
  0x42,
  0x60,
  0x82,
]);

/// A 4x4 PNG image sample.
final Uint8List k4x4PngImage = Uint8List.fromList(<int>[
  0x89,
  0x50,
  0x4e,
  0x47,
  0x0d,
  0x0a,
  0x1a,
  0x0a,
  0x00,
  0x00,
  0x00,
  0x0d,
  0x49,
  0x48,
  0x44,
  0x52,
  0x00,
  0x00,
  0x00,
  0x04,
  0x00,
  0x00,
  0x00,
  0x04,
  0x08,
  0x06,
  0x00,
  0x00,
  0x00,
  0xa9,
  0xf1,
  0x9e,
  0x7e,
  0x00,
  0x00,
  0x00,
  0x13,
  0x49,
  0x44,
  0x41,
  0x54,
  0x78,
  0xda,
  0x63,
  0xfc,
  0xcf,
  0xc0,
  0x50,
  0xcf,
  0x80,
  0x04,
  0x18,
  0x49,
  0x17,
  0x00,
  0x00,
  0xf2,
  0xae,
  0x05,
  0xfd,
  0x52,
  0x01,
  0xc2,
  0xde,
  0x00,
  0x00,
  0x00,
  0x00,
  0x49,
  0x45,
  0x4e,
  0x44,
  0xae,
  0x42,
  0x60,
  0x82,
]);

/// An animated GIF image with 3 1x1 pixel frames (a red, green, and blue
/// frames). The GIF animates forever, and each frame has a 100ms delay.
final Uint8List kAnimatedGif = Uint8List.fromList(<int>[
  0x47,
  0x49,
  0x46,
  0x38,
  0x39,
  0x61,
  0x01,
  0x00,
  0x01,
  0x00,
  0xa1,
  0x03,
  0x00,
  0x00,
  0x00,
  0xff,
  0xff,
  0x00,
  0x00,
  0x00,
  0xff,
  0x00,
  0xff,
  0xff,
  0xff,
  0x21,
  0xff,
  0x0b,
  0x4e,
  0x45,
  0x54,
  0x53,
  0x43,
  0x41,
  0x50,
  0x45,
  0x32,
  0x2e,
  0x30,
  0x03,
  0x01,
  0x00,
  0x00,
  0x00,
  0x21,
  0xf9,
  0x04,
  0x00,
  0x0a,
  0x00,
  0xff,
  0x00,
  0x2c,
  0x00,
  0x00,
  0x00,
  0x00,
  0x01,
  0x00,
  0x01,
  0x00,
  0x00,
  0x02,
  0x02,
  0x4c,
  0x01,
  0x00,
  0x21,
  0xf9,
  0x04,
  0x00,
  0x0a,
  0x00,
  0xff,
  0x00,
  0x2c,
  0x00,
  0x00,
  0x00,
  0x00,
  0x01,
  0x00,
  0x01,
  0x00,
  0x00,
  0x02,
  0x02,
  0x54,
  0x01,
  0x00,
  0x21,
  0xf9,
  0x04,
  0x00,
  0x0a,
  0x00,
  0xff,
  0x00,
  0x2c,
  0x00,
  0x00,
  0x00,
  0x00,
  0x01,
  0x00,
  0x01,
  0x00,
  0x00,
  0x02,
  0x02,
  0x44,
  0x01,
  0x00,
  0x3b,
]);

/// A 2x2 translucent PNG.
final Uint8List kTranslucentPng = Uint8List.fromList(<int>[
  0x89,
  0x50,
  0x4e,
  0x47,
  0x0d,
  0x0a,
  0x1a,
  0x0a,
  0x00,
  0x00,
  0x00,
  0x0d,
  0x49,
  0x48,
  0x44,
  0x52,
  0x00,
  0x00,
  0x00,
  0x02,
  0x00,
  0x00,
  0x00,
  0x02,
  0x01,
  0x03,
  0x00,
  0x00,
  0x00,
  0x48,
  0x78,
  0x9f,
  0x67,
  0x00,
  0x00,
  0x00,
  0x20,
  0x63,
  0x48,
  0x52,
  0x4d,
  0x00,
  0x00,
  0x7a,
  0x26,
  0x00,
  0x00,
  0x80,
  0x84,
  0x00,
  0x00,
  0xfa,
  0x00,
  0x00,
  0x00,
  0x80,
  0xe8,
  0x00,
  0x00,
  0x75,
  0x30,
  0x00,
  0x00,
  0xea,
  0x60,
  0x00,
  0x00,
  0x3a,
  0x98,
  0x00,
  0x00,
  0x17,
  0x70,
  0x9c,
  0xba,
  0x51,
  0x3c,
  0x00,
  0x00,
  0x00,
  0x06,
  0x50,
  0x4c,
  0x54,
  0x45,
  0x22,
  0x44,
  0x66,
  0xff,
  0xff,
  0xff,
  0x5c,
  0x83,
  0x6d,
  0xb6,
  0x00,
  0x00,
  0x00,
  0x01,
  0x74,
  0x52,
  0x4e,
  0x53,
  0x80,
  0xad,
  0x5e,
  0x5b,
  0x46,
  0x00,
  0x00,
  0x00,
  0x01,
  0x62,
  0x4b,
  0x47,
  0x44,
  0x01,
  0xff,
  0x02,
  0x2d,
  0xde,
  0x00,
  0x00,
  0x00,
  0x07,
  0x74,
  0x49,
  0x4d,
  0x45,
  0x07,
  0xe8,
  0x04,
  0x0c,
  0x15,
  0x16,
  0x21,
  0xc3,
  0x89,
  0xee,
  0x25,
  0x00,
  0x00,
  0x00,
  0x0c,
  0x49,
  0x44,
  0x41,
  0x54,
  0x08,
  0xd7,
  0x63,
  0x60,
  0x60,
  0x60,
  0x00,
  0x00,
  0x00,
  0x04,
  0x00,
  0x01,
  0x27,
  0x34,
  0x27,
  0x0a,
  0x00,
  0x00,
  0x00,
  0x00,
  0x49,
  0x45,
  0x4e,
  0x44,
  0xae,
  0x42,
  0x60,
  0x82
]);
