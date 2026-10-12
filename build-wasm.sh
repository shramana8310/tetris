#!/usr/bin/env bash
set -e

emcmake cmake -S . -B build-wasm \
  -DSDL3_DIR="$HOME/emsdk-libs/lib/cmake/SDL3" \
  -DSDL3_ttf_DIR="$HOME/emsdk-libs/lib/cmake/SDL3_ttf" \
  -DCMAKE_PREFIX_PATH="$HOME/emsdk-libs"

cmake --build build-wasm
