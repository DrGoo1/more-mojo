#!/bin/bash
set -e

# Build script for MoreMojo Pro GUI using JUCE framework
# This script configures and builds the GUI app using CMake

# Script directory
SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
cd "$SCRIPT_DIR"

# Colors for output
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
RED='\033[0;31m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# Create build directory
echo -e "${BLUE}[1/5] Creating build directory...${NC}"
mkdir -p build

# Detect JUCE location
echo -e "${BLUE}[2/5] Detecting JUCE location...${NC}"
if [ -d "../ChatGPT/JUCE" ]; then
    JUCE_DIR="../ChatGPT/JUCE"
    echo -e "${GREEN}Found JUCE at $JUCE_DIR${NC}"
    export JUCE_DIR="$JUCE_DIR"
else
    echo -e "${YELLOW}JUCE directory not found at standard location. Trying to find JUCE...${NC}"
    JUCE_DIR=$(find /Volumes/MP\ 1/MoreMojo -name "JUCE" -type d | grep -v build | head -1)
    
    if [ -z "$JUCE_DIR" ]; then
        echo -e "${RED}Error: JUCE directory not found. Please make sure JUCE is installed.${NC}"
        exit 1
    else
        echo -e "${GREEN}Found JUCE at $JUCE_DIR${NC}"
        export JUCE_DIR="$JUCE_DIR"
    fi
fi

# Configure with CMake
echo -e "${BLUE}[3/5] Configuring with CMake...${NC}"
cd build

# Create JuceLibraryCode directory if it doesn't exist
mkdir -p ../JuceLibraryCode

# Run CMake with proper configuration
cmake .. -DCMAKE_BUILD_TYPE=Debug -DCMAKE_EXPORT_COMPILE_COMMANDS=ON

# Build
echo -e "${BLUE}[4/5] Building MoreMojoProGUI...${NC}"
cmake --build . --config Debug

# Create assets directory if it doesn't exist
echo -e "${BLUE}[5/5] Creating assets directory...${NC}"
mkdir -p ../Assets

# Show completion message
echo -e "${GREEN}Build completed successfully!${NC}"

# On macOS, show the location of the app
if [ "$(uname)" == "Darwin" ]; then
    APP_PATH=$(find . -name "*.app" -type d)
    if [ -n "$APP_PATH" ]; then
        echo -e "${GREEN}App built at: $APP_PATH${NC}"
        echo -e "${YELLOW}Run the app with: open $APP_PATH${NC}"
    fi
fi
