// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#import <UIKit/UIKit.h>
#import "AppDelegate.h"
extern "C" {
int RunBenchmarks(int argc, char** argv);
}
int main(int argc, char* argv[]) {
  return RunBenchmarks(argc, argv);
}
