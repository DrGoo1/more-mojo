#!/bin/bash

# Build script for the Pro GUI test application
# This script builds the test application, linking it with our Pro GUI components

set -e

# Configuration
TEST_APP_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROGUI_DIR="$(cd "$TEST_APP_DIR/../.." && pwd)"
BUILD_DIR="$PROGUI_DIR/build"
FRAMEWORK_NAME="MoreMojoProGUI"
FRAMEWORK_DIR="$BUILD_DIR/$FRAMEWORK_NAME.framework"
TEST_BUILD_DIR="$TEST_APP_DIR/build"
JUCE_DIR="/Volumes/MP 1/MoreMojo/ChatGPT/JUCE"

# Export JUCE_DIR environment variable so CMake can find it
export JUCE_DIR

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

# Create build directories
mkdir -p "$TEST_BUILD_DIR"

# First, make sure the Pro GUI components are built
print_header "Building Pro GUI Components"

if [ ! -d "$FRAMEWORK_DIR" ]; then
    print_info "Pro GUI components not found, building them now..."
    
    # Check if build script exists
    if [ -f "$PROGUI_DIR/build_combined.sh" ]; then
        cd "$PROGUI_DIR"
        bash ./build_combined.sh
    else
        print_error "Could not find build_combined.sh script"
    fi
else
    print_info "Pro GUI components already built"
fi

# Build the test app
print_header "Building Test Application"

# Generate Info.plist for the app
cat > "$TEST_BUILD_DIR/Info.plist" << EOL
<?xml version="1.0" encoding="UTF-8"?>
<!DOCTYPE plist PUBLIC "-//Apple//DTD PLIST 1.0//EN" "http://www.apple.com/DTDs/PropertyList-1.0.dtd">
<plist version="1.0">
<dict>
    <key>CFBundleDevelopmentRegion</key>
    <string>en</string>
    <key>CFBundleExecutable</key>
    <string>ProGUITestApp</string>
    <key>CFBundleIconFile</key>
    <string>AppIcon</string>
    <key>CFBundleIdentifier</key>
    <string>com.moremojo.proguitestapp</string>
    <key>CFBundleInfoDictionaryVersion</key>
    <string>6.0</string>
    <key>CFBundleName</key>
    <string>ProGUITestApp</string>
    <key>CFBundlePackageType</key>
    <string>APPL</string>
    <key>CFBundleShortVersionString</key>
    <string>1.0</string>
    <key>CFBundleVersion</key>
    <string>1</string>
    <key>LSMinimumSystemVersion</key>
    <string>11.0</string>
    <key>NSHumanReadableCopyright</key>
    <string>Copyright © 2025 MoreMojo. All rights reserved.</string>
    <key>NSPrincipalClass</key>
    <string>NSApplication</string>
</dict>
</plist>
EOL

# Create the app structure
APP_BUNDLE_DIR="$TEST_BUILD_DIR/ProGUITestApp.app"
mkdir -p "$APP_BUNDLE_DIR/Contents/MacOS"
mkdir -p "$APP_BUNDLE_DIR/Contents/Resources"
mkdir -p "$APP_BUNDLE_DIR/Contents/Frameworks"

# Copy the Info.plist
cp "$TEST_BUILD_DIR/Info.plist" "$APP_BUNDLE_DIR/Contents/"

# Copy the framework
if [ -d "$FRAMEWORK_DIR" ]; then
    print_info "Copying framework to app bundle..."
    cp -R "$FRAMEWORK_DIR" "$APP_BUNDLE_DIR/Contents/Frameworks/"
else
    print_error "Framework not found at $FRAMEWORK_DIR"
fi

# Create a bridging header for Swift to access our C functions
BRIDGING_HEADER="$TEST_BUILD_DIR/ProGUITestApp-Bridging-Header.h"
cat > "$BRIDGING_HEADER" << EOL
//
//  ProGUITestApp-Bridging-Header.h
//  ProGUITestApp
//

#ifndef ProGUITestApp_Bridging_Header_h
#define ProGUITestApp_Bridging_Header_h

// Headers for MoreMojoProGUI.framework are added to the header search path
// so we can import directly by header name.
#import "ProGUIBridgeC.h"

#endif /* ProGUITestApp_Bridging_Header_h */
EOL

# Compile the Swift app
print_info "Compiling Swift application..."

PROGUI_SWIFT="$PROGUI_DIR/SwiftBridge/Swift/ProGUIBridge.swift"

swiftc -o "$APP_BUNDLE_DIR/Contents/MacOS/ProGUITestApp" \
    "$TEST_APP_DIR/ProGUITestApp.swift" \
    "$PROGUI_SWIFT" \
    -import-objc-header "$BRIDGING_HEADER" \
    -F"$APP_BUNDLE_DIR/Contents/Frameworks" \
    -I"$PROGUI_DIR/SwiftBridge" \
    -I"$APP_BUNDLE_DIR/Contents/Frameworks/$FRAMEWORK_NAME.framework/Headers" \
    -framework "$FRAMEWORK_NAME" \
    -framework Cocoa \
    -framework SwiftUI \
    -swift-version 5 \
    -parse-as-library

if [ $? -ne 0 ]; then
    print_error "Failed to compile Swift application"
fi

# Fix the run path to find the framework
install_name_tool -add_rpath "@executable_path/../Frameworks" \
    "$APP_BUNDLE_DIR/Contents/MacOS/ProGUITestApp"

# Ensure the app links to the framework-style path instead of the raw dylib name
if otool -L "$APP_BUNDLE_DIR/Contents/MacOS/ProGUITestApp" | grep -q "@rpath/lib$FRAMEWORK_NAME.dylib"; then
    print_info "Rewriting dylib dependency to framework path..."
    install_name_tool -change "@rpath/lib$FRAMEWORK_NAME.dylib" \
        "@rpath/$FRAMEWORK_NAME.framework/Versions/A/$FRAMEWORK_NAME" \
        "$APP_BUNDLE_DIR/Contents/MacOS/ProGUITestApp"
fi

print_success "Test application built successfully!"
print_info "You can now run the test app with: open $APP_BUNDLE_DIR"
