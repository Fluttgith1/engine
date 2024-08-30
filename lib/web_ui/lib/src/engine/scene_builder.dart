// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import 'dart:typed_data';

import 'package:ui/src/engine.dart';
import 'package:ui/ui.dart' as ui;

// This file implements a SceneBuilder and Scene that works with any renderer
// implementation that provides:
//   * A `ui.Canvas` that conforms to `SceneCanvas`
//   * A `ui.Picture` that conforms to `ScenePicture`
//   * A `ui.ImageFilter` that conforms to `SceneImageFilter`
//
// These contain a few augmentations to the normal `dart:ui` API that provide
// additional sizing information that the scene builder uses to determine how
// these object might occlude one another.


class EngineScene implements ui.Scene {
  EngineScene(this.rootLayer);

  final EngineRootLayer rootLayer;

  // We keep a refcount here because this can be asynchronously rendered, so we
  // don't necessarily want to dispose immediately when the user calls dispose.
  // Instead, we need to stay alive until we're done rendering.
  int _refCount = 1;

  void beginRender() {
    assert(_refCount > 0);
    _refCount++;
  }

  void endRender() {
    _refCount--;
    _disposeIfNeeded();
  }

  @override
  void dispose() {
    _refCount--;
    _disposeIfNeeded();
  }

  void _disposeIfNeeded() {
    assert(_refCount >= 0);
    if (_refCount == 0) {
      rootLayer.dispose();
    }
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

    // Only rasterizes the pictures.
    for (final LayerSlice? slice in rootLayer.slices) {
      if (slice != null) {
        canvas.drawPicture(slice.picture);
      }
    }
    return recorder.endRecording().toImageSync(width, height);
  }
}

class OcclusionMap {
  void addRect(ui.Rect rect) {
    occlusionRects.add(rect);
  }

  bool overlaps(ui.Rect rect) {
    return occlusionRects.any((ui.Rect o) => o.overlaps(rect));
  }

  // TODO(jacksongardner): Make this actually smart
  List<ui.Rect> occlusionRects = <ui.Rect>[];
}

class SceneSlice {
  final OcclusionMap pictureOcclusionMap = OcclusionMap();
  final OcclusionMap platformViewOcclusionMap = OcclusionMap();
}

class EngineSceneBuilder implements ui.SceneBuilder {
  LayerBuilder currentBuilder = LayerBuilder.rootLayer();

  final List<SceneSlice> sceneSlices = <SceneSlice>[SceneSlice()];

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
    _placePicture(offset, picture as ScenePicture);
    currentBuilder.addDrawCommand(PictureDrawCommand(offset, picture));
  }

  void _placePicture(ui.Offset offset, ScenePicture picture) {
    final ui.Rect cullRect = picture.cullRect.shift(offset);
    currentBuilder.mapLocalToGlobal(cullRect);
    int sliceIndex = sceneSlices.length;
    while (sliceIndex > 0) {
      final SceneSlice sliceBelow = sceneSlices[sliceIndex - 1];
      if (sliceBelow.platformViewOcclusionMap.overlaps(cullRect)) {
        break;
      }
      sliceIndex--;
      if (sliceBelow.pictureOcclusionMap.overlaps(cullRect)) {
        break;
      }
    }
    if (sliceIndex == sceneSlices.length) {
      // Insert a new slice.
      sceneSlices.add(SceneSlice());
    }
    final SceneSlice slice = sceneSlices[sliceIndex];
    slice.pictureOcclusionMap.addRect(cullRect);
    currentBuilder.addPicture(
      offset,
      picture,
      sliceIndex: sliceIndex,
    );
  }

  @override
  void addPlatformView(
    int viewId, {
    ui.Offset offset = ui.Offset.zero,
    double width = 0.0,
    double height = 0.0
  }) {
    final ui.Rect platformViewRect = ui.Rect.fromLTWH(offset.dx, offset.dy, width, height);
    _placePlatformView(viewId, platformViewRect);
    currentBuilder.addDrawCommand(PlatformViewDrawCommand(viewId, platformViewRect));
  }

  void _placePlatformView(int viewId, ui.Rect rect) {
    final ui.Rect globalPlatformViewRect = currentBuilder.mapLocalToGlobal(rect);
    int sliceIndex = sceneSlices.length - 1;
    while (sliceIndex > 0) {
      final SceneSlice sliceBelow = sceneSlices[sliceIndex - 1];
      if (sliceBelow.platformViewOcclusionMap.overlaps(globalPlatformViewRect) ||
          sliceBelow.pictureOcclusionMap.overlaps(globalPlatformViewRect)) {
        break;
      }
      sliceIndex--;
    }
    if (sliceIndex == sceneSlices.length) {
      // Insert a new slice.
      sceneSlices.add(SceneSlice());
    }
    final SceneSlice slice = sceneSlices[sliceIndex];
    slice.platformViewOcclusionMap.addRect(globalPlatformViewRect);

    currentBuilder.addPlatformView(
      viewId,
      bounds: rect,
      sliceIndex: sliceIndex,
    );
  }

  @override
  void addRetained(ui.EngineLayer retainedLayer) {
    _placeRetainedLayer(retainedLayer as PictureEngineLayer);
    currentBuilder.addDrawCommand(RetainedLayerDrawCommand(retainedLayer));
  }

  void _placeRetainedLayer(PictureEngineLayer retainedLayer) {
    // TODO(jacksongardner): add optimized path
    for (final LayerDrawCommand command in retainedLayer.drawCommands) {
      switch (command) {
        case PictureDrawCommand(offset: final ui.Offset offset, picture: final ScenePicture picture):
          _placePicture(offset, picture);
        case PlatformViewDrawCommand(viewId: final int viewId, bounds: final ui.Rect bounds):
          // TODO(jacksongardner): we need to somehow deal with transformations here
          _placePlatformView(viewId, bounds);
        case RetainedLayerDrawCommand(layer: final PictureEngineLayer layer):
          _placeRetainedLayer(layer);
      }
    }
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
    // addTexture is not implemented on web.
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
      ClipPathOperation(path as ScenePath, clipBehavior),
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
      ImageFilterOperation(filter as SceneImageFilter, offset),
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
  void setProperties(
    double width,
    double height,
    double insetTop,
    double insetRight,
    double insetBottom,
    double insetLeft,
    bool focusable
  ) {
    // Not implemented on web
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
