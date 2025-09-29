#pragma once
#include <juce_gui_basics/juce_gui_basics.h>
#include "../../shared/ui_core/ProcessSubwindow.h"

// UI-only subwindow for TransientDetect / Shaper
class TransientWindow : public ProcessSubwindow {
public:
    TransientWindow() : ProcessSubwindow("Transient — Detect/Shaper")
    {
        // Bands
        addAndMakeVisible(bands);
        bands.addItem("Full-band", 1);
        bands.addItem("2-band", 2);
        bands.addItem("3-band", 3);
        bands.setSelectedId(1);

        // Sensitivity
        addAndMakeVisible(sensitivity);
        sensitivity.setRange(0.0, 1.0, 0.001);
        sensitivity.setSliderStyle(juce::Slider::LinearHorizontal);
        sensitivity.setTextBoxStyle(juce::Slider::TextBoxRight, false, 72, 20);
        sensitivity.setValue(0.5);

        // Hysteresis
        addAndMakeVisible(hysteresis);
        hysteresis.setRange(0.0, 0.5, 0.001);
        hysteresis.setSliderStyle(juce::Slider::LinearHorizontal);
        hysteresis.setTextBoxStyle(juce::Slider::TextBoxRight, false, 72, 20);
        hysteresis.setValue(0.1);

        // Smoothing
        addAndMakeVisible(smoothing);
        smoothing.setRange(0.0, 50.0, 0.1);
        smoothing.setSliderStyle(juce::Slider::LinearHorizontal);
        smoothing.setTextBoxStyle(juce::Slider::TextBoxRight, false, 72, 20);
        smoothing.setTextValueSuffix(" ms");
        smoothing.setValue(8.0);

        // Labels
        addLabel(lblBands, "Bands");
        addLabel(lblSens,  "Sensitivity");
        addLabel(lblHyst,  "Hysteresis");
        addLabel(lblSmooth,"Smoothing");
    }

    void resized() override
    {
        ProcessSubwindow::resized();
        auto r = body().getBounds().reduced(12);
        const int h = 28; const int gap = 10; const int labelW = 140;

        auto row = r.removeFromTop(h);
        lblBands.setBounds(row.removeFromLeft(labelW));
        bands.setBounds(row.removeFromLeft(180));
        r.removeFromTop(gap);

        row = r.removeFromTop(h);
        lblSens.setBounds(row.removeFromLeft(labelW));
        sensitivity.setBounds(row);
        r.removeFromTop(gap);

        row = r.removeFromTop(h);
        lblHyst.setBounds(row.removeFromLeft(labelW));
        hysteresis.setBounds(row);
        r.removeFromTop(gap);

        row = r.removeFromTop(h);
        lblSmooth.setBounds(row.removeFromLeft(labelW));
        smoothing.setBounds(row);
    }

private:
    void addLabel(juce::Label& l, const juce::String& text)
    {
        addAndMakeVisible(l);
        l.setText(text, juce::dontSendNotification);
        l.setJustificationType(juce::Justification::centredLeft);
        l.setColour(juce::Label::textColourId, juce::Colours::white.withAlpha(0.85f));
    }

    juce::ComboBox bands;
    juce::Slider sensitivity, hysteresis, smoothing;
    juce::Label lblBands, lblSens, lblHyst, lblSmooth;
};
