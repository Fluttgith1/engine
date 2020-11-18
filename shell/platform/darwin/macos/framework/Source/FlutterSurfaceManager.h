// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#import <Cocoa/Cocoa.h>
#include <QuartzCore/CAMetalLayer.h>

#include "flutter/shell/platform/darwin/macos/framework/Source/FlutterResizableBackingStoreProvider.h"

@protocol FlutterSurfaceManager

- (void)ensureSurfaceSize:(CGSize)size;

- (void)swapBuffers;

- (FlutterBackingStoreDescriptor*)getBackingStore;

@end

// Manages the IOSurfaces for FlutterView
@interface FlutterGLSurfaceManager : NSObject <FlutterSurfaceManager>

- (nullable instancetype)initWithLayer:(nonnull CALayer*)containingLayer
                         openGLContext:(nonnull NSOpenGLContext*)opengLContext;

@end

@interface FlutterMetalSurfaceManager : NSObject <FlutterSurfaceManager>

- (nullable instancetype)initWithDevice:(nonnull id<MTLDevice>)device
                        andCommandQueue:(nonnull id<MTLCommandQueue>)commandQueue
                        andCAMetalLayer:(nonnull CAMetalLayer*)layer;

@end
