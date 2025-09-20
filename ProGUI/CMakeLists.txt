cmake_minimum_required(VERSION 3.15 FATAL_ERROR)

project(MoreMojoProGUI VERSION 0.1.0 LANGUAGES C CXX)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_OSX_DEPLOYMENT_TARGET "11.0" CACHE STRING "macOS deployment target" FORCE)

# --- JUCE discovery (robust) ---
# If JUCE was cloned by CI and JUCE_DIR is set, use add_subdirectory.
# Otherwise, try find_package with PREFIX path support.
if(DEFINED ENV{JUCE_DIR})
  set(JUCE_DIR "$ENV{JUCE_DIR}")
endif()

if(JUCE_DIR AND EXISTS "${JUCE_DIR}/CMakeLists.txt")
  message(STATUS "Using JUCE from add_subdirectory: ${JUCE_DIR}")
  add_subdirectory("${JUCE_DIR}" JUCE)
else()
  message(STATUS "Using find_package(JUCE)")
  # CMake will search CMAKE_PREFIX_PATH for JUCEConfig.cmake
  find_package(JUCE CONFIG REQUIRED)
endif()

# Create a JUCE-based GUI application
add_subdirectory(shared/ui_core)
juce_add_gui_app(MoreMojoProGUIApp
    COMPANY_NAME "Umbo Gumbo"
    PRODUCT_NAME "More Mojo Pro GUI"
    COMPANY_WEBSITE "www.umbogumbo.com"
    BUNDLE_ID "com.UmboGumbo.MoreMojoProGUI"
    JUCE_GENERATED_INCLUDE_FILE_NAME "${CMAKE_CURRENT_BINARY_DIR}/JuceHeader.h"
)

# Add source files
target_sources(MoreMojoProGUIApp PRIVATE
    Source/Main.cpp
    Source/MainComponent.cpp
    Source/MainComponent.h
    Source/ConsumerOverlay.h
    Source/ProEQComponent.cpp
    Source/ProEQComponent.h
    Source/ProCompressorComponent.cpp
    Source/ProCompressorComponent.h
    Source/CustomLookAndFeel.cpp
    Source/CustomLookAndFeel.h
    Source/SpectrumAnalyzer.cpp
    Source/SpectrumAnalyzer.h
)

# Set compiler definitions
target_compile_definitions(MoreMojoProGUIApp
    PRIVATE
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
    JUCE_APPLICATION_NAME_STRING="$<TARGET_PROPERTY:MoreMojoProGUIApp,JUCE_PRODUCT_NAME>"
    JUCE_APPLICATION_VERSION_STRING="$<TARGET_PROPERTY:MoreMojoProGUIApp,JUCE_VERSION>"
)

# Add include directories
target_include_directories(MoreMojoProGUIApp
    PRIVATE
    ${CMAKE_CURRENT_BINARY_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}/JuceLibraryCode
)

# Add PluginGuiMagic module path
target_include_directories(MoreMojoProGUIApp
    PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/modules/foleys_gui_magic
)

# Link against necessary JUCE modules
target_link_libraries(MoreMojoProGUIApp
    PRIVATE
        juce::juce_core
        juce::juce_data_structures
        juce::juce_events
        juce::juce_graphics
        juce::juce_gui_basics
        juce::juce_gui_extra
        juce::juce_dsp
        juce::juce_audio_basics
        juce::juce_audio_devices
        juce::juce_audio_formats
        juce::juce_audio_processors
        juce::juce_audio_utils
        ui_core
    PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags
)

# Create directory for assets
file(MAKE_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/Assets")

# Shared library to package into a Framework for the Swift TestApp
add_library(MoreMojoProGUI SHARED
    Source/ProGUIWrapper.cpp
    Source/ProGUIWrapper.h
    Source/MainComponent.cpp
    Source/MainComponent.h
    Source/ConsumerOverlay.h
    Source/ProEQComponent.cpp
    Source/ProEQComponent.h
    Source/ProCompressorComponent.cpp
    Source/ProCompressorComponent.h
    Source/CustomLookAndFeel.cpp
    Source/CustomLookAndFeel.h
    Source/SpectrumAnalyzer.cpp
    Source/SpectrumAnalyzer.h
    SwiftBridge/ProGUIBridgeC.cpp
    SwiftBridge/ProGUIBridgeC.h
)

target_compile_definitions(MoreMojoProGUI PRIVATE
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
)

target_include_directories(MoreMojoProGUI PRIVATE
    ${CMAKE_CURRENT_BINARY_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}/JuceLibraryCode
    ${CMAKE_CURRENT_SOURCE_DIR}/modules/foleys_gui_magic
    ${CMAKE_CURRENT_SOURCE_DIR}/SwiftBridge
)

target_link_libraries(MoreMojoProGUI PRIVATE
    juce::juce_gui_extra
    juce::juce_gui_basics
    juce::juce_graphics
    juce::juce_core
    juce::juce_events
    ui_core
)

# Temporarily export all symbols (disable exported_symbols_list filtering)
# set(EXPORTED_SYMBOLS_LIST "${CMAKE_CURRENT_SOURCE_DIR}/SwiftBridge/exported_symbols.list")
# if (APPLE AND EXISTS "${EXPORTED_SYMBOLS_LIST}")
#     target_link_options(MoreMojoProGUI PRIVATE
#         "-Wl,-exported_symbols_list,${EXPORTED_SYMBOLS_LIST}")
# endif()

target_link_libraries(MoreMojoProGUI PRIVATE
    juce::juce_dsp
    juce::juce_audio_basics
    juce::juce_audio_devices
    juce::juce_audio_formats
    juce::juce_audio_processors
    juce::juce_audio_utils
)
