#!/bin/bash
set -e

BUILD_DIR="build"

echo "=== Building SENTAnalyzer Plugin for Saleae Logic 2 ==="

mkdir -p "$BUILD_DIR"
cd "$BUILD_DIR"

cmake .. -DCMAKE_BUILD_TYPE=Release
cmake --build . --config Release

echo "=== Build Complete! ==="
echo "Shared library output is located in: $BUILD_DIR/Analyzers/"
