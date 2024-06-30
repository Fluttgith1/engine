// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import 'package:test/bootstrap/browser.dart';
import 'package:test/test.dart';
import 'package:ui/ui.dart';

import '../common/test_initialization.dart';
import 'utils.dart';

void main() {
  internalBootstrapBrowserTest(() => testMain);
}

Future<void> testMain() async {
  setUpUnitTests(
    withImplicitView: true,
    emulateTesterEnvironment: false,
    setUpTestViewDimensions: false,
  );

  test('Should be able to build and layout a paragraph', () {
    final builder = ParagraphBuilder(ParagraphStyle());
    builder.addText('Hello');
    final paragraph = builder.build();
    expect(paragraph, isNotNull);

    paragraph.layout(const ParagraphConstraints(width: 800.0));
    expect(paragraph.width, isNonZero);
    expect(paragraph.height, isNonZero);
  });

  test('the presence of foreground style should not throw', () {
    final builder = ParagraphBuilder(ParagraphStyle());
    builder.pushStyle(TextStyle(
      foreground: Paint()..color = const Color(0xFFABCDEF),
    ));
    builder.addText('hi');

    expect(() => builder.build(), returnsNormally);
  });

  test('getWordBoundary respects position affinity', () {
    final builder = ParagraphBuilder(ParagraphStyle());
    builder.addText('hello world');

    final paragraph = builder.build();
    paragraph.layout(const ParagraphConstraints(width: double.infinity));

    final downstreamWordBoundary = paragraph.getWordBoundary(const TextPosition(
      offset: 5,
    ));
    expect(downstreamWordBoundary, const TextRange(start: 5, end: 6));

    final upstreamWordBoundary = paragraph.getWordBoundary(const TextPosition(
      offset: 5,
      affinity: TextAffinity.upstream,
    ));
    expect(upstreamWordBoundary, const TextRange(start: 0, end: 5));
  });

  test('getLineBoundary at the last character position gives correct results', () {
    final builder = ParagraphBuilder(ParagraphStyle());
    builder.addText('hello world');

    final paragraph = builder.build();
    paragraph.layout(const ParagraphConstraints(width: double.infinity));

    final lineBoundary = paragraph.getLineBoundary(const TextPosition(
      offset: 11,
    ));
    expect(lineBoundary, const TextRange(start: 0, end: 11));
  });

  test('build and layout a paragraph with an empty addText', () {
    final builder = ParagraphBuilder(ParagraphStyle());
    builder.addText('');
    final paragraph = builder.build();
    expect(
      () => paragraph.layout(const ParagraphConstraints(width: double.infinity)),
      returnsNormally,
    );
  });

  test('kTextHeightNone unsets the height multiplier', () {
    const fontSize = 10.0;
    const text = 'A';
    final builder = ParagraphBuilder(ParagraphStyle(fontSize: fontSize, height: 10));
    builder.pushStyle(TextStyle(height: kTextHeightNone));
    builder.addText(text);
    final paragraph = builder.build()
      ..layout(const ParagraphConstraints(width: 1000));
    // The height should be much smaller than fontSize * 10.
    expect(paragraph.height, lessThan(2 * fontSize));
  });

  test('kTextHeightNone ParagraphStyle', () {
    const fontSize = 10.0;
    final builder = ParagraphBuilder(
      ParagraphStyle(fontSize: fontSize, height: kTextHeightNone, fontFamily: 'FlutterTest'),
    );
    builder.addText('A');
    final paragraph = builder.build()
      ..layout(const ParagraphConstraints(width: 1000));
    // The height should be much smaller than fontSize * 10.
    expect(paragraph.height, lessThan(2 * fontSize));
  });

  test('kTextHeightNone StrutStyle', () {
    const fontSize = 10.0;
    final builder = ParagraphBuilder(
      ParagraphStyle(
        fontSize: 100,
        fontFamily: 'FlutterTest',
        strutStyle: StrutStyle(forceStrutHeight: true, height: kTextHeightNone, fontSize: fontSize),
      ),
    );
    builder.addText('A');
    final paragraph = builder.build()
      ..layout(const ParagraphConstraints(width: 1000));
    // The height should be much smaller than fontSize * 10.
    expect(paragraph.height, lessThan(2 * fontSize));
  },
    skip: isHtml, // The HTML renderer does not support struts.
  );
}
