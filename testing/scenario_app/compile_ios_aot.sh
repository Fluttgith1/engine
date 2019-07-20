#!/bin/bash
# Copyright 2013 The Flutter Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.


set -e

HOST_TOOLS=$1
DEVICE_TOOLS=$2

if [[ ! -d "$HOST_TOOLS" ]]; then
  echo "Must specify the host out directory containing dart."
  exit 1
fi

if [[ ! -d "$DEVICE_TOOLS" ]]; then
  echo "Must specify the device out directory containing gen_snapshot."
  exit 1
fi

echo "Using dart from $HOST_TOOLS, gen_snapshot from $DEVICE_TOOLS."

OUTDIR="build/ios"

echo "Creating $OUTDIR..."

mkdir -p $OUTDIR
mkdir -p "$OUTDIR/App.framework"

echo "Compiling kernel..."

"$HOST_TOOLS/dart" \
  "$HOST_TOOLS/gen/frontend_server.dart.snapshot" \
  --sdk-root "$HOST_TOOLS/flutter_patched_sdk" \
  --aot --tfa --target=flutter \
  --output-dill $OUTDIR/app.dill \
  lib/main.dart

echo "Compiling AOT Assembly..."

"$DEVICE_TOOLS/gen_snapshot" --snapshot_kind=app-aot-assembly --assembly=$OUTDIR/snapshot_assembly.S $OUTDIR/app.dill

SYSROOT=$(xcrun --sdk iphoneos --show-sdk-path)
echo "Using $SYSROOT as sysroot."
echo "Compiling Assembly..."

cc -arch arm64 \
  -isysroot "$SYSROOT" \
  -miphoneos-version-min=8.0 \
  -c $OUTDIR/snapshot_assembly.S \
  -o $OUTDIR/snapshot_assembly.o

echo "Linking App using $SYSROOT..."

clang -arch arm64 \
  -isysroot "$SYSROOT" \
  -miphoneos-version-min=8.0 \
  -dynamiclib -Xlinker -rpath -Xlinker @executable_path/Frameworks \
  -Xlinker -rpath -Xlinker @loader_path/Frameworks \
  -install_name @rpath/App.framework/App \
  -o $OUTDIR/App.framework/.App \
  $OUTDIR/snapshot_assembly.o

echo "Created $OUTDIR/App."
