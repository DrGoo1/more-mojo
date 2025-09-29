#pragma once
#include <juce_gui_basics/juce_gui_basics.h>
#include "NeptuneKnob.h"

class ProcessControl : public juce::Component
{
public:
    ProcessControl(const juce::String& processName, const juce::String& description);
    
    void paint(juce::Graphics& g) override;
    void resized() override;
    
    // Set the level for the cyan bar (0.0 to 1.0)
    void setLevel(float level);
    
    // Get the current knob value
    float getKnobValue() const;
    
    // Set knob value change callback
    std::function<void(float)> onKnobChanged;
    std::function<void()> onOpenControls;
    
private:
    juce::String processName;
    juce::String description;
    float currentLevel = 0.5f;
    
    std::unique_ptr<NeptuneKnob> knob;
    std::unique_ptr<juce::TextButton> openButton;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ProcessControl)
};
