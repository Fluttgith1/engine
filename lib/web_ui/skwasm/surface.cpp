// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>
#include <emscripten.h>
#include <emscripten/html5_webgl.h>
#include <emscripten/threading.h>
#include <webgl/webgl1.h>
#include "export.h"
#include "third_party/skia/include/core/SkCanvas.h"
#include "third_party/skia/include/core/SkColorSpace.h"
#include "third_party/skia/include/core/SkPicture.h"
#include "third_party/skia/include/core/SkSurface.h"
#include "third_party/skia/include/gpu/GrDirectContext.h"
#include "third_party/skia/include/gpu/gl/GrGLInterface.h"
#include "third_party/skia/include/gpu/gl/GrGLTypes.h"
#include "wrappers.h"

using namespace Skwasm;

using OnRenderCompleteCallback = void(uint32_t);

namespace {
class Surface;
void fDispose(Surface* surface);
void fSetCanvasSize(Surface* surface, int width, int height);
void fRenderPicture(Surface* surface, SkPicture* picture, uint32_t renderId);
void fOnRenderComplete(Surface* surface, uint32_t renderId);

class Surface {
 public:
  Surface(const char* canvasID) : _canvasID(canvasID) {
    pthread_attr_t attr;
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
    emscripten_pthread_attr_settransferredcanvases(&attr, _canvasID.c_str());

    pthread_create(
        &_thread, &attr,
        [](void* context) -> void* {
          static_cast<Surface*>(context)->_runWorker();
          return nullptr;
        },
        this);
  }

  void dispose() {
    emscripten_dispatch_to_thread(_thread, EM_FUNC_SIG_VI,
                                  reinterpret_cast<void*>(fDispose), nullptr,
                                  this);
  }

  void setCanvasSize(int width, int height) {
    emscripten_dispatch_to_thread(_thread, EM_FUNC_SIG_VIII,
                                  reinterpret_cast<void*>(fSetCanvasSize),
                                  nullptr, this, width, height);
  }

  uint32_t renderPicture(SkPicture* picture) {
    uint32_t renderId = ++_currentRenderId;
    picture->ref();
    emscripten_dispatch_to_thread(_thread, EM_FUNC_SIG_VIII,
                                  reinterpret_cast<void*>(fRenderPicture),
                                  nullptr, this, picture, renderId);
    return renderId;
  }

  void setOnRenderCallback(OnRenderCompleteCallback* callback) {
    _onRenderCompleteCallback = callback;
  }

 private:
  void _runWorker() {
    _init();
    emscripten_unwind_to_js_event_loop();
  }

  void _init() {
    EmscriptenWebGLContextAttributes attributes;
    emscripten_webgl_init_context_attributes(&attributes);

    attributes.alpha = true;
    attributes.depth = true;
    attributes.stencil = true;
    attributes.antialias = false;
    attributes.premultipliedAlpha = true;
    attributes.preserveDrawingBuffer = 0;
    attributes.powerPreference = EM_WEBGL_POWER_PREFERENCE_DEFAULT;
    attributes.failIfMajorPerformanceCaveat = false;
    attributes.enableExtensionsByDefault = true;
    attributes.explicitSwapControl = false;
    attributes.renderViaOffscreenBackBuffer = true;
    attributes.majorVersion = 2;

    _glContext =
        emscripten_webgl_create_context(_canvasID.c_str(), &attributes);
    if (!_glContext) {
      printf("Failed to create context!\n");
      return;
    }

    makeCurrent(_glContext);

    _grContext = GrDirectContext::MakeGL(GrGLMakeNativeInterface());

    // WebGL should already be clearing the color and stencil buffers, but do it
    // again here to ensure Skia receives them in the expected state.
    emscripten_glBindFramebuffer(GL_FRAMEBUFFER, 0);
    emscripten_glClearColor(0, 0, 0, 0);
    emscripten_glClearStencil(0);
    emscripten_glClear(GL_COLOR_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
    _grContext->resetContext(kRenderTarget_GrGLBackendState |
                             kMisc_GrGLBackendState);

    // The on-screen canvas is FBO 0. Wrap it in a Skia render target so Skia
    // can render to it.
    _fbInfo.fFBOID = 0;
    _fbInfo.fFormat = GL_RGBA8_OES;

    emscripten_glGetIntegerv(GL_SAMPLES, &_sampleCount);
    emscripten_glGetIntegerv(GL_STENCIL_BITS, &_stencil);
  }

  void _dispose() { delete this; }

  void _setCanvasSize(int width, int height) {
    if (_canvasWidth != width || _canvasHeight != height) {
      emscripten_set_canvas_element_size(_canvasID.c_str(), width, height);
      _canvasWidth = width;
      _canvasHeight = height;
      _recreateSurface();
    }
  }

  void _recreateSurface() {
    makeCurrent(_glContext);
    GrBackendRenderTarget target(_canvasWidth, _canvasHeight, _sampleCount,
                                 _stencil, _fbInfo);
    _surface = SkSurface::MakeFromBackendRenderTarget(
        _grContext.get(), target, kBottomLeft_GrSurfaceOrigin,
        kRGBA_8888_SkColorType, SkColorSpace::MakeSRGB(), nullptr);
  }

  void _renderPicture(const SkPicture* picture, uint32_t renderId) {
    if (!_surface) {
      printf("Can't render picture with no surface.\n");
      return;
    }

    makeCurrent(_glContext);
    auto canvas = _surface->getCanvas();
    canvas->drawPicture(picture);
    _surface->flush();

    emscripten_sync_run_in_main_runtime_thread(
        EM_FUNC_SIG_VII, fOnRenderComplete, this, renderId);
  }

  void _onRenderComplete(uint32_t renderId) {
    _onRenderCompleteCallback(renderId);
  }

  std::string _canvasID;
  OnRenderCompleteCallback* _onRenderCompleteCallback = nullptr;
  uint32_t _currentRenderId = 0;

  int _canvasWidth = 0;
  int _canvasHeight = 0;

  EMSCRIPTEN_WEBGL_CONTEXT_HANDLE _glContext = 0;
  sk_sp<GrDirectContext> _grContext = nullptr;
  sk_sp<SkSurface> _surface = nullptr;
  GrGLFramebufferInfo _fbInfo;
  GrGLint _sampleCount;
  GrGLint _stencil;

  pthread_t _thread;

  friend void fDispose(Surface* surface);
  friend void fSetCanvasSize(Surface* surface, int width, int height);
  friend void fRenderPicture(Surface* surface,
                             SkPicture* picture,
                             uint32_t renderId);
  friend void fOnRenderComplete(Surface* surface, uint32_t renderId);
};

void fDispose(Surface* surface) {
  surface->_dispose();
}

void fSetCanvasSize(Surface* surface, int width, int height) {
  surface->_setCanvasSize(width, height);
}

void fRenderPicture(Surface* surface, SkPicture* picture, uint32_t renderId) {
  surface->_renderPicture(picture, renderId);
  picture->unref();
}

void fOnRenderComplete(Surface* surface, uint32_t renderId) {
  surface->_onRenderComplete(renderId);
}
}  // namespace

SKWASM_EXPORT Surface* surface_createFromCanvas(const char* canvasID) {
  return new Surface(canvasID);
}

SKWASM_EXPORT void surface_setOnRenderCallback(
    Surface* surface,
    OnRenderCompleteCallback* callback) {
  surface->setOnRenderCallback(callback);
}

SKWASM_EXPORT void surface_destroy(Surface* surface) {
  surface->dispose();
}

SKWASM_EXPORT void surface_setCanvasSize(Surface* surface,
                                         int width,
                                         int height) {
  surface->setCanvasSize(width, height);
}

SKWASM_EXPORT uint32_t surface_renderPicture(Surface* surface,
                                             SkPicture* picture) {
  return surface->renderPicture(picture);
}
