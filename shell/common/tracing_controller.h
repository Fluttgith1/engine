// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SHELL_COMMON_TRACING_CONTROLLER_H_
#define SHELL_COMMON_TRACING_CONTROLLER_H_

#include <string>

#include "lib/ftl/macros.h"

namespace shell {

class TracingController {
 public:
  TracingController();
  ~TracingController();

  void StartTracing();

  void StopTracing();

  std::string PictureTracingPathForCurrentTime() const;
  std::string PictureTracingPathForCurrentTime(
      const std::string& directory) const;

  bool tracing_active() const { return tracing_active_; }

  void set_traces_base_path(std::string base_path) {
    traces_base_path_ = std::move(base_path);
  }

  void set_picture_tracing_enabled(bool enabled) {
    picture_tracing_enabled_ = enabled;
  }

  bool picture_tracing_enabled() const { return picture_tracing_enabled_; }

 private:
  std::string traces_base_path_;
  bool picture_tracing_enabled_;
  bool tracing_active_;

  std::string TracePathWithExtension(const std::string& directory,
                                     const std::string& extension) const;

  FTL_DISALLOW_COPY_AND_ASSIGN(TracingController);
};

}  // namespace shell

#endif  //  SHELL_COMMON_TRACING_CONTROLLER_H_
