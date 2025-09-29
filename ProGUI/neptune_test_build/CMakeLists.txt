cmake_minimum_required(VERSION 3.22)
project(SimpleNeptuneTest VERSION 1.0.0)

add_subdirectory(../JUCE JUCE)

juce_add_gui_app(SimpleNeptuneTest
    PRODUCT_NAME "Simple Neptune Test"
    VERSION 1.0.0
)

target_sources(SimpleNeptuneTest PRIVATE
    ../SimpleNeptuneTest.cpp
)

target_link_libraries(SimpleNeptuneTest PRIVATE
    juce::juce_gui_basics
    juce::juce_gui_extra
)

target_compile_features(SimpleNeptuneTest PRIVATE cxx_std_17)
