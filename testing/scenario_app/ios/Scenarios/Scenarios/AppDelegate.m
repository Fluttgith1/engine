// Copyright 2019 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "AppDelegate.h"
#import "FlutterEngine+ScenariosTest.h"
#import "ScreenBeforeFlutter.h"
#import "TextPlatformView.h"

@interface NoStatusBarFlutterViewController : FlutterViewController

@end

@implementation NoStatusBarFlutterViewController
- (BOOL)prefersStatusBarHidden {
  return YES;
}
@end

@implementation AppDelegate

- (BOOL)application:(UIApplication*)application
    didFinishLaunchingWithOptions:(NSDictionary*)launchOptions {
  self.window = [[UIWindow alloc] initWithFrame:[[UIScreen mainScreen] bounds]];

  // This argument is used by the XCUITest for Platform Views so that the app
  // under test will create platform views.
  // If the test is one of the platform view golden tests,
  // the launchArgsMap should match the one in the `PlatformVieGoldenTestManager`.
  NSDictionary<NSString*, NSString*>* launchArgsMap = @{
    @"--platform-view" : @"platform_view",
    @"--platform-view-no-overlay-intersection" : @"platform_view_no_overlay_intersection",
    @"--platform-view-two-intersecting-overlays" : @"platform_view_two_intersecting_overlays",
    @"--platform-view-partial-intersection" : @"platform_view_partial_intersection",
    @"--platform-view-one-overlay-two-intersecting-overlays" :
        @"platform_view_one_overlay_two_intersecting_overlays",
    @"--platform-view-multiple-without-overlays" : @"platform_view_multiple_without_overlays",
    @"--platform-view-max-overlays" : @"platform_view_max_overlays",
    @"--platform-view-multiple" : @"platform_view_multiple",
    @"--platform-view-multiple-background-foreground" :
        @"platform_view_multiple_background_foreground",
    @"--platform-view-cliprect" : @"platform_view_cliprect",
    @"--platform-view-cliprrect" : @"platform_view_cliprrect",
    @"--platform-view-clippath" : @"platform_view_clippath",
    @"--platform-view-transform" : @"platform_view_transform",
    @"--platform-view-opacity" : @"platform_view_opacity",
    @"--platform-view-rotate" : @"platform_view_rotate",
    @"--gesture-reject-after-touches-ended" : @"platform_view_gesture_reject_after_touches_ended",
    @"--gesture-reject-eager" : @"platform_view_gesture_reject_eager",
    @"--gesture-accept" : @"platform_view_gesture_accept",
    @"--tap-status-bar" : @"tap_status_bar",
  };
  __block NSString* platformViewTestName = nil;
  [launchArgsMap
      enumerateKeysAndObjectsUsingBlock:^(NSString* argument, NSString* testName, BOOL* stop) {
        if ([[[NSProcessInfo processInfo] arguments] containsObject:argument]) {
          platformViewTestName = testName;
          *stop = YES;
        }
      }];

  if (platformViewTestName) {
    [self readyContextForPlatformViewTests:platformViewTestName];
  } else if ([[[NSProcessInfo processInfo] arguments] containsObject:@"--screen-before-flutter"]) {
    self.window.rootViewController = [[ScreenBeforeFlutter alloc] initWithEngineRunCompletion:nil];
  } else {
    self.window.rootViewController = [[UIViewController alloc] init];
  }

  [self.window makeKeyAndVisible];
  return [super application:application didFinishLaunchingWithOptions:launchOptions];
}

- (void)readyContextForPlatformViewTests:(NSString*)scenarioIdentifier {
  FlutterEngine* engine = [[FlutterEngine alloc] initWithName:@"PlatformViewTest" project:nil];
  [engine runWithEntrypoint:nil];

  FlutterViewController* flutterViewController;
  if ([scenarioIdentifier isEqualToString:@"tap_status_bar"]) {
    flutterViewController = [[FlutterViewController alloc] initWithEngine:engine
                                                                  nibName:nil
                                                                   bundle:nil];
  } else {
    flutterViewController = [[NoStatusBarFlutterViewController alloc] initWithEngine:engine
                                                                             nibName:nil
                                                                              bundle:nil];
  }
  [engine.binaryMessenger
      setMessageHandlerOnChannel:@"scenario_status"
            binaryMessageHandler:^(NSData* _Nullable message, FlutterBinaryReply _Nonnull reply) {
              [engine.binaryMessenger
                  sendOnChannel:@"set_scenario"
                        message:[scenarioIdentifier dataUsingEncoding:NSUTF8StringEncoding]];
            }];
  [engine.binaryMessenger
      setMessageHandlerOnChannel:@"touches_scenario"
            binaryMessageHandler:^(NSData* _Nullable message, FlutterBinaryReply _Nonnull reply) {
              NSDictionary* dict = [NSJSONSerialization JSONObjectWithData:message
                                                                   options:0
                                                                     error:nil];
              UITextField* text = [[UITextField alloc] initWithFrame:CGRectMake(0, 0, 300, 100)];
              text.text = dict[@"change"];
              [flutterViewController.view addSubview:text];
            }];
  TextPlatformViewFactory* textPlatformViewFactory =
      [[TextPlatformViewFactory alloc] initWithMessenger:flutterViewController.binaryMessenger];
  NSObject<FlutterPluginRegistrar>* registrar =
      [flutterViewController.engine registrarForPlugin:@"scenarios/TextPlatformViewPlugin"];
  [registrar registerViewFactory:textPlatformViewFactory
                                withId:@"scenarios/textPlatformView"
      gestureRecognizersBlockingPolicy:FlutterPlatformViewGestureRecognizersBlockingPolicyEager];
  [registrar registerViewFactory:textPlatformViewFactory
                                withId:@"scenarios/textPlatformView_blockPolicyUntilTouchesEnded"
      gestureRecognizersBlockingPolicy:
          FlutterPlatformViewGestureRecognizersBlockingPolicyWaitUntilTouchesEnded];
  self.window.rootViewController = flutterViewController;
}

@end
