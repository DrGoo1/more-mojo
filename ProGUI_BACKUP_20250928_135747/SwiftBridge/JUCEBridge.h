#pragma once

/**
 * JUCEBridge.h
 * Bridge header for connecting JUCE C++ components with Swift code
 *
 * This header defines the C++ interface that will be exposed to Swift
 * through a bridging header and Objective-C++ wrapper classes.
 */

#ifdef __cplusplus
extern "C" {
#endif

// Component type enumeration
typedef enum {
    kSpectrumAnalyzerComponent = 0,
    kProEQComponent = 1,
    kProCompressorComponent = 2
} JUCEComponentType;

// Forward declarations for opaque pointers
typedef struct JUCEComponentWrapper JUCEComponentWrapper;
typedef struct AudioBuffer AudioBuffer;

// Component creation and destruction
JUCEComponentWrapper* CreateJUCEComponent(JUCEComponentType type, float width, float height);
void DestroyJUCEComponent(JUCEComponentWrapper* component);

// Component sizing
void ResizeJUCEComponent(JUCEComponentWrapper* component, float width, float height);

// Parameter control
void SetParameterValue(JUCEComponentWrapper* component, const char* parameterId, float value);
float GetParameterValue(JUCEComponentWrapper* component, const char* parameterId);

// Audio buffer handling
AudioBuffer* CreateAudioBuffer(int numChannels, int numSamples);
void DestroyAudioBuffer(AudioBuffer* buffer);
void SetAudioBufferData(AudioBuffer* buffer, int channel, int index, float value);
void ProcessAudioBuffer(JUCEComponentWrapper* component, AudioBuffer* buffer);

// NSView integration
void* GetNativeView(JUCEComponentWrapper* component);
void AttachToParentView(JUCEComponentWrapper* component, void* parentView);

#ifdef __cplusplus
}
#endif
