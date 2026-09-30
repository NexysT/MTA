#!/usr/bin/env bash
set -euo pipefail
mkdir -p build
g++ -std=c++17 -O2 -fPIC -fvisibility=hidden -shared \
  playerbots_native_linux_x64.cpp -ldl \
  -o build/playerbots_native.so
echo "OK: build/playerbots_native.so"
