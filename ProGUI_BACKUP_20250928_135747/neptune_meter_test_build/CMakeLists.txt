cmake_minimum_required(VERSION 3.22)
project(NeptuneMeterTest VERSION 1.0.0)

add_subdirectory(../JUCE JUCE)

juce_add_gui_app(NeptuneMeterTest
    PRODUCT_NAME "Neptune VU Meter Test"
    VERSION 1.0.0
)

target_sources(NeptuneMeterTest PRIVATE
    ../NeptuneMeterTest.cpp
)

target_link_libraries(NeptuneMeterTest PRIVATE
    juce::juce_gui_basics
    juce::juce_gui_extra
)

target_compile_features(NeptuneMeterTest PRIVATE cxx_std_17)
