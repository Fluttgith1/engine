// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_FML_MESSAGE_LOOP_H_
#define FLUTTER_FML_MESSAGE_LOOP_H_

#include <stack>

#include "flutter/fml/closure.h"
#include "flutter/fml/macros.h"
#include "flutter/fml/memory/ref_counted.h"
#include "flutter/fml/memory/ref_ptr.h"
#include "flutter/fml/task_runner.h"

namespace fml {

class TaskRunner;
class MessageLoopImpl;

class MessageLoop : public fml::RefCountedThreadSafe<MessageLoop> {
 public:
  FML_EMBEDDER_ONLY
  static MessageLoop& GetCurrent();

  bool IsValid() const;

  size_t GetActivationCount() const;

  void Run(fml::closure on_done = nullptr);

  void Terminate();

  void AddTaskObserver(intptr_t key, fml::closure callback);

  void RemoveTaskObserver(intptr_t key);

  fml::RefPtr<fml::TaskRunner> GetTaskRunner();

  // Exposed for the embedder shell which allows clients to poll for events
  // instead of dedicating a thread to the message loop.
  void RunExpiredTasksNow();

  static void EnsureInitializedForCurrentThread();

  static bool IsInitializedForCurrentThread();

  ~MessageLoop();

 private:
  friend class TaskRunner;
  friend class MessageLoopImpl;

  std::stack<fml::RefPtr<MessageLoopImpl>> impls_;

  MessageLoop();

  fml::RefPtr<MessageLoopImpl> GetLoopImpl() const;

  void PushMessageLoop();

  FML_FRIEND_MAKE_REF_COUNTED(MessageLoop);
  FML_FRIEND_REF_COUNTED_THREAD_SAFE(MessageLoop);
  FML_DISALLOW_COPY_AND_ASSIGN(MessageLoop);
};

}  // namespace fml

#endif  // FLUTTER_FML_MESSAGE_LOOP_H_
