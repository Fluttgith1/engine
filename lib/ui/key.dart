// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// @dart = 2.10

part of dart.ui;

/// How the key has changed since the last report.
///
// Must match the KeyChange enum in ui/window/key_data.h.
enum KeyChange {
  /// The key is pressed.
  down,

  /// The key is released.
  up,

  /// The key is held, causing a repeated key input.
  repeat,
}

/// Information about the change of a key.
class KeyData {
  /// Creates an object that represents the change of a key.
  const KeyData({
    required this.timeStamp,
    required this.change,
    required this.physical,
    required this.logical,
    required this.character,
    required this.locks,
  });

  /// Time of event dispatch, relative to an arbitrary timeline.
  ///
  /// For [KeyChange.synchronize] and [KeyChange.cancel] events, the [timeStamp]
  /// might not be the actual time that the key press or release happens.
  final Duration timeStamp;

  /// How the key has changed since the last report.
  final KeyChange change;

  /// The key code for the physical key that has changed.
  final int physical;

  /// The key code for the logical key that has changed.
  final int logical;

  /// Character input from the event.
  ///
  /// Not available to up events.
  final String? character;

  /// A bitmask that represents lock keys (such as CapsLock) that are active
  /// after this event.
  final int locks;

  @override
  String toString() => 'KeyData(timeStamp: $timeStamp, change: $change, physical: $physical, '
    'logical: $logical, character: $character, locks: $locks)';

  /// Returns a complete textual description of the information in this object.
  String toStringFull() {
    return '$runtimeType('
           ')';
  }
}
