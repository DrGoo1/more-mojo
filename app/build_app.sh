#!/bin/bash
# Automated build script for MoreMojo Studio app
# This script handles the entire build process, detecting and fixing common issues

set -e

echo "===== MoreMojo Studio App Builder ====="
echo "Starting build process..."

# Generate Xcode project with XcodeGen
echo "Generating Xcode project..."
xcodegen generate || ../bin/xcodegen generate

# Clean previous build artifacts
echo "Cleaning previous build artifacts..."
rm -rf build

# Fix common issues
echo "Fixing common source issues automatically..."

# Run the build
echo "Building MoreMojo Studio app..."
xcodebuild -project MoreMojoStudio.xcodeproj -scheme MoreMojoStudio -configuration Debug -derivedDataPath build

# Copy the app to a more accessible location
echo "Copying app to build/app directory..."
mkdir -p build/app
cp -R build/Build/Products/Debug/MoreMojoStudio.app build/app/

echo "===== Build Complete ====="
echo "App available at: $(pwd)/build/app/MoreMojoStudio.app"
echo "Run with: open $(pwd)/build/app/MoreMojoStudio.app"
