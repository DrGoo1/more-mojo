#!/bin/bash
# MoreMojo "More Like The Record" Plugin Build Script
# ------------------------------------------------

# Exit on error
set -e

# Print colored status messages
print_status() {
    echo -e "\033[1;34m[BUILD]\033[0m $1"
}

print_success() {
    echo -e "\033[1;32m[SUCCESS]\033[0m $1"
}

print_error() {
    echo -e "\033[1;31m[ERROR]\033[0m $1"
    exit 1
}

# Check if JUCE exists in parent directory or subdirectory
find_juce() {
    if [ -d "JUCE" ]; then
        JUCE_PATH="JUCE"
    elif [ -d "../JUCE" ]; then
        JUCE_PATH="../JUCE"
    else
        print_status "JUCE not found. Cloning JUCE from GitHub..."
        git clone https://github.com/juce-framework/JUCE.git
        JUCE_PATH="JUCE"
    fi
    
    print_status "Using JUCE at: $JUCE_PATH"
}

# Create directories if they don't exist
create_dirs() {
    print_status "Creating directory structure..."
    
    # Create resources directory if it doesn't exist
    if [ ! -d "Resources" ]; then
        mkdir -p Resources/icons
    fi
    
    if [ ! -d "Resources/icons" ]; then
        mkdir -p Resources/icons
    fi
    
    # Create build directory
    mkdir -p build
}

# Configure CMake
configure_cmake() {
    print_status "Configuring CMake build..."
    cd build
    
    # Determine build type
    BUILD_TYPE="Release"
    if [ "$1" == "debug" ]; then
        BUILD_TYPE="Debug"
    fi
    
    # Run CMake configuration
    cmake .. -DCMAKE_BUILD_TYPE=$BUILD_TYPE -DJUCE_DIR=../$JUCE_PATH
    
    cd ..
}

# Build the plugin
build_plugin() {
    print_status "Building plugin ($BUILD_TYPE configuration)..."
    cd build
    
    # Determine number of CPU cores for parallel build
    if [ -n "$(command -v nproc)" ]; then
        # Linux
        NUM_CORES=$(nproc)
    else
        # macOS
        NUM_CORES=$(sysctl -n hw.ncpu)
    fi
    
    # Build with all available cores
    cmake --build . --config $BUILD_TYPE -j $NUM_CORES
    
    cd ..
}

# Main execution
main() {
    print_status "Starting build process for 'More Like The Record' Plugin..."
    
    # Process command-line arguments
    BUILD_TYPE="Release"
    if [ "$1" == "debug" ]; then
        BUILD_TYPE="Debug"
        print_status "Building in Debug mode..."
    else
        print_status "Building in Release mode..."
    fi
    
    find_juce
    create_dirs
    configure_cmake $1
    build_plugin
    
    print_success "Build completed successfully!"
    print_success "Plugin files can be found in: build/MoreLikeTheRecord_artefacts/$BUILD_TYPE"
}

# Execute main function with all arguments passed to the script
main "$@"
