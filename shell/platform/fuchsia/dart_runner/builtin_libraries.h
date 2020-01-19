// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_SHELL_PLATFORM_FUCHSIA_DART_RUNNER_BUILTIN_LIBRARIES_H_
#define FLUTTER_SHELL_PLATFORM_FUCHSIA_DART_RUNNER_BUILTIN_LIBRARIES_H_

#include <fuchsia/io/cpp/fidl.h>
#include <fuchsia/sys/cpp/fidl.h>
#include <lib/fdio/namespace.h>
#include <lib/fidl/cpp/interface_handle.h>
#include <lib/fidl/cpp/interface_request.h>

#include <string>

namespace dart_runner {

void InitBuiltinLibraries(
    const std::string& script_uri,
    fdio_ns_t* namespc,
    fidl::InterfaceHandle<fuchsia::sys::Environment> environment,
    fidl::InterfaceRequest<fuchsia::io::Directory> directory_request,
    int stdoutfd,
    int stderrfd,
    bool service_isolate);

}  // namespace dart_runner

#endif  // FLUTTER_SHELL_PLATFORM_FUCHSIA_DART_RUNNER_BUILTIN_LIBRARIES_H_
