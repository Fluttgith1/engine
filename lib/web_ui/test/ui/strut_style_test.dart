// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import 'package:test/bootstrap/browser.dart';
import 'package:test/test.dart';
import 'package:ui/ui.dart' as ui;

import '../common/test_initialization.dart';

void main() {
  internalBootstrapBrowserTest(() => testMain);
}

Future<void> testMain() async {
  setUpUnitTests(
    emulateTesterEnvironment: false,
    setUpTestViewDimensions: false,
  );

  test('blanks are equal to each other', () {
    final a = ui.StrutStyle();
    final b = ui.StrutStyle();
    expect(a, b);
    expect(a.hashCode, b.hashCode);
  });

  test('each property individually equal', () {
    for (final property in _populatorsA.keys) {
      final populator = _populatorsA[property]!;

      final aBuilder = _TestStrutStyleBuilder();
      populator(aBuilder);
      final a = aBuilder.build();

      final bBuilder = _TestStrutStyleBuilder();
      populator(bBuilder);
      final b = bBuilder.build();

      expect(reason: '$property property is equal', a, b);
      expect(reason: '$property hashCode is equal', a.hashCode, b.hashCode);
    }
  });

  test('each property individually not equal', () {
    for (final property in _populatorsA.keys) {
      final populatorA = _populatorsA[property]!;

      final aBuilder = _TestStrutStyleBuilder();
      populatorA(aBuilder);
      final a = aBuilder.build();

      final populatorB = _populatorsB[property]!;
      final bBuilder = _TestStrutStyleBuilder();
      populatorB(bBuilder);
      final b = bBuilder.build();

      expect(reason: '$property property is not equal', a, isNot(b));
      expect(reason: '$property hashCode is not equal', a.hashCode, isNot(b.hashCode));
    }
  });

  test('all properties altogether equal', () {
    final aBuilder = _TestStrutStyleBuilder();
    final bBuilder = _TestStrutStyleBuilder();

    for (final property in _populatorsA.keys) {
      final populator = _populatorsA[property]!;
      populator(aBuilder);
      populator(bBuilder);
    }

    final a = aBuilder.build();
    final b = bBuilder.build();

    expect(a, b);
    expect(a.hashCode, b.hashCode);
  });

  test('all properties altogether not equal', () {
    final aBuilder = _TestStrutStyleBuilder();
    final bBuilder = _TestStrutStyleBuilder();

    for (final property in _populatorsA.keys) {
      final populatorA = _populatorsA[property]!;
      populatorA(aBuilder);

      final populatorB = _populatorsB[property]!;
      populatorB(bBuilder);
    }

    final a = aBuilder.build();
    final b = bBuilder.build();

    expect(a, isNot(b));
    expect(a.hashCode, isNot(b.hashCode));
  });
}

typedef _StrutStylePropertyPopulator = void Function(_TestStrutStyleBuilder builder);

final Map<String, _StrutStylePropertyPopulator> _populatorsA = <String, _StrutStylePropertyPopulator>{
  'fontFamily': (_TestStrutStyleBuilder builder) { builder.fontFamily = 'Arial'; },
  // Intentionally do not use const List to make sure Object.hashAll is used to compute hashCode
  'fontFamilyFallback': (_TestStrutStyleBuilder builder) { builder.fontFamilyFallback = <String>['Roboto']; },
  'fontSize': (_TestStrutStyleBuilder builder) { builder.fontSize = 12; },
  'height': (_TestStrutStyleBuilder builder) { builder.height = 13; },
  'leading': (_TestStrutStyleBuilder builder) { builder.leading = 0.1; },
  'fontWeight': (_TestStrutStyleBuilder builder) { builder.fontWeight = ui.FontWeight.w400; },
  'fontStyle': (_TestStrutStyleBuilder builder) { builder.fontStyle = ui.FontStyle.normal; },
  'forceStrutHeight': (_TestStrutStyleBuilder builder) { builder.forceStrutHeight = false; },
  'leadingDistribution': (_TestStrutStyleBuilder builder) { builder.leadingDistribution = ui.TextLeadingDistribution.proportional; },
};

final Map<String, _StrutStylePropertyPopulator> _populatorsB = <String, _StrutStylePropertyPopulator>{
  'fontFamily': (_TestStrutStyleBuilder builder) { builder.fontFamily = 'Noto'; },
  // Intentionally do not use const List to make sure Object.hashAll is used to compute hashCode
  'fontFamilyFallback': (_TestStrutStyleBuilder builder) { builder.fontFamilyFallback = <String>['Verdana']; },
  'fontSize': (_TestStrutStyleBuilder builder) { builder.fontSize = 12.1; },
  'height': (_TestStrutStyleBuilder builder) { builder.height = 13.1; },
  'leading': (_TestStrutStyleBuilder builder) { builder.leading = 0.2; },
  'fontWeight': (_TestStrutStyleBuilder builder) { builder.fontWeight = ui.FontWeight.w600; },
  'fontStyle': (_TestStrutStyleBuilder builder) { builder.fontStyle = ui.FontStyle.italic; },
  'forceStrutHeight': (_TestStrutStyleBuilder builder) { builder.forceStrutHeight = true; },
  'leadingDistribution': (_TestStrutStyleBuilder builder) { builder.leadingDistribution = ui.TextLeadingDistribution.even; },
};

class _TestStrutStyleBuilder {
  String? fontFamily;
  List<String>? fontFamilyFallback;
  double? fontSize;
  double? height;
  double? leading;
  ui.FontWeight? fontWeight;
  ui.FontStyle? fontStyle;
  bool? forceStrutHeight;
  ui.TextLeadingDistribution? leadingDistribution;

  ui.StrutStyle build() {
    return ui.StrutStyle(
      fontFamily: fontFamily,
      fontFamilyFallback: fontFamilyFallback,
      fontSize: fontSize,
      height: height,
      leading: leading,
      fontWeight: fontWeight,
      fontStyle: fontStyle,
      forceStrutHeight: forceStrutHeight,
      leadingDistribution: leadingDistribution,
    );
  }
}
