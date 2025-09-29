#pragma once

#include "ProcessSubwindow.h"

/**
 * Align (Phase/Time) Subwindow
 * Controls: Delay (samples/ms), Polarity, Phase Rotate, Tilt AP, Crossover, Auto-Align, Link
 * Meters: Correlation, Phase scope, IACC timeline, Group delay
 */
class AlignSubwindow : public ProcessSubwindow
{
public:
    AlignSubwindow();
    ~AlignSubwindow() override = default;

protected:
    void createControls() override;
    void layoutControls(juce::Rectangle<int> controlArea) override;
    void paintMeters(juce::Graphics& g, juce::Rectangle<int> meterArea) override;

private:
    // Align-specific controls
    std::unique_ptr<SkinnedKnob> delayKnob;
    std::unique_ptr<juce::ComboBox> delayUnitsCombo;
    std::unique_ptr<juce::ToggleButton> polarityButton;
    std::unique_ptr<SkinnedKnob> phaseRotateKnob;
    std::unique_ptr<SkinnedKnob> tiltAPKnob;
    std::unique_ptr<SkinnedKnob> crossoverKnob;
    std::unique_ptr<juce::ToggleButton> autoAlignButton;
    std::unique_ptr<juce::ToggleButton> linkButton;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(AlignSubwindow)
};
