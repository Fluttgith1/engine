// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_SHELL_PLATFORM_FUCHSIA_DART_RUNNER_DART_COMPONENT_CONTROLLER_H_
#define FLUTTER_SHELL_PLATFORM_FUCHSIA_DART_RUNNER_DART_COMPONENT_CONTROLLER_H_

#include <memory>

#include <fuchsia/sys/cpp/fidl.h>
#include <lib/async-loop/cpp/loop.h>
#include <lib/async/cpp/wait.h>
#include <lib/fdio/namespace.h>
#include <lib/sys/cpp/component_context.h>
#include <lib/sys/cpp/service_directory.h>
#include <lib/zx/timer.h>

#include "lib/fidl/cpp/binding.h"
#include "runtime/dart/utils/mapped_resource.h"
#include "third_party/dart/runtime/include/dart_api.h"

namespace dart_runner {

/// The base class for Dart components.
class DartComponentController {
 public:
  // Called before the application is run.
  bool Setup();

  // Calling this method will run the given application.
  void Run();

 protected:
  static constexpr char kComponentTmpPath[] = "/tmp";
  static constexpr char kServiceRootPath[] = "/svc";

  DartComponentController(
      std::string resolved_url,
      std::shared_ptr<sys::ServiceDirectory> runner_incoming_services);
  ~DartComponentController();

  void Shutdown();

  std::unique_ptr<async::Loop> loop_;
  std::string label_;
  std::string data_path_;
  std::string url_;

  int32_t return_code_ = 0;

  fdio_ns_t* namespace_ = nullptr;
  int stdoutfd_ = -1;
  int stderrfd_ = -1;

 private:
  /// Runs the Dart component. Returns true if the component ran and
  /// the main result was successful, otherwise false.
  bool RunDartMain();

  /// Override this method to send the return code to the caller.
  virtual void SendReturnCode() = 0;

  /// Called when the application is starting up. Subclasses should
  /// populate the namespace and return the pointer.
  virtual fdio_ns_t* PrepareNamespace() = 0;

  /// Initializes the builtin libraries given the namespace.
  virtual bool PrepareBuiltinLibraries() = 0;

  // Returns the file descriptors for stdout/stderr.
  virtual int GetStdoutFileDescriptor() = 0;
  virtual int GetStderrFileDescriptor() = 0;

  /// Called by the main method to pass incoming arguments to the dart
  /// application.
  virtual std::vector<std::string> GetArguments() = 0;

  bool SetupFromKernel();
  bool SetupFromAppSnapshot();

  bool CreateIsolate(const uint8_t* isolate_snapshot_data,
                     const uint8_t* isolate_snapshot_instructions);

  // Idle notification.
  void MessageEpilogue(Dart_Handle result);
  void OnIdleTimer(async_dispatcher_t* dispatcher,
                   async::WaitBase* wait,
                   zx_status_t status,
                   const zx_packet_signal* signal);

  std::shared_ptr<sys::ServiceDirectory> runner_incoming_services_;

  dart_utils::ElfSnapshot elf_snapshot_;                      // AOT snapshot
  dart_utils::MappedResource isolate_snapshot_data_;          // JIT snapshot
  dart_utils::MappedResource isolate_snapshot_instructions_;  // JIT snapshot
  std::vector<dart_utils::MappedResource> kernel_peices_;

  Dart_Isolate isolate_;

  zx::time idle_start_{0};
  zx::timer idle_timer_;
  async::WaitMethod<DartComponentController,
                    &DartComponentController::OnIdleTimer>
      idle_wait_{this};

  // Disallow copy and assignment.
  DartComponentController(const DartComponentController&) = delete;
  DartComponentController& operator=(const DartComponentController&) = delete;
};

}  // namespace dart_runner

#endif  // FLUTTER_SHELL_PLATFORM_FUCHSIA_DART_RUNNER_DART_COMPONENT_CONTROLLER_H_
