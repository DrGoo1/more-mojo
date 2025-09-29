#!/bin/bash

# Simple script to build and run the Audio Graphics test

echo "Building Audio Graphics Test..."

# Create build directory
mkdir -p graphics_test_build
cd graphics_test_build

# Create simple CMakeLists.txt
cat > CMakeLists.txt << 'EOF'
cmake_minimum_required(VERSION 3.22)
project(AudioGraphicsTest VERSION 1.0.0)

add_subdirectory(../JUCE JUCE)

juce_add_gui_app(AudioGraphicsTest
    PRODUCT_NAME "Audio Graphics Test"
    VERSION 1.0.0
)

target_sources(AudioGraphicsTest PRIVATE
    ../AudioGraphicsTest.cpp
)

target_link_libraries(AudioGraphicsTest PRIVATE
    juce::juce_gui_basics
    juce::juce_gui_extra
)

target_compile_features(AudioGraphicsTest PRIVATE cxx_std_17)
EOF

# Build
cmake . && cmake --build . --config Release

# Run if build succeeded
if [ $? -eq 0 ]; then
    echo "Build successful! Opening Audio Graphics Test..."
    open "AudioGraphicsTest_artefacts/Audio Graphics Test.app"
else
    echo "Build failed!"
fi
