#!/usr/bin/env bash
set -euo pipefail
ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
cd "$ROOT"

if [ -z "${JUCE_DIR:-}" ]; then
  echo "JUCE_DIR not set. Export JUCE_DIR=/path/to/JUCE"; exit 1
fi

cmake -B plugin/build -S plugin -DJUCE_DIR="$JUCE_DIR"
cmake --build plugin/build --config Release

mkdir -p dist
cp -R plugin/build/Release/*.component dist/ 2>/dev/null || true
cp -R plugin/build/Release/*.vst3 dist/ 2>/dev/null || true
echo "OK: Plugins in ./dist"
