#!/usr/bin/env python3
"""CMake agent to fix CMake configuration issues."""

import shutil
from pathlib import Path

from .base_agent import BaseAgent
from .registry import register_agent

@register_agent
class CMakeAgent(BaseAgent):
    """Fix CMake configuration issues."""
    
    @classmethod
    def name(cls) -> str:
        return "CMakeAgent"
    
    @classmethod
    def description(cls) -> str:
        return "Fixes CMake configuration issues for plugin builds"
    
    def wants(self, app_logs: str, plugin_logs: str) -> bool:
        """Determine if the agent wants to run based on log content."""
        return ("TARGET_BUNDLE_DIR is allowed only for Bundle targets" in plugin_logs or 
                "$<TARGET_BUNDLE_DIR:MoreMojoPlugin>" in plugin_logs)
    
    def run(self) -> bool:
        """Fix CMake configuration issues."""
        cmk = self.plugin / "CMakeLists.txt"
        cmake_text = """cmake_minimum_required(VERSION 3.15 FATAL_ERROR)
project(MoreMojoPlugin VERSION 0.1.0 LANGUAGES C CXX)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_OSX_DEPLOYMENT_TARGET "11.0" CACHE STRING "macOS deployment target" FORCE)
find_package(JUCE CONFIG REQUIRED)
set(MOJO_FORMATS "AU;VST3;Standalone" CACHE STRING "Plugin formats to build")
juce_add_plugin(MoreMojoPlugin
    COMPANY_NAME "Umbo Gumbo"
    FORMATS ${MOJO_FORMATS}
    PRODUCT_NAME "More Mojo by Umbo Gumbo"
    COPY_PLUGIN_AFTER_BUILD TRUE
    NEEDS_MIDI_INPUT FALSE
    NEEDS_MIDI_OUTPUT FALSE
    IS_MIDI_EFFECT FALSE
    IS_SYNTH FALSE)
target_sources(MoreMojoPlugin PRIVATE
    Source/PluginProcessor.cpp
    Source/PluginProcessor.h)
target_compile_definitions(MoreMojoPlugin PRIVATE
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
    JUCE_VST3_CAN_REPLACE_VST2=0)
target_link_libraries(MoreMojoPlugin PRIVATE
    juce::juce_audio_utils
    juce::juce_dsp)
"""
        changed = self.write_file(cmk, cmake_text)
        
        if changed:
            # Clean up build directory to force regeneration
            build_dir = self.plugin / "build"
            if build_dir.exists():
                self.logger.info(f"Removing {build_dir} to force CMake regeneration")
                shutil.rmtree(build_dir, ignore_errors=True)
            
            self.logger.info("Updated CMakeLists.txt")
        
        return changed
