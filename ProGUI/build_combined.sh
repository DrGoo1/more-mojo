#!/bin/bash

# Build script for combining Pro GUI components with the main app
# This script builds the Pro GUI components as a framework and integrates it with the Swift app

set -e

# Configuration
PROGUI_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
BUILD_DIR="$PROGUI_DIR/build"
MAIN_APP_DIR="/Volumes/MP 1/MoreMojo/ChatGPT"
JUCE_DIR="/Volumes/MP 1/MoreMojo/ChatGPT/JUCE"
FRAMEWORK_NAME="MoreMojoProGUI"
FRAMEWORK_DIR="$BUILD_DIR/$FRAMEWORK_NAME.framework"

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
}

# Print info message
function print_info() {
    echo -e "${YELLOW}INFO: $1${NC}"
}

# Check if JUCE exists
if [ ! -d "$JUCE_DIR" ]; then
    print_error "JUCE directory not found at $JUCE_DIR"
    print_info "Please set the correct JUCE_DIR in the script"
    exit 1
fi

# Create build directory if it doesn't exist
mkdir -p "$BUILD_DIR"

# Build Pro GUI components
print_header "Building Pro GUI Components"

# Use CMake to configure and build
cd "$PROGUI_DIR"

print_info "Configuring CMake build..."
# Ensure fresh configure so JUCE_DIR is honored
rm -f "$BUILD_DIR/CMakeCache.txt"
export JUCE_DIR
cmake -B "$BUILD_DIR" -S "$PROGUI_DIR" -DCMAKE_BUILD_TYPE=Release \
    -DJUCE_DIR="$JUCE_DIR" \
    -DBUILD_SHARED_LIBS=ON

print_info "Building Pro GUI components..."
cmake --build "$BUILD_DIR" --config Release

# Check if build was successful
if [ ! -f "$BUILD_DIR/libMoreMojoProGUI.dylib" ] && [ ! -d "$FRAMEWORK_DIR" ]; then
    print_error "Pro GUI components build failed"
    exit 1
fi

# Create framework structure if needed
if [ ! -d "$FRAMEWORK_DIR" ]; then
    print_info "Creating framework structure..."
    
    mkdir -p "$FRAMEWORK_DIR/Versions/A/Headers"
    mkdir -p "$FRAMEWORK_DIR/Versions/A/Resources"
    
    # Copy the dylib to the framework
    cp "$BUILD_DIR/libMoreMojoProGUI.dylib" "$FRAMEWORK_DIR/Versions/A/$FRAMEWORK_NAME"
    
    # Copy header files
    cp "$PROGUI_DIR/SwiftBridge/ProGUIBridgeC.h" "$FRAMEWORK_DIR/Versions/A/Headers/"
    
    # Create symbolic links
    ln -sf "A" "$FRAMEWORK_DIR/Versions/Current"
    ln -sf "Versions/Current/Headers" "$FRAMEWORK_DIR/Headers"
    ln -sf "Versions/Current/Resources" "$FRAMEWORK_DIR/Resources"
    ln -sf "Versions/Current/$FRAMEWORK_NAME" "$FRAMEWORK_DIR/$FRAMEWORK_NAME"
    
    # Create Info.plist
    cat > "$FRAMEWORK_DIR/Versions/A/Resources/Info.plist" << EOL
<?xml version="1.0" encoding="UTF-8"?>
<!DOCTYPE plist PUBLIC "-//Apple//DTD PLIST 1.0//EN" "http://www.apple.com/DTDs/PropertyList-1.0.dtd">
<plist version="1.0">
<dict>
    <key>CFBundleDevelopmentRegion</key>
    <string>English</string>
    <key>CFBundleExecutable</key>
    <string>$FRAMEWORK_NAME</string>
    <key>CFBundleIdentifier</key>
    <string>com.moremojo.progui</string>
    <key>CFBundleInfoDictionaryVersion</key>
    <string>6.0</string>
    <key>CFBundleName</key>
    <string>$FRAMEWORK_NAME</string>
    <key>CFBundlePackageType</key>
    <string>FMWK</string>
    <key>CFBundleShortVersionString</key>
    <string>1.0</string>
    <key>CFBundleVersion</key>
    <string>1</string>
    <key>NSHumanReadableCopyright</key>
    <string>Copyright 2025 MoreMojo. All rights reserved.</string>
</dict>
</plist>
EOL

    # Set the framework binary install name to a framework-style rpath
    install_name_tool -id "@rpath/$FRAMEWORK_NAME.framework/Versions/A/$FRAMEWORK_NAME" \
        "$FRAMEWORK_DIR/Versions/A/$FRAMEWORK_NAME"
fi

# Copy Swift bridge files to main app
print_header "Copying Swift bridge files to main app"

SWIFT_BRIDGE_DIR="$MAIN_APP_DIR/Source/ProGUI"
mkdir -p "$SWIFT_BRIDGE_DIR"

print_info "Copying Swift bridge files..."
cp "$PROGUI_DIR/SwiftBridge/Swift/ProGUIBridge.swift" "$SWIFT_BRIDGE_DIR/"
cp "$PROGUI_DIR/SwiftBridge/Swift/MoreMojo-Bridging-Header.h" "$SWIFT_BRIDGE_DIR/"

# Copy framework to main app
print_header "Copying framework to main app"

MAIN_APP_FRAMEWORKS_DIR="$MAIN_APP_DIR/Frameworks"
mkdir -p "$MAIN_APP_FRAMEWORKS_DIR"

print_info "Copying framework..."
cp -R "$FRAMEWORK_DIR" "$MAIN_APP_FRAMEWORKS_DIR/"

# Add Swift usage example
print_header "Creating Swift usage example"

EXAMPLE_DIR="$MAIN_APP_DIR/Examples"
mkdir -p "$EXAMPLE_DIR"

cat > "$EXAMPLE_DIR/ProGUIExample.swift" << EOL
import SwiftUI
import Cocoa

// Example of how to use Pro GUI components in SwiftUI
struct ProGUIEQView: NSViewRepresentable {
    // EQ parameters
    @Binding var band1Gain: Float
    @Binding var band1Freq: Float
    @Binding var band1Q: Float
    
    // Audio buffer for processing
    var audioBuffer: (left: [Float], right: [Float])?
    
    func makeNSView(context: Context) -> NSView {
        // Create the ProGUIBridge
        let bridge = ProGUIBridge(type: .eq, frame: NSRect(x: 0, y: 0, width: 600, height: 400))
        
        // Set initial parameter values
        bridge.setEQBandGain(band: 1, gainDB: band1Gain)
        bridge.setEQBandFrequency(band: 1, frequency: band1Freq)
        bridge.setEQBandQ(band: 1, q: band1Q)
        
        return bridge.view
    }
    
    func updateNSView(_ nsView: NSView, context: Context) {
        // Get the ProGUIBridge from the view's first subview
        if let view = nsView.subviews.first?.superview as? NSView,
           let bridge = context.coordinator.bridge {
            
            // Update parameters if they've changed
            let currentGain = bridge.getEQBandGain(band: 1)
            if currentGain != band1Gain {
                bridge.setEQBandGain(band: 1, gainDB: band1Gain)
            }
            
            let currentFreq = bridge.getEQBandFrequency(band: 1)
            if currentFreq != band1Freq {
                bridge.setEQBandFrequency(band: 1, frequency: band1Freq)
            }
            
            let currentQ = bridge.getEQBandQ(band: 1)
            if currentQ != band1Q {
                bridge.setEQBandQ(band: 1, q: band1Q)
            }
            
            // Process audio if available
            if let buffer = audioBuffer {
                bridge.processAudio(leftChannel: buffer.left, rightChannel: buffer.right)
            }
        }
    }
    
    func makeCoordinator() -> Coordinator {
        Coordinator(self)
    }
    
    class Coordinator {
        var parent: ProGUIEQView
        var bridge: ProGUIBridge?
        
        init(_ parent: ProGUIEQView) {
            self.parent = parent
            self.bridge = ProGUIBridge(type: .eq, frame: NSRect(x: 0, y: 0, width: 600, height: 400))
        }
    }
}
EOL

print_success "Build completed successfully!"
print_info "Pro GUI framework is located at: $MAIN_APP_FRAMEWORKS_DIR/$FRAMEWORK_NAME.framework"
print_info "Swift bridge files are located at: $SWIFT_BRIDGE_DIR"
print_info "Example usage is located at: $EXAMPLE_DIR/ProGUIExample.swift"
