cmake_minimum_required(VERSION 3.22)
project(LedBarMeterTest VERSION 1.0.0)

add_subdirectory(../JUCE JUCE)

juce_add_gui_app(LedBarMeterTest
    PRODUCT_NAME "LED Bar Meter Test"
    VERSION 1.0.0
)

target_sources(LedBarMeterTest PRIVATE
    ../LedBarMeterTest.cpp
)

target_link_libraries(LedBarMeterTest PRIVATE
    juce::juce_gui_basics
    juce::juce_gui_extra
)

target_compile_features(LedBarMeterTest PRIVATE cxx_std_17)
