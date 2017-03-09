// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_FLUTTERCHANNELS_H_
#define FLUTTER_FLUTTERCHANNELS_H_

#include "FlutterCodecs.h"
#include "FlutterViewController.h"

typedef void (^FlutterReplyHandler)(id reply);
typedef void (^FlutterMessageHandler)(id message, FlutterReplyHandler replyHandler);

FLUTTER_EXPORT
@interface FlutterMessageChannel : NSObject
+ (instancetype) withController:(FlutterViewController*)controller
                           name:(NSString*)name
                          codec:(NSObject<FlutterMessageCodec>*)codec;
- (void) send:(id)message andHandleReplyWith:(FlutterReplyHandler)handler;
- (void) handleMessagesWith:(FlutterMessageHandler)handler;
@end

typedef void (^FlutterResultReceiver)(id successResult, FlutterError* errorResult);
typedef void (^FlutterEventReceiver)(id successEvent, FlutterError* errorEvent, BOOL done);
typedef void (^FlutterMethodCallHandler)(FlutterMethodCall* call, FlutterResultReceiver resultReceiver);
typedef void (^FlutterStreamHandler)(FlutterMethodCall* call, FlutterResultReceiver resultReceiver, FlutterEventReceiver eventReceiver);

FLUTTER_EXPORT
@interface FlutterMethodChannel : NSObject
+ (instancetype)withController:(FlutterViewController*)controller
                          name:(NSString*)name
                         codec:(NSObject<FlutterMethodCodec>*)codec;
- (void) handleMethodCallsWith:(FlutterMethodCallHandler)handler;
- (void) handleStreamWith:(FlutterStreamHandler)handler;
@end

#endif  // FLUTTER_FLUTTERCHANNELS_H_
