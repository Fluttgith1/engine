// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import 'dart:convert' show ByteConversionSink, jsonDecode, utf8;
import 'dart:io' as io;
import 'dart:typed_data';

import 'package:args/command_runner.dart';
import 'package:convert/convert.dart';
import 'package:crypto/crypto.dart' as crypto;
import 'package:http/http.dart' as http;
import 'package:path/path.dart' as path;

import 'cipd.dart';
import 'environment.dart';
import 'exceptions.dart';
import 'utils.dart';

const String expectedUrlPrefix = 'https://fonts.gstatic.com/s/';

class RollFallbackFontsCommand extends Command<bool>
    with ArgUtils<bool> {
  RollFallbackFontsCommand() {
    argParser.addOption(
      'key',
      defaultsTo: '',
      help: 'The Google Fonts API key. Used to get data about fonts hosted on '
          'Google Fonts.',
    );
    argParser.addFlag(
      'dry-run',
      help: 'Whether or not to push changes to CIPD. When --dry-run is set, the '
            'script will download everything and attempt to prepare the bundle '
            'but will stop before publishing. When not set, the bundle will be '
            'published.',
      negatable: false,
    );
  }

  @override
  final String name = 'roll-fallback-fonts';

  @override
  final String description = 'Generate fallback font data from GoogleFonts and '
                             'upload fonts to cipd.';

  String get apiKey => stringArg('key');
  bool get isDryRun => boolArg('dry-run');

  @override
  Future<bool> run() async {
    await _generateFallbackFontData();
    return true;
  }

  Future<void> _generateFallbackFontData() async {
    if (apiKey.isEmpty) {
      throw UsageException('No Google Fonts API key provided', argParser.usage);
    }
    final http.Client client = http.Client();
    final http.Response response = await client.get(Uri.parse(
        'https://www.googleapis.com/webfonts/v1/webfonts?key=$apiKey'));
    if (response.statusCode != 200) {
      throw ToolExit('Failed to download Google Fonts list.');
    }
    final Map<String, dynamic> googleFontsResult =
        jsonDecode(response.body) as Map<String, dynamic>;
    final List<Map<String, dynamic>> fontDatas =
        (googleFontsResult['items'] as List<dynamic>)
            .cast<Map<String, dynamic>>();
    final Map<String, Uri> urlForFamily = <String, Uri>{};
    for (final Map<String, dynamic> fontData in fontDatas) {
      if (fallbackFonts.contains(fontData['family'])) {
        final Uri uri = Uri.parse(fontData['files']['regular'] as String)
            .replace(scheme: 'https');
        urlForFamily[fontData['family'] as String] = uri;
      }
    }
    final Map<String, String> charsetForFamily = <String, String>{};
    final io.Directory fontDir = await io.Directory.systemTemp.createTemp('flutter_fallback_fonts');
    print('Downloading fonts into temp directory: ${fontDir.path}');
    final AccumulatorSink<crypto.Digest> hashSink = AccumulatorSink<crypto.Digest>();
    final ByteConversionSink hasher = crypto.sha256.startChunkedConversion(hashSink);
    for (final String family in fallbackFonts) {
      print('Downloading $family...');
      final Uri? uri = urlForFamily[family];
      if (uri == null) {
        throw ToolExit('Unable to determine URL to download $family. '
            'Check if it is still hosted on Google Fonts.');
      }
      final http.Response fontResponse = await client.get(uri);
      if (fontResponse.statusCode != 200) {
        throw ToolExit('Failed to download font for $family');
      }
      final String urlString = uri.toString();
      if (!urlString.startsWith(expectedUrlPrefix)) {
        throw ToolExit('Unexpected url format received from Google Fonts API: $urlString.');
      }
      final String urlSuffix = urlString.substring(expectedUrlPrefix.length);
      final io.File fontFile =
          io.File(path.join(fontDir.path, urlSuffix));

      final Uint8List bodyBytes = fontResponse.bodyBytes;
      hasher.add(utf8.encode(urlSuffix));
      hasher.add(bodyBytes);

      await fontFile.create(recursive: true);
      await fontFile.writeAsBytes(bodyBytes);
      final io.ProcessResult fcQueryResult =
          await io.Process.run('fc-query', <String>[
        '--format=%{charset}',
        '--',
        fontFile.path,
      ]);
      final String encodedCharset = fcQueryResult.stdout as String;
      charsetForFamily[family] = encodedCharset;
    }
    hasher.close();

    final StringBuffer sb = StringBuffer();

    sb.writeln('// Copyright 2013 The Flutter Authors. All rights reserved.');
    sb.writeln('// Use of this source code is governed by a BSD-style license '
        'that can be');
    sb.writeln('// found in the LICENSE file.');
    sb.writeln();
    sb.writeln('// DO NOT EDIT! This file is generated. See:');
    sb.writeln('// dev/roll_fallback_fonts.dart');
    sb.writeln("import '../configuration.dart';");
    sb.writeln("import 'noto_font.dart';");
    sb.writeln();
    sb.writeln('final List<NotoFont> fallbackFonts = <NotoFont>[');

    for (final String family in fallbackFonts) {
      if (family == 'Noto Emoji') {
        sb.write(' if (!configuration.useColorEmoji)');
      }
      if (family == 'Noto Color Emoji') {
        sb.write(' if (configuration.useColorEmoji)');
      }
      final String urlString = urlForFamily[family]!.toString();
      if (!urlString.startsWith(expectedUrlPrefix)) {
        throw ToolExit('Unexpected url format received from Google Fonts API: $urlString.');
      }
      final String urlSuffix = urlString.substring(expectedUrlPrefix.length);
      sb.writeln(" NotoFont('$family', '$urlSuffix',");
      final List<String> starts = <String>[];
      final List<String> ends = <String>[];
      for (final String range in charsetForFamily[family]!.split(' ')) {
        final List<String> parts = range.split('-');
        if (parts.length == 1) {
          starts.add(parts[0]);
          ends.add(parts[0]);
        } else {
          starts.add(parts[0]);
          ends.add(parts[1]);
        }
      }

      // Print the unicode ranges in a readable format for easier review. This
      // shouldn't affect code size because comments are removed in release mode.
      sb.write('   // <int>[');
      for (final String start in starts) {
        sb.write('0x$start,');
      }
      sb.writeln('],');
      sb.write('   // <int>[');
      for (final String end in ends) {
        sb.write('0x$end,');
      }
      sb.writeln(']');

      sb.writeln("   '${_packFontRanges(starts, ends)}',");
      sb.writeln(' ),');
    }
    sb.writeln('];');

    final io.File fontDataFile = io.File(path.join(
      environment.webUiRootDir.path,
      'lib',
      'src',
      'engine',
      'canvaskit',
      'font_fallback_data.dart',
    ));
    await fontDataFile.writeAsString(sb.toString());

    final crypto.Digest digest = hashSink.events.single;
    final String versionString = digest.toString();
    const String packageName = 'flutter/flutter_font_fallbacks';
    if (await cipdKnowsPackageVersion(
      package: packageName,
      versionTag: versionString)) {
        print('Package already exists with hash $versionString. Skipping upload');
    } else {
      print('Uploading fallback fonts to CIPD with hash $versionString');
      await uploadDirectoryToCipd(
        directory: fontDir,
        packageName: packageName,
        configFileName: 'cipd.flutter_font_fallbacks.yaml',
        description: 'A set of Noto fonts to fall back to for use in testing.',
        root: fontDir.path,
        version: versionString,
        buildId: versionString,
        isDryRun: isDryRun,
      );
    }

    print('Setting new fallback fonts deps version to $versionString');
    final String depFilePath = path.join(
      environment.engineSrcDir.path,
      'flutter',
      'DEPS',
    );
    await runProcess('gclient', <String>[
      'setdep',
      '--revision=src/third_party/flutter_font_fallbacks:$packageName@$versionString',
      '--deps-file=$depFilePath'
    ]);
  }
}

const List<String> fallbackFonts = <String>[
  'Noto Sans',
  'Noto Color Emoji',
  'Noto Emoji',
  'Noto Sans Symbols',
  'Noto Sans Symbols 2',
  'Noto Sans Adlam',
  'Noto Sans Anatolian Hieroglyphs',
  'Noto Sans Arabic',
  'Noto Sans Armenian',
  'Noto Sans Avestan',
  'Noto Sans Balinese',
  'Noto Sans Bamum',
  'Noto Sans Bassa Vah',
  'Noto Sans Batak',
  'Noto Sans Bengali',
  'Noto Sans Bhaiksuki',
  'Noto Sans Brahmi',
  'Noto Sans Buginese',
  'Noto Sans Buhid',
  'Noto Sans Canadian Aboriginal',
  'Noto Sans Carian',
  'Noto Sans Caucasian Albanian',
  'Noto Sans Chakma',
  'Noto Sans Cham',
  'Noto Sans Cherokee',
  'Noto Sans Coptic',
  'Noto Sans Cuneiform',
  'Noto Sans Cypriot',
  'Noto Sans Deseret',
  'Noto Sans Devanagari',
  'Noto Sans Duployan',
  'Noto Sans Egyptian Hieroglyphs',
  'Noto Sans Elbasan',
  'Noto Sans Elymaic',
  'Noto Sans Georgian',
  'Noto Sans Glagolitic',
  'Noto Sans Gothic',
  'Noto Sans Grantha',
  'Noto Sans Gujarati',
  'Noto Sans Gunjala Gondi',
  'Noto Sans Gurmukhi',
  'Noto Sans HK',
  'Noto Sans Hanunoo',
  'Noto Sans Hatran',
  'Noto Sans Hebrew',
  'Noto Sans Imperial Aramaic',
  'Noto Sans Indic Siyaq Numbers',
  'Noto Sans Inscriptional Pahlavi',
  'Noto Sans Inscriptional Parthian',
  'Noto Sans JP',
  'Noto Sans Javanese',
  'Noto Sans KR',
  'Noto Sans Kaithi',
  'Noto Sans Kannada',
  'Noto Sans Kayah Li',
  'Noto Sans Kharoshthi',
  'Noto Sans Khmer',
  'Noto Sans Khojki',
  'Noto Sans Khudawadi',
  'Noto Sans Lao',
  'Noto Sans Lepcha',
  'Noto Sans Limbu',
  'Noto Sans Linear A',
  'Noto Sans Linear B',
  'Noto Sans Lisu',
  'Noto Sans Lycian',
  'Noto Sans Lydian',
  'Noto Sans Mahajani',
  'Noto Sans Malayalam',
  'Noto Sans Mandaic',
  'Noto Sans Manichaean',
  'Noto Sans Marchen',
  'Noto Sans Masaram Gondi',
  'Noto Sans Math',
  'Noto Sans Mayan Numerals',
  'Noto Sans Medefaidrin',
  'Noto Sans Meetei Mayek',
  'Noto Sans Meroitic',
  'Noto Sans Miao',
  'Noto Sans Modi',
  'Noto Sans Mongolian',
  'Noto Sans Mro',
  'Noto Sans Multani',
  'Noto Sans Myanmar',
  'Noto Sans NKo',
  'Noto Sans Nabataean',
  'Noto Sans New Tai Lue',
  'Noto Sans Newa',
  'Noto Sans Nushu',
  'Noto Sans Ogham',
  'Noto Sans Ol Chiki',
  'Noto Sans Old Hungarian',
  'Noto Sans Old Italic',
  'Noto Sans Old North Arabian',
  'Noto Sans Old Permic',
  'Noto Sans Old Persian',
  'Noto Sans Old Sogdian',
  'Noto Sans Old South Arabian',
  'Noto Sans Old Turkic',
  'Noto Sans Oriya',
  'Noto Sans Osage',
  'Noto Sans Osmanya',
  'Noto Sans Pahawh Hmong',
  'Noto Sans Palmyrene',
  'Noto Sans Pau Cin Hau',
  'Noto Sans Phags Pa',
  'Noto Sans Phoenician',
  'Noto Sans Psalter Pahlavi',
  'Noto Sans Rejang',
  'Noto Sans Runic',
  'Noto Sans SC',
  'Noto Sans Saurashtra',
  'Noto Sans Sharada',
  'Noto Sans Shavian',
  'Noto Sans Siddham',
  'Noto Sans Sinhala',
  'Noto Sans Sogdian',
  'Noto Sans Sora Sompeng',
  'Noto Sans Soyombo',
  'Noto Sans Sundanese',
  'Noto Sans Syloti Nagri',
  'Noto Sans Syriac',
  'Noto Sans TC',
  'Noto Sans Tagalog',
  'Noto Sans Tagbanwa',
  'Noto Sans Tai Le',
  'Noto Sans Tai Tham',
  'Noto Sans Tai Viet',
  'Noto Sans Takri',
  'Noto Sans Tamil',
  'Noto Sans Tamil Supplement',
  'Noto Sans Telugu',
  'Noto Sans Thaana',
  'Noto Sans Thai',
  'Noto Sans Tifinagh',
  'Noto Sans Tirhuta',
  'Noto Sans Ugaritic',
  'Noto Sans Vai',
  'Noto Sans Wancho',
  'Noto Sans Warang Citi',
  'Noto Sans Yi',
  'Noto Sans Zanabazar Square',
];

String _packFontRanges(List<String> starts, List<String> ends) {
  assert(starts.length == ends.length);

  final StringBuffer sb = StringBuffer();

  for (int i = 0; i < starts.length; i++) {
    final int start = int.parse(starts[i], radix: 16);
    final int end = int.parse(ends[i], radix: 16);

    sb.write(start.toRadixString(36));
    sb.write('|');
    if (start != end) {
      sb.write((end - start).toRadixString(36));
    }
    sb.write(';');
  }

  return sb.toString();
}
