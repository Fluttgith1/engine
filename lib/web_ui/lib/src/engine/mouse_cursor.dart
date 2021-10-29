// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import 'dart:convert';
import 'dart:typed_data';

import 'package:ui/ui.dart' as ui;

import 'dom_renderer.dart';

/// Provides mouse cursor bindings, such as the `flutter/mousecursor` channel.
class MouseCursor {
  /// Initializes the [MouseCursor] singleton.
  ///
  /// Use the [instance] getter to get the singleton after calling this method.
  static void initialize() {
    _instance ??= MouseCursor._();
  }

  /// The [MouseCursor] singleton.
  static MouseCursor? get instance => _instance;
  static MouseCursor? _instance;

  MouseCursor._();

  // Map from Flutter's kind values to CSS's cursor values.
  //
  // This map must be kept in sync with Flutter framework's
  // rendering/mouse_cursor.dart.
  static const Map<String, String> _kindToCssValueMap = <String, String>{
    'alias': 'alias',
    'allScroll': 'all-scroll',
    'basic': 'default',
    'cell': 'cell',
    'click': 'pointer',
    'contextMenu': 'context-menu',
    'copy': 'copy',
    'forbidden': 'not-allowed',
    'grab': 'grab',
    'grabbing': 'grabbing',
    'help': 'help',
    'move': 'move',
    'none': 'none',
    'noDrop': 'no-drop',
    'precise': 'crosshair',
    'progress': 'progress',
    'text': 'text',
    'resizeColumn': 'col-resize',
    'resizeDown': 's-resize',
    'resizeDownLeft': 'sw-resize',
    'resizeDownRight': 'se-resize',
    'resizeLeft': 'w-resize',
    'resizeLeftRight': 'ew-resize',
    'resizeRight': 'e-resize',
    'resizeRow': 'row-resize',
    'resizeUp': 'n-resize',
    'resizeUpDown': 'ns-resize',
    'resizeUpLeft': 'nw-resize',
    'resizeUpRight': 'ne-resize',
    'resizeUpLeftDownRight': 'nwse-resize',
    'resizeUpRightDownLeft': 'nesw-resize',
    'verticalText': 'vertical-text',
    'wait': 'wait',
    'zoomIn': 'zoom-in',
    'zoomOut': 'zoom-out',
  };
  static String _mapKindToCssValue(String? kind) {
    return _kindToCssValueMap[kind] ?? 'default';
  }

  void activateSystemCursor(String? kind) {
    DomRenderer.setElementStyle(
      domRenderer.glassPaneElement!,
      'cursor',
      _mapKindToCssValue(kind),
    );
  }

  Future<int> createImageCursor(List<int> data, int width, int height, int offsetX, int offsetY) async {
    final Uint8List byteArray = Uint8List.fromList(data);
    final ui.ImageDescriptor image = ui.ImageDescriptor.raw(
      await ui.ImmutableBuffer.fromUint8List(byteArray),
      width: width,
      height: height,
      pixelFormat: ui.PixelFormat.rgba8888,
    );
    final ui.Codec codec = await image.instantiateCodec();
    final ui.FrameInfo frame = await codec.getNextFrame();
    final ByteData pngData = (await frame.image.toByteData(format: ui.ImageByteFormat.png))!;
    final Uint8List pngBytes = Uint8List.sublistView(pngData);
    final String cursorString = 'data:image/png;base64,${base64.encode(pngBytes)}';
    _lastImageCursorId += 1;
    _imageMouseCursors[_lastImageCursorId] = cursorString;
    return _lastImageCursorId;
  }

  void activateImageCursor(int cursorId) {
    DomRenderer.setElementStyle(
      domRenderer.glassPaneElement!,
      'cursor',
      'url(${_imageMouseCursors[cursorId]!}),pointer',
    );
  }

  int _lastImageCursorId = 1;
  final Map<int, String> _imageMouseCursors = <int, String>{};
}
