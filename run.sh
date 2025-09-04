#!/usr/bin/env bash

set -e

BUILD_DIR="build/debug"

if [ ! -d "$BUILD_DIR" ]; then
  echo "Build directory not found. Please run build.sh first."
  exit 1
fi

echo "Running Raptor from $BUILD_DIR"
pushd "$BUILD_DIR" > /dev/null
./Raptor
popd > /dev/null
