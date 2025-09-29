#pragma once

/**
 * ProGUIBridgeC.h - C-style interface for bridging Swift with C++
 * 
 * This header provides C-style functions that can be called from Swift
 * to interact with the C++ implementation of the Pro GUI components.
 */

#if defined(__GNUC__)
#define PROGUI_EXPORT __attribute__((visibility("default"))) __attribute__((used))
#else
#define PROGUI_EXPORT
#endif

#ifdef __cplusplus
extern "C" {
#endif

/// Create a Pro GUI component
PROGUI_EXPORT void* ProGUI_CreateComponent(int componentType, float width, float height);

/// Destroy a Pro GUI component
PROGUI_EXPORT void ProGUI_DestroyComponent(void* component);

/// Resize a Pro GUI component
PROGUI_EXPORT void ProGUI_ResizeComponent(void* component, float width, float height);

/// Set a parameter value
PROGUI_EXPORT void ProGUI_SetParameterValue(void* component, const char* parameterId, float value);

/// Get a parameter value
PROGUI_EXPORT float ProGUI_GetParameterValue(void* component, const char* parameterId);

/// Process audio data
PROGUI_EXPORT void ProGUI_ProcessAudio(void* component, float** audioData, int numChannels, int numSamples);

/// Get the native view for embedding in Swift UI
PROGUI_EXPORT void* ProGUI_GetNativeView(void* component);

/// Set overlay visibility
PROGUI_EXPORT void ProGUI_SetOverlayVisible(void* component, int visible);

/// Enable or disable continuous test signal generation
PROGUI_EXPORT void ProGUI_SetTestSignalEnabled(void* component, int enabled);

// Attach the active JUCE component to a provided parent NSView (macOS)
PROGUI_EXPORT void ProGUI_AttachToParent(void* component, void* parentNSView);

#ifdef __cplusplus
}
#endif
