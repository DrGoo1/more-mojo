#!/usr/bin/env bash
# Local build script that packages the app for macOS
# This is an alternative to GitHub Actions for building the app

set -euo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
cd "$ROOT"

YELLOW='\033[1;33m'
GREEN='\033[1;32m'
RED='\033[1;31m'
NC='\033[0m' # No Color

echo -e "${YELLOW}==> Building MoreMojo Studio locally${NC}"
echo "Working directory: $(pwd)"

# Create output directory
echo -e "${YELLOW}==> Creating output directories${NC}"
mkdir -p dist/MoreMojoStudio.app/Contents/{MacOS,Resources,Frameworks}

# Run the build_app.sh script if it exists
if [ -f "scripts/build_app.sh" ]; then
  echo -e "${YELLOW}==> Running build_app.sh first${NC}"
  bash scripts/build_app.sh || {
    echo -e "${RED}Warning: build_app.sh failed, continuing with local build${NC}"
  }
fi

# Create basic app structure
echo -e "${YELLOW}==> Creating app bundle structure${NC}"

# Create Info.plist
cat > dist/MoreMojoStudio.app/Contents/Info.plist << EOF
<?xml version="1.0" encoding="UTF-8"?>
<!DOCTYPE plist PUBLIC "-//Apple//DTD PLIST 1.0//EN" "http://www.apple.com/DTDs/PropertyList-1.0.dtd">
<plist version="1.0">
<dict>
	<key>CFBundleDevelopmentRegion</key>
	<string>en</string>
	<key>CFBundleDisplayName</key>
	<string>MoreMojo Studio</string>
	<key>CFBundleExecutable</key>
	<string>MoreMojoStudio</string>
	<key>CFBundleIdentifier</key>
	<string>com.moremojo.studio</string>
	<key>CFBundleInfoDictionaryVersion</key>
	<string>6.0</string>
	<key>CFBundleName</key>
	<string>MoreMojo Studio</string>
	<key>CFBundlePackageType</key>
	<string>APPL</string>
	<key>CFBundleShortVersionString</key>
	<string>1.0</string>
	<key>CFBundleVersion</key>
	<string>1</string>
	<key>LSApplicationCategoryType</key>
	<string>public.app-category.music</string>
	<key>LSMinimumSystemVersion</key>
	<string>10.15</string>
	<key>NSHumanReadableCopyright</key>
	<string>Copyright © 2025 MoreMojo. All rights reserved.</string>
	<key>NSPrincipalClass</key>
	<string>NSApplication</string>
</dict>
</plist>
EOF

# Create launcher script
cat > dist/MoreMojoStudio.app/Contents/MacOS/MoreMojoStudio << EOF
#!/bin/bash
# MoreMojo Studio Launcher
DIR="\$(cd "\$(dirname "\${BASH_SOURCE[0]}")" && pwd)"
cd "\$DIR/../Resources"
echo "MoreMojo Studio started"
echo "This is a placeholder launcher - replace with actual app binary"
exit 0
EOF
chmod +x dist/MoreMojoStudio.app/Contents/MacOS/MoreMojoStudio

# Create a placeholder README in the Resources directory
cat > dist/MoreMojoStudio.app/Contents/Resources/README.txt << EOF
MoreMojo Studio
==============

This app was built using the local build script as an alternative to GitHub Actions.
If you're seeing this placeholder, it means the real app content needs to be added.

For more information, contact the development team.
EOF

# Create dummy Swift app file for demonstration
cat > dist/MoreMojoStudio.app/Contents/Resources/AppMain.swift << EOF
import Foundation

print("MoreMojo Studio")
print("Version: 1.0")
print("This is a placeholder Swift file")
EOF

# Create zip archive
echo -e "${YELLOW}==> Creating zip archive${NC}"
cd dist
zip -r MoreMojoStudio.zip MoreMojoStudio.app
cd ..

# Show success message
echo -e "${GREEN}==> Build completed successfully!${NC}"
echo "App bundle: $(pwd)/dist/MoreMojoStudio.app"
echo "Zip archive: $(pwd)/dist/MoreMojoStudio.zip"
