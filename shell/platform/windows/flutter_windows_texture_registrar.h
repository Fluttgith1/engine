// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_SHELL_PLATFORM_WINDOWS_FLUTTER_WINDOWS_TEXTURE_REGISTRAR_H_
#define FLUTTER_SHELL_PLATFORM_WINDOWS_FLUTTER_WINDOWS_TEXTURE_REGISTRAR_H_

#include <functional>
#include <memory>
#include <mutex>
#include <unordered_map>

#include "flutter/shell/platform/common/public/flutter_texture_registrar.h"
#include "flutter/shell/platform/windows/external_texture.h"

namespace flutter {

class FlutterWindowsEngine;

// An object managing the registration of an external texture.
// Thread safety: All member methods are thread safe.
class FlutterWindowsTextureRegistrar {
 public:
  explicit FlutterWindowsTextureRegistrar(FlutterWindowsEngine* engine,
                                          const GlProcs& gl_procs);

  // Registers a texture described by the given |texture_info| object.
  // Returns the non-zero, positive texture id or -1 on error.
  int64_t RegisterTexture(const FlutterDesktopTextureInfo* texture_info);

  // Attempts to unregister the texture identified by |texture_id|.
  void UnregisterTexture(int64_t texture_id,
                         std::function<void()> callback = nullptr);

  // Notifies the engine about a new frame being available.
  // Returns true on success.
  bool MarkTextureFrameAvailable(int64_t texture_id);

  // Attempts to populate the given |texture| by copying the
  // contents of the texture identified by |texture_id|.
  // Returns true on success.
  bool PopulateTexture(int64_t texture_id,
                       size_t width,
                       size_t height,
                       FlutterOpenGLTexture* texture);

  // Populates the OpenGL function pointers in |gl_procs|.
  static void ResolveGlFunctions(GlProcs& gl_procs);

 private:
  FlutterWindowsEngine* engine_ = nullptr;
  const GlProcs& gl_procs_;

  // All registered textures, keyed by their IDs.
  std::unordered_map<int64_t, std::unique_ptr<flutter::ExternalTexture>>
      textures_;
  std::mutex map_mutex_;

  int64_t EmplaceTexture(std::unique_ptr<ExternalTexture> texture);
};

};  // namespace flutter

#endif  // FLUTTER_SHELL_PLATFORM_WINDOWS_FLUTTER_WINDOWS_TEXTURE_REGISTRAR_H_
