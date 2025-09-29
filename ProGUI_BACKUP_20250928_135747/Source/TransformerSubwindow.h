#pragma once

#include "ProcessSubwindow.h"

/**
 * Transformer (Harmonic Driver) Subwindow
 * Controls: Drive, Even/Odd Balance (H2/H3), Bias, Core Mode (Iron/Nickel/Copper), 
 *          HF Emphasis, LF Sag, Dynamics (Fast/Slow), Mix, Output Trim, Bypass
 * Meters: Harmonic spectrum (H2/H3 bars), THD, In/Out meters, Gain reduction style ring for Amount
 */
class TransformerSubwindow : public ProcessSubwindow
{
public:
    TransformerSubwindow();
    ~TransformerSubwindow() override = default;

protected:
    void createControls() override;
    void layoutControls(juce::Rectangle<int> controlArea) override;
    void paintMeters(juce::Graphics& g, juce::Rectangle<int> meterArea) override;

private:
    // Transformer-specific controls
    std::unique_ptr<SkinnedKnob> driveKnob;
    std::unique_ptr<SkinnedKnob> evenOddBalanceKnob;
    std::unique_ptr<SkinnedKnob> biasKnob;
    std::unique_ptr<juce::ComboBox> coreModeCombo;
    std::unique_ptr<SkinnedKnob> hfEmphasisKnob;
    std::unique_ptr<SkinnedKnob> lfSagKnob;
    std::unique_ptr<juce::ComboBox> dynamicsCombo;
    std::unique_ptr<SkinnedKnob> mixKnob;
    std::unique_ptr<SkinnedKnob> transformerOutputTrimKnob;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(TransformerSubwindow)
};
