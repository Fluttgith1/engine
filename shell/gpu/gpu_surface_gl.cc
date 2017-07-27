// Copyright 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "gpu_surface_gl.h"

#include "flutter/glue/trace_event.h"
#include "lib/ftl/arraysize.h"
#include "lib/ftl/logging.h"
#include "third_party/skia/include/core/SkSurface.h"
#include "third_party/skia/include/gpu/GrBackendSurface.h"
#include "third_party/skia/include/gpu/GrContextOptions.h"
#include "third_party/skia/include/gpu/gl/GrGLInterface.h"

namespace shell {

// Default maximum number of budgeted resources in the cache.
static const int kGrCacheMaxCount = 8192;

// Default maximum number of bytes of GPU memory of budgeted resources in the
// cache.
static const size_t kGrCacheMaxByteSize = 512 * (1 << 20);

GPUSurfaceGL::GPUSurfaceGL(GPUSurfaceGLDelegate* delegate)
    : delegate_(delegate),
      onscreen_surface_supports_sgrb_(delegate_->SurfaceSupportsSRGB()),
      weak_factory_(this) {
  if (!delegate_->GLContextMakeCurrent()) {
    FTL_LOG(ERROR)
        << "Could not make the context current to setup the gr context.";
    return;
  }

  auto backend_context =
      reinterpret_cast<GrBackendContext>(GrGLCreateNativeInterface());

  GrContextOptions options;
  options.fRequireDecodeDisableForSRGB = false;

  auto context = sk_sp<GrContext>(
      GrContext::Create(kOpenGL_GrBackend, backend_context, options));

  if (context == nullptr) {
    FTL_LOG(ERROR) << "Failed to setup Skia Gr context.";
    return;
  }

  context_ = std::move(context);

  context_->setResourceCacheLimits(kGrCacheMaxCount, kGrCacheMaxByteSize);

  delegate_->GLContextClearCurrent();

  valid_ = true;
}

GPUSurfaceGL::~GPUSurfaceGL() {
  if (!valid_) {
    return;
  }

  if (!delegate_->GLContextMakeCurrent()) {
    FTL_LOG(ERROR) << "Could not make the context current to destroy the "
                      "GrContext resources.";
    return;
  }

  onscreen_surface_ = nullptr;
  offscreen_surface_ = nullptr;
  context_->releaseResourcesAndAbandonContext();
  context_ = nullptr;

  delegate_->GLContextClearCurrent();
}

bool GPUSurfaceGL::IsValid() {
  return valid_;
}

static sk_sp<SkSurface> WrapOnscreenSurface(GrContext* context,
                                            const SkISize& size,
                                            intptr_t fbo,
                                            bool supports_srgb) {
  const GrGLFramebufferInfo framebuffer_info = {
      .fFBOID = fbo,
  };

  const GrPixelConfig pixel_config =
      supports_srgb ? kSRGBA_8888_GrPixelConfig : kRGBA_8888_GrPixelConfig;

  GrBackendRenderTarget render_target(size.fWidth,      // width
                                      size.fHeight,     // height
                                      0,                // sample count
                                      0,                // stencil bits (TODO)
                                      pixel_config,     // pixel config
                                      framebuffer_info  // framebuffer info
                                      );

  sk_sp<SkColorSpace> colorspace =
      supports_srgb ? SkColorSpace::MakeSRGB() : nullptr;

  SkSurfaceProps surface_props(
      SkSurfaceProps::InitType::kLegacyFontHost_InitType);

  return SkSurface::MakeFromBackendRenderTarget(
      context,                                       // gr context
      render_target,                                 // render target
      GrSurfaceOrigin::kBottomLeft_GrSurfaceOrigin,  // origin
      colorspace,                                    // colorspace
      &surface_props                                 // surface properties
      );
}

static sk_sp<SkSurface> CreateOffscreenSurface(GrContext* context,
                                               const SkISize& size) {
  const SkImageInfo image_info =
      SkImageInfo::MakeS32(size.fWidth, size.fHeight, kOpaque_SkAlphaType);

  const SkSurfaceProps surface_props(
      SkSurfaceProps::InitType::kLegacyFontHost_InitType);

  return SkSurface::MakeRenderTarget(
      context,                      // context
      SkBudgeted::kNo,              // budgeted
      image_info,                   // image info
      0,                            // sample count
      kBottomLeft_GrSurfaceOrigin,  // surface origin
      &surface_props                // surface props
      );
}

bool GPUSurfaceGL::CreateOrUpdateSurfaces(const SkISize& size) {
  if (onscreen_surface_ != nullptr &&
      size == SkISize::Make(onscreen_surface_->width(),
                            onscreen_surface_->height())) {
    // We know that if there is an offscreen surface, it will be sized to be
    // equal to the size of the onscreen surface. And the onscreen surface size
    // appears unchained. So bail.
    return true;
  }

  // We need to do some updates.
  TRACE_EVENT0("flutter", "UpdateSurfacesSize");

  // Either way, we need to get rid of previous surfaces.
  onscreen_surface_ = nullptr;
  offscreen_surface_ = nullptr;

  if (size.isEmpty()) {
    FTL_LOG(ERROR) << "Cannot create surfaces of empty size.";
    return false;
  }

  sk_sp<SkSurface> onscreen_surface, offscreen_surface;

  onscreen_surface =
      WrapOnscreenSurface(context_.get(), size, delegate_->GLContextFBO(),
                          onscreen_surface_supports_sgrb_);
  if (onscreen_surface == nullptr) {
    FTL_LOG(ERROR) << "Could not wrap onscreen surface.";
    return false;
  }

  if (!onscreen_surface_supports_sgrb_) {
    offscreen_surface = CreateOffscreenSurface(context_.get(), size);
    if (offscreen_surface == nullptr) {
      FTL_LOG(ERROR) << "Could not create offscreen surface.";
      return false;
    }
  }

  onscreen_surface_ = std::move(onscreen_surface);
  offscreen_surface_ = std::move(offscreen_surface);

  return true;
}

std::unique_ptr<SurfaceFrame> GPUSurfaceGL::AcquireFrame(const SkISize& size) {
  if (delegate_ == nullptr) {
    return nullptr;
  }

  if (!delegate_->GLContextMakeCurrent()) {
    FTL_LOG(ERROR)
        << "Could not make the context current to acquire the frame.";
    return nullptr;
  }

  sk_sp<SkSurface> surface = AcquireRenderSurface(size);

  if (surface == nullptr) {
    return nullptr;
  }

  auto weak_this = weak_factory_.GetWeakPtr();

  SurfaceFrame::SubmitCallback submit_callback =
      [weak_this](const SurfaceFrame& surface_frame, SkCanvas* canvas) {
        return weak_this ? weak_this->PresentSurface(canvas) : false;
      };

  return std::make_unique<SurfaceFrame>(surface, submit_callback);
}

bool GPUSurfaceGL::PresentSurface(SkCanvas* canvas) {
  if (delegate_ == nullptr || canvas == nullptr) {
    return false;
  }

  if (!onscreen_surface_supports_sgrb_) {
    // Because the surface did not support SRGB, we rendered offscreen surface.
    // Now we must ensure that the texture is copied onscreen.
    TRACE_EVENT0("flutter", "CopyTextureOnscreen");
    FTL_DCHECK(offscreen_surface_ != nullptr);
    onscreen_surface_->getCanvas()->drawImage(
        offscreen_surface_->makeImageSnapshot(),  // image
        0,                                        // left
        0,                                        // top
        nullptr                                   // paint (TODO)
        );
  }

  {
    TRACE_EVENT0("flutter", "SkCanvas::Flush");
    onscreen_surface_->getCanvas()->flush();
  }

  delegate_->GLContextPresent();

  return true;
}

sk_sp<SkSurface> GPUSurfaceGL::AcquireRenderSurface(const SkISize& size) {
  if (!CreateOrUpdateSurfaces(size)) {
    return nullptr;
  }

  return onscreen_surface_supports_sgrb_ ? onscreen_surface_
                                         : offscreen_surface_;
}

GrContext* GPUSurfaceGL::GetContext() {
  return context_.get();
}

}  // namespace shell
