#pragma once
#include <juce_gui_basics/juce_gui_basics.h>

class HorizontalBarMeter : public juce::Component
{
public:
    HorizontalBarMeter(const juce::String& label = "");
    
    void setLevel(float newLevel);
    void setLabel(const juce::String& newLabel);
    void paint(juce::Graphics& g) override;
    
private:
    float currentLevel = 0.0f;
    juce::String labelText;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(HorizontalBarMeter)
};
