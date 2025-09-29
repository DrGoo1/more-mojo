#pragma once

#include "ProcessSubwindow.h"

/**
 * Sample Rate Conversion (SRC) Subwindow
 * Controls: Target SR, Quality (Fast/HQ/Extreme), Passband Ripple, Stopband Atten, Phase (lin/min/mixed)
 * Meters: Aliasing residual, Latency
 */
class SRCSubwindow : public ProcessSubwindow
{
public:
    SRCSubwindow();
    ~SRCSubwindow() override = default;

protected:
    void createControls() override;
    void layoutControls(juce::Rectangle<int> controlArea) override;
    void paintMeters(juce::Graphics& g, juce::Rectangle<int> meterArea) override;

private:
    // SRC-specific controls
    std::unique_ptr<juce::ComboBox> targetSRCombo;
    std::unique_ptr<juce::ComboBox> qualityCombo;
    std::unique_ptr<juce::ComboBox> phaseCombo;
    std::unique_ptr<SkinnedKnob> passbandRippleKnob;
    std::unique_ptr<SkinnedKnob> stopbandAttenKnob;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SRCSubwindow)
};
