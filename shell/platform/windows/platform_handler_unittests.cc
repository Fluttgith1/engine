// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/shell/platform/windows/platform_handler.h"

#include <memory>

#include "flutter/shell/platform/common/json_method_codec.h"
#include "flutter/shell/platform/windows/flutter_windows_view.h"
#include "flutter/shell/platform/windows/testing/mock_window_binding_handler.h"
#include "flutter/shell/platform/windows/testing/test_binary_messenger.h"
#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "rapidjson/document.h"

namespace flutter {
namespace testing {

namespace {
using ::testing::_;
using ::testing::NiceMock;
using ::testing::Return;

static constexpr char kChannelName[] = "flutter/platform";

static constexpr char kClipboardGetDataMessage[] =
    "{\"method\":\"Clipboard.getData\",\"args\":\"text/plain\"}";
static constexpr char kClipboardGetDataFakeContentTypeMessage[] =
    "{\"method\":\"Clipboard.getData\",\"args\":\"text/madeupcontenttype\"}";
static constexpr char kClipboardHasStringsMessage[] =
    "{\"method\":\"Clipboard.hasStrings\",\"args\":\"text/plain\"}";
static constexpr char kClipboardHasStringsFakeContentTypeMessage[] =
    "{\"method\":\"Clipboard.hasStrings\",\"args\":\"text/madeupcontenttype\"}";
static constexpr char kClipboardSetDataMessage[] =
    "{\"method\":\"Clipboard.setData\",\"args\":{\"text\":\"hello\"}}";
static constexpr char kClipboardSetDataUnknownTypeMessage[] =
    "{\"method\":\"Clipboard.setData\",\"args\":{\"madeuptype\":\"hello\"}}";
static constexpr char kSystemSoundTypeAlertMessage[] =
    "{\"method\":\"SystemSound.play\",\"args\":\"SystemSoundType.alert\"}";

static constexpr int kAccessDeniedErrorCode = 5;
static constexpr int kErrorSuccess = 0;
static constexpr int kArbitraryErrorCode = 1;

// Test implementation of PlatformHandler to allow testing the PlatformHandler
// logic.
class TestPlatformHandler : public PlatformHandler {
 public:
  explicit TestPlatformHandler(
      BinaryMessenger* messenger,
      FlutterWindowsView* view,
      std::optional<std::function<std::unique_ptr<ScopedClipboardInterface>()>>
          scoped_clipboard_provider = std::nullopt)
      : PlatformHandler(messenger, view, scoped_clipboard_provider) {}

  virtual ~TestPlatformHandler() = default;

  MOCK_METHOD2(GetPlainText,
               void(std::unique_ptr<MethodResult<rapidjson::Document>>,
                    std::string_view key));
  MOCK_METHOD1(GetHasStrings,
               void(std::unique_ptr<MethodResult<rapidjson::Document>>));
  MOCK_METHOD2(SetPlainText,
               void(const std::string&,
                    std::unique_ptr<MethodResult<rapidjson::Document>>));
  MOCK_METHOD2(SystemSoundPlay,
               void(const std::string&,
                    std::unique_ptr<MethodResult<rapidjson::Document>>));
};

// A test version of the private ScopedClipboard.
class MockScopedClipboard : public ScopedClipboardInterface {
 public:
  MOCK_METHOD(int, Open, (HWND window), (override));
  MOCK_METHOD(bool, HasString, (), (override));
  MOCK_METHOD((std::variant<std::wstring, int>), GetString, (), (override));
  MOCK_METHOD(int, SetString, (const std::wstring string), (override));
};

std::string SimulatePlatformMessage(TestBinaryMessenger* messenger,
                                    const uint8_t* message,
                                    size_t message_size) {
  std::string result;
  EXPECT_TRUE(messenger->SimulateEngineMessage(
      kChannelName, message, message_size,
      [result = &result](const uint8_t* reply, size_t reply_size) {
        std::string response(reinterpret_cast<const char*>(reply), reply_size);

        *result = response;
      }));

  return result;
}

}  // namespace

TEST(PlatformHandler, GetClipboardData) {
  TestBinaryMessenger messenger;
  FlutterWindowsView view(
      std::make_unique<NiceMock<MockWindowBindingHandler>>());

  PlatformHandler platform_handler(&messenger, &view, []() {
    auto clipboard = std::make_unique<MockScopedClipboard>();

    EXPECT_CALL(*clipboard.get(), Open)
        .Times(1)
        .WillOnce(Return(kErrorSuccess));
    EXPECT_CALL(*clipboard.get(), HasString).Times(1).WillOnce(Return(true));
    EXPECT_CALL(*clipboard.get(), GetString)
        .Times(1)
        .WillOnce(Return(std::wstring(L"Hello world")));

    return clipboard;
  });

  std::string result = SimulatePlatformMessage(
      &messenger, reinterpret_cast<const uint8_t*>(kClipboardGetDataMessage),
      sizeof(kClipboardGetDataMessage));

  EXPECT_EQ(result, "[{\"text\":\"Hello world\"}]");
}

TEST(PlatformHandler, GetClipboardDataRejectsUnknownContentType) {
  TestBinaryMessenger messenger;
  FlutterWindowsView view(
      std::make_unique<NiceMock<MockWindowBindingHandler>>());
  PlatformHandler platform_handler(&messenger, &view);

  // Requesting an unknown content type is an error.
  std::string result = SimulatePlatformMessage(
      &messenger,
      reinterpret_cast<const uint8_t*>(kClipboardGetDataFakeContentTypeMessage),
      sizeof(kClipboardGetDataFakeContentTypeMessage));

  EXPECT_EQ(result, "[\"Clipboard error\",\"Unknown clipboard format\",null]");
}

TEST(PlatformHandler, GetClipboardDataReportsOpenFailure) {
  TestBinaryMessenger messenger;
  FlutterWindowsView view(
      std::make_unique<NiceMock<MockWindowBindingHandler>>());

  PlatformHandler platform_handler(&messenger, &view, []() {
    auto clipboard = std::make_unique<MockScopedClipboard>();

    EXPECT_CALL(*clipboard.get(), Open)
        .Times(1)
        .WillOnce(Return(kArbitraryErrorCode));

    return clipboard;
  });

  std::string result = SimulatePlatformMessage(
      &messenger, reinterpret_cast<const uint8_t*>(kClipboardGetDataMessage),
      sizeof(kClipboardGetDataMessage));

  EXPECT_EQ(result, "[\"Clipboard error\",\"Unable to open clipboard\",1]");
}

TEST(PlatformHandler, GetClipboardDataReportsGetDataFailure) {
  TestBinaryMessenger messenger;
  FlutterWindowsView view(
      std::make_unique<NiceMock<MockWindowBindingHandler>>());

  PlatformHandler platform_handler(&messenger, &view, []() {
    auto clipboard = std::make_unique<MockScopedClipboard>();

    EXPECT_CALL(*clipboard.get(), Open)
        .Times(1)
        .WillOnce(Return(kErrorSuccess));
    EXPECT_CALL(*clipboard.get(), HasString).Times(1).WillOnce(Return(true));
    EXPECT_CALL(*clipboard.get(), GetString)
        .Times(1)
        .WillOnce(Return(kArbitraryErrorCode));

    return clipboard;
  });

  std::string result = SimulatePlatformMessage(
      &messenger, reinterpret_cast<const uint8_t*>(kClipboardGetDataMessage),
      sizeof(kClipboardGetDataMessage));

  EXPECT_EQ(result, "[\"Clipboard error\",\"Unable to get clipboard data\",1]");
}

TEST(PlatformHandler, ClipboardHasStrings) {
  TestBinaryMessenger messenger;
  FlutterWindowsView view(
      std::make_unique<NiceMock<MockWindowBindingHandler>>());

  PlatformHandler platform_handler(&messenger, &view, []() {
    auto clipboard = std::make_unique<MockScopedClipboard>();

    EXPECT_CALL(*clipboard.get(), Open)
        .Times(1)
        .WillOnce(Return(kErrorSuccess));
    EXPECT_CALL(*clipboard.get(), HasString).Times(1).WillOnce(Return(true));

    return clipboard;
  });

  std::string result = SimulatePlatformMessage(
      &messenger, reinterpret_cast<const uint8_t*>(kClipboardHasStringsMessage),
      sizeof(kClipboardHasStringsMessage));

  EXPECT_EQ(result, "[{\"value\":true}]");
}

TEST(PlatformHandler, ClipboardHasStringsReturnsFalse) {
  TestBinaryMessenger messenger;
  FlutterWindowsView view(
      std::make_unique<NiceMock<MockWindowBindingHandler>>());

  PlatformHandler platform_handler(&messenger, &view, []() {
    auto clipboard = std::make_unique<MockScopedClipboard>();

    EXPECT_CALL(*clipboard.get(), Open)
        .Times(1)
        .WillOnce(Return(kErrorSuccess));
    EXPECT_CALL(*clipboard.get(), HasString).Times(1).WillOnce(Return(false));

    return clipboard;
  });

  std::string result = SimulatePlatformMessage(
      &messenger, reinterpret_cast<const uint8_t*>(kClipboardHasStringsMessage),
      sizeof(kClipboardHasStringsMessage));

  EXPECT_EQ(result, "[{\"value\":false}]");
}

TEST(PlatformHandler, ClipboardHasStringsRejectsUnknownContentType) {
  TestBinaryMessenger messenger;
  FlutterWindowsView view(
      std::make_unique<NiceMock<MockWindowBindingHandler>>());
  PlatformHandler platform_handler(&messenger, &view);

  std::string result = SimulatePlatformMessage(
      &messenger,
      reinterpret_cast<const uint8_t*>(
          kClipboardHasStringsFakeContentTypeMessage),
      sizeof(kClipboardHasStringsFakeContentTypeMessage));

  EXPECT_EQ(result, "[\"Clipboard error\",\"Unknown clipboard format\",null]");
}

// Regression test for https://github.com/flutter/flutter/issues/95817.
TEST(PlatformHandler, ClipboardHasStringsIgnoresPermissionErrors) {
  TestBinaryMessenger messenger;
  FlutterWindowsView view(
      std::make_unique<NiceMock<MockWindowBindingHandler>>());

  PlatformHandler platform_handler(&messenger, &view, []() {
    auto clipboard = std::make_unique<MockScopedClipboard>();

    EXPECT_CALL(*clipboard.get(), Open)
        .Times(1)
        .WillOnce(Return(kAccessDeniedErrorCode));

    return clipboard;
  });

  std::string result = SimulatePlatformMessage(
      &messenger, reinterpret_cast<const uint8_t*>(kClipboardHasStringsMessage),
      sizeof(kClipboardHasStringsMessage));

  EXPECT_EQ(result, "[{\"value\":false}]");
}

TEST(PlatformHandler, ClipboardHasStringsReportsErrors) {
  TestBinaryMessenger messenger;
  FlutterWindowsView view(
      std::make_unique<NiceMock<MockWindowBindingHandler>>());

  PlatformHandler platform_handler(&messenger, &view, []() {
    auto clipboard = std::make_unique<MockScopedClipboard>();

    EXPECT_CALL(*clipboard.get(), Open)
        .Times(1)
        .WillOnce(Return(kArbitraryErrorCode));

    return clipboard;
  });

  std::string result = SimulatePlatformMessage(
      &messenger, reinterpret_cast<const uint8_t*>(kClipboardHasStringsMessage),
      sizeof(kClipboardHasStringsMessage));

  EXPECT_EQ(result, "[\"Clipboard error\",\"Unable to open clipboard\",1]");
}

TEST(PlatformHandler, ClipboardSetData) {
  TestBinaryMessenger messenger;
  FlutterWindowsView view(
      std::make_unique<NiceMock<MockWindowBindingHandler>>());

  PlatformHandler platform_handler(&messenger, &view, []() {
    auto clipboard = std::make_unique<MockScopedClipboard>();

    EXPECT_CALL(*clipboard.get(), Open)
        .Times(1)
        .WillOnce(Return(kErrorSuccess));
    EXPECT_CALL(*clipboard.get(), SetString)
        .Times(1)
        .WillOnce([](std::wstring string) {
          EXPECT_EQ(string, L"hello");
          return kErrorSuccess;
        });

    return clipboard;
  });

  std::string result = SimulatePlatformMessage(
      &messenger, reinterpret_cast<const uint8_t*>(kClipboardSetDataMessage),
      sizeof(kClipboardSetDataMessage));

  EXPECT_EQ(result, "[null]");
}

TEST(PlatformHandler, ClipboardSetDataUnknownType) {
  TestBinaryMessenger messenger;
  FlutterWindowsView view(
      std::make_unique<NiceMock<MockWindowBindingHandler>>());
  PlatformHandler platform_handler(&messenger, &view);

  std::string result = SimulatePlatformMessage(
      &messenger,
      reinterpret_cast<const uint8_t*>(kClipboardSetDataUnknownTypeMessage),
      sizeof(kClipboardSetDataUnknownTypeMessage));

  EXPECT_EQ(result, "[\"Clipboard error\",\"Unknown clipboard format\",null]");
}

TEST(PlatformHandler, ClipboardSetDataReportsOpenFailure) {
  TestBinaryMessenger messenger;
  FlutterWindowsView view(
      std::make_unique<NiceMock<MockWindowBindingHandler>>());

  PlatformHandler platform_handler(&messenger, &view, []() {
    auto clipboard = std::make_unique<MockScopedClipboard>();

    EXPECT_CALL(*clipboard.get(), Open)
        .Times(1)
        .WillOnce(Return(kArbitraryErrorCode));

    return clipboard;
  });

  std::string result = SimulatePlatformMessage(
      &messenger, reinterpret_cast<const uint8_t*>(kClipboardSetDataMessage),
      sizeof(kClipboardSetDataMessage));

  EXPECT_EQ(result, "[\"Clipboard error\",\"Unable to open clipboard\",1]");
}

TEST(PlatformHandler, ClipboardSetDataReportsSetDataFailure) {
  TestBinaryMessenger messenger;
  FlutterWindowsView view(
      std::make_unique<NiceMock<MockWindowBindingHandler>>());

  PlatformHandler platform_handler(&messenger, &view, []() {
    auto clipboard = std::make_unique<MockScopedClipboard>();

    EXPECT_CALL(*clipboard.get(), Open)
        .Times(1)
        .WillOnce(Return(kErrorSuccess));
    EXPECT_CALL(*clipboard.get(), SetString)
        .Times(1)
        .WillOnce(Return(kArbitraryErrorCode));

    return clipboard;
  });

  std::string result = SimulatePlatformMessage(
      &messenger, reinterpret_cast<const uint8_t*>(kClipboardSetDataMessage),
      sizeof(kClipboardSetDataMessage));

  EXPECT_EQ(result, "[\"Clipboard error\",\"Unable to set clipboard data\",1]");
}

TEST(PlatformHandler, PlaySystemSound) {
  TestBinaryMessenger messenger;
  FlutterWindowsView view(
      std::make_unique<NiceMock<MockWindowBindingHandler>>());
  TestPlatformHandler platform_handler(&messenger, &view);

  EXPECT_CALL(platform_handler, SystemSoundPlay("SystemSoundType.alert", _))
      .WillOnce([](const std::string& sound,
                   std::unique_ptr<MethodResult<rapidjson::Document>> result) {
        result->Success();
      });

  std::string result = SimulatePlatformMessage(
      &messenger,
      reinterpret_cast<const uint8_t*>(kSystemSoundTypeAlertMessage),
      sizeof(kSystemSoundTypeAlertMessage));

  EXPECT_EQ(result, "[null]");
}

}  // namespace testing
}  // namespace flutter
