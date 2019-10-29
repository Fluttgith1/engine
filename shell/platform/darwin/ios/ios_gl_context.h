// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_SHELL_PLATFORM_DARWIN_IOS_IOS_GL_CONTEXT_H_
#define FLUTTER_SHELL_PLATFORM_DARWIN_IOS_IOS_GL_CONTEXT_H_

#import <OpenGLES/EAGL.h>
#import <OpenGLES/ES2/gl.h>
#import <OpenGLES/ES2/glext.h>
#import <QuartzCore/CAEAGLLayer.h>

#include "flutter/fml/macros.h"
#include "flutter/fml/platform/darwin/scoped_nsobject.h"
#include "flutter/shell/common/platform_view.h"
#include "ios_gl_render_target.h"

namespace flutter {

class IOSGLContext {
 public:
  IOSGLContext();

  ~IOSGLContext();

  std::unique_ptr<IOSGLRenderTarget> CreateRenderTarget(
      fml::scoped_nsobject<CAEAGLLayer> layer);

  std::unique_ptr<GLContextSwitchManager::GLContextSwitch> MakeCurrent();

  std::unique_ptr<GLContextSwitchManager::GLContextSwitch> ResourceMakeCurrent();

  std::shared_ptr<IOSGLContextSwitchManager> GetIOSGLContextSwitchManager() {
    return gl_context_guard_manager_;
  }

  sk_sp<SkColorSpace> ColorSpace() const { return color_space_; }

  fml::scoped_nsobject<EAGLContext> GetContext() const { return gl_context_guard_manager_->context_; }

 private:
  sk_sp<SkColorSpace> color_space_;
  std::shared_ptr<IOSGLContextSwitchManager> gl_context_guard_manager_;

  FML_DISALLOW_COPY_AND_ASSIGN(IOSGLContext);
};

}  // namespace flutter

#endif  // FLUTTER_SHELL_PLATFORM_DARWIN_IOS_IOS_GL_CONTEXT_H_
