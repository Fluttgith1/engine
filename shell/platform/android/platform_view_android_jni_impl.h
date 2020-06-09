// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_SHELL_PLATFORM_ANDROID_PLATFORM_VIEW_ANDROID_JNI_IMPL_H_
#define FLUTTER_SHELL_PLATFORM_ANDROID_PLATFORM_VIEW_ANDROID_JNI_IMPL_H_

#include "flutter/fml/platform/android/jni_weak_ref.h"
#include "flutter/shell/platform/android/jni/platform_view_android_jni.h"

namespace flutter {

//------------------------------------------------------------------------------
/// @brief      Concrete implementation of `PlatformViewAndroidJNI` that is
///             compiled with the Android toolchain.
///
class PlatformViewAndroidJNIImpl final : public PlatformViewAndroidJNI {
 public:
  PlatformViewAndroidJNIImpl(fml::jni::JavaObjectWeakGlobalRef java_object);

  ~PlatformViewAndroidJNIImpl() override;

  void FlutterViewHandlePlatformMessage(
      fml::RefPtr<flutter::PlatformMessage> message,
      int responseId) override;

  void FlutterViewHandlePlatformMessageResponse(
      int responseId,
      std::unique_ptr<fml::Mapping> data) override;

  void FlutterViewUpdateSemantics(std::vector<uint8_t> buffer,
                                  std::vector<std::string> strings) override;

  void FlutterViewUpdateCustomAccessibilityActions(
      std::vector<uint8_t> actions_buffer,
      std::vector<std::string> strings) override;

  void FlutterViewOnFirstFrame() override;

  void FlutterViewOnPreEngineRestart() override;

  void SurfaceTextureAttachToGLContext(std::any surface_texture,
                                       int textureId) override;

  void SurfaceTextureUpdateTexImage(std::any surface_texture) override;

  void SurfaceTextureGetTransformMatrix(std::any surface_texture,
                                        SkMatrix& transform) override;

  void SurfaceTextureDetachFromGLContext(std::any surface_texture) override;

  void FlutterViewOnDisplayPlatformView(int view_id,
                                        int x,
                                        int y,
                                        int width,
                                        int height) override;

 private:
  // Reference to FlutterJNI object.
  const fml::jni::JavaObjectWeakGlobalRef java_object_;

  // Reference to the current Surface texture object.
  fml::jni::JavaObjectWeakGlobalRef surface_texture_;

  FML_DISALLOW_COPY_AND_ASSIGN(PlatformViewAndroidJNIImpl);
};

}  // namespace flutter

#endif  // FLUTTER_SHELL_PLATFORM_ANDROID_PLATFORM_VIEW_ANDROID_JNI_IMPL_H_
