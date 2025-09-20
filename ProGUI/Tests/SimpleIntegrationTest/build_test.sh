#!/bin/bash

# Build script for the simple Pro GUI integration test
# This script builds a SwiftUI application that demonstrates the integration of Pro GUI components

set -e

# Configuration
TEST_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
BUILD_DIR="$TEST_DIR/build"
APP_NAME="SimpleIntegrationTestApp"

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[0;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# Print section header
function print_header() {
    echo -e "\n${BLUE}===== $1 =====${NC}\n"
}

# Print success message
function print_success() {
    echo -e "${GREEN}SUCCESS: $1${NC}"
}

# Print error message
function print_error() {
    echo -e "${RED}ERROR: $1${NC}"
    exit 1
}

# Print info message
function print_info() {
    echo -e "${YELLOW}INFO: $1${NC}"
}

# Create build directory
mkdir -p "$BUILD_DIR"

# Create app directory structure
APP_BUNDLE="$BUILD_DIR/$APP_NAME.app"
mkdir -p "$APP_BUNDLE/Contents/MacOS"
mkdir -p "$APP_BUNDLE/Contents/Resources"

# Create Info.plist
cat > "$APP_BUNDLE/Contents/Info.plist" << EOL
<?xml version="1.0" encoding="UTF-8"?>
<!DOCTYPE plist PUBLIC "-//Apple//DTD PLIST 1.0//EN" "http://www.apple.com/DTDs/PropertyList-1.0.dtd">
<plist version="1.0">
<dict>
    <key>CFBundleDevelopmentRegion</key>
    <string>en</string>
    <key>CFBundleExecutable</key>
    <string>$APP_NAME</string>
    <key>CFBundleIdentifier</key>
    <string>com.moremojo.simpleintegrationtest</string>
    <key>CFBundleInfoDictionaryVersion</key>
    <string>6.0</string>
    <key>CFBundleName</key>
    <string>$APP_NAME</string>
    <key>CFBundlePackageType</key>
    <string>APPL</string>
    <key>CFBundleShortVersionString</key>
    <string>1.0</string>
    <key>CFBundleVersion</key>
    <string>1</string>
    <key>LSMinimumSystemVersion</key>
    <string>11.0</string>
    <key>NSPrincipalClass</key>
    <string>NSApplication</string>
</dict>
</plist>
EOL

# Build the Swift app
print_header "Building Swift Test App"

print_info "Compiling Swift application..."

swiftc -o "$APP_BUNDLE/Contents/MacOS/$APP_NAME" \
    "$TEST_DIR/SimpleMainFixed.swift" \
    -target x86_64-apple-macos11.0 \
    -parse-as-library \
    -framework SwiftUI \
    -framework Cocoa \
    -framework AppKit

if [ $? -eq 0 ]; then
    print_success "Build completed successfully!"
    print_info "You can now run the test app with: open '$APP_BUNDLE'"
else
    print_error "Failed to build test app"
fi
