// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#import <Cocoa/Cocoa.h>

#include <iostream>

#include "flutter/fml/message_loop.h"
#include "flutter/shell/common/shell.h"
#include "flutter/shell/common/switches.h"
#include "flutter/shell/platform/darwin/desktop/flutter_application_delegate.h"
#include "flutter/shell/testing/testing.h"

int main(int argc, const char* argv[]) {
  std::vector<std::string> args_vector;

  for (NSString* arg in [NSProcessInfo processInfo].arguments) {
    args_vector.emplace_back(arg.UTF8String);
  }

  auto command_line = fxl::CommandLineFromIterators(args_vector.begin(), args_vector.end());

  // Print help.
  if (command_line.HasOption(shell::FlagForSwitch(shell::Switch::Help))) {
    shell::PrintUsage([NSProcessInfo processInfo].processName.UTF8String);
    return EXIT_SUCCESS;
  }

  // Decide between interactive and non-interactive modes.
  if (command_line.HasOption(shell::FlagForSwitch(shell::Switch::NonInteractive))) {
    if (!shell::InitForTesting(std::move(command_line)))
      return 1;
    fml::MessageLoop::GetCurrent().Run();
    return EXIT_SUCCESS;
  } else {
    [NSApplication sharedApplication].delegate =
        [[[FlutterApplicationDelegate alloc] init] autorelease];
    return NSApplicationMain(argc, argv);
  }
}
