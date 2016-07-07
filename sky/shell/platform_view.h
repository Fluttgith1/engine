// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SKY_SHELL_PLATFORM_VIEW_H_
#define SKY_SHELL_PLATFORM_VIEW_H_

#include "base/macros.h"
#include "base/memory/weak_ptr.h"
#include "base/synchronization/waitable_event.h"
#include "sky/shell/ui_delegate.h"

namespace sky {
namespace shell {

class Rasterizer;

class PlatformView {
 public:
  struct Config {
    Config();
    ~Config();

    base::WeakPtr<UIDelegate> ui_delegate;
    Rasterizer* rasterizer;
    scoped_refptr<base::SingleThreadTaskRunner> ui_task_runner;
  };

  struct SurfaceConfig {
    uint8_t red_bits = 8;
    uint8_t green_bits = 8;
    uint8_t blue_bits = 8;
    uint8_t alpha_bits = 8;
    uint8_t depth_bits = 0;
    uint8_t stencil_bits = 0;
  };

  // Implemented by each platform.
  static PlatformView* Create(const Config& config,
                              SurfaceConfig surface_configuration);

  virtual ~PlatformView();

  void ConnectToEngine(mojo::InterfaceRequest<SkyEngine> request);

  void NotifyCreated();

  void NotifyCreated(base::Closure continuation);

  void NotifyDestroyed();

  virtual base::WeakPtr<sky::shell::PlatformView> GetWeakViewPtr() = 0;

  virtual uint64_t DefaultFramebuffer() const = 0;

  virtual bool ContextMakeCurrent() = 0;

  virtual bool SwapBuffers() = 0;

 protected:
  explicit PlatformView(const Config& config, SurfaceConfig surface_config);

  Config config_;
  SurfaceConfig surface_config_;

 private:
  DISALLOW_COPY_AND_ASSIGN(PlatformView);
};

}  // namespace shell
}  // namespace sky

#endif  // SKY_SHELL_PLATFORM_VIEW_H_
