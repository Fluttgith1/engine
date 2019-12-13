// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

part of engine;

/// Make the content editable span visible to facilitate debugging.
const bool _debugVisibleTextEditing = false;

/// The `keyCode` of the "Enter" key.
const int _kReturnKeyCode = 13;

void _emptyCallback(dynamic _) {}

/// These style attributes are constant throughout the life time of an input
/// element.
///
/// They are assigned once during the creation of the DOM element.
void _setStaticStyleAttributes(html.HtmlElement domElement) {
  domElement.classes.add(HybridTextEditing.textEditingClass);

  final html.CssStyleDeclaration elementStyle = domElement.style;
  elementStyle
    ..whiteSpace = 'pre-wrap'
    ..alignContent = 'center'
    ..position = 'absolute'
    ..top = '0'
    ..left = '0'
    ..padding = '0'
    ..opacity = '1'
    ..color = 'transparent'
    ..backgroundColor = 'transparent'
    ..background = 'transparent'
    ..outline = 'none'
    ..border = 'none'
    ..resize = 'none'
    ..textShadow = 'transparent'
    ..transformOrigin = '0 0 0';

  /// This property makes the input's blinking cursor transparent.
  elementStyle.setProperty('caret-color', 'transparent');

  if (_debugVisibleTextEditing) {
    elementStyle
      ..color = 'purple'
      ..outline = '1px solid purple';
  }
}

/// The current text and selection state of a text field.
@visibleForTesting
class EditingState {
  EditingState({this.text, this.baseOffset = 0, this.extentOffset = 0});

  /// Creates an [EditingState] instance using values from an editing state Map
  /// coming from Flutter.
  ///
  /// The `editingState` Map has the following structure:
  /// ```json
  /// {
  ///   "text": "The text here",
  ///   "selectionBase": 0,
  ///   "selectionExtent": 0,
  ///   "selectionAffinity": "TextAffinity.upstream",
  ///   "selectionIsDirectional": false,
  ///   "composingBase": -1,
  ///   "composingExtent": -1
  /// }
  /// ```
  ///
  /// Flutter Framework can send the [selectionBase] and [selectionExtent] as
  /// -1, if so 0 assigned to the [baseOffset] and [extentOffset]. -1 is not a
  /// valid selection range for input DOM elements.
  factory EditingState.fromFlutter(Map<String, dynamic> flutterEditingState) {
    final int selectionBase = flutterEditingState['selectionBase'];
    final int selectionExtent = flutterEditingState['selectionExtent'];
    final String text = flutterEditingState['text'];

    return EditingState(
        text: text,
        baseOffset: math.max(0, selectionBase),
        extentOffset: math.max(0, selectionExtent));
  }

  /// Creates an [EditingState] instance using values from the editing element
  /// in the DOM.
  ///
  /// [domElement] can be a [InputElement] or a [TextAreaElement] depending on
  /// the [InputType] of the text field.
  factory EditingState.fromDomElement(html.HtmlElement domElement) {
    if (domElement is html.InputElement) {
      html.InputElement element = domElement;
      return EditingState(
          text: element.value,
          baseOffset: element.selectionStart,
          extentOffset: element.selectionEnd);
    } else if (domElement is html.TextAreaElement) {
      html.TextAreaElement element = domElement;
      return EditingState(
          text: element.value,
          baseOffset: element.selectionStart,
          extentOffset: element.selectionEnd);
    } else {
      throw UnsupportedError('Initialized with unsupported input type');
    }
  }

  /// The counterpart of [EditingState.fromFlutter]. It generates a Map that
  /// can be sent to Flutter.
  // TODO(mdebbar): Should we get `selectionAffinity` and other properties from flutter's editing state?
  Map<String, dynamic> toFlutter() => <String, dynamic>{
        'text': text,
        'selectionBase': baseOffset,
        'selectionExtent': extentOffset,
      };

  /// The current text being edited.
  final String text;

  /// The offset at which the text selection originates.
  final int baseOffset;

  /// The offset at which the text selection terminates.
  final int extentOffset;

  /// Whether the current editing state is valid or not.
  bool get isValid => baseOffset >= 0 && extentOffset >= 0;

  @override
  int get hashCode => ui.hashValues(text, baseOffset, extentOffset);

  @override
  bool operator ==(dynamic other) {
    if (identical(this, other)) {
      return true;
    }
    if (runtimeType != other.runtimeType) {
      return false;
    }
    final EditingState typedOther = other;
    return text == typedOther.text &&
        baseOffset == typedOther.baseOffset &&
        extentOffset == typedOther.extentOffset;
  }

  @override
  String toString() {
    return assertionsEnabled
        ? 'EditingState("$text", base:$baseOffset, extent:$extentOffset)'
        : super.toString();
  }

  /// Sets the selection values of a DOM element using this [EditingState].
  ///
  /// [domElement] can be a [InputElement] or a [TextAreaElement] depending on
  /// the [InputType] of the text field.
  void applyToDomElement(html.HtmlElement domElement) {
    if (domElement is html.InputElement) {
      html.InputElement element = domElement;
      element.value = text;
      element.setSelectionRange(baseOffset, extentOffset);
    } else if (domElement is html.TextAreaElement) {
      html.TextAreaElement element = domElement;
      element.value = text;
      element.setSelectionRange(baseOffset, extentOffset);
    } else {
      throw UnsupportedError('Unsupported DOM element type');
    }
  }
}

/// Controls the appearance of the input control being edited.
///
/// For example, [inputType] determines whether we should use `<input>` or
/// `<textarea>` as a backing DOM element.
///
/// This corresponds to Flutter's [TextInputConfiguration].
class InputConfiguration {
  InputConfiguration({
    @required this.inputType,
    @required this.inputAction,
    @required this.obscureText,
    @required this.autocorrect,
  });

  InputConfiguration.fromFlutter(Map<String, dynamic> flutterInputConfiguration)
      : inputType = EngineInputType.fromName(
            flutterInputConfiguration['inputType']['name']),
        inputAction = flutterInputConfiguration['inputAction'],
        obscureText = flutterInputConfiguration['obscureText'],
        autocorrect = flutterInputConfiguration['autocorrect'];

  /// The type of information being edited in the input control.
  final EngineInputType inputType;

  /// The default action for the input field.
  final String inputAction;

  /// Whether to hide the text being edited.
  final bool obscureText;

  /// Whether to enable autocorrection.
  ///
  /// Definition of autocorrect can be found in:
  /// https://developer.mozilla.org/en-US/docs/Web/HTML/Element/input
  ///
  /// For future manual tests, note that autocorrect is an attribute only
  /// supported by Safari.
  final bool autocorrect;
}

typedef _OnChangeCallback = void Function(EditingState editingState);
typedef _OnActionCallback = void Function(String inputAction);

/// Interface defining the template for text editing strategies.
///
/// The algorithms will be picked in the runtime depending on the concrete
/// class implementing the interface.
///
/// These algorithms is expected to differ by operating system and/or browser.
abstract class TextEditingStrategy {
  void initializeTextEditing(
    InputConfiguration inputConfig, {
    @required _OnChangeCallback onChange,
    @required _OnActionCallback onAction,
  });

  /// Place the DOM element to its initial position.
  ///
  /// It will be located exactly in the same place with the editable widgets,
  /// however it's contents and cursor will be invisible.
  ///
  /// Users can interact with the element and use the functionalities of the
  /// right-click menu. Such as copy,paste, cut, select, translate...
  void initializeElementPosition();

  /// Register event listeners to the DOM element.
  ///
  /// These event listener will be removed in [disable].
  void addEventHandlers();

  /// Update the element's position.
  ///
  /// The position will be updated everytime Flutter Framework sends
  /// 'TextInput.setEditableSizeAndTransform' message.
  void updateElementPosition(_GeometricInfo geometricInfo);

  /// Set editing state of the element.
  ///
  /// This includes text and selection relelated states. The editing state will
  /// be updated everytime Flutter Framework sends 'TextInput.setEditingState'
  /// message.
  void setEditingState(EditingState editingState);

  /// Set style to the native DOM element used for text editing.
  void updateElementStyle(_EditingStyle style);

  /// Disables the element so it's no longer used for text editing.
  ///
  /// Calling [disable] also removes any registered event listeners.
  void disable();
}

/// Class implementing the default editing strategies for text editing.
///
/// This class uses a DOM element to provide text editing capabilities.
///
///  The backing DOM element could be one of:
///
/// 1. `<input>`.
/// 2. `<textarea>`.
/// 3. `<span contenteditable="true">`.
///
/// This class includes all the default behaviour for an editing element as
/// well as the common properties such as [domElement].
///
/// Strategies written for different formfactor's/browser's should extend this
/// class instead of extending the interface [TextEditingStrategy].
///
/// Unless a formfactor/browser requires specific implementation for a specific
/// strategy the methods in this class should be used.
class DefaultTextEditingStrategy implements TextEditingStrategy {
  final HybridTextEditing owner;

  DefaultTextEditingStrategy(this.owner);

  @visibleForTesting
  bool isEnabled = false;

  html.HtmlElement domElement;
  InputConfiguration _inputConfiguration;
  EditingState _lastEditingState;

  /// Styles associated with the editable text.
  _EditingStyle _style;

  /// Size and transform of the editable text on the page.
  _GeometricInfo _geometricInfo;

  _OnChangeCallback _onChange;
  _OnActionCallback _onAction;

  final List<StreamSubscription<html.Event>> _subscriptions =
      <StreamSubscription<html.Event>>[];

  @override
  void initializeTextEditing(
    InputConfiguration inputConfig, {
    @required _OnChangeCallback onChange,
    @required _OnActionCallback onAction,
  }) {
    assert(!isEnabled);

    domElement = inputConfig.inputType.createDomElement();
    if (inputConfig.obscureText) {
      domElement.setAttribute('type', 'password');
    }

    final String autocorrectValue = inputConfig.autocorrect ? 'on' : 'off';
    domElement.setAttribute('autocorrect', autocorrectValue);

    _setStaticStyleAttributes(domElement);
    _style?.applyToDomElement(domElement);
    initializeElementPosition();
    domRenderer.glassPaneElement.append(domElement);

    isEnabled = true;
    _inputConfiguration = inputConfig;
    _onChange = onChange;
    _onAction = onAction;
  }

  @override
  void initializeElementPosition() {
    positionElement();
  }

  @override
  void addEventHandlers() {
    // Subscribe to text and selection changes.
    _subscriptions.add(domElement.onInput.listen(_handleChange));

    _subscriptions.add(domElement.onKeyDown.listen(_maybeSendAction));

    _subscriptions.add(html.document.onSelectionChange.listen(_handleChange));
  }

  @override
  void updateElementPosition(_GeometricInfo geometricInfo) {
    _geometricInfo = geometricInfo;
    if (isEnabled) {
      positionElement();
    }
  }

  @mustCallSuper
  @override
  void updateElementStyle(_EditingStyle style) {
    _style = style;
    if (isEnabled) {
      _style.applyToDomElement(domElement);
    }
  }

  @override
  void disable() {
    assert(isEnabled);

    isEnabled = false;
    _lastEditingState = null;
    _style = null;
    _geometricInfo = null;

    for (int i = 0; i < _subscriptions.length; i++) {
      _subscriptions[i].cancel();
    }
    _subscriptions.clear();
    domElement.remove();
    domElement = null;
  }

  @mustCallSuper
  @override
  void setEditingState(EditingState editingState) {
    _lastEditingState = editingState;
    if (!isEnabled || !editingState.isValid) {
      return;
    }
    _lastEditingState.applyToDomElement(domElement);
  }

  void positionElement() {
    _geometricInfo?.applyToDomElement(domElement);
    domElement.focus();
  }

  void _handleChange(html.Event event) {
    assert(isEnabled);
    assert(domElement != null);

    EditingState newEditingState = EditingState.fromDomElement(domElement);

    assert(newEditingState != null);

    if (newEditingState != _lastEditingState) {
      _lastEditingState = newEditingState;
      _onChange(_lastEditingState);
    }
  }

  void _maybeSendAction(html.KeyboardEvent event) {
    if (_inputConfiguration.inputType.submitActionOnEnter &&
        event.keyCode == _kReturnKeyCode) {
      event.preventDefault();
      _onAction(_inputConfiguration.inputAction);
    }
  }

  /// Enables the element so it can be used to edit text.
  ///
  /// Register [callback] so that it gets invoked whenever any change occurs in
  /// the text editing element.
  ///
  /// Changes could be:
  /// - Text changes, or
  /// - Selection changes.
  void enable(
    InputConfiguration inputConfig, {
    @required _OnChangeCallback onChange,
    @required _OnActionCallback onAction,
  }) {
    assert(!isEnabled);

    initializeTextEditing(inputConfig, onChange: onChange, onAction: onAction);

    addEventHandlers();

    if (_lastEditingState != null) {
      setEditingState(this._lastEditingState);
    }

    // Re-focuses after setting editing state.
    domElement.focus();
  }
}

/// IOS/Safari behaviour for text editing.
///
/// In iOS, the virtual keyboard might shifts the screen up to make input
/// visible depending on the location of the focused input element.
///
/// Due to this [initializeElementPosition] and [updateElementPosition]
/// strategies are different.
///
/// [disable] is also different since the [_positionInputElementTimer]
/// also needs to be cleaned.
///
/// inputmodeAttribute needs to be set for mobile devices. Due to this
/// [initializeTextEditing] is different.
class IOSTextEditingStrategy extends DefaultTextEditingStrategy {
  IOSTextEditingStrategy(HybridTextEditing owner) : super(owner);

  /// Timer that times when to set the location of the input text.
  ///
  /// This is only used for iOS. In iOS, virtual keyboard shifts the screen.
  /// There is no callback to know if the keyboard is up and how much the screen
  /// has shifted. Therefore instead of listening to the shift and passing this
  /// information to Flutter Framework, we are trying to stop the shift.
  ///
  /// In iOS, the virtual keyboard shifts the screen up if the focused input
  /// element is under the keyboard or very close to the keyboard. Before the
  /// focus is called we are positioning it offscreen. The location of the input
  /// in iOS is set to correct place, 100ms after focus. We use this timer for
  /// timing this delay.
  Timer _positionInputElementTimer;
  static const Duration _delayBeforePositioning =
      const Duration(milliseconds: 100);

  /// Whether or not the input element can be positioned at this point in time.
  ///
  /// This is currently only used in iOS. It's set to false before focusing the
  /// input field, and set back to true after a short timer. We do this because
  /// if the input field is positioned before focus, it could be pushed to an
  /// incorrect position by the virtual keyboard.
  ///
  /// See:
  ///
  /// * [_delayBeforePositioning] which controls how long to wait before
  ///   positioning the input field.
  bool _canPosition = true;

  @override
  void initializeTextEditing(
    InputConfiguration inputConfig, {
    @required _OnChangeCallback onChange,
    @required _OnActionCallback onAction,
  }) {
    super.initializeTextEditing(inputConfig,
        onChange: onChange, onAction: onAction);
    inputConfig.inputType.configureInputMode(domElement);
  }

  @override
  void initializeElementPosition() {
    /// Position the element outside of the page before focusing on it. This is
    /// useful for not triggering a scroll when iOS virtual keyboard is
    /// coming up.
    domElement.style.transform = 'translate(-9999px, -9999px)';

    _canPosition = false;

    _subscriptions.add(domElement.onFocus.listen((_) {
      // Cancel previous timer if exists.
      _positionInputElementTimer?.cancel();
      _positionInputElementTimer = Timer(_delayBeforePositioning, () {
        _canPosition = true;
        positionElement();
      });

      // When the virtual keyboard is closed on iOS, onBlur is triggered.
      _subscriptions.add(domElement.onBlur.listen((_) {
        // Cancel the timer since there is no need to set the location of the
        // input element anymore. It needs to be focused again to be editable
        // by the user.
        _positionInputElementTimer?.cancel();
        _positionInputElementTimer = null;
      }));
    }));
  }

  @override
  void updateElementPosition(_GeometricInfo geometricInfo) {
    _geometricInfo = geometricInfo;
    if (isEnabled && _canPosition) {
      positionElement();
    }
  }

  @override
  void disable() {
    super.disable();
    _positionInputElementTimer?.cancel();
    _positionInputElementTimer = null;
  }
}

/// Android behaviour for text editing.
///
/// inputmodeAttribute needs to be set for mobile devices. Due to this
/// [initializeTextEditing] is different.
///
/// Keyboard acts differently than other devices. [addEventHandlers] handles
/// this case as an extra.
class AndroidTextEditingStrategy extends DefaultTextEditingStrategy {
  AndroidTextEditingStrategy(HybridTextEditing owner) : super(owner);

  @override
  void initializeTextEditing(
    InputConfiguration inputConfig, {
    @required _OnChangeCallback onChange,
    @required _OnActionCallback onAction,
  }) {
    super.initializeTextEditing(inputConfig,
        onChange: onChange, onAction: onAction);
    inputConfig.inputType.configureInputMode(domElement);
  }

  @override
  void addEventHandlers() {
    super.addEventHandlers();
    // Chrome on Android will hide the onscreen keyboard when you tap outside
    // the text box. Instead, we want the framework to tell us to hide the
    // keyboard via `TextInput.clearClient` or `TextInput.hide`.
    if (browserEngine == BrowserEngine.blink ||
        browserEngine == BrowserEngine.unknown) {
      _subscriptions.add(domElement.onBlur.listen((_) {
        if (isEnabled) {
          // Refocus.
          domElement.focus();
        }
      }));
    }
  }
}

/// Firefox behaviour for text editing.
///
/// Selections are different in Firefox. [addEventHandlers] strategy is
/// impelemented diefferently in Firefox.
class FirefoxTextEditingStrategy extends DefaultTextEditingStrategy {
  FirefoxTextEditingStrategy(HybridTextEditing owner) : super(owner);

  @override
  void addEventHandlers() {
    // Subscribe to text and selection changes.
    _subscriptions.add(domElement.onInput.listen(_handleChange));

    _subscriptions.add(domElement.onKeyDown.listen(_maybeSendAction));

    /// Detects changes in text selection.
    ///
    /// In Firefox, when cursor moves, neither selectionChange nor onInput
    /// events are triggered. We are listening to keyup event. Selection start,
    /// end values are used to decide if the text cursor moved.
    ///
    /// Specific keycodes are not checked since users/applications can bind
    /// their own keys to move the text cursor.
    /// Decides if the selection has changed (cursor moved) compared to the
    /// previous values.
    ///
    /// After each keyup, the start/end values of the selection is compared to
    /// the previously saved editing state.
    _subscriptions.add(domElement.onKeyUp.listen((event) {
      _handleChange(event);
    }));

    /// In Firefox the context menu item "Select All" does not work without
    /// listening to onSelect. On the other browsers onSelectionChange is
    /// enough for covering "Select All" functionality.
    _subscriptions.add(domElement.onSelect.listen(_handleChange));
  }
}

/// Text editing used by accesibilty mode.
///
/// [PersistentTextEditingElement] assumes the caller will own the creation,
/// insertion and disposal of the DOM element. Due to this
/// [initializeElementPosition], [initializeTextEditing] and
/// [disable] strategies are handled differently.
///
/// This class is still responsible for hooking up the DOM element with the
/// [HybridTextEditing] instance so that changes are communicated to Flutter.
class PersistentTextEditingElement extends DefaultTextEditingStrategy {
  /// Creates a [PersistentTextEditingElement] that eagerly instantiates
  /// [domElement] so the caller can insert it before calling
  /// [PersistentTextEditingElement.enable].
  PersistentTextEditingElement(
      HybridTextEditing owner, html.HtmlElement domElement)
      : super(owner) {
    // Make sure the DOM element is of a type that we support for text editing.
    // TODO(yjbanov): move into initializer list when https://github.com/dart-lang/sdk/issues/37881 is fixed.
    assert((domElement is html.InputElement) ||
        (domElement is html.TextAreaElement));
    super.domElement = domElement;
  }

  @override
  void disable() {
    // We don't want to remove the DOM element because the caller is responsible
    // for that.
    //
    // Remove focus from the editable element to cause the keyboard to hide.
    // Otherwise, the keyboard stays on screen even when the user navigates to
    // a different screen (e.g. by hitting the "back" button).
    domElement.blur();
  }

  @override
  void initializeElementPosition() {
    // No-op
  }

  @override
  void initializeTextEditing(InputConfiguration inputConfig,
      {_OnChangeCallback onChange, _OnActionCallback onAction}) {
    // In accesibilty mode, the user of this class is supposed to insert the
    // [domElement] on their own. Let's make sure they did.
    assert(domElement != null);
    assert(html.document.body.contains(domElement));

    isEnabled = true;
    _inputConfiguration = inputConfig;
    _onChange = onChange;
    _onAction = onAction;

    domElement.focus();
  }

  @override
  void setEditingState(EditingState editingState) {
    super.setEditingState(editingState);

    // Refocus after setting editing state.
    domElement.focus();
  }

}

/// Text editing singleton.
final HybridTextEditing textEditing = HybridTextEditing();

/// Should be used as a singleton to provide support for text editing in
/// Flutter Web.
///
/// The approach is "hybrid" because it relies on Flutter for
/// displaying, and HTML for user interactions:
///
/// - HTML's contentEditable feature handles typing and text changes.
/// - HTML's selection API handles selection changes and cursor movements.
class HybridTextEditing {
  /// The text editing stategy used. It can change depending on the
  /// formfactor/browser.
  ///
  /// It uses an HTML element to manage editing state when a custom element is
  /// not provided via [useCustomEditableElement]
  DefaultTextEditingStrategy _defaultEditingElement;

  /// Private constructor so this class can be a singleton.
  ///
  /// The constructor also decides which text editing strategy to use depending
  /// on the operating system and browser engine.
  HybridTextEditing() {
    if (browserEngine == BrowserEngine.webkit &&
        operatingSystem == OperatingSystem.iOs) {
      this._defaultEditingElement = IOSTextEditingStrategy(this);
    } else if (browserEngine == BrowserEngine.blink &&
        operatingSystem == OperatingSystem.android) {
      this._defaultEditingElement = AndroidTextEditingStrategy(this);
    } else if (browserEngine == BrowserEngine.firefox) {
      this._defaultEditingElement = FirefoxTextEditingStrategy(this);
    } else {
      this._defaultEditingElement = DefaultTextEditingStrategy(this);
    }
  }

  /// The HTML element used to manage editing state.
  ///
  /// This field is populated using [useCustomEditableElement]. If `null` the
  /// [_defaultEditingElement] is used instead.
  DefaultTextEditingStrategy _customEditingElement;

  DefaultTextEditingStrategy get editingElement {
    if (_customEditingElement != null) {
      return _customEditingElement;
    }
    return _defaultEditingElement;
  }

  /// A CSS class name used to identify all elements used for text editing.
  @visibleForTesting
  static const String textEditingClass = 'flt-text-editing';

  static bool isEditingElement(html.Element element) {
    return element.classes.contains(textEditingClass);
  }

  /// Requests that [customEditingElement] is used for managing text editing state
  /// instead of the hidden default element.
  ///
  /// Use [stopUsingCustomEditableElement] to switch back to default element.
  void useCustomEditableElement(
      DefaultTextEditingStrategy customEditingElement) {
    if (isEditing && customEditingElement != _customEditingElement) {
      stopEditing();
    }
    _customEditingElement = customEditingElement;
  }

  /// Switches back to using the built-in default element for managing text
  /// editing state.
  void stopUsingCustomEditableElement() {
    useCustomEditableElement(null);
  }

  int _clientId;

  /// Flag which shows if there is an ongoing editing.
  ///
  /// Also used to define if a keyboard is needed.
  @visibleForTesting
  bool isEditing = false;

  InputConfiguration _configuration;

  /// All "flutter/textinput" platform messages should be sent to this method.
  void handleTextInput(ByteData data) {
    final MethodCall call = const JSONMethodCodec().decodeMethodCall(data);
    switch (call.method) {
      case 'TextInput.setClient':
        final bool clientIdChanged =
            _clientId != null && _clientId != call.arguments[0];
        if (clientIdChanged && isEditing) {
          stopEditing();
        }
        _clientId = call.arguments[0];
        _configuration = InputConfiguration.fromFlutter(call.arguments[1]);
        break;

      case 'TextInput.setEditingState':
        editingElement
            .setEditingState(EditingState.fromFlutter(call.arguments));
        break;

      case 'TextInput.show':
        if (!isEditing) {
          _startEditing();
        }
        break;

      case 'TextInput.setEditableSizeAndTransform':
        editingElement
            .updateElementPosition(_GeometricInfo.fromFlutter(call.arguments));
        break;

      case 'TextInput.setStyle':
        editingElement
            .updateElementStyle(_EditingStyle.fromFlutter(call.arguments));
        break;

      case 'TextInput.clearClient':
      case 'TextInput.hide':
        if (isEditing) {
          stopEditing();
        }
        break;
    }
  }

  void _startEditing() {
    assert(!isEditing);
    isEditing = true;
    editingElement.enable(
      _configuration,
      onChange: _syncEditingStateToFlutter,
      onAction: _sendInputActionToFlutter,
    );
  }

  void stopEditing() {
    assert(isEditing);
    isEditing = false;
    editingElement.disable();
  }

  void _syncEditingStateToFlutter(EditingState editingState) {
    ui.window.onPlatformMessage(
      'flutter/textinput',
      const JSONMethodCodec().encodeMethodCall(
        MethodCall('TextInputClient.updateEditingState', <dynamic>[
          _clientId,
          editingState.toFlutter(),
        ]),
      ),
      _emptyCallback,
    );
  }

  void _sendInputActionToFlutter(String inputAction) {
    ui.window.onPlatformMessage(
      'flutter/textinput',
      const JSONMethodCodec().encodeMethodCall(
        MethodCall(
          'TextInputClient.performAction',
          <dynamic>[_clientId, inputAction],
        ),
      ),
      _emptyCallback,
    );
  }

  void sendTextConnectionClosedToFlutterIfAny() {
    print('sendTextConnectionClosedToFlutter for client: $clientId');
    if (isEditing) {
      stopEditing();
      ui.window.onPlatformMessage(
        'flutter/textinput',
        const JSONMethodCodec().encodeMethodCall(
          MethodCall(
            'TextInputClient.onConnectionClosed',
            <dynamic>[
              _clientId,
            ],
          ),
        ),
        _emptyCallback,
      );
    }
  }
}

/// Information on the font and alignment of a text editing element.
///
/// This information is received via TextInput.setStyle message.
class _EditingStyle {
  _EditingStyle({
    @required this.textDirection,
    @required this.fontSize,
    @required this.textAlign,
    @required this.fontFamily,
    @required this.fontWeight,
  });

  factory _EditingStyle.fromFlutter(Map<String, dynamic> flutterStyle) {
    assert(flutterStyle.containsKey('fontSize'));
    assert(flutterStyle.containsKey('fontFamily'));
    assert(flutterStyle.containsKey('textAlignIndex'));
    assert(flutterStyle.containsKey('textDirectionIndex'));

    final int textAlignIndex = flutterStyle['textAlignIndex'];
    final int textDirectionIndex = flutterStyle['textDirectionIndex'];
    final int fontWeightIndex = flutterStyle['fontWeightIndex'];

    // Convert [fontWeightIndex] to its CSS equivalent value.
    final String fontWeight = fontWeightIndex != null
        ? fontWeightIndexToCss(fontWeightIndex: fontWeightIndex)
        : 'normal';

    // Also convert [textAlignIndex] and [textDirectionIndex] to their
    // corresponding enum values in [ui.TextAlign] and [ui.TextDirection]
    // respectively.
    return _EditingStyle(
      fontSize: flutterStyle['fontSize'],
      fontFamily: flutterStyle['fontFamily'],
      textAlign: ui.TextAlign.values[textAlignIndex],
      textDirection: ui.TextDirection.values[textDirectionIndex],
      fontWeight: fontWeight,
    );
  }

  /// This information will be used for changing the style of the hidden input
  /// element, which will match it's size to the size of the editable widget.
  final double fontSize;
  final String fontWeight;
  final String fontFamily;
  final ui.TextAlign textAlign;
  final ui.TextDirection textDirection;

  String get align => textAlignToCssValue(textAlign, textDirection);

  String get cssFont => '${fontWeight} ${fontSize}px ${fontFamily}';

  void applyToDomElement(html.HtmlElement domElement) {
    domElement.style
      ..textAlign = align
      ..font = cssFont;
  }
}

/// Information on the location and size of the editing element.
///
/// This information is received via "TextInput.setEditableSizeAndTransform"
/// message. Framework currently sends this information on paint.
class _GeometricInfo {
  _GeometricInfo({
    @required this.width,
    @required this.height,
    @required this.transform,
  });

  factory _GeometricInfo.fromFlutter(
    Map<String, dynamic> flutterMap,
  ) {
    assert(flutterMap.containsKey('width'));
    assert(flutterMap.containsKey('height'));
    assert(flutterMap.containsKey('transform'));

    final List<double> transformList =
        List<double>.from(flutterMap['transform']);
    return _GeometricInfo(
      width: flutterMap['width'],
      height: flutterMap['height'],
      transform: Float64List.fromList(transformList),
    );
  }

  final double width;
  final double height;
  final Float64List transform;

  String get cssTransform => float64ListToCssTransform(transform);

  void applyToDomElement(html.HtmlElement domElement) {
    domElement.style
      ..width = '${width}px'
      ..height = '${height}px'
      ..transform = cssTransform;
  }
}
