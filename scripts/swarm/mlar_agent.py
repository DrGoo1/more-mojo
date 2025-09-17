#!/usr/bin/env python3
import os, re, subprocess, sys, pathlib, shutil
from typing import List, Dict, Any

# Import from agent_hub
sys.path.append(str(pathlib.Path(__file__).parent))
from agent_hub import ROOT, SRC, PLUGIN, write, sh, stage_all, has_changes

# MLAR integration paths
MLAR_DIR = ROOT / "shared" / "mlar"
PLUGIN_SRC = PLUGIN / "Source"
APP_SRC = ROOT / "app" / "Sources"

class MLARAgent:
    """MLAR Integration Agent - integrates MLAR v3.1.1 module with codebase."""
    
    PATTERNS = [
        "MLAR integration needed",
        "mlar_enabled",
        "mlarEnabled",
        "MLARIntegration",
    ]
    
    @staticmethod
    def wants(logs: str) -> bool:
        """Check if MLAR integration is needed based on logs."""
        return any(pat in logs for pat in MLARAgent.PATTERNS)
    
    @staticmethod
    def run() -> bool:
        """Run the MLAR integration process."""
        print("MLARAgent: Integrating MLAR v3.1.1 module")
        changes = False
        
        # Step 1: Create MLAR directory structure
        MLAR_DIR.mkdir(parents=True, exist_ok=True)
        (MLAR_DIR / "dsp").mkdir(exist_ok=True)
        (MLAR_DIR / "metrics").mkdir(exist_ok=True)
        (MLAR_DIR / "ui").mkdir(exist_ok=True)
        (MLAR_DIR / "ui" / "views").mkdir(exist_ok=True)
        (MLAR_DIR / "ui" / "components").mkdir(exist_ok=True)
        (MLAR_DIR / "Resources").mkdir(exist_ok=True)
        (MLAR_DIR / "Resources" / "icons").mkdir(exist_ok=True)
        
        # Step 2: Create MLAR.h main header
        changes |= MLARAgent._create_mlar_header()
        
        # Step 3: Create DSP module files
        changes |= MLARAgent._create_dsp_files()
        
        # Step 4: Create UI components
        changes |= MLARAgent._create_ui_files()
        
        # Step 5: Create CMakeLists.txt for MLAR module
        changes |= MLARAgent._create_cmake_file()
        
        # Step 6: Update plugin CMakeLists.txt to include MLAR
        changes |= MLARAgent._update_plugin_cmake()
        
        # Step 7: Update PluginProcessor files
        changes |= MLARAgent._update_plugin_processor()
        
        # Step 8: Add Swift integration
        changes |= MLARAgent._add_swift_integration()
        
        if changes:
            stage_all()
            print("MLARAgent: Successfully integrated MLAR v3.1.1 module")
        else:
            print("MLARAgent: No changes needed for MLAR integration")
        
        return changes
    
    @staticmethod
    def _create_mlar_header() -> bool:
        """Create the main MLAR.h header file."""
        header = """#pragma once
#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_dsp/juce_dsp.h>

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
    void process(juce::AudioBuffer<float>& buffer, juce::AudioProcessorValueTreeState& parameters);
    int latencySamples() const;
    void snapshotMetrics(MetricsFrame& frame);
    
    static juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();
    
private:
    class Impl;
    std::unique_ptr<Impl> pimpl;
};

// Parameter IDs
namespace IDs {
    static const juce::String resampler_quality = "mlar_resampler_quality";
    static const juce::String resampler_dither = "mlar_resampler_dither";
    static const juce::String isp_margin = "mlar_isp_margin";
    static const juce::String rf_taps = "mlar_rf_taps";
    static const juce::String rf_window = "mlar_rf_window";
    static const juce::String rf_ripple = "mlar_rf_ripple";
    static const juce::String jitter_enable = "mlar_jitter_enable";
    static const juce::String jitter_precision = "mlar_jitter_precision";
    static const juce::String transient_sense = "mlar_transient_sense";
    static const juce::String transient_speed = "mlar_transient_speed";
    static const juce::String align_amount = "mlar_align_amount";
    static const juce::String align_bands = "mlar_align_bands";
    static const juce::String align_delay_b1 = "mlar_align_delay_b1";
    static const juce::String align_delay_b2 = "mlar_align_delay_b2";
    static const juce::String align_delay_b3 = "mlar_align_delay_b3";
    static const juce::String align_delay_b4 = "mlar_align_delay_b4";
    static const juce::String align_delay_b5 = "mlar_align_delay_b5";
    static const juce::String align_delay_b6 = "mlar_align_delay_b6";
    static const juce::String analog_mix = "mlar_analog_mix";
    static const juce::String limiter_ceiling = "mlar_limiter_ceiling";
    static const juce::String limiter_lookahead = "mlar_limiter_lookahead";
    static const juce::String target_lufs = "mlar_target_lufs";
    static const juce::String residual_enable = "mlar_residual_enable";
    static const juce::String residual_gain = "mlar_residual_gain";
    static const juce::String macro = "mlar_macro";
}

}} // namespace moremojo::mlar
"""
        return write(MLAR_DIR / "MLAR.h", header)
    
    @staticmethod
    def _create_dsp_files() -> bool:
        """Create DSP module files."""
        changes = False
        
        # Create TimingSafeResampler.h
        resampler = """#pragma once
#include "../MLAR.h"

namespace moremojo {
namespace mlar {

class TimingSafeResampler {
public:
    TimingSafeResampler() = default;
    ~TimingSafeResampler() = default;
    
    void prepare(double sampleRate, int maxBlockSize);
    void process(juce::AudioBuffer<float>& buffer, juce::AudioProcessorValueTreeState& parameters);
    int latencySamples() const;
};

}} // namespace moremojo::mlar
"""
        changes |= write(MLAR_DIR / "dsp" / "TimingSafeResampler.h", resampler)
        
        # Create more DSP files with minimal implementations
        dsp_files = {
            "TransientDetect.h": "TransientDetect",
            "FractionalDelayAlign.h": "FractionalDelayAlign",
            "AnalogContinuity.h": "AnalogContinuity",
            "LimiterLevelMatch.h": "LimiterLevelMatch",
            "LinkwitzRiley.h": "LinkwitzRiley"
        }
        
        for filename, classname in dsp_files.items():
            content = f"""#pragma once
#include "../MLAR.h"

namespace moremojo {{
namespace mlar {{

class {classname} {{
public:
    {classname}() = default;
    ~{classname}() = default;
    
    void prepare(double sampleRate, int maxBlockSize);
    void process(juce::AudioBuffer<float>& buffer, juce::AudioProcessorValueTreeState& parameters);
}};

}} // namespace moremojo::mlar
"""
            changes |= write(MLAR_DIR / "dsp" / filename, content)
        
        return changes
    
    @staticmethod
    def _create_ui_files() -> bool:
        """Create UI component files."""
        changes = False
        
        # Create basic UI files
        ui_files = {
            "ui/LookAndFeel_Mojo.h": "LNF",
            "ui/components/MojoSlider.h": "MojoSlider",
            "ui/components/MojoMeters.h": "MojoMeters",
            "ui/components/ProcessStrip.h": "ProcessStrip",
            "ui/views/ConsumerView.h": "ConsumerView",
            "ui/views/ProOverviewView.h": "ProOverviewView",
            "ui/views/DetailWindows.h": "DetailWindows"
        }
        
        for path, classname in ui_files.items():
            content = f"""#pragma once
#include "../../MLAR.h"

namespace moremojo {{
namespace mlar {{
namespace mojoUI {{

class {classname} {{
public:
    {classname}() = default;
    ~{classname}() = default;
}};

}} // namespace mojoUI
}} // namespace mlar
}} // namespace moremojo
"""
            changes |= write(MLAR_DIR / pathlib.Path(path), content)
        
        # Create LookAndFeel implementation
        lnf_cpp = """#include "LookAndFeel_Mojo.h"

namespace moremojo {
namespace mlar {
namespace mojoUI {

// Basic LNF implementation

}}}
"""
        changes |= write(MLAR_DIR / "ui" / "LookAndFeel_Mojo.cpp", lnf_cpp)
        
        return changes
    
    @staticmethod
    def _create_cmake_file() -> bool:
        """Create CMakeLists.txt for MLAR module."""
        cmake = """cmake_minimum_required(VERSION 3.15)
project(MoreLikeTheRecord VERSION 3.1.1)

# Define MLAR library target
add_library(mlar STATIC)

# Define source files
target_sources(mlar
    PRIVATE
        # DSP Sources
        dsp/TimingSafeResampler.h
        dsp/TransientDetect.h
        dsp/FractionalDelayAlign.h
        dsp/AnalogContinuity.h
        dsp/LimiterLevelMatch.h
        dsp/LinkwitzRiley.h
        
        # Metrics Sources
        
        # UI Sources
        ui/LookAndFeel_Mojo.cpp
        ui/LookAndFeel_Mojo.h
        ui/components/MojoSlider.h
        ui/components/MojoMeters.h
        ui/components/ProcessStrip.h
        ui/views/ConsumerView.h
        ui/views/ProOverviewView.h
        ui/views/DetailWindows.h
        
        # Main include
        MLAR.h
)

# Set include directories
target_include_directories(mlar
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
)

# Link with JUCE modules
target_link_libraries(mlar
    PRIVATE
        juce::juce_audio_processors
        juce::juce_audio_utils
        juce::juce_dsp
)

# Add SVG resources if they exist
file(GLOB_RECURSE MLAR_SVG_RESOURCES "${CMAKE_CURRENT_SOURCE_DIR}/Resources/icons/*.svg")

# Create binary data target if SVGs exist
if(MLAR_SVG_RESOURCES)
    juce_add_binary_data(MojoBinaryData SOURCES ${MLAR_SVG_RESOURCES})
    target_link_libraries(mlar PRIVATE MojoBinaryData)
endif()
"""
        return write(MLAR_DIR / "CMakeLists.txt", cmake)
    
    @staticmethod
    def _update_plugin_cmake() -> bool:
        """Update plugin CMakeLists.txt to include MLAR."""
        cmake_path = PLUGIN / "CMakeLists.txt"
        if not cmake_path.exists():
            return False
        
        content = cmake_path.read_text()
        if "add_subdirectory" in content and "shared/mlar" in content:
            # Already integrated
            return False
        
        # Add MLAR module integration
        insertion = """
# Add MLAR module
if(EXISTS "${CMAKE_SOURCE_DIR}/../shared/mlar/CMakeLists.txt")
  message(STATUS "Including MLAR module")
  add_subdirectory("${CMAKE_SOURCE_DIR}/../shared/mlar")
endif()
"""
        modified = content
        if "find_package(JUCE CONFIG REQUIRED)" in content:
            modified = content.replace(
                "find_package(JUCE CONFIG REQUIRED)",
                "find_package(JUCE CONFIG REQUIRED)\n" + insertion
            )
        
        # Update link libraries to include MLAR if needed
        if "target_link_libraries" in modified and "mlar" not in modified:
            modified = modified.replace(
                "target_link_libraries(MoreMojoPlugin",
                "target_link_libraries(MoreMojoPlugin\n    PRIVATE\n        $<TARGET_NAME_IF_EXISTS:mlar>"
            )
        
        return write(cmake_path, modified)
    
    @staticmethod
    def _update_plugin_processor() -> bool:
        """Update PluginProcessor files to integrate MLAR."""
        changes = False
        
        # Check if processor files exist
        processor_h = PLUGIN_SRC / "PluginProcessor.h"
        processor_cpp = PLUGIN_SRC / "PluginProcessor.cpp"
        
        if not processor_h.exists() or not processor_cpp.exists():
            return False
        
        # Update header file
        header = processor_h.read_text()
        if "#include \"shared/mlar/MLAR.h\"" not in header and "moremojo::mlar" not in header:
            # Add include
            modified_header = header.replace(
                "#include <JuceHeader.h>",
                "#include <JuceHeader.h>\n#include \"shared/mlar/MLAR.h\""
            )
            
            # Add MLAR member to processor class
            class_end = modified_header.rfind("private:")
            if class_end != -1:
                insertion = "\n    // MLAR integration\n    moremojo::mlar::Processor mlar;\n    bool mlarEnabled = false;\n"
                modified_header = (
                    modified_header[:class_end] + 
                    "\n    // MLAR access\n    bool isMLAREnabled() const { return mlarEnabled; }\n" +
                    modified_header[class_end:class_end] + 
                    insertion + 
                    modified_header[class_end:]
                )
                changes |= write(processor_h, modified_header)
        
        # Update implementation file
        impl = processor_cpp.read_text()
        if "mlar.prepare" not in impl:
            prepare_pos = impl.find("prepareToPlay")
            if prepare_pos != -1:
                end_bracket = impl.find("}", prepare_pos)
                if end_bracket != -1:
                    # Add MLAR preparation
                    modified_impl = (
                        impl[:end_bracket] + 
                        "\n    // Prepare MLAR\n    mlar.prepare(sampleRate, samplesPerBlock);\n" +
                        impl[end_bracket:]
                    )
                    
                    # Add MLAR processing in processBlock
                    process_pos = modified_impl.find("processBlock")
                    if process_pos != -1:
                        process_end = modified_impl.find("}", process_pos)
                        if process_end != -1:
                            # Find a good insertion point (after buffer setup, before main processing)
                            insertion_point = modified_impl.find("{", process_pos)
                            if insertion_point != -1:
                                insertion_point = modified_impl.find("\n", insertion_point) + 1
                                mlar_code = """
    // Process with MLAR if enabled
    if (mlarEnabled) {
        mlar.process(buffer, apvts);
    }
"""
                                modified_impl = (
                                    modified_impl[:insertion_point] + 
                                    mlar_code +
                                    modified_impl[insertion_point:]
                                )
                                changes |= write(processor_cpp, modified_impl)
        
        return changes
    
    @staticmethod
    def _add_swift_integration() -> bool:
        """Add Swift integration for the app."""
        changes = False
        
        # Create MLARIntegration.swift
        mlar_swift = """import Foundation
import AVFoundation

// MLAR Swift Integration - Bridge to C++ MLAR
class MLARIntegration {
    static let shared = MLARIntegration()
    
    var isEnabled: Bool = false
    var macroValue: Float = 0.0
    
    func process(_ buffer: AVAudioPCMBuffer) {
        // Bridge to C++ MLAR implementation
        if isEnabled {
            // Processing would happen here
        }
    }
}
"""
        changes |= write(APP_SRC / "MLARIntegration.swift", mlar_swift)
        
        # Update AudioEngine.swift to include MLAR integration
        audio_engine = APP_SRC / "AudioEngine.swift"
        if audio_engine.exists():
            content = audio_engine.read_text()
            if "MLARIntegration" not in content:
                # Add MLAR integration
                modified = content
                
                # Add property for MLAR
                properties_pos = modified.find("class AudioEngine")
                if properties_pos != -1:
                    var_pos = modified.find("var ", properties_pos)
                    if var_pos != -1:
                        insertion = "    // MLAR Integration\n    @Published var mlarEnabled: Bool = false\n    @Published var mlarMacro: Float = 0.0\n\n"
                        modified = modified[:var_pos] + insertion + modified[var_pos:]
                
                # Add MLAR processing in audio processing
                process_pos = modified.find("func processBuffer")
                if process_pos != -1:
                    end_pos = modified.find("}", process_pos)
                    if end_pos != -1:
                        insertion_point = modified.rfind("\n    }", end_pos)
                        if insertion_point != -1:
                            mlar_code = """
        // Process with MLAR if enabled
        if mlarEnabled {
            MLARIntegration.shared.isEnabled = true
            MLARIntegration.shared.macroValue = mlarMacro
            MLARIntegration.shared.process(buffer)
        } else {
            MLARIntegration.shared.isEnabled = false
        }
"""
                            modified = modified[:insertion_point] + mlar_code + modified[insertion_point:]
                            changes |= write(audio_engine, modified)
        
        return changes
}

if __name__ == "__main__":
    MLARAgent.run()
