cmake_minimum_required(VERSION 3.15 FATAL_ERROR)

project(MoreMojoPlugin VERSION 0.1.0 LANGUAGES C CXX)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_OSX_DEPLOYMENT_TARGET "11.0" CACHE STRING "macOS deployment target" FORCE)

# --- JUCE discovery (robust) ---
# If JUCE was cloned by CI and JUCE_DIR is set, use add_subdirectory.
# Otherwise, try find_package with PREFIX path support.
if(DEFINED ENV{JUCE_DIR})
  set(JUCE_DIR "$ENV{JUCE_DIR}")
endif()

if(JUCE_DIR AND EXISTS "${JUCE_DIR}/CMakeLists.txt")
  message(STATUS "Using JUCE from add_subdirectory: ${JUCE_DIR}")
  add_subdirectory("${JUCE_DIR}" JUCE)
else()
  message(STATUS "Using find_package(JUCE)")
  # CMake will search CMAKE_PREFIX_PATH for JUCEConfig.cmake
  find_package(JUCE CONFIG REQUIRED)
endif()

juce_add_plugin(MoreMojoPlugin
    COMPANY_NAME "Umbo Gumbo"
    FORMATS AU VST3 Standalone
    PRODUCT_NAME "More Mojo by Umbo Gumbo"
    BUNDLE_ID "com.UmboGumbo.MoreMojoPlugin"
    COPY_PLUGIN_AFTER_BUILD TRUE

# Explicitly include source files to ensure createPluginFilter() is found
target_sources(MoreMojoPlugin PRIVATE
    Source/PluginProcessor.cpp
    Source/PluginProcessor.h
)

# Add necessary modules for plugin
target_compile_definitions(MoreMojoPlugin
    PUBLIC
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
    JUCE_VST3_CAN_REPLACE_VST2=0)

target_link_libraries(MoreMojoPlugin
    PRIVATE
        juce::juce_audio_utils
        juce::juce_audio_devices
        juce::juce_dsp
        juce::juce_audio_plugin_client
    PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags)
