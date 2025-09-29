#pragma once

#include "../../JUCE/modules/juce_core/juce_core.h"
#include "../../JUCE/modules/juce_events/juce_events.h"
#include "../../JUCE/modules/juce_graphics/juce_graphics.h"
#include "../../JUCE/modules/juce_gui_basics/juce_gui_basics.h"
#include "../../JUCE/modules/juce_gui_extra/juce_gui_extra.h"
#include "ProEQComponent.h"
#include "ProCompressorComponent.h"
#include "ConsumerOverlay.h"
#include "UIDemoComponent.h"

/**
 * ProGUIWrapper - Main wrapper class for Pro GUI components
 * 
 * This class serves as the main integration point between Swift and JUCE,
 * providing an interface for creating and controlling the Pro GUI components.
 */
class ProGUIWrapper : public juce::Component, private juce::Timer
{
public:
    //==============================================================================
    enum ComponentType
    {
        EQComponent = 0,
        CompressorComponent,
        DemoComponent
    };
    
    //==============================================================================
    ProGUIWrapper(ComponentType type);
    ~ProGUIWrapper() override;
    
    //==============================================================================
    void paint(juce::Graphics& g) override;
    void resized() override;
    
    //==============================================================================
    /**
     * Set a parameter value
     * 
     * @param parameterId The ID of the parameter to set
     * @param value The value to set
     */
    void setParameterValue(const char* parameterId, float value);
    
    /**
     * Get a parameter value
     * 
     * @param parameterId The ID of the parameter to get
     * @return The parameter value
     */
    float getParameterValue(const char* parameterId);
    
    /**
     * Process audio data through the component
     * 
     * @param audioData The audio data to process
     * @param numChannels The number of channels in the audio data
     * @param numSamples The number of samples per channel
     */
    void processAudio(float** audioData, int numChannels, int numSamples);
    
    // Overlay controls
    void setOverlayVisible(bool shouldShow);
    bool isOverlayVisible() const { return overlayVisible; }

    // Test signal controls
    void setTestSignalEnabled(bool enabled);
    bool isTestSignalEnabled() const { return testSignalEnabled; }

    // Timer for test signal generation
    void timerCallback() override;

    /**
     * Get the native component for embedding in Swift UI
     * 
     * @return The native NSView component
     */
    void* getNativeView();
    
    /**
     * Attach the active JUCE component to a provided parent NSView (macOS).
     * This embeds the component as a child view rather than creating a top-level peer.
     */
    void attachToParent(void* parentNSView);
    
private:
    //==============================================================================
    std::unique_ptr<ProEQComponent> eqComponent;
    std::unique_ptr<ProCompressorComponent> compressorComponent;
    std::unique_ptr<UIDemoComponent> demoComponent;
    std::unique_ptr<juce::NSViewComponent> nativeWrapper;
    std::unique_ptr<ConsumerOverlay> consumerOverlay;
    bool overlayVisible = false;
    bool testSignalEnabled = false;
    double testPhase = 0.0;
    double testSampleRate = 44100.0;
    double testFrequency = 440.0;
    
    ComponentType activeComponentType;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ProGUIWrapper)
};
