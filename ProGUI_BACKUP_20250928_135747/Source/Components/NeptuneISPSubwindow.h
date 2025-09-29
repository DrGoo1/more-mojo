#pragma once
#include <juce_gui_basics/juce_gui_basics.h>
#include "NeptuneKnob.h"

class NeptuneISPSubwindow : public juce::Component
{
public:
    NeptuneISPSubwindow();
    
    void paint(juce::Graphics& g) override;
    void resized() override;
    
private:
    // Dropdowns
    std::unique_ptr<juce::ComboBox> osFactorCombo;
    std::unique_ptr<juce::ComboBox> filterTypeCombo;
    
    // Neptune knobs (2x2 grid)
    std::unique_ptr<NeptuneKnob> passbandRolloffKnob;
    std::unique_ptr<NeptuneKnob> stopbandAttenKnob;
    std::unique_ptr<NeptuneKnob> tpCeilingKnob;
    std::unique_ptr<NeptuneKnob> lookaheadKnob;
    
    // Labels
    std::unique_ptr<juce::Label> osFactorLabel;
    std::unique_ptr<juce::Label> filterTypeLabel;
    std::unique_ptr<juce::Label> passbandLabel;
    std::unique_ptr<juce::Label> stopbandLabel;
    std::unique_ptr<juce::Label> tpCeilingLabel;
    std::unique_ptr<juce::Label> lookaheadLabel;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(NeptuneISPSubwindow)
};
