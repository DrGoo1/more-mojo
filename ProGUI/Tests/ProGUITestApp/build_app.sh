#!/bin/bash

# Build script for Pro GUI Test App
# This script builds a macOS application that demonstrates the integration of Pro GUI components

set -e

# Configuration
APP_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
BUILD_DIR="$APP_DIR/build"
APP_NAME="ProGUITestApp"

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

# Copy Info.plist
cp "$APP_DIR/Info.plist" "$APP_BUNDLE/Contents/"

# Create a minimal MainMenu.nib
print_info "Creating minimal NIB file..."
mkdir -p "$APP_BUNDLE/Contents/Resources/Base.lproj"
cat > "$APP_BUNDLE/Contents/Resources/Base.lproj/MainMenu.nib" << EOL
bplist00Ñ
EOL

# Build the Swift app
print_header "Building Pro GUI Test App"

print_info "Compiling Swift application..."

swiftc -o "$APP_BUNDLE/Contents/MacOS/$APP_NAME" \
    "$APP_DIR/AppDelegate.swift" \
    "$APP_DIR/ContentView.swift" \
    -target x86_64-apple-macos11.0 \
    -framework SwiftUI \
    -framework Cocoa \
    -framework AppKit

if [ $? -eq 0 ]; then
    print_success "Build completed successfully!"
    print_info "Running the test app..."
    open "$APP_BUNDLE"
else
    print_error "Failed to build test app"
fi
