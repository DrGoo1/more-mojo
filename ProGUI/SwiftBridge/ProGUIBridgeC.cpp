#include "ProGUIBridgeC.h"
#include "../Source/ProGUIWrapper.h"

extern "C" {

// Create a Pro GUI component
PROGUI_EXPORT void* ProGUI_CreateComponent(int componentType, float width, float height)
{
    auto wrapper = new ProGUIWrapper(static_cast<ProGUIWrapper::ComponentType>(componentType));
    wrapper->setSize(static_cast<int>(width), static_cast<int>(height));
    return static_cast<void*>(wrapper);
}

// Destroy a Pro GUI component
PROGUI_EXPORT void ProGUI_DestroyComponent(void* component)
{
    auto wrapper = static_cast<ProGUIWrapper*>(component);
    if (wrapper != nullptr)
        delete wrapper;
}

// Resize a Pro GUI component
PROGUI_EXPORT void ProGUI_ResizeComponent(void* component, float width, float height)
{
    auto wrapper = static_cast<ProGUIWrapper*>(component);
    if (wrapper != nullptr)
        wrapper->setSize(static_cast<int>(width), static_cast<int>(height));
}

// Set a parameter value
PROGUI_EXPORT void ProGUI_SetParameterValue(void* component, const char* parameterId, float value)
{
    auto wrapper = static_cast<ProGUIWrapper*>(component);
    if (wrapper != nullptr)
        wrapper->setParameterValue(parameterId, value);
}

// Get a parameter value
PROGUI_EXPORT float ProGUI_GetParameterValue(void* component, const char* parameterId)
{
    auto wrapper = static_cast<ProGUIWrapper*>(component);
    if (wrapper != nullptr)
        return wrapper->getParameterValue(parameterId);
    return 0.0f;
}

// Process audio data
PROGUI_EXPORT void ProGUI_ProcessAudio(void* component, float** audioData, int numChannels, int numSamples)
{
    auto wrapper = static_cast<ProGUIWrapper*>(component);
    if (wrapper != nullptr)
        wrapper->processAudio(audioData, numChannels, numSamples);
}

// Get the native view for embedding in Swift UI
PROGUI_EXPORT void* ProGUI_GetNativeView(void* component)
{
    auto wrapper = static_cast<ProGUIWrapper*>(component);
    if (wrapper != nullptr)
        return wrapper->getNativeView();
    return nullptr;
}

// Set overlay visibility
PROGUI_EXPORT void ProGUI_SetOverlayVisible(void* component, int visible)
{
    auto wrapper = static_cast<ProGUIWrapper*>(component);
    if (wrapper != nullptr)
        wrapper->setOverlayVisible(visible != 0);
}

// Enable/disable test signal
PROGUI_EXPORT void ProGUI_SetTestSignalEnabled(void* component, int enabled)
{
    auto wrapper = static_cast<ProGUIWrapper*>(component);
    if (wrapper != nullptr)
        wrapper->setTestSignalEnabled(enabled != 0);
}

// Attach the JUCE component to a provided parent NSView
PROGUI_EXPORT void ProGUI_AttachToParent(void* component, void* parentNSView)
{
    auto wrapper = static_cast<ProGUIWrapper*>(component);
    if (wrapper != nullptr)
        wrapper->attachToParent(parentNSView);
}

} // extern "C"
