#!/usr/bin/env bash
set -euo pipefail

# Build script that manages the full build process
BASE_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
TARGET_DIR="$BASE_DIR/dist"
mkdir -p "$TARGET_DIR"

# Utils
function echo_info {
  echo "ℹ️ $1"
}

function echo_success {
  echo "✅ $1"
}

function echo_error {
  echo "❌ $1"
}

# Part 1: Setup and environment
echo_info "Running Part 1: Environment setup..."
if [ -x "${BASE_DIR}/scripts/bootstrap_macos.sh" ]; then
  "${BASE_DIR}/scripts/bootstrap_macos.sh"
fi

# Part 2: Plugin build process
echo_info "Running Part 2: Plugin build..."
if [ -x "${BASE_DIR}/scripts/build_plugin.sh" ]; then
  "${BASE_DIR}/scripts/build_plugin.sh"
fi

# Part 3: Preflight checks
echo_info "Running Part 3: App preflight checks..."
if [ -x "${BASE_DIR}/scripts/preflight_guard.sh" ]; then
  "${BASE_DIR}/scripts/preflight_guard.sh"
fi

# Part 4: App build via Xcode (preferred)
echo_info "Running Part 4: App build via Xcode..."
if [ -x "${BASE_DIR}/scripts/build_app_xcode.sh" ]; then
  "${BASE_DIR}/scripts/build_app_xcode.sh"
elif [ -x "${BASE_DIR}/../scripts/build_app_xcode.sh" ]; then
  "${BASE_DIR}/../scripts/build_app_xcode.sh"
else
  echo_error "scripts/build_app_xcode.sh not found"
  exit 1
fi

echo "Building MoreMojoStudio SwiftUI app with Xcode (see ../scripts/build_app_xcode.sh)"

# Final packaging
echo_info "Running final packaging..."
DIST_DIR="${BASE_DIR}/dist"
mkdir -p "${DIST_DIR}"

APP_PATH="${BASE_DIR}/app/build/Build/Products/Release/MoreMojoStudio.app"
if [ -d "$APP_PATH" ]; then
  cp -R "$APP_PATH" "${DIST_DIR}/"
  echo_success "App copied to ${DIST_DIR}/MoreMojoStudio.app"
else
  echo_error "App not found at expected path: $APP_PATH"
  exit 1
fi

echo_success "Build process completed successfully!"
