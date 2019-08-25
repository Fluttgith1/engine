// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_SHELL_PLATFORM_WINDOWS_CLIENT_WRAPPER_INCLUDE_FLUTTER_FLUTTER_VIEW_CONTROLLER_H_
#define FLUTTER_SHELL_PLATFORM_WINDOWS_CLIENT_WRAPPER_INCLUDE_FLUTTER_FLUTTER_VIEW_CONTROLLER_H_

#include <flutter_windows.h>

#include <string>
#include <vector>

#include "flutter_view.h"
#include "plugin_registrar.h"

namespace flutter {

// A controller for a view displaying Flutter content.
//
// This is the primary wrapper class for the desktop C API.
// If you use this class, you should not call any of the setup or teardown
// methods in the C API directly, as this class will do that internally.
class FlutterViewController {
 public:
  // There must be only one instance of this class in an application at any
  // given time, as Flutter does not support multiple engines in one process,
  // or multiple views in one engine.
  explicit FlutterViewController(const std::string& icu_data_path);

  ~FlutterViewController();

  // Prevent copying.
  FlutterViewController(FlutterViewController const&) = delete;
  FlutterViewController& operator=(FlutterViewController const&) = delete;

  // Creates a FlutterView that can be parented into a Windows View hierarchy either using HWNDs 
  // or in the future into a CoreWindow, or using compositor.

   // The |assets_path| is the path to the flutter_assets folder for the Flutter
  // application to be run. |icu_data_path| is the path to the icudtl.dat file
  // for the version of Flutter you are using.
  //
  // The |arguments| are passed to the Flutter engine. See:
  // https://github.com/flutter/engine/blob/master/shell/common/switches.h for
  // for details. Not all arguments will apply to desktop.
  //
  // Only one Flutter view can exist at a time; see constructor comment.
  FlutterView CreateFlutterView(
      int width,
      int height,
      const std::string& assets_path,
                         const std::vector<std::string>& arguments);

  // Returns the FlutterDesktopPluginRegistrarRef to register a plugin with the
  // given name.
  //
  // The name must be unique across the application.
  FlutterDesktopPluginRegistrarRef GetRegistrarForPlugin(
      const std::string& plugin_name);

  // The FlutterView managed by this controller, if any. Returns nullptr
  // before CreateView is called.
  FlutterViewWin32* View() { return view_.get(); }

 private:
  // The path to the ICU data file. Set at creation time since it is the same
  // for any view created.
  std::string icu_data_path_;

  // The owned FlutterView, if any.
  std::shared_ptr<FlutterViewWin32> view_;

  // Handle for interacting with the C API's view controller, if any.
  FlutterDesktopViewControllerRef controller_ = nullptr;
};

}  // namespace flutter

#endif  // FLUTTER_SHELL_PLATFORM_WINDOWS_CLIENT_WRAPPER_INCLUDE_FLUTTER_FLUTTER_VIEW_CONTROLLER_H_
