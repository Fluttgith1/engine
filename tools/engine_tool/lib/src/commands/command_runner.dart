// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import 'package:args/command_runner.dart';
import 'package:engine_build_configs/engine_build_configs.dart';

import '../environment.dart';
import '../logger.dart';
import 'build_command.dart';
import 'fetch_command.dart';
import 'flags.dart';
import 'format_command.dart';
import 'lint_command.dart';
import 'query_command.dart';
import 'run_command.dart';
import 'test_command.dart';

const int _usageLineLength = 100;

/// The root command runner.
final class ToolCommandRunner extends CommandRunner<int> {
  /// Constructs the runner and populates commands, subcommands, and global
  /// options and flags.
  ToolCommandRunner({
    required this.environment,
    required this.configs,
    this.verbose = false,
    this.help = false,
  }) : super(toolName, toolDescription, usageLineLength: _usageLineLength) {
    final List<Command<int>> commands = <Command<int>>[
      FetchCommand(
        environment: environment,
        usageLineLength: _usageLineLength,
      ),
      FormatCommand(
        environment: environment,
        usageLineLength: _usageLineLength,
      ),
      QueryCommand(
        environment: environment,
        configs: configs,
        verbose: verbose,
        help: help,
        usageLineLength: _usageLineLength,
      ),
      BuildCommand(
        environment: environment,
        configs: configs,
        verbose: verbose,
        help: help,
        usageLineLength: _usageLineLength,
      ),
      RunCommand(
        environment: environment,
        configs: configs,
        verbose: verbose,
        usageLineLength: _usageLineLength,
      ),
      LintCommand(
        environment: environment,
        usageLineLength: _usageLineLength,
      ),
      TestCommand(
        environment: environment,
        configs: configs,
        verbose: verbose,
        help: help,
        usageLineLength: _usageLineLength,
      ),
    ];
    commands.forEach(addCommand);

    argParser.addFlag(
      verboseFlag,
      abbr: 'v',
      help: 'Prints verbose output',
      negatable: false,
    );
  }

  /// The name of the tool as reported in the tool's usage and help
  /// messages.
  static const String toolName = 'et';

  /// The description of the tool reported in the tool's usage and help
  /// messages.
  static const String toolDescription = 'A command line tool for working on '
      'the Flutter Engine.';

  /// The host system environment.
  final Environment environment;

  /// Build configurations loaded from the engine from under ci/builders.
  final Map<String, BuilderConfig> configs;

  /// Whether et should emit verbose logs.
  final bool verbose;

  /// Whether the invocation is for a help command
  final bool help;

  @override
  Future<int> run(Iterable<String> args) async {
    if (verbose) {
      environment.logger.level = Logger.infoLevel;
    }
    try {
      return await runCommand(parse(args)) ?? 0;
    } on FormatException catch (e) {
      environment.logger.error(e);
      return 1;
    } on UsageException catch (e) {
      environment.logger.error(e);
      return 1;
    }
  }
}
