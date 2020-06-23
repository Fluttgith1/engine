// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// @dart = 2.6
part of engine;

/// When set to true, all platform messages will be printed to the console.
const bool/*!*/ _debugPrintPlatformMessages = false;

/// Requests that the browser schedule a frame.
///
/// This may be overridden in tests, for example, to pump fake frames.
ui.VoidCallback/*?*/ scheduleFrameCallback;

/// The Web implementation of [ui.PlatformDispatcher].
///
/// This is the central entry point for platform messages and configuration
/// events from the platform.
class EnginePlatformDispatcher extends ui.PlatformDispatcher {
  /// Private constructor, since only dart:ui is supposed to create one of
  /// these.
  EnginePlatformDispatcher._() {
    _addBrightnessMediaQueryListener();
    js.context['_flutter_web_set_location_strategy'] = (LocationStrategy strategy) {
      locationStrategy = strategy;
    };
    registerHotRestartListener(() {
      js.context['_flutter_web_set_location_strategy'] = null;
    });
  }

  /// The [EnginePlatformDispatcher] singleton.
  static EnginePlatformDispatcher/*!*/ get instance => _instance;
  static final EnginePlatformDispatcher/*!*/ _instance = EnginePlatformDispatcher._();

  /// The current platform configuration.
  @override
  ui.PlatformConfiguration/*!*/ get configuration => _configuration;
  ui.PlatformConfiguration/*!*/ _configuration = ui.PlatformConfiguration(locales: parseBrowserLanguages());

  /// Receives all events related to platform configuration changes.
  @override
  ui.VoidCallback/*?*/ get onPlatformConfigurationChanged => _onPlatformConfigurationChanged;
  ui.VoidCallback/*?*/ _onPlatformConfigurationChanged;
  Zone/*!*/ _onPlatformConfigurationChangedZone = Zone.root;
  @override
  set onPlatformConfigurationChanged(ui.VoidCallback/*?*/ callback) {
    _onPlatformConfigurationChanged = callback;
    _onPlatformConfigurationChangedZone = Zone.current;
  }

  /// Engine code should use this method instead of the callback directly.
  /// Otherwise zones won't work properly.
  void invokeOnPlatformConfigurationChanged() {
    _invoke(_onPlatformConfigurationChanged, _onPlatformConfigurationChangedZone);
  }

  /// The current list of screens.
  Iterable<ui.Screen/*!*/>/*!*/ get screens => _screens.values;
  Map<Object/*!*/, ui.Screen/*!*/>/*!*/ _screens = <Object/*!*/, ui.Screen/*!*/>{};

  /// A map of opaque platform screen identifiers to screen configurations.
  ///
  /// This should be considered a protected member, only to be used by
  /// [PlatformDispatcher] subclasses.
  Map<Object/*!*/, ui.ScreenConfiguration/*!*/> _screenConfigurations = <Object/*!*/, ui.ScreenConfiguration/*!*/>{};

  /// The current list of windows,
  Iterable<ui.FlutterView/*!*/>/*!*/ get views => _windows.values;
  Map<Object, ui.FlutterWindow/*!*/>/*!*/ _windows = <Object/*!*/, ui.FlutterWindow/*!*/>{};

  /// A map of opaque platform window identifiers to window configurations.
  ///
  /// This should be considered a protected member, only to be used by
  /// [PlatformDispatcher] subclasses.
  Map<Object/*!*/, ui.ViewConfiguration/*!*/>/*!*/ _windowConfigurations = <Object/*!*/, ui.ViewConfiguration/*!*/>{};

  /// A callback that is invoked whenever the platform's [devicePixelRatio],
  /// [physicalSize], [padding], [viewInsets], or [systemGestureInsets]
  /// values change, for example when the device is rotated or when the
  /// application is resized (e.g. when showing applications side-by-side
  /// on Android).
  ///
  /// The engine invokes this callback in the same zone in which the callback
  /// was set.
  ///
  /// The framework registers with this callback and updates the layout
  /// appropriately.
  ///
  /// See also:
  ///
  ///  * [WidgetsBindingObserver], for a mechanism at the widgets layer to
  ///    register for notifications when this is called.
  ///  * [MediaQuery.of], a simpler mechanism for the same.
  @override
  ui.VoidCallback/*?*/ get onMetricsChanged => _onMetricsChanged;
  ui.VoidCallback/*?*/ _onMetricsChanged;
  Zone/*!*/ _onMetricsChangedZone = Zone.root; // ignore: unused_field
  @override
  set onMetricsChanged(ui.VoidCallback/*?*/ callback) {
    _onMetricsChanged = callback;
    _onMetricsChangedZone = Zone.current;
  }

  /// Engine code should use this method instead of the callback directly.
  /// Otherwise zones won't work properly.
  void invokeOnMetricsChanged() {
    _invoke(_onMetricsChanged, _onMetricsChangedZone);
  }

  /// Returns device pixel ratio returned by browser.
  static double/*!*/ get browserDevicePixelRatio {
    double/*?*/ ratio = html.window.devicePixelRatio;
    // Guard against WebOS returning 0.
    return (ratio == null || ratio == 0.0) ? 1.0 : ratio;
  }

  /// A callback invoked when any window begins a frame.
  ///
  /// {@template flutter.foundation.PlatformDispatcher.onBeginFrame}
  /// A callback that is invoked to notify the application that it is an
  /// appropriate time to provide a scene using the [SceneBuilder] API and the
  /// [PlatformWindow.render] method.
  /// When possible, this is driven by the hardware VSync signal of the attached
  /// screen with the highest VSync rate. This is only called if
  /// [PlatformWindow.scheduleFrame] has been called since the last time this
  /// callback was invoked.
  /// {@endtemplate}
  @override
  ui.FrameCallback/*?*/ get onBeginFrame => _onBeginFrame;
  ui.FrameCallback/*?*/ _onBeginFrame;
  Zone/*!*/ _onBeginFrameZone = Zone.root;
  @override
  set onBeginFrame(ui.FrameCallback/*?*/ callback) {
    _onBeginFrame = callback;
    _onBeginFrameZone = Zone.current;
  }

  /// Engine code should use this method instead of the callback directly.
  /// Otherwise zones won't work properly.
  void invokeOnBeginFrame(Duration/*!*/ duration) {
    _invoke1<Duration>(_onBeginFrame, _onBeginFrameZone, duration);
  }

  /// {@template flutter.foundation.PlatformDispatcher.onDrawFrame}
  /// A callback that is invoked for each frame after [onBeginFrame] has
  /// completed and after the microtask queue has been drained.
  ///
  /// This can be used to implement a second phase of frame rendering that
  /// happens after any deferred work queued by the [onBeginFrame] phase.
  /// {@endtemplate}
  @override
  ui.VoidCallback/*?*/ get onDrawFrame => _onDrawFrame;
  ui.VoidCallback/*?*/ _onDrawFrame;
  Zone/*!*/ _onDrawFrameZone = Zone.root;
  @override
  set onDrawFrame(ui.VoidCallback/*?*/ callback) {
    _onDrawFrame = callback;
    _onDrawFrameZone = Zone.current;
  }

  /// Engine code should use this method instead of the callback directly.
  /// Otherwise zones won't work properly.
  void invokeOnDrawFrame() {
    _invoke(_onDrawFrame, _onDrawFrameZone);
  }

  /// A callback that is invoked when pointer data is available.
  ///
  /// The framework invokes this callback in the same zone in which the
  /// callback was set.
  ///
  /// See also:
  ///
  ///  * [GestureBinding], the Flutter framework class which manages pointer
  ///    events.
  @override
  ui.PointerDataPacketCallback/*?*/ get onPointerDataPacket => _onPointerDataPacket;
  ui.PointerDataPacketCallback/*?*/ _onPointerDataPacket;
  Zone/*!*/ _onPointerDataPacketZone = Zone.root;
  @override
  set onPointerDataPacket(ui.PointerDataPacketCallback/*?*/ callback) {
    _onPointerDataPacket = callback;
    _onPointerDataPacketZone = Zone.current;
  }

  /// Engine code should use this method instead of the callback directly.
  /// Otherwise zones won't work properly.
  void invokeOnPointerDataPacket(ui.PointerDataPacket/*!*/ dataPacket) {
    _invoke1<ui.PointerDataPacket>(_onPointerDataPacket, _onPointerDataPacketZone, dataPacket);
  }

  /// A callback that is invoked to report the [FrameTiming] of recently
  /// rasterized frames.
  ///
  /// It's preferred to use [SchedulerBinding.addTimingsCallback] than to use
  /// [Window.onReportTimings] directly because
  /// [SchedulerBinding.addTimingsCallback] allows multiple callbacks.
  ///
  /// This can be used to see if the application has missed frames (through
  /// [FrameTiming.buildDuration] and [FrameTiming.rasterDuration]), or high
  /// latencies (through [FrameTiming.totalSpan]).
  ///
  /// Unlike [Timeline], the timing information here is available in the release
  /// mode (additional to the profile and the debug mode). Hence this can be
  /// used to monitor the application's performance in the wild.
  ///
  /// {@macro dart.ui.TimingsCallback.list}
  ///
  /// If this is null, no additional work will be done. If this is not null,
  /// Flutter spends less than 0.1ms every 1 second to report the timings
  /// (measured on iPhone6S). The 0.1ms is about 0.6% of 16ms (frame budget for
  /// 60fps), or 0.01% CPU usage per second.
  @override
  ui.TimingsCallback/*?*/ get onReportTimings => _onReportTimings;
  ui.TimingsCallback/*?*/ _onReportTimings;
  Zone/*!*/ _onReportTimingsZone = Zone.root;
  @override
  set onReportTimings(ui.TimingsCallback/*?*/ callback) {
    _onReportTimings = callback;
    _onReportTimingsZone = Zone.current;
  }

  /// Engine code should use this method instead of the callback directly.
  /// Otherwise zones won't work properly.
  void invokeOnReportTimings(List<ui.FrameTiming/*!*/>/*!*/ timings) {
    _invoke1<List<ui.FrameTiming>>(_onReportTimings, _onReportTimingsZone, timings);
  }


  @override
  ui.PlatformMessageCallback/*?*/ get onPlatformMessage => _onPlatformMessage;
  ui.PlatformMessageCallback/*?*/ _onPlatformMessage;
  Zone/*!*/ _onPlatformMessageZone = Zone.root;
  @override
  set onPlatformMessage(ui.PlatformMessageCallback/*?*/ callback) {
    _onPlatformMessage = callback;
    _onPlatformMessageZone = Zone.current;
  }

  /// Engine code should use this method instead of the callback directly.
  /// Otherwise zones won't work properly.
  void invokeOnPlatformMessage(String/*!*/ name, ByteData/*?*/ data, ui.PlatformMessageResponseCallback/*?*/ callback) {
    _invoke3<String, ByteData, ui.PlatformMessageResponseCallback>(
      _onPlatformMessage,
      _onPlatformMessageZone,
      name,
      data,
      callback,
    );
  }

  @override
  void sendPlatformMessage(
    String/*!*/ name,
    ByteData/*?*/ data,
    ui.PlatformMessageResponseCallback/*?*/ callback,
  ) {
    _sendPlatformMessage(
        name, data, _zonedPlatformMessageResponseCallback(callback));
  }

  /// Wraps the given [callback] in another callback that ensures that the
  /// original callback is called in the zone it was registered in.
  static ui.PlatformMessageResponseCallback _zonedPlatformMessageResponseCallback(ui.PlatformMessageResponseCallback/*?*/ callback) {
    if (callback == null)
      return null;

    // Store the zone in which the callback is being registered.
    final Zone registrationZone = Zone.current;

    return (ByteData data) {
      registrationZone.runUnaryGuarded(callback, data);
    };
  }

  void _sendPlatformMessage(
    String/*?*/ name,
    ByteData/*?*/ data,
    ui.PlatformMessageResponseCallback/*?*/ callback,
  ) {
    // In widget tests we want to bypass processing of platform messages.
    if (assertionsEnabled && ui.debugEmulateFlutterTesterEnvironment) {
      return;
    }

    if (_debugPrintPlatformMessages) {
      print('Sent platform message on channel: "$name"');
    }

    if (assertionsEnabled && name == 'flutter/debug-echo') {
      // Echoes back the data unchanged. Used for testing purposes.
      _replyToPlatformMessage(callback, data);
      return;
    }

    switch (name) {
      case 'flutter/assets':
        assert(ui.webOnlyAssetManager != null);
        final String url = utf8.decode(data.buffer.asUint8List());
        ui.webOnlyAssetManager.load(url).then((ByteData assetData) {
          _replyToPlatformMessage(callback, assetData);
        }, onError: (dynamic error) {
          html.window.console
              .warn('Error while trying to load an asset: $error');
          _replyToPlatformMessage(callback, null);
        });
        return;

      case 'flutter/platform':
        const MethodCodec codec = JSONMethodCodec();
        final MethodCall decoded = codec.decodeMethodCall(data);
        switch (decoded.method) {
          case 'SystemNavigator.pop':
            _browserHistory.exit().then((_) {
              _replyToPlatformMessage(
                  callback, codec.encodeSuccessEnvelope(true));
            });
            return;
          case 'HapticFeedback.vibrate':
            final String type = decoded.arguments;
            domRenderer.vibrate(_getHapticFeedbackDuration(type));
            _replyToPlatformMessage(callback, codec.encodeSuccessEnvelope(true));
            return;
          case 'SystemChrome.setApplicationSwitcherDescription':
            final Map<String, dynamic> arguments = decoded.arguments;
            domRenderer.setTitle(arguments['label']);
            domRenderer.setThemeColor(ui.Color(arguments['primaryColor']));
            _replyToPlatformMessage(callback, codec.encodeSuccessEnvelope(true));
            return;
          case 'SystemChrome.setPreferredOrientations':
            final List<dynamic> arguments = decoded.arguments;
            domRenderer.setPreferredOrientation(arguments).then((bool success) {
              _replyToPlatformMessage(callback,
                  codec.encodeSuccessEnvelope(success));
            });
            return;
          case 'SystemSound.play':
            // There are no default system sounds on web.
            _replyToPlatformMessage(callback, codec.encodeSuccessEnvelope(true));
            return;
          case 'Clipboard.setData':
            ClipboardMessageHandler().setDataMethodCall(decoded, callback);
            return;
          case 'Clipboard.getData':
            ClipboardMessageHandler().getDataMethodCall(callback);
            return;
        }
        break;

      case 'flutter/textinput':
        textEditing.channel.handleTextInput(data, callback);
        return;

      case 'flutter/mousecursor':
        const MethodCodec codec = StandardMethodCodec();
        final MethodCall decoded = codec.decodeMethodCall(data);
        final Map<dynamic, dynamic> arguments = decoded.arguments;
        switch (decoded.method) {
          case 'activateSystemCursor':
            MouseCursor.instance.activateSystemCursor(arguments['kind']);
        }
        return;

      case 'flutter/web_test_e2e':
        const MethodCodec codec = JSONMethodCodec();
        _replyToPlatformMessage(callback, codec.encodeSuccessEnvelope(
          _handleWebTestEnd2EndMessage(codec, data)
        ));
        return;

      case 'flutter/platform_views':
        if (experimentalUseSkia) {
          rasterizer.viewEmbedder.handlePlatformViewCall(data, callback);
        } else {
          ui.handlePlatformViewCall(data, callback);
        }
        return;

      case 'flutter/accessibility':
        // In widget tests we want to bypass processing of platform messages.
        final StandardMessageCodec codec = StandardMessageCodec();
        accessibilityAnnouncements.handleMessage(codec, data);
        _replyToPlatformMessage(callback, codec.encodeMessage(true));
        return;

      case 'flutter/navigation':
        const MethodCodec codec = JSONMethodCodec();
        final MethodCall decoded = codec.decodeMethodCall(data);
        final Map<String, dynamic> message = decoded.arguments;
        switch (decoded.method) {
          case 'routeUpdated':
          case 'routePushed':
          case 'routeReplaced':
            _browserHistory.setRouteName(message['routeName']);
            _replyToPlatformMessage(callback, codec.encodeSuccessEnvelope(true));
            break;
          case 'routePopped':
            _browserHistory.setRouteName(message['previousRouteName']);
            _replyToPlatformMessage(callback, codec.encodeSuccessEnvelope(true));
            break;
        }
        // As soon as Flutter starts taking control of the app navigation, we
        // should reset [_initialRouteName] to "/" so it doesn't have any
        // further effect after this point.
        _initialRouteName = '/';
        return;
    }

    if (pluginMessageCallHandler != null) {
      pluginMessageCallHandler(name, data, callback);
      return;
    }

    // TODO(flutter_web): Some Flutter widgets send platform messages that we
    // don't handle on web. So for now, let's just ignore them. In the future,
    // we should consider uncommenting the following "callback(null)" line.

    // Passing [null] to [callback] indicates that the platform message isn't
    // implemented. Look at [MethodChannel.invokeMethod] to see how [null] is
    // handled.
    // callback(null);
  }

  int _getHapticFeedbackDuration(String/*!*/ type) {
    switch (type) {
      case 'HapticFeedbackType.lightImpact':
        return DomRenderer.vibrateLightImpact;
      case 'HapticFeedbackType.mediumImpact':
        return DomRenderer.vibrateMediumImpact;
      case 'HapticFeedbackType.heavyImpact':
        return DomRenderer.vibrateHeavyImpact;
      case 'HapticFeedbackType.selectionClick':
        return DomRenderer.vibrateSelectionClick;
      default:
        return DomRenderer.vibrateLongPress;
    }
  }

  /// Requests that, at the next appropriate opportunity, the [onBeginFrame]
  /// and [onDrawFrame] callbacks be invoked.
  ///
  /// See also:
  ///
  ///  * [SchedulerBinding], the Flutter framework class which manages the
  ///    scheduling of frames.
  @override
  void scheduleFrame() {
    if (scheduleFrameCallback == null) {
      throw new Exception(
          'scheduleFrameCallback must be initialized first.');
    }
    scheduleFrameCallback();
  }

  /// Updates the application's rendering on the GPU with the newly provided
  /// [Scene]. This function must be called within the scope of the
  /// [onBeginFrame] or [onDrawFrame] callbacks being invoked. If this function
  /// is called a second time during a single [onBeginFrame]/[onDrawFrame]
  /// callback sequence or called outside the scope of those callbacks, the call
  /// will be ignored.
  ///
  /// To record graphical operations, first create a [PictureRecorder], then
  /// construct a [Canvas], passing that [PictureRecorder] to its constructor.
  /// After issuing all the graphical operations, call the
  /// [PictureRecorder.endRecording] function on the [PictureRecorder] to obtain
  /// the final [Picture] that represents the issued graphical operations.
  ///
  /// Next, create a [SceneBuilder], and add the [Picture] to it using
  /// [SceneBuilder.addPicture]. With the [SceneBuilder.build] method you can
  /// then obtain a [Scene] object, which you can display to the user via this
  /// [render] function.
  ///
  /// See also:
  ///
  ///  * [SchedulerBinding], the Flutter framework class which manages the
  ///    scheduling of frames.
  ///  * [RendererBinding], the Flutter framework class which manages layout and
  ///    painting.
  @override
  void render(ui.Scene/*!*/ scene, [ui.FlutterView/*!*/ view]) {
    if (experimentalUseSkia) {
      final LayerScene layerScene = scene;
      rasterizer.draw(layerScene.layerTree);
    } else {
      final SurfaceScene surfaceScene = scene;
      domRenderer.renderScene(surfaceScene.webOnlyRootElement);
    }
  }

  /// Additional accessibility features that may be enabled by the platform.
  ui.AccessibilityFeatures/*!*/ get accessibilityFeatures => configuration.accessibilityFeatures;

  /// A callback that is invoked when the value of [accessibilityFeatures] changes.
  ///
  /// The framework invokes this callback in the same zone in which the
  /// callback was set.
  ui.VoidCallback/*?*/ get onAccessibilityFeaturesChanged => _onAccessibilityFeaturesChanged;
  ui.VoidCallback/*?*/ _onAccessibilityFeaturesChanged;
  Zone/*!*/ _onAccessibilityFeaturesChangedZone = Zone.root;
  set onAccessibilityFeaturesChanged(ui.VoidCallback/*?*/ callback) {
    _onAccessibilityFeaturesChanged = callback;
    _onAccessibilityFeaturesChangedZone = Zone.current;
  }

  /// Engine code should use this method instead of the callback directly.
  /// Otherwise zones won't work properly.
  void invokeOnAccessibilityFeaturesChanged() {
    _invoke(_onAccessibilityFeaturesChanged, _onAccessibilityFeaturesChangedZone);
  }

  /// Change the retained semantics data about this window.
  ///
  /// If [semanticsEnabled] is true, the user has requested that this function
  /// be called whenever the semantic content of this window changes.
  ///
  /// In either case, this function disposes the given update, which means the
  /// semantics update cannot be used further.
  void updateSemantics(ui.SemanticsUpdate/*!*/ update) {
    EngineSemanticsOwner.instance.updateSemantics(update);
  }

  /// We use the first locale in the [locales] list instead of the browser's
  /// built-in `navigator.language` because browsers do not agree on the
  /// implementation.
  ///
  /// See also:
  ///
  /// * https://developer.mozilla.org/en-US/docs/Web/API/NavigatorLanguage/languages,
  ///   which explains browser quirks in the implementation notes.
  ui.Locale/*!*/ get locale => locales.first;

  /// The full system-reported supported locales of the device.
  ///
  /// This establishes the language and formatting conventions that application
  /// should, if possible, use to render their user interface.
  ///
  /// The list is ordered in order of priority, with lower-indexed locales being
  /// preferred over higher-indexed ones. The first element is the primary [locale].
  ///
  /// The [onLocaleChanged] callback is called whenever this value changes.
  ///
  /// See also:
  ///
  ///  * [WidgetsBindingObserver], for a mechanism at the widgets layer to
  ///    observe when this value changes.
  List<ui.Locale/*!*/>/*!*/ get locales => configuration.locales;

  /// The locale that the platform's native locale resolution system resolves to.
  ///
  /// This value may differ between platforms and is meant to allow Flutter's locale
  /// resolution algorithms access to a locale that is consistent with other apps
  /// on the device. Using this property is optional.
  ///
  /// This value may be used in a custom [localeListResolutionCallback] or used directly
  /// in order to arrive at the most appropriate locale for the app.
  ///
  /// See [locales], which is the list of locales the user/device prefers.
  ui.Locale/*?*/ computePlatformResolvedLocale(List<ui.Locale> supportedLocales) {
    // TODO(garyq): Implement on web.
    return null;
  }

  /// A callback that is invoked whenever [locale] changes value.
  ///
  /// The framework invokes this callback in the same zone in which the
  /// callback was set.
  ///
  /// See also:
  ///
  ///  * [WidgetsBindingObserver], for a mechanism at the widgets layer to
  ///    observe when this callback is invoked.
  ui.VoidCallback/*?*/ get onLocaleChanged => _onLocaleChanged;
  ui.VoidCallback/*?*/ _onLocaleChanged;
  Zone/*!*/ _onLocaleChangedZone = Zone.root; // ignore: unused_field
  set onLocaleChanged(ui.VoidCallback/*?*/ callback) {
    _onLocaleChanged = callback;
    _onLocaleChangedZone = Zone.current;
  }

  /// The locale used when we fail to get the list from the browser.
  static const ui.Locale/*!*/ _defaultLocale = const ui.Locale('en', 'US');

  /// Sets locales to an empty list.
  ///
  /// The empty list is not a valid value for locales. This is only used for
  /// testing locale update logic.
  void debugResetLocales() {
    _configuration = _configuration.copyWith(locales: const <ui.Locale>[]);
  }

  // Called by DomRenderer when browser languages change.
  void _updateLocales() {
    _configuration = _configuration.copyWith(locales: parseBrowserLanguages());
  }

  static List<ui.Locale/*!*/>/*!*/ parseBrowserLanguages() {
    // TODO(yjbanov): find a solution for IE
    final bool languagesFeatureMissing = !js_util.hasProperty(html.window.navigator, 'languages');
    if (languagesFeatureMissing || html.window.navigator.languages.isEmpty) {
      // To make it easier for the app code, let's not leave the locales list
      // empty. This way there's fewer corner cases for apps to handle.
      return const [_defaultLocale];
    }

    final List<ui.Locale> locales = <ui.Locale>[];
    for (final String language in html.window.navigator.languages) {
      final List<String> parts = language.split('-');
      if (parts.length > 1) {
        locales.add(ui.Locale(parts.first, parts.last));
      } else {
        locales.add(ui.Locale(language));
      }
    }

    assert(locales.isNotEmpty);
    return locales;
  }

  /// Engine code should use this method instead of the callback directly.
  /// Otherwise zones won't work properly.
  void invokeOnLocaleChanged() {
    _invoke(_onLocaleChanged, _onLocaleChangedZone);
  }

  /// The lifecycle state immediately after dart isolate initialization.
  ///
  /// This property will not be updated as the lifecycle changes.
  ///
  /// It is used to initialize [SchedulerBinding.lifecycleState] at startup
  /// with any buffered lifecycle state events.
  String/*?*/ get initialLifecycleState => 'AppLifecycleState.resumed';

  /// The system-reported text scale.
  ///
  /// This establishes the text scaling factor to use when rendering text,
  /// according to the user's platform preferences.
  ///
  /// The [onTextScaleFactorChanged] callback is called whenever this value
  /// changes.
  ///
  /// See also:
  ///
  ///  * [WidgetsBindingObserver], for a mechanism at the widgets layer to
  ///    observe when this value changes.
  double/*!*/ get textScaleFactor => configuration.textScaleFactor;

  /// The setting indicating whether time should always be shown in the 24-hour
  /// format.
  ///
  /// This option is used by [showTimePicker].
  bool/*!*/ get alwaysUse24HourFormat => configuration.alwaysUse24HourFormat;

  /// A callback that is invoked whenever [textScaleFactor] changes value.
  ///
  /// The framework invokes this callback in the same zone in which the
  /// callback was set.
  ///
  /// See also:
  ///
  ///  * [WidgetsBindingObserver], for a mechanism at the widgets layer to
  ///    observe when this callback is invoked.
  ui.VoidCallback/*?*/ get onTextScaleFactorChanged => _onTextScaleFactorChanged;
  ui.VoidCallback/*?*/ _onTextScaleFactorChanged;
  Zone/*!*/ _onTextScaleFactorChangedZone = Zone.root;
  set onTextScaleFactorChanged(ui.VoidCallback/*?*/ callback) {
    _onTextScaleFactorChanged = callback;
    _onTextScaleFactorChangedZone = Zone.current;
  }

  /// Engine code should use this method instead of the callback directly.
  /// Otherwise zones won't work properly.
  void invokeOnTextScaleFactorChanged() {
    _invoke(_onTextScaleFactorChanged, _onTextScaleFactorChangedZone);
  }

  /// The setting indicating the current brightness mode of the host platform.
  /// If the platform has no preference, [platformBrightness] defaults to [Brightness.light].
  ui.Brightness/*!*/ get platformBrightness => configuration.platformBrightness;

  /// Updates [_platformBrightness] and invokes [onPlatformBrightnessChanged]
  /// callback if [_platformBrightness] changed.
  void _updatePlatformBrightness(ui.Brightness/*!*/ value) {
    if (configuration.platformBrightness != value &&
        onPlatformBrightnessChanged != null) {
      _configuration = configuration.copyWith(platformBrightness: value);
      invokeOnPlatformConfigurationChanged();
      invokeOnPlatformBrightnessChanged();
    }
  }

  /// Reference to css media query that indicates the user theme preference on the web.
  final html.MediaQueryList/*!*/ _brightnessMediaQuery =
  html.window.matchMedia('(prefers-color-scheme: dark)');

  /// A callback that is invoked whenever [_brightnessMediaQuery] changes value.
  ///
  /// Updates the [_platformBrightness] with the new user preference.
  html.EventListener/*?*/ _brightnessMediaQueryListener;

  /// Set the callback function for listening changes in [_brightnessMediaQuery] value.
  void _addBrightnessMediaQueryListener() {
    _updatePlatformBrightness(_brightnessMediaQuery.matches
        ? ui.Brightness.dark
        : ui.Brightness.light);

    _brightnessMediaQueryListener = (html.Event event) {
      final html.MediaQueryListEvent mqEvent = event;
      _updatePlatformBrightness(
          mqEvent.matches ? ui.Brightness.dark : ui.Brightness.light);
    };
    _brightnessMediaQuery.addListener(_brightnessMediaQueryListener);
    registerHotRestartListener(() {
      _removeBrightnessMediaQueryListener();
    });
  }

  /// Remove the callback function for listening changes in [_brightnessMediaQuery] value.
  void _removeBrightnessMediaQueryListener() {
    _brightnessMediaQuery.removeListener(_brightnessMediaQueryListener);
    _brightnessMediaQueryListener = null;
  }

  /// A callback that is invoked whenever [platformBrightness] changes value.
  ///
  /// The framework invokes this callback in the same zone in which the
  /// callback was set.
  ///
  /// See also:
  ///
  ///  * [WidgetsBindingObserver], for a mechanism at the widgets layer to
  ///    observe when this callback is invoked.
  ui.VoidCallback/*?*/ get onPlatformBrightnessChanged => _onPlatformBrightnessChanged;
  ui.VoidCallback/*?*/ _onPlatformBrightnessChanged;
  Zone/*!*/ _onPlatformBrightnessChangedZone = Zone.root;
  set onPlatformBrightnessChanged(ui.VoidCallback/*?*/ callback) {
    _onPlatformBrightnessChanged = callback;
    _onPlatformBrightnessChangedZone = Zone.current;
  }

  /// Engine code should use this method instead of the callback directly.
  /// Otherwise zones won't work properly.
  void invokeOnPlatformBrightnessChanged() {
    _invoke(_onPlatformBrightnessChanged, _onPlatformBrightnessChangedZone);
  }

  /// Whether the user has requested that [updateSemantics] be called when
  /// the semantic contents of window changes.
  ///
  /// The [onSemanticsEnabledChanged] callback is called whenever this value
  /// changes.
  bool/*!*/ get semanticsEnabled => configuration.semanticsEnabled;

  /// A callback that is invoked when the value of [semanticsEnabled] changes.
  ///
  /// The framework invokes this callback in the same zone in which the
  /// callback was set.
  ui.VoidCallback/*?*/ get onSemanticsEnabledChanged => _onSemanticsEnabledChanged;
  ui.VoidCallback/*?*/ _onSemanticsEnabledChanged;
  Zone/*!*/ _onSemanticsEnabledChangedZone = Zone.root;
  set onSemanticsEnabledChanged(ui.VoidCallback/*?*/ callback) {
    _onSemanticsEnabledChanged = callback;
    _onSemanticsEnabledChangedZone = Zone.current;
  }

  /// Engine code should use this method instead of the callback directly.
  /// Otherwise zones won't work properly.
  void invokeOnSemanticsEnabledChanged() {
    _invoke(_onSemanticsEnabledChanged, _onSemanticsEnabledChangedZone);
  }

  /// A callback that is invoked whenever the user requests an action to be
  /// performed.
  ///
  /// This callback is used when the user expresses the action they wish to
  /// perform based on the semantics supplied by [updateSemantics].
  ///
  /// The framework invokes this callback in the same zone in which the
  /// callback was set.
  ui.SemanticsActionCallback/*?*/ get onSemanticsAction => _onSemanticsAction;
  ui.SemanticsActionCallback/*?*/ _onSemanticsAction;
  Zone/*!*/ _onSemanticsActionZone = Zone.root;
  set onSemanticsAction(ui.SemanticsActionCallback/*?*/ callback) {
    _onSemanticsAction = callback;
    _onSemanticsActionZone = Zone.current;
  }

  /// Engine code should use this method instead of the callback directly.
  /// Otherwise zones won't work properly.
  void invokeOnSemanticsAction(int/*!*/ id, ui.SemanticsAction/*!*/ action, ByteData/*?*/ args) {
    _invoke3<int, ui.SemanticsAction, ByteData>(_onSemanticsAction, _onSemanticsActionZone, id, action, args);
  }

  /// The route or path that the embedder requested when the application was
  /// launched.
  ///
  /// This will be the string "`/`" if no particular route was requested.
  ///
  /// ## Android
  ///
  /// On Android, calling
  /// [`FlutterView.setInitialRoute`](/javadoc/io/flutter/view/FlutterView.html#setInitialRoute-java.lang.String-)
  /// will set this value. The value must be set sufficiently early, i.e. before
  /// the [runApp] call is executed in Dart, for this to have any effect on the
  /// framework. The `createFlutterView` method in your `FlutterActivity`
  /// subclass is a suitable time to set the value. The application's
  /// `AndroidManifest.xml` file must also be updated to have a suitable
  /// [`<intent-filter>`](https://developer.android.com/guide/topics/manifest/intent-filter-element.html).
  ///
  /// ## iOS
  ///
  /// On iOS, calling
  /// [`FlutterViewController.setInitialRoute`](/objcdoc/Classes/FlutterViewController.html#/c:objc%28cs%29FlutterViewController%28im%29setInitialRoute:)
  /// will set this value. The value must be set sufficiently early, i.e. before
  /// the [runApp] call is executed in Dart, for this to have any effect on the
  /// framework. The `application:didFinishLaunchingWithOptions:` method is a
  /// suitable time to set this value.
  ///
  /// See also:
  ///
  ///  * [Navigator], a widget that handles routing.
  ///  * [SystemChannels.navigation], which handles subsequent navigation
  ///    requests from the embedder.
  @override
  String/*!*/ get initialRouteName => _initialRouteName ??= _browserHistory.currentPath;

  /// Lazily initialized when the `initialRouteName` getter is invoked.
  ///
  /// The reason for the lazy initialization is to give enough time for the app
  /// to set [locationStrategy] in `lib/src/ui/initialization.dart`.
  String/*?*/ _initialRouteName;

  /// Handles the browser history integration to allow users to use the back
  /// button, etc.
  final BrowserHistory/*!*/ _browserHistory = BrowserHistory();

  /// Simulates clicking the browser's back button.
  Future<void> webOnlyBack() => _browserHistory.back();

  /// Change the strategy to use for handling browser history location.
  /// Setting this member will automatically update [_browserHistory].
  ///
  /// By setting this to null, the browser history will be disabled.
  set locationStrategy(LocationStrategy/*!*/ strategy) {
    _browserHistory.locationStrategy = strategy;
  }

  /// Returns the currently active location strategy.
  @visibleForTesting
  LocationStrategy get locationStrategy => _browserHistory.locationStrategy;

  @visibleForTesting
  Rasterizer/*?*/ rasterizer = experimentalUseSkia ? Rasterizer(Surface()) : null;

  /// In Flutter, platform messages are exchanged between threads so the
  /// messages and responses have to be exchanged asynchronously. We simulate
  /// that by adding a zero-length delay to the reply.
  void _replyToPlatformMessage(
      ui.PlatformMessageResponseCallback callback,
      ByteData data,
      ) {
    Future<void>.delayed(Duration.zero).then((_) {
      if (callback != null) {
        callback(data);
      }
    });
  }
}

bool _handleWebTestEnd2EndMessage(MethodCodec/*!*/ codec, ByteData/*!*/ data) {
  final MethodCall decoded = codec.decodeMethodCall(data);
  double ratio = double.parse(decoded.arguments);
  switch(decoded.method) {
    case 'setDevicePixelRatio':
      window.debugOverrideDevicePixelRatio(ratio);
      window.onMetricsChanged();
      return true;
  }
  return false;
}

/// Invokes [callback] inside the given [zone].
void _invoke(void callback()/*?*/, Zone/*!*/ zone) {
  if (callback == null)
    return;

  assert(zone != null);

  if (identical(zone, Zone.current)) {
    callback();
  } else {
    zone.runGuarded(callback);
  }
}

/// Invokes [callback] inside the given [zone] passing it [arg].
void _invoke1<A>(void callback(A a)/*?*/, Zone zone/*!*/, A arg) {
  if (callback == null)
    return;

  assert(zone != null);

  if (identical(zone, Zone.current)) {
    callback(arg);
  } else {
    zone.runUnaryGuarded<A>(callback, arg);
  }
}

/// Invokes [callback] inside the given [zone] passing it [arg1], [arg2], and [arg3].
void _invoke3<A1, A2, A3>(
    void callback(A1 a1, A2 a2, A3 a3)/*?*/,
    Zone/*!*/ zone,
    A1 arg1,
    A2 arg2,
    A3 arg3,
  ) {
  if (callback == null)
    return;

  assert(zone != null);

  if (identical(zone, Zone.current)) {
    callback(arg1, arg2, arg3);
  } else {
    zone.runGuarded(() {
      callback(arg1, arg2, arg3);
    });
  }
}

