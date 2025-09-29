#pragma once
#include <juce_gui_basics/juce_gui_basics.h>

class NeptuneMixKnob : public juce::Component
{
public:
    NeptuneMixKnob();
    
    void setValue(float newValue);
    float getValue() const { return currentValue; }
    
    void paint(juce::Graphics& g) override;
    void mouseDown(const juce::MouseEvent& event) override;
    void mouseDrag(const juce::MouseEvent& event) override;
    
    std::function<void(float)> onValueChanged;
    
private:
    void loadNeptuneFrames();
    
    juce::Array<juce::Image> neptuneFrames;
    float currentValue = 0.5f;
    juce::Point<int> lastMousePos;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(NeptuneMixKnob)
};
