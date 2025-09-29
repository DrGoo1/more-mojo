#!/bin/bash

# Simple script to build and run the Neptune test

echo "Building Neptune Test..."

# Create build directory
mkdir -p neptune_test_build
cd neptune_test_build

# Create simple CMakeLists.txt
cat > CMakeLists.txt << 'EOF'
cmake_minimum_required(VERSION 3.22)
project(SimpleNeptuneTest VERSION 1.0.0)

add_subdirectory(../JUCE JUCE)

juce_add_gui_app(SimpleNeptuneTest
    PRODUCT_NAME "Simple Neptune Test"
    VERSION 1.0.0
)

target_sources(SimpleNeptuneTest PRIVATE
    ../SimpleNeptuneTest.cpp
)

target_link_libraries(SimpleNeptuneTest PRIVATE
    juce::juce_gui_basics
    juce::juce_gui_extra
)

target_compile_features(SimpleNeptuneTest PRIVATE cxx_std_17)
EOF

# Build
cmake . && cmake --build . --config Release

# Run if build succeeded
if [ $? -eq 0 ]; then
    echo "Build successful! Opening Neptune Test..."
    open "SimpleNeptuneTest_artefacts/Simple Neptune Test.app"
else
    echo "Build failed!"
fi
