#pragma once
#include <juce_gui_basics/juce_gui_basics.h>
#include "../../shared/ui_core/ProcessSubwindow.h"

// UI-only subwindow for FractionalDelayAlign (Phase/Time)
class AlignWindow : public ProcessSubwindow {
public:
    AlignWindow() : ProcessSubwindow("Align — Phase/Time")
    {
        // Per-band delay (ms)
        addAndMakeVisible(perBandDelay);
        perBandDelay.setRange(0.0, 20.0, 0.01);
        perBandDelay.setSliderStyle(juce::Slider::LinearHorizontal);
        perBandDelay.setTextBoxStyle(juce::Slider::TextBoxRight, false, 72, 20);
        perBandDelay.setTextValueSuffix(" ms");
        perBandDelay.setValue(2.0);

        // Stereo ITD stabilizer
        addAndMakeVisible(itdStabilizer);
        itdStabilizer.setButtonText("Stereo ITD Stabilizer");
        itdStabilizer.setToggleState(true, juce::dontSendNotification);

        // Max correction (samples)
        addAndMakeVisible(maxCorrection);
        maxCorrection.setRange(0.0, 1024.0, 1.0);
        maxCorrection.setSliderStyle(juce::Slider::LinearHorizontal);
        maxCorrection.setTextBoxStyle(juce::Slider::TextBoxRight, false, 72, 20);
        maxCorrection.setValue(256.0);

        // Comb-avoid safety
        addAndMakeVisible(combAvoid);
        combAvoid.setButtonText("Comb-Avoid Safety");
        combAvoid.setToggleState(true, juce::dontSendNotification);

        // Labels
        addLabel(lblDelay, "Per-Band Delay");
        addLabel(lblMax,   "Max Correction (samples)");
    }

    void resized() override
    {
        ProcessSubwindow::resized();
        auto r = body().getBounds().reduced(12);
        const int h = 28; const int gap = 10; const int labelW = 220;

        auto row = r.removeFromTop(h);
        lblDelay.setBounds(row.removeFromLeft(labelW));
        perBandDelay.setBounds(row);
        r.removeFromTop(gap);

        row = r.removeFromTop(h);
        itdStabilizer.setBounds(row.removeFromLeft(220));
        r.removeFromTop(gap);

        row = r.removeFromTop(h);
        lblMax.setBounds(row.removeFromLeft(labelW));
        maxCorrection.setBounds(row);
        r.removeFromTop(gap);

        row = r.removeFromTop(h);
        combAvoid.setBounds(row.removeFromLeft(220));
    }

private:
    void addLabel(juce::Label& l, const juce::String& text)
    {
        addAndMakeVisible(l);
        l.setText(text, juce::dontSendNotification);
        l.setJustificationType(juce::Justification::centredLeft);
        l.setColour(juce::Label::textColourId, juce::Colours::white.withAlpha(0.85f));
    }

    juce::Slider perBandDelay, maxCorrection;
    juce::ToggleButton itdStabilizer, combAvoid;
    juce::Label lblDelay, lblMax;
};
