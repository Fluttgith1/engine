// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/shell/platform/android/external_view_embedder/external_view_embedder.h"

#include "flutter/fml/trace_event.h"

namespace flutter {

AndroidExternalViewEmbedder::AndroidExternalViewEmbedder(
    std::shared_ptr<PlatformViewAndroidJNI> jni_facade,
    std::shared_ptr<AndroidContext> android_context)
    : ExternalViewEmbedder(),
      jni_facade_(jni_facade),
      android_context_(android_context),
      surface_pool_(std::make_unique<SurfacePool>()) {}

// |ExternalViewEmbedder|
void AndroidExternalViewEmbedder::PrerollCompositeEmbeddedView(
    int view_id,
    std::unique_ptr<EmbeddedViewParams> params) {
  auto rtree_factory = RTreeFactory();
  view_rtrees_[view_id] = rtree_factory.getInstance();

  TRACE_EVENT0("flutter",
               "AndroidExternalViewEmbedder::PrerollCompositeEmbeddedView");
  picture_recorders_[view_id] = std::make_unique<SkPictureRecorder>();
  picture_recorders_[view_id]->beginRecording(SkRect::Make(frame_size_),
                                              &rtree_factory);

  composition_order_.push_back(view_id);
  // Update params if they changed.
  if (view_params_.count(view_id) == 1 &&
      view_params_[view_id] == *params.get()) {
    return;
  }
  view_params_[view_id] = EmbeddedViewParams(*params.get());
}

// |ExternalViewEmbedder|
SkCanvas* AndroidExternalViewEmbedder::CompositeEmbeddedView(int view_id) {
  return picture_recorders_[view_id]->getRecordingCanvas();
}

// |ExternalViewEmbedder|
std::vector<SkCanvas*> AndroidExternalViewEmbedder::GetCurrentCanvases() {
  std::vector<SkCanvas*> canvases;
  for (size_t i = 0; i < composition_order_.size(); i++) {
    int64_t view_id = composition_order_[i];
    canvases.push_back(picture_recorders_[view_id]->getRecordingCanvas());
  }
  return canvases;
}

// |ExternalViewEmbedder|
bool AndroidExternalViewEmbedder::SubmitFrame(
    GrContext* context,
    std::unique_ptr<SurfaceFrame> frame) {
  TRACE_EVENT0("flutter", "AndroidExternalViewEmbedder::SubmitFrame");

  if (should_run_rasterizer_on_platform_thread_) {
    // Don't submit the current frame if the frame will be resubmitted.
    return true;
  }

  std::map<int64_t, std::list<SkRect>> overlay_layers;
  std::map<int64_t, sk_sp<SkPicture>> pictures;
  SkCanvas* background_canvas = frame->SkiaCanvas();

  // Restore the clip context after exiting this method since it's changed
  // below.
  SkAutoCanvasRestore save(background_canvas, /*doSave=*/true);

  for (size_t i = 0; i < composition_order_.size(); i++) {
    int64_t view_id = composition_order_[i];
    SkPoint view_offset = view_params_[view_id].offsetPixels;
    SkSize view_size = view_params_[view_id].sizePoints;

    // Display the platform view. If it's already displayed, then it's
    // just positioned and sized.
    jni_facade_->FlutterViewOnDisplayPlatformView(view_id,            //
                                                  view_offset.x(),    //
                                                  view_offset.y(),    //
                                                  view_size.width(),  //
                                                  view_size.height()  //
    );

    sk_sp<RTree> rtree = view_rtrees_[view_id];
    // Determinate if Flutter UI intersects with any of the previous
    // platform views stacked by z position.
    //
    // This is done by querying the r-tree that holds the records for the
    // picture recorder corresponding to the flow layers added after a platform
    // view layer.
    for (size_t j = i + 1; j > 0; j--) {
      int64_t current_view_id = composition_order_[j - 1];
      SkRect current_view_rect = GetPlatformViewRect(current_view_id);
      // Each rect corresponds to a native view that renders Flutter UI.
      std::list<SkRect> intersection_rects =
          rtree->searchNonOverlappingDrawnRects(current_view_rect);
      auto allocation_size = intersection_rects.size();
      // Limit the number of native views, so it doesn't grow for ever.
      //
      // In this case, the rects are merged into a single one that is the union
      // of all the rects.
      if (allocation_size > kMaxLayerAllocations) {
        SkRect joined_rect;
        for (const SkRect& rect : intersection_rects) {
          joined_rect.join(rect);
        }
        intersection_rects.clear();
        intersection_rects.push_back(joined_rect);
      }
      for (SkRect& intersection_rect : intersection_rects) {
        // Get the intersection rect between the current rect
        // and the platform view rect.
        // joined_rect.intersect(platform_view_rect);
        // Subpixels in the platform may not align with the canvas subpixels.
        //
        // To workaround it, round the floating point bounds and make the rect
        // slighly larger. For example, {0.3, 0.5, 3.1, 4.7} becomes {0, 0, 4,
        // 5}.
        intersection_rect.setLTRB(std::floor(intersection_rect.left()),  //
                                  std::floor(intersection_rect.top()),   //
                                  std::ceil(intersection_rect.right()),  //
                                  std::ceil(intersection_rect.bottom())  //
        );
        // Clip the background canvas, so it doesn't contain any of the pixels
        // drawn on the overlay layer.
        background_canvas->clipRect(joined_rect, SkClipOp::kDifference);
      }
      overlay_layers[current_view_id] = intersection_rects;
    }
    pictures[view_id] = picture_recorders_[view_id]->finishRecordingAsPicture();
    background_canvas->drawPicture(pictures[view_id]);
  }
  // Submit the background canvas frame before switching the GL context to
  // the surfaces above.
  frame->Submit();

  for (size_t i = 0; i < composition_order_.size(); i++) {
    int64_t view_id = composition_order_[i];
    for (SkRect& overlay_rect : overlay_layers[view_id]) {
      CreateSurfaceIfNeeded(context,            //
                            view_id,            //
                            pictures[view_id],  //
                            overlay_rect,       //
                            )
          ->Submit();
    }
  }
  return true;
}

// |ExternalViewEmbedder|
std::unique_ptr<SurfaceFrame>
AndroidExternalViewEmbedder::CreateSurfaceIfNeeded(GrContext* context,
                                                   int64_t view_id,
                                                   sk_sp<SkPicture> picture,
                                                   SkRect& rect) {
  std::shared_ptr<OverlayLayer> layer =
      surface_pool_->GetLayer(context, jni_facade_, android_context_);

  std::unique_ptr<SurfaceFrame> frame =
      layer->surface->AcquireFrame(frame_size_);
  // Display the overlay surface. If it's already displayed, then it's
  // just positioned and sized.
  jni_facade_->FlutterViewDisplayOverlaySurface(layer->id,     //
                                                rect.x(),      //
                                                rect.y(),      //
                                                rect.width(),  //
                                                rect.height()  //
  );
  SkCanvas* overlay_canvas = frame->SkiaCanvas();
  overlay_canvas->clear(SK_ColorTRANSPARENT);
  // Offset the picture since its absolute position on the scene is determined
  // by the position of the overlay view.
  overlay_canvas->translate(-rect.x(), -rect.y());
  overlay_canvas->drawPicture(picture);
  return frame;
}

// |ExternalViewEmbedder|
PostPrerollResult AndroidExternalViewEmbedder::PostPrerollAction(
    fml::RefPtr<fml::RasterThreadMerger> raster_thread_merger) {
  // This frame may remove existing platform views that aren't contained
  // in `composition_order_`.
  //
  // If this frame doesn't have platform views, it's still required to keep
  // the rasterizer running on the platform thread for at least one more
  // frame.
  //
  // To keep the rasterizer running on the platform thread one more frame,
  // `kDefaultMergedLeaseDuration` must be at least `1`.
  bool has_platform_views = composition_order_.size() > 0;
  if (has_platform_views) {
    if (raster_thread_merger->IsMerged()) {
      raster_thread_merger->ExtendLeaseTo(kDefaultMergedLeaseDuration);
    } else {
      // Merge the raster and platform threads in `EndFrame`.
      should_run_rasterizer_on_platform_thread_ = true;
      CancelFrame();
      return PostPrerollResult::kResubmitFrame;
    }
  }
  return PostPrerollResult::kSuccess;
}

// |ExternalViewEmbedder|
SkCanvas* AndroidExternalViewEmbedder::GetRootCanvas() {
  // On Android, the root surface is created from the on-screen render target.
  return nullptr;
}

void AndroidExternalViewEmbedder::Reset() {
  composition_order_.clear();
  picture_recorders_.clear();
}

// |ExternalViewEmbedder|
void AndroidExternalViewEmbedder::BeginFrame(SkISize frame_size,
                                             GrContext* context,
                                             double device_pixel_ratio) {
  Reset();
  frame_size_ = frame_size;
}

// |ExternalViewEmbedder|
void AndroidExternalViewEmbedder::CancelFrame() {
  Reset();
}

// |ExternalViewEmbedder|
void AndroidExternalViewEmbedder::EndFrame(
    fml::RefPtr<fml::RasterThreadMerger> raster_thread_merger) {
  if (should_run_rasterizer_on_platform_thread_) {
    raster_thread_merger->MergeWithLease(kDefaultMergedLeaseDuration);
    should_run_rasterizer_on_platform_thread_ = false;
  }
}

}  // namespace flutter
