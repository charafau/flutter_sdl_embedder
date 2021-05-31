#!/bin/bash
set -e # Exit if any program returns an error.
#################################################################
# Make the host C++ project.
#################################################################
if [ ! -d debug ]; then
    mkdir debug
fi
cd debug
cmake -DCMAKE_BUILD_TYPE=Debug ..
make
#################################################################
# Make the guest Flutter project.
#################################################################
if [ ! -d myapp ]; then
    flutter create myapp
fi
cd myapp
cp ../../main.dart lib/main.dart
flutter build bundle --debug
cd -
#################################################################
# Run the Flutter Engine Embedder
#################################################################
#./flutter_glfw ./myapp /home/charafau/Projects/flutter/flatpaktest/build/linux/x64/debug/bundle/data/icudtl.dat
./embedder ./myapp /home/charafau/Utils/flutterSdk/flutterDev/bin/cache/artifacts/engine/linux-x64/icudtl.dat
