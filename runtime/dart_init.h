// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_RUNTIME_DART_INIT_H_
#define FLUTTER_RUNTIME_DART_INIT_H_

#include "dart/runtime/include/dart_api.h"
#include "lib/ftl/functional/closure.h"
#include "lib/ftl/build_config.h"

#include <memory>
#include <string>

namespace blink {

#define DART_ALLOW_DYNAMIC_RESOLUTION (OS_IOS || FLUTTER_AOT)

#if DART_ALLOW_DYNAMIC_RESOLUTION

extern const char kDartVmIsolateSnapshotBufferName[];
extern const char kDartIsolateSnapshotBufferName[];
extern const char kInstructionsSnapshotName[];
extern const char kDataSnapshotName[];

void* _DartSymbolLookup(const char* symbol_name);

#define DART_SYMBOL(symbol) _DartSymbolLookup(symbol##Name)

#else  // DART_ALLOW_DYNAMIC_RESOLUTION

extern "C" {
extern void* kDartVmIsolateSnapshotBuffer;
extern void* kDartIsolateSnapshotBuffer;
}

#define DART_SYMBOL(symbol) (&symbol)

#endif  // DART_ALLOW_DYNAMIC_RESOLUTION

// Name of the snapshot blob asset within the FLX bundle.
extern const char kSnapshotAssetKey[];

bool IsRunningPrecompiledCode();

using EmbedderTracingCallback = ftl::Closure;

typedef void (*ServiceIsolateHook)(bool);
typedef void (*RegisterNativeServiceProtocolExtensionHook)(bool);
typedef std::string (*LookupFileNameForSymbolNameHook)(const char*);

struct EmbedderTracingCallbacks {
  EmbedderTracingCallback start_tracing_callback;
  EmbedderTracingCallback stop_tracing_callback;

  EmbedderTracingCallbacks(EmbedderTracingCallback start,
                           EmbedderTracingCallback stop);
};

void InitDartVM();

void SetEmbedderTracingCallbacks(
    std::unique_ptr<EmbedderTracingCallbacks> callbacks);

// Provide a function that will be called during initialization of the
// service isolate.
void SetServiceIsolateHook(ServiceIsolateHook hook);

// Provide a function that will be called to register native service protocol
// extensions.
void SetRegisterNativeServiceProtocolExtensionHook(
    RegisterNativeServiceProtocolExtensionHook hook);

// Provide a function that will be called to lookup the name of the asset
// file that holds the named part of the precompiled snapshot.
void SetLookupFileNameForSymbolNameHook(
    LookupFileNameForSymbolNameHook hook);

}  // namespace blink

#endif  // FLUTTER_RUNTIME_DART_INIT_H_
