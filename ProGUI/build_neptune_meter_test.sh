#!/bin/bash

# Simple script to build and run the Neptune VU Meter test

echo "Building Neptune VU Meter Test..."

# Create build directory
mkdir -p neptune_meter_test_build
cd neptune_meter_test_build

# Create simple CMakeLists.txt
cat > CMakeLists.txt << 'EOF'
cmake_minimum_required(VERSION 3.22)
project(NeptuneMeterTest VERSION 1.0.0)

add_subdirectory(../JUCE JUCE)

juce_add_gui_app(NeptuneMeterTest
    PRODUCT_NAME "Neptune VU Meter Test"
    VERSION 1.0.0
)

target_sources(NeptuneMeterTest PRIVATE
    ../NeptuneMeterTest.cpp
)

target_link_libraries(NeptuneMeterTest PRIVATE
    juce::juce_gui_basics
    juce::juce_gui_extra
)

target_compile_features(NeptuneMeterTest PRIVATE cxx_std_17)
EOF

# Build
cmake . && cmake --build . --config Release

# Run if build succeeded
if [ $? -eq 0 ]; then
    echo "Build successful! Opening Neptune VU Meter Test..."
    open "NeptuneMeterTest_artefacts/Neptune VU Meter Test.app"
else
    echo "Build failed!"
fi
