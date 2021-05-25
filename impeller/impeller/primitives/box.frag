// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

in vec3 color;
in vec3 color2;

out vec3 fragColor;

void main() {
  fragColor = color * color2;
}
