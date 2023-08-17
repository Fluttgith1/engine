// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import 'dart:typed_data';

import 'package:ui/src/engine.dart';
import 'package:ui/ui.dart' as ui;

class EngineScene implements ui.Scene {
  EngineScene(this.rootLayer);

  final EngineRootLayer rootLayer;

  bool _disposeCalled = false;
  bool _isRendering = false;
  bool _isDisposed = false;

  set isRendering(bool isRendering) {
    if (_isRendering != isRendering) {
      _isRendering = isRendering;
      _updateDispose();
    }
  }

  @override
  void dispose() {
    _disposeCalled = true;
    _updateDispose();
  }

  void _updateDispose() {
    if (_isDisposed || _isRendering || !_disposeCalled) {
      return;
    }
    rootLayer.dispose();
    _isDisposed = true;
  }

  @override
  Future<ui.Image> toImage(int width, int height) async {
    return toImageSync(width, height);
  }

  @override
  ui.Image toImageSync(int width, int height) {
    final ui.PictureRecorder recorder = ui.PictureRecorder();
    final ui.Rect canvasRect = ui.Rect.fromLTWH(0, 0, width.toDouble(), height.toDouble());
    final ui.Canvas canvas = ui.Canvas(recorder, canvasRect);

    // Only rasterizes the picture slices.
    for (final PictureSlice slice in rootLayer.slices.whereType<PictureSlice>()) {
      canvas.drawPicture(slice.picture);
    }
    return recorder.endRecording().toImageSync(width, height);
  }
}

class EngineSceneBuilder implements ui.SceneBuilder {
  LayerBuilder currentBuilder = LayerBuilder.rootLayer();

  @override
  void addPerformanceOverlay(int enabledOptions, ui.Rect bounds) {
    // We don't plan to implement this on the web.
    throw UnimplementedError();
  }

  @override
  void addPicture(
    ui.Offset offset,
    ui.Picture picture, {
    bool isComplexHint = false,
    bool willChangeHint = false
  }) {
    currentBuilder.addPicture(
      offset,
      picture,
      isComplexHint:
      isComplexHint,
      willChangeHint: willChangeHint
    );
  }

  @override
  void addPlatformView(
    int viewId, {
    ui.Offset offset = ui.Offset.zero,
    double width = 0.0,
    double height = 0.0
  }) {
    currentBuilder.addPlatformView(
      viewId,
      offset: offset,
      width: width,
      height: height
    );
  }

  @override
  void addRetained(ui.EngineLayer retainedLayer) {
    currentBuilder.mergeLayer(retainedLayer as PictureEngineLayer);
  }

  @override
  void addTexture(
    int textureId, {
    ui.Offset offset = ui.Offset.zero,
    double width = 0.0,
    double height = 0.0,
    bool freeze = false,
    ui.FilterQuality filterQuality = ui.FilterQuality.low
  }) {
    // TODO(jacksongardner): implement addTexture
  }

  @override
  ui.BackdropFilterEngineLayer pushBackdropFilter(
    ui.ImageFilter filter, {
    ui.BlendMode blendMode = ui.BlendMode.srcOver,
    ui.BackdropFilterEngineLayer? oldLayer
  }) => pushLayer<BackdropFilterLayer>(
      BackdropFilterLayer(),
      BackdropFilterOperation(filter, blendMode),
    );

  @override
  ui.ClipPathEngineLayer pushClipPath(
    ui.Path path, {
    ui.Clip clipBehavior = ui.Clip.antiAlias,
    ui.ClipPathEngineLayer? oldLayer
  }) => pushLayer<ClipPathLayer>(
      ClipPathLayer(),
      ClipPathOperation(path, clipBehavior),
    );

  @override
  ui.ClipRRectEngineLayer pushClipRRect(
    ui.RRect rrect, {
    required ui.Clip clipBehavior,
    ui.ClipRRectEngineLayer? oldLayer
  }) => pushLayer<ClipRRectLayer>(
      ClipRRectLayer(),
      ClipRRectOperation(rrect, clipBehavior)
    );

  @override
  ui.ClipRectEngineLayer pushClipRect(
    ui.Rect rect, {
    ui.Clip clipBehavior = ui.Clip.antiAlias,
    ui.ClipRectEngineLayer? oldLayer
  }) {
    return pushLayer<ClipRectLayer>(
      ClipRectLayer(),
      ClipRectOperation(rect, clipBehavior)
    );
  }

  @override
  ui.ColorFilterEngineLayer pushColorFilter(
    ui.ColorFilter filter, {
    ui.ColorFilterEngineLayer? oldLayer
  }) => pushLayer<ColorFilterLayer>(
      ColorFilterLayer(),
      ColorFilterOperation(filter),
    );

  @override
  ui.ImageFilterEngineLayer pushImageFilter(
    ui.ImageFilter filter, {
    ui.Offset offset = ui.Offset.zero,
    ui.ImageFilterEngineLayer? oldLayer
  }) => pushLayer<ImageFilterLayer>(
      ImageFilterLayer(),
      ImageFilterOperation(filter, offset),
    );

  @override
  ui.OffsetEngineLayer pushOffset(
    double dx,
    double dy, {
    ui.OffsetEngineLayer? oldLayer
  }) => pushLayer<OffsetLayer>(
      OffsetLayer(),
      OffsetOperation(dx, dy)
    );

  @override
  ui.OpacityEngineLayer pushOpacity(int alpha, {
    ui.Offset offset = ui.Offset.zero,
    ui.OpacityEngineLayer? oldLayer
  }) => pushLayer<OpacityLayer>(
      OpacityLayer(),
      OpacityOperation(alpha, offset),
    );
  @override
  ui.ShaderMaskEngineLayer pushShaderMask(
    ui.Shader shader,
    ui.Rect maskRect,
    ui.BlendMode blendMode, {
    ui.ShaderMaskEngineLayer? oldLayer,
    ui.FilterQuality filterQuality = ui.FilterQuality.low
  }) => pushLayer<ShaderMaskLayer>(
      ShaderMaskLayer(),
      ShaderMaskOperation(shader, maskRect, blendMode)
    );

  @override
  ui.TransformEngineLayer pushTransform(
    Float64List matrix4, {
    ui.TransformEngineLayer? oldLayer
  }) => pushLayer<TransformLayer>(
      TransformLayer(),
      TransformOperation(matrix4),
    );

  @override
  void setCheckerboardOffscreenLayers(bool checkerboard) {
  }

  @override
  void setCheckerboardRasterCacheImages(bool checkerboard) {
  }

  @override
  void setProperties(
    double width,
    double height,
    double insetTop,
    double insetRight,
    double insetBottom,
    double insetLeft,
    bool focusable
  ) {
  }

  @override
  void setRasterizerTracingThreshold(int frameInterval) {
  }

  @override
  ui.Scene build() {
    while (currentBuilder.parent != null) {
      pop();
    }
    final PictureEngineLayer rootLayer = currentBuilder.build();
    return EngineScene(rootLayer as EngineRootLayer);
  }

  @override
  void pop() {
    final PictureEngineLayer layer = currentBuilder.build();
    final LayerBuilder? parentBuilder = currentBuilder.parent;
    if (parentBuilder == null) {
      throw StateError('Popped too many times.');
    }
    currentBuilder = parentBuilder;
    currentBuilder.mergeLayer(layer);
  }

  T pushLayer<T extends PictureEngineLayer>(T layer, LayerOperation operation) {
    currentBuilder = LayerBuilder.childLayer(
      parent: currentBuilder,
      layer: layer,
      operation: operation
    );
    return layer;
  }
}
