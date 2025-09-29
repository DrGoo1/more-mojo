#pragma once
#include <juce_gui_basics/juce_gui_basics.h>

class NeptuneVUMeter : public juce::Component
{
public:
    NeptuneVUMeter();
    
    void setLevel(float newLevel);
    void paint(juce::Graphics& g) override;
    
private:
    void loadNeptuneFrames();
    
    juce::Array<juce::Image> neptuneFrames;
    float currentLevel = 0.0f;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(NeptuneVUMeter)
};
