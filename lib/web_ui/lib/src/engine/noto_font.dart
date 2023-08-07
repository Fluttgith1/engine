// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import 'text/unicode_range.dart';

class NotoFont {
  NotoFont(this.name, this.url, {this.enabled = true});

  final String name;
  final String url;
  final bool enabled;

  final int index = _index++;
  static int _index = 0;

  /// During fallback font selection this is the number of missing code points
  /// that are covered by (i.e. in) this font.
  int coverCount = 0;

  /// During fallback font selection this is a list of [FallbackFontComponent]s
  /// from this font that are required to cover some of the missing code
  /// points. The cover count for the font is the sum of the cover counts for
  /// the components that make up the font.
  final List<FallbackFontComponent> coverComponents = [];
}

class FallbackFontComponent {
  FallbackFontComponent(this.fonts);
  final List<NotoFont> fonts;

  /// During fallback font selection this is the number of missing code points
  /// that are covered by this component, i.e. the intersection of all [fonts].
  int coverCount = 0;
}



class CodePointRange {
  const CodePointRange(this.start, this.end);

  final int start;
  final int end;

  bool contains(int codeUnit) {
    return start <= codeUnit && codeUnit <= end;
  }

  @override
  bool operator ==(Object other) {
    if (other is! CodePointRange) {
      return false;
    }
    final CodePointRange range = other;
    return range.start == start && range.end == end;
  }

  @override
  int get hashCode => Object.hash(start, end);

  @override
  String toString() => '[$start, $end]';
}
