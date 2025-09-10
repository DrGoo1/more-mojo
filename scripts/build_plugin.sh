#!/usr/bin/env bash
set -euo pipefail

# === Configuration ===
# JUCE_DIR must point to your JUCE checkout (can be set in CI or shell profile)
if [ -z "${JUCE_DIR:-}" ]; then
  echo "ERROR: JUCE_DIR is not set. Please export JUCE_DIR=/path/to/JUCE"
  exit 1
fi

# Base plugin name must match the one you pass to juce_add_plugin(...)
# and the PLUGIN_BASE_NAME you used in CMakeLists.txt
PLUGIN_BASE_NAME="MoreMojoPlugin"

# Paths
ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
PLUGIN_DIR="$ROOT/plugin"
BUILD_DIR="$PLUGIN_DIR/build"
DEPLOY_DIR="$BUILD_DIR/deploy"

echo "==> Configuring CMake"
cmake -B "$BUILD_DIR" -S "$PLUGIN_DIR" -DJUCE_DIR="$JUCE_DIR"

echo "==> Building (Release)"
cmake --build "$BUILD_DIR" --config Release

echo "==> Deploy tree (if post-build copy ran from CMake):"
if [ -d "$DEPLOY_DIR" ]; then
  find "$DEPLOY_DIR" -maxdepth 2 -mindepth 1 -print || true
else
  echo "NOTE: $DEPLOY_DIR does not exist. If you disabled the CMake deploy block, installer will still work if bundles exist in build outputs."
fi

# Ensure installer exists
if [ ! -x "$PLUGIN_DIR/scripts/install_plugin.sh" ]; then
  echo "ERROR: $PLUGIN_DIR/scripts/install_plugin.sh not found or not executable."
  echo "       Create it and run: chmod +x plugin/scripts/install_plugin.sh"
  exit 1
fi

echo "==> Installing to user plug-in folders"
"$PLUGIN_DIR/scripts/install_plugin.sh" "$DEPLOY_DIR" "$PLUGIN_BASE_NAME"

echo "✅ Plugin build complete"
