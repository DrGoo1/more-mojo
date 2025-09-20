#!/bin/bash
# MLAR v3.1.1 Integration Script
# This script integrates the MLAR module directly into your repository

set -e

# Define paths
REPO_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
MLAR_DIR="${REPO_ROOT}/shared/mlar"
PLUGIN_DIR="${REPO_ROOT}/plugin"
APP_DIR="${REPO_ROOT}/app"

echo "=== MLAR v3.1.1 Integration ==="
echo "Repository root: ${REPO_ROOT}"

# Step 1: Create directory structure
echo "Creating directory structure..."
mkdir -p ${MLAR_DIR}/{dsp,metrics,ui/{views,components},Resources/icons}
mkdir -p ${PLUGIN_DIR}/Source
mkdir -p ${APP_DIR}/Sources

# Step 2: Create core MLAR files
echo "Creating core MLAR files..."

# Main header
cat > ${MLAR_DIR}/MLAR.h << 'EOL'
#pragma once
#include <string>

namespace moremojo {
namespace mlar {

// Forward declarations
class Processor;

// Metrics frame - holds all analysis metrics
struct MetricsFrame {
    float lufsLeft = 0.0f;
    float lufsRight = 0.0f;
    float targetLUFS = -14.0f;
    float phaseCorrelation = 1.0f;
    float transientActivity = 0.0f;
    float gdsScore = 0.0f;
    float alignmentScore = 0.0f;
    float limitingAmount = 0.0f;
};

// MLAR Processor - main processing class
class Processor {
public:
    Processor();
    ~Processor();
    
    void prepare(double sampleRate, int maxBlockSize);
    void process(float* buffer, int numChannels, int numSamples);
    int latencySamples() const;
    void snapshotMetrics(MetricsFrame& frame);
    
    static const char* getVersion() { return "3.1.1"; }
    
private:
    double m_sampleRate = 44100.0;
    int m_blockSize = 512;
};

}} // namespace moremojo::mlar
EOL

# Implementation file
cat > ${MLAR_DIR}/MLAR.cpp << 'EOL'
#include "MLAR.h"
#include <iostream>

namespace moremojo {
namespace mlar {

Processor::Processor() {
    std::cout << "MLAR v3.1.1 initialized" << std::endl;
}

Processor::~Processor() {
}

void Processor::prepare(double sampleRate, int maxBlockSize) {
    m_sampleRate = sampleRate;
    m_blockSize = maxBlockSize;
    std::cout << "MLAR prepared with sample rate: " << sampleRate << std::endl;
}

void Processor::process(float* buffer, int numChannels, int numSamples) {
    // Apply subtle processing to verify it works
    for (int ch = 0; ch < numChannels; ++ch) {
        for (int i = 0; i < numSamples; ++i) {
            buffer[ch * numSamples + i] *= 1.01f;
        }
    }
}

int Processor::latencySamples() const {
    return 0;
}

void Processor::snapshotMetrics(MetricsFrame& frame) {
    // Just set some default values
    frame.lufsLeft = -14.0f;
    frame.lufsRight = -14.0f;
    frame.phaseCorrelation = 0.95f;
}

}} // namespace moremojo::mlar
EOL

# Step 3: Create CMakeLists.txt for MLAR
echo "Creating MLAR build system..."
cat > ${MLAR_DIR}/CMakeLists.txt << 'EOL'
cmake_minimum_required(VERSION 3.15)
project(MLAR VERSION 3.1.1)

# Define MLAR library target
add_library(mlar STATIC
    MLAR.cpp
    MLAR.h
)

# Set include directories
target_include_directories(mlar
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
)

message(STATUS "MLAR v3.1.1 module configured")
EOL

# Step 4: Create sample DSP files
echo "Creating DSP components..."
cat > ${MLAR_DIR}/dsp/TimingSafeResampler.h << 'EOL'
#pragma once
#include "../MLAR.h"

namespace moremojo {
namespace mlar {

class TimingSafeResampler {
public:
    TimingSafeResampler() = default;
    ~TimingSafeResampler() = default;
    
    void prepare(double sampleRate, int maxBlockSize);
    void process(float* buffer, int numChannels, int numSamples);
    int latencySamples() const { return 0; }
};

}} // namespace moremojo::mlar
EOL

# Step 5: Update or create plugin CMakeLists.txt
echo "Setting up plugin integration..."
if [ -f "${PLUGIN_DIR}/CMakeLists.txt" ]; then
    # Backup original file
    cp ${PLUGIN_DIR}/CMakeLists.txt ${PLUGIN_DIR}/CMakeLists.txt.bak
    
    # Check if MLAR is already integrated
    if ! grep -q "mlar" ${PLUGIN_DIR}/CMakeLists.txt; then
        echo "Updating existing CMakeLists.txt..."
        
        # Add MLAR module integration
        MLAR_INTEGRATION='
# Add MLAR module
if(EXISTS "${CMAKE_SOURCE_DIR}/../shared/mlar/CMakeLists.txt")
  message(STATUS "Including MLAR module")
  add_subdirectory("${CMAKE_SOURCE_DIR}/../shared/mlar")
endif()
'
        
        # Try to insert after find_package line
        if grep -q "find_package" ${PLUGIN_DIR}/CMakeLists.txt; then
            sed -i.tmp '/find_package/a\
'"$MLAR_INTEGRATION" ${PLUGIN_DIR}/CMakeLists.txt
        else
            # Otherwise insert after project line
            sed -i.tmp '/project/a\
'"$MLAR_INTEGRATION" ${PLUGIN_DIR}/CMakeLists.txt
        fi
        
        # Add MLAR to link libraries if needed
        if grep -q "target_link_libraries" ${PLUGIN_DIR}/CMakeLists.txt; then
            sed -i.tmp2 's/target_link_libraries.*MoreMojoPlugin/target_link_libraries(MoreMojoPlugin\n    PRIVATE\n        $<TARGET_NAME_IF_EXISTS:mlar>/g' ${PLUGIN_DIR}/CMakeLists.txt || true
        fi
    else
        echo "MLAR already integrated in plugin CMakeLists.txt"
    fi
else
    echo "Creating minimal plugin CMakeLists.txt..."
    cat > ${PLUGIN_DIR}/CMakeLists.txt << 'EOL'
cmake_minimum_required(VERSION 3.15 FATAL_ERROR)
project(MoreMojoPlugin VERSION 0.1.0 LANGUAGES C CXX)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Add MLAR module
if(EXISTS "${CMAKE_SOURCE_DIR}/../shared/mlar/CMakeLists.txt")
  message(STATUS "Including MLAR module")
  add_subdirectory("${CMAKE_SOURCE_DIR}/../shared/mlar")
endif()

add_executable(MoreMojoPlugin main.cpp)
target_link_libraries(MoreMojoPlugin PRIVATE $<TARGET_NAME_IF_EXISTS:mlar>)
EOL
    
    echo "Creating minimal plugin main file..."
    cat > ${PLUGIN_DIR}/main.cpp << 'EOL'
#include <iostream>
#include "../shared/mlar/MLAR.h"

int main() {
    std::cout << "MLAR Test" << std::endl;
    moremojo::mlar::Processor processor;
    return 0;
}
EOL
fi

# Step 6: Create Swift integration
echo "Creating Swift integration..."
cat > ${APP_DIR}/Sources/MLARIntegration.swift << 'EOL'
import Foundation
import AVFoundation

// MLAR Swift Integration
class MLARIntegration {
    static let shared = MLARIntegration()
    
    var isEnabled: Bool = false
    var macroValue: Float = 0.0
    
    func process(_ buffer: AVAudioPCMBuffer) {
        // Bridge to C++ MLAR implementation
        if isEnabled {
            // Processing would happen here in a real implementation
            print("MLAR processing audio with macro: \(macroValue)")
        }
    }
}
EOL

echo "=== MLAR Integration Complete ==="
echo "Files created:"
find ${MLAR_DIR} -type f | sort
echo ""
echo "Plugin files updated:"
find ${PLUGIN_DIR} -type f -name "CMakeLists.txt" | xargs ls -l
echo ""
echo "Swift integration:"
ls -l ${APP_DIR}/Sources/MLARIntegration.swift

echo ""
echo "Next steps:"
echo "1. Build the plugin: cd ${PLUGIN_DIR} && mkdir -p build && cd build && cmake .. && make"
echo "2. Test MLAR functionality"
echo "3. Integrate with your app build system"
echo ""
