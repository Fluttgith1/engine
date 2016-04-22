// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flow/layers/layer_tree.h"

#include "base/trace_event/trace_event.h"
#include "flow/layers/layer.h"

namespace flow {

LayerTree::LayerTree() : scene_version_(0), rasterizer_tracing_threshold_(0) {
}

LayerTree::~LayerTree() {
}

SkRect LayerTree::GetBounds() const {
  return SkRect::MakeWH(frame_size_.width(), frame_size_.height());
}

void LayerTree::UpdateScene(mojo::gfx::composition::SceneUpdate* update,
                            mojo::gfx::composition::Node* container) {
  TRACE_EVENT0("flutter", "LayerTree::UpdateScene");
  root_layer_->UpdateScene(update, container);
}

}  // namespace flow
