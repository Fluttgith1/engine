// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_FLOW_FRAME_TIMINGS_H_
#define FLUTTER_FLOW_FRAME_TIMINGS_H_

#include <mutex>

#include "flutter/common/settings.h"
#include "flutter/fml/macros.h"
#include "flutter/fml/time/time_delta.h"
#include "flutter/fml/time/time_point.h"

namespace flutter {

/// Records timestamps for various phases of a frame rendering process.
///
/// Recorder is created on vsync and destroyed after the rasterization of the
/// frame.
class FrameTimingsRecorder {
 public:
  /// Various states that the recorder can be in. When created the recorder is
  /// in an unitialized state and transtions in sequential order of the states.
  enum class State : uint32_t {
    kUninitialized = 1,
    kVsync = 2,
    kBuildStart = 3,
    kBuildEnd = 4,
    kRasterStart = 5,
    kRasterEnd = 6,
  };

  /// Default constructor, initializes the recorder with State::kUninitialized.
  FrameTimingsRecorder();

  ~FrameTimingsRecorder();

  /// Timestamp of the vsync signal.
  fml::TimePoint GetVsyncStartTime() const;

  /// Timestamp of when the frame was targeted to be presented.
  ///
  /// This is typically the next vsync signal timestamp.
  fml::TimePoint GetVsyncTargetTime() const;

  /// Timestamp of when the frame building started.
  fml::TimePoint GetBuildStartTime() const;

  /// Timestamp of when the frame was finished building.
  fml::TimePoint GetBuildEndTime() const;

  /// Timestamp of when the frame rasterization started.
  fml::TimePoint GetRasterStartTime() const;

  /// Timestamp of when the frame rasterization finished.
  fml::TimePoint GetRasterEndTime() const;

  /// Duration of the frame build time.
  fml::TimeDelta GetBuildDuration() const;

  /// Records a vsync event.
  void RecordVsync(fml::TimePoint vsync_start, fml::TimePoint vsync_target);

  /// Records a build start event.
  void RecordBuildStart(fml::TimePoint build_start);

  /// Records a build end event.
  void RecordBuildEnd(fml::TimePoint build_end);

  /// Records a raster start event.
  void RecordRasterStart(fml::TimePoint raster_start);

  /// Clones the recorder until (and including) the specified state.
  std::unique_ptr<FrameTimingsRecorder> CloneUntil(State state);

  /// Records a raster end event, and builds a `FrameTiming` that summarizes all
  /// the events. This summary is sent to the framework.
  FrameTiming RecordRasterEnd(fml::TimePoint raster_end);

 private:
  mutable std::mutex state_mutex_;
  State state_ = State::kUninitialized;

  fml::TimePoint vsync_start_ = fml::TimePoint::Min();
  fml::TimePoint vsync_target_ = fml::TimePoint::Min();
  fml::TimePoint build_start_ = fml::TimePoint::Min();
  fml::TimePoint build_end_ = fml::TimePoint::Min();
  fml::TimePoint raster_start_ = fml::TimePoint::Min();
  fml::TimePoint raster_end_ = fml::TimePoint::Min();

  FML_DISALLOW_COPY_ASSIGN_AND_MOVE(FrameTimingsRecorder);
};

}  // namespace flutter

#endif  // FLUTTER_FLOW_FRAME_TIMINGS_H_
