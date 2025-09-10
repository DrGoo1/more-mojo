#!/usr/bin/env bash
set -euo pipefail

# Usage:
#   install_plugin.sh <DEPLOY_DIR> <PLUGIN_BASE_NAME>
#
# Example:
#   install_plugin.sh plugin/build/deploy MoreMojoPlugin
#
# This copies any built bundles (AU/VST3/Standalone) from the deploy/ tree
# into the user's standard macOS plug-in folders.

DEPLOY_ROOT="${1:-}"
PLUGIN_BASE_NAME="${2:-}"

if [ -z "$DEPLOY_ROOT" ] || [ -z "$PLUGIN_BASE_NAME" ]; then
  echo "Usage: $0 <DEPLOY_DIR> <PLUGIN_BASE_NAME>"
  echo "Example: $0 plugin/build/deploy MoreMojoPlugin"
  exit 1
fi

AU_DST="$HOME/Library/Audio/Plug-Ins/Components"
VST3_DST="$HOME/Library/Audio/Plug-Ins/VST3"
STANDALONE_DST="$HOME/Applications"

mkdir -p "$AU_DST" "$VST3_DST" "$STANDALONE_DST"

echo "==> Installing from: $DEPLOY_ROOT"
echo "==> Plugin base name: $PLUGIN_BASE_NAME"

# AU (.component)
if [ -d "$DEPLOY_ROOT/AU/$PLUGIN_BASE_NAME.component" ]; then
  echo "Installing AU -> $AU_DST/$PLUGIN_BASE_NAME.component"
  rm -rf "$AU_DST/$PLUGIN_BASE_NAME.component"
  cp -R "$DEPLOY_ROOT/AU/$PLUGIN_BASE_NAME.component" "$AU_DST/"
else
  echo "AU bundle not found at: $DEPLOY_ROOT/AU/$PLUGIN_BASE_NAME.component (skipping)"
fi

# VST3 (.vst3)
if [ -d "$DEPLOY_ROOT/VST3/$PLUGIN_BASE_NAME.vst3" ]; then
  echo "Installing VST3 -> $VST3_DST/$PLUGIN_BASE_NAME.vst3"
  rm -rf "$VST3_DST/$PLUGIN_BASE_NAME.vst3"
  cp -R "$DEPLOY_ROOT/VST3/$PLUGIN_BASE_NAME.vst3" "$VST3_DST/"
else
  echo "VST3 bundle not found at: $DEPLOY_ROOT/VST3/$PLUGIN_BASE_NAME.vst3 (skipping)"
fi

# Standalone (.app) — optional
if [ -d "$DEPLOY_ROOT/Standalone/$PLUGIN_BASE_NAME.app" ]; then
  echo "Installing Standalone -> $STANDALONE_DST/$PLUGIN_BASE_NAME.app"
  rm -rf "$STANDALONE_DST/$PLUGIN_BASE_NAME.app"
  cp -R "$DEPLOY_ROOT/Standalone/$PLUGIN_BASE_NAME.app" "$STANDALONE_DST/"
else
  echo "Standalone app not found at: $DEPLOY_ROOT/Standalone/$PLUGIN_BASE_NAME.app (skipping)"
fi

echo "✅ Install finished"
