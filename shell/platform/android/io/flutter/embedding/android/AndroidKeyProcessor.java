// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package io.flutter.embedding.android;

import android.app.Activity;
import android.content.Context;
import android.content.ContextWrapper;
import android.util.Log;
import android.view.KeyCharacterMap;
import android.view.KeyEvent;
import androidx.annotation.NonNull;
import androidx.annotation.Nullable;
import io.flutter.embedding.engine.systemchannels.KeyEventChannel;
import io.flutter.plugin.editing.TextInputPlugin;
import java.util.AbstractMap.SimpleImmutableEntry;
import java.util.ArrayDeque;
import java.util.Deque;
import java.util.Map.Entry;

/**
 * A class to process key events from Android, passing them to the framework as messages using
 * {@link KeyEventChannel}.
 */
public class AndroidKeyProcessor {
  private static final String TAG = "AndroidKeyProcessor";

  @NonNull private final KeyEventChannel keyEventChannel;
  @NonNull private final TextInputPlugin textInputPlugin;
  @NonNull private int combiningCharacter;
  @NonNull public final EventResponder eventResponder;

  public AndroidKeyProcessor(
      @NonNull Context context,
      @NonNull KeyEventChannel keyEventChannel,
      @NonNull TextInputPlugin textInputPlugin) {
    this.keyEventChannel = keyEventChannel;
    this.textInputPlugin = textInputPlugin;
    this.eventResponder = new EventResponder(context);
    this.keyEventChannel.setEventResponseHandler(eventResponder);
  }

  /**
   * Called when a key up event is received by the {@link FlutterView}.
   *
   * @param keyEvent the Android key event to respond to.
   * @return true if the key event was handled and should not be propagated.
   */
  public boolean onKeyUp(@NonNull KeyEvent keyEvent) {
    if (eventResponder.dispatchingKeyEvent) {
      // Don't handle it if it is from our own delayed event synthesis.
      return false;
    }

    Character complexCharacter = applyCombiningCharacterToBaseCharacter(keyEvent.getUnicodeChar());
    KeyEventChannel.FlutterKeyEvent flutterEvent =
        new KeyEventChannel.FlutterKeyEvent(keyEvent, complexCharacter);
    keyEventChannel.keyUp(flutterEvent);
    eventResponder.addEvent(flutterEvent.eventId, keyEvent);
    return true;
  }

  /**
   * Called when a key down event is received by the {@link FlutterView}.
   *
   * @param keyEvent the Android key event to respond to.
   * @return true if the key event was handled and should not be propagated.
   */
  public boolean onKeyDown(@NonNull KeyEvent keyEvent) {
    if (eventResponder.dispatchingKeyEvent) {
      // Don't handle it if it is from our own delayed event synthesis.
      return false;
    }

    // If the textInputPlugin is still valid and accepting text, then we'll try
    // and send the key event to it, assuming that if the event can be sent, that
    // it has been handled.
    if (textInputPlugin.getLastInputConnection() != null
        && textInputPlugin.getInputMethodManager().isAcceptingText()) {
      if (textInputPlugin.getLastInputConnection().sendKeyEvent(keyEvent)) {
        return true;
      }
    }

    Character complexCharacter = applyCombiningCharacterToBaseCharacter(keyEvent.getUnicodeChar());
    KeyEventChannel.FlutterKeyEvent flutterEvent =
        new KeyEventChannel.FlutterKeyEvent(keyEvent, complexCharacter);
    keyEventChannel.keyDown(flutterEvent);
    eventResponder.addEvent(flutterEvent.eventId, keyEvent);
    return true;
  }

  /**
   * Applies the given Unicode character in {@code newCharacterCodePoint} to a previously entered
   * Unicode combining character and returns the combination of these characters if a combination
   * exists.
   *
   * <p>This method mutates {@link #combiningCharacter} over time to combine characters.
   *
   * <p>One of the following things happens in this method:
   *
   * <ul>
   *   <li>If no previous {@link #combiningCharacter} exists and the {@code newCharacterCodePoint}
   *       is not a combining character, then {@code newCharacterCodePoint} is returned.
   *   <li>If no previous {@link #combiningCharacter} exists and the {@code newCharacterCodePoint}
   *       is a combining character, then {@code newCharacterCodePoint} is saved as the {@link
   *       #combiningCharacter} and null is returned.
   *   <li>If a previous {@link #combiningCharacter} exists and the {@code newCharacterCodePoint} is
   *       also a combining character, then the {@code newCharacterCodePoint} is combined with the
   *       existing {@link #combiningCharacter} and null is returned.
   *   <li>If a previous {@link #combiningCharacter} exists and the {@code newCharacterCodePoint} is
   *       not a combining character, then the {@link #combiningCharacter} is applied to the regular
   *       {@code newCharacterCodePoint} and the resulting complex character is returned. The {@link
   *       #combiningCharacter} is cleared.
   * </ul>
   *
   * <p>The following reference explains the concept of a "combining character":
   * https://en.wikipedia.org/wiki/Combining_character
   */
  @Nullable
  private Character applyCombiningCharacterToBaseCharacter(int newCharacterCodePoint) {
    if (newCharacterCodePoint == 0) {
      return null;
    }

    Character complexCharacter = (char) newCharacterCodePoint;
    boolean isNewCodePointACombiningCharacter =
        (newCharacterCodePoint & KeyCharacterMap.COMBINING_ACCENT) != 0;
    if (isNewCodePointACombiningCharacter) {
      // If a combining character was entered before, combine this one with that one.
      int plainCodePoint = newCharacterCodePoint & KeyCharacterMap.COMBINING_ACCENT_MASK;
      if (combiningCharacter != 0) {
        combiningCharacter = KeyCharacterMap.getDeadChar(combiningCharacter, plainCodePoint);
      } else {
        combiningCharacter = plainCodePoint;
      }
    } else {
      // The new character is a regular character. Apply combiningCharacter to it, if
      // it exists.
      if (combiningCharacter != 0) {
        int combinedChar = KeyCharacterMap.getDeadChar(combiningCharacter, newCharacterCodePoint);
        if (combinedChar > 0) {
          complexCharacter = (char) combinedChar;
        }
        combiningCharacter = 0;
      }
    }

    return complexCharacter;
  }

  public static class EventResponder implements KeyEventChannel.EventResponseHandler {
    // The maximum number of pending events that are held before starting to
    // complain.
    private static final long MAX_PENDING_EVENTS = 1000;
    private final Deque<Entry<Long, KeyEvent>> pendingEvents =
        new ArrayDeque<Entry<Long, KeyEvent>>();
    @NonNull private final Context context;
    public boolean dispatchingKeyEvent = false;

    public EventResponder(@NonNull Context context) {
      this.context = context;
    }

    /**
     * Removes the pending event with the given id from the cache of pending events.
     *
     * @param id the id of the event to be removed.
     */
    private KeyEvent removePendingEvent(@NonNull long id) {
      if (pendingEvents.getFirst().getKey() != id) {
        throw new AssertionError("Event response received out of order");
      }
      return pendingEvents.removeFirst().getValue();
    }

    /**
     * Called whenever the framework responds that a given key event was handled by the framework.
     *
     * @param id the event id of the event to be marked as being handled by the framework. Must not
     *     be null.
     */
    @Override
    public void onKeyEventHandled(@NonNull long id) {
      removePendingEvent(id);
    }

    /**
     * Called whenever the framework responds that a given key event wasn't handled by the
     * framework.
     *
     * @param id the event id of the event to be marked as not being handled by the framework. Must
     *     not be null.
     */
    @Override
    public void onKeyEventNotHandled(@NonNull long id) {
      KeyEvent pendingEvent = removePendingEvent(id);

      // Since the framework didn't handle it, dispatch the key again.
      Activity activity = getActivity(context);
      if (activity != null) {
        // Turn on dispatchingKeyEvent so that we don't dispatch to ourselves and
        // send it to the framework again.
        dispatchingKeyEvent = true;
        activity.dispatchKeyEvent(pendingEvent);
        dispatchingKeyEvent = false;
      }
    }

    /** Adds an Android key event with an id to the event responder to wait for a response. */
    public void addEvent(long id, @NonNull KeyEvent event) {
      pendingEvents.addLast(new SimpleImmutableEntry<Long, KeyEvent>(id, event));
      if (pendingEvents.size() > MAX_PENDING_EVENTS) {
        Log.e(
            TAG,
            "There are "
                + pendingEvents.size()
                + " keyboard events "
                + "that have not yet received a response. Are responses being sent?");
      }
    }

    /**
     * Gets the nearest ancestor Activity for the given Context.
     *
     * @param context the context to look in for the activity.
     * @return null if no Activity found.
     */
    private Activity getActivity(Context context) {
      if (context instanceof Activity) {
        return (Activity) context;
      }
      if (context instanceof ContextWrapper) {
        // Recurse up chain of base contexts until we find an Activity.
        return getActivity(((ContextWrapper) context).getBaseContext());
      }
      return null;
    }
  }
}
