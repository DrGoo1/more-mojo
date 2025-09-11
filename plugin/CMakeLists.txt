cmake_minimum_required(VERSION 3.15 FATAL_ERROR)

# ---------------------------------------------------------
# Project
# ---------------------------------------------------------
project(MoreMojoPlugin VERSION 0.1.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_OSX_DEPLOYMENT_TARGET "11.0" CACHE STRING "macOS deployment target" FORCE)

# ---------------------------------------------------------
# JUCE
# ---------------------------------------------------------
# Expect JUCE to be provided via -DJUCE_DIR=... or a submodule in your CMake prefix path.
# In CI/local builds we pass -DJUCE_DIR=$ENV{JUCE_DIR}
find_package(JUCE CONFIG REQUIRED)

# ---------------------------------------------------------
# Options (formats can be overridden from the command line)
# Example: -DMOJO_FORMATS="AU;VST3;Standalone"
# ---------------------------------------------------------
set(MOJO_FORMATS "AU;VST3;Standalone" CACHE STRING "Plugin formats to build")

# ---------------------------------------------------------
# Plugin target
# The name here is the *base name*. JUCE creates:
#   <BASE>_VST3, <BASE>_AU, <BASE>_Standalone
# and COPY_PLUGIN_AFTER_BUILD installs to user folders on macOS.
# ---------------------------------------------------------
juce_add_plugin(MoreMojoPlugin
    COMPANY_NAME "Umbo Gumbo"
    FORMATS ${MOJO_FORMATS}
    PRODUCT_NAME "More Mojo by Umbo Gumbo"
    COPY_PLUGIN_AFTER_BUILD TRUE       # <— Let JUCE install to ~/Library automatically
    # If you provide an editor class, set this to TRUE and add PluginEditor files below
    NEEDS_MIDI_INPUT FALSE
    NEEDS_MIDI_OUTPUT FALSE
    IS_MIDI_EFFECT FALSE
    IS_SYNTH FALSE
)

# ---------------------------------------------------------
# Sources
# Keep this minimal; JUCE provides a generic editor if you don't add your own.
# ---------------------------------------------------------
target_sources(MoreMojoPlugin PRIVATE
    Source/PluginProcessor.cpp
    Source/PluginProcessor.h
    # If you have a custom editor, uncomment these lines:
    # Source/PluginEditor.cpp
    # Source/PluginEditor.h
)

# ---------------------------------------------------------
# Compile definitions (tweak to taste)
# ---------------------------------------------------------
target_compile_definitions(MoreMojoPlugin PRIVATE
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
    JUCE_VST3_CAN_REPLACE_VST2=0
)

# ---------------------------------------------------------
# Link JUCE modules
# ---------------------------------------------------------
target_link_libraries(MoreMojoPlugin PRIVATE
    juce::juce_audio_utils
    juce::juce_dsp
)

# ---------------------------------------------------------
# (Optional) Extra: leave a deploy/ mirror for CI artifacts
# If you want a deploy folder as well as JUCE's auto-install,
# uncomment the block below. Note: always target the *per-format*
# bundle names, never the base target.
# ---------------------------------------------------------
# set(PLUGIN_BASE_NAME MoreMojoPlugin)
#
# if (APPLE AND TARGET ${PLUGIN_BASE_NAME}_VST3)
#   add_custom_command(TARGET ${PLUGIN_BASE_NAME}_VST3 POST_BUILD
#     COMMAND ${CMAKE_COMMAND} -E make_directory
#             "${CMAKE_BINARY_DIR}/deploy/VST3"
#     COMMAND ${CMAKE_COMMAND} -E copy_directory
#             "$<TARGET_BUNDLE_DIR:${PLUGIN_BASE_NAME}_VST3>"
#             "${CMAKE_BINARY_DIR}/deploy/VST3/${PLUGIN_BASE_NAME}.vst3"
#     COMMENT "Deploying ${PLUGIN_BASE_NAME}.vst3")
# endif()
#
# if (APPLE AND TARGET ${PLUGIN_BASE_NAME}_AU)
#   add_custom_command(TARGET ${PLUGIN_BASE_NAME}_AU POST_BUILD
#     COMMAND ${CMAKE_COMMAND} -E make_directory
#             "${CMAKE_BINARY_DIR}/deploy/AU"
#     COMMAND ${CMAKE_COMMAND} -E copy_directory
#             "$<TARGET_BUNDLE_DIR:${PLUGIN_BASE_NAME}_AU>"
#             "${CMAKE_BINARY_DIR}/deploy/AU/${PLUGIN_BASE_NAME}.component"
#     COMMENT "Deploying ${PLUGIN_BASE_NAME}.component")
# endif()
#
# if (APPLE AND TARGET ${PLUGIN_BASE_NAME}_Standalone)
#   add_custom_command(TARGET ${PLUGIN_BASE_NAME}_Standalone POST_BUILD
#     COMMAND ${CMAKE_COMMAND} -E make_directory
#             "${CMAKE_BINARY_DIR}/deploy/Standalone"
#     COMMAND ${CMAKE_COMMAND} -E copy_directory
#             "$<TARGET_BUNDLE_DIR:${PLUGIN_BASE_NAME}_Standalone>"
#             "${CMAKE_BINARY_DIR}/deploy/Standalone/${PLUGIN_BASE_NAME}.app"
#     COMMENT "Deploying ${PLUGIN_BASE_NAME}.app")
# endif()
