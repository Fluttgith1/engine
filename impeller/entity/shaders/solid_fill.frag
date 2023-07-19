// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

precision mediump float;

uniform FragInfo {
  highp vec4 color;
}
frag_info;

out highp vec4 frag_color;

void main() {
  frag_color = frag_info.color;
}
