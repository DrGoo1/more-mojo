#pragma once
#include <juce_gui_basics/juce_gui_basics.h>
#include "../../shared/ui_core/ProcessSubwindow.h"

// UI-only subwindow for Resampler / SRC
class SRCWindow : public ProcessSubwindow {
public:
    SRCWindow() : ProcessSubwindow("SRC — Resampler")
    {
        // Target SR
        addAndMakeVisible(targetSR);
        targetSR.setTextWhenNoChoicesAvailable("Target SR");
        targetSR.addItem("44.1 kHz", 1);
        targetSR.addItem("48 kHz", 2);
        targetSR.addItem("88.2 kHz", 3);
        targetSR.addItem("96 kHz", 4);
        targetSR.addItem("176.4 kHz", 5);
        targetSR.addItem("192 kHz", 6);
        targetSR.setSelectedId(2);

        // Quality
        addAndMakeVisible(quality);
        quality.addItem("Fast", 1);
        quality.addItem("HQ", 2);
        quality.addItem("Extreme", 3);
        quality.setSelectedId(2);

        // Passband ripple
        addAndMakeVisible(passbandRipple);
        passbandRipple.setRange(0.0, 1.0, 0.0001);
        passbandRipple.setSkewFactor(0.5);
        passbandRipple.setSliderStyle(juce::Slider::LinearHorizontal);
        passbandRipple.setTextBoxStyle(juce::Slider::TextBoxRight, false, 72, 20);
        passbandRipple.setTextValueSuffix(" dB");
        passbandRipple.setValue(0.01);

        // Stopband attenuation
        addAndMakeVisible(stopbandAtten);
        stopbandAtten.setRange(40.0, 140.0, 0.1);
        stopbandAtten.setSliderStyle(juce::Slider::LinearHorizontal);
        stopbandAtten.setTextBoxStyle(juce::Slider::TextBoxRight, false, 72, 20);
        stopbandAtten.setTextValueSuffix(" dB");
        stopbandAtten.setValue(96.0);

        // Phase
        addAndMakeVisible(phase);
        phase.addItem("Linear", 1);
        phase.addItem("Minimum", 2);
        phase.addItem("Mixed", 3);
        phase.setSelectedId(1);

        // Labels
        addLabel(lblSR, "Target Sample Rate");
        addLabel(lblQual, "Quality");
        addLabel(lblPR, "Passband Ripple");
        addLabel(lblSB, "Stopband Atten");
        addLabel(lblPhase, "Phase");
    }

    void resized() override
    {
        ProcessSubwindow::resized();
        auto r = body().getBounds().reduced(12);
        const int h = 28; const int gap = 10; const int labelW = 160;

        auto row = r.removeFromTop(h);
        lblSR.setBounds(row.removeFromLeft(labelW));
        targetSR.setBounds(row.removeFromLeft(220));
        r.removeFromTop(gap);

        row = r.removeFromTop(h);
        lblQual.setBounds(row.removeFromLeft(labelW));
        quality.setBounds(row.removeFromLeft(200));
        r.removeFromTop(gap);

        row = r.removeFromTop(h);
        lblPR.setBounds(row.removeFromLeft(labelW));
        passbandRipple.setBounds(row);
        r.removeFromTop(gap);

        row = r.removeFromTop(h);
        lblSB.setBounds(row.removeFromLeft(labelW));
        stopbandAtten.setBounds(row);
        r.removeFromTop(gap);

        row = r.removeFromTop(h);
        lblPhase.setBounds(row.removeFromLeft(labelW));
        phase.setBounds(row.removeFromLeft(200));
    }

private:
    void addLabel(juce::Label& l, const juce::String& text)
    {
        addAndMakeVisible(l);
        l.setText(text, juce::dontSendNotification);
        l.setJustificationType(juce::Justification::centredLeft);
        l.setColour(juce::Label::textColourId, juce::Colours::white.withAlpha(0.85f));
    }

    juce::ComboBox targetSR, quality, phase;
    juce::Slider passbandRipple, stopbandAtten;
    juce::Label lblSR, lblQual, lblPR, lblSB, lblPhase;
};
