#!/bin/bash
set -e

echo "[*] Configuring with CMake..."
cmake -B build -DCMAKE_BUILD_TYPE=Release

echo "[*] Building project..."
cmake --build build -j$(nproc)
