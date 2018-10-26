// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
#ifndef FLUTTER_FLOW_EMBEDDED_VIEWS_H_
#define FLUTTER_FLOW_EMBEDDED_VIEWS_H_

#include "flutter/fml/memory/ref_counted.h"

namespace flow {

class EmbeddedViewParams {
 public:
  double x;
  double y;
  double width;
  double height;
};

// This is only used on iOS when running in a non headless mode,
// in this case ViewEmbedded is a reference to the
// FlutterPlatformViewsController which is owned by FlutterViewController.
class ViewEmbedder {
 public:
  // Must be called on the UI thread.
  virtual void CompositeEmbeddedView(int view_id,
                                     const EmbeddedViewParams& params) {}

  virtual ~ViewEmbedder() {}
};

}  // namespace flow

#endif  // FLUTTER_FLOW_EMBEDDED_VIEWS_H_
