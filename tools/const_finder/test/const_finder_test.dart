// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// ignore_for_file: avoid_dynamic_calls

import 'dart:convert';
import 'dart:io';

import 'package:collection/collection.dart';
import 'package:const_finder/const_finder.dart';
import 'package:path/path.dart' as path;

void expect<T>(T value, T expected) {
  if (value != expected) {
    stderr.writeln('Expected: $expected');
    stderr.writeln('Actual:   $value');
    exitCode = -1;
  }
}

void expectInstances(dynamic value, dynamic expected, Compiler compiler) {
  // To ensure we ignore insertion order into maps as well as lists we use
  // DeepCollectionEquality as well as sort the lists.

  int compareByStringValue(dynamic a, dynamic b) {
    return a['stringValue'].compareTo(b['stringValue']) as int;
  }
  value['constantInstances'].sort(compareByStringValue);
  expected['constantInstances'].sort(compareByStringValue);

  final Equality<Object?> equality;
  if (compiler == Compiler.dart2js) {
    (value as Map<String, Object?>).remove('nonConstantLocations');
    (expected as Map<String, Object?>).remove('nonConstantLocations');
    equality = const Dart2JSDeepCollectionEquality();
  } else {
    equality = const DeepCollectionEquality();
  }
  if (!equality.equals(value, expected)) {
    stderr.writeln('Expected: ${const JsonEncoder.withIndent('  ').convert(expected)}');
    stderr.writeln('Actual:   ${const JsonEncoder.withIndent('  ').convert(value)}');
    exitCode = -1;
  }
}

// This test is assuming the `dart` used to invoke the tests is compatible
// with the version of package:kernel in //third-party/dart/pkg/kernel
final String dart = Platform.resolvedExecutable;
final String bat = Platform.isWindows ? '.bat' : '';

void _checkRecursion(String dillPath, Compiler compiler) {
  stdout.writeln('Checking recursive calls.');
  final ConstFinder finder = ConstFinder(
    kernelFilePath: dillPath,
    classLibraryUri: 'package:const_finder_fixtures/box.dart',
    className: 'Box',
  );
  // Will timeout if we did things wrong.
  jsonEncode(finder.findInstances());
}

void _checkConsts(String dillPath, Compiler compiler) {
  stdout.writeln('Checking for expected constants.');
  final ConstFinder finder = ConstFinder(
    kernelFilePath: dillPath,
    classLibraryUri: 'package:const_finder_fixtures/target.dart',
    className: 'Target',
  );
  expectInstances(
    finder.findInstances(),
    <String, dynamic>{
      'constantInstances': <Map<String, dynamic>>[
        <String, dynamic>{'stringValue': '100', 'intValue': 100, 'targetValue': null},
        <String, dynamic>{'stringValue': '102', 'intValue': 102, 'targetValue': null},
        <String, dynamic>{'stringValue': '101', 'intValue': 101},
        <String, dynamic>{'stringValue': '103', 'intValue': 103, 'targetValue': null},
        <String, dynamic>{'stringValue': '105', 'intValue': 105, 'targetValue': null},
        <String, dynamic>{'stringValue': '104', 'intValue': 104},
        <String, dynamic>{'stringValue': '106', 'intValue': 106, 'targetValue': null},
        <String, dynamic>{'stringValue': '108', 'intValue': 108, 'targetValue': null},
        <String, dynamic>{'stringValue': '107', 'intValue': 107},
        <String, dynamic>{'stringValue': '1', 'intValue': 1, 'targetValue': null},
        <String, dynamic>{'stringValue': '4', 'intValue': 4, 'targetValue': null},
        <String, dynamic>{'stringValue': '2', 'intValue': 2},
        <String, dynamic>{'stringValue': '6', 'intValue': 6, 'targetValue': null},
        <String, dynamic>{'stringValue': '8', 'intValue': 8, 'targetValue': null},
        <String, dynamic>{'stringValue': '10', 'intValue': 10, 'targetValue': null},
        <String, dynamic>{'stringValue': '9', 'intValue': 9},
        <String, dynamic>{'stringValue': '7', 'intValue': 7, 'targetValue': null},
        <String, dynamic>{'stringValue': '11', 'intValue': 11, 'targetValue': null},
        <String, dynamic>{'stringValue': '12', 'intValue': 12, 'targetValue': null},
        <String, dynamic>{'stringValue': 'package', 'intValue':-1, 'targetValue': null},
      ],
      'nonConstantLocations': <dynamic>[],
    },
    compiler,
  );

  final ConstFinder finder2 = ConstFinder(
    kernelFilePath: dillPath,
    classLibraryUri: 'package:const_finder_fixtures/target.dart',
    className: 'MixedInTarget',
  );
  expectInstances(
    finder2.findInstances(),
    <String, dynamic>{
      'constantInstances': <Map<String, dynamic>>[
        <String, dynamic>{'val': '13'},
      ],
      'nonConstantLocations': <dynamic>[],
    },
    compiler,
  );
}

void _checkDenyList(String dillPath, Compiler compiler) {
  stdout.writeln('Checking constant instances in a denylist are ignored with $compiler');
  final ConstFinder finder = ConstFinder(
    kernelFilePath: dillPath,
    classLibraryUri: 'package:const_finder_fixtures/target.dart',
    className: 'Target',
    ignoredClasses: <List<String>>[<String>[
      'package:const_finder_fixtures/denylist.dart',
      'Targets',
    ]],
  );
  expectInstances(
    finder.findInstances(),
    <String, dynamic>{
      'constantInstances': <Map<String, Object?>>[
        <String, Object?>{
          'stringValue': 'used1',
          'intValue': 1,
          'targetValue': null,
        },
        <String, Object?>{
          'stringValue': 'used2',
          'intValue': 2,
          'targetValue': null,
        },
      ],
      'nonConstantLocations': <Object?>[],
    },
    compiler,
  );
}
void _checkNonConsts(String dillPath, Compiler compiler) {
  stdout.writeln('Checking for non-constant instances with $compiler');
  final ConstFinder finder = ConstFinder(
    kernelFilePath: dillPath,
    classLibraryUri: 'package:const_finder_fixtures/target.dart',
    className: 'Target',
  );
  final String fixturesUrl = Platform.isWindows
    ? '/$fixtures'.replaceAll(Platform.pathSeparator, '/')
    : fixtures;

  expectInstances(
    finder.findInstances(),
    <String, dynamic>{
      'constantInstances': <dynamic>[
        <String, dynamic>{'stringValue': '1', 'intValue': 1, 'targetValue': null},
        <String, dynamic>{'stringValue': '4', 'intValue': 4, 'targetValue': null},
        <String, dynamic>{'stringValue': '6', 'intValue': 6, 'targetValue': null},
        <String, dynamic>{'stringValue': '8', 'intValue': 8, 'targetValue': null},
        <String, dynamic>{'stringValue': '10', 'intValue': 10, 'targetValue': null},
        <String, dynamic>{'stringValue': '9', 'intValue': 9},
        <String, dynamic>{'stringValue': '7', 'intValue': 7, 'targetValue': null},
      ],
      'nonConstantLocations': <dynamic>[
        <String, dynamic>{
          'file': 'file://$fixturesUrl/lib/consts_and_non.dart',
          'line': 14,
          'column': 26,
        },
        <String, dynamic>{
          'file': 'file://$fixturesUrl/lib/consts_and_non.dart',
          'line': 16,
          'column': 26,
        },
        <String, dynamic>{
          'file': 'file://$fixturesUrl/lib/consts_and_non.dart',
          'line': 16,
          'column': 41,
        },
        <String, dynamic>{
          'file': 'file://$fixturesUrl/lib/consts_and_non.dart',
          'line': 17,
          'column': 26,
        },
        <String, dynamic>{
          'file': 'file://$fixturesUrl/pkg/package.dart',
          'line': 14,
          'column': 25,
        }
      ]
    },
    Compiler.frontendServer, // TODO foo bar
  );
}

void checkProcessResult(ProcessResult result) {
  if (result.exitCode != 0) {
    stdout.writeln(result.stdout);
    stderr.writeln(result.stderr);
  }
  expect(result.exitCode, 0);
}

Future<void> main(List<String> args) async {
  if (args.length != 3) {
    stderr.writeln('The first argument must be the path to the frontend server dill.');
    stderr.writeln('The second argument must be the path to the flutter_patched_sdk');
    stderr.writeln('The third argument must be the path to libraries.json');
    exit(-1);
  }

  TestRunner(
    frontendServer: args[0],
    sdkRoot: args[1],
    librariesSpec: args[2],
  ).test();
}

final String basePath =
    path.canonicalize(path.join(path.dirname(Platform.script.toFilePath()), '..'));
final String fixtures = path.join(basePath, 'test', 'fixtures');
final String packageConfig = path.join(fixtures, '.dart_tool', 'package_config.json');

class TestRunner {
  TestRunner({
    required this.frontendServer,
    required this.sdkRoot,
    required this.librariesSpec,
  });

  //static final String box = path.join(fixtures, 'lib', 'box.dart');
  //static final String consts = path.join(fixtures, 'lib', 'consts.dart');
  //static final String constsAndNon = path.join(fixtures, 'lib', 'consts_and_non.dart');

  final String frontendServer;
  final String sdkRoot;
  final String librariesSpec;

  void test() {
    final List<_Test> tests = <_Test>[
      //_Test(
      //  name: 'box_frontend',
      //  dartSource: path.join(fixtures, 'lib', 'box.dart'),
      //  frontendServer: frontendServer,
      //  sdkRoot: sdkRoot,
      //  librariesSpec: librariesSpec,
      //  verify: _checkRecursion,
      //  compiler: Compiler.frontendServer,
      //),
      //_Test(
      //  name: 'box_web',
      //  dartSource: path.join(fixtures, 'lib', 'box.dart'),
      //  frontendServer: frontendServer,
      //  sdkRoot: sdkRoot,
      //  librariesSpec: librariesSpec,
      //  verify: _checkRecursion,
      //  compiler: Compiler.dart2js,
      //),
      //_Test(
      //  name: 'consts_frontend',
      //  dartSource: path.join(fixtures, 'lib', 'consts.dart'),
      //  frontendServer: frontendServer,
      //  sdkRoot: sdkRoot,
      //  librariesSpec: librariesSpec,
      //  verify: _checkConsts,
      //  compiler: Compiler.frontendServer,
      //),
      //_Test(
      //  name: 'consts_web',
      //  dartSource: path.join(fixtures, 'lib', 'consts.dart'),
      //  frontendServer: frontendServer,
      //  sdkRoot: sdkRoot,
      //  librariesSpec: librariesSpec,
      //  verify: _checkConsts,
      //  compiler: Compiler.dart2js,
      //),
      //_Test(
      //  name: 'consts_and_non_frontend',
      //  dartSource: path.join(fixtures, 'lib', 'consts_and_non.dart'),
      //  frontendServer: frontendServer,
      //  sdkRoot: sdkRoot,
      //  librariesSpec: librariesSpec,
      //  verify: _checkNonConsts,
      //  compiler: Compiler.frontendServer,
      //),
      //_Test(
      //  name: 'consts_and_non_web',
      //  dartSource: path.join(fixtures, 'lib', 'consts_and_non.dart'),
      //  frontendServer: frontendServer,
      //  sdkRoot: sdkRoot,
      //  librariesSpec: librariesSpec,
      //  verify: _checkNonConsts,
      //  compiler: Compiler.dart2js,
      //),
      //_Test(
      //  name: 'denylist_frontend',
      //  dartSource: path.join(fixtures, 'lib', 'denylist.dart'),
      //  frontendServer: frontendServer,
      //  sdkRoot: sdkRoot,
      //  librariesSpec: librariesSpec,
      //  verify: _checkDenyList,
      //  compiler: Compiler.frontendServer,
      //),
      _Test(
        name: 'denylist_web',
        dartSource: path.join(fixtures, 'lib', 'denylist.dart'),
        frontendServer: frontendServer,
        sdkRoot: sdkRoot,
        librariesSpec: librariesSpec,
        verify: _checkDenyList,
        compiler: Compiler.dart2js,
      ),
    ];
    try {
      stdout.writeln('Generating kernel fixtures...');

      for (final _Test test in tests) {
        test.run();
      }
    } finally {
      try {
        for (final _Test test in tests) {
          test.dispose();
        }
      } finally {
        stdout.writeln('Tests ${exitCode == 0 ? 'succeeded' : 'failed'} - exit code: $exitCode');
      }
    }
  }
}

enum Compiler {
  frontendServer,
  dart2js,
}

class _Test {
  _Test({
    required this.name,
    required this.dartSource,
    required this.sdkRoot,
    required this.verify,
    required this.frontendServer,
    required this.librariesSpec,
    required this.compiler,
  }) : dillPath = path.join(fixtures, '$name.dill');

  final String name;
  final String dartSource;
  final String sdkRoot;
  final String frontendServer;
  final String librariesSpec;
  final String dillPath;
  void Function(String, Compiler) verify;
  final Compiler compiler;

  final List<String> resourcesToDispose = <String>[];

  void run() {
    stdout.writeln('Compiling $dartSource to $dillPath');

    if (compiler == Compiler.frontendServer) {
      _compileTFADill();
    } else {
      _compileWebDill();
    }

    stdout.writeln('Testing $dillPath');

    verify(dillPath, compiler);
  }

  void dispose() {
    for (final String resource in resourcesToDispose) {
      stdout.writeln('Deleting $resource');
      File(resource).deleteSync();
    }
  }

  void _compileTFADill() {
    checkProcessResult(Process.runSync(dart, <String>[
      frontendServer,
      '--sdk-root=$sdkRoot',
      '--target=flutter',
      '--aot',
      '--tfa',
      '--packages=$packageConfig',
      '--output-dill=$dillPath',
      dartSource,
    ]));

    resourcesToDispose.add(dillPath);
  }

  void _compileWebDill() {
    final ProcessResult result = Process.runSync(dart, <String>[
      'compile',
      'js',
      '--libraries-spec=$librariesSpec',
      '-Ddart.vm.product=true',
      '-o',
      dillPath,
      '--packages=$packageConfig',
      '--cfe-only',
      dartSource,
    ]);
    if ((result.stderr as String).trim().isNotEmpty) {
      print(result.stderr);
    }
    checkProcessResult(result);

    resourcesToDispose.add(dillPath);
  }
}

/// Equality that casts all [num]'s to [double] before comparing.
class Dart2JSDeepCollectionEquality extends DeepCollectionEquality {
  const Dart2JSDeepCollectionEquality();

  @override
  bool equals(Object? e1, Object? e2) {
    if (e1 is num && e2 is num) {
      return e1.toDouble() == e2.toDouble();
    }
    return super.equals(e1, e2);
  }
}
