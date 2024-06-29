// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import 'package:test/bootstrap/browser.dart';
import 'package:test/test.dart';
import 'package:ui/src/engine.dart';

void main() {
  internalBootstrapBrowserTest(() => testMain);
}

void testMain() {
  group('$ContextMenu', () {
    test('can disable context menu', () {
      final rootViewElement = createDomElement('div');
      final contextMenu = ContextMenu(rootViewElement);

      // When the app starts, contextmenu events are not prevented.
      var event = createDomEvent('Event', 'contextmenu');
      expect(event.defaultPrevented, isFalse);
      rootViewElement.dispatchEvent(event);
      expect(event.defaultPrevented, isFalse);

      // Disabling the context menu causes contextmenu events to be prevented.
      contextMenu.disable();
      event = createDomEvent('Event', 'contextmenu');
      expect(event.defaultPrevented, isFalse);
      rootViewElement.dispatchEvent(event);
      expect(event.defaultPrevented, isTrue);

      // Disabling again has no effect.
      contextMenu.disable();
      event = createDomEvent('Event', 'contextmenu');
      expect(event.defaultPrevented, isFalse);
      rootViewElement.dispatchEvent(event);
      expect(event.defaultPrevented, isTrue);
    });

    test('does not disable context menu outside root view element', () {
      final rootViewElement = createDomElement('div');
      final contextMenu = ContextMenu(rootViewElement);

      contextMenu.disable();

      // Dispatching on a DOM element outside of target's subtree has no effect.
      final event = createDomEvent('Event', 'contextmenu');
      expect(event.defaultPrevented, isFalse);
      domDocument.body!.dispatchEvent(event);
      expect(event.defaultPrevented, isFalse);
    });

    test('can enable context menu after disabling', () {
      final rootViewElement = createDomElement('div');
      final contextMenu = ContextMenu(rootViewElement);

      // When the app starts, contextmenu events are not prevented.
      var event = createDomEvent('Event', 'contextmenu');
      expect(event.defaultPrevented, isFalse);
      rootViewElement.dispatchEvent(event);
      expect(event.defaultPrevented, isFalse);

      // Disabling the context menu causes contextmenu events to be prevented.
      contextMenu.disable();
      event = createDomEvent('Event', 'contextmenu');
      expect(event.defaultPrevented, isFalse);
      rootViewElement.dispatchEvent(event);
      expect(event.defaultPrevented, isTrue);

      // Enabling the context menu means that contextmenu events are back to not
      // being prevented.
      contextMenu.enable();
      event = createDomEvent('Event', 'contextmenu');
      expect(event.defaultPrevented, isFalse);
      rootViewElement.dispatchEvent(event);
      expect(event.defaultPrevented, isFalse);

      // Enabling again has no effect.
      contextMenu.enable();
      event = createDomEvent('Event', 'contextmenu');
      expect(event.defaultPrevented, isFalse);
      rootViewElement.dispatchEvent(event);
      expect(event.defaultPrevented, isFalse);
    });

    test('enabling before disabling has no effect', () {
      final rootViewElement = createDomElement('div');
      final contextMenu = ContextMenu(rootViewElement);

      // When the app starts, contextmenu events are not prevented.
      var event = createDomEvent('Event', 'contextmenu');
      expect(event.defaultPrevented, isFalse);
      rootViewElement.dispatchEvent(event);
      expect(event.defaultPrevented, isFalse);

      // Enabling has no effect.
      contextMenu.enable();
      event = createDomEvent('Event', 'contextmenu');
      expect(event.defaultPrevented, isFalse);
      rootViewElement.dispatchEvent(event);
      expect(event.defaultPrevented, isFalse);
    });
  });
}
