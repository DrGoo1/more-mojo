#!/bin/bash

# Simple script to build and run the LED Bar Meter test

echo "Building LED Bar Meter Test..."

# Create build directory
mkdir -p meter_test_build
cd meter_test_build

# Create simple CMakeLists.txt
cat > CMakeLists.txt << 'EOF'
cmake_minimum_required(VERSION 3.22)
project(LedBarMeterTest VERSION 1.0.0)

add_subdirectory(../JUCE JUCE)

juce_add_gui_app(LedBarMeterTest
    PRODUCT_NAME "LED Bar Meter Test"
    VERSION 1.0.0
)

target_sources(LedBarMeterTest PRIVATE
    ../LedBarMeterTest.cpp
)

target_link_libraries(LedBarMeterTest PRIVATE
    juce::juce_gui_basics
    juce::juce_gui_extra
)

target_compile_features(LedBarMeterTest PRIVATE cxx_std_17)
EOF

# Build
cmake . && cmake --build . --config Release

# Run if build succeeded
if [ $? -eq 0 ]; then
    echo "Build successful! Opening LED Bar Meter Test..."
    open "LedBarMeterTest_artefacts/LED Bar Meter Test.app"
else
    echo "Build failed!"
fi
