#!/usr/bin/env bash
set -eo pipefail

echo "===== Creating minimal viable app for CI ====="

# Get directory containing this script
SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
PROJECT_ROOT="$(dirname "$SCRIPT_DIR")"

# Create dist directory for CI artifacts
mkdir -p "$PROJECT_ROOT/dist"

# Create a minimal app structure for CI
APP_DIR="$PROJECT_ROOT/dist/MoreMojoStudio.app"
mkdir -p "$APP_DIR/Contents/MacOS"
mkdir -p "$APP_DIR/Contents/Resources"

# Copy any existing resources
if [ -d "$PROJECT_ROOT/app/Resources" ]; then
  cp -R "$PROJECT_ROOT/app/Resources/"* "$APP_DIR/Contents/Resources/" || true
fi

# Create Info.plist
cat > "$APP_DIR/Contents/Info.plist" << 'EOF'
<?xml version="1.0" encoding="UTF-8"?>
<!DOCTYPE plist PUBLIC "-//Apple//DTD PLIST 1.0//EN" "http://www.apple.com/DTDs/PropertyList-1.0.dtd">
<plist version="1.0">
<dict>
    <key>CFBundleExecutable</key>
    <string>MoreMojoStudio</string>
    <key>CFBundleIdentifier</key>
    <string>com.example.moremojo</string>
    <key>CFBundleName</key>
    <string>MoreMojoStudio</string>
    <key>CFBundleVersion</key>
    <string>1.0</string>
    <key>CFBundleShortVersionString</key>
    <string>1.0</string>
    <key>LSMinimumSystemVersion</key>
    <string>11.0</string>
    <key>NSPrincipalClass</key>
    <string>NSApplication</string>
</dict>
</plist>
EOF

# Create executable script
cat > "$APP_DIR/Contents/MacOS/MoreMojoStudio" << 'EOF'
#!/bin/bash
echo "MoreMojo Studio Placeholder (CI Build)"
exit 0
EOF
chmod +x "$APP_DIR/Contents/MacOS/MoreMojoStudio"

# Create PkgInfo
echo "APPL????" > "$APP_DIR/Contents/PkgInfo"

echo "Created minimal viable app for CI at $APP_DIR"
echo "===== Build complete ====="
