// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
import 'dart:math' as math;

import 'package:ui/ui.dart' as ui;

import '../../engine.dart' show PlatformViewManager;
import '../display.dart';
import '../dom.dart';
import '../html/path_to_svg_clip.dart';
import '../platform_views/slots.dart';
import '../svg.dart';
import '../util.dart';
import '../vector_math.dart';
import 'canvas.dart';
import 'overlay_scene_optimizer.dart';
import 'path.dart';
import 'picture.dart';
import 'picture_recorder.dart';
import 'rasterizer.dart';

/// This composites HTML views into the [ui.Scene].
class HtmlViewEmbedder {
  HtmlViewEmbedder(this.sceneHost, this.rasterizer);

  final DomElement sceneHost;
  final ViewRasterizer rasterizer;

  /// The context for the current frame.
  EmbedderFrameContext _context = EmbedderFrameContext();

  /// The most recent composition parameters for a given view id.
  ///
  /// If we receive a request to composite a view, but the composition
  /// parameters haven't changed, we can avoid having to recompute the
  /// element stack that correctly composites the view into the scene.
  final Map<int, EmbeddedViewParams> _currentCompositionParams =
      <int, EmbeddedViewParams>{};

  /// The clip chain for a view Id.
  ///
  /// This contains:
  /// * The root view in the stack of mutator elements for the view id.
  /// * The slot view in the stack (what shows the actual platform view contents).
  /// * The number of clipping elements used last time the view was composited.
  final Map<int, ViewClipChain> _viewClipChains = <int, ViewClipChain>{};

  /// The maximum number of render canvases to create. Too many canvases can
  /// cause a performance burden.
  static const int maximumCanvases = 8;

  /// Canvases used to draw on top of platform views, keyed by platform view ID.
  final Map<int, DisplayCanvas> _overlays = <int, DisplayCanvas>{};

  /// The views that need to be recomposited into the scene on the next frame.
  final Set<int> _viewsToRecomposite = <int>{};

  /// The list of view ids that should be composited, in order.
  final List<int> _compositionOrder = <int>[];

  /// The most recent composition order.
  final List<int> _activeCompositionOrder = <int>[];

  /// The most recent overlay groups.
  List<OverlayGroup> _activeOverlayGroups = <OverlayGroup>[];

  /// The most recent rendering.
  Rendering _activeRendering = Rendering();

  DisplayCanvas? debugBoundsCanvas;

  /// The size of the frame, in physical pixels.
  late ui.Size _frameSize;

  set frameSize(ui.Size size) {
    _frameSize = size;
  }

  /// Returns a list of canvases which will be overlaid on top of the "base"
  /// canvas after a platform view is composited into the scene.
  ///
  /// The engine asks for the overlay canvases immediately before the paint
  /// phase, after the preroll phase. In the preroll phase we must be
  /// conservative and assume that every platform view which is prerolled is
  /// also composited, and therefore requires an overlay canvas. However, not
  /// every platform view which is prerolled ends up being composited (it may be
  /// clipped out and not actually drawn). This means that we may end up
  /// overallocating canvases. This isn't a problem in practice, however, as
  /// unused recording canvases are simply deleted at the end of the frame.
  Iterable<CkCanvas> getOverlayCanvases() {
    return _context.pictureRecordersCreatedDuringPreroll
        .map((CkPictureRecorder r) => r.recordingCanvas!);
  }

  void prerollCompositeEmbeddedView(int viewId, EmbeddedViewParams params) {
    final CkPictureRecorder pictureRecorder = CkPictureRecorder();
    pictureRecorder.beginRecording(ui.Offset.zero & _frameSize);
    _context.pictureRecordersCreatedDuringPreroll.add(pictureRecorder);

    // Do nothing if the params didn't change.
    if (_currentCompositionParams[viewId] == params) {
      // If the view was prerolled but not composited, then it needs to be
      // recomposited.
      if (!_activeCompositionOrder.contains(viewId)) {
        _viewsToRecomposite.add(viewId);
      }
      return;
    }
    _currentCompositionParams[viewId] = params;
    _viewsToRecomposite.add(viewId);
  }

  /// Prepares to composite [viewId].
  ///
  /// If this returns a [CkCanvas], then that canvas should be the new leaf
  /// node. Otherwise, keep the same leaf node.
  CkCanvas? compositeEmbeddedView(int viewId) {
    // Ensure platform view with `viewId` is injected into the `rasterizer.view`.
    rasterizer.view.dom.injectPlatformView(viewId);

    final int overlayIndex = _context.viewCount;
    _compositionOrder.add(viewId);
    _context.viewCount++;

    CkPictureRecorder? recorderToUseForRendering;
    if (overlayIndex < _context.pictureRecordersCreatedDuringPreroll.length) {
      recorderToUseForRendering =
          _context.pictureRecordersCreatedDuringPreroll[overlayIndex];
      _context.pictureRecorders.add(recorderToUseForRendering);
    }

    if (_viewsToRecomposite.contains(viewId)) {
      _compositeWithParams(viewId, _currentCompositionParams[viewId]!);
      _viewsToRecomposite.remove(viewId);
    }
    return recorderToUseForRendering?.recordingCanvas;
  }

  void _compositeWithParams(int platformViewId, EmbeddedViewParams params) {
    // If we haven't seen this viewId yet, cache it for clips/transforms.
    final ViewClipChain clipChain =
        _viewClipChains.putIfAbsent(platformViewId, () {
      return ViewClipChain(view: createPlatformViewSlot(platformViewId));
    });

    final DomElement slot = clipChain.slot;

    // See `apply()` in the PersistedPlatformView class for the HTML version
    // of this code.
    slot.style
      ..width = '${params.size.width}px'
      ..height = '${params.size.height}px'
      ..position = 'absolute';

    // Recompute the position in the DOM of the `slot` element...
    final int currentClippingCount = _countClips(params.mutators);
    final int previousClippingCount = clipChain.clipCount;
    if (currentClippingCount != previousClippingCount) {
      final DomElement oldPlatformViewRoot = clipChain.root;
      final DomElement newPlatformViewRoot = _reconstructClipViewsChain(
        currentClippingCount,
        slot,
        oldPlatformViewRoot,
      );
      // Store the updated root element, and clip count
      clipChain.updateClipChain(
        root: newPlatformViewRoot,
        clipCount: currentClippingCount,
      );
    }

    // Apply mutators to the slot
    _applyMutators(params, slot, platformViewId);
  }

  int _countClips(MutatorsStack mutators) {
    int clipCount = 0;
    for (final Mutator mutator in mutators) {
      if (mutator.isClipType) {
        clipCount++;
      }
    }
    return clipCount;
  }

  DomElement _reconstructClipViewsChain(
    int numClips,
    DomElement platformView,
    DomElement headClipView,
  ) {
    DomNode? headClipViewNextSibling;
    bool headClipViewWasAttached = false;
    if (headClipView.parentNode != null) {
      headClipViewWasAttached = true;
      headClipViewNextSibling = headClipView.nextSibling;
      headClipView.remove();
    }
    DomElement head = platformView;
    int clipIndex = 0;
    // Re-use as much existing clip views as needed.
    while (head != headClipView && clipIndex < numClips) {
      head = head.parent!;
      clipIndex++;
    }
    // If there weren't enough existing clip views, add more.
    while (clipIndex < numClips) {
      final DomElement clippingView = createDomElement('flt-clip');
      clippingView.append(head);
      head = clippingView;
      clipIndex++;
    }
    head.remove();

    // If the chain was previously attached, attach it to the same position.
    if (headClipViewWasAttached) {
      sceneHost.insertBefore(head, headClipViewNextSibling);
    }
    return head;
  }

  /// Clean up the old SVG clip definitions, as this platform view is about to
  /// be recomposited.
  void _cleanUpClipDefs(int viewId) {
    if (_svgClipDefs.containsKey(viewId)) {
      final DomElement clipDefs = _svgPathDefs!.querySelector('#sk_path_defs')!;
      final List<DomElement> nodesToRemove = <DomElement>[];
      final Set<String> oldDefs = _svgClipDefs[viewId]!;
      for (final DomElement child in clipDefs.children) {
        if (oldDefs.contains(child.id)) {
          nodesToRemove.add(child);
        }
      }
      for (final DomElement node in nodesToRemove) {
        node.remove();
      }
      _svgClipDefs[viewId]!.clear();
    }
  }

  void _applyMutators(
      EmbeddedViewParams params, DomElement embeddedView, int viewId) {
    final MutatorsStack mutators = params.mutators;
    DomElement head = embeddedView;
    Matrix4 headTransform = params.offset == ui.Offset.zero
        ? Matrix4.identity()
        : Matrix4.translationValues(params.offset.dx, params.offset.dy, 0);
    double embeddedOpacity = 1.0;
    _resetAnchor(head);
    _cleanUpClipDefs(viewId);

    for (final Mutator mutator in mutators) {
      switch (mutator.type) {
        case MutatorType.transform:
          headTransform = mutator.matrix!.multiplied(headTransform);
          head.style.transform =
              float64ListToCssTransform(headTransform.storage);
        case MutatorType.clipRect:
        case MutatorType.clipRRect:
        case MutatorType.clipPath:
          final DomElement clipView = head.parent!;
          clipView.style.clip = '';
          clipView.style.clipPath = '';
          headTransform = Matrix4.identity();
          clipView.style.transform = '';
          // We need to set width and height for the clipView to cover the
          // bounds of the path since Safari seem to incorrectly intersect
          // the element bounding rect with the clip path.
          clipView.style.width = '100%';
          clipView.style.height = '100%';
          if (mutator.rect != null) {
            final ui.Rect rect = mutator.rect!;
            clipView.style.clip = 'rect(${rect.top}px, ${rect.right}px, '
                '${rect.bottom}px, ${rect.left}px)';
          } else if (mutator.rrect != null) {
            final CkPath path = CkPath();
            path.addRRect(mutator.rrect!);
            _ensureSvgPathDefs();
            final DomElement pathDefs =
                _svgPathDefs!.querySelector('#sk_path_defs')!;
            _clipPathCount += 1;
            final String clipId = 'svgClip$_clipPathCount';
            final SVGClipPathElement newClipPath = createSVGClipPathElement();
            newClipPath.id = clipId;
            newClipPath.append(
                createSVGPathElement()..setAttribute('d', path.toSvgString()!));

            pathDefs.append(newClipPath);
            // Store the id of the node instead of [newClipPath] directly. For
            // some reason, calling `newClipPath.remove()` doesn't remove it
            // from the DOM.
            _svgClipDefs.putIfAbsent(viewId, () => <String>{}).add(clipId);
            clipView.style.clipPath = 'url(#$clipId)';
          } else if (mutator.path != null) {
            final CkPath path = mutator.path! as CkPath;
            _ensureSvgPathDefs();
            final DomElement pathDefs =
                _svgPathDefs!.querySelector('#sk_path_defs')!;
            _clipPathCount += 1;
            final String clipId = 'svgClip$_clipPathCount';
            final SVGClipPathElement newClipPath = createSVGClipPathElement();
            newClipPath.id = clipId;
            newClipPath.append(
                createSVGPathElement()..setAttribute('d', path.toSvgString()!));
            pathDefs.append(newClipPath);
            // Store the id of the node instead of [newClipPath] directly. For
            // some reason, calling `newClipPath.remove()` doesn't remove it
            // from the DOM.
            _svgClipDefs.putIfAbsent(viewId, () => <String>{}).add(clipId);
            clipView.style.clipPath = 'url(#$clipId)';
          }
          _resetAnchor(clipView);
          head = clipView;
        case MutatorType.opacity:
          embeddedOpacity *= mutator.alphaFloat;
      }
    }

    embeddedView.style.opacity = embeddedOpacity.toString();

    // Reverse scale based on screen scale.
    //
    // HTML elements use logical (CSS) pixels, but we have been using physical
    // pixels, so scale down the head element to match the logical resolution.
    final double scale = EngineFlutterDisplay.instance.devicePixelRatio;
    final double inverseScale = 1 / scale;
    final Matrix4 scaleMatrix =
        Matrix4.diagonal3Values(inverseScale, inverseScale, 1);
    headTransform = scaleMatrix.multiplied(headTransform);
    head.style.transform = float64ListToCssTransform(headTransform.storage);
  }

  /// Sets the transform origin to the top-left corner of the element.
  ///
  /// By default, the transform origin is the center of the element, but
  /// Flutter assumes the transform origin is the top-left point.
  void _resetAnchor(DomElement element) {
    element.style.transformOrigin = '0 0 0';
    element.style.position = 'absolute';
  }

  int _clipPathCount = 0;

  DomElement? _svgPathDefs;

  /// The nodes containing the SVG clip definitions needed to clip this view.
  final Map<int, Set<String>> _svgClipDefs = <int, Set<String>>{};

  /// Ensures we add a container of SVG path defs to the DOM so they can
  /// be referred to in clip-path: url(#blah).
  void _ensureSvgPathDefs() {
    if (_svgPathDefs != null) {
      return;
    }
    _svgPathDefs = kSvgResourceHeader.cloneNode(false) as SVGElement;
    _svgPathDefs!.append(createSVGDefsElement()..id = 'sk_path_defs');
    sceneHost.append(_svgPathDefs!);
  }

  Future<void> submitFrame(CkPicture basePicture) async {
    final List<CkPicture> pictures = <CkPicture>[basePicture];
    for (final CkPictureRecorder recorder in _context.pictureRecorders) {
      pictures.add(recorder.endRecording());
    }
    Rendering rendering = createOptimizedRendering(
        pictures, _compositionOrder, _currentCompositionParams);
    rendering = _modifyRenderingForMaxCanvases(rendering);
    final List<OverlayGroup>? overlayGroups = _updateOverlays(rendering);
    _activeRendering = rendering;
    if (overlayGroups != null) {
      _activeOverlayGroups = overlayGroups;
    }
    print('RENDERING: ${rendering.entities}');
    assert(
      _context.pictureRecorders.length >= _overlays.length,
      'There should be at least as many picture recorders '
      '(${_context.pictureRecorders.length}) as overlays (${_overlays.length}).',
    );

    final List<RenderingRenderCanvas> renderCanvases = rendering.canvases;
    int renderCanvasIndex = 0;

    for (final OverlayGroup overlayGroup in _activeOverlayGroups) {
      if (overlayGroup.isEmpty) {
        // This is a canvas that comes before any platform views.
        final DisplayCanvas baseCanvas = rasterizer.displayFactory.baseCanvas;
        sceneHost.prepend(baseCanvas.hostElement);
        final List<CkPicture> pictures =
            renderCanvases[renderCanvasIndex].pictures;
        renderCanvasIndex++;
        await rasterizer.rasterizeToCanvas(baseCanvas, pictures);
        continue;
      }
      final DisplayCanvas overlay = _overlays[overlayGroup.last]!;
      final List<CkPicture> pictures =
          renderCanvases[renderCanvasIndex].pictures;
      renderCanvasIndex++;
      await rasterizer.rasterizeToCanvas(overlay, pictures);
    }
    for (final CkPictureRecorder recorder
        in _context.pictureRecordersCreatedDuringPreroll) {
      if (recorder.isRecording) {
        recorder.endRecording();
      }
    }
    // Reset the context.
    _context = EmbedderFrameContext();
    if (listEquals(_compositionOrder, _activeCompositionOrder)) {
      _compositionOrder.clear();
      return;
    }

    final Set<int> unusedViews = Set<int>.from(_activeCompositionOrder);
    _activeCompositionOrder.clear();

    List<int>? debugInvalidViewIds;

    rasterizer.removeOverlaysFromDom();
    for (int i = 0; i < _compositionOrder.length; i++) {
      final int viewId = _compositionOrder[i];

      bool isViewInvalid = false;
      assert(() {
        isViewInvalid = !PlatformViewManager.instance.knowsViewId(viewId);
        if (isViewInvalid) {
          debugInvalidViewIds ??= <int>[];
          debugInvalidViewIds!.add(viewId);
        }
        return true;
      }());
      if (isViewInvalid) {
        continue;
      }

      final DomElement platformViewRoot = _viewClipChains[viewId]!.root;
      final DisplayCanvas? overlay = _overlays[viewId];
      sceneHost.append(platformViewRoot);
      if (overlay != null) {
        sceneHost.append(overlay.hostElement);
      }
      _activeCompositionOrder.add(viewId);
      unusedViews.remove(viewId);
    }

    _compositionOrder.clear();

    disposeViews(unusedViews);

    assert(
      debugInvalidViewIds == null || debugInvalidViewIds!.isEmpty,
      'Cannot render platform views: ${debugInvalidViewIds!.join(', ')}. '
      'These views have not been created, or they have been deleted.',
    );
  }

  void disposeViews(Iterable<int> viewsToDispose) {
    for (final int viewId in viewsToDispose) {
      // Remove viewId from the _viewClipChains Map, and then from the DOM.
      final ViewClipChain? clipChain = _viewClipChains.remove(viewId);
      clipChain?.root.remove();
      // More cleanup
      _currentCompositionParams.remove(viewId);
      _viewsToRecomposite.remove(viewId);
      _cleanUpClipDefs(viewId);
      _svgClipDefs.remove(viewId);
    }
  }

  void _releaseOverlay(int viewId) {
    if (_overlays[viewId] != null) {
      final DisplayCanvas overlay = _overlays[viewId]!;
      rasterizer.releaseOverlay(overlay);
      _overlays.remove(viewId);
    }
  }

  /// Modify the given rendering by removing canvases until the number of
  /// canvases is less than or equal to the maximum number of canvases.
  Rendering _modifyRenderingForMaxCanvases(Rendering rendering) {
    final Rendering result = Rendering();
    final int numCanvases = rendering.canvases.length;
    if (numCanvases <= maximumCanvases) {
      return rendering;
    }
    int numCanvasesToDelete = numCanvases - maximumCanvases;
    final List<CkPicture> picturesForLastCanvas = <CkPicture>[];
    final List<RenderingEntity> modifiedEntities =
        List<RenderingEntity>.from(rendering.entities);
    bool sawLastCanvas = false;
    for (int i = rendering.entities.length - 1; i > 0; i--) {
      final RenderingEntity entity = modifiedEntities[i];
      if (entity is RenderingRenderCanvas) {
        if (!sawLastCanvas) {
          sawLastCanvas = true;
          picturesForLastCanvas.insertAll(0, entity.pictures);
          continue;
        }
        modifiedEntities.removeAt(i);
        picturesForLastCanvas.insertAll(0, entity.pictures);
        numCanvasesToDelete--;
        if (numCanvasesToDelete == 0) {
          break;
        }
      }
    }
    // Replace the pictures in the last canvas with all the pictures from the
    // deleted canvases.
    for (int i = modifiedEntities.length - 1; i > 0; i--) {
      final RenderingEntity entity = modifiedEntities[i];
      if (entity is RenderingRenderCanvas) {
        entity.pictures.clear();
        entity.pictures.addAll(picturesForLastCanvas);
        break;
      }
    }

    result.entities.addAll(modifiedEntities);
    return result;
  }

  // Assigns overlays to the embedded views in the scene.
  //
  // This method attempts to be efficient by taking advantage of the
  // [diffResult] and trying to re-use overlays which have already been
  // assigned.
  //
  // This method accounts for invisible platform views by grouping them
  // with the last visible platform view which precedes it. All invisible
  // platform views that come after a visible view share the same overlay
  // as the preceding visible view.
  //
  // This is called right before compositing the scene.
  //
  // [_compositionOrder] and [_activeComposition] order should contain the
  // composition order of the current and previous frame, respectively.
  //
  // TODO(hterkelsen): Test this more thoroughly.
  List<OverlayGroup>? _updateOverlays(Rendering rendering) {
    if (rendering.equalsForRendering(_activeRendering)) {
      // The rendering has not changed, continue using the assigned overlays.
      return null;
    }
    final List<int> indexMap =
        _getIndexMapFromPreviousRendering(_activeRendering, rendering);
    print('index map: $indexMap');
    // Group platform views from their composition order.
    // Each group contains one visible view, and any number of invisible views
    // before or after that visible view.
    final List<OverlayGroup> overlayGroups = getOverlayGroups(rendering);
    final List<int> viewsNeedingOverlays = overlayGroups
        .where((OverlayGroup group) => !group.isEmpty)
        .map((OverlayGroup group) => group.last)
        .toList();

    // Everything is going to be explicitly recomposited anyway. Release all
    // the surfaces and assign an overlay to all the surfaces needing one.
    rasterizer.releaseOverlays();
    _overlays.clear();

    viewsNeedingOverlays.forEach(_initializeOverlay);
    assert(_overlays.length == viewsNeedingOverlays.length);
    return overlayGroups;
  }

  List<int> _getIndexMapFromPreviousRendering(
      Rendering previous, Rendering next) {
    assert(!previous.equalsForRendering(next),
        'Should not be in this method if the Renderings are equal');
    final List<int> result = <int>[];
    int index = 0;

    final int maxLength =
        math.min(previous.entities.length, next.entities.length);

    // A canvas in the previous rendering can only be used once in the next
    // rendering. So if it is matched with one in the next rendering, mark it
    // here so it is only matched once.
    final Set<int> alreadyClaimedCanvases = <int>{};

    // Add the unchanged elements from the beginning of the list.
    while (index < maxLength &&
        previous.entities[index].equalsForRendering(next.entities[index])) {
      result.add(index);
      if (previous.entities[index] is RenderingRenderCanvas) {
        alreadyClaimedCanvases.add(index);
      }
      index += 1;
    }

    while (index < next.entities.length) {
      for (int oldIndex = 0;
          oldIndex < previous.entities.length;
          oldIndex += 1) {
        if (previous.entities[oldIndex]
                .equalsForRendering(next.entities[index]) &&
            !alreadyClaimedCanvases.contains(oldIndex)) {
          result.add(oldIndex);
          if (previous.entities[oldIndex] is RenderingRenderCanvas) {
            alreadyClaimedCanvases.add(oldIndex);
          }
          break;
        }
      }
      index += 1;
    }

    return result;
  }

  // Group the platform views into "overlay groups". These are sublists
  // of the composition order which can share the same overlay. Every overlay
  // group is a list containing a visible view followed by zero or more
  // invisible views.
  //
  // If there are more visible views than overlays, then the views which cannot
  // be assigned an overlay are grouped together and will be rendered on top of
  // the rest of the scene.
  List<OverlayGroup> getOverlayGroups(Rendering rendering) {
    final List<OverlayGroup> result = <OverlayGroup>[];
    OverlayGroup currentGroup = OverlayGroup();

    for (final RenderingEntity entity in rendering.entities) {
      /// We are at an overlay, end the current group and start the next group.
      if (entity is RenderingRenderCanvas) {
        result.add(currentGroup);
        currentGroup = OverlayGroup();
      } else if (entity is RenderingPlatformView) {
        currentGroup.add(entity.viewId);
      }
    }
    assert(_overlayGroupsAreValid(result));
    return result;
  }

  bool _overlayGroupsAreValid(List<OverlayGroup> overlayGroups) {
    // Overlay groups are valid if they are all non-empty except for (possibly)
    // the first one.
    for (int i = 0; i < overlayGroups.length; i++) {
      final OverlayGroup overlayGroup = overlayGroups[i];
      if (overlayGroup.isEmpty && i != 0) {
        return false;
      }
    }
    return true;
  }

  void _initializeOverlay(int viewId) {
    assert(!_overlays.containsKey(viewId));

    // Try reusing a cached overlay created for another platform view.
    final DisplayCanvas overlay = rasterizer.getOverlay();
    _overlays[viewId] = overlay;
  }

  /// Deletes SVG clip paths, useful for tests.
  void debugCleanupSvgClipPaths() {
    final DomElement? parent = _svgPathDefs?.children.single;
    if (parent != null) {
      for (DomNode? child = parent.lastChild;
          child != null;
          child = parent.lastChild) {
        parent.removeChild(child);
      }
    }
    _svgClipDefs.clear();
  }

  static void removeElement(DomElement element) {
    element.remove();
  }

  /// Disposes the state of this view embedder.
  void dispose() {
    disposeViews(_viewClipChains.keys.toList());
    _context = EmbedderFrameContext();
    _currentCompositionParams.clear();
    debugCleanupSvgClipPaths();
    _currentCompositionParams.clear();
    _viewClipChains.clear();
    _overlays.clear();
    _viewsToRecomposite.clear();
    _activeCompositionOrder.clear();
    _compositionOrder.clear();
    _activeRendering = Rendering();
  }

  /// Clears the state. Used in tests.
  void debugClear() {
    dispose();
    rasterizer.removeOverlaysFromDom();
  }
}

/// A group of views that will be composited together within the same overlay.
///
/// Each OverlayGroup is a sublist of the composition order which can share the
/// same overlay.
///
/// Every overlay group is a list containing a visible view preceded or followed
/// by zero or more invisible views.
class OverlayGroup {
  OverlayGroup() : _group = <int>[];

  // The internal list of ints.
  final List<int> _group;

  /// The number of visible views in this group.
  int _visibleCount = 0;

  /// Add a [view] (maybe [visible]) to this group.
  void add(int view, {bool visible = false}) {
    _group.add(view);
    if (visible) {
      _visibleCount++;
    }
  }

  /// Get the "last" view added to this group.
  int get last => _group.last;

  /// Returns true if this group contains any visible view.
  bool get hasVisibleView => _visibleCount > 0;

  /// Returns the number of visible views in this overlay group.
  int get visibleCount => _visibleCount;

  /// Returns [true] if this overlay group is empty.
  bool get isEmpty => _group.isEmpty;
}

/// Represents a Clip Chain (for a view).
///
/// Objects of this class contain:
/// * The root view in the stack of mutator elements for the view id.
/// * The slot view in the stack (the actual contents of the platform view).
/// * The number of clipping elements used last time the view was composited.
class ViewClipChain {
  ViewClipChain({required DomElement view})
      : _root = view,
        _slot = view;

  DomElement _root;
  final DomElement _slot;
  int _clipCount = -1;

  DomElement get root => _root;
  DomElement get slot => _slot;
  int get clipCount => _clipCount;

  void updateClipChain({required DomElement root, required int clipCount}) {
    _root = root;
    _clipCount = clipCount;
  }
}

/// The parameters passed to the view embedder.
class EmbeddedViewParams {
  EmbeddedViewParams(this.offset, this.size, MutatorsStack mutators)
      : mutators = MutatorsStack._copy(mutators);

  final ui.Offset offset;
  final ui.Size size;
  final MutatorsStack mutators;

  @override
  bool operator ==(Object other) {
    if (identical(this, other)) {
      return true;
    }
    return other is EmbeddedViewParams &&
        other.offset == offset &&
        other.size == size &&
        other.mutators == mutators;
  }

  @override
  int get hashCode => Object.hash(offset, size, mutators);
}

enum MutatorType {
  clipRect,
  clipRRect,
  clipPath,
  transform,
  opacity,
}

/// Stores mutation information like clipping or transform.
class Mutator {
  const Mutator.clipRect(ui.Rect rect)
      : this._(MutatorType.clipRect, rect, null, null, null, null);
  const Mutator.clipRRect(ui.RRect rrect)
      : this._(MutatorType.clipRRect, null, rrect, null, null, null);
  const Mutator.clipPath(ui.Path path)
      : this._(MutatorType.clipPath, null, null, path, null, null);
  const Mutator.transform(Matrix4 matrix)
      : this._(MutatorType.transform, null, null, null, matrix, null);
  const Mutator.opacity(int alpha)
      : this._(MutatorType.opacity, null, null, null, null, alpha);

  const Mutator._(
    this.type,
    this.rect,
    this.rrect,
    this.path,
    this.matrix,
    this.alpha,
  );

  final MutatorType type;
  final ui.Rect? rect;
  final ui.RRect? rrect;
  final ui.Path? path;
  final Matrix4? matrix;
  final int? alpha;

  bool get isClipType =>
      type == MutatorType.clipRect ||
      type == MutatorType.clipRRect ||
      type == MutatorType.clipPath;

  double get alphaFloat => alpha! / 255.0;

  @override
  bool operator ==(Object other) {
    if (identical(this, other)) {
      return true;
    }
    if (other is! Mutator) {
      return false;
    }

    final Mutator typedOther = other;
    if (type != typedOther.type) {
      return false;
    }

    switch (type) {
      case MutatorType.clipRect:
        return rect == typedOther.rect;
      case MutatorType.clipRRect:
        return rrect == typedOther.rrect;
      case MutatorType.clipPath:
        return path == typedOther.path;
      case MutatorType.transform:
        return matrix == typedOther.matrix;
      case MutatorType.opacity:
        return alpha == typedOther.alpha;
      default:
        return false;
    }
  }

  @override
  int get hashCode => Object.hash(type, rect, rrect, path, matrix, alpha);
}

/// A stack of mutators that can be applied to an embedded view.
class MutatorsStack extends Iterable<Mutator> {
  MutatorsStack() : _mutators = <Mutator>[];

  MutatorsStack._copy(MutatorsStack original)
      : _mutators = List<Mutator>.from(original._mutators);

  final List<Mutator> _mutators;

  void pushClipRect(ui.Rect rect) {
    _mutators.add(Mutator.clipRect(rect));
  }

  void pushClipRRect(ui.RRect rrect) {
    _mutators.add(Mutator.clipRRect(rrect));
  }

  void pushClipPath(ui.Path path) {
    _mutators.add(Mutator.clipPath(path));
  }

  void pushTransform(Matrix4 matrix) {
    _mutators.add(Mutator.transform(matrix));
  }

  void pushOpacity(int alpha) {
    _mutators.add(Mutator.opacity(alpha));
  }

  void pop() {
    _mutators.removeLast();
  }

  @override
  bool operator ==(Object other) {
    if (identical(other, this)) {
      return true;
    }
    return other is MutatorsStack &&
        listEquals<Mutator>(other._mutators, _mutators);
  }

  @override
  int get hashCode => Object.hashAll(_mutators);

  @override
  Iterator<Mutator> get iterator => _mutators.reversed.iterator;
}

/// The state for the current frame.
class EmbedderFrameContext {
  /// Picture recorders which were created during the preroll phase.
  ///
  /// These picture recorders will be "claimed" in the paint phase by platform
  /// views being composited into the scene.
  final List<CkPictureRecorder> pictureRecordersCreatedDuringPreroll =
      <CkPictureRecorder>[];

  /// Picture recorders which were actually used in the paint phase.
  ///
  /// This is a subset of [_pictureRecordersCreatedDuringPreroll].
  final List<CkPictureRecorder> pictureRecorders = <CkPictureRecorder>[];

  /// The number of platform views in this frame.
  int viewCount = 0;
}
