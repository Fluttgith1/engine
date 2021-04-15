// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package io.flutter.embedding.engine.plugins.util;

import androidx.annotation.NonNull;
import io.flutter.Log;
import io.flutter.embedding.engine.FlutterEngine;
import java.lang.reflect.Method;

public class GeneratedPluginRegister {
  private static final String TAG = "GeneratedPluginsRegister";
  /**
   * Registers all plugins that an app lists in its pubspec.yaml.
   *
   * <p>To let each plugin set itself up to listen to usage calls from Dart via platform channel,
   * each plugin is given a chance to initialize itself and setup platform channel listeners
   * in {@link io.flutter.embedding.engine.plugins.FlutterPlugin#onAttachedToEngine(io.flutter.embedding.engine.plugins.FlutterPlugin.FlutterPluginBinding)}.
   *
   * <p>The list of plugins needing setup is not known to the Flutter engine, but generated at build
   * time based on the plugin dependencies specified in the Flutter project's pubspec.yaml.
   *
   * <p>The Flutter tool generates that list in a class called {@code GeneratedPluginRegistrant}. That
   * class contains generated code to register every plugin in the pubspec.yaml with a
   * {@link FlutterEngine}. That file is generated in the Flutter project's directory.
   *
   * <p>The {@link FlutterEngine} (when a {@link FlutterEngine} is explicitly created) or the
   * {@link io.flutter.embedding.android.FlutterActivity} (when a {@link FlutterEngine} is
   * implicitly created) will automatically call the {@code GeneratedPluginRegistrant}, typically at
   * the beginning of the application for normal full-Flutter apps, to register all plugins when a
   * {@link FlutterEngine} is created.
   *
   * <p>Since the {@link FlutterEngine} belongs to the Flutter engine and the GeneratedPluginRegistrant
   * class belongs to the app project, the {@link FlutterEngine} cannot place a compile-time
   * dependency on GeneratedPluginRegistrant to invoke it. Instead, this class uses reflection to
   * attempt to locate the generated file and then use it at runtime.
   *
   * <p>This method fizzles if the GeneratedPluginRegistrant cannot be found or invoked. This
   * situation should never occur, but if any eventuality comes up that prevents an app from using
   * this behavior, that app can still write code that explicitly registers plugins.
   *
   * <p>To disable this automatic plugin registration behavior:
   * <ul>
   * <li>If you manually construct {@link FlutterEngine}s, construct the {@link FlutterEngine} with
   * the {@code automaticallyRegisterPlugins} construction parameter set to false.
   * <li>If you let the {@link io.flutter.embedding.android.FlutterActivity} or
   * {@link io.flutter.embedding.android.FlutterFragmentActivity} construct a {@link FlutterEngine}
   * implicitly, override the {@code configureFlutterEngine} implementation and don't call through
   * to the superclass implementation.
   * </ul>
   * <p>Disabling the automatic plugin registration or deferring it by calling this method
   * explicitly may be useful in fine tuning the application launch latency characteristics for your
   * application.
   *
   * It's also possible to not use GeneratedPluginRegistrant and this method at all in order to fine
   * tune not only when plugins are registered but which plugins are registered when. Inspecting
   * the content of the GeneratedPluginRegistrant class will reveal that it's just going through
   * each of the plugins referenced in pubspec.yaml and calling
   * {@link io.flutter.embedding.engine.plugins.FlutterPlugin#onAttachedToEngine(io.flutter.embedding.engine.plugins.FlutterPlugin.FlutterPluginBinding)}
   * on each plugin. That code can be copy pasted and invoked directly per plugin to determine which
   * plugin gets registered when in your own application's code. Note that when that's done without
   * using the GeneratedPluginRegistrant, updating pubspec.yaml will no longer automatically update
   * the list of plugins being registered.
   */
  public static void registerGeneratedPlugins(@NonNull FlutterEngine flutterEngine) {
    try {
      Class<?> generatedPluginRegistrant =
          Class.forName("io.flutter.plugins.GeneratedPluginRegistrant");
      Method registrationMethod =
          generatedPluginRegistrant.getDeclaredMethod("registerWith", FlutterEngine.class);
      registrationMethod.invoke(null, flutterEngine);
    } catch (Exception e) {
      Log.e(
          TAG,
          "Tried to automatically register plugins with FlutterEngine ("
              + flutterEngine
              + ") but could not find or invoke the GeneratedPluginRegistrant.");
      Log.e(TAG, "Received exception while registering", e);
    }
  }
}
