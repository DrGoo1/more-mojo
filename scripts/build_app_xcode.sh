#!/usr/bin/env bash
set -euo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
APP_DIR="$ROOT/app"
LOG_DIR="$ROOT/ci_logs"
DERIVED="$APP_DIR/build"
XCRESULT="$DERIVED/App.xcresult"

mkdir -p "$LOG_DIR" "$DERIVED"

echo "==> Tooling"
{
  echo "=== Tooling ==="
  xcodebuild -version || true
  sw_vers || true
  swift --version || true
} | tee "$LOG_DIR/app_tooling.txt"

echo "==> xcodebuild (Release)"
xcodebuild \
  -project "$APP_DIR/MoreMojoStudio.xcodeproj" \
  -scheme "MoreMojoStudio" \
  -configuration Release \
  -sdk macosx \
  -derivedDataPath "$DERIVED" \
  -resultBundlePath "$XCRESULT" \
  build 2>&1 | tee "$LOG_DIR/xcodebuild_app_stdout.log"

echo "==> Done. App at: $DERIVED/Build/Products/Release/MoreMojoStudio.app"
