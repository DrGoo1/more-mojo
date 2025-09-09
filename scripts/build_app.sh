#!/usr/bin/env bash
set -euo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
cd "$ROOT"

echo "==> Preflight: ensure tools"
if ! command -v xcodegen >/dev/null 2>&1; then
  echo "xcodegen missing; install with Homebrew."; exit 1
fi

echo "==> Preflight fixups (Swift sources)"
bash scripts/preflight_fix.sh

echo "==> Generate Xcode project (XcodeGen)"
cd app
xcodegen generate

echo "==> Validate single @main"
swift ../scripts/check_main.swift Sources || {
  echo "ERROR: @main validation failed. See messages above."; exit 1;
}

echo "==> Build app (xcodebuild)"
DERIVED="build"
xcodebuild -project "MoreMojoStudio.xcodeproj" \
  -scheme "MoreMojoStudio" \
  -configuration Release \
  -derivedDataPath "$DERIVED" \
  clean build

echo "==> Package artifact"
cd "$ROOT"
mkdir -p dist
APP="app/build/Build/Products/Release/MoreMojoStudio.app"
if [ -d "$APP" ]; then
  rm -rf "dist/MoreMojoStudio.app"
  cp -R "$APP" dist/
  echo "OK: App artifact at dist/MoreMojoStudio.app"
else
  echo "ERROR: build produced no .app in expected path"; exit 1
fi
