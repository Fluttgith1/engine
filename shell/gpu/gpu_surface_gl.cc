// Copyright 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "gpu_surface_gl.h"

#include "flutter/fml/arraysize.h"
#include "flutter/fml/logging.h"
#include "flutter/fml/trace_event.h"
#include "flutter/shell/common/persistent_cache.h"
#include "third_party/skia/include/core/SkColorFilter.h"
#include "third_party/skia/include/core/SkSurface.h"
#include "third_party/skia/include/gpu/GrBackendSurface.h"
#include "third_party/skia/include/gpu/GrContextOptions.h"
#include "third_party/skia/include/gpu/gl/GrGLAssembleInterface.h"
#include "third_party/skia/include/gpu/gl/GrGLInterface.h"

// These are common defines present on all OpenGL headers. However, we don't
// want to perform GL header reasolution on each platform we support. So just
// define these upfront. It is unlikely we will need more. But, if we do, we can
// add the same here.
#define GPU_GL_RGBA8 0x8058
#define GPU_GL_RGBA4 0x8056
#define GPU_GL_RGB565 0x8D62
#define GPU_GL_VERSION 0x1F02

#ifdef ERROR
#undef ERROR
#endif

namespace shell {

// Default maximum number of budgeted resources in the cache.
static const int kGrCacheMaxCount = 8192;

// Default maximum number of bytes of GPU memory of budgeted resources in the
// cache.
static const size_t kGrCacheMaxByteSize = 512 * (1 << 20);

// Version string prefix that identifies an OpenGL ES implementation.
static const char kGLESVersionPrefix[] = "OpenGL ES";

GPUSurfaceGL::GPUSurfaceGL(GPUSurfaceGLDelegate* delegate)
    : delegate_(delegate), weak_factory_(this) {
  if (!delegate_->GLContextMakeCurrent()) {
    FML_LOG(ERROR)
        << "Could not make the context current to setup the gr context.";
    return;
  }

  proc_resolver_ = delegate_->GetGLProcResolver();

  GrContextOptions options;

  options.fPersistentCache = PersistentCache::GetCacheForProcess();

  options.fAvoidStencilBuffers = true;

  // To get video playback on the widest range of devices, we limit Skia to
  // ES2 shading language when the ES3 external image extension is missing.
  options.fPreferExternalImagesOverES3 = true;

  sk_sp<const GrGLInterface> interface;

  if (proc_resolver_ == nullptr) {
    interface = GrGLMakeNativeInterface();
  } else {
    auto gl_get_proc = [](void* context,
                          const char gl_proc_name[]) -> GrGLFuncPtr {
      return reinterpret_cast<GrGLFuncPtr>(
          reinterpret_cast<GPUSurfaceGL*>(context)->proc_resolver_(
              gl_proc_name));
    };

    if (IsProcResolverOpenGLES()) {
      interface = GrGLMakeAssembledGLESInterface(this, gl_get_proc);
    } else {
      interface = GrGLMakeAssembledGLInterface(this, gl_get_proc);
    }
  }

  auto context = GrContext::MakeGL(interface, options);

  if (context == nullptr) {
    FML_LOG(ERROR) << "Failed to setup Skia Gr context.";
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
    FML_LOG(ERROR) << "Could not make the context current to destroy the "
                      "GrContext resources.";
    return;
  }

  onscreen_surface_ = nullptr;
  context_->releaseResourcesAndAbandonContext();
  context_ = nullptr;

  delegate_->GLContextClearCurrent();
}

bool GPUSurfaceGL::IsProcResolverOpenGLES() {
  using GLGetStringProc = const char* (*)(uint32_t);
  GLGetStringProc gl_get_string =
      reinterpret_cast<GLGetStringProc>(proc_resolver_("glGetString"));
  FML_CHECK(gl_get_string)
      << "The GL proc resolver could not resolve glGetString";
  const char* gl_version_string = gl_get_string(GPU_GL_VERSION);
  FML_CHECK(gl_version_string)
      << "The GL proc resolver's glGetString(GL_VERSION) failed";

  return strncmp(gl_version_string, kGLESVersionPrefix,
                 strlen(kGLESVersionPrefix)) == 0;
}

// |shell::Surface|
bool GPUSurfaceGL::IsValid() {
  return valid_;
}

static SkColorType FirstSupportedColorType(GrContext* context,
                                           GrGLenum* format) {
#define RETURN_IF_RENDERABLE(x, y)                 \
  if (context->colorTypeSupportedAsSurface((x))) { \
    *format = (y);                                 \
    return (x);                                    \
  }
  RETURN_IF_RENDERABLE(kRGBA_8888_SkColorType, GPU_GL_RGBA8);
  RETURN_IF_RENDERABLE(kARGB_4444_SkColorType, GPU_GL_RGBA4);
  RETURN_IF_RENDERABLE(kRGB_565_SkColorType, GPU_GL_RGB565);
  return kUnknown_SkColorType;
}

static sk_sp<SkSurface> WrapOnscreenSurface(GrContext* context,
                                            const SkISize& size,
                                            intptr_t fbo) {
  GrGLenum format;
  const SkColorType color_type = FirstSupportedColorType(context, &format);

  GrGLFramebufferInfo framebuffer_info = {};
  framebuffer_info.fFBOID = static_cast<GrGLuint>(fbo);
  framebuffer_info.fFormat = format;

  GrBackendRenderTarget render_target(size.width(),     // width
                                      size.height(),    // height
                                      0,                // sample count
                                      0,                // stencil bits (TODO)
                                      framebuffer_info  // framebuffer info
  );

  sk_sp<SkColorSpace> colorspace = nullptr;

  SkSurfaceProps surface_props(
      SkSurfaceProps::InitType::kLegacyFontHost_InitType);

  return SkSurface::MakeFromBackendRenderTarget(
      context,                                       // gr context
      render_target,                                 // render target
      GrSurfaceOrigin::kBottomLeft_GrSurfaceOrigin,  // origin
      color_type,                                    // color type
      colorspace,                                    // colorspace
      &surface_props                                 // surface properties
  );
}

static sk_sp<SkSurface> CreateOffscreenSurface(GrContext* context,
                                               const SkISize& size) {
  const SkImageInfo image_info =
      SkImageInfo::MakeN32(size.fWidth, size.fHeight, kOpaque_SkAlphaType);

  const SkSurfaceProps surface_props(
      SkSurfaceProps::InitType::kLegacyFontHost_InitType);

  return SkSurface::MakeRenderTarget(context, SkBudgeted::kNo, image_info, 0,
                                     kBottomLeft_GrSurfaceOrigin,
                                     &surface_props);
}

bool GPUSurfaceGL::CreateOrUpdateSurfaces(const SkISize& size) {
  if (onscreen_surface_ != nullptr &&
      size == SkISize::Make(onscreen_surface_->width(),
                            onscreen_surface_->height())) {
    // Surface size appears unchanged. So bail.
    return true;
  }

  // We need to do some updates.
  TRACE_EVENT0("flutter", "UpdateSurfacesSize");

  // Either way, we need to get rid of previous surface.
  onscreen_surface_ = nullptr;
  offscreen_surface_ = nullptr;

  if (size.isEmpty()) {
    FML_LOG(ERROR) << "Cannot create surfaces of empty size.";
    return false;
  }

  sk_sp<SkSurface> onscreen_surface, offscreen_surface;

  onscreen_surface =
      WrapOnscreenSurface(context_.get(),            // GL context
                          size,                      // root surface size
                          delegate_->GLContextFBO()  // window FBO ID
      );

  if (onscreen_surface == nullptr) {
    // If the onscreen surface could not be wrapped. There is absolutely no
    // point in moving forward.
    FML_LOG(ERROR) << "Could not wrap onscreen surface.";
    return false;
  }

  if (delegate_->UseOffscreenSurface()) {
    offscreen_surface = CreateOffscreenSurface(context_.get(), size);
    if (offscreen_surface == nullptr) {
      FML_LOG(ERROR) << "Could not create offscreen surface.";
      return false;
    }
  }

  onscreen_surface_ = std::move(onscreen_surface);
  offscreen_surface_ = std::move(offscreen_surface);

  return true;
}

// |shell::Surface|
SkMatrix GPUSurfaceGL::GetRootTransformation() const {
  return delegate_->GLContextSurfaceTransformation();
}

// |shell::Surface|
std::unique_ptr<SurfaceFrame> GPUSurfaceGL::AcquireFrame(const SkISize& size) {
  if (delegate_ == nullptr) {
    return nullptr;
  }

  if (!delegate_->GLContextMakeCurrent()) {
    FML_LOG(ERROR)
        << "Could not make the context current to acquire the frame.";
    return nullptr;
  }

  const auto root_surface_transformation = GetRootTransformation();

  sk_sp<SkSurface> surface =
      AcquireRenderSurface(size, root_surface_transformation);

  if (surface == nullptr) {
    return nullptr;
  }

  surface->getCanvas()->setMatrix(root_surface_transformation);

  SurfaceFrame::SubmitCallback submit_callback =
      [weak = weak_factory_.GetWeakPtr()](const SurfaceFrame& surface_frame,
                                          SkCanvas* canvas) {
        return weak ? weak->PresentSurface(canvas) : false;
      };

  return std::make_unique<SurfaceFrame>(surface, submit_callback);
}

bool GPUSurfaceGL::PresentSurface(SkCanvas* canvas) {
  if (delegate_ == nullptr || canvas == nullptr || context_ == nullptr) {
    return false;
  }

  if (offscreen_surface_ != nullptr) {
    TRACE_EVENT0("flutter", "CopyTextureOnscreen");
    SkPaint paint;
    onscreen_surface_->getCanvas()->drawImage(
        offscreen_surface_->makeImageSnapshot(), 0, 0, &paint);
  }

  {
    TRACE_EVENT0("flutter", "SkCanvas::Flush");
    onscreen_surface_->getCanvas()->flush();
  }

  if (!delegate_->GLContextPresent()) {
    return false;
  }

  if (delegate_->GLContextFBOResetAfterPresent()) {
    auto current_size =
        SkISize::Make(onscreen_surface_->width(), onscreen_surface_->height());

    // The FBO has changed, ask the delegate for the new FBO and do a surface
    // re-wrap.
    auto new_onscreen_surface =
        WrapOnscreenSurface(context_.get(),            // GL context
                            current_size,              // root surface size
                            delegate_->GLContextFBO()  // window FBO ID
        );

    if (!new_onscreen_surface) {
      return false;
    }

    onscreen_surface_ = std::move(new_onscreen_surface);
  }

  return true;
}

sk_sp<SkSurface> GPUSurfaceGL::AcquireRenderSurface(
    const SkISize& untransformed_size,
    const SkMatrix& root_surface_transformation) {
  const auto transformed_rect = root_surface_transformation.mapRect(
      SkRect::MakeWH(untransformed_size.width(), untransformed_size.height()));

  const auto transformed_size =
      SkISize::Make(transformed_rect.width(), transformed_rect.height());

  if (!CreateOrUpdateSurfaces(transformed_size)) {
    return nullptr;
  }

  return offscreen_surface_ != nullptr ? offscreen_surface_ : onscreen_surface_;
}

// |shell::Surface|
GrContext* GPUSurfaceGL::GetContext() {
  return context_.get();
}

}  // namespace shell
