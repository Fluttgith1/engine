// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/lib/ui/volatile_path_tracker.h"

namespace flutter {

VolatilePathTracker::VolatilePathTracker(
    fml::RefPtr<fml::TaskRunner> ui_task_runner)
    : ui_task_runner_(ui_task_runner) {}

void VolatilePathTracker::insert(std::shared_ptr<Path> path) {
  FML_DCHECK(ui_task_runner_->RunsTasksOnCurrentThread());
  FML_DCHECK(path);
  FML_DCHECK(path->path_.isVolatile());
  paths_.insert(path);
}

void VolatilePathTracker::erase(std::shared_ptr<Path> path) {
  FML_DCHECK(path);
  fml::TaskRunner::RunNowOrPostTask(ui_task_runner_,
                                    [&]() { paths_.erase(path); });
}

void VolatilePathTracker::onFrame() {
  FML_DCHECK(ui_task_runner_->RunsTasksOnCurrentThread());
  TRACE_EVENT0("flutter", "VolatilePathTracker::onFrame");
  for (auto it = paths_.begin(), last = paths_.end(); it != last;) {
    auto path = *it;
    path->frame_count++;
    if (path->frame_count >= number_of_frames_until_non_volatile) {
      path->path_.setIsVolatile(false);
      path->tracking_volatility = false;
      it = paths_.erase(it);
    } else {
      ++it;
    }
  }
}

}  // namespace flutter
