cmake_minimum_required(VERSION 3.15)
project(ui_core LANGUAGES C CXX)

add_library(ui_core INTERFACE)

target_include_directories(ui_core INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})

# Consumers must link JUCE; ui_core is header-only.
