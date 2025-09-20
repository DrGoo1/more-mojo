cmake_minimum_required(VERSION 3.15)
project(MoreLikeTheRecord VERSION 3.0.0)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Define MLAR library target
add_library(mlar STATIC)

# Define source files
target_sources(mlar
    PRIVATE
        # DSP Sources
        dsp/TimingSafeResampler.h
        dsp/TransientDetect.h
        dsp/FractionalDelayAlign.h
        dsp/AnalogContinuity.h
        dsp/LimiterLevelMatch.h
        
        # Metrics Sources
        metrics/GDSComputer.h
        metrics/IACCAnalyzer.h
        metrics/TEIAnalyzer.h
        metrics/ResidualNull.h
        
        # UI Sources
        ui/LookAndFeel_Mojo.cpp
        ui/LookAndFeel_Mojo.h
        ui/components/MojoSlider.h
        ui/components/MojoMeters.h
        ui/components/ProcessStrip.h
        ui/views/ConsumerView.cpp
        ui/views/ConsumerView.h
        ui/views/ProOverviewView.cpp
        ui/views/ProOverviewView.h
        ui/views/DetailWindows.h
        
        # Main include
        MLAR.h
)

# Set include directories
target_include_directories(mlar
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
)

# Find or include JUCE
if(NOT TARGET juce::juce_audio_utils)
    # Try to find JUCE if already installed
    find_package(JUCE QUIET)
    
    # If not found, include from subdirectory if available
    if(NOT TARGET juce::juce_audio_utils)
        if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/../../JUCE/CMakeLists.txt")
            add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/../../JUCE" JUCE)
        endif()
    endif()
    
    # Still not found, print message
    if(NOT TARGET juce::juce_audio_utils)
        message(STATUS "JUCE not found - MLAR will be built without JUCE dependencies")
    endif()
endif()

# Link with JUCE modules if available
if(TARGET juce::juce_audio_utils)
    target_link_libraries(mlar
        PRIVATE
            juce::juce_audio_processors
            juce::juce_audio_utils
            juce::juce_dsp
    )
endif()

# Set up binary resources for SVG icons
if(TARGET juce::juce_audio_utils)
    file(GLOB_RECURSE MLAR_SVG_RESOURCES "${CMAKE_CURRENT_SOURCE_DIR}/Resources/icons/*.svg")
    
    # Create binary data target if SVGs exist and not already created
    if(MLAR_SVG_RESOURCES AND NOT TARGET MojoBinaryData)
        juce_add_binary_data(MojoBinaryData SOURCES ${MLAR_SVG_RESOURCES})
        target_link_libraries(mlar PRIVATE MojoBinaryData)
    endif()
endif()

# Add test executable if this is the main project
if(CMAKE_CURRENT_SOURCE_DIR STREQUAL CMAKE_SOURCE_DIR AND TARGET juce::juce_audio_utils)
    # Create test application
    juce_add_gui_app(MLARTest
        VERSION 1.0.0
        PRODUCT_NAME "MLAR Test"
        COMPANY_NAME "MoreMojo Audio"
    )
    
    # Set up test sources
    target_sources(MLARTest
        PRIVATE
            test/Main.cpp
    )
    
    # Link with MLAR and JUCE
    target_link_libraries(MLARTest
        PRIVATE
            mlar
            juce::juce_audio_utils
            juce::juce_dsp
            juce::juce_recommended_config_flags
            juce::juce_recommended_lto_flags
            juce::juce_recommended_warning_flags
    )
    
    # Set C++17 standard for test app
    target_compile_features(MLARTest PRIVATE cxx_std_17)
endif()
