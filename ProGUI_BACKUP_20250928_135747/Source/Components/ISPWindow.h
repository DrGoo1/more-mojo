#pragma once
#include <juce_gui_basics/juce_gui_basics.h>
#include "../../shared/ui_core/ProcessSubwindow.h"

// UI-only subwindow for Intersample Interpolation / True-Peak (ISP)
class ISPWindow : public ProcessSubwindow {
public:
    ISPWindow() : ProcessSubwindow("ISP — True-Peak & Interpolation")
    {
        // Controls
        addAndMakeVisible(osFactor);
        osFactor.setTextWhenNoChoicesAvailable("Oversample");
        osFactor.addItem("2×", 1);
        osFactor.addItem("4×", 2);
        osFactor.addItem("8×", 3);
        osFactor.setSelectedId(2);

        addAndMakeVisible(filterType);
        filterType.addItem("Linear", 1);
        filterType.addItem("Minimum", 2);
        filterType.addItem("Polyphase", 3);
        filterType.setSelectedId(3);

        addAndMakeVisible(ispMargin);
        ispMargin.setRange(0.0, 1.0, 0.001);
        ispMargin.setSkewFactor(0.5);
        ispMargin.setSliderStyle(juce::Slider::LinearHorizontal);
        ispMargin.setTextBoxStyle(juce::Slider::TextBoxRight, false, 72, 20);
        ispMargin.setTextValueSuffix(" dB");
        ispMargin.setValue(0.1);

        addAndMakeVisible(interInterp);
        interInterp.setButtonText("Intersample Interpolation");
        interInterp.setToggleState(true, juce::dontSendNotification);

        addAndMakeVisible(dither);
        dither.addItem("Off", 1);
        dither.addItem("TPDF", 2);
        dither.addItem("Noise Shaped", 3);
        dither.setSelectedId(1);

        // Labels
        addLabel(lblOS, "Oversampling");
        addLabel(lblFilter, "Filter Type");
        addLabel(lblISPM, "ISP Margin");
        addLabel(lblDither, "Dither");
    }

    void resized() override
    {
        ProcessSubwindow::resized();
        auto r = body().getBounds().reduced(12);
        const int h = 28; const int gap = 10; const int labelW = 140;

        auto row = r.removeFromTop(h);
        lblOS.setBounds(row.removeFromLeft(labelW));
        osFactor.setBounds(row);
        r.removeFromTop(gap);

        row = r.removeFromTop(h);
        lblFilter.setBounds(row.removeFromLeft(labelW));
        filterType.setBounds(row);
        r.removeFromTop(gap);

        row = r.removeFromTop(h);
        lblISPM.setBounds(row.removeFromLeft(labelW));
        ispMargin.setBounds(row);
        r.removeFromTop(gap);

        row = r.removeFromTop(h);
        interInterp.setBounds(row.removeFromLeft(240));
        r.removeFromTop(gap);

        row = r.removeFromTop(h);
        lblDither.setBounds(row.removeFromLeft(labelW));
        dither.setBounds(row.removeFromLeft(200));
    }

private:
    void addLabel(juce::Label& l, const juce::String& text)
    {
        addAndMakeVisible(l);
        l.setText(text, juce::dontSendNotification);
        l.setJustificationType(juce::Justification::centredLeft);
        l.setColour(juce::Label::textColourId, juce::Colours::white.withAlpha(0.85f));
    }

    juce::ComboBox osFactor, filterType, dither;
    juce::Slider  ispMargin;
    juce::ToggleButton interInterp;

    juce::Label lblOS, lblFilter, lblISPM, lblDither;
};
