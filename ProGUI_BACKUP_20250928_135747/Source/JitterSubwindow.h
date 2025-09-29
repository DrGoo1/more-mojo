#pragma once

#include "ProcessSubwindow.h"

/**
 * Jitter & Accumulation (Quant/Dither) Subwindow
 * Controls: Jitter RMS, Jitter Spectrum, Accum Bit-Depth, Quant Mode, Dither Type, Dither Level, Noise Floor Target
 * Meters: Noise spectrum, THD+N, Time-variance
 */
class JitterSubwindow : public ProcessSubwindow
{
public:
    JitterSubwindow();
    ~JitterSubwindow() override = default;

protected:
    void createControls() override;
    void layoutControls(juce::Rectangle<int> controlArea) override;
    void paintMeters(juce::Graphics& g, juce::Rectangle<int> meterArea) override;

private:
    // Jitter-specific controls
    std::unique_ptr<SkinnedKnob> jitterRMSKnob;
    std::unique_ptr<juce::ComboBox> jitterSpectrumCombo;
    std::unique_ptr<juce::ComboBox> accumBitDepthCombo;
    std::unique_ptr<juce::ComboBox> quantModeCombo;
    std::unique_ptr<juce::ComboBox> ditherTypeCombo;
    std::unique_ptr<SkinnedKnob> ditherLevelKnob;
    std::unique_ptr<SkinnedKnob> noiseFloorKnob;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(JitterSubwindow)
};
