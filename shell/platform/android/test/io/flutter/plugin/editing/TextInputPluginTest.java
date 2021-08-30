package io.flutter.plugin.editing;

import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertThrows;
import static org.junit.Assert.assertTrue;
import static org.mockito.AdditionalMatchers.aryEq;
import static org.mockito.AdditionalMatchers.gt;
import static org.mockito.Matchers.anyInt;
import static org.mockito.Mockito.any;
import static org.mockito.Mockito.atLeast;
import static org.mockito.Mockito.eq;
import static org.mockito.Mockito.isNull;
import static org.mockito.Mockito.mock;
import static org.mockito.Mockito.notNull;
import static org.mockito.Mockito.spy;
import static org.mockito.Mockito.times;
import static org.mockito.Mockito.verify;
import static org.mockito.Mockito.when;

import android.annotation.TargetApi;
import android.content.Context;
import android.content.res.AssetManager;
import android.graphics.Insets;
import android.graphics.Rect;
import android.os.Build;
import android.os.Bundle;
import android.provider.Settings;
import android.text.InputType;
import android.text.Selection;
import android.util.SparseArray;
import android.util.SparseIntArray;
import android.view.KeyEvent;
import android.view.View;
import android.view.ViewStructure;
import android.view.WindowInsets;
import android.view.WindowInsetsAnimation;
import android.view.autofill.AutofillManager;
import android.view.autofill.AutofillValue;
import android.view.inputmethod.CursorAnchorInfo;
import android.view.inputmethod.EditorInfo;
import android.view.inputmethod.InputConnection;
import android.view.inputmethod.InputMethodManager;
import android.view.inputmethod.InputMethodSubtype;
import io.flutter.embedding.android.FlutterView;
import io.flutter.embedding.android.KeyboardManager;
import io.flutter.embedding.engine.FlutterEngine;
import io.flutter.embedding.engine.FlutterJNI;
import io.flutter.embedding.engine.dart.DartExecutor;
import io.flutter.embedding.engine.loader.FlutterLoader;
import io.flutter.embedding.engine.renderer.FlutterRenderer;
import io.flutter.embedding.engine.systemchannels.TextInputChannel;
import io.flutter.embedding.engine.systemchannels.TextInputChannel.TextEditState;
import io.flutter.plugin.common.BinaryMessenger;
import io.flutter.plugin.common.JSONMethodCodec;
import io.flutter.plugin.common.MethodCall;
import io.flutter.plugin.platform.PlatformViewsController;
import java.nio.ByteBuffer;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import org.json.JSONArray;
import org.json.JSONException;
import org.json.JSONObject;
import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.mockito.ArgumentCaptor;
import org.mockito.Mock;
import org.mockito.MockitoAnnotations;
import org.robolectric.RobolectricTestRunner;
import org.robolectric.RuntimeEnvironment;
import org.robolectric.annotation.Config;
import org.robolectric.annotation.Implementation;
import org.robolectric.annotation.Implements;
import org.robolectric.shadow.api.Shadow;
import org.robolectric.shadows.ShadowAutofillManager;
import org.robolectric.shadows.ShadowBuild;
import org.robolectric.shadows.ShadowInputMethodManager;

@Config(
    manifest = Config.NONE,
    shadows = {TextInputPluginTest.TestImm.class, TextInputPluginTest.TestAfm.class})
@RunWith(RobolectricTestRunner.class)
public class TextInputPluginTest {
  @Mock FlutterJNI mockFlutterJni;
  @Mock FlutterLoader mockFlutterLoader;

  @Before
  public void setUp() {
    MockitoAnnotations.initMocks(this);
    when(mockFlutterJni.isAttached()).thenReturn(true);
  }

  // Verifies the method and arguments for a captured method call.
  private void verifyMethodCall(ByteBuffer buffer, String methodName, String[] expectedArgs)
      throws JSONException {
    buffer.rewind();
    MethodCall methodCall = JSONMethodCodec.INSTANCE.decodeMethodCall(buffer);
    assertEquals(methodName, methodCall.method);
    if (expectedArgs != null) {
      JSONArray args = methodCall.arguments();
      assertEquals(expectedArgs.length, args.length());
      for (int i = 0; i < args.length(); i++) {
        assertEquals(expectedArgs[i], args.get(i).toString());
      }
    }
  }

  private static void sendToBinaryMessageHandler(
      BinaryMessenger.BinaryMessageHandler binaryMessageHandler, String method, Object args) {
    MethodCall methodCall = new MethodCall(method, args);
    ByteBuffer encodedMethodCall = JSONMethodCodec.INSTANCE.encodeMethodCall(methodCall);
    binaryMessageHandler.onMessage(
        (ByteBuffer) encodedMethodCall.flip(), mock(BinaryMessenger.BinaryReply.class));
  }

  @Test
  public void textInputPlugin_RequestsReattachOnCreation() throws JSONException {
    // Initialize a general TextInputPlugin.
    InputMethodSubtype inputMethodSubtype = mock(InputMethodSubtype.class);
    TestImm testImm =
        Shadow.extract(
            RuntimeEnvironment.application.getSystemService(Context.INPUT_METHOD_SERVICE));
    testImm.setCurrentInputMethodSubtype(inputMethodSubtype);
    View testView = new View(RuntimeEnvironment.application);

    FlutterJNI mockFlutterJni = mock(FlutterJNI.class);
    DartExecutor dartExecutor = spy(new DartExecutor(mockFlutterJni, mock(AssetManager.class)));
    TextInputChannel textInputChannel = new TextInputChannel(dartExecutor);
    TextInputPlugin textInputPlugin =
        new TextInputPlugin(testView, textInputChannel, mock(PlatformViewsController.class));

    ArgumentCaptor<String> channelCaptor = ArgumentCaptor.forClass(String.class);
    ArgumentCaptor<ByteBuffer> bufferCaptor = ArgumentCaptor.forClass(ByteBuffer.class);

    verify(dartExecutor, times(1)).send(channelCaptor.capture(), bufferCaptor.capture(), isNull());
    assertEquals("flutter/textinput", channelCaptor.getValue());
    verifyMethodCall(bufferCaptor.getValue(), "TextInputClient.requestExistingInputState", null);
  }

  @Test
  public void setTextInputEditingState_doesNotInvokeUpdateEditingState() {
    // Initialize a general TextInputPlugin.
    InputMethodSubtype inputMethodSubtype = mock(InputMethodSubtype.class);
    TestImm testImm =
        Shadow.extract(
            RuntimeEnvironment.application.getSystemService(Context.INPUT_METHOD_SERVICE));
    testImm.setCurrentInputMethodSubtype(inputMethodSubtype);
    View testView = new View(RuntimeEnvironment.application);
    TextInputChannel textInputChannel = spy(new TextInputChannel(mock(DartExecutor.class)));
    TextInputPlugin textInputPlugin =
        new TextInputPlugin(testView, textInputChannel, mock(PlatformViewsController.class));
    textInputPlugin.setTextInputClient(
        0,
        new TextInputChannel.Configuration(
            false,
            false,
            true,
            true,
            false,
            TextInputChannel.TextCapitalization.NONE,
            null,
            null,
            null,
            null,
            null));

    textInputPlugin.setTextInputEditingState(
        testView, new TextInputChannel.TextEditState("initial input from framework", 0, 0, -1, -1));
    assertTrue(textInputPlugin.getEditable().toString().equals("initial input from framework"));

    verify(textInputChannel, times(0))
        .updateEditingState(anyInt(), any(), anyInt(), anyInt(), anyInt(), anyInt());

    textInputPlugin.setTextInputEditingState(
        testView,
        new TextInputChannel.TextEditState("more update from the framework", 1, 2, -1, -1));

    assertTrue(textInputPlugin.getEditable().toString().equals("more update from the framework"));
    verify(textInputChannel, times(0))
        .updateEditingState(anyInt(), any(), anyInt(), anyInt(), anyInt(), anyInt());
  }

  @Test
  public void setTextInputEditingState_doesNotInvokeUpdateEditingStateWithDeltas() {
    // Initialize a general TextInputPlugin.
    InputMethodSubtype inputMethodSubtype = mock(InputMethodSubtype.class);
    TestImm testImm =
        Shadow.extract(
            RuntimeEnvironment.application.getSystemService(Context.INPUT_METHOD_SERVICE));
    testImm.setCurrentInputMethodSubtype(inputMethodSubtype);
    View testView = new View(RuntimeEnvironment.application);
    TextInputChannel textInputChannel = spy(new TextInputChannel(mock(DartExecutor.class)));
    TextInputPlugin textInputPlugin =
        new TextInputPlugin(testView, textInputChannel, mock(PlatformViewsController.class));
    textInputPlugin.setTextInputClient(
        0,
        new TextInputChannel.Configuration(
            false,
            false,
            true,
            true,
            true, // Enable delta model.
            TextInputChannel.TextCapitalization.NONE,
            null,
            null,
            null,
            null,
            null));

    textInputPlugin.setTextInputEditingState(
        testView,
        new TextInputChannel.TextEditState("receiving initial input from framework", 0, 0, -1, -1));
    assertTrue(
        textInputPlugin.getEditable().toString().equals("receiving initial input from framework"));

    verify(textInputChannel, times(0)).updateEditingStateWithDeltas(anyInt(), any());

    textInputPlugin.setTextInputEditingState(
        testView,
        new TextInputChannel.TextEditState(
            "receiving more updates from the framework", 1, 2, -1, -1));

    assertTrue(
        textInputPlugin
            .getEditable()
            .toString()
            .equals("receiving more updates from the framework"));
    verify(textInputChannel, times(0)).updateEditingStateWithDeltas(anyInt(), any());
  }

  @Test
  public void textEditingDelta_TestInsertionDeltaIsCreatedWhenComposingTextSet()
      throws NullPointerException {
    // Initialize a general TextInputPlugin.
    InputMethodSubtype inputMethodSubtype = mock(InputMethodSubtype.class);
    TestImm testImm =
        Shadow.extract(
            RuntimeEnvironment.application.getSystemService(Context.INPUT_METHOD_SERVICE));
    testImm.setCurrentInputMethodSubtype(inputMethodSubtype);
    View testView = new View(RuntimeEnvironment.application);
    EditorInfo outAttrs = new EditorInfo();
    outAttrs.inputType = InputType.TYPE_CLASS_TEXT | InputType.TYPE_TEXT_FLAG_MULTI_LINE;
    TextInputChannel textInputChannel = spy(new TextInputChannel(mock(DartExecutor.class)));
    TextInputPlugin textInputPlugin =
        new TextInputPlugin(testView, textInputChannel, mock(PlatformViewsController.class));
    CharSequence newText = "I do not fear computers. I fear the lack of them.";
    final TextEditingDelta expectedDelta =
        new TextEditingDelta(
            "",
            newText,
            "TextEditingDeltaType.insertion",
            0,
            0,
            newText.length(),
            newText.length(),
            0,
            49);

    // Change InputTarget to FRAMEWORK_CLIENT.
    textInputPlugin.setTextInputClient(
        0,
        new TextInputChannel.Configuration(
            false,
            false,
            true,
            true,
            true, // Enable delta model.
            TextInputChannel.TextCapitalization.NONE,
            new TextInputChannel.InputType(TextInputChannel.TextInputType.TEXT, false, false),
            null,
            null,
            null,
            null));

    // There's a pending restart since we initialized the text input client. Flush that now.
    textInputPlugin.setTextInputEditingState(
        testView, new TextInputChannel.TextEditState("", 0, 0, -1, -1));
    verify(textInputChannel, times(0)).updateEditingStateWithDeltas(anyInt(), any());
    assertEquals(
        "TextEditingDeltaType.equality",
        ((ListenableEditingState) textInputPlugin.getEditable())
            .getBatchTextEditingDeltas()
            .get(0)
            .getDeltaType());
    assertEquals(
        1,
        ((ListenableEditingState) textInputPlugin.getEditable())
            .getBatchTextEditingDeltas()
            .size());

    InputConnection inputConnection =
        textInputPlugin.createInputConnection(testView, mock(KeyboardManager.class), outAttrs);

    inputConnection.beginBatchEdit();
    verify(textInputChannel, times(0)).updateEditingStateWithDeltas(anyInt(), any());
    inputConnection.setComposingText(newText, newText.length());
    assertEquals(
        2,
        ((ListenableEditingState) textInputPlugin.getEditable())
            .getBatchTextEditingDeltas()
            .size());
    final ArrayList<TextEditingDelta> actualDeltas =
        ((ListenableEditingState) textInputPlugin.getEditable()).getBatchTextEditingDeltas();
    final TextEditingDelta delta = actualDeltas.get(1);
    verify(textInputChannel, times(0)).updateEditingStateWithDeltas(anyInt(), any());
    inputConnection.endBatchEdit();

    assertEquals(
        0,
        ((ListenableEditingState) textInputPlugin.getEditable())
            .getBatchTextEditingDeltas()
            .size());

    // Verify delta is what we expect.
    assertEquals(expectedDelta.getDeltaType(), delta.getDeltaType());
    assertEquals(expectedDelta.getOldText(), delta.getOldText());
    assertEquals(expectedDelta.getDeltaText(), delta.getDeltaText());
    assertEquals(expectedDelta.getDeltaStart(), delta.getDeltaStart());
    assertEquals(expectedDelta.getDeltaEnd(), delta.getDeltaEnd());
    assertEquals(expectedDelta.getNewSelectionStart(), delta.getNewSelectionStart());
    assertEquals(expectedDelta.getNewSelectionEnd(), delta.getNewSelectionEnd());
    assertEquals(expectedDelta.getNewComposingStart(), delta.getNewComposingStart());
    assertEquals(expectedDelta.getNewComposingEnd(), delta.getNewComposingEnd());

    verify(textInputChannel, times(1)).updateEditingStateWithDeltas(anyInt(), any());

    inputConnection.beginBatchEdit();

    verify(textInputChannel, times(1)).updateEditingStateWithDeltas(anyInt(), any());

    inputConnection.endBatchEdit();

    assertEquals(
        0,
        ((ListenableEditingState) textInputPlugin.getEditable())
            .getBatchTextEditingDeltas()
            .size());

    verify(textInputChannel, times(1)).updateEditingStateWithDeltas(anyInt(), any());

    inputConnection.beginBatchEdit();

    verify(textInputChannel, times(1)).updateEditingStateWithDeltas(anyInt(), any());

    // Selection changes so this will trigger an update to the framework through
    // updateEditingStateWithDeltas after the batch edit has completed and notified all listeners
    // of the editing state.
    inputConnection.setSelection(3, 4);
    assertEquals(Selection.getSelectionStart(textInputPlugin.getEditable()), 3);
    assertEquals(Selection.getSelectionEnd(textInputPlugin.getEditable()), 4);

    verify(textInputChannel, times(1)).updateEditingStateWithDeltas(anyInt(), any());

    verify(textInputChannel, times(1)).updateEditingStateWithDeltas(anyInt(), any());

    inputConnection.endBatchEdit();

    verify(textInputChannel, times(2)).updateEditingStateWithDeltas(anyInt(), any());
  }

  @Test
  public void textEditingDelta_TestDeletionDeltaIsCreatedWhenComposingTextSet()
      throws NullPointerException {
    // Initialize a general TextInputPlugin.
    InputMethodSubtype inputMethodSubtype = mock(InputMethodSubtype.class);
    TestImm testImm =
        Shadow.extract(
            RuntimeEnvironment.application.getSystemService(Context.INPUT_METHOD_SERVICE));
    testImm.setCurrentInputMethodSubtype(inputMethodSubtype);
    View testView = new View(RuntimeEnvironment.application);
    EditorInfo outAttrs = new EditorInfo();
    outAttrs.inputType = InputType.TYPE_CLASS_TEXT | InputType.TYPE_TEXT_FLAG_MULTI_LINE;
    TextInputChannel textInputChannel = spy(new TextInputChannel(mock(DartExecutor.class)));
    TextInputPlugin textInputPlugin =
        new TextInputPlugin(testView, textInputChannel, mock(PlatformViewsController.class));
    CharSequence newText = "I do not fear computers. I fear the lack of them.";
    final TextEditingDelta expectedDelta =
        new TextEditingDelta(newText, ".", "TextEditingDeltaType.deletion", 49, 49, 48, 48, 0, 48);

    // Change InputTarget to FRAMEWORK_CLIENT.
    textInputPlugin.setTextInputClient(
        0,
        new TextInputChannel.Configuration(
            false,
            false,
            true,
            true,
            true, // Enable delta model.
            TextInputChannel.TextCapitalization.NONE,
            new TextInputChannel.InputType(TextInputChannel.TextInputType.TEXT, false, false),
            null,
            null,
            null,
            null));

    // There's a pending restart since we initialized the text input client. Flush that now.
    textInputPlugin.setTextInputEditingState(
        testView, new TextInputChannel.TextEditState(newText.toString(), 49, 49, 0, 49));
    verify(textInputChannel, times(0)).updateEditingStateWithDeltas(anyInt(), any());
    assertEquals(
        "TextEditingDeltaType.equality",
        ((ListenableEditingState) textInputPlugin.getEditable())
            .getBatchTextEditingDeltas()
            .get(0)
            .getDeltaType());
    assertEquals(
        1,
        ((ListenableEditingState) textInputPlugin.getEditable())
            .getBatchTextEditingDeltas()
            .size());

    InputConnection inputConnection =
        textInputPlugin.createInputConnection(testView, mock(KeyboardManager.class), outAttrs);

    inputConnection.beginBatchEdit();
    verify(textInputChannel, times(0)).updateEditingStateWithDeltas(anyInt(), any());
    inputConnection.setComposingText("I do not fear computers. I fear the lack of them", 48);
    final ArrayList<TextEditingDelta> actualDeltas =
        ((ListenableEditingState) textInputPlugin.getEditable()).getBatchTextEditingDeltas();
    final TextEditingDelta delta = actualDeltas.get(1);
    System.out.println(delta.getDeltaText());
    verify(textInputChannel, times(0)).updateEditingStateWithDeltas(anyInt(), any());
    inputConnection.endBatchEdit();

    assertEquals(
        0,
        ((ListenableEditingState) textInputPlugin.getEditable())
            .getBatchTextEditingDeltas()
            .size());

    // Verify delta is what we expect.
    assertEquals(expectedDelta.getDeltaType(), delta.getDeltaType());
    assertEquals(expectedDelta.getOldText(), delta.getOldText());
    assertEquals(expectedDelta.getDeltaText(), delta.getDeltaText());
    assertEquals(expectedDelta.getDeltaStart(), delta.getDeltaStart());
    assertEquals(expectedDelta.getDeltaEnd(), delta.getDeltaEnd());
    assertEquals(expectedDelta.getNewSelectionStart(), delta.getNewSelectionStart());
    assertEquals(expectedDelta.getNewSelectionEnd(), delta.getNewSelectionEnd());
    assertEquals(expectedDelta.getNewComposingStart(), delta.getNewComposingStart());
    assertEquals(expectedDelta.getNewComposingEnd(), delta.getNewComposingEnd());

    verify(textInputChannel, times(1)).updateEditingStateWithDeltas(anyInt(), any());

    inputConnection.beginBatchEdit();

    verify(textInputChannel, times(1)).updateEditingStateWithDeltas(anyInt(), any());

    inputConnection.endBatchEdit();

    assertEquals(
        0,
        ((ListenableEditingState) textInputPlugin.getEditable())
            .getBatchTextEditingDeltas()
            .size());

    verify(textInputChannel, times(1)).updateEditingStateWithDeltas(anyInt(), any());

    inputConnection.beginBatchEdit();

    verify(textInputChannel, times(1)).updateEditingStateWithDeltas(anyInt(), any());

    // Selection changes so this will trigger an update to the framework through
    // updateEditingStateWithDeltas after the batch edit has completed and notified all listeners
    // of the editing state.
    inputConnection.setSelection(3, 4);
    assertEquals(Selection.getSelectionStart(textInputPlugin.getEditable()), 3);
    assertEquals(Selection.getSelectionEnd(textInputPlugin.getEditable()), 4);

    verify(textInputChannel, times(1)).updateEditingStateWithDeltas(anyInt(), any());

    verify(textInputChannel, times(1)).updateEditingStateWithDeltas(anyInt(), any());

    inputConnection.endBatchEdit();

    verify(textInputChannel, times(2)).updateEditingStateWithDeltas(anyInt(), any());
  }

  @Test
  public void textEditingDelta_TestReplacementDeltaIsCreatedWhenComposingTextSet()
      throws NullPointerException {
    // Initialize a general TextInputPlugin.
    InputMethodSubtype inputMethodSubtype = mock(InputMethodSubtype.class);
    TestImm testImm =
        Shadow.extract(
            RuntimeEnvironment.application.getSystemService(Context.INPUT_METHOD_SERVICE));
    testImm.setCurrentInputMethodSubtype(inputMethodSubtype);
    View testView = new View(RuntimeEnvironment.application);
    EditorInfo outAttrs = new EditorInfo();
    outAttrs.inputType = InputType.TYPE_CLASS_TEXT | InputType.TYPE_TEXT_FLAG_MULTI_LINE;
    TextInputChannel textInputChannel = spy(new TextInputChannel(mock(DartExecutor.class)));
    TextInputPlugin textInputPlugin =
        new TextInputPlugin(testView, textInputChannel, mock(PlatformViewsController.class));
    CharSequence newText = "helfo";
    final TextEditingDelta expectedDelta =
        new TextEditingDelta(newText, "hello", "TextEditingDeltaType.replacement", 0, 5, 5, 5, 0, 5);

    // Change InputTarget to FRAMEWORK_CLIENT.
    textInputPlugin.setTextInputClient(
        0,
        new TextInputChannel.Configuration(
            false,
            false,
            true,
            true,
            true, // Enable delta model.
            TextInputChannel.TextCapitalization.NONE,
            new TextInputChannel.InputType(TextInputChannel.TextInputType.TEXT, false, false),
            null,
            null,
            null,
            null));

    // There's a pending restart since we initialized the text input client. Flush that now.
    textInputPlugin.setTextInputEditingState(
        testView, new TextInputChannel.TextEditState(newText.toString(), 5, 5, 0, 5));
    verify(textInputChannel, times(0)).updateEditingStateWithDeltas(anyInt(), any());
    assertEquals(
        "TextEditingDeltaType.equality",
        ((ListenableEditingState) textInputPlugin.getEditable())
            .getBatchTextEditingDeltas()
            .get(0)
            .getDeltaType());
    assertEquals(
        1,
        ((ListenableEditingState) textInputPlugin.getEditable())
            .getBatchTextEditingDeltas()
            .size());

    InputConnection inputConnection =
        textInputPlugin.createInputConnection(testView, mock(KeyboardManager.class), outAttrs);

    inputConnection.beginBatchEdit();
    verify(textInputChannel, times(0)).updateEditingStateWithDeltas(anyInt(), any());
    inputConnection.setComposingText("hello", 5);
    final ArrayList<TextEditingDelta> actualDeltas =
        ((ListenableEditingState) textInputPlugin.getEditable()).getBatchTextEditingDeltas();
    final TextEditingDelta delta = actualDeltas.get(1);
    System.out.println(delta.getDeltaText());
    verify(textInputChannel, times(0)).updateEditingStateWithDeltas(anyInt(), any());
    inputConnection.endBatchEdit();

    assertEquals(
        0,
        ((ListenableEditingState) textInputPlugin.getEditable())
            .getBatchTextEditingDeltas()
            .size());

    // Verify delta is what we expect.
    assertEquals(expectedDelta.getDeltaType(), delta.getDeltaType());
    assertEquals(expectedDelta.getOldText(), delta.getOldText());
    assertEquals(expectedDelta.getDeltaText(), delta.getDeltaText());
    assertEquals(expectedDelta.getDeltaStart(), delta.getDeltaStart());
    assertEquals(expectedDelta.getDeltaEnd(), delta.getDeltaEnd());
    assertEquals(expectedDelta.getNewSelectionStart(), delta.getNewSelectionStart());
    assertEquals(expectedDelta.getNewSelectionEnd(), delta.getNewSelectionEnd());
    assertEquals(expectedDelta.getNewComposingStart(), delta.getNewComposingStart());
    assertEquals(expectedDelta.getNewComposingEnd(), delta.getNewComposingEnd());

    verify(textInputChannel, times(1)).updateEditingStateWithDeltas(anyInt(), any());

    inputConnection.beginBatchEdit();

    verify(textInputChannel, times(1)).updateEditingStateWithDeltas(anyInt(), any());

    inputConnection.endBatchEdit();

    assertEquals(
        0,
        ((ListenableEditingState) textInputPlugin.getEditable())
            .getBatchTextEditingDeltas()
            .size());

    verify(textInputChannel, times(1)).updateEditingStateWithDeltas(anyInt(), any());

    inputConnection.beginBatchEdit();

    verify(textInputChannel, times(1)).updateEditingStateWithDeltas(anyInt(), any());

    // Selection changes so this will trigger an update to the framework through
    // updateEditingStateWithDeltas after the batch edit has completed and notified all listeners
    // of the editing state.
    inputConnection.setSelection(3, 4);
    assertEquals(Selection.getSelectionStart(textInputPlugin.getEditable()), 3);
    assertEquals(Selection.getSelectionEnd(textInputPlugin.getEditable()), 4);

    verify(textInputChannel, times(1)).updateEditingStateWithDeltas(anyInt(), any());

    verify(textInputChannel, times(1)).updateEditingStateWithDeltas(anyInt(), any());

    inputConnection.endBatchEdit();

    verify(textInputChannel, times(2)).updateEditingStateWithDeltas(anyInt(), any());
  }

  @Test
  public void inputConnectionAdaptor_RepeatFilter() throws NullPointerException {
    // Initialize a general TextInputPlugin.
    InputMethodSubtype inputMethodSubtype = mock(InputMethodSubtype.class);
    TestImm testImm =
        Shadow.extract(
            RuntimeEnvironment.application.getSystemService(Context.INPUT_METHOD_SERVICE));
    testImm.setCurrentInputMethodSubtype(inputMethodSubtype);
    View testView = new View(RuntimeEnvironment.application);
    EditorInfo outAttrs = new EditorInfo();
    outAttrs.inputType = InputType.TYPE_CLASS_TEXT | InputType.TYPE_TEXT_FLAG_MULTI_LINE;
    TextInputChannel textInputChannel = spy(new TextInputChannel(mock(DartExecutor.class)));
    TextInputPlugin textInputPlugin =
        new TextInputPlugin(testView, textInputChannel, mock(PlatformViewsController.class));

    // Change InputTarget to FRAMEWORK_CLIENT.
    textInputPlugin.setTextInputClient(
        0,
        new TextInputChannel.Configuration(
            false,
            false,
            true,
            true,
            false,
            TextInputChannel.TextCapitalization.NONE,
            new TextInputChannel.InputType(TextInputChannel.TextInputType.TEXT, false, false),
            null,
            null,
            null,
            null));

    // There's a pending restart since we initialized the text input client. Flush that now.
    textInputPlugin.setTextInputEditingState(
        testView, new TextInputChannel.TextEditState("", 0, 0, -1, -1));
    verify(textInputChannel, times(0))
        .updateEditingState(anyInt(), any(), anyInt(), anyInt(), anyInt(), anyInt());

    InputConnectionAdaptor inputConnectionAdaptor =
        (InputConnectionAdaptor)
            textInputPlugin.createInputConnection(testView, mock(KeyboardManager.class), outAttrs);

    inputConnectionAdaptor.beginBatchEdit();
    verify(textInputChannel, times(0))
        .updateEditingState(anyInt(), any(), anyInt(), anyInt(), anyInt(), anyInt());
    inputConnectionAdaptor.setComposingText("I do not fear computers. I fear the lack of them.", 1);
    verify(textInputChannel, times(0))
        .updateEditingState(anyInt(), any(), anyInt(), anyInt(), anyInt(), anyInt());
    inputConnectionAdaptor.endBatchEdit();
    verify(textInputChannel, times(1))
        .updateEditingState(
            anyInt(),
            eq("I do not fear computers. I fear the lack of them."),
            eq(49),
            eq(49),
            eq(0),
            eq(49));

    inputConnectionAdaptor.beginBatchEdit();

    verify(textInputChannel, times(1))
        .updateEditingState(anyInt(), any(), anyInt(), anyInt(), anyInt(), anyInt());

    inputConnectionAdaptor.endBatchEdit();

    verify(textInputChannel, times(1))
        .updateEditingState(anyInt(), any(), anyInt(), anyInt(), anyInt(), anyInt());

    inputConnectionAdaptor.beginBatchEdit();

    verify(textInputChannel, times(1))
        .updateEditingState(anyInt(), any(), anyInt(), anyInt(), anyInt(), anyInt());

    inputConnectionAdaptor.setSelection(3, 4);
    assertEquals(Selection.getSelectionStart(textInputPlugin.getEditable()), 3);
    assertEquals(Selection.getSelectionEnd(textInputPlugin.getEditable()), 4);

    verify(textInputChannel, times(1))
        .updateEditingState(anyInt(), any(), anyInt(), anyInt(), anyInt(), anyInt());

    verify(textInputChannel, times(1))
        .updateEditingState(anyInt(), any(), anyInt(), anyInt(), anyInt(), anyInt());

    inputConnectionAdaptor.endBatchEdit();

    verify(textInputChannel, times(1))
        .updateEditingState(
            anyInt(),
            eq("I do not fear computers. I fear the lack of them."),
            eq(3),
            eq(4),
            eq(0),
            eq(49));
  }

  @Test
  public void setTextInputEditingState_doesNotRestartWhenTextIsIdentical() {
    // Initialize a general TextInputPlugin.
    InputMethodSubtype inputMethodSubtype = mock(InputMethodSubtype.class);
    TestImm testImm =
        Shadow.extract(
            RuntimeEnvironment.application.getSystemService(Context.INPUT_METHOD_SERVICE));
    testImm.setCurrentInputMethodSubtype(inputMethodSubtype);
    View testView = new View(RuntimeEnvironment.application);
    TextInputChannel textInputChannel = new TextInputChannel(mock(DartExecutor.class));
    TextInputPlugin textInputPlugin =
        new TextInputPlugin(testView, textInputChannel, mock(PlatformViewsController.class));
    textInputPlugin.setTextInputClient(
        0,
        new TextInputChannel.Configuration(
            false,
            false,
            true,
            true,
            false,
            TextInputChannel.TextCapitalization.NONE,
            null,
            null,
            null,
            null,
            null));
    // There's a pending restart since we initialized the text input client. Flush that now.
    textInputPlugin.setTextInputEditingState(
        testView, new TextInputChannel.TextEditState("", 0, 0, -1, -1));

    // Move the cursor.
    assertEquals(1, testImm.getRestartCount(testView));
    textInputPlugin.setTextInputEditingState(
        testView, new TextInputChannel.TextEditState("", 0, 0, -1, -1));

    // Verify that we haven't restarted the input.
    assertEquals(1, testImm.getRestartCount(testView));
  }

  @Test
  public void setTextInputEditingState_alwaysSetEditableWhenDifferent() {
    // Initialize a general TextInputPlugin.
    InputMethodSubtype inputMethodSubtype = mock(InputMethodSubtype.class);
    TestImm testImm =
        Shadow.extract(
            RuntimeEnvironment.application.getSystemService(Context.INPUT_METHOD_SERVICE));
    testImm.setCurrentInputMethodSubtype(inputMethodSubtype);
    View testView = new View(RuntimeEnvironment.application);
    TextInputChannel textInputChannel = new TextInputChannel(mock(DartExecutor.class));
    TextInputPlugin textInputPlugin =
        new TextInputPlugin(testView, textInputChannel, mock(PlatformViewsController.class));
    textInputPlugin.setTextInputClient(
        0,
        new TextInputChannel.Configuration(
            false,
            false,
            true,
            true,
            false,
            TextInputChannel.TextCapitalization.NONE,
            null,
            null,
            null,
            null,
            null));
    // There's a pending restart since we initialized the text input client. Flush that now. With
    // changed text, we should
    // always set the Editable contents.
    textInputPlugin.setTextInputEditingState(
        testView, new TextInputChannel.TextEditState("hello", 0, 0, -1, -1));
    assertEquals(1, testImm.getRestartCount(testView));
    assertTrue(textInputPlugin.getEditable().toString().equals("hello"));

    // No pending restart, set Editable contents anyways.
    textInputPlugin.setTextInputEditingState(
        testView, new TextInputChannel.TextEditState("Shibuyawoo", 0, 0, -1, -1));
    assertEquals(1, testImm.getRestartCount(testView));
    assertTrue(textInputPlugin.getEditable().toString().equals("Shibuyawoo"));
  }

  // See https://github.com/flutter/flutter/issues/29341 and
  // https://github.com/flutter/flutter/issues/31512
  // All modern Samsung keybords are affected including non-korean languages and thus
  // need the restart.
  // Update: many other keyboards need this too:
  // https://github.com/flutter/flutter/issues/78827
  @Test
  public void setTextInputEditingState_restartsIMEOnlyWhenFrameworkChangesComposingRegion() {
    // Initialize a TextInputPlugin that needs to be always restarted.
    InputMethodSubtype inputMethodSubtype =
        new InputMethodSubtype(0, 0, /*locale=*/ "en", "", "", false, false);
    TestImm testImm =
        Shadow.extract(
            RuntimeEnvironment.application.getSystemService(Context.INPUT_METHOD_SERVICE));
    testImm.setCurrentInputMethodSubtype(inputMethodSubtype);
    View testView = new View(RuntimeEnvironment.application);
    TextInputChannel textInputChannel = new TextInputChannel(mock(DartExecutor.class));
    TextInputPlugin textInputPlugin =
        new TextInputPlugin(testView, textInputChannel, mock(PlatformViewsController.class));
    textInputPlugin.setTextInputClient(
        0,
        new TextInputChannel.Configuration(
            false,
            false,
            true,
            true,
            false,
            TextInputChannel.TextCapitalization.NONE,
            new TextInputChannel.InputType(TextInputChannel.TextInputType.TEXT, false, false),
            null,
            null,
            null,
            null));
    // There's a pending restart since we initialized the text input client. Flush that now.
    textInputPlugin.setTextInputEditingState(
        testView, new TextInputChannel.TextEditState("", 0, 0, -1, -1));
    assertEquals(1, testImm.getRestartCount(testView));
    InputConnection connection =
        textInputPlugin.createInputConnection(
            testView, mock(KeyboardManager.class), new EditorInfo());
    connection.setComposingText("POWERRRRR", 1);

    textInputPlugin.setTextInputEditingState(
        testView, new TextInputChannel.TextEditState("UNLIMITED POWERRRRR", 0, 0, 10, 19));
    // Does not restart since the composing text is not changed.
    assertEquals(1, testImm.getRestartCount(testView));

    connection.finishComposingText();
    // Does not restart since the composing text is committed by the IME.
    assertEquals(1, testImm.getRestartCount(testView));

    // Does not restart since the composing text is changed by the IME.
    connection.setComposingText("POWERRRRR", 1);
    assertEquals(1, testImm.getRestartCount(testView));

    // The framework tries to commit the composing region.
    textInputPlugin.setTextInputEditingState(
        testView, new TextInputChannel.TextEditState("POWERRRRR", 0, 0, -1, -1));

    // Verify that we've restarted the input.
    assertEquals(2, testImm.getRestartCount(testView));
  }

  @Test
  public void TextEditState_throwsOnInvalidStatesReceived() {
    // Index OOB:
    assertThrows(IndexOutOfBoundsException.class, () -> new TextEditState("", 0, -9, -1, -1));
    assertThrows(IndexOutOfBoundsException.class, () -> new TextEditState("", -9, 0, -1, -1));
    assertThrows(IndexOutOfBoundsException.class, () -> new TextEditState("", 0, 1, -1, -1));
    assertThrows(IndexOutOfBoundsException.class, () -> new TextEditState("", 1, 0, -1, -1));
    assertThrows(IndexOutOfBoundsException.class, () -> new TextEditState("Text", 0, 0, 1, 5));
    assertThrows(IndexOutOfBoundsException.class, () -> new TextEditState("Text", 0, 0, 5, 1));
    assertThrows(IndexOutOfBoundsException.class, () -> new TextEditState("Text", 0, 0, 5, 5));

    // Invalid Selections:
    assertThrows(IndexOutOfBoundsException.class, () -> new TextEditState("", -1, -2, -1, -1));
    assertThrows(IndexOutOfBoundsException.class, () -> new TextEditState("", -2, -1, -1, -1));
    assertThrows(IndexOutOfBoundsException.class, () -> new TextEditState("", -9, -9, -1, -1));

    // Invalid Composing Ranges:
    assertThrows(IndexOutOfBoundsException.class, () -> new TextEditState("Text", 0, 0, -9, -1));
    assertThrows(IndexOutOfBoundsException.class, () -> new TextEditState("Text", 0, 0, -1, -9));
    assertThrows(IndexOutOfBoundsException.class, () -> new TextEditState("Text", 0, 0, -9, -9));
    assertThrows(IndexOutOfBoundsException.class, () -> new TextEditState("Text", 0, 0, 2, 1));

    // Valid values (does not throw):
    // Nothing selected/composing:
    TextEditState state = new TextEditState("", -1, -1, -1, -1);
    assertEquals("", state.text);
    assertEquals(-1, state.selectionStart);
    assertEquals(-1, state.selectionEnd);
    assertEquals(-1, state.composingStart);
    assertEquals(-1, state.composingEnd);
    // Collapsed selection.
    state = new TextEditState("x", 0, 0, 0, 1);
    assertEquals(0, state.selectionStart);
    assertEquals(0, state.selectionEnd);
    // Reversed Selection.
    state = new TextEditState("REEEE", 4, 2, -1, -1);
    assertEquals(4, state.selectionStart);
    assertEquals(2, state.selectionEnd);
    // A collapsed selection and composing range.
    state = new TextEditState("text", 0, 0, 0, 0);
    assertEquals("text", state.text);
    assertEquals(0, state.selectionStart);
    assertEquals(0, state.selectionEnd);
    assertEquals(0, state.composingStart);
    assertEquals(0, state.composingEnd);
  }

  @Test
  public void setTextInputEditingState_nullInputMethodSubtype() {
    TestImm testImm =
        Shadow.extract(
            RuntimeEnvironment.application.getSystemService(Context.INPUT_METHOD_SERVICE));
    testImm.setCurrentInputMethodSubtype(null);

    View testView = new View(RuntimeEnvironment.application);
    TextInputChannel textInputChannel = new TextInputChannel(mock(DartExecutor.class));
    TextInputPlugin textInputPlugin =
        new TextInputPlugin(testView, textInputChannel, mock(PlatformViewsController.class));
    textInputPlugin.setTextInputClient(
        0,
        new TextInputChannel.Configuration(
            false,
            false,
            true,
            true,
            false,
            TextInputChannel.TextCapitalization.NONE,
            null,
            null,
            null,
            null,
            null));
    // There's a pending restart since we initialized the text input client. Flush that now.
    textInputPlugin.setTextInputEditingState(
        testView, new TextInputChannel.TextEditState("", 0, 0, -1, -1));
    assertEquals(1, testImm.getRestartCount(testView));
  }

  @Test
  public void destroy_clearTextInputMethodHandler() {
    View testView = new View(RuntimeEnvironment.application);
    TextInputChannel textInputChannel = spy(new TextInputChannel(mock(DartExecutor.class)));
    TextInputPlugin textInputPlugin =
        new TextInputPlugin(testView, textInputChannel, mock(PlatformViewsController.class));
    verify(textInputChannel, times(1))
        .setTextInputMethodHandler(notNull(TextInputChannel.TextInputMethodHandler.class));
    textInputPlugin.destroy();
    verify(textInputChannel, times(1))
        .setTextInputMethodHandler(isNull(TextInputChannel.TextInputMethodHandler.class));
  }

  @Test
  public void inputConnection_createsActionFromEnter() throws JSONException {
    TestImm testImm =
        Shadow.extract(
            RuntimeEnvironment.application.getSystemService(Context.INPUT_METHOD_SERVICE));
    FlutterJNI mockFlutterJni = mock(FlutterJNI.class);
    View testView = new View(RuntimeEnvironment.application);
    DartExecutor dartExecutor = spy(new DartExecutor(mockFlutterJni, mock(AssetManager.class)));
    TextInputChannel textInputChannel = new TextInputChannel(dartExecutor);
    TextInputPlugin textInputPlugin =
        new TextInputPlugin(testView, textInputChannel, mock(PlatformViewsController.class));
    textInputPlugin.setTextInputClient(
        0,
        new TextInputChannel.Configuration(
            false,
            false,
            true,
            true,
            false,
            TextInputChannel.TextCapitalization.NONE,
            new TextInputChannel.InputType(TextInputChannel.TextInputType.TEXT, false, false),
            null,
            null,
            null,
            null));
    // There's a pending restart since we initialized the text input client. Flush that now.
    textInputPlugin.setTextInputEditingState(
        testView, new TextInputChannel.TextEditState("", 0, 0, -1, -1));

    ArgumentCaptor<String> channelCaptor = ArgumentCaptor.forClass(String.class);
    ArgumentCaptor<ByteBuffer> bufferCaptor = ArgumentCaptor.forClass(ByteBuffer.class);
    verify(dartExecutor, times(1)).send(channelCaptor.capture(), bufferCaptor.capture(), isNull());
    assertEquals("flutter/textinput", channelCaptor.getValue());
    verifyMethodCall(bufferCaptor.getValue(), "TextInputClient.requestExistingInputState", null);
    InputConnectionAdaptor connection =
        (InputConnectionAdaptor)
            textInputPlugin.createInputConnection(
                testView, mock(KeyboardManager.class), new EditorInfo());

    connection.handleKeyEvent(new KeyEvent(KeyEvent.ACTION_DOWN, KeyEvent.KEYCODE_ENTER));
    verify(dartExecutor, times(2)).send(channelCaptor.capture(), bufferCaptor.capture(), isNull());
    assertEquals("flutter/textinput", channelCaptor.getValue());
    verifyMethodCall(
        bufferCaptor.getValue(),
        "TextInputClient.performAction",
        new String[] {"0", "TextInputAction.done"});
    connection.handleKeyEvent(new KeyEvent(KeyEvent.ACTION_UP, KeyEvent.KEYCODE_ENTER));

    connection.handleKeyEvent(new KeyEvent(KeyEvent.ACTION_DOWN, KeyEvent.KEYCODE_NUMPAD_ENTER));
    verify(dartExecutor, times(3)).send(channelCaptor.capture(), bufferCaptor.capture(), isNull());
    assertEquals("flutter/textinput", channelCaptor.getValue());
    verifyMethodCall(
        bufferCaptor.getValue(),
        "TextInputClient.performAction",
        new String[] {"0", "TextInputAction.done"});
  }

  @Test
  public void inputConnection_finishComposingTextUpdatesIMM() throws JSONException {
    if (Build.VERSION.SDK_INT < Build.VERSION_CODES.LOLLIPOP) {
      return;
    }
    ShadowBuild.setManufacturer("samsung");
    InputMethodSubtype inputMethodSubtype =
        new InputMethodSubtype(0, 0, /*locale=*/ "en", "", "", false, false);
    Settings.Secure.putString(
        RuntimeEnvironment.application.getContentResolver(),
        Settings.Secure.DEFAULT_INPUT_METHOD,
        "com.sec.android.inputmethod/.SamsungKeypad");
    TestImm testImm =
        Shadow.extract(
            RuntimeEnvironment.application.getSystemService(Context.INPUT_METHOD_SERVICE));
    testImm.setCurrentInputMethodSubtype(inputMethodSubtype);
    FlutterJNI mockFlutterJni = mock(FlutterJNI.class);
    View testView = new View(RuntimeEnvironment.application);
    DartExecutor dartExecutor = spy(new DartExecutor(mockFlutterJni, mock(AssetManager.class)));
    TextInputChannel textInputChannel = new TextInputChannel(dartExecutor);
    TextInputPlugin textInputPlugin =
        new TextInputPlugin(testView, textInputChannel, mock(PlatformViewsController.class));
    textInputPlugin.setTextInputClient(
        0,
        new TextInputChannel.Configuration(
            false,
            false,
            true,
            true,
            false,
            TextInputChannel.TextCapitalization.NONE,
            new TextInputChannel.InputType(TextInputChannel.TextInputType.TEXT, false, false),
            null,
            null,
            null,
            null));
    // There's a pending restart since we initialized the text input client. Flush that now.
    textInputPlugin.setTextInputEditingState(
        testView, new TextInputChannel.TextEditState("text", 0, 0, -1, -1));
    InputConnection connection =
        textInputPlugin.createInputConnection(
            testView, mock(KeyboardManager.class), new EditorInfo());

    connection.requestCursorUpdates(
        InputConnection.CURSOR_UPDATE_MONITOR | InputConnection.CURSOR_UPDATE_IMMEDIATE);

    connection.finishComposingText();

    assertEquals(-1, testImm.getLastCursorAnchorInfo().getComposingTextStart());
    assertEquals(0, testImm.getLastCursorAnchorInfo().getComposingText().length());
  }

  @Test
  public void inputConnection_textInputTypeNone() {
    View testView = new View(RuntimeEnvironment.application);
    DartExecutor dartExecutor = mock(DartExecutor.class);
    TextInputChannel textInputChannel = new TextInputChannel(dartExecutor);
    TextInputPlugin textInputPlugin =
        new TextInputPlugin(testView, textInputChannel, mock(PlatformViewsController.class));
    textInputPlugin.setTextInputClient(
        0,
        new TextInputChannel.Configuration(
            false,
            false,
            true,
            true,
            false,
            TextInputChannel.TextCapitalization.NONE,
            new TextInputChannel.InputType(TextInputChannel.TextInputType.NONE, false, false),
            null,
            null,
            null,
            null));

    InputConnection connection =
        textInputPlugin.createInputConnection(
            testView, mock(KeyboardManager.class), new EditorInfo());
    assertEquals(connection, null);
  }

  @Test
  public void showTextInput_textInputTypeNone() {
    TestImm testImm =
        Shadow.extract(
            RuntimeEnvironment.application.getSystemService(Context.INPUT_METHOD_SERVICE));
    View testView = new View(RuntimeEnvironment.application);
    DartExecutor dartExecutor = mock(DartExecutor.class);
    TextInputChannel textInputChannel = new TextInputChannel(dartExecutor);
    TextInputPlugin textInputPlugin =
        new TextInputPlugin(testView, textInputChannel, mock(PlatformViewsController.class));
    textInputPlugin.setTextInputClient(
        0,
        new TextInputChannel.Configuration(
            false,
            false,
            true,
            true,
            false,
            TextInputChannel.TextCapitalization.NONE,
            new TextInputChannel.InputType(TextInputChannel.TextInputType.NONE, false, false),
            null,
            null,
            null,
            null));

    textInputPlugin.showTextInput(testView);
    assertEquals(testImm.isSoftInputVisible(), false);
  }

  // -------- Start: Autofill Tests -------
  @Test
  public void autofill_enabledByDefault() {
    if (Build.VERSION.SDK_INT < Build.VERSION_CODES.O) {
      return;
    }
    FlutterView testView = new FlutterView(RuntimeEnvironment.application);
    TextInputChannel textInputChannel = new TextInputChannel(mock(DartExecutor.class));
    TextInputPlugin textInputPlugin =
        new TextInputPlugin(testView, textInputChannel, mock(PlatformViewsController.class));
    final TextInputChannel.Configuration.Autofill autofill =
        new TextInputChannel.Configuration.Autofill(
            "1", new String[] {}, null, new TextInputChannel.TextEditState("", 0, 0, -1, -1));

    final TextInputChannel.Configuration config =
        new TextInputChannel.Configuration(
            false,
            false,
            true,
            true,
            TextInputChannel.TextCapitalization.NONE,
            null,
            null,
            null,
            autofill,
            null);

    textInputPlugin.setTextInputClient(
        0,
        new TextInputChannel.Configuration(
            false,
            false,
            true,
            true,
            TextInputChannel.TextCapitalization.NONE,
            null,
            null,
            null,
            autofill,
            new TextInputChannel.Configuration[] {config}));

    final ViewStructure viewStructure = mock(ViewStructure.class);
    final ViewStructure[] children = {mock(ViewStructure.class), mock(ViewStructure.class)};

    when(viewStructure.newChild(anyInt()))
        .thenAnswer(invocation -> children[(int) invocation.getArgument(0)]);

    textInputPlugin.onProvideAutofillVirtualStructure(viewStructure, 0);

    verify(viewStructure).newChild(0);

    verify(children[0]).setAutofillId(any(), eq("1".hashCode()));
    verify(children[0]).setAutofillHints(aryEq(new String[] {}));
    verify(children[0]).setDimens(anyInt(), anyInt(), anyInt(), anyInt(), gt(0), gt(0));
  }

  @Test
  public void autofill_canBeDisabled() {
    if (Build.VERSION.SDK_INT < Build.VERSION_CODES.O) {
      return;
    }
    FlutterView testView = new FlutterView(RuntimeEnvironment.application);
    TextInputChannel textInputChannel = new TextInputChannel(mock(DartExecutor.class));
    TextInputPlugin textInputPlugin =
        new TextInputPlugin(testView, textInputChannel, mock(PlatformViewsController.class));
    final TextInputChannel.Configuration.Autofill autofill =
        new TextInputChannel.Configuration.Autofill(
            "1", new String[] {}, null, new TextInputChannel.TextEditState("", 0, 0, -1, -1));

    final TextInputChannel.Configuration config =
        new TextInputChannel.Configuration(
            false,
            false,
            true,
            true,
            TextInputChannel.TextCapitalization.NONE,
            null,
            null,
            null,
            null,
            null);

    textInputPlugin.setTextInputClient(0, config);

    final ViewStructure viewStructure = mock(ViewStructure.class);

    textInputPlugin.onProvideAutofillVirtualStructure(viewStructure, 0);

    verify(viewStructure, times(0)).newChild(anyInt());
  }

  @Test
  public void autofill_hintText() {
    if (Build.VERSION.SDK_INT < Build.VERSION_CODES.O) {
      return;
    }
    FlutterView testView = new FlutterView(RuntimeEnvironment.application);
    TextInputChannel textInputChannel = new TextInputChannel(mock(DartExecutor.class));
    TextInputPlugin textInputPlugin =
        new TextInputPlugin(testView, textInputChannel, mock(PlatformViewsController.class));
    final TextInputChannel.Configuration.Autofill autofill =
        new TextInputChannel.Configuration.Autofill(
            "1",
            new String[] {},
            "placeholder",
            new TextInputChannel.TextEditState("", 0, 0, -1, -1));

    final TextInputChannel.Configuration config =
        new TextInputChannel.Configuration(
            false,
            false,
            true,
            true,
            TextInputChannel.TextCapitalization.NONE,
            null,
            null,
            null,
            autofill,
            null);

    textInputPlugin.setTextInputClient(0, config);

    final ViewStructure viewStructure = mock(ViewStructure.class);
    final ViewStructure[] children = {mock(ViewStructure.class), mock(ViewStructure.class)};

    when(viewStructure.newChild(anyInt()))
        .thenAnswer(invocation -> children[(int) invocation.getArgument(0)]);

    textInputPlugin.onProvideAutofillVirtualStructure(viewStructure, 0);
    verify(children[0]).setHint("placeholder");
  }

  @Test
  public void autofill_onProvideVirtualViewStructure() {
    if (Build.VERSION.SDK_INT < Build.VERSION_CODES.O) {
      return;
    }

    FlutterView testView = new FlutterView(RuntimeEnvironment.application);
    TextInputChannel textInputChannel = new TextInputChannel(mock(DartExecutor.class));
    TextInputPlugin textInputPlugin =
        new TextInputPlugin(testView, textInputChannel, mock(PlatformViewsController.class));
    final TextInputChannel.Configuration.Autofill autofill1 =
        new TextInputChannel.Configuration.Autofill(
            "1",
            new String[] {"HINT1"},
            "placeholder1",
            new TextInputChannel.TextEditState("", 0, 0, -1, -1));
    final TextInputChannel.Configuration.Autofill autofill2 =
        new TextInputChannel.Configuration.Autofill(
            "2",
            new String[] {"HINT2", "EXTRA"},
            null,
            new TextInputChannel.TextEditState("", 0, 0, -1, -1));

    final TextInputChannel.Configuration config1 =
        new TextInputChannel.Configuration(
            false,
            false,
            true,
            true,
            false,
            TextInputChannel.TextCapitalization.NONE,
            null,
            null,
            null,
            autofill1,
            null);
    final TextInputChannel.Configuration config2 =
        new TextInputChannel.Configuration(
            false,
            false,
            true,
            true,
            false,
            TextInputChannel.TextCapitalization.NONE,
            null,
            null,
            null,
            autofill2,
            null);

    textInputPlugin.setTextInputClient(
        0,
        new TextInputChannel.Configuration(
            false,
            false,
            true,
            true,
            false,
            TextInputChannel.TextCapitalization.NONE,
            null,
            null,
            null,
            autofill1,
            new TextInputChannel.Configuration[] {config1, config2}));

    final ViewStructure viewStructure = mock(ViewStructure.class);
    final ViewStructure[] children = {mock(ViewStructure.class), mock(ViewStructure.class)};

    when(viewStructure.newChild(anyInt()))
        .thenAnswer(invocation -> children[(int) invocation.getArgument(0)]);

    textInputPlugin.onProvideAutofillVirtualStructure(viewStructure, 0);

    verify(viewStructure).newChild(0);
    verify(viewStructure).newChild(1);

    verify(children[0]).setAutofillId(any(), eq("1".hashCode()));
    verify(children[0]).setAutofillHints(aryEq(new String[] {"HINT1"}));
    verify(children[0]).setDimens(anyInt(), anyInt(), anyInt(), anyInt(), gt(0), gt(0));
    verify(children[0]).setHint("placeholder1");

    verify(children[1]).setAutofillId(any(), eq("2".hashCode()));
    verify(children[1]).setAutofillHints(aryEq(new String[] {"HINT2", "EXTRA"}));
    verify(children[1]).setDimens(anyInt(), anyInt(), anyInt(), anyInt(), gt(0), gt(0));
    verify(children[1], times(0)).setHint(any());
  }

  @Test
  public void autofill_onProvideVirtualViewStructure_single() {
    if (Build.VERSION.SDK_INT < Build.VERSION_CODES.O) {
      return;
    }

    FlutterView testView = new FlutterView(RuntimeEnvironment.application);
    TextInputChannel textInputChannel = new TextInputChannel(mock(DartExecutor.class));
    TextInputPlugin textInputPlugin =
        new TextInputPlugin(testView, textInputChannel, mock(PlatformViewsController.class));
    final TextInputChannel.Configuration.Autofill autofill =
        new TextInputChannel.Configuration.Autofill(
            "1",
            new String[] {"HINT1"},
            "placeholder",
            new TextInputChannel.TextEditState("", 0, 0, -1, -1));

    // Autofill should still work without AutofillGroup.
    textInputPlugin.setTextInputClient(
        0,
        new TextInputChannel.Configuration(
            false,
            false,
            true,
            true,
            false,
            TextInputChannel.TextCapitalization.NONE,
            null,
            null,
            null,
            autofill,
            null));

    final ViewStructure viewStructure = mock(ViewStructure.class);
    final ViewStructure[] children = {mock(ViewStructure.class)};

    when(viewStructure.newChild(anyInt()))
        .thenAnswer(invocation -> children[(int) invocation.getArgument(0)]);

    textInputPlugin.onProvideAutofillVirtualStructure(viewStructure, 0);

    verify(viewStructure).newChild(0);

    verify(children[0]).setAutofillId(any(), eq("1".hashCode()));
    verify(children[0]).setAutofillHints(aryEq(new String[] {"HINT1"}));
    verify(children[0]).setHint("placeholder");
    // Verifies that the child has a non-zero size.
    verify(children[0]).setDimens(anyInt(), anyInt(), anyInt(), anyInt(), gt(0), gt(0));
  }

  @Test
  public void autofill_testLifeCycle() {
    if (Build.VERSION.SDK_INT < Build.VERSION_CODES.O) {
      return;
    }

    TestAfm testAfm =
        Shadow.extract(RuntimeEnvironment.application.getSystemService(AutofillManager.class));
    FlutterView testView = new FlutterView(RuntimeEnvironment.application);
    TextInputChannel textInputChannel = new TextInputChannel(mock(DartExecutor.class));
    TextInputPlugin textInputPlugin =
        new TextInputPlugin(testView, textInputChannel, mock(PlatformViewsController.class));

    // Set up an autofill scenario with 2 fields.
    final TextInputChannel.Configuration.Autofill autofill1 =
        new TextInputChannel.Configuration.Autofill(
            "1",
            new String[] {"HINT1"},
            "placeholder1",
            new TextInputChannel.TextEditState("", 0, 0, -1, -1));
    final TextInputChannel.Configuration.Autofill autofill2 =
        new TextInputChannel.Configuration.Autofill(
            "2",
            new String[] {"HINT2", "EXTRA"},
            null,
            new TextInputChannel.TextEditState("", 0, 0, -1, -1));

    final TextInputChannel.Configuration config1 =
        new TextInputChannel.Configuration(
            false,
            false,
            true,
            true,
            false,
            TextInputChannel.TextCapitalization.NONE,
            null,
            null,
            null,
            autofill1,
            null);
    final TextInputChannel.Configuration config2 =
        new TextInputChannel.Configuration(
            false,
            false,
            true,
            true,
            false,
            TextInputChannel.TextCapitalization.NONE,
            null,
            null,
            null,
            autofill2,
            null);

    // Set client. This should call notifyViewExited on the FlutterView if the previous client is
    // also eligible for autofill.
    final TextInputChannel.Configuration autofillConfiguration =
        new TextInputChannel.Configuration(
            false,
            false,
            true,
            true,
            false,
            TextInputChannel.TextCapitalization.NONE,
            null,
            null,
            null,
            autofill1,
            new TextInputChannel.Configuration[] {config1, config2});

    textInputPlugin.setTextInputClient(0, autofillConfiguration);

    // notifyViewExited should not be called as this is the first client we set.
    assertEquals(testAfm.empty, testAfm.exitId);

    // The framework updates the text, call notifyValueChanged.
    textInputPlugin.setTextInputEditingState(
        testView, new TextInputChannel.TextEditState("new text", -1, -1, -1, -1));
    assertEquals("new text", testAfm.changeString);
    assertEquals("1".hashCode(), testAfm.changeVirtualId);

    // The input method updates the text, call notifyValueChanged.
    testAfm.resetStates();
    final KeyboardManager mockKeyboardManager = mock(KeyboardManager.class);
    InputConnectionAdaptor adaptor =
        new InputConnectionAdaptor(
            testView,
            0,
            mock(TextInputChannel.class),
            mockKeyboardManager,
            (ListenableEditingState) textInputPlugin.getEditable(),
            new EditorInfo());
    adaptor.commitText("input from IME ", 1);

    assertEquals("input from IME new text", testAfm.changeString);
    assertEquals("1".hashCode(), testAfm.changeVirtualId);

    // notifyViewExited should be called on the previous client.
    testAfm.resetStates();
    textInputPlugin.setTextInputClient(
        1,
        new TextInputChannel.Configuration(
            false,
            false,
            true,
            true,
            false,
            TextInputChannel.TextCapitalization.NONE,
            null,
            null,
            null,
            null,
            null));

    assertEquals("1".hashCode(), testAfm.exitId);

    // TextInputPlugin#clearTextInputClient calls notifyViewExited.
    testAfm.resetStates();
    textInputPlugin.setTextInputClient(3, autofillConfiguration);
    assertEquals(testAfm.empty, testAfm.exitId);
    textInputPlugin.clearTextInputClient();
    assertEquals("1".hashCode(), testAfm.exitId);

    // TextInputPlugin#destroy calls notifyViewExited.
    testAfm.resetStates();
    textInputPlugin.setTextInputClient(4, autofillConfiguration);
    assertEquals(testAfm.empty, testAfm.exitId);
    textInputPlugin.destroy();
    assertEquals("1".hashCode(), testAfm.exitId);
  }

  @Test
  public void autofill_testAutofillUpdatesTheFramework() {
    if (Build.VERSION.SDK_INT < Build.VERSION_CODES.O) {
      return;
    }

    TestAfm testAfm =
        Shadow.extract(RuntimeEnvironment.application.getSystemService(AutofillManager.class));
    FlutterView testView = new FlutterView(RuntimeEnvironment.application);
    TextInputChannel textInputChannel = spy(new TextInputChannel(mock(DartExecutor.class)));
    TextInputPlugin textInputPlugin =
        new TextInputPlugin(testView, textInputChannel, mock(PlatformViewsController.class));

    // Set up an autofill scenario with 2 fields.
    final TextInputChannel.Configuration.Autofill autofill1 =
        new TextInputChannel.Configuration.Autofill(
            "1",
            new String[] {"HINT1"},
            null,
            new TextInputChannel.TextEditState("field 1", 0, 0, -1, -1));
    final TextInputChannel.Configuration.Autofill autofill2 =
        new TextInputChannel.Configuration.Autofill(
            "2",
            new String[] {"HINT2", "EXTRA"},
            null,
            new TextInputChannel.TextEditState("field 2", 0, 0, -1, -1));

    final TextInputChannel.Configuration config1 =
        new TextInputChannel.Configuration(
            false,
            false,
            true,
            true,
            false,
            TextInputChannel.TextCapitalization.NONE,
            null,
            null,
            null,
            autofill1,
            null);
    final TextInputChannel.Configuration config2 =
        new TextInputChannel.Configuration(
            false,
            false,
            true,
            true,
            false,
            TextInputChannel.TextCapitalization.NONE,
            null,
            null,
            null,
            autofill2,
            null);

    final TextInputChannel.Configuration autofillConfiguration =
        new TextInputChannel.Configuration(
            false,
            false,
            true,
            true,
            false,
            TextInputChannel.TextCapitalization.NONE,
            null,
            null,
            null,
            autofill1,
            new TextInputChannel.Configuration[] {config1, config2});

    textInputPlugin.setTextInputClient(0, autofillConfiguration);
    textInputPlugin.setTextInputEditingState(
        testView, new TextInputChannel.TextEditState("", 0, 0, -1, -1));

    final SparseArray<AutofillValue> autofillValues = new SparseArray();
    autofillValues.append("1".hashCode(), AutofillValue.forText("focused field"));
    autofillValues.append("2".hashCode(), AutofillValue.forText("unfocused field"));

    // Autofill both fields.
    textInputPlugin.autofill(autofillValues);

    // Verify the Editable has been updated.
    assertTrue(textInputPlugin.getEditable().toString().equals("focused field"));

    // The autofill value of the focused field is sent via updateEditingState.
    verify(textInputChannel, times(1))
        .updateEditingState(anyInt(), eq("focused field"), eq(13), eq(13), eq(-1), eq(-1));

    final ArgumentCaptor<HashMap> mapCaptor = ArgumentCaptor.forClass(HashMap.class);

    verify(textInputChannel, times(1)).updateEditingStateWithTag(anyInt(), mapCaptor.capture());
    final TextInputChannel.TextEditState editState =
        (TextInputChannel.TextEditState) mapCaptor.getValue().get("2");
    assertEquals(editState.text, "unfocused field");
  }

  @Test
  public void autofill_testSetTextIpnutClientUpdatesSideFields() {
    if (Build.VERSION.SDK_INT < Build.VERSION_CODES.O) {
      return;
    }

    TestAfm testAfm =
        Shadow.extract(RuntimeEnvironment.application.getSystemService(AutofillManager.class));
    FlutterView testView = new FlutterView(RuntimeEnvironment.application);
    TextInputChannel textInputChannel = new TextInputChannel(mock(DartExecutor.class));
    TextInputPlugin textInputPlugin =
        new TextInputPlugin(testView, textInputChannel, mock(PlatformViewsController.class));

    // Set up an autofill scenario with 2 fields.
    final TextInputChannel.Configuration.Autofill autofill1 =
        new TextInputChannel.Configuration.Autofill(
            "1",
            new String[] {"HINT1"},
            "null",
            new TextInputChannel.TextEditState("", 0, 0, -1, -1));
    final TextInputChannel.Configuration.Autofill autofill2 =
        new TextInputChannel.Configuration.Autofill(
            "2",
            new String[] {"HINT2", "EXTRA"},
            "null",
            new TextInputChannel.TextEditState(
                "Unfocused fields need love like everything does", 0, 0, -1, -1));

    final TextInputChannel.Configuration config1 =
        new TextInputChannel.Configuration(
            false,
            false,
            true,
            true,
            false,
            TextInputChannel.TextCapitalization.NONE,
            null,
            null,
            null,
            autofill1,
            null);
    final TextInputChannel.Configuration config2 =
        new TextInputChannel.Configuration(
            false,
            false,
            true,
            true,
            false,
            TextInputChannel.TextCapitalization.NONE,
            null,
            null,
            null,
            autofill2,
            null);

    final TextInputChannel.Configuration autofillConfiguration =
        new TextInputChannel.Configuration(
            false,
            false,
            true,
            true,
            false,
            TextInputChannel.TextCapitalization.NONE,
            null,
            null,
            null,
            autofill1,
            new TextInputChannel.Configuration[] {config1, config2});

    textInputPlugin.setTextInputClient(0, autofillConfiguration);

    // notifyValueChanged should be called for unfocused fields.
    assertEquals("2".hashCode(), testAfm.changeVirtualId);
    assertEquals("Unfocused fields need love like everything does", testAfm.changeString);
  }
  // -------- End: Autofill Tests -------

  @Test
  public void respondsToInputChannelMessages() {
    ArgumentCaptor<BinaryMessenger.BinaryMessageHandler> binaryMessageHandlerCaptor =
        ArgumentCaptor.forClass(BinaryMessenger.BinaryMessageHandler.class);
    DartExecutor mockBinaryMessenger = mock(DartExecutor.class);
    TextInputChannel.TextInputMethodHandler mockHandler =
        mock(TextInputChannel.TextInputMethodHandler.class);
    TextInputChannel textInputChannel = new TextInputChannel(mockBinaryMessenger);

    textInputChannel.setTextInputMethodHandler(mockHandler);

    verify(mockBinaryMessenger, times(1))
        .setMessageHandler(any(String.class), binaryMessageHandlerCaptor.capture());

    BinaryMessenger.BinaryMessageHandler binaryMessageHandler =
        binaryMessageHandlerCaptor.getValue();

    sendToBinaryMessageHandler(binaryMessageHandler, "TextInput.requestAutofill", null);
    verify(mockHandler, times(1)).requestAutofill();

    sendToBinaryMessageHandler(binaryMessageHandler, "TextInput.finishAutofillContext", true);
    verify(mockHandler, times(1)).finishAutofillContext(true);

    sendToBinaryMessageHandler(binaryMessageHandler, "TextInput.finishAutofillContext", false);
    verify(mockHandler, times(1)).finishAutofillContext(false);
  }

  @Test
  public void sendAppPrivateCommand_dataIsEmpty() throws JSONException {
    ArgumentCaptor<BinaryMessenger.BinaryMessageHandler> binaryMessageHandlerCaptor =
        ArgumentCaptor.forClass(BinaryMessenger.BinaryMessageHandler.class);
    DartExecutor mockBinaryMessenger = mock(DartExecutor.class);
    TextInputChannel textInputChannel = new TextInputChannel(mockBinaryMessenger);

    EventHandler mockEventHandler = mock(EventHandler.class);
    TestImm testImm =
        Shadow.extract(
            RuntimeEnvironment.application.getSystemService(Context.INPUT_METHOD_SERVICE));
    testImm.setEventHandler(mockEventHandler);

    View testView = new View(RuntimeEnvironment.application);
    TextInputPlugin textInputPlugin =
        new TextInputPlugin(testView, textInputChannel, mock(PlatformViewsController.class));

    verify(mockBinaryMessenger, times(1))
        .setMessageHandler(any(String.class), binaryMessageHandlerCaptor.capture());

    JSONObject arguments = new JSONObject();
    arguments.put("action", "actionCommand");
    arguments.put("data", "");

    BinaryMessenger.BinaryMessageHandler binaryMessageHandler =
        binaryMessageHandlerCaptor.getValue();
    sendToBinaryMessageHandler(binaryMessageHandler, "TextInput.sendAppPrivateCommand", arguments);
    verify(mockEventHandler, times(1))
        .sendAppPrivateCommand(any(View.class), eq("actionCommand"), eq(null));
  }

  @Test
  public void sendAppPrivateCommand_hasData() throws JSONException {
    ArgumentCaptor<BinaryMessenger.BinaryMessageHandler> binaryMessageHandlerCaptor =
        ArgumentCaptor.forClass(BinaryMessenger.BinaryMessageHandler.class);
    DartExecutor mockBinaryMessenger = mock(DartExecutor.class);
    TextInputChannel textInputChannel = new TextInputChannel(mockBinaryMessenger);

    EventHandler mockEventHandler = mock(EventHandler.class);
    TestImm testImm =
        Shadow.extract(
            RuntimeEnvironment.application.getSystemService(Context.INPUT_METHOD_SERVICE));
    testImm.setEventHandler(mockEventHandler);

    View testView = new View(RuntimeEnvironment.application);
    TextInputPlugin textInputPlugin =
        new TextInputPlugin(testView, textInputChannel, mock(PlatformViewsController.class));

    verify(mockBinaryMessenger, times(1))
        .setMessageHandler(any(String.class), binaryMessageHandlerCaptor.capture());

    JSONObject arguments = new JSONObject();
    arguments.put("action", "actionCommand");
    arguments.put("data", "actionData");

    ArgumentCaptor<Bundle> bundleCaptor = ArgumentCaptor.forClass(Bundle.class);
    BinaryMessenger.BinaryMessageHandler binaryMessageHandler =
        binaryMessageHandlerCaptor.getValue();
    sendToBinaryMessageHandler(binaryMessageHandler, "TextInput.sendAppPrivateCommand", arguments);
    verify(mockEventHandler, times(1))
        .sendAppPrivateCommand(any(View.class), eq("actionCommand"), bundleCaptor.capture());
    assertEquals("actionData", bundleCaptor.getValue().getCharSequence("data"));
  }

  @Test
  @TargetApi(30)
  @Config(sdk = 30)
  public void ime_windowInsetsSync() {
    FlutterView testView = new FlutterView(RuntimeEnvironment.application);
    TextInputChannel textInputChannel = new TextInputChannel(mock(DartExecutor.class));
    TextInputPlugin textInputPlugin =
        new TextInputPlugin(testView, textInputChannel, mock(PlatformViewsController.class));
    ImeSyncDeferringInsetsCallback imeSyncCallback = textInputPlugin.getImeSyncCallback();
    FlutterEngine flutterEngine =
        spy(new FlutterEngine(RuntimeEnvironment.application, mockFlutterLoader, mockFlutterJni));
    FlutterRenderer flutterRenderer = spy(new FlutterRenderer(mockFlutterJni));
    when(flutterEngine.getRenderer()).thenReturn(flutterRenderer);
    testView.attachToFlutterEngine(flutterEngine);

    WindowInsetsAnimation animation = mock(WindowInsetsAnimation.class);
    when(animation.getTypeMask()).thenReturn(WindowInsets.Type.ime());

    List<WindowInsetsAnimation> animationList = new ArrayList();
    animationList.add(animation);

    WindowInsets.Builder builder = new WindowInsets.Builder();
    WindowInsets noneInsets = builder.build();

    // imeInsets0, 1, and 2 contain unique IME bottom insets, and are used
    // to distinguish which insets were sent at each stage.
    builder.setInsets(WindowInsets.Type.ime(), Insets.of(0, 0, 0, 100));
    builder.setInsets(WindowInsets.Type.navigationBars(), Insets.of(10, 10, 10, 40));
    WindowInsets imeInsets0 = builder.build();

    builder.setInsets(WindowInsets.Type.ime(), Insets.of(0, 0, 0, 30));
    builder.setInsets(WindowInsets.Type.navigationBars(), Insets.of(10, 10, 10, 40));
    WindowInsets imeInsets1 = builder.build();

    builder.setInsets(WindowInsets.Type.ime(), Insets.of(0, 0, 0, 50));
    builder.setInsets(WindowInsets.Type.navigationBars(), Insets.of(10, 10, 10, 40));
    WindowInsets imeInsets2 = builder.build();

    builder.setInsets(WindowInsets.Type.ime(), Insets.of(0, 0, 0, 200));
    builder.setInsets(WindowInsets.Type.navigationBars(), Insets.of(10, 10, 10, 0));
    WindowInsets deferredInsets = builder.build();

    ArgumentCaptor<FlutterRenderer.ViewportMetrics> viewportMetricsCaptor =
        ArgumentCaptor.forClass(FlutterRenderer.ViewportMetrics.class);

    imeSyncCallback.getInsetsListener().onApplyWindowInsets(testView, deferredInsets);
    imeSyncCallback.getInsetsListener().onApplyWindowInsets(testView, noneInsets);

    verify(flutterRenderer, atLeast(1)).setViewportMetrics(viewportMetricsCaptor.capture());
    assertEquals(0, viewportMetricsCaptor.getValue().viewPaddingBottom);
    assertEquals(0, viewportMetricsCaptor.getValue().viewPaddingTop);
    assertEquals(0, viewportMetricsCaptor.getValue().viewInsetBottom);
    assertEquals(0, viewportMetricsCaptor.getValue().viewInsetTop);

    imeSyncCallback.getAnimationCallback().onPrepare(animation);
    imeSyncCallback.getInsetsListener().onApplyWindowInsets(testView, deferredInsets);
    imeSyncCallback.getAnimationCallback().onStart(animation, null);
    // Only the final state call is saved, extra calls are passed on.
    imeSyncCallback.getInsetsListener().onApplyWindowInsets(testView, imeInsets2);

    verify(flutterRenderer, atLeast(1)).setViewportMetrics(viewportMetricsCaptor.capture());
    // No change, as deferredInset is stored to be passed in onEnd()
    assertEquals(0, viewportMetricsCaptor.getValue().viewPaddingBottom);
    assertEquals(0, viewportMetricsCaptor.getValue().viewPaddingTop);
    assertEquals(0, viewportMetricsCaptor.getValue().viewInsetBottom);
    assertEquals(0, viewportMetricsCaptor.getValue().viewInsetTop);

    imeSyncCallback.getAnimationCallback().onProgress(imeInsets0, animationList);

    verify(flutterRenderer, atLeast(1)).setViewportMetrics(viewportMetricsCaptor.capture());
    assertEquals(0, viewportMetricsCaptor.getValue().viewPaddingBottom);
    assertEquals(10, viewportMetricsCaptor.getValue().viewPaddingTop);
    assertEquals(60, viewportMetricsCaptor.getValue().viewInsetBottom);
    assertEquals(0, viewportMetricsCaptor.getValue().viewInsetTop);

    imeSyncCallback.getAnimationCallback().onProgress(imeInsets1, animationList);

    verify(flutterRenderer, atLeast(1)).setViewportMetrics(viewportMetricsCaptor.capture());
    assertEquals(0, viewportMetricsCaptor.getValue().viewPaddingBottom);
    assertEquals(10, viewportMetricsCaptor.getValue().viewPaddingTop);
    assertEquals(0, viewportMetricsCaptor.getValue().viewInsetBottom); // Cannot be negative
    assertEquals(0, viewportMetricsCaptor.getValue().viewInsetTop);

    imeSyncCallback.getAnimationCallback().onEnd(animation);

    verify(flutterRenderer, atLeast(1)).setViewportMetrics(viewportMetricsCaptor.capture());
    // Values should be of deferredInsets, not imeInsets2
    assertEquals(0, viewportMetricsCaptor.getValue().viewPaddingBottom);
    assertEquals(10, viewportMetricsCaptor.getValue().viewPaddingTop);
    assertEquals(200, viewportMetricsCaptor.getValue().viewInsetBottom);
    assertEquals(0, viewportMetricsCaptor.getValue().viewInsetTop);
  }

  interface EventHandler {
    void sendAppPrivateCommand(View view, String action, Bundle data);
  }

  @Implements(InputMethodManager.class)
  public static class TestImm extends ShadowInputMethodManager {
    private InputMethodSubtype currentInputMethodSubtype;
    private SparseIntArray restartCounter = new SparseIntArray();
    private CursorAnchorInfo cursorAnchorInfo;
    private ArrayList<Integer> selectionUpdateValues;
    private boolean trackSelection = false;
    private EventHandler handler;

    public TestImm() {
      selectionUpdateValues = new ArrayList<Integer>();
    }

    @Implementation
    public InputMethodSubtype getCurrentInputMethodSubtype() {
      return currentInputMethodSubtype;
    }

    @Implementation
    public void restartInput(View view) {
      int count = restartCounter.get(view.hashCode(), /*defaultValue=*/ 0) + 1;
      restartCounter.put(view.hashCode(), count);
    }

    public void setCurrentInputMethodSubtype(InputMethodSubtype inputMethodSubtype) {
      this.currentInputMethodSubtype = inputMethodSubtype;
    }

    public int getRestartCount(View view) {
      return restartCounter.get(view.hashCode(), /*defaultValue=*/ 0);
    }

    public void setEventHandler(EventHandler eventHandler) {
      handler = eventHandler;
    }

    @Implementation
    public void sendAppPrivateCommand(View view, String action, Bundle data) {
      handler.sendAppPrivateCommand(view, action, data);
    }

    @Implementation
    public void updateCursorAnchorInfo(View view, CursorAnchorInfo cursorAnchorInfo) {
      this.cursorAnchorInfo = cursorAnchorInfo;
    }

    // We simply store the values to verify later.
    @Implementation
    public void updateSelection(
        View view, int selStart, int selEnd, int candidatesStart, int candidatesEnd) {
      if (trackSelection) {
        this.selectionUpdateValues.add(selStart);
        this.selectionUpdateValues.add(selEnd);
        this.selectionUpdateValues.add(candidatesStart);
        this.selectionUpdateValues.add(candidatesEnd);
      }
    }

    // only track values when enabled via this.
    public void setTrackSelection(boolean val) {
      trackSelection = val;
    }

    // Returns true if the last updateSelection call passed the following values.
    public ArrayList<Integer> getSelectionUpdateValues() {
      return selectionUpdateValues;
    }

    public CursorAnchorInfo getLastCursorAnchorInfo() {
      return cursorAnchorInfo;
    }
  }

  @Implements(AutofillManager.class)
  public static class TestAfm extends ShadowAutofillManager {
    public static int empty = -999;

    String finishState;
    int changeVirtualId = empty;
    String changeString;

    int enterId = empty;
    int exitId = empty;

    @Implementation
    public void cancel() {
      finishState = "cancel";
    }

    public void commit() {
      finishState = "commit";
    }

    public void notifyViewEntered(View view, int virtualId, Rect absBounds) {
      enterId = virtualId;
    }

    public void notifyViewExited(View view, int virtualId) {
      exitId = virtualId;
    }

    public void notifyValueChanged(View view, int virtualId, AutofillValue value) {
      if (Build.VERSION.SDK_INT < Build.VERSION_CODES.O) {
        return;
      }
      changeVirtualId = virtualId;
      changeString = value.getTextValue().toString();
    }

    public void resetStates() {
      finishState = null;
      changeVirtualId = empty;
      changeString = null;
      enterId = empty;
      exitId = empty;
    }
  }
}
