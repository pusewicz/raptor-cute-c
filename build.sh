#!/usr/bin/env bash

set -e

BUILD_DIR="build/debug"

if [ ! -d "$BUILD_DIR" ]; then
  cmake -S "$(pwd)" -B "$BUILD_DIR" -DCMAKE_BUILD_TYPE=Debug -G Ninja
fi

cmake --build "$BUILD_DIR" --config Debug --parallel
