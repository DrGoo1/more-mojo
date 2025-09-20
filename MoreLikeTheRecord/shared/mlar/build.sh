#!/bin/bash
# MLAR build script

# Exit on error
set -e

# Create build directory if it doesn't exist
mkdir -p build
cd build

# Configure with CMake
echo "Configuring MLAR with CMake..."
cmake .. -DCMAKE_BUILD_TYPE=Release

# Build
echo "Building MLAR library and test application..."
cmake --build . --config Release --parallel $(sysctl -n hw.ncpu)

echo "Build completed successfully!"
echo "To run the test application: ./MLARTest"

# If test app was built, offer to run it
if [ -f "MLARTest" ]; then
    echo ""
    read -p "Run test application? (y/n) " -n 1 -r
    echo ""
    if [[ $REPLY =~ ^[Yy]$ ]]; then
        ./MLARTest
    fi
fi
