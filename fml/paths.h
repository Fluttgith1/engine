// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_FML_PATH_SERVICE_H_
#define FLUTTER_FML_PATH_SERVICE_H_

#include <memory>
#include <string>
#include <utility>

namespace fml {
namespace paths {

std::pair<bool, std::string> GetExecutableDirectoryPath();

}  // namespace paths
}  // namespace fml

#endif  // FLUTTER_FML_PATH_SERVICE_H_
