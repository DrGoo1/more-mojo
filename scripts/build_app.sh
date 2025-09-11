#!/usr/bin/env bash
set -euo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
cd "$ROOT"

mkdir -p ci_logs

echo "==> Preflight fixups (optional - keep if you already have this)" | tee -a ci_logs/build_steps.log
# Use preflight_fix.sh if it exists, but don't fail if it doesn't
bash scripts/preflight_fix.sh || true

echo "==> Generate Xcode project (XcodeGen if used)" | tee -a ci_logs/build_steps.log
if command -v xcodegen >/dev/null 2>&1; then
  (cd app && xcodegen generate)
fi

echo "==> Build app (xcodebuild)" | tee -a ci_logs/build_steps.log
cd app
DERIVED="$ROOT/app/build"
XCRESULT="$DERIVED/App.xcresult"
mkdir -p "$DERIVED"

# Capture build output to both stdout and log file
xcodebuild -project "MoreMojoStudio.xcodeproj" \
  -scheme "MoreMojoStudio" \
  -configuration Release \
  -derivedDataPath "$DERIVED" \
  -resultBundlePath "$XCRESULT" \
  -destination 'platform=macOS' \
  -showBuildTimingSummary \
  clean build 2>&1 | tee "$ROOT/ci_logs/xcodebuild_app.log"

echo "==> Package artifact if present" | tee -a ci_logs/build_steps.log
cd "$ROOT"
mkdir -p dist
APP="app/build/Build/Products/Release/MoreMojoStudio.app"
if [ -d "$APP" ]; then
  rm -rf "dist/MoreMojoStudio.app"
  cp -R "$APP" dist/
  echo "OK: App artifact at dist/MoreMojoStudio.app" | tee -a ci_logs/build_steps.log
else
  echo "WARN: no .app found at expected path" | tee -a ci_logs/xcodebuild_app.log
fi
