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

echo "==> Building app with MLAR integration" | tee -a ci_logs/build_steps.log
cd app
DERIVED="$ROOT/app/build"
mkdir -p "$DERIVED"

# Ensure MLARIntegration.swift is included by explicitly specifying all source files
echo "Compiling with MLAR integration..." | tee -a "$ROOT/ci_logs/build_steps.log"

# Build app directly with swiftc to ensure MLARIntegration.swift is included
SOURCES=(
  Sources/*.swift
)

# Create app structure
mkdir -p "$DERIVED/MoreMojoStudio.app/Contents/MacOS"
mkdir -p "$DERIVED/MoreMojoStudio.app/Contents/Resources"
mkdir -p "$DERIVED/MoreMojoStudio.app/Contents/Frameworks"

# Compile the app with MLARIntegration.swift
swiftc -sdk "$(xcrun --show-sdk-path)" \
  -target x86_64-apple-macosx10.15 \
  -o "$DERIVED/MoreMojoStudio.app/Contents/MacOS/MoreMojoStudio" \
  -emit-executable \
  "${SOURCES[@]}" \
  2>&1 | tee "$ROOT/ci_logs/xcodebuild_app.log"

# Create Info.plist
cat > "$DERIVED/MoreMojoStudio.app/Contents/Info.plist" << EOF
<?xml version="1.0" encoding="UTF-8"?>
<!DOCTYPE plist PUBLIC "-//Apple//DTD PLIST 1.0//EN" "http://www.apple.com/DTDs/PropertyList-1.0.dtd">
<plist version="1.0">
<dict>
    <key>CFBundleIdentifier</key>
    <string>com.moremojo.studio</string>
    <key>CFBundleName</key>
    <string>MoreMojoStudio</string>
    <key>CFBundleExecutable</key>
    <string>MoreMojoStudio</string>
    <key>CFBundlePackageType</key>
    <string>APPL</string>
    <key>CFBundleInfoDictionaryVersion</key>
    <string>6.0</string>
    <key>CFBundleVersion</key>
    <string>1.0</string>
    <key>CFBundleShortVersionString</key>
    <string>1.0</string>
    <key>NSPrincipalClass</key>
    <string>NSApplication</string>
    <key>NSHighResolutionCapable</key>
    <true/>
</dict>
</plist>
EOF

# Copy Assets
if [ -d "Assets.xcassets" ]; then
  cp -R "Assets.xcassets" "$DERIVED/MoreMojoStudio.app/Contents/Resources/"
fi

echo "==> Package artifact if present" | tee -a "$ROOT/ci_logs/build_steps.log"
cd "$ROOT"
mkdir -p dist
APP="app/build/MoreMojoStudio.app"
if [ -d "$APP" ]; then
  rm -rf "dist/MoreMojoStudio.app"
  cp -R "$APP" dist/
  echo "OK: App artifact at dist/MoreMojoStudio.app" | tee -a ci_logs/build_steps.log
else
  echo "WARN: no .app found at expected path" | tee -a ci_logs/xcodebuild_app.log
fi

# Create a launcher script to execute the app
cat > "dist/launch_app.sh" << EOF
#!/bin/bash
cd "\$(dirname "\$0")"
open MoreMojoStudio.app
EOF
chmod +x "dist/launch_app.sh"

echo "==> Build completed. Run ./dist/launch_app.sh to start the app." | tee -a ci_logs/build_steps.log
