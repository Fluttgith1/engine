// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import 'dart:async';

import 'package:test/bootstrap/browser.dart';
import 'package:test/test.dart';
import 'package:ui/src/engine.dart';
import 'package:ui/ui_web/src/ui_web.dart' as ui_web;

import '../common/test_initialization.dart';

DomElement get defaultTextEditingRoot =>
    EnginePlatformDispatcher.instance.implicitView!.dom.textEditingHost;

void main() {
  internalBootstrapBrowserTest(() => testMain);
}

class _MockWithCompositionAwareMixin with CompositionAwareMixin {
  // These variables should be equal to their counterparts in CompositionAwareMixin.
  // Separate so the counterparts in CompositionAwareMixin can be private.
  static const String _kCompositionUpdate = 'compositionupdate';
  static const String _kCompositionStart = 'compositionstart';
  static const String _kCompositionEnd = 'compositionend';
}

DomHTMLInputElement get _inputElement {
  return defaultTextEditingRoot.querySelectorAll('input').first as DomHTMLInputElement;
}

GloballyPositionedTextEditingStrategy _enableEditingStrategy({
    required bool deltaModel,
    void Function(EditingState?, TextEditingDeltaState?)? onChange,
  }) {
  final owner = HybridTextEditing();

  owner.configuration = InputConfiguration(
    viewId: kImplicitViewId,
    enableDeltaModel: deltaModel,
  );

  final editingStrategy =
      GloballyPositionedTextEditingStrategy(owner);

  owner.debugTextEditingStrategyOverride = editingStrategy;

  editingStrategy.enable(owner.configuration!, onChange: onChange ?? (_, __) {}, onAction: (_) {});
  return editingStrategy;
}

Future<void> testMain() async {
  await bootstrapAndRunApp(withImplicitView: true);

  const fakeComposingText = 'ImComposingText';

  group('$CompositionAwareMixin', () {
    late TextEditingStrategy editingStrategy;

    setUp(() {
      editingStrategy = _enableEditingStrategy(deltaModel: false);
    });

    tearDown(() {
      editingStrategy.disable();
    });

    group('composition end', () {
      test('should reset composing text on handle composition end', () {
        final mockWithCompositionAwareMixin =
            _MockWithCompositionAwareMixin();
        mockWithCompositionAwareMixin.composingText = fakeComposingText;
        mockWithCompositionAwareMixin.addCompositionEventHandlers(_inputElement);

        _inputElement.dispatchEvent(createDomEvent('Event', _MockWithCompositionAwareMixin._kCompositionEnd));

        expect(mockWithCompositionAwareMixin.composingText, null);
      });
    });

    group('composition start', () {
      test('should reset composing text on handle composition start', () {
        final mockWithCompositionAwareMixin =
            _MockWithCompositionAwareMixin();
        mockWithCompositionAwareMixin.composingText = fakeComposingText;
        mockWithCompositionAwareMixin.addCompositionEventHandlers(_inputElement);

        _inputElement.dispatchEvent(createDomEvent('Event', _MockWithCompositionAwareMixin._kCompositionStart));

        expect(mockWithCompositionAwareMixin.composingText, null);
      });
    });

    group('composition update', () {
      test('should set composing text to event composing text', () {
        const fakeEventText = 'IAmComposingThis';
        final mockWithCompositionAwareMixin =
            _MockWithCompositionAwareMixin();
        mockWithCompositionAwareMixin.composingText = fakeComposingText;
        mockWithCompositionAwareMixin.addCompositionEventHandlers(_inputElement);

        _inputElement.dispatchEvent(createDomCompositionEvent(
            _MockWithCompositionAwareMixin._kCompositionUpdate,
            <Object?, Object?>{ 'data': fakeEventText }
        ));

        expect(mockWithCompositionAwareMixin.composingText, fakeEventText);
      });
    });

    group('determine composition state', () {
      test('should return editing state if extentOffset is null', () {
        final editingState = EditingState(text: 'Test');

        final mockWithCompositionAwareMixin =
            _MockWithCompositionAwareMixin();
        mockWithCompositionAwareMixin.composingText = 'Test';

        expect(
          mockWithCompositionAwareMixin.determineCompositionState(editingState),
          editingState,
        );
      });

      test('should return editing state if composingText is null', () {
        final editingState = EditingState(
          text: 'Test',
          baseOffset: 0,
          extentOffset: 4,
        );

        final mockWithCompositionAwareMixin =
            _MockWithCompositionAwareMixin();

        expect(
          mockWithCompositionAwareMixin.determineCompositionState(editingState),
          editingState,
        );
      });

      test('should return editing state if text is null', () {
        final editingState = EditingState(
          baseOffset: 0,
          extentOffset: 0,
        );

        final mockWithCompositionAwareMixin =
            _MockWithCompositionAwareMixin();
        mockWithCompositionAwareMixin.composingText = 'Test';

        expect(
          mockWithCompositionAwareMixin.determineCompositionState(editingState),
          editingState,
        );
      });

      test(
          'should return editing state if extentOffset is smaller than composingText length',
          () {
        const composingText = 'composeMe';

        final editingState = EditingState(
          text: 'Test',
          baseOffset: 0,
          extentOffset: 4,
        );

        final mockWithCompositionAwareMixin =
            _MockWithCompositionAwareMixin();
        mockWithCompositionAwareMixin.composingText = composingText;

        expect(
          mockWithCompositionAwareMixin.determineCompositionState(editingState),
          editingState,
        );
      });

      test('should return new composition state - compositing middle of text',
          () {
        const baseOffset = 7;
        const composingText = 'Test';

        final editingState = EditingState(
          text: 'Testing',
          baseOffset: baseOffset,
          extentOffset: baseOffset,
        );

        final mockWithCompositionAwareMixin =
            _MockWithCompositionAwareMixin();
        mockWithCompositionAwareMixin.composingText = composingText;

        const expectedComposingBase = baseOffset - composingText.length;

        expect(
          mockWithCompositionAwareMixin.determineCompositionState(editingState),
          editingState.copyWith(
            composingBaseOffset: expectedComposingBase,
            composingExtentOffset: expectedComposingBase + composingText.length,
          ),
        );
      });

      test(
          'should return new composition state - compositing from beginning of text',
          () {
        const composingText = '今日は';

        final editingState = EditingState(
          text: '今日は',
          baseOffset: 0,
          extentOffset: 3,
        );

        final mockWithCompositionAwareMixin =
            _MockWithCompositionAwareMixin();
        mockWithCompositionAwareMixin.composingText = composingText;

        const expectedComposingBase = 0;

        expect(
            mockWithCompositionAwareMixin
                .determineCompositionState(editingState),
            editingState.copyWith(
                composingBaseOffset: expectedComposingBase,
                composingExtentOffset:
                    expectedComposingBase + composingText.length));
      });
    });
  });

  group('composing range', () {
    late GloballyPositionedTextEditingStrategy editingStrategy;

    setUp(() {
      editingStrategy = _enableEditingStrategy(deltaModel: false);
    });

    tearDown(() {
      editingStrategy.disable();
    });

    test('should be [0, compostionStrLength] on new composition', () {
      const composingText = 'hi';

      _inputElement.dispatchEvent(createDomCompositionEvent(
              _MockWithCompositionAwareMixin._kCompositionUpdate,
              <Object?, Object?>{'data': composingText}));

      // Set the selection text.
      _inputElement.value = composingText;
      _inputElement.dispatchEvent(createDomEvent('Event', 'input'));

      expect(
          editingStrategy.lastEditingState,
          isA<EditingState>()
              .having((EditingState editingState) => editingState.composingBaseOffset,
                  'composingBaseOffset', 0)
              .having((EditingState editingState) => editingState.composingExtentOffset,
                  'composingExtentOffset', composingText.length));
    });

    test(
        'should be [beforeComposingText - composingText, compostionStrLength] on composition in the middle of text',
        () {
      const composingText = 'hi';
      const beforeComposingText = 'beforeComposingText';
      const afterComposingText = 'afterComposingText';

      // Type in the text box, then move cursor to the middle.
      _inputElement.value = '$beforeComposingText$afterComposingText';
      _inputElement.setSelectionRange(beforeComposingText.length, beforeComposingText.length);

      _inputElement.dispatchEvent(createDomCompositionEvent(
          _MockWithCompositionAwareMixin._kCompositionUpdate,
          <Object?, Object?>{ 'data': composingText }
      ));

      // Flush editing state (since we did not compositionend).
      _inputElement.dispatchEvent(createDomEvent('Event', 'input'));

      expect(
          editingStrategy.lastEditingState,
          isA<EditingState>()
              .having((EditingState editingState) => editingState.composingBaseOffset,
                  'composingBaseOffset', beforeComposingText.length - composingText.length)
              .having((EditingState editingState) => editingState.composingExtentOffset,
                  'composingExtentOffset', beforeComposingText.length));
    });
  });

  group('Text Editing Delta Model', () {
    late GloballyPositionedTextEditingStrategy editingStrategy;

    final deltaStream =
        StreamController<TextEditingDeltaState?>.broadcast();

    setUp(() {
        editingStrategy = _enableEditingStrategy(
          deltaModel: true,
          onChange: (_, TextEditingDeltaState? deltaState) => deltaStream.add(deltaState)
        );
    });

    tearDown(() {
      editingStrategy.disable();
    });

    test('should have newly entered composing characters', () async {
      const newComposingText = 'n';

      editingStrategy.setEditingState(EditingState(text: newComposingText, baseOffset: 1, extentOffset: 1));

      final containExpect = expectLater(
          deltaStream.stream.first,
          completion(isA<TextEditingDeltaState>()
            .having((TextEditingDeltaState deltaState) => deltaState.composingOffset, 'composingOffset', 0)
            .having((TextEditingDeltaState deltaState) => deltaState.composingExtent, 'composingExtent', newComposingText.length)
          ));


      _inputElement.dispatchEvent(createDomCompositionEvent(
          _MockWithCompositionAwareMixin._kCompositionUpdate,
          <Object?, Object?>{ 'data': newComposingText }));

      await containExpect;
    });

    test('should emit changed composition', () async {
      const newComposingCharsInOrder = 'hiCompose';

      for (var currCharIndex = 0; currCharIndex < newComposingCharsInOrder.length; currCharIndex++) {
        final currComposingSubstr = newComposingCharsInOrder.substring(0, currCharIndex + 1);

        editingStrategy.setEditingState(
          EditingState(text: currComposingSubstr, baseOffset: currCharIndex + 1, extentOffset: currCharIndex + 1)
        );

        final containExpect = expectLater(
          deltaStream.stream.first,
          completion(isA<TextEditingDeltaState>()
            .having((TextEditingDeltaState deltaState) => deltaState.composingOffset, 'composingOffset', 0)
            .having((TextEditingDeltaState deltaState) => deltaState.composingExtent, 'composingExtent', currCharIndex + 1)
        ));

        _inputElement.dispatchEvent(createDomCompositionEvent(
          _MockWithCompositionAwareMixin._kCompositionUpdate,
          <Object?, Object?>{ 'data': currComposingSubstr }));

        await containExpect;
      }
    },
    // TODO(antholeole): This test fails on Firefox because of how it orders events;
    // it's likely that this will be fixed by https://github.com/flutter/flutter/issues/105243.
    // Until the refactor gets merged, this test should run on all other browsers to prevent
    // regressions in the meantime.
    skip: ui_web.browser.browserEngine == ui_web.BrowserEngine.firefox);
  });
}
