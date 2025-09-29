cmake_minimum_required(VERSION 3.22)
project(NeptuneTest VERSION 1.0.0)

# Add JUCE
add_subdirectory(../../JUCE JUCE)

# Create the Neptune Test application
juce_add_gui_app(NeptuneTest
    PRODUCT_NAME "Neptune Test"
    COMPANY_NAME "MoreMojo"
    BUNDLE_ID "com.moremojo.neptunetest"
)

# Add sources
target_sources(NeptuneTest PRIVATE
    main.cpp
    ../../Source/Neptune/NeptuneTestWindow.cpp
    ../../Source/Neptune/NeptuneKnobDemo.cpp
)

# Include directories
target_include_directories(NeptuneTest PRIVATE
    ../../Source
    ../../Source/Neptune
)

# Link JUCE modules
target_link_libraries(NeptuneTest PRIVATE
    juce::juce_gui_basics
    juce::juce_gui_extra
    juce::juce_graphics
    juce::juce_core
    juce::juce_events
    juce::juce_data_structures
)

# Compiler features
target_compile_features(NeptuneTest PRIVATE cxx_std_17)
