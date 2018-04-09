// Copyright 2018 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/fml/paths.h"

namespace fml {
namespace paths {

std::string JoinPaths(std::initializer_list<fxl::StringView> components) {
  std::stringstream stream;
  size_t i = 0;
  const size_t size = components.size();
  for (const auto& component : components) {
    i++;
    stream << component;
    if (i != size) {
      stream << "/";
    }
  }
  return stream.str();
}

}  // namespace paths
}  // namespace fml
