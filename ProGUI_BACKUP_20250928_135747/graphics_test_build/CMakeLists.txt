cmake_minimum_required(VERSION 3.22)
project(AudioGraphicsTest VERSION 1.0.0)

add_subdirectory(../JUCE JUCE)

juce_add_gui_app(AudioGraphicsTest
    PRODUCT_NAME "Audio Graphics Test"
    VERSION 1.0.0
)

target_sources(AudioGraphicsTest PRIVATE
    ../AudioGraphicsTest.cpp
)

target_link_libraries(AudioGraphicsTest PRIVATE
    juce::juce_gui_basics
    juce::juce_gui_extra
)

target_compile_features(AudioGraphicsTest PRIVATE cxx_std_17)
