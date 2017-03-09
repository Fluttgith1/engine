// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_FML_PLATFORM_LINUX_TIMER_FD_H_
#define FLUTTER_FML_PLATFORM_LINUX_TIMER_FD_H_

// clang-format off
#if __has_include(<sys/timerfd.h>)
// clang-format on

#include <sys/timerfd.h>

#define FML_TIMERFD_AVAILABLE 1

#else  // __has_include(<sys/timerfd.h>)

#define FML_TIMERFD_AVAILABLE 0

#include <sys/types.h>
// Must come after sys/types
#include <linux/time.h>

#define TFD_TIMER_ABSTIME (1 << 0)
#define TFD_TIMER_CANCEL_ON_SET (1 << 1)

#define TFD_CLOEXEC O_CLOEXEC
#define TFD_NONBLOCK O_NONBLOCK

int timerfd_create(int clockid, int flags);

int timerfd_settime(int ufc,
                    int flags,
                    const struct itimerspec* utmr,
                    struct itimerspec* otmr);

#endif  // __has_include(<sys/timerfd.h>)

#endif  // FLUTTER_FML_PLATFORM_LINUX_TIMER_FD_H_
