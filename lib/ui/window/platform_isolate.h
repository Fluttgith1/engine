// Copyright 2023 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_LIB_UI_WINDOW_PLATFORM_ISOLATE_H_
#define FLUTTER_LIB_UI_WINDOW_PLATFORM_ISOLATE_H_

#include "third_party/dart/runtime/include/dart_api.h"

namespace flutter {

class PlatformIsolateNativeApi {
 public:
  static void Spawn(Dart_Handle entry_point, Dart_Handle debug_name);
  static uint32_t GetCurrentThreadId();

  static fml::RefPtr<fml::TaskRunner> global_platform_task_runner;
  static Settings global_settings;
};

}  // namespace flutter

#endif  // FLUTTER_LIB_UI_WINDOW_PLATFORM_ISOLATE_H_
