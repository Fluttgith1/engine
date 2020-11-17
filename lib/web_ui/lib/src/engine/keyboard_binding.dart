// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// @dart = 2.12
part of engine;

typedef VoidCallback = void Function();
typedef ValueGetter<T> = T Function();
typedef _ModifierGetter = bool Function(FlutterHtmlKeyboardEvent event);

// Set this flag to true to see all the fired events in the console.
const bool _debugLogKeyEvents = false;

// Bitmask for lock flags. Must be kept up-to-date with FlutterKeyLockFlags in
// embedder.h.
const int _kLockFlagCapsLock = 0x01;
const int _kLockFlagNumLock = 0x02;
const int _kLockFlagScrollLock = 0x04;
// Map physical keys for lock keys to their flags.
const Map<String, int> _kPhysicalKeyToLockFlag = {
  _kPhysicalCapsLock: _kLockFlagCapsLock,
  _kPhysicalNumLock: _kLockFlagNumLock,
  _kPhysicalScrollLock: _kLockFlagScrollLock,
};

const int _kLogicalAltLeft = 0x0300000102;
const int _kLogicalAltRight = 0x0400000102;
const int _kLogicalControlLeft = 0x0300000105;
const int _kLogicalControlRight = 0x0400000105;
const int _kLogicalShiftLeft = 0x030000010d;
const int _kLogicalShiftRight = 0x040000010d;
const int _kLogicalMetaLeft = 0x0300000109;
const int _kLogicalMetaRight = 0x0400000109;
// Map logical keys for modifier keys to the functions that can get their
// modifier flag out of an event.
final Map<int, _ModifierGetter> _kLogicalKeyToModifierGetter = {
  _kLogicalAltLeft: (FlutterHtmlKeyboardEvent event) => event.altKey,
  _kLogicalAltRight: (FlutterHtmlKeyboardEvent event) => event.altKey,
  _kLogicalControlLeft: (FlutterHtmlKeyboardEvent event) => event.ctrlKey,
  _kLogicalControlRight: (FlutterHtmlKeyboardEvent event) => event.ctrlKey,
  _kLogicalShiftLeft: (FlutterHtmlKeyboardEvent event) => event.shiftKey,
  _kLogicalShiftRight: (FlutterHtmlKeyboardEvent event) => event.shiftKey,
  _kLogicalMetaLeft: (FlutterHtmlKeyboardEvent event) => event.metaKey,
  _kLogicalMetaRight: (FlutterHtmlKeyboardEvent event) => event.metaKey,
};

// After a keydown is received, this is the duration we wait for a repeat event
// before we decide to synthesize a keyup event.
//
// On Linux and Windows, the typical ranges for keyboard repeat delay go up to
// 1000ms. On Mac, the range goes up to 2000ms.
const Duration _kKeydownCancelDurationNormal = Duration(milliseconds: 1000);
const Duration _kKeydownCancelDurationMacOs = Duration(milliseconds: 2000);

late final int _kCharLowerA = 'a'.codeUnitAt(0);
late final int _kCharLowerZ = 'z'.codeUnitAt(0);
late final int _kCharUpperA = 'A'.codeUnitAt(0);
late final int _kCharUpperZ = 'Z'.codeUnitAt(0);
bool isAlphabet(int charCode) {
  return (charCode >= _kCharLowerA && charCode <= _kCharLowerZ)
      || (charCode >= _kCharUpperA && charCode <= _kCharUpperZ);
}

const String _kPhysicalCapsLock = 'CapsLock';
const String _kPhysicalNumLock = 'NumLock';
const String _kPhysicalScrollLock = 'ScrollLock';

const String _kLogicalDead = 'Dead';

const int _kWebKeyIdPlane = 0x00800000000;
const int _kAutogeneratedMask = 0x10000000000;

// Bits in a Flutter logical event to generate the logical key for dead keys.
//
// Logical keys for dead keys are generated by annotating physical keys with
// modifiers (see `_getLogicalCode`).
const int _kDeadKeyCtrl = 0x100000000000;
const int _kDeadKeyShift = 0x200000000000;
const int _kDeadKeyAlt = 0x400000000000;
const int _kDeadKeyMeta = 0x800000000000;

typedef DispatchKeyData = void Function(ui.KeyData data);

/// Converts a floating number timestamp (in milliseconds) to a [Duration] by
/// splitting it into two integer components: milliseconds + microseconds.
Duration _eventTimeStampToDuration(num milliseconds) {
  final int ms = milliseconds.toInt();
  final int micro = ((milliseconds - ms) * Duration.microsecondsPerMillisecond).toInt();
  return Duration(milliseconds: ms, microseconds: micro);
}

class KeyboardBinding {
  /// The singleton instance of this object.
  static KeyboardBinding? get instance => _instance;
  static KeyboardBinding? _instance;

  static void initInstance(html.Element glassPaneElement) {
    if (_instance == null) {
      _instance = KeyboardBinding._(glassPaneElement);
      assert(() {
        registerHotRestartListener(_instance!._reset);
        return true;
      }());
    }
  }

  KeyboardBinding._(this.glassPaneElement) {
    _setup();
  }

  final html.Element glassPaneElement;
  late KeyboardConverter _converter;
  final Map<String, html.EventListener> _listeners = <String, html.EventListener>{};

  void _addEventListener(String eventName, html.EventListener handler) {
    final html.EventListener loggedHandler = (html.Event event) {
      if (_debugLogKeyEvents) {
        print(event.type);
      }
      if (EngineSemanticsOwner.instance.receiveGlobalEvent(event)) {
        handler(event);
      }
    };
    assert(!_listeners.containsKey(eventName));
    _listeners[eventName] = loggedHandler;
    html.window.addEventListener(eventName, loggedHandler, true);
  }

  /// Remove all active event listeners.
  void _clearListeners() {
    _listeners.forEach((String eventName, html.EventListener listener) {
      html.window.removeEventListener(eventName, listener, true);
    });
    _listeners.clear();
  }
  void _onKeyData(ui.KeyData data) {
    EnginePlatformDispatcher.instance.invokeOnKeyData(data);
  }

  void _setup() {
    _addEventListener('keydown', (html.Event event) {
      return _converter.handleEvent(FlutterHtmlKeyboardEvent(event as html.KeyboardEvent));
    });
    _addEventListener('keyup', (html.Event event) {
      return _converter.handleEvent(FlutterHtmlKeyboardEvent(event as html.KeyboardEvent));
    });
    _converter = KeyboardConverter(_onKeyData, onMacOs: operatingSystem == OperatingSystem.macOs);
  }

  void _reset() {
    _clearListeners();
    _converter.dispose();

    _setup();
  }
}

class AsyncKeyboardDispatching {
  AsyncKeyboardDispatching({
    required this.keyData,
    this.callback,
  });

  final ui.KeyData keyData;
  final VoidCallback? callback;
}

// A wrapper of [html.KeyboardEvent] with reduced methods delegated to the event
// for the convenience of testing.
class FlutterHtmlKeyboardEvent {
  FlutterHtmlKeyboardEvent(this._event);

  final html.KeyboardEvent _event;

  String get type => _event.type;
  String? get code => _event.code;
  String? get key => _event.key;
  bool? get repeat => _event.repeat;
  int? get location => _event.location;
  num? get timeStamp => _event.timeStamp;
  bool get altKey => _event.altKey;
  bool get ctrlKey => _event.ctrlKey;
  bool get shiftKey => _event.shiftKey;
  bool get metaKey => _event.metaKey;

  bool getModifierState(String key) => _event.getModifierState(key);
  void preventDefault() => _event.preventDefault();
}

// Reads [html.KeyboardEvent], then [dispatches ui.KeyData] accordingly.
//
// The events are read through [handleEvent], and dispatched through the
// [dispatchKeyData] as given in the constructor. Some key data might be
// dispatched asynchronously.
class KeyboardConverter {
  KeyboardConverter(this.dispatchKeyData, {this.onMacOs = false});

  final DispatchKeyData dispatchKeyData;
  final bool onMacOs;

  bool _disposed = false;
  void dispose() {
    _disposed = true;
  }

  // On macOS, CapsLock behaves differently in that, a keydown event occurs when
  // the key is pressed and the light turns on, while a keyup event occurs when the
  // key is pressed and the light turns off. Flutter considers both events as
  // key down, and synthesizes immediate cancel events following them. The state
  // of "whether CapsLock is on" should be accessed by "activeLocks".
  bool _shouldSynthesizeCapsLockUp() {
    return onMacOs;
  }

  Duration get _keydownCancelDuration => onMacOs ? _kKeydownCancelDurationMacOs : _kKeydownCancelDurationNormal;

  bool _shouldPreventDefault(FlutterHtmlKeyboardEvent event) {
    switch (event.key) {
      case 'Tab':
        return true;

      default:
        return false;
    }
  }

  static int _getPhysicalCode(String code) {
    return kWebToPhysicalKey[code] ?? (code.hashCode + _kWebKeyIdPlane + _kAutogeneratedMask);
  }

  static int _getModifierMask(FlutterHtmlKeyboardEvent event) {
    final bool altDown = event.altKey;
    final bool ctrlDown = event.ctrlKey;
    final bool shiftDown = event.shiftKey;
    final bool metaDown = event.metaKey;
    return (altDown ? _kDeadKeyAlt : 0) +
           (ctrlDown ? _kDeadKeyCtrl : 0) +
           (shiftDown ? _kDeadKeyShift : 0) +
           (metaDown ? _kDeadKeyMeta : 0);
  }

  // Whether `event.key` should be considered a key name.
  //
  // The `event.key` can either be a key name or the printable character. If the
  // first character is an alphabet, it must be either 'A' to 'Z' ( and return
  // true), or be a key name (and return false). Otherwise, return true.
  static bool _eventKeyIsKeyname(String key) {
    assert(key.length > 0);
    return isAlphabet(key.codeUnitAt(0)) && key.length > 1;
  }

  static int _characterToLogicalKey(String key) {
    // Assume the length being <= 2 to be sufficient in all cases. If not,
    // extend the algorithm.
    assert(key.length <= 2);
    int result = key.codeUnitAt(0) & 0xffff;
    if (key.length == 2) {
      result += key.codeUnitAt(1) << 16;
    }
    // Convert lower letters to upper letters
    if (result >= _kCharLowerA && result <= _kCharLowerZ) {
      result = result - _kCharLowerA + _kCharUpperA;
    }
    return result;
  }

  static int _deadKeyToLogicalKey(int physicalKey, FlutterHtmlKeyboardEvent event) {
    // 'Dead' is used to represent dead keys, such as a diacritic to the
    // following base letter (such as Option-e results in ´).
    //
    // Assume they can be told apart with the physical key and the modifiers
    // pressed.
    return physicalKey + _getModifierMask(event) + _kWebKeyIdPlane + _kAutogeneratedMask;
  }

  static int _otherLogicalKey(String key) {
    return kWebToLogicalKey[key] ?? (key.hashCode + _kWebKeyIdPlane + _kAutogeneratedMask);
  }

  // Map from pressed physical key to corresponding pressed logical key.
  //
  // Multiple physical keys can be mapped to the same logical key, usually due
  // to positioned keys (left/right/numpad) or multiple keyboards.
  final Map<int, int> _pressingRecords = <int, int>{};

  int _activeLocks = 0;
  void _updateLockFlag(int bit, bool value) {
    if (value) {
      _activeLocks |= bit;
    } else {
      _activeLocks &= ~bit;
    }
  }
  void _toggleLockFlag(int bit) {
    _updateLockFlag(bit, (_activeLocks & bit) == 0);
  }

  // Schedule the dispatching of an event in the future. The `callback` will
  // invoked before that.
  //
  // Returns a callback that cancels the schedule. Disposal of
  // `KeyBoardConverter` also cancels the shedule automatically.
  VoidCallback _scheduleAsyncEvent(Duration duration, ValueGetter<ui.KeyData> getData, VoidCallback callback) {
    bool canceled = false;
    Future<void>.delayed(duration).then<void>((_) {
      if (!canceled && !_disposed) {
        callback();
        dispatchKeyData(getData());
      }
    });
    return () { canceled = true; };
  }

  // ## About Key guards
  //
  // When the user enters a browser/system shortcut (e.g. `cmd+alt+i`) the
  // browser doesn't send a keyup for it. This puts the framework in a corrupt
  // state because it thinks the key was never released.
  //
  // To avoid this, we rely on the fact that browsers send repeat events
  // while the key is held down by the user. If we don't receive a repeat
  // event within a specific duration ([_keydownCancelDuration]) we assume
  // the user has released the key and we synthesize a keyup event.
  final Map<int, VoidCallback> _keyGuards = <int, VoidCallback>{};
  // Call this method on the down or repeated event of a non-modifier key.
  void _startGuardingKey(int physicalKey, int logicalKey, Duration currentTimeStamp) {
    final VoidCallback cancelingCallback = _scheduleAsyncEvent(
      _keydownCancelDuration,
      () => ui.KeyData(
        timeStamp: currentTimeStamp + _keydownCancelDuration,
        change: ui.KeyChange.up,
        physical: physicalKey,
        logical: logicalKey,
        locks: _activeLocks,
        character: null,
        synthesized: true,
      ),
      () {
        _pressingRecords.remove(physicalKey);
      }
    );
    _keyGuards.remove(physicalKey)?.call();
    _keyGuards[physicalKey] = cancelingCallback;
  }
  // Call this method on an up event event of a non-modifier key.
  void _stopGuardingKey(int physicalKey) {
    _keyGuards.remove(physicalKey)?.call();
  }

  // Parse the HTML event, update states, and dispatch Flutter key data through
  // [dispatchKeyData].
  //
  //  * The method might dispatch some synthesized key data first to update states,
  //    results discarded.
  //  * Then it dispatches exactly one non-synthesized key data that corresponds
  //    to the `event`, i.e. the main key data. The result of this dispatching is
  //    returned to indicate whether the event is processed by Flutter.
  //  * Some key data might be synthesized to update states after the main key
  //    data. They are always scheduled asynchronously with results discarded.
  bool handleEvent(FlutterHtmlKeyboardEvent event) {
    final Duration timeStamp = _eventTimeStampToDuration(event.timeStamp!);

    if (_shouldPreventDefault(event)) {
      event.preventDefault();
    }
    final String eventKey = event.key!;

    final int physicalKey = _getPhysicalCode(event.code!);
    final bool logicalKeyIsCharacter = !_eventKeyIsKeyname(eventKey);
    final String? character = logicalKeyIsCharacter ? eventKey : null;
    final int logicalKey = () {
      if (kWebLogicalLocationMap.containsKey(event.key!)) {
        final int? result = kWebLogicalLocationMap[event.key!]?[event.location!];
        assert(result != null, 'Invalid modifier location: ${event.key}, ${event.location}');
        return result!;
      }
      if (character != null)
        return _characterToLogicalKey(character);
      if (eventKey == _kLogicalDead)
        return _deadKeyToLogicalKey(physicalKey, event);
      return _otherLogicalKey(eventKey);
    }();

    assert(event.type == 'keydown' || event.type == 'keyup');
    final bool isPhysicalDown = event.type == 'keydown' ||
      // On macOS, both keydown and keyup events of CapsLock should be considered keydown,
      // followed by an immediate cancel event.
      (_shouldSynthesizeCapsLockUp() && event.code! == _kPhysicalCapsLock);

    final int? lastLogicalRecord = _pressingRecords[physicalKey];

    ui.KeyChange change;

    if (_shouldSynthesizeCapsLockUp() && event.code! == _kPhysicalCapsLock) {
      // Case 1: Handle CapsLock on macOS
      //
      // On macOS, both keydown and keyup events of CapsLock are considered
      // keydown, followed by an immediate synchronized up event.

      _scheduleAsyncEvent(
        Duration.zero,
        () => ui.KeyData(
          timeStamp: timeStamp,
          change: ui.KeyChange.up,
          physical: physicalKey,
          logical: logicalKey,
          character: null,
          locks: _activeLocks,
          synthesized: true,
        ),
        () {
          _pressingRecords.remove(physicalKey);
        }
      );
      change = ui.KeyChange.down;

    } else if (isPhysicalDown) {
      // Case 2: Handle key down of normal keys
      change = ui.KeyChange.down;
      if (lastLogicalRecord != null) {
        // This physical key is being pressed according to the record.
        if (event.repeat ?? false) {
          // A normal repeated key.
          change = ui.KeyChange.repeat;
        } else {
          // A non-repeated key has been pressed that has the exact physical key as
          // a currently pressed one, usually indicating multiple keyboards are
          // pressing keys with the same physical key, or the up event was lost
          // during a loss of focus. The down event is ignored.
          return false;
        }
      } else {
        // This physical key is not being pressed according to the record. It's a
        // normal down event, whether the system event is a repeat or not.
      }

    } else { // isPhysicalDown is false and not CapsLock
      // Case 2: Handle key up of normal keys
      if (lastLogicalRecord == null) {
        // The physical key has been released before. It indicates multiple
        // keyboards pressed keys with the same physical key. Ignore the up event.
        return false;
      }

      change = ui.KeyChange.up;
    }

    late final int? nextLogicalRecord;
    switch (change) {
      case ui.KeyChange.down:
        assert(lastLogicalRecord == null);
        nextLogicalRecord = logicalKey;
        break;
      case ui.KeyChange.up:
        assert(lastLogicalRecord != null);
        nextLogicalRecord = null;
        break;
      case ui.KeyChange.repeat:
        assert(lastLogicalRecord != null);
        nextLogicalRecord = lastLogicalRecord;
        break;
    }
    if (nextLogicalRecord == null) {
      _pressingRecords.remove(physicalKey);
    } else {
      _pressingRecords[physicalKey] = nextLogicalRecord;
    }

    // After updating _pressingRecords, synchronize modifier states. The
    // `event.***Key` fields can be used to reduce some omitted modifier key
    // events. We can deduce key cancel events if they are false. Key sync
    // events can not be deduced since we don't know which physical key they
    // represent.
    _kLogicalKeyToModifierGetter.forEach((int logicalKey, _ModifierGetter getModifier) {
      // print(_pressingRecords);
      if (_pressingRecords.containsValue(logicalKey) && !getModifier(event)) {
        _pressingRecords.removeWhere((int physicalKey, int logicalRecord) {
          if (logicalRecord != logicalKey)
            return false;

          dispatchKeyData(ui.KeyData(
            timeStamp: timeStamp,
            change: ui.KeyChange.up,
            physical: physicalKey,
            logical: logicalKey,
            character: null,
            locks: _activeLocks,
            synthesized: true,
          ));

          return true;
        });
      }
    });

    // Update lock flags
    if (!logicalKeyIsCharacter) {
      assert(event.repeat == false);
      if (_shouldSynthesizeCapsLockUp() && event.code! == _kPhysicalCapsLock) {
        // If `_shouldSynthesizeCapsLockUp` is false, CapsLock is handled in
        // the following else clause.
        _updateLockFlag(_kLockFlagCapsLock, event.type != 'keyup');
      } else if (nextLogicalRecord != null) {
        final int? lockFlag = _kPhysicalKeyToLockFlag[event.code!];
        if (lockFlag != null)
          _toggleLockFlag(lockFlag);
      }
    }

    // Update key guards
    if (logicalKeyIsCharacter) {
      if (nextLogicalRecord != null) {
        _startGuardingKey(physicalKey, logicalKey, timeStamp);
      } else {
        _stopGuardingKey(physicalKey);
      }
    }

    final ui.KeyData keyData = ui.KeyData(
      timeStamp: timeStamp,
      change: change,
      physical: physicalKey,
      logical: lastLogicalRecord ?? logicalKey,
      character: change == ui.KeyChange.up ? null : character,
      locks: _activeLocks,
      synthesized: false,
    );

    dispatchKeyData(keyData);
    return true;
  }
}
