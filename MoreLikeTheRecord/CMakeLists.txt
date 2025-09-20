cmake_minimum_required(VERSION 3.15)
project(MoreLikeTheRecord VERSION 3.0.0)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find or clone JUCE as a dependency
if(NOT TARGET juce::juce_audio_utils)
    add_subdirectory(JUCE)
endif()

# Define plugin formats to build
set(FORMATS AU VST3)

# Add SVG resources directory
file(GLOB_RECURSE SVG_RESOURCES "${CMAKE_CURRENT_SOURCE_DIR}/Resources/icons/*.svg")

# Create binary resources
juce_add_binary_data(MoreLikeTheRecordResources SOURCES ${SVG_RESOURCES})

# Add JUCE plugin target
juce_add_plugin(MoreLikeTheRecord
    VERSION 3.0.0
    COMPANY_NAME "MoreMojo Audio"
    COMPANY_COPYRIGHT "Copyright (c) 2025 MoreMojo Audio"
    COMPANY_WEBSITE "https://moremojo.audio"
    PLUGIN_MANUFACTURER_CODE Mojo
    PLUGIN_CODE MLTR
    FORMATS ${FORMATS}
    VST3_CATEGORIES "Fx" "Dynamics" "Mastering"
    AU_MAIN_TYPE "kAudioUnitType_Effect"
    PRODUCT_NAME "More Like The Record"
)

# Define source files
target_sources(MoreLikeTheRecord
    PRIVATE
        Source/PluginProcessor.cpp
        Source/PluginEditor.cpp
)

# Set include directories
target_include_directories(MoreLikeTheRecord
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/Source
)

# Link JUCE modules
target_link_libraries(MoreLikeTheRecord
    PRIVATE
        MoreLikeTheRecordResources
        juce::juce_audio_processors
        juce::juce_audio_utils
        juce::juce_dsp
        juce::juce_opengl
    PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags
)

# Set additional compiler options
target_compile_definitions(MoreLikeTheRecord
    PUBLIC
        JUCE_WEB_BROWSER=0
        JUCE_USE_CURL=0
        JUCE_VST3_CAN_REPLACE_VST2=0
        JUCE_DISPLAY_SPLASH_SCREEN=0
)

# Add install step
install(TARGETS MoreLikeTheRecord
    LIBRARY DESTINATION ${CMAKE_INSTALL_PREFIX}/lib/vst3
    FRAMEWORK DESTINATION ${CMAKE_INSTALL_PREFIX}/Library/Audio/Plug-Ins/Components
)
