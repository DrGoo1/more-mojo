#!/usr/bin/env bash
set -euo pipefail

# Master build script that orchestrates the build process
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

# Part 1: Setup
echo_info "Starting Part 1: Environment setup..."
if [ -x "${BASE_DIR}/scripts/bootstrap_macos.sh" ]; then
  "${BASE_DIR}/scripts/bootstrap_macos.sh"
fi
echo_success "Part 1 completed successfully"

# Part 2: Plugin build 
echo_info "Starting Part 2: Plugin build..."
if [ -x "${BASE_DIR}/scripts/build_plugin.sh" ]; then
  "${BASE_DIR}/scripts/build_plugin.sh"
fi
echo_success "Part 2 completed successfully"

# Part 3: App setup and preflight
echo_info "Starting Part 3: App setup and preflight..."
if [ -x "${BASE_DIR}/scripts/app_preflight_fix.sh" ]; then
  "${BASE_DIR}/scripts/app_preflight_fix.sh"
fi
echo_success "Part 3 completed successfully"

# Part 4: App build via Xcode (preferred over raw swiftc)
echo_info "Starting Part 4: Building app with Xcode..."
if [ -x "${BASE_DIR}/scripts/build_app_xcode.sh" ]; then
  "${BASE_DIR}/scripts/build_app_xcode.sh"
else
  if [ -x "${BASE_DIR}/../scripts/build_app_xcode.sh" ]; then
    "${BASE_DIR}/../scripts/build_app_xcode.sh"
  else
    echo_error "scripts/build_app_xcode.sh not found; please add it and re-run."
    exit 1
  fi
fi
echo_success "App build completed"

# MoreMojoStudio legacy swiftc build script with a wrapper that calls Xcode build
cat > "${TARGET_DIR}/build_xcode.sh" << 'EOFBuildScript'
#!/bin/bash
set -euo pipefail
REPO_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
if [ -x "$REPO_ROOT/scripts/build_app_xcode.sh" ]; then
  "$REPO_ROOT/scripts/build_app_xcode.sh"
else
  echo "scripts/build_app_xcode.sh not found"
  exit 1
fi
EOFBuildScript
chmod +x "${TARGET_DIR}/build_xcode.sh"

echo_success "Build master process completed successfully"
