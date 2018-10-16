// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import 'dart:ui';

import 'package:test/test.dart';

void main() {
  group('Locale', () {
    test('Unnamed constructor', () {
      final Null $null = null;
      expect(const Locale('en').toString(), 'en');
      expect(const Locale('en'), new Locale('en', $null));
      expect(const Locale('en').hashCode, new Locale('en', $null).hashCode);
      expect(const Locale('en'), new Locale('en', ''),
             reason: 'Empty string means no subtag present.');
      expect(const Locale('en').hashCode, new Locale('en', '').hashCode,
             reason: 'Empty string means no subtag present.');
      expect(const Locale('en', 'US').toString(), 'en_US');
      expect(const Locale('iw').toString(), 'he');
      expect(const Locale('iw', 'DD').toString(), 'he_DE');
      expect(const Locale('iw', 'DD'), const Locale('he', 'DE'));
    });
  });
}
