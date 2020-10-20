// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_SHELL_PLATFORM_DARWIN_MACOS_FRAMEWORK_SOURCE_FLUTTERPLATFORMVIEWS_INTERNAL_H_
#define FLUTTER_SHELL_PLATFORM_DARWIN_MACOS_FRAMEWORK_SOURCE_FLUTTERPLATFORMVIEWS_INTERNAL_H_

#include <map>
#include "flutter/fml/macros.h"
// #include "flutter/fml/platform/darwin/scoped_nsobject.h"
#include "flutter/shell/platform/darwin/common/framework/Headers/FlutterChannels.h"

namespace flutter {

class FlutterPlatformViewsControllerMacOS {
 public:
  FlutterPlatformViewsControllerMacOS();

  ~FlutterPlatformViewsControllerMacOS();

  void OnMethodCall(FlutterMethodCall* call, FlutterResult& result);

 private:
  // fml::scoped_nsobject<NSView> view;

  FML_DISALLOW_COPY_AND_ASSIGN(FlutterPlatformViewsControllerMacOS);
};

}  // namespace flutter

#endif 
