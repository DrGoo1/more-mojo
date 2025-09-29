#pragma once
#include <juce_gui_basics/juce_gui_basics.h>
#include "../../shared/ui_core/ProcessSubwindow.h"

// UI-only subwindow for De-esser / HF Control
class DeEsserWindow : public ProcessSubwindow {
public:
    DeEsserWindow() : ProcessSubwindow("De-esser — HF Control")
    {
        // Frequency (Hz)
        addAndMakeVisible(freq);
        freq.setRange(2000.0, 12000.0, 1.0);
        freq.setSliderStyle(juce::Slider::LinearHorizontal);
        freq.setTextBoxStyle(juce::Slider::TextBoxRight, false, 72, 20);
        freq.setTextValueSuffix(" Hz");
        freq.setValue(6500.0);

        // Q / Bandwidth
        addAndMakeVisible(qbw);
        qbw.setRange(0.1, 10.0, 0.01);
        qbw.setSliderStyle(juce::Slider::LinearHorizontal);
        qbw.setTextBoxStyle(juce::Slider::TextBoxRight, false, 72, 20);
        qbw.setValue(2.0);

        // Threshold (dB)
        addAndMakeVisible(threshold);
        threshold.setRange(-60.0, 0.0, 0.1);
        threshold.setSliderStyle(juce::Slider::LinearHorizontal);
        threshold.setTextBoxStyle(juce::Slider::TextBoxRight, false, 72, 20);
        threshold.setTextValueSuffix(" dB");
        threshold.setValue(-24.0);

        // Amount (%)
        addAndMakeVisible(amount);
        amount.setRange(0.0, 100.0, 0.1);
        amount.setSliderStyle(juce::Slider::LinearHorizontal);
        amount.setTextBoxStyle(juce::Slider::TextBoxRight, false, 72, 20);
        amount.setTextValueSuffix(" %");
        amount.setValue(50.0);

        // Mode
        addAndMakeVisible(mode);
        mode.addItem("Split-band", 1);
        mode.addItem("Full-band", 2);
        mode.setSelectedId(1);

        // Listen (monitor band)
        addAndMakeVisible(listen);
        listen.setButtonText("Listen");

        // Labels
        addLabel(lblFreq, "Frequency");
        addLabel(lblQBW, "Q / Bandwidth");
        addLabel(lblThr, "Threshold");
        addLabel(lblAmt, "Amount");
        addLabel(lblMode, "Mode");
    }

    void resized() override
    {
        ProcessSubwindow::resized();
        auto r = body().getBounds().reduced(12);
        const int h = 28; const int gap = 10; const int labelW = 140;

        auto row = r.removeFromTop(h);
        lblFreq.setBounds(row.removeFromLeft(labelW));
        freq.setBounds(row);
        r.removeFromTop(gap);

        row = r.removeFromTop(h);
        lblQBW.setBounds(row.removeFromLeft(labelW));
        qbw.setBounds(row);
        r.removeFromTop(gap);

        row = r.removeFromTop(h);
        lblThr.setBounds(row.removeFromLeft(labelW));
        threshold.setBounds(row);
        r.removeFromTop(gap);

        row = r.removeFromTop(h);
        lblAmt.setBounds(row.removeFromLeft(labelW));
        amount.setBounds(row);
        r.removeFromTop(gap);

        row = r.removeFromTop(h);
        lblMode.setBounds(row.removeFromLeft(labelW));
        mode.setBounds(row.removeFromLeft(160));
        row.removeFromLeft(10);
        listen.setBounds(row.removeFromLeft(100));
    }

private:
    void addLabel(juce::Label& l, const juce::String& text)
    {
        addAndMakeVisible(l);
        l.setText(text, juce::dontSendNotification);
        l.setJustificationType(juce::Justification::centredLeft);
        l.setColour(juce::Label::textColourId, juce::Colours::white.withAlpha(0.85f));
    }

    juce::Slider freq, qbw, threshold, amount;
    juce::ComboBox mode;
    juce::ToggleButton listen;

    juce::Label lblFreq, lblQBW, lblThr, lblAmt, lblMode;
};
