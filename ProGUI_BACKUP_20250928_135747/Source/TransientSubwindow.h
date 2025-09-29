#pragma once

#include "ProcessSubwindow.h"

/**
 * Transient Detect / Shaper Subwindow
 * Controls: Sensitivity, Attack/Hold/Release (ms), Boost/Soften, HF Focus, Band Mode (full/split)
 * Meters: Transient timeline, Before/After waveform, Spectral emphasis
 */
class TransientSubwindow : public ProcessSubwindow
{
public:
    TransientSubwindow();
    ~TransientSubwindow() override = default;

protected:
    void createControls() override;
    void layoutControls(juce::Rectangle<int> controlArea) override;
    void paintMeters(juce::Graphics& g, juce::Rectangle<int> meterArea) override;

private:
    // Transient-specific controls
    std::unique_ptr<SkinnedKnob> sensitivityKnob;
    std::unique_ptr<SkinnedKnob> attackKnob;
    std::unique_ptr<SkinnedKnob> holdKnob;
    std::unique_ptr<SkinnedKnob> releaseKnob;
    std::unique_ptr<SkinnedKnob> boostSoftenKnob;
    std::unique_ptr<SkinnedKnob> hfFocusKnob;
    std::unique_ptr<juce::ComboBox> bandModeCombo;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(TransientSubwindow)
};
