// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import 'dart:io' as io;
import 'package:path/path.dart' as pathlib;

/// Contains various environment variables, such as common file paths and command-line options.
Environment get environment {
  return _environment ??= Environment();
}

Environment? _environment;

/// Contains various environment variables, such as common file paths and command-line options.
class Environment {
  /// Creates an environment deduced from the path of the main Dart script.
  factory Environment() {
    final io.File script = io.File.fromUri(io.Platform.script);
    io.Directory directory = script.parent;
    bool foundEngineRepoRoot = false;
    while (!foundEngineRepoRoot) {
      foundEngineRepoRoot = directory
        .listSync()
        .whereType<io.File>()
        .map((io.File file) => pathlib.basename(file.path))
        .contains('DEPS');
      final io.Directory parent = directory.parent;
      if (parent.path == directory.path) {
        throw Exception(
          'Failed to locate the root of the engine repository starting from ${script.path}',
        );
      }
      directory = parent;
    }
    return _prepareEnvironmentFromEngineDir(script, directory);
  }

  static Environment _prepareEnvironmentFromEngineDir(
      io.File self, io.Directory engineSrcDir) {
    final io.Directory engineToolsDir =
        io.Directory(pathlib.join(engineSrcDir.path, 'flutter', 'tools'));
    final io.Directory outDir =
        io.Directory(pathlib.join(engineSrcDir.path, 'out'));
    final io.Directory flutterWebSdkOutDir =
        io.Directory(pathlib.join(outDir.path, 'web_debug'));
    final io.Directory dartSdkDir =
        io.Directory(pathlib.join(engineSrcDir.path, 'third_party', 'dart', 'tools', 'sdks', 'dart-sdk'));
    final io.Directory webUiRootDir = io.Directory(
        pathlib.join(engineSrcDir.path, 'flutter', 'lib', 'web_ui'));

    for (final io.Directory expectedDirectory in <io.Directory>[
      engineSrcDir,
      outDir,
      flutterWebSdkOutDir,
      dartSdkDir,
      webUiRootDir
    ]) {
      if (!expectedDirectory.existsSync()) {
        throw Exception('$expectedDirectory does not exist.');
      }
    }

    return Environment._(
      self: self,
      webUiRootDir: webUiRootDir,
      engineSrcDir: engineSrcDir,
      engineToolsDir: engineToolsDir,
      outDir: outDir,
      dartSdkDir: dartSdkDir,
    );
  }

  Environment._({
    required this.self,
    required this.webUiRootDir,
    required this.engineSrcDir,
    required this.engineToolsDir,
    required this.outDir,
    required this.dartSdkDir,
  });

  /// The Dart script that's currently running.
  final io.File self;

  /// Path to the "web_ui" package sources.
  final io.Directory webUiRootDir;

  /// Path to the engine's "src" directory.
  final io.Directory engineSrcDir;

  /// Path to the engine's "tools" directory.
  final io.Directory engineToolsDir;

  /// Path to the engine's "out" directory.
  ///
  /// This is where you'll find the ninja output, such as the Dart SDK.
  final io.Directory outDir;

  /// The root of the Dart SDK.
  final io.Directory dartSdkDir;

  /// The "dart" executable file.
  String get dartExecutable => pathlib.join(dartSdkDir.path, 'bin', 'dart');

  /// The "pub" executable file.
  String get pubExecutable => pathlib.join(dartSdkDir.path, 'bin', 'pub');

  /// Path to where github.com/flutter/engine is checked out inside the engine workspace.
  io.Directory get flutterDirectory =>
      io.Directory(pathlib.join(engineSrcDir.path, 'flutter'));

  /// Path to the "web_sdk" directory in the engine source tree.
  io.Directory get webSdkRootDir => io.Directory(pathlib.join(
        flutterDirectory.path,
        'web_sdk',
      ));

  /// Path to the "web_engine_tester" package.
  io.Directory get webEngineTesterRootDir => io.Directory(pathlib.join(
        webSdkRootDir.path,
        'web_engine_tester',
      ));

  /// Path to the "build" directory, generated by "package:build_runner".
  ///
  /// This is where compiled output goes.
  io.Directory get webUiBuildDir => io.Directory(pathlib.join(
        webUiRootDir.path,
        'build',
      ));

  /// Path to the ".dart_tool" directory, generated by various Dart tools.
  io.Directory get webUiDartToolDir => io.Directory(pathlib.join(
        webUiRootDir.path,
        '.dart_tool',
      ));

  /// Path to the ".dart_tool" directory living under `engine/src/flutter`.
  ///
  /// This is a designated area for tool downloads which can be used by
  /// multiple platforms. For exampe: Flutter repo for e2e tests.
  io.Directory get engineDartToolDir => io.Directory(pathlib.join(
        engineSrcDir.path,
        'flutter',
        '.dart_tool',
      ));

  /// Path to the "dev" directory containing engine developer tools and
  /// configuration files.
  io.Directory get webUiDevDir => io.Directory(pathlib.join(
        webUiRootDir.path,
        'dev',
      ));

  /// Path to the "test" directory containing web engine tests.
  io.Directory get webUiTestDir => io.Directory(pathlib.join(
        webUiRootDir.path,
        'test',
      ));

  /// Path to the "lib" directory containing web engine code.
  io.Directory get webUiLibDir => io.Directory(pathlib.join(
        webUiRootDir.path,
        'lib',
      ));

  /// Path to the base directory to be used by Skia Gold.
  io.Directory get webUiSkiaGoldDirectory => io.Directory(pathlib.join(
        webUiDartToolDir.path,
        'skia_gold',
      ));

  /// Directory to add test results which would later be uploaded to a gcs
  /// bucket by LUCI.
  io.Directory get webUiTestResultsDirectory => io.Directory(pathlib.join(
        webUiDartToolDir.path,
        'test_results',
      ));

  /// Path to the screenshots taken by iOS simulator.
  io.Directory get webUiSimulatorScreenshotsDirectory =>
      io.Directory(pathlib.join(
        webUiDartToolDir.path,
        'ios_screenshots',
      ));

  /// Path to the script that clones the Flutter repo.
  io.File get cloneFlutterScript => io.File(pathlib.join(
        engineToolsDir.path,
        'clone_flutter.sh',
      ));

  /// Path to flutter.
  ///
  /// For example, this can be used to run `flutter pub get`.
  ///
  /// Only use [cloneFlutterScript] to clone flutter to the engine build.
  io.File get flutterCommand => io.File(pathlib.join(
        engineDartToolDir.path,
        'flutter',
        'bin',
        'flutter',
      ));
}
