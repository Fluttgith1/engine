// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_FML_TIME_DART_TIMESTAMP_PROVIDER_H_
#define FLUTTER_FML_TIME_DART_TIMESTAMP_PROVIDER_H_

#include "flutter/fml/time/timestamp_provider.h"

#include "flutter/fml/macros.h"
#include "flutter/fml/time/time_point.h"

namespace fml {

fml::TimePoint DartTimelineTicksSinceEpoch();

/// TimestampProvider implementation that is backed by Dart_TimelineGetTicks
class DartTimestampProvider : TimestampProvider {
 public:
  static DartTimestampProvider& Instance() {
    static DartTimestampProvider instance;
    return instance;
  }

  ~DartTimestampProvider() override;

  fml::TimePoint Now() override;

 private:
  static constexpr int64_t kNanosPerSecond = 1000000000;

  DartTimestampProvider();

  FML_DISALLOW_COPY_AND_ASSIGN(DartTimestampProvider);
};

}  // namespace fml

#endif  // FLUTTER_FML_TIME_DART_TIMESTAMP_PROVIDER_H_
