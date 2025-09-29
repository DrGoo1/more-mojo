#pragma once

#include "ProcessSubwindow.h"

/**
 * De-esser / HF Control Subwindow
 * Controls: Freq, Q/BW, Threshold, Amount, Mode (split/full), Listen
 * Meters: Sibilance activity, HF spectrum
 */
class DeEsserSubwindow : public ProcessSubwindow
{
public:
    DeEsserSubwindow();
    ~DeEsserSubwindow() override = default;

protected:
    void createControls() override;
    void layoutControls(juce::Rectangle<int> controlArea) override;
    void paintMeters(juce::Graphics& g, juce::Rectangle<int> meterArea) override;

private:
    // De-esser specific controls
    std::unique_ptr<SkinnedKnob> freqKnob;
    std::unique_ptr<SkinnedKnob> qBandwidthKnob;
    std::unique_ptr<SkinnedKnob> thresholdKnob;
    std::unique_ptr<SkinnedKnob> amountKnob;
    std::unique_ptr<juce::ComboBox> modeCombo;
    std::unique_ptr<juce::ToggleButton> listenButton;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(DeEsserSubwindow)
};
