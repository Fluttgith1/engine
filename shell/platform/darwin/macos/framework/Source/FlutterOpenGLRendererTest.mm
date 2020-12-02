// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#import <OCMock/OCMock.h>

#import "flutter/shell/platform/darwin/macos/framework/Headers/FlutterEngine.h"
#import "flutter/shell/platform/darwin/macos/framework/Source/FlutterDartProject_Internal.h"
#import "flutter/shell/platform/darwin/macos/framework/Source/FlutterEngine_Internal.h"
#include "flutter/shell/platform/embedder/embedder.h"
#include "flutter/shell/platform/embedder/test_utils/proc_table_replacement.h"
#include "flutter/testing/testing.h"

namespace flutter::testing {

namespace {
// Returns an engine configured for the text fixture resource configuration.
FlutterEngine* CreateTestEngine() {
  NSString* fixtures = @(testing::GetFixturesPath());
  FlutterDartProject* project = [[FlutterDartProject alloc]
      initWithAssetsPath:fixtures
             ICUDataPath:[fixtures stringByAppendingString:@"/icudtl.dat"]];
  return [[FlutterEngine alloc] initWithName:@"test" project:project allowHeadlessExecution:true];
}
}  // namespace

TEST(FlutterOpenGLRenderer, RegisterExternalTexture) {
  FlutterEngine* engine = CreateTestEngine();
  EXPECT_TRUE([engine runWithEntrypoint:@"main"]);

  id<FlutterTexture> flutterTexture = OCMProtocolMock(@protocol(FlutterTexture));
  bool called = false;

  engine.embedderAPI.RegisterExternalTexture =
      MOCK_ENGINE_PROC(RegisterExternalTexture, [&](auto engine, int64_t textureIdentifier) {
        called = true;
        EXPECT_EQ(textureIdentifier, reinterpret_cast<int64_t>(flutterTexture));
        return kSuccess;
      });

  [engine.openGLRenderer registerTexture:flutterTexture];
  EXPECT_TRUE(called);

  [engine shutDownEngine];
}

TEST(FlutterOpenGLRenderer, UnregisterExternalTexture) {
  FlutterEngine* engine = CreateTestEngine();
  EXPECT_TRUE([engine runWithEntrypoint:@"main"]);

  id<FlutterTexture> flutterTexture = OCMProtocolMock(@protocol(FlutterTexture));
  bool called = false;

  int64_t registeredTextureId = [engine.openGLRenderer registerTexture:flutterTexture];
  engine.embedderAPI.UnregisterExternalTexture =
      MOCK_ENGINE_PROC(UnregisterExternalTexture, [&](auto engine, int64_t textureIdentifier) {
        called = true;
        EXPECT_EQ(textureIdentifier, registeredTextureId);
        return kSuccess;
      });

  [engine.openGLRenderer unregisterTexture:registeredTextureId];
  EXPECT_TRUE(called);

  [engine shutDownEngine];
}

TEST(FlutterOpenGLRenderer, MarkExternalTextureFrameAvailable) {
  FlutterEngine* engine = CreateTestEngine();
  EXPECT_TRUE([engine runWithEntrypoint:@"main"]);

  id<FlutterTexture> flutterTexture = OCMProtocolMock(@protocol(FlutterTexture));
  bool called = false;

  int64_t registeredTextureId = [engine.openGLRenderer registerTexture:flutterTexture];
  engine.embedderAPI.MarkExternalTextureFrameAvailable = MOCK_ENGINE_PROC(
      MarkExternalTextureFrameAvailable, [&](auto engine, int64_t textureIdentifier) {
        called = true;
        EXPECT_EQ(textureIdentifier, registeredTextureId);
        return kSuccess;
      });

  [engine.openGLRenderer textureFrameAvailable:registeredTextureId];
  EXPECT_TRUE(called);

  [engine shutDownEngine];
}

}  // namespace flutter::testing
