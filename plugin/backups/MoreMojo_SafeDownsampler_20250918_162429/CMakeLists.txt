cmake_minimum_required(VERSION 3.15 FATAL_ERROR)

project(MoreMojoPlugin VERSION 0.1.0 LANGUAGES C CXX)

# SafeDownsampler option
option(MOREMOJO_USE_SAFE_DOWNSAMPLER "Use crash-safe FIR downsampler" ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_OSX_DEPLOYMENT_TARGET "11.0" CACHE STRING "macOS deployment target" FORCE)

# Add MLAR module first
if(EXISTS "${CMAKE_SOURCE_DIR}/../shared/mlar")
  message(STATUS "Including MLAR module")
  add_subdirectory("${CMAKE_SOURCE_DIR}/../shared/mlar" mlar_build)
endif()

# Set explicit path to JUCE
set(JUCE_DIR "${CMAKE_SOURCE_DIR}/../JUCE")
message(STATUS "Setting JUCE_DIR to: ${JUCE_DIR}")

# Add JUCE as a subdirectory
if(EXISTS "${JUCE_DIR}/CMakeLists.txt")
  message(STATUS "Using JUCE from add_subdirectory: ${JUCE_DIR}")
  add_subdirectory("${JUCE_DIR}" JUCE)
else()
  message(FATAL_ERROR "JUCE not found at ${JUCE_DIR}. Please ensure JUCE is available.")
endif()

juce_add_plugin(MoreMojoPlugin
    COMPANY_NAME "Umbo Gumbo"
    FORMATS AU VST3 Standalone
    PRODUCT_NAME "More Mojo by Umbo Gumbo"
    BUNDLE_ID "com.UmboGumbo.MoreMojoPlugin"
    COPY_PLUGIN_AFTER_BUILD TRUE
    PLUGIN_MANUFACTURER_CODE Umbo
    PLUGIN_CODE MjPl
    JUCE_GENERATED_INCLUDE_FILE_NAME "JuceHeader.h")

# Explicitly include source files to ensure createPluginFilter() is found
target_sources(MoreMojoPlugin PRIVATE
    Source/PluginProcessor.cpp
    Source/PluginProcessor.h
    Source/JuceInit_fix.h
)

# Add necessary modules for plugin
target_compile_definitions(MoreMojoPlugin
    PUBLIC
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
    JUCE_VST3_CAN_REPLACE_VST2=0
    $<$<BOOL:${MOREMOJO_USE_SAFE_DOWNSAMPLER}>:MLAR_USE_SAFE_DOWNSAMPLER=1>)

# Standard standalone entry point (no custom entry point)

target_link_libraries(MoreMojoPlugin
    PRIVATE
        mlar
        juce::juce_audio_utils
        juce::juce_audio_devices
        juce::juce_dsp
        juce::juce_audio_plugin_client
    PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags)
