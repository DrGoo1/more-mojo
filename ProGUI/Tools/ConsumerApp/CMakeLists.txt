cmake_minimum_required(VERSION 3.22)

project(MoreMojoConsumer VERSION 1.0.0)

# Add JUCE
add_subdirectory(../../JUCE JUCE EXCLUDE_FROM_ALL)

# Consumer App executable
juce_add_gui_app(MoreMojoConsumer
    PRODUCT_NAME "MoreMojo"
    COMPANY_NAME "MoreMojo Audio"
    BUNDLE_ID "com.moremojo.consumer"
)

# Source files
target_sources(MoreMojoConsumer PRIVATE
    main.cpp
    ../../Source/Components/ConsumerGUI.h
    ../../Source/Components/PsychedelicTheme.h
    ../../Source/Components/StealMojoComponent.h
    ../../Source/UIDemoComponent.cpp
    ../../Source/UIDemoComponent.h
    ../../Source/Components/NeptuneMixKnob.cpp
    ../../Source/Components/NeptuneMixKnob.h
    ../../Source/Components/HorizontalBarMeter.cpp
    ../../Source/Components/HorizontalBarMeter.h
    ../../shared/ui_core/SkinManager.h
    ../../shared/ui_core/SkinnedKnob.h
    ../../Source/Audio/StemExtractor.h
    ../../Source/Audio/StemExtractor.cpp
    ../../Source/Audio/DSPSeparator.h
    ../../Source/Audio/DSPSeparator.cpp
    ../../Source/Audio/AudioAnalyzer.h
    ../../Source/Audio/AudioAnalyzer.cpp
    ../../Source/Audio/PythonBridge.h
    ../../Source/Audio/PythonBridge.cpp
    ../../Source/Components/MojoRadarChart.h
)

# Include directories
target_include_directories(MoreMojoConsumer PRIVATE
    ../../Source
    ../../Source/Components
    ../../Source/Audio
    ../../shared/ui_core
)

# Link JUCE modules
target_link_libraries(MoreMojoConsumer PRIVATE
    juce::juce_gui_extra
    juce::juce_audio_basics
    juce::juce_audio_devices
    juce::juce_audio_formats
    juce::juce_audio_utils
    juce::juce_dsp
)

# Compiler definitions
target_compile_definitions(MoreMojoConsumer PRIVATE
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
    JUCE_APPLICATION_NAME_STRING="$<TARGET_PROPERTY:MoreMojoConsumer,JUCE_PRODUCT_NAME>"
    JUCE_APPLICATION_VERSION_STRING="$<TARGET_PROPERTY:MoreMojoConsumer,JUCE_VERSION>"
)
