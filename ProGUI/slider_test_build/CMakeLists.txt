cmake_minimum_required(VERSION 3.22)
project(NeptuneSliderTest VERSION 1.0.0)

add_subdirectory(../JUCE JUCE)

juce_add_gui_app(NeptuneSliderTest
    PRODUCT_NAME "Neptune Slider Test"
    VERSION 1.0.0
)

target_sources(NeptuneSliderTest PRIVATE
    ../NeptuneSliderTest.cpp
)

target_link_libraries(NeptuneSliderTest PRIVATE
    juce::juce_gui_basics
    juce::juce_gui_extra
)

target_compile_features(NeptuneSliderTest PRIVATE cxx_std_17)
