// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "service_isolate.h"

#include "runtime/dart/utils/inlines.h"
#include "third_party/dart/runtime/include/bin/dart_io_api.h"
#include "third_party/tonic/converter/dart_converter.h"
#include "third_party/tonic/dart_library_natives.h"
#include "third_party/tonic/dart_microtask_queue.h"
#include "third_party/tonic/dart_state.h"
#include "third_party/tonic/typed_data/typed_list.h"

#include "builtin_libraries.h"
#include "dart_component_controller.h"
#include "logging.h"

namespace dart_runner {
namespace {

MappedResource mapped_isolate_snapshot_data;
MappedResource mapped_isolate_snapshot_instructions;
MappedResource mapped_shared_snapshot_data;
MappedResource mapped_shared_snapshot_instructions;
tonic::DartLibraryNatives* service_natives = nullptr;

Dart_NativeFunction GetNativeFunction(Dart_Handle name,
                                      int argument_count,
                                      bool* auto_setup_scope) {
  dart_utils::Check(service_natives, LOG_TAG);
  return service_natives->GetNativeFunction(name, argument_count,
                                            auto_setup_scope);
}

const uint8_t* GetSymbol(Dart_NativeFunction native_function) {
  dart_utils::Check(service_natives, LOG_TAG);
  return service_natives->GetSymbol(native_function);
}

#define SHUTDOWN_ON_ERROR(handle)           \
  if (Dart_IsError(handle)) {               \
    *error = strdup(Dart_GetError(handle)); \
    FX_LOG(ERROR, LOG_TAG, *error);         \
    Dart_ExitScope();                       \
    Dart_ShutdownIsolate();                 \
    return nullptr;                         \
  }

void NotifyServerState(Dart_NativeArguments args) {
  // NOP.
}

void Shutdown(Dart_NativeArguments args) {
  // NOP.
}

void EmbedderInformationCallback(Dart_EmbedderInformation* info) {
  info->version = DART_EMBEDDER_INFORMATION_CURRENT_VERSION;
  info->name = "dart_runner";
  info->current_rss = -1;
  info->max_rss = -1;

  zx_info_task_stats_t task_stats;
  zx_handle_t process = zx_process_self();
  zx_status_t status = zx_object_get_info(
      process, ZX_INFO_TASK_STATS, &task_stats, sizeof(task_stats), NULL, NULL);
  if (status == ZX_OK) {
    info->current_rss =
        task_stats.mem_private_bytes + task_stats.mem_shared_bytes;
  }
}

}  // namespace

Dart_Isolate CreateServiceIsolate(const char* uri,
                                  Dart_IsolateFlags* flags,
                                  char** error) {
  Dart_SetEmbedderInformationCallback(EmbedderInformationCallback);

#if defined(AOT_RUNTIME)
  // The VM service was compiled as a separate app.
  const char* snapshot_data_path =
      "pkg/data/vmservice_isolate_snapshot_data.bin";
  const char* snapshot_instructions_path =
      "pkg/data/vmservice_isolate_snapshot_instructions.bin";
#else
  // The VM service is embedded in the core snapshot.
  const char* snapshot_data_path = "pkg/data/isolate_core_snapshot_data.bin";
  const char* snapshot_instructions_path =
      "pkg/data/isolate_core_snapshot_instructions.bin";
#endif

  if (!MappedResource::LoadFromNamespace(nullptr, snapshot_data_path,
                                         mapped_isolate_snapshot_data)) {
    *error = strdup("Failed to load snapshot for service isolate");
    FX_LOG(ERROR, LOG_TAG, *error);
    return nullptr;
  }
  if (!MappedResource::LoadFromNamespace(nullptr, snapshot_instructions_path,
                                         mapped_isolate_snapshot_instructions,
                                         true /* executable */)) {
    *error = strdup("Failed to load snapshot for service isolate");
    FX_LOG(ERROR, LOG_TAG, *error);
    return nullptr;
  }

#if defined(AOT_RUNTIME)
  if (!MappedResource::LoadFromNamespace(
          nullptr, "pkg/data/vmservice_shared_snapshot_data.bin",
          mapped_shared_snapshot_data)) {
    *error = strdup("Failed to load snapshot for service isolate");
    FX_LOG(ERROR, LOG_TAG, *error);
    return nullptr;
  }
  if (!MappedResource::LoadFromNamespace(
          nullptr, "pkg/data/vmservice_shared_snapshot_instructions.bin",
          mapped_shared_snapshot_instructions, true /* executable */)) {
    *error = strdup("Failed to load snapshot for service isolate");
    FX_LOG(ERROR, LOG_TAG, *error);
    return nullptr;
  }
#endif

  auto state = new std::shared_ptr<tonic::DartState>(new tonic::DartState());
  Dart_Isolate isolate = Dart_CreateIsolate(
      uri, DART_VM_SERVICE_ISOLATE_NAME, mapped_isolate_snapshot_data.address(),
      mapped_isolate_snapshot_instructions.address(),
      mapped_shared_snapshot_data.address(),
      mapped_shared_snapshot_instructions.address(), nullptr /* flags */, state,
      error);
  if (!isolate) {
    FX_LOGF(ERROR, LOG_TAG, "Dart_CreateIsolate failed: %s", *error);
    return nullptr;
  }

  state->get()->SetIsolate(isolate);

  // Setup native entries.
  service_natives = new tonic::DartLibraryNatives();
  service_natives->Register({
      {"VMServiceIO_NotifyServerState", NotifyServerState, 1, true},
      {"VMServiceIO_Shutdown", Shutdown, 0, true},
  });

  Dart_EnterScope();

  Dart_Handle library =
      Dart_LookupLibrary(Dart_NewStringFromCString("dart:vmservice_io"));
  SHUTDOWN_ON_ERROR(library);
  Dart_Handle result = Dart_SetRootLibrary(library);
  SHUTDOWN_ON_ERROR(result);
  result = Dart_SetNativeResolver(library, GetNativeFunction, GetSymbol);
  SHUTDOWN_ON_ERROR(result);

  // _ip = '127.0.0.1'
  result = Dart_SetField(library, Dart_NewStringFromCString("_ip"),
                         Dart_NewStringFromCString("127.0.0.1"));
  SHUTDOWN_ON_ERROR(result);

  // _port = 0
  result = Dart_SetField(library, Dart_NewStringFromCString("_port"),
                         Dart_NewInteger(0));
  SHUTDOWN_ON_ERROR(result);

  // _autoStart = true
  result = Dart_SetField(library, Dart_NewStringFromCString("_autoStart"),
                         Dart_NewBoolean(true));
  SHUTDOWN_ON_ERROR(result);

  // _originCheckDisabled = false
  result =
      Dart_SetField(library, Dart_NewStringFromCString("_originCheckDisabled"),
                    Dart_NewBoolean(false));
  SHUTDOWN_ON_ERROR(result);

  // _authCodesDisabled = false
  result =
      Dart_SetField(library, Dart_NewStringFromCString("_authCodesDisabled"),
                    Dart_NewBoolean(false));
  SHUTDOWN_ON_ERROR(result);

  InitBuiltinLibrariesForIsolate(std::string(uri), nullptr, fileno(stdout),
                                 fileno(stderr), nullptr, zx::channel(), true);

  // Make runnable.
  Dart_ExitScope();
  Dart_ExitIsolate();
  *error = Dart_IsolateMakeRunnable(isolate);
  if (*error != nullptr) {
    FX_LOG(ERROR, LOG_TAG, *error);
    Dart_EnterIsolate(isolate);
    Dart_ShutdownIsolate();
    return nullptr;
  }
  return isolate;
}  // namespace dart_runner

Dart_Handle GetVMServiceAssetsArchiveCallback() {
  MappedResource observatory_tar;
  if (!MappedResource::LoadFromNamespace(nullptr, "pkg/data/observatory.tar",
                                         observatory_tar)) {
    FX_LOG(ERROR, LOG_TAG, "Failed to load Observatory assets");
    return nullptr;
  }
  // TODO(rmacnak): Should we avoid copying the tar? Or does the service library
  // not hold onto it anyway?
  return tonic::DartConverter<tonic::Uint8List>::ToDart(
      reinterpret_cast<const uint8_t*>(observatory_tar.address()),
      observatory_tar.size());
}

}  // namespace dart_runner
