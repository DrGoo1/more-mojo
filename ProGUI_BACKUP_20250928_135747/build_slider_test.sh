#!/bin/bash

# Simple script to build and run the Neptune Slider test

echo "Building Neptune Slider Test..."

# Create build directory
mkdir -p slider_test_build
cd slider_test_build

# Create simple CMakeLists.txt
cat > CMakeLists.txt << 'EOF'
cmake_minimum_required(VERSION 3.22)
project(NeptuneSliderTest VERSION 1.0.0)

add_subdirectory(../JUCE JUCE)

juce_add_gui_app(NeptuneSliderTest
    PRODUCT_NAME "Neptune Slider Test"
    VERSION 1.0.0
)

target_sources(NeptuneSliderTest PRIVATE
    ../NeptuneSliderTest.cpp
)

target_link_libraries(NeptuneSliderTest PRIVATE
    juce::juce_gui_basics
    juce::juce_gui_extra
)

target_compile_features(NeptuneSliderTest PRIVATE cxx_std_17)
EOF

# Build
cmake . && cmake --build . --config Release

# Run if build succeeded
if [ $? -eq 0 ]; then
    echo "Build successful! Opening Neptune Slider Test..."
    open "NeptuneSliderTest_artefacts/Neptune Slider Test.app"
else
    echo "Build failed!"
fi
