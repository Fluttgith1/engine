// Copyright 2018 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// TODO(mdebbar): To reduce the size of generated code, we could pack the data
//   into a smaller format, e.g:
//
// ```dart
// const _rawData = [
//   0x000A, 0x000A, 1,
//   0x000B, 0x000C, 2,
//   0x000D, 0x000D, 3,
//   0x0020, 0x0020, 4,
//   // ...
// ];
// ```
//
// Then we could lazily build the lookup instance on demand.
import 'dart:io';
import 'package:path/path.dart' as path;

/// A tuple that holds a [start] and [end] of a unicode range and a [property].
class PropertyTuple {
  const PropertyTuple(this.start, this.end, this.property);

  final int start;
  final int end;
  final String property;

  /// Checks if there's an overlap between this tuple's range and [other]'s
  /// range.
  bool isOverlapping(PropertyTuple other) {
    return start <= other.end && end >= other.start;
  }

  /// Checks if the [other] tuple is adjacent to this tuple.
  ///
  /// Two tuples are considered adjacent if:
  /// - The new tuple's range immediately follows this tuple's range, and
  /// - The new tuple has the same property as this tuple.
  bool isAdjacent(PropertyTuple other) {
    return other.start == end + 1 && property == other.property;
  }

  /// Merges the ranges of the 2 [PropertyTuples] if they are adjacent.
  PropertyTuple extendRange(PropertyTuple extension) {
    assert(isAdjacent(extension));
    return PropertyTuple(start, extension.end, property);
  }
}

/// Usage (from the root of the project):
///
/// ```
/// dart tool/unicode_sync_script.dart <path/to/word/break/properties>
/// ```
///
/// This script parses the unicode word break properties(1) and generates Dart
/// code(2) that can perform lookups in the unicode ranges to find what property
/// a letter has.
///
/// (1) The properties file can be downloaded from:
///     https://www.unicode.org/Public/11.0.0/ucd/auxiliary/WordBreakProperty.txt
///
/// (2) The codegen'd Dart file is located at:
///     lib/src/text/word_break_properties.dart
void main(List<String> arguments) async {
  final propertiesFile = arguments[0];
  final codegenFile = path.join(
    path.dirname(Platform.script.toFilePath()),
    '../lib/src/text/word_break_properties.dart',
  );
  WordBreakPropertiesSyncer(propertiesFile, codegenFile).perform();
}

/// Base class that provides common logic for syncing all kinds of unicode
/// properties (e.g. word break properties, line break properties, etc).
///
/// Subclasses implement the [template] method which receives as argument the
/// list of data parsed by [processLines].
abstract class PropertiesSyncer {
  PropertiesSyncer(this._src, this._dest);

  final String _src;
  final String _dest;

  void perform() async {
    final List<String> lines = await File(_src).readAsLines();
    final List<String> header = extractHeader(lines);
    final List<PropertyTuple> data = processLines(lines);

    final sink = File(_dest).openWrite();
    sink.write(template(header, data));
  }

  String template(List<String> header, List<PropertyTuple> data);
}

/// Syncs Unicode's word break properties.
class WordBreakPropertiesSyncer extends PropertiesSyncer {
  WordBreakPropertiesSyncer(String src, String dest) : super(src, dest);

  String template(List<String> header, List<PropertyTuple> data) {
    return '''
// AUTO-GENERATED FILE.
// Generated by: bin/unicode_sync_script.dart
//
// Source:
// ${header.join('\n// ')}

import 'unicode_range.dart';

CharProperty getCharProperty(String text, int index) {
  if (index < 0 || index >= text.length) {
    return null;
  }
  return lookup.find(text.codeUnitAt(index));
}

enum CharProperty {
  ${getEnumValues(data).join(',\n  ')}
}

const UnicodePropertyLookup<CharProperty> lookup =
    UnicodePropertyLookup<CharProperty>([
  ${getLookupEntries(data).join(',\n  ')}
]);
''';
  }

  Iterable<String> getEnumValues(List<PropertyTuple> data) {
    return Set<String>.from(data.map((tuple) => tuple.property))
        .map(normalizePropertyName);
  }

  Iterable<String> getLookupEntries(List<PropertyTuple> data) {
    data.sort(
      // Ranges don't overlap so it's safe to sort based on the start of each
      // range.
      (tuple1, tuple2) => tuple1.start.compareTo(tuple2.start),
    );
    verifyNoOverlappingRanges(data);
    return combineAdjacentRanges(data)
        .map((tuple) => generateLookupEntry(tuple));
  }

  String generateLookupEntry(PropertyTuple tuple) {
    var propertyStr = 'CharProperty.${normalizePropertyName(tuple.property)}';
    return 'UnicodeRange(${toHex(tuple.start)}, ${toHex(tuple.end)}, $propertyStr)';
  }
}

/// Example:
///    UnicodeRange(0x01C4, 0x0293, CharProperty.ALetter),
///    UnicodeRange(0x0294, 0x0294, CharProperty.ALetter),
///    UnicodeRange(0x0295, 0x02AF, CharProperty.ALetter),
///
/// will get combined into:
///    UnicodeRange(0x01C4, 0x02AF, CharProperty.ALetter)
List<PropertyTuple> combineAdjacentRanges(List<PropertyTuple> data) {
  final List<PropertyTuple> result = [data.first];
  for (int i = 1; i < data.length; i++) {
    if (result.last.isAdjacent(data[i])) {
      result.last = result.last.extendRange(data[i]);
    } else {
      result.add(data[i]);
    }
  }
  return result;
}

int getRangeStart(String range) {
  return int.parse(range.split('..')[0], radix: 16);
}

int getRangeEnd(String range) {
  if (range.contains('..')) {
    return int.parse(range.split('..')[1], radix: 16);
  }
  return int.parse(range, radix: 16);
}

String toHex(int value) {
  return '0x${value.toRadixString(16).padLeft(4, '0').toUpperCase()}';
}

void verifyNoOverlappingRanges(List<PropertyTuple> data) {
  for (int i = 1; i < data.length; i++) {
    if (data[i].isOverlapping(data[i - 1])) {
      throw Exception('Data contains overlapping ranges.');
    }
  }
}

List<String> extractHeader(List<String> lines) {
  final List<String> headerLines = [];
  for (String line in lines) {
    if (line.contains('=======')) {
      break;
    }
    if (line.isNotEmpty) {
      headerLines.add(line);
    }
  }
  return headerLines;
}

List<PropertyTuple> processLines(List<String> lines) {
  return lines
      .map(removeCommentFromLine)
      .where((line) => line.isNotEmpty)
      .map(parseLineIntoPropertyTuple)
      .toList();
}

String normalizePropertyName(String property) {
  return property.replaceAll('_', '');
}

String removeCommentFromLine(String line) {
  int poundIdx = line.indexOf('#');
  return (poundIdx == -1) ? line : line.substring(0, poundIdx);
}

/// Examples:
///
/// 00C0..00D6    ; ALetter
/// 037F          ; ALetter
///
/// Would be parsed into:
///
/// ```dart
/// PropertyTuple(192, 214, 'ALetter');
/// PropertyTuple(895, 895, 'ALetter');
/// ```
PropertyTuple parseLineIntoPropertyTuple(String line) {
  var split = line.split(';');
  var rangeStr = split[0].trim();
  var propertyStr = split[1].trim();

  var rangeSplit =
      rangeStr.contains('..') ? rangeStr.split('..') : [rangeStr, rangeStr];
  return PropertyTuple(
    int.parse(rangeSplit[0], radix: 16),
    int.parse(rangeSplit[1], radix: 16),
    propertyStr,
  );
}
