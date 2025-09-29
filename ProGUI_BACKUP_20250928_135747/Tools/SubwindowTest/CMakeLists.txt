cmake_minimum_required(VERSION 3.15)
project(SubwindowTest VERSION 1.0.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_OSX_DEPLOYMENT_TARGET "11.0" CACHE STRING "macOS deployment target" FORCE)

# Find JUCE
if(DEFINED ENV{JUCE_DIR})
  set(JUCE_DIR "$ENV{JUCE_DIR}")
endif()

if(JUCE_DIR AND EXISTS "${JUCE_DIR}/CMakeLists.txt")
  message(STATUS "Using JUCE from add_subdirectory: ${JUCE_DIR}")
  add_subdirectory("${JUCE_DIR}" JUCE)
else()
  message(STATUS "Using find_package(JUCE)")
  find_package(JUCE CONFIG REQUIRED)
endif()

# Create the test application
juce_add_gui_app(SubwindowTest
    COMPANY_NAME "Test"
    PRODUCT_NAME "Subwindow Test"
    BUNDLE_ID "com.test.SubwindowTest"
)

# Add source files
target_sources(SubwindowTest PRIVATE
    main.cpp
    ../../Source/ProcessSubwindow.cpp
    ../../Source/ProcessSubwindow.h
    ../../Source/ISPSubwindow.cpp
    ../../Source/ISPSubwindow.h
    ../../Source/SRCSubwindow.cpp
    ../../Source/SRCSubwindow.h
    ../../Source/JitterSubwindow.cpp
    ../../Source/JitterSubwindow.h
    ../../Source/AlignSubwindow.cpp
    ../../Source/AlignSubwindow.h
    ../../Source/TransientSubwindow.cpp
    ../../Source/TransientSubwindow.h
    ../../Source/DeEsserSubwindow.cpp
    ../../Source/DeEsserSubwindow.h
    ../../Source/MLARSubwindow.cpp
    ../../Source/MLARSubwindow.h
    ../../Source/TransformerSubwindow.cpp
    ../../Source/TransformerSubwindow.h
)

# Include directories
target_include_directories(SubwindowTest PRIVATE
    ../../Source
    ../../shared/ui_core
)

# Link libraries
target_link_libraries(SubwindowTest PRIVATE
    juce::juce_gui_basics
    juce::juce_gui_extra
    juce::juce_recommended_config_flags
    juce::juce_recommended_lto_flags
    juce::juce_recommended_warning_flags
)
