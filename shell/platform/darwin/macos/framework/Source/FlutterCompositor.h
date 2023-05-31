// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_COMPOSITOR_H_
#define FLUTTER_COMPOSITOR_H_

#include <functional>
#include <list>

#include "flutter/fml/macros.h"
#import "flutter/shell/platform/darwin/macos/framework/Source/FlutterPlatformViewController.h"
#import "flutter/shell/platform/darwin/macos/framework/Source/FlutterViewProvider.h"
#include "flutter/shell/platform/embedder/embedder.h"

@class FlutterMutatorView;

namespace flutter {

// FlutterCompositor creates and manages the backing stores used for
// rendering Flutter content and presents Flutter content and Platform views.
// Platform views are not yet supported.
class FlutterCompositor {
 public:
  // Create a FlutterCompositor with a view provider.
  //
  // The view_provider is used to query FlutterViews from view IDs,
  // which are used for presenting and creating backing stores.
  // It must not be null, and is typically FlutterViewEngineProvider.
  FlutterCompositor(id<FlutterViewProvider> view_provider,
                    int64_t view_id,
                    FlutterPlatformViewController* platform_views_controller);

  ~FlutterCompositor() = default;

  // Creates a backing store and saves updates the backing_store_out data with
  // the new FlutterBackingStore data.
  //
  // If the backing store is being requested for the first time for a given
  // frame, this compositor does not create a new backing store but rather
  // returns the backing store associated with the FlutterView's
  // FlutterSurfaceManager.
  //
  // Any additional state allocated for the backing store and saved as
  // user_data in the backing store must be collected in the backing_store's
  // destruction_callback field which will be called when the embedder collects
  // the backing store.
  bool CreateBackingStore(const FlutterBackingStoreConfig* config,
                          FlutterBackingStore* backing_store_out);

  // Presents the FlutterLayers by updating the FlutterView. Sets frame_started_
  // to false.
  bool Present(const FlutterLayer** layers, size_t layers_count);

 private:
  void PresentPlatformViews(FlutterView* default_base_view,
                            const FlutterLayer** layers,
                            size_t layers_count);

  // Presents the platform view layer represented by `layer`. `layer_index` is
  // used to position the layer in the z-axis. If the layer does not have a
  // superview, it will become subview of `default_base_view`.
  FlutterMutatorView* PresentPlatformView(FlutterView* default_base_view,
                                          const FlutterLayer* layer,
                                          size_t layer_position);

  id<FlutterViewProvider> const view_provider_;
  int64_t view_id_;

  // The controller used to manage creation and deletion of platform views.
  const FlutterPlatformViewController* platform_view_controller_;

  // Platform view to FlutterMutatorView that contains it.
  NSMapTable<NSView*, FlutterMutatorView*>* mutator_views_;

  FML_DISALLOW_COPY_AND_ASSIGN(FlutterCompositor);
};

}  // namespace flutter

#endif  // FLUTTER_COMPOSITOR_H_
