// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/lib/ui/assets.h"

#include "flutter/assets/asset_manager.h"
#include "flutter/lib/ui/ui_dart_state.h"
#include "flutter/lib/ui/window/platform_configuration.h"
#include "third_party/tonic/dart_binding_macros.h"
#include "third_party/tonic/dart_library_natives.h"
#include "third_party/tonic/logging/dart_invoke.h"
#include "third_party/tonic/typed_data/dart_byte_data.h"
#include "third_party/tonic/typed_data/typed_list.h"

using tonic::DartInvoke;
using tonic::DartPersistentValue;
using tonic::ToDart;

namespace flutter {

  void FinalizeSkData(void* isolate_callback_data,
                    Dart_WeakPersistentHandle handle,
                    void* peer) {

}

void Assets::loadAssetBytes(Dart_NativeArguments args) {
  UIDartState::ThrowIfUIOperationsProhibited();
  Dart_Handle callback = Dart_GetNativeArgument(args, 1);
  if (!Dart_IsClosure(callback)) {
    Dart_SetReturnValue(args, tonic::ToDart("Callback must be a function"));
    return;
  }
  Dart_Handle asset_name_handle = Dart_GetNativeArgument(args, 0);
  uint8_t* chars = nullptr;
  intptr_t asset_length = 0;
  Dart_Handle result =
      Dart_StringToUTF8(asset_name_handle, &chars, &asset_length);
  if (Dart_IsError(result)) {
    Dart_PropagateError(result);
    return;
  }
  std::string asset_name = std::string{reinterpret_cast<const char*>(chars),
                                       static_cast<size_t>(asset_length)};

  std::shared_ptr<AssetManager> asset_manager = UIDartState::Current()
                                                    ->platform_configuration()
                                                    ->client()
                                                    ->GetAssetManager();
  std::unique_ptr<fml::Mapping> data = asset_manager->GetAsMapping(asset_name);

  if (data == nullptr) {
    return;
  }
  const void* bytes_ = static_cast<const void*>(data->GetMapping());
  void* bytes = const_cast<void*>(bytes_);
  const intptr_t length = data->GetSize();
  void* peer = reinterpret_cast<void*>(data.release());
  Dart_Handle byte_buffer = Dart_NewExternalTypedDataWithFinalizer(
      Dart_TypedData_kUint8, bytes, length, peer, length, FinalizeSkData);

  // Dart_Handle byte_buffer =
  //     tonic::DartByteData::Create(data->GetMapping(), data->GetSize());
  tonic::DartInvoke(callback, {ToDart(byte_buffer)});
}

void Assets::RegisterNatives(tonic::DartLibraryNatives* natives) {
  natives->Register({
      {"loadAssetBytes", loadAssetBytes, 2, true},
  });
}

}  // namespace flutter
