#pragma once
#include <juce_gui_basics/juce_gui_basics.h>
#include "../../shared/ui_core/ProcessSubwindow.h"

// UI-only subwindow for Jitter Control & Quant/Dither
class QuantDitherWindow : public ProcessSubwindow {
public:
    QuantDitherWindow() : ProcessSubwindow("Quant/Dither — Jitter & Accumulator")
    {
        // Jitter reduction
        addAndMakeVisible(jitterOn);
        jitterOn.setButtonText("Jitter Reduction");
        jitterOn.setToggleState(true, juce::dontSendNotification);

        // Accumulator precision
        addAndMakeVisible(accumPrecision);
        accumPrecision.addItem("Auto", 1);
        accumPrecision.addItem("High", 2);
        accumPrecision.setSelectedId(1);

        // Dither type
        addAndMakeVisible(ditherType);
        ditherType.addItem("Off", 1);
        ditherType.addItem("TPDF", 2);
        ditherType.addItem("Noise Shaped", 3);
        ditherType.setSelectedId(2);

        // Dither level
        addAndMakeVisible(ditherLevel);
        ditherLevel.setRange(0.0, 1.0, 0.001);
        ditherLevel.setSliderStyle(juce::Slider::LinearHorizontal);
        ditherLevel.setTextBoxStyle(juce::Slider::TextBoxRight, false, 72, 20);
        ditherLevel.setValue(0.25);

        // Noise floor target
        addAndMakeVisible(noiseFloor);
        noiseFloor.setRange(-120.0, -40.0, 0.1);
        noiseFloor.setSliderStyle(juce::Slider::LinearHorizontal);
        noiseFloor.setTextBoxStyle(juce::Slider::TextBoxRight, false, 72, 20);
        noiseFloor.setTextValueSuffix(" dBFS");
        noiseFloor.setValue(-96.0);

        // Labels
        addLabel(lblJitter, "Jitter Reduction");
        addLabel(lblAccum, "Accumulator Precision");
        addLabel(lblType,  "Dither Type");
        addLabel(lblLevel, "Dither Level");
        addLabel(lblNoise, "Noise Floor Target");
    }

    void resized() override
    {
        ProcessSubwindow::resized();
        auto r = body().getBounds().reduced(12);
        const int h = 28; const int gap = 10; const int labelW = 200;

        auto row = r.removeFromTop(h);
        lblJitter.setBounds(row.removeFromLeft(labelW));
        jitterOn.setBounds(row.removeFromLeft(200));
        r.removeFromTop(gap);

        row = r.removeFromTop(h);
        lblAccum.setBounds(row.removeFromLeft(labelW));
        accumPrecision.setBounds(row.removeFromLeft(200));
        r.removeFromTop(gap);

        row = r.removeFromTop(h);
        lblType.setBounds(row.removeFromLeft(labelW));
        ditherType.setBounds(row.removeFromLeft(200));
        r.removeFromTop(gap);

        row = r.removeFromTop(h);
        lblLevel.setBounds(row.removeFromLeft(labelW));
        ditherLevel.setBounds(row);
        r.removeFromTop(gap);

        row = r.removeFromTop(h);
        lblNoise.setBounds(row.removeFromLeft(labelW));
        noiseFloor.setBounds(row);
    }

private:
    void addLabel(juce::Label& l, const juce::String& text)
    {
        addAndMakeVisible(l);
        l.setText(text, juce::dontSendNotification);
        l.setJustificationType(juce::Justification::centredLeft);
        l.setColour(juce::Label::textColourId, juce::Colours::white.withAlpha(0.85f));
    }

    juce::ToggleButton jitterOn;
    juce::ComboBox accumPrecision, ditherType;
    juce::Slider ditherLevel, noiseFloor;

    juce::Label lblJitter, lblAccum, lblType, lblLevel, lblNoise;
};
