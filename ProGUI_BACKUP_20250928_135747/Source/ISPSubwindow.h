#pragma once

#include "ProcessSubwindow.h"

/**
 * Intersample Interpolation / True-Peak (ISP) Subwindow
 * Controls: OS Factor (2×/4×/8×), Filter (linear/min/polyphase), Passband Rolloff, 
 *          Stopband Atten, TP Ceiling, Lookahead (ms)
 * Meters: True-Peak, ISP count, Ceiling margin
 */
class ISPSubwindow : public ProcessSubwindow
{
public:
    ISPSubwindow();
    ~ISPSubwindow() override = default;

protected:
    void createControls() override;
    void layoutControls(juce::Rectangle<int> controlArea) override;
    void paintMeters(juce::Graphics& g, juce::Rectangle<int> meterArea) override;

private:
    // ISP-specific controls
    std::unique_ptr<juce::ComboBox> osFactorCombo;
    std::unique_ptr<juce::ComboBox> filterTypeCombo;
    std::unique_ptr<SkinnedKnob> passbandRolloffKnob;
    std::unique_ptr<SkinnedKnob> stopbandAttenKnob;
    std::unique_ptr<SkinnedKnob> tpCeilingKnob;
    std::unique_ptr<SkinnedKnob> lookaheadKnob;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ISPSubwindow)
};
