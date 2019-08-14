#!/bin/sh

set -e

FLUTTER_ENGINE=ios_debug_sim_unopt

if [ $# -eq 1 ]; then
  FLUTTER_ENGINE=$1
fi

PRETTY="cat"
if which xcpretty; then
  PRETTY="xcpretty"
fi

cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd

./compile_ios_jit.sh ../../../out/host_debug_unopt ../../../out/ios_debug_sim/clang_x64

pushd ios/Scenarios

set -o pipefail && xcodebuild -sdk iphonesimulator \
  -scheme ScenariosTests \
  -destination 'platform=iOS Simulator,name=iPhone SE,OS=12.2' \
  test \
  FLUTTER_ENGINE=$FLUTTER_ENGINE | $PRETTY

popd
