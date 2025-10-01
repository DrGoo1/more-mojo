# If the reusable UI lib isn't present (standalone runner build), build it here
if (NOT TARGET progui_ui)
    add_library(progui_ui STATIC
        ${CMAKE_CURRENT_LIST_DIR}/../../Source/UIDemoComponent.cpp
        ${CMAKE_CURRENT_LIST_DIR}/../../Source/UIDemoComponent.h
        ${CMAKE_CURRENT_LIST_DIR}/../../Source/Components/NeptuneVUMeter.cpp
        ${CMAKE_CURRENT_LIST_DIR}/../../Source/Components/NeptuneVUMeter.h
        ${CMAKE_CURRENT_LIST_DIR}/../../Source/Components/NeptuneMixKnob.cpp
        ${CMAKE_CURRENT_LIST_DIR}/../../Source/Components/NeptuneMixKnob.h
        ${CMAKE_CURRENT_LIST_DIR}/../../Source/Components/NeptuneKnob.cpp
        ${CMAKE_CURRENT_LIST_DIR}/../../Source/Components/NeptuneKnob.h
        ${CMAKE_CURRENT_LIST_DIR}/../../Source/Components/NeptuneISPSubwindow.cpp
        ${CMAKE_CURRENT_LIST_DIR}/../../Source/Components/NeptuneISPSubwindow.h
        ${CMAKE_CURRENT_LIST_DIR}/../../Source/Components/HorizontalBarMeter.cpp
        ${CMAKE_CURRENT_LIST_DIR}/../../Source/Components/HorizontalBarMeter.h
        # Steal The Mojo - Psychedelic UI components (header-only)
        ${CMAKE_CURRENT_LIST_DIR}/../../Source/Components/PsychedelicStyles.h
        ${CMAKE_CURRENT_LIST_DIR}/../../Source/Components/PsychedelicKnob.h
        ${CMAKE_CURRENT_LIST_DIR}/../../Source/Components/MojoRadarChart.h
        ${CMAKE_CURRENT_LIST_DIR}/../../Source/Components/CharacterMeter.h
        ${CMAKE_CURRENT_LIST_DIR}/../../Source/Components/StealMojoComponent.h
        # Steal The Mojo - Stem extraction system
        ${CMAKE_CURRENT_LIST_DIR}/../../Source/Audio/StemExtractor.cpp
        ${CMAKE_CURRENT_LIST_DIR}/../../Source/Audio/StemExtractor.h
        ${CMAKE_CURRENT_LIST_DIR}/../../Source/Audio/PythonBridge.cpp
        ${CMAKE_CURRENT_LIST_DIR}/../../Source/Audio/PythonBridge.h
        ${CMAKE_CURRENT_LIST_DIR}/../../Source/Audio/DSPSeparator.cpp
        ${CMAKE_CURRENT_LIST_DIR}/../../Source/Audio/DSPSeparator.h
        # Steal The Mojo - Audio analysis system (Phase 2)
        ${CMAKE_CURRENT_LIST_DIR}/../../Source/Audio/AudioAnalyzer.cpp
        ${CMAKE_CURRENT_LIST_DIR}/../../Source/Audio/AudioAnalyzer.h
    )
    target_include_directories(progui_ui PUBLIC
        ${CMAKE_CURRENT_LIST_DIR}/../../Source
        ${CMAKE_CURRENT_LIST_DIR}/../../shared/ui_core
        ${CMAKE_CURRENT_BINARY_DIR}/ProGUIJuceRunner_artefacts/JuceLibraryCode
    )
    target_link_libraries(progui_ui PUBLIC
        juce::juce_gui_extra
        juce::juce_gui_basics
        juce::juce_graphics
        juce::juce_events
        juce::juce_core
        juce::juce_dsp
        juce::juce_audio_basics
        juce::juce_audio_formats
    )
endif()
cmake_minimum_required(VERSION 3.22)
project(ProGUI_JuceRunner VERSION 1.0.0 LANGUAGES C CXX)

# JUCE location (as an embedded directory in this repo)
# Adjust this if your JUCE source is elsewhere
set(JUCE_DIR "${CMAKE_CURRENT_LIST_DIR}/../../JUCE")

# Only add JUCE if it hasn't been added by parent CMakeLists
if(NOT TARGET juce::juce_core)
    if(NOT EXISTS "${JUCE_DIR}/CMakeLists.txt")
        message(FATAL_ERROR "JUCE not found at ${JUCE_DIR}. Update JUCE_DIR or add JUCE as a submodule at ProGUI/JUCE")
    endif()

    # JUCE requires C++17
    set(CMAKE_CXX_STANDARD 17)
    set(CMAKE_CXX_STANDARD_REQUIRED ON)
    set(CMAKE_C_STANDARD 11)
    set(CMAKE_C_STANDARD_REQUIRED ON)

    # Add JUCE
    message(STATUS "Adding JUCE from JuceRunner subdirectory")
    add_subdirectory("${JUCE_DIR}" JUCE-build)
else()
    message(STATUS "JUCE already added by parent - skipping")
endif()

# Sources
set(RUNNER_SOURCES
    ${CMAKE_CURRENT_LIST_DIR}/main.cpp
    ${CMAKE_CURRENT_LIST_DIR}/../../Source/UIDemoComponent.h
)

# Create a GUI app
juce_add_gui_app(ProGUIJuceRunner
    VERSION ${PROJECT_VERSION}
    PRODUCT_NAME "ProGUI JUCE Runner"
)

# Add sources to target
target_sources(ProGUIJuceRunner PRIVATE ${RUNNER_SOURCES})

# JUCE modules required
target_link_libraries(ProGUIJuceRunner PRIVATE
    progui_ui
    juce::juce_gui_basics
    juce::juce_gui_extra
    juce::juce_graphics
    juce::juce_data_structures
    juce::juce_core
    juce::juce_dsp
    juce::juce_audio_basics
    juce::juce_audio_formats
    juce::juce_audio_devices
    juce::juce_audio_processors
    juce::juce_audio_utils
)

# Ensure generated JuceHeader.h is found
target_include_directories(ProGUIJuceRunner PRIVATE
    ${CMAKE_CURRENT_BINARY_DIR}/ProGUIJuceRunner_artefacts/JuceLibraryCode
    ${CMAKE_CURRENT_LIST_DIR}/../../Source
    ${CMAKE_CURRENT_LIST_DIR}/../../shared/ui_core
)

# macOS app settings
if(APPLE)
    set_target_properties(ProGUIJuceRunner PROPERTIES
        MACOSX_BUNDLE TRUE
        MACOSX_BUNDLE_GUI_IDENTIFIER com.moremojo.progui.jucerunner
        MACOSX_BUNDLE_BUNDLE_NAME "ProGUI JUCE Runner"
    )
endif()
