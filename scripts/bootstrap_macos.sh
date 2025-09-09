#!/bin/bash
# Bootstrap script for macOS
# Installs required tools for building MoreMojo app and plugins

set -e

echo "MoreMojo Suite - macOS Bootstrap"
echo "================================"

# Function to check if a command exists
command_exists() {
    command -v "$1" >/dev/null 2>&1
}

# Install Homebrew if not already installed
if ! command_exists brew; then
    echo "Installing Homebrew..."
    /bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"
else
    echo "Homebrew is already installed"
fi

# Make sure Homebrew is in PATH
eval "$(/opt/homebrew/bin/brew shellenv)" 2>/dev/null || eval "$(brew shellenv)" 2>/dev/null || true

# Install XcodeGen if not already installed
if ! command_exists xcodegen; then
    echo "Installing XcodeGen..."
    brew install xcodegen
else
    echo "XcodeGen is already installed"
fi

# Install CMake if not already installed
if ! command_exists cmake; then
    echo "Installing CMake..."
    brew install cmake
else
    echo "CMake is already installed"
fi

# Create bin directory if it doesn't exist
mkdir -p bin

# Install XcodeGen directly if brew install failed
if ! command_exists xcodegen; then
    echo "Installing XcodeGen from binary..."
    curl -L https://github.com/yonaskolb/XcodeGen/releases/download/2.38.0/xcodegen.zip > xcodegen.zip
    unzip -o xcodegen.zip -d ./bin
    rm xcodegen.zip
    chmod +x ./bin/xcodegen
    echo "XcodeGen installed in ./bin"
fi

echo ""
echo "Bootstrap completed successfully!"
echo "You can now run 'make app' to build the MoreMojo Studio app"
echo "or 'make plugin' to build the audio plugins."
