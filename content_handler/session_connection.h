// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_CONTENT_HANDLER_SESSION_CONNECTION_H_
#define FLUTTER_CONTENT_HANDLER_SESSION_CONNECTION_H_

#include "apps/mozart/lib/scene/client/resources.h"
#include "apps/mozart/lib/scene/client/session.h"
#include "flutter/common/threads.h"
#include "lib/fidl/cpp/bindings/interface_handle.h"
#include "lib/ftl/macros.h"
#include "magenta/system/ulib/mx/include/mx/eventpair.h"

namespace flutter_runner {

class SessionConnection {
 public:
  SessionConnection(fidl::InterfaceHandle<mozart2::Session> session_handle,
                    mx::eventpair import_token);

  ~SessionConnection();

  mozart::client::ImportNode& root_node() {
    ASSERT_IS_GPU_THREAD;
    return root_node_;
  }

  mozart::client::Session* session() {
    ASSERT_IS_GPU_THREAD;
    return &session_;
  }

  void Present(ftl::Closure on_present_callback);

 private:
  mozart::client::Session session_;
  mozart::client::ImportNode root_node_;
  mozart::client::Session::PresentCallback present_callback_;
  ftl::Closure pending_on_present_callback_;

  void OnSessionError();

  void EnqueueClearOps();

  void OnPresent(mozart2::PresentationInfoPtr info);

  FTL_DISALLOW_COPY_AND_ASSIGN(SessionConnection);
};

}  // namespace flutter_runner

#endif  // FLUTTER_CONTENT_HANDLER_SESSION_CONNECTION_H_
