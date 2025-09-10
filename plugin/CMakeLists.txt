cmake_minimum_required(VERSION 3.15)

# Default to Phase 2 if not specified
if(NOT DEFINED PHASE)
    set(PHASE 2)
endif()

# Project details
project(MoreMojoPlugin VERSION 0.2.0)

# Find the JUCE library
if(NOT DEFINED JUCE_DIR)
    message(FATAL_ERROR "JUCE_DIR must be defined! Please provide the path to your JUCE installation.")
endif()

add_subdirectory(${JUCE_DIR} juce)

# Set up basic JUCE configuration
juce_add_plugin(MoreMojoPlugin
    COMPANY_NAME "MoreMojo"
    PLUGIN_MANUFACTURER_CODE Mojo
    PLUGIN_CODE MMP2
    FORMATS AU VST3 
    PRODUCT_NAME "MoreMojo"
    COMPANY_WEBSITE "https://more-mojo.com"
    COMPANY_EMAIL "info@more-mojo.com"
    IS_SYNTH FALSE
    NEEDS_MIDI_INPUT FALSE
    NEEDS_MIDI_OUTPUT FALSE
    IS_MIDI_EFFECT FALSE
    EDITOR_WANTS_KEYBOARD_FOCUS FALSE
    COPY_PLUGIN_AFTER_BUILD TRUE
    VST3_CATEGORIES "Fx" "Distortion" "Dynamics"
    AU_MAIN_TYPE "kAudioUnitType_Effect"
    PLUGIN_NAME "MoreMojo"
    PLUGIN_DESCRIPTION "MoreMojo analog shaper with multiple interpolation modes"
    PLUGIN_AU_EXPORT_PREFIX "MoreMojoAU"
)

# Tell CMake about the Phase
add_compile_definitions(PHASE=${PHASE})

# Source files
target_sources(MoreMojoPlugin
    PRIVATE
        Source/PluginProcessor.cpp
)

# Include directories
target_include_directories(MoreMojoPlugin
    PRIVATE
        Source
)

# Set C++ standard
target_compile_features(MoreMojoPlugin PRIVATE cxx_std_17)

# Link against JUCE modules
target_link_libraries(MoreMojoPlugin
    PRIVATE
        juce::juce_audio_basics
        juce::juce_audio_devices
        juce::juce_audio_formats
        juce::juce_audio_plugin_client
        juce::juce_audio_processors
        juce::juce_audio_utils
        juce::juce_core
        juce::juce_data_structures
        juce::juce_dsp
        juce::juce_events
        juce::juce_graphics
        juce::juce_gui_basics
        juce::juce_gui_extra
    PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags
)

# Add optimization flags
if(CMAKE_BUILD_TYPE MATCHES Release)
    target_compile_options(MoreMojoPlugin PRIVATE -O3)
endif()

# Add version information
set_target_properties(MoreMojoPlugin PROPERTIES
    VERSION ${PROJECT_VERSION}
    SOVERSION ${PROJECT_VERSION_MAJOR}
)

# Plugin target installations are handled by JUCE and our custom deploy commands
# We don't need a direct install command as it would use the wrong target

# --- Begin JUCE post-build deploy (MoreMojobyUmboGumbo) ---

# This must match the 1st arg you used in juce_add_plugin(...)
set(PLUGIN_BASE_NAME MoreMojoPlugin)

# Copy VST3 bundle after build
if (APPLE AND TARGET ${PLUGIN_BASE_NAME}_VST3)
  add_custom_command(TARGET ${PLUGIN_BASE_NAME}_VST3 POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory
            "${CMAKE_BINARY_DIR}/deploy/VST3"
    COMMAND ${CMAKE_COMMAND} -E copy_directory
            "$<TARGET_BUNDLE_DIR:${PLUGIN_BASE_NAME}_VST3>"
            "${CMAKE_BINARY_DIR}/deploy/VST3/${PLUGIN_BASE_NAME}.vst3"
    COMMENT "Deploying ${PLUGIN_BASE_NAME}.vst3"
    VERBATIM)
endif()

# Copy AU bundle after build
if (APPLE AND TARGET ${PLUGIN_BASE_NAME}_AU)
  add_custom_command(TARGET ${PLUGIN_BASE_NAME}_AU POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory
            "${CMAKE_BINARY_DIR}/deploy/AU"
    COMMAND ${CMAKE_COMMAND} -E copy_directory
            "$<TARGET_BUNDLE_DIR:${PLUGIN_BASE_NAME}_AU>"
            "${CMAKE_BINARY_DIR}/deploy/AU/${PLUGIN_BASE_NAME}.component"
    COMMENT "Deploying ${PLUGIN_BASE_NAME}.component"
    VERBATIM)
endif()

# Copy Standalone app after build (optional)
if (APPLE AND TARGET ${PLUGIN_BASE_NAME}_Standalone)
  add_custom_command(TARGET ${PLUGIN_BASE_NAME}_Standalone POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory
            "${CMAKE_BINARY_DIR}/deploy/Standalone"
    COMMAND ${CMAKE_COMMAND} -E copy_directory
            "$<TARGET_BUNDLE_DIR:${PLUGIN_BASE_NAME}_Standalone>"
            "${CMAKE_BINARY_DIR}/deploy/Standalone/${PLUGIN_BASE_NAME}.app"
    COMMENT "Deploying ${PLUGIN_BASE_NAME}.app"
    VERBATIM)
endif()

# --- End JUCE post-build deploy ---
