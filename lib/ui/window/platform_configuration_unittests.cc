// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#define FML_USED_ON_EMBEDDER

#include "flutter/lib/ui/window/platform_configuration.h"

#include <memory>

#include "flutter/common/task_runners.h"
#include "flutter/fml/synchronization/waitable_event.h"
#include "flutter/lib/ui/painting/vertices.h"
#include "flutter/runtime/dart_vm.h"
#include "flutter/shell/common/shell_test.h"
#include "flutter/shell/common/thread_host.h"
#include "flutter/testing/testing.h"
#include "gmock/gmock.h"

///\note Deprecated MOCK_METHOD macros used until this issue is resolved:
// https://github.com/google/googletest/issues/2490

namespace flutter {

namespace {

static constexpr int64_t kImplicitViewId = 0;

static void PostSync(const fml::RefPtr<fml::TaskRunner>& task_runner,
                     const fml::closure& task) {
  fml::AutoResetWaitableEvent latch;
  fml::TaskRunner::RunNowOrPostTask(task_runner, [&latch, &task] {
    task();
    latch.Signal();
  });
  latch.Wait();
}

class MockRuntimeDelegate : public RuntimeDelegate {
 public:
  MOCK_METHOD0(ImplicitViewEnabled, bool());
  MOCK_METHOD0(DefaultRouteName, std::string());
  MOCK_METHOD1(ScheduleFrame, void(bool));
  MOCK_METHOD2(Render, void(std::unique_ptr<flutter::LayerTree>, float));
  MOCK_METHOD2(UpdateSemantics,
               void(SemanticsNodeUpdates, CustomAccessibilityActionUpdates));
  MOCK_METHOD1(HandlePlatformMessage, void(std::unique_ptr<PlatformMessage>));
  MOCK_METHOD0(GetFontCollection, FontCollection&());
  MOCK_METHOD0(GetAssetManager, std::shared_ptr<AssetManager>());
  MOCK_METHOD0(OnRootIsolateCreated, void());
  MOCK_METHOD2(UpdateIsolateDescription, void(const std::string, int64_t));
  MOCK_METHOD1(SetNeedsReportTimings, void(bool));
  MOCK_METHOD1(ComputePlatformResolvedLocale,
               std::unique_ptr<std::vector<std::string>>(
                   const std::vector<std::string>&));
  MOCK_METHOD1(RequestDartDeferredLibrary, void(intptr_t));
  MOCK_CONST_METHOD0(GetPlatformMessageHandler,
                     std::weak_ptr<PlatformMessageHandler>());
  MOCK_CONST_METHOD2(GetScaledFontSize,
                     double(double font_size, int configuration_id));
};

class MockPlatformMessageHandler : public PlatformMessageHandler {
 public:
  MOCK_METHOD1(HandlePlatformMessage,
               void(std::unique_ptr<PlatformMessage> message));
  MOCK_CONST_METHOD0(DoesHandlePlatformMessageOnPlatformThread, bool());
  MOCK_METHOD2(InvokePlatformMessageResponseCallback,
               void(int response_id, std::unique_ptr<fml::Mapping> mapping));
  MOCK_METHOD1(InvokePlatformMessageEmptyResponseCallback,
               void(int response_id));
};

class MockRuntimeControllerContext {
 public:
  MockRuntimeControllerContext(Settings settings,
                               TaskRunners task_runners,
                               RuntimeDelegate& client)
      : settings_(settings),
        task_runners_(task_runners),
        vm_snapshot_(DartSnapshot::VMSnapshotFromSettings(settings)),
        isolate_snapshot_(DartSnapshot::IsolateSnapshotFromSettings(settings)),
        vm_(DartVMRef::Create(settings, vm_snapshot_, isolate_snapshot_)) {
    // Always use the `vm_snapshot` and `isolate_snapshot` provided by the
    // settings to launch the VM.  If the VM is already running, the snapshot
    // arguments are ignored.
    FML_CHECK(vm_) << "Must be able to initialize the VM.";
    if (!isolate_snapshot_) {
      isolate_snapshot_ = vm_->GetVMData()->GetIsolateSnapshot();
    }
    runtime_controller_ = std::make_unique<RuntimeController>(
        client, &vm_, std::move(isolate_snapshot_),
        settings.idle_notification_callback,  // idle notification callback
        flutter::PlatformData(),              // platform data
        settings.isolate_create_callback,     // isolate create callback
        settings.isolate_shutdown_callback,   // isolate shutdown callback
        settings.persistent_isolate_data,     // persistent isolate data
        UIDartState::Context{task_runners});
  }

  void LaunchRootIsolate(RunConfiguration& configuration,
                         ViewportMetrics implicit_view_metrics) {
    PostSync(task_runners_.GetUITaskRunner(), [&]() {
      bool launch_success = runtime_controller_->LaunchRootIsolate(
          settings_,                                  //
          []() {},                                    //
          configuration.GetEntrypoint(),              //
          configuration.GetEntrypointLibrary(),       //
          configuration.GetEntrypointArgs(),          //
          configuration.TakeIsolateConfiguration());  //
      ASSERT_TRUE(launch_success);
      runtime_controller_->AddView(kImplicitViewId, implicit_view_metrics);
    });
  }

  RuntimeController& GetController() { return *runtime_controller_; }

 private:
  Settings settings_;
  TaskRunners task_runners_;
  fml::RefPtr<const DartSnapshot> vm_snapshot_;
  fml::RefPtr<const DartSnapshot> isolate_snapshot_;
  DartVMRef vm_;
  std::unique_ptr<RuntimeController> runtime_controller_;
};
}  // namespace

namespace testing {

using ::testing::_;
using ::testing::Exactly;
using ::testing::Return;

class PlatformConfigurationTest : public ShellTest {};

TEST_F(PlatformConfigurationTest, Initialization) {
  auto message_latch = std::make_shared<fml::AutoResetWaitableEvent>();

  auto nativeValidateConfiguration =
      [message_latch](Dart_NativeArguments args) {
        PlatformConfiguration* configuration =
            UIDartState::Current()->platform_configuration();
        ASSERT_NE(configuration->GetMetrics(0), nullptr);
        ASSERT_EQ(configuration->GetMetrics(0)->device_pixel_ratio, 1.0);
        ASSERT_EQ(configuration->GetMetrics(0)->physical_width, 0.0);
        ASSERT_EQ(configuration->GetMetrics(0)->physical_height, 0.0);

        message_latch->Signal();
      };

  Settings settings = CreateSettingsForFixture();
  TaskRunners task_runners("test",                  // label
                           GetCurrentTaskRunner(),  // platform
                           CreateNewThread(),       // raster
                           CreateNewThread(),       // ui
                           CreateNewThread()        // io
  );

  AddNativeCallback("ValidateConfiguration",
                    CREATE_NATIVE_ENTRY(nativeValidateConfiguration));

  std::unique_ptr<Shell> shell = CreateShell(settings, task_runners);

  ASSERT_TRUE(shell->IsSetup());
  auto run_configuration = RunConfiguration::InferFromSettings(settings);
  run_configuration.SetEntrypoint("validateConfiguration");

  shell->RunEngine(std::move(run_configuration), [&](auto result) {
    ASSERT_EQ(result, Engine::RunStatus::Success);
  });

  message_latch->Wait();
  DestroyShell(std::move(shell), task_runners);
}

TEST_F(PlatformConfigurationTest, WindowMetricsUpdate) {
  auto message_latch = std::make_shared<fml::AutoResetWaitableEvent>();

  auto nativeValidateConfiguration =
      [message_latch](Dart_NativeArguments args) {
        PlatformConfiguration* configuration =
            UIDartState::Current()->platform_configuration();

        ASSERT_NE(configuration->GetMetrics(0), nullptr);
        bool has_view = configuration->UpdateViewMetrics(
            0, ViewportMetrics{2.0, 10.0, 20.0, 22, 0});
        ASSERT_TRUE(has_view);
        ASSERT_EQ(configuration->GetMetrics(0)->device_pixel_ratio, 2.0);
        ASSERT_EQ(configuration->GetMetrics(0)->physical_width, 10.0);
        ASSERT_EQ(configuration->GetMetrics(0)->physical_height, 20.0);
        ASSERT_EQ(configuration->GetMetrics(0)->physical_touch_slop, 22);

        message_latch->Signal();
      };

  Settings settings = CreateSettingsForFixture();
  TaskRunners task_runners("test",                  // label
                           GetCurrentTaskRunner(),  // platform
                           CreateNewThread(),       // raster
                           CreateNewThread(),       // ui
                           CreateNewThread()        // io
  );

  AddNativeCallback("ValidateConfiguration",
                    CREATE_NATIVE_ENTRY(nativeValidateConfiguration));

  std::unique_ptr<Shell> shell = CreateShell(settings, task_runners);

  ASSERT_TRUE(shell->IsSetup());
  auto run_configuration = RunConfiguration::InferFromSettings(settings);
  run_configuration.SetEntrypoint("validateConfiguration");

  shell->RunEngine(std::move(run_configuration), [&](auto result) {
    ASSERT_EQ(result, Engine::RunStatus::Success);
  });

  message_latch->Wait();
  DestroyShell(std::move(shell), task_runners);
}

TEST_F(PlatformConfigurationTest, GetWindowReturnsNullForNonexistentId) {
  auto message_latch = std::make_shared<fml::AutoResetWaitableEvent>();

  auto nativeValidateConfiguration =
      [message_latch](Dart_NativeArguments args) {
        PlatformConfiguration* configuration =
            UIDartState::Current()->platform_configuration();

        ASSERT_EQ(configuration->GetMetrics(1), nullptr);
        ASSERT_EQ(configuration->GetMetrics(2), nullptr);

        message_latch->Signal();
      };

  Settings settings = CreateSettingsForFixture();
  TaskRunners task_runners("test",                  // label
                           GetCurrentTaskRunner(),  // platform
                           CreateNewThread(),       // raster
                           CreateNewThread(),       // ui
                           CreateNewThread()        // io
  );

  AddNativeCallback("ValidateConfiguration",
                    CREATE_NATIVE_ENTRY(nativeValidateConfiguration));

  std::unique_ptr<Shell> shell = CreateShell(settings, task_runners);

  ASSERT_TRUE(shell->IsSetup());
  auto run_configuration = RunConfiguration::InferFromSettings(settings);
  run_configuration.SetEntrypoint("validateConfiguration");

  shell->RunEngine(std::move(run_configuration), [&](auto result) {
    ASSERT_EQ(result, Engine::RunStatus::Success);
  });

  message_latch->Wait();
  DestroyShell(std::move(shell), task_runners);
}

TEST_F(PlatformConfigurationTest, OnErrorHandlesError) {
  auto message_latch = std::make_shared<fml::AutoResetWaitableEvent>();
  bool did_throw = false;

  auto finish = [message_latch](Dart_NativeArguments args) {
    message_latch->Signal();
  };
  AddNativeCallback("Finish", CREATE_NATIVE_ENTRY(finish));

  Settings settings = CreateSettingsForFixture();
  settings.unhandled_exception_callback =
      [&did_throw](const std::string& exception,
                   const std::string& stack_trace) -> bool {
    did_throw = true;
    return false;
  };

  TaskRunners task_runners("test",                  // label
                           GetCurrentTaskRunner(),  // platform
                           CreateNewThread(),       // raster
                           CreateNewThread(),       // ui
                           CreateNewThread()        // io
  );

  std::unique_ptr<Shell> shell = CreateShell(settings, task_runners);

  ASSERT_TRUE(shell->IsSetup());
  auto run_configuration = RunConfiguration::InferFromSettings(settings);
  run_configuration.SetEntrypoint("customOnErrorTrue");

  shell->RunEngine(std::move(run_configuration), [&](auto result) {
    ASSERT_EQ(result, Engine::RunStatus::Success);
  });

  message_latch->Wait();

  // Flush the UI task runner to make sure errors that were triggered had a turn
  // to propagate.
  task_runners.GetUITaskRunner()->PostTask(
      [&message_latch]() { message_latch->Signal(); });
  message_latch->Wait();

  ASSERT_FALSE(did_throw);
  DestroyShell(std::move(shell), task_runners);
}

TEST_F(PlatformConfigurationTest, OnErrorDoesNotHandleError) {
  auto message_latch = std::make_shared<fml::AutoResetWaitableEvent>();
  std::string ex;
  std::string st;
  size_t throw_count = 0;

  auto finish = [message_latch](Dart_NativeArguments args) {
    message_latch->Signal();
  };
  AddNativeCallback("Finish", CREATE_NATIVE_ENTRY(finish));

  Settings settings = CreateSettingsForFixture();
  settings.unhandled_exception_callback =
      [&ex, &st, &throw_count](const std::string& exception,
                               const std::string& stack_trace) -> bool {
    throw_count += 1;
    ex = exception;
    st = stack_trace;
    return true;
  };

  TaskRunners task_runners("test",                  // label
                           GetCurrentTaskRunner(),  // platform
                           CreateNewThread(),       // raster
                           CreateNewThread(),       // ui
                           CreateNewThread()        // io
  );

  std::unique_ptr<Shell> shell = CreateShell(settings, task_runners);

  ASSERT_TRUE(shell->IsSetup());
  auto run_configuration = RunConfiguration::InferFromSettings(settings);
  run_configuration.SetEntrypoint("customOnErrorFalse");

  shell->RunEngine(std::move(run_configuration), [&](auto result) {
    ASSERT_EQ(result, Engine::RunStatus::Success);
  });

  message_latch->Wait();

  // Flush the UI task runner to make sure errors that were triggered had a turn
  // to propagate.
  task_runners.GetUITaskRunner()->PostTask(
      [&message_latch]() { message_latch->Signal(); });
  message_latch->Wait();

  ASSERT_EQ(throw_count, 1ul);
  ASSERT_EQ(ex, "Exception: false") << ex;
  ASSERT_EQ(st.rfind("#0      customOnErrorFalse", 0), 0ul) << st;
  DestroyShell(std::move(shell), task_runners);
}

TEST_F(PlatformConfigurationTest, OnErrorThrows) {
  auto message_latch = std::make_shared<fml::AutoResetWaitableEvent>();
  std::vector<std::string> errors;
  size_t throw_count = 0;

  auto finish = [message_latch](Dart_NativeArguments args) {
    message_latch->Signal();
  };
  AddNativeCallback("Finish", CREATE_NATIVE_ENTRY(finish));

  Settings settings = CreateSettingsForFixture();
  settings.unhandled_exception_callback =
      [&errors, &throw_count](const std::string& exception,
                              const std::string& stack_trace) -> bool {
    throw_count += 1;
    errors.push_back(exception);
    errors.push_back(stack_trace);
    return true;
  };

  TaskRunners task_runners("test",                  // label
                           GetCurrentTaskRunner(),  // platform
                           CreateNewThread(),       // raster
                           CreateNewThread(),       // ui
                           CreateNewThread()        // io
  );

  std::unique_ptr<Shell> shell = CreateShell(settings, task_runners);

  ASSERT_TRUE(shell->IsSetup());
  auto run_configuration = RunConfiguration::InferFromSettings(settings);
  run_configuration.SetEntrypoint("customOnErrorThrow");

  shell->RunEngine(std::move(run_configuration), [&](auto result) {
    ASSERT_EQ(result, Engine::RunStatus::Success);
  });

  message_latch->Wait();

  // Flush the UI task runner to make sure errors that were triggered had a turn
  // to propagate.
  task_runners.GetUITaskRunner()->PostTask(
      [&message_latch]() { message_latch->Signal(); });
  message_latch->Wait();

  ASSERT_EQ(throw_count, 2ul);
  ASSERT_EQ(errors.size(), 4ul);
  ASSERT_EQ(errors[0], "Exception: throw2") << errors[0];
  ASSERT_EQ(errors[1].rfind("#0      customOnErrorThrow"), 0ul) << errors[1];
  ASSERT_EQ(errors[2], "Exception: throw1") << errors[2];
  ASSERT_EQ(errors[3].rfind("#0      customOnErrorThrow"), 0ul) << errors[3];

  DestroyShell(std::move(shell), task_runners);
}

TEST_F(PlatformConfigurationTest, SetDartPerformanceMode) {
  auto message_latch = std::make_shared<fml::AutoResetWaitableEvent>();
  auto finish = [message_latch](Dart_NativeArguments args) {
    // call needs to happen on the UI thread.
    Dart_PerformanceMode prev =
        Dart_SetPerformanceMode(Dart_PerformanceMode_Default);
    ASSERT_EQ(Dart_PerformanceMode_Latency, prev);
    message_latch->Signal();
  };
  AddNativeCallback("Finish", CREATE_NATIVE_ENTRY(finish));

  Settings settings = CreateSettingsForFixture();

  TaskRunners task_runners("test",                  // label
                           GetCurrentTaskRunner(),  // platform
                           CreateNewThread(),       // raster
                           CreateNewThread(),       // ui
                           CreateNewThread()        // io
  );

  std::unique_ptr<Shell> shell = CreateShell(settings, task_runners);

  ASSERT_TRUE(shell->IsSetup());
  auto run_configuration = RunConfiguration::InferFromSettings(settings);
  run_configuration.SetEntrypoint("setLatencyPerformanceMode");

  shell->RunEngine(std::move(run_configuration), [&](auto result) {
    ASSERT_EQ(result, Engine::RunStatus::Success);
  });

  message_latch->Wait();
  DestroyShell(std::move(shell), task_runners);
}

TEST_F(PlatformConfigurationTest, OutOfScopeRenderCallsAreIgnored) {
  Settings settings = CreateSettingsForFixture();
  TaskRunners task_runners = GetTaskRunnersForFixture();

  MockRuntimeDelegate client;
  auto platform_message_handler =
      std::make_shared<MockPlatformMessageHandler>();
  EXPECT_CALL(client, GetPlatformMessageHandler())
      .WillOnce(Return(platform_message_handler));
  // Render should not be called.
  EXPECT_CALL(client, Render(_, _)).Times(Exactly(0));

  auto context = std::make_unique<MockRuntimeControllerContext>(
      settings, task_runners, client);

  auto configuration = RunConfiguration::InferFromSettings(settings);
  configuration.SetEntrypoint("incorrectImmediateRender");
  ViewportMetrics implicit_view_metrics(
      /*device_pixel_ratio=*/1.0, /*width=*/20, /*height=*/20,
      /*physical_touch_slop=*/2, /*display_id=*/0);
  context->LaunchRootIsolate(configuration, implicit_view_metrics);

  // Wait for the Dart main function to end.
  fml::AutoResetWaitableEvent latch;
  PostSync(task_runners.GetUITaskRunner(), [&]() { latch.Signal(); });
  latch.Wait();

  PostSync(task_runners.GetUITaskRunner(), [&]() { context.reset(); });
}

TEST_F(PlatformConfigurationTest, DuplicateRenderCallsAreIgnored) {
  Settings settings = CreateSettingsForFixture();
  TaskRunners task_runners = GetTaskRunnersForFixture();

  MockRuntimeDelegate client;
  auto platform_message_handler =
      std::make_shared<MockPlatformMessageHandler>();
  EXPECT_CALL(client, GetPlatformMessageHandler())
      .WillOnce(Return(platform_message_handler));
  // Render should only be called once, because the second call is ignored.
  EXPECT_CALL(client, Render(_, _)).Times(Exactly(1));

  auto context = std::make_unique<MockRuntimeControllerContext>(
      settings, task_runners, client);

  auto configuration = RunConfiguration::InferFromSettings(settings);
  configuration.SetEntrypoint("incorrectDoubleRender");
  ViewportMetrics implicit_view_metrics(
      /*device_pixel_ratio=*/1.0, /*width=*/20, /*height=*/20,
      /*physical_touch_slop=*/2, /*display_id=*/0);
  context->LaunchRootIsolate(configuration, implicit_view_metrics);

  // Wait for the Dart main function to end, which means the callbacks have been
  // registered.
  fml::AutoResetWaitableEvent latch;
  PostSync(task_runners.GetUITaskRunner(), [&]() { latch.Signal(); });
  latch.Wait();

  context->GetController().BeginFrame(fml::TimePoint::Now(), 0);

  PostSync(task_runners.GetUITaskRunner(), [&]() { context.reset(); });
}

}  // namespace testing
}  // namespace flutter
