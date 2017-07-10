// Copyright 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_FLOW_LAYERS_CHILD_SCENE_LAYER_H_
#define FLUTTER_FLOW_LAYERS_CHILD_SCENE_LAYER_H_

#include "flutter/flow/layers/layer.h"

namespace flow {

class ChildSceneLayer : public Layer {
 public:
  ChildSceneLayer();
  ~ChildSceneLayer() override;

  void set_offset(const SkPoint& offset) { offset_ = offset; }

  void set_device_pixel_ratio(float device_pixel_ratio) {
    device_pixel_ratio_ = device_pixel_ratio;
  }

  void set_physical_size(const SkISize& physical_size) {
    physical_size_ = physical_size;
  }

  void set_scene_token(uint32_t scene_token) { scene_token_ = scene_token; }

  void set_hit_testable(bool hit_testable) { hit_testable_ = hit_testable; }

  void Preroll(PrerollContext* context, const SkMatrix& matrix) override;

  void Paint(PaintContext& context) override;

  void UpdateScene(mozart::client::Session& session,
                   SceneUpdateContext& context,
                   ContainerNode& container) override;

 private:
  SkPoint offset_;
  float device_pixel_ratio_ = 1.0;
  SkISize physical_size_;
  uint32_t scene_token_ = 0;
  bool hit_testable_ = true;
  SkMatrix transform_;

  FTL_DISALLOW_COPY_AND_ASSIGN(ChildSceneLayer);
};

}  // namespace flow

#endif  // FLUTTER_FLOW_LAYERS_CHILD_SCENE_LAYER_H_
