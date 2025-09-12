#!/usr/bin/env bash
set -euo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
APP_DIR="$ROOT/app"
LOG_DIR="$ROOT/ci_logs"
DERIVED="$APP_DIR/build"
XCRESULT="$DERIVED/App.xcresult"
COMPAT_DIR="$APP_DIR/compat_project"

mkdir -p "$LOG_DIR" "$DERIVED"

echo "==> Tooling"
{
  echo "=== Tooling ==="
  xcodebuild -version || true
  sw_vers || true
  swift --version || true
} | tee "$LOG_DIR/app_tooling.txt"

echo "==> xcodebuild (Release)"

# Try original project first
if xcodebuild -project "$APP_DIR/MoreMojoStudio.xcodeproj" -list &>/dev/null; then
  echo "Building original project..."
  xcodebuild \
    -project "$APP_DIR/MoreMojoStudio.xcodeproj" \
    -scheme "MoreMojoStudio" \
    -configuration Release \
    -sdk macosx \
    -derivedDataPath "$DERIVED" \
    -resultBundlePath "$XCRESULT" \
    CODE_SIGNING_ALLOWED=NO CODE_SIGNING_REQUIRED=NO DEVELOPMENT_TEAM="" \
    OTHER_CODE_SIGN_FLAGS="--keychain none" \
    build 2>&1 | tee "$LOG_DIR/xcodebuild_app_stdout.log"
  echo "==> Done. App at: $DERIVED/Build/Products/Release/MoreMojoStudio.app"
  exit 0
fi

echo "Original project failed, trying compatibility project..."

# Ensure compat project exists
if [ ! -d "$COMPAT_DIR" ] || ! find "$COMPAT_DIR" -name "*.xcodeproj" | grep -q "\.xcodeproj"; then
  echo "Generating compatibility project..."
  "$ROOT/scripts/generate_compat_project.sh"
fi

# Find and build compat project
COMPAT_PROJ=$(find "$COMPAT_DIR" -name "*.xcodeproj" | head -1)
if [ -n "$COMPAT_PROJ" ]; then
  COMPAT_SCHEME=$(xcodebuild -project "$COMPAT_PROJ" -list | grep -A 10 "Schemes:" | grep -v "Schemes:" | head -1 | xargs)
  
  echo "Building compatibility project: $COMPAT_PROJ with scheme: $COMPAT_SCHEME"
  xcodebuild \
    -project "$COMPAT_PROJ" \
    -scheme "$COMPAT_SCHEME" \
    -configuration Release \
    -sdk macosx \
    -derivedDataPath "$DERIVED" \
    -resultBundlePath "$XCRESULT" \
    CODE_SIGNING_ALLOWED=NO CODE_SIGNING_REQUIRED=NO DEVELOPMENT_TEAM="" \
    OTHER_CODE_SIGN_FLAGS="--keychain none" \
    build 2>&1 | tee "$LOG_DIR/xcodebuild_compat_stdout.log"
    
  echo "==> Done. Compatibility build complete."
else
  echo "Error: No compatibility project found or generated."
  exit 1
fi
