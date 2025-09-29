#pragma once

#include "ProcessSubwindow.h"

/**
 * More Like The Record (MLAR) Subwindow
 * Controls: Amount (0–100%), Profile (Instrument/Buss/Master), Focus (Timing/Phase/Transient), 
 *          Sensitivity, Emphasis (HF/Mid/LF), Blend (Dry/Wet), Output Trim, Bypass
 * Meters: Before/After waveform, Correlation/Phase meter, Residual difference, Activity timeline
 */
class MLARSubwindow : public ProcessSubwindow
{
public:
    MLARSubwindow();
    ~MLARSubwindow() override = default;

protected:
    void createControls() override;
    void layoutControls(juce::Rectangle<int> controlArea) override;
    void paintMeters(juce::Graphics& g, juce::Rectangle<int> meterArea) override;

private:
    // MLAR-specific controls
    std::unique_ptr<SkinnedKnob> mlarAmountKnob;
    std::unique_ptr<juce::ComboBox> profileCombo;
    std::unique_ptr<juce::ComboBox> focusCombo;
    std::unique_ptr<SkinnedKnob> sensitivityKnob;
    std::unique_ptr<SkinnedKnob> hfEmphasisKnob;
    std::unique_ptr<SkinnedKnob> midEmphasisKnob;
    std::unique_ptr<SkinnedKnob> lfEmphasisKnob;
    std::unique_ptr<SkinnedKnob> blendKnob;
    std::unique_ptr<SkinnedKnob> outputTrimKnob;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MLARSubwindow)
};
