// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/shell/platform/darwin/ios/framework/Headers/FlutterChannels.h"

#pragma mark - Basic message channel

@implementation FlutterMessageChannel {
  NSObject<FlutterBinaryMessenger>* _messenger;
  NSString* _name;
  NSObject<FlutterMessageCodec>* _codec;
}
+ (instancetype)messageChannelNamed:(NSString*)name
                    binaryMessenger:(NSObject<FlutterBinaryMessenger>*)messenger
                              codec:(NSObject<FlutterMessageCodec>*)codec {
  return [[[FlutterMessageChannel alloc] initWithName:name
                                      binaryMessenger:messenger
                                                codec:codec] autorelease];
}

- (instancetype)initWithName:(NSString*)name
             binaryMessenger:(NSObject<FlutterBinaryMessenger>*)messenger
                       codec:(NSObject<FlutterMessageCodec>*)codec {
  if (self = [super init]) {
    _name = [name retain];
    _messenger = [messenger retain];
    _codec = [codec retain];
  }
  return self;
}

- (void)dealloc {
  [_name release];
  [_messenger release];
  [_codec release];
  [super dealloc];
}

- (void)sendMessage:(id)message {
  [_messenger sendBinaryMessage:[_codec encode:message] channelName:_name];
}

- (void)sendMessage:(id)message replyHandler:(FlutterReplyHandler)handler {
  [_messenger sendBinaryMessage:[_codec encode:message]
                    channelName:_name
             binaryReplyHandler:^(NSData* reply) {
               if (handler)
                 handler([_codec decode:reply]);
             }];
}

- (void)setMessageHandler:(FlutterMessageHandler)handler {
  if (handler) {
    [_messenger setBinaryMessageHandlerOnChannel:_name
                            binaryMessageHandler:^(
                                NSData* message,
                                FlutterBinaryReplyHandler replyHandler) {
                              handler([_codec decode:message], ^(id reply) {
                                replyHandler([_codec encode:reply]);
                              });
                            }];
  } else {
    [_messenger setBinaryMessageHandlerOnChannel:_name
                            binaryMessageHandler:nil];
  }
}
@end

#pragma mark - Method channel

@implementation FlutterError
+ (instancetype)errorWithCode:(NSString*)code
                      message:(NSString*)message
                      details:(id)details {
  return
      [[[FlutterError alloc] initWithCode:code message:message details:details]
          autorelease];
}

- (instancetype)initWithCode:(NSString*)code
                     message:(NSString*)message
                     details:(id)details {
  NSAssert(code, @"Code cannot be nil");
  if (self = [super init]) {
    _code = [code retain];
    _message = [message retain];
    _details = [details retain];
  }
  return self;
}

- (void)dealloc {
  [_code release];
  [_message release];
  [_details release];
  [super dealloc];
}

- (BOOL)isEqual:(id)object {
  if (self == object)
    return YES;
  if (![object isKindOfClass:[FlutterError class]])
    return NO;
  return [self isEqualToError:(FlutterError*)object];
}

- (BOOL)isEqualToError:(FlutterError*)other {
  return other && [self.code isEqualToString:other.code] &&
         ((!self.message && !other.message) ||
          [self.message isEqualToString:other.message]) &&
         ((!self.details && !other.details) ||
          [self.details isEqual:other.details]);
}

- (NSUInteger)hash {
  return [self.code hash] ^ [self.message hash] ^ [self.details hash];
}
@end

@implementation FlutterMethodCall
+ (instancetype)methodCallWithMethodName:(NSString*)method
                               arguments:(id)arguments {
  return [[[FlutterMethodCall alloc] initWithMethodName:method
                                              arguments:arguments] autorelease];
}

- (instancetype)initWithMethodName:(NSString*)method arguments:(id)arguments {
  NSAssert(method, @"Method name cannot be nil");
  if (self = [super init]) {
    _method = [method retain];
    _arguments = [arguments retain];
  }
  return self;
}

- (void)dealloc {
  [_method release];
  [_arguments release];
  [super dealloc];
}

- (BOOL)isEqual:(id)object {
  if (self == object)
    return YES;
  if (![object isKindOfClass:[FlutterMethodCall class]])
    return NO;
  return [self isEqualToMethodCall:(FlutterMethodCall*)object];
}

- (BOOL)isEqualToMethodCall:(FlutterMethodCall*)other {
  return other && [self.method isEqualToString:[other method]] &&
         ((!self.arguments && !other.arguments) ||
          [self.arguments isEqual:other.arguments]);
}

- (NSUInteger)hash {
  return [self.method hash] ^ [self.arguments hash];
}
@end

@implementation FlutterMethodChannel {
  NSObject<FlutterBinaryMessenger>* _messenger;
  NSString* _name;
  NSObject<FlutterMethodCodec>* _codec;
}

+ (instancetype)methodChannelNamed:(NSString*)name
                   binaryMessenger:(NSObject<FlutterBinaryMessenger>*)messenger
                             codec:(NSObject<FlutterMethodCodec>*)codec {
  return [[[FlutterMethodChannel alloc] initWithName:name
                                     binaryMessenger:messenger
                                               codec:codec] autorelease];
}

- (instancetype)initWithName:(NSString*)name
             binaryMessenger:(NSObject<FlutterBinaryMessenger>*)messenger
                       codec:(NSObject<FlutterMethodCodec>*)codec {
  if (self = [super init]) {
    _name = [name retain];
    _messenger = [messenger retain];
    _codec = [codec retain];
  }
  return self;
}

- (void)dealloc {
  [_name release];
  [_messenger release];
  [_codec release];
  [super dealloc];
}

- (void)invokeMethod:(NSString*)method arguments:(id)arguments {
  [_messenger
      sendBinaryMessage:
          [_codec encodeMethodCall:[FlutterMethodCall
                                       methodCallWithMethodName:method
                                                      arguments:arguments]]
            channelName:_name];
}

- (void)invokeMethod:(NSString*)method
           arguments:(id)arguments
      resultReceiver:(FlutterResultReceiver)resultReceiver {
  [_messenger
       sendBinaryMessage:
           [_codec encodeMethodCall:[FlutterMethodCall
                                        methodCallWithMethodName:method
                                                       arguments:arguments]]
             channelName:_name
      binaryReplyHandler:^(NSData* reply) {
        if (resultReceiver) {
          FlutterError* flutterError = nil;
          id result = [_codec decodeEnvelope:reply error:&flutterError];
          resultReceiver(result, flutterError);
        }
      }];
}

- (void)setMethodCallHandler:(FlutterMethodCallHandler)handler {
  [_messenger
      setBinaryMessageHandlerOnChannel:_name
                  binaryMessageHandler:^(NSData* message,
                                         FlutterBinaryReplyHandler reply) {
                    FlutterMethodCall* call = [_codec decodeMethodCall:message];
                    handler(call, ^(id result, FlutterError* error) {
                      if (error)
                        reply([_codec encodeErrorEnvelope:error]);
                      else
                        reply([_codec encodeSuccessEnvelope:result]);
                    });
                  }];
}

- (void)setStreamHandler:(FlutterStreamHandler)handler {
  if (!handler) {
    [_messenger setBinaryMessageHandlerOnChannel:_name
                            binaryMessageHandler:nil];
    return;
  }
  [_messenger
      setBinaryMessageHandlerOnChannel:_name
                  binaryMessageHandler:^(NSData* message,
                                         FlutterBinaryReplyHandler reply) {
                    FlutterMethodCall* call = [_codec decodeMethodCall:message];
                    handler(
                        call,
                        ^(id result, FlutterError* error) {
                          if (error)
                            reply([_codec encodeErrorEnvelope:error]);
                          else
                            reply([_codec encodeSuccessEnvelope:nil]);
                        },
                        ^(id event, FlutterError* error, BOOL done) {
                          if (error)
                            [_messenger
                                sendBinaryMessage:[_codec
                                                      encodeErrorEnvelope:error]
                                      channelName:_name];
                          else if (done)
                            [_messenger sendBinaryMessage:[NSData data]
                                              channelName:_name];
                          else
                            [_messenger sendBinaryMessage:
                                            [_codec encodeSuccessEnvelope:event]
                                              channelName:_name];
                        });
                  }];
}
@end
