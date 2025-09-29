#pragma once
#include <juce_gui_basics/juce_gui_basics.h>
#include "../../shared/ui_core/ProcessSubwindow.h"

// UI-only subwindow for More Like The Record (MLAR)
class MLARWindow : public ProcessSubwindow {
public:
    MLARWindow() : ProcessSubwindow("MLAR — More Like The Record")
    {
        addAndMakeVisible(amount);      amount.setRange(0.0, 100.0, 0.1); amount.setSliderStyle(juce::Slider::LinearHorizontal); amount.setTextBoxStyle(juce::Slider::TextBoxRight, false, 72, 20); amount.setTextValueSuffix(" %"); amount.setValue(50.0);
        addAndMakeVisible(profile);     profile.addItem("Instrument",1); profile.addItem("Buss",2); profile.addItem("Master",3); profile.setSelectedId(2);
        addAndMakeVisible(focus);       focus.addItem("Timing",1); focus.addItem("Phase",2); focus.addItem("Transient",3); focus.setSelectedId(1);
        addAndMakeVisible(sensitivity); sensitivity.setRange(0.0,1.0,0.001); sensitivity.setSliderStyle(juce::Slider::LinearHorizontal); sensitivity.setTextBoxStyle(juce::Slider::TextBoxRight,false,72,20); sensitivity.setValue(0.5);
        addAndMakeVisible(emphasisHF);  emphasisHF.setRange(0.0,1.0,0.001); emphasisHF.setSliderStyle(juce::Slider::LinearHorizontal); emphasisHF.setTextBoxStyle(juce::Slider::TextBoxRight,false,72,20); emphasisHF.setValue(0.4);
        addAndMakeVisible(emphasisMid); emphasisMid.setRange(0.0,1.0,0.001); emphasisMid.setSliderStyle(juce::Slider::LinearHorizontal); emphasisMid.setTextBoxStyle(juce::Slider::TextBoxRight,false,72,20); emphasisMid.setValue(0.5);
        addAndMakeVisible(emphasisLF);  emphasisLF.setRange(0.0,1.0,0.001); emphasisLF.setSliderStyle(juce::Slider::LinearHorizontal); emphasisLF.setTextBoxStyle(juce::Slider::TextBoxRight,false,72,20); emphasisLF.setValue(0.6);
        addAndMakeVisible(blend);       blend.setRange(0.0,100.0,0.1); blend.setSliderStyle(juce::Slider::LinearHorizontal); blend.setTextBoxStyle(juce::Slider::TextBoxRight,false,72,20); blend.setTextValueSuffix(" %"); blend.setValue(50.0);
        addAndMakeVisible(outputTrim);  outputTrim.setRange(-24.0,24.0,0.1); outputTrim.setSliderStyle(juce::Slider::LinearHorizontal); outputTrim.setTextBoxStyle(juce::Slider::TextBoxRight,false,72,20); outputTrim.setTextValueSuffix(" dB"); outputTrim.setValue(0.0);
        addAndMakeVisible(bypass);      bypass.setButtonText("Bypass");

        addLabel(lblAmount, "Amount"); addLabel(lblProfile, "Profile"); addLabel(lblFocus, "Focus"); addLabel(lblSens, "Sensitivity");
        addLabel(lblHF, "Emphasis HF"); addLabel(lblMid, "Emphasis Mid"); addLabel(lblLF, "Emphasis LF"); addLabel(lblBlend, "Blend (Dry/Wet)"); addLabel(lblTrim, "Output Trim");
    }

    void resized() override
    {
        ProcessSubwindow::resized(); auto r = body().getBounds().reduced(12); const int h=28,g=10,w=160;
        auto row=r.removeFromTop(h); lblAmount.setBounds(row.removeFromLeft(w)); amount.setBounds(row); r.removeFromTop(g);
        row=r.removeFromTop(h); lblProfile.setBounds(row.removeFromLeft(w)); profile.setBounds(row.removeFromLeft(200)); r.removeFromTop(g);
        row=r.removeFromTop(h); lblFocus.setBounds(row.removeFromLeft(w)); focus.setBounds(row.removeFromLeft(200)); r.removeFromTop(g);
        row=r.removeFromTop(h); lblSens.setBounds(row.removeFromLeft(w)); sensitivity.setBounds(row); r.removeFromTop(g);
        row=r.removeFromTop(h); lblHF.setBounds(row.removeFromLeft(w)); emphasisHF.setBounds(row); r.removeFromTop(g);
        row=r.removeFromTop(h); lblMid.setBounds(row.removeFromLeft(w)); emphasisMid.setBounds(row); r.removeFromTop(g);
        row=r.removeFromTop(h); lblLF.setBounds(row.removeFromLeft(w)); emphasisLF.setBounds(row); r.removeFromTop(g);
        row=r.removeFromTop(h); lblBlend.setBounds(row.removeFromLeft(w)); blend.setBounds(row); r.removeFromTop(g);
        row=r.removeFromTop(h); lblTrim.setBounds(row.removeFromLeft(w)); outputTrim.setBounds(row); r.removeFromTop(g);
        row=r.removeFromTop(h); bypass.setBounds(row.removeFromLeft(100));
    }

private:
    void addLabel(juce::Label& l, const juce::String& t){ addAndMakeVisible(l); l.setText(t, juce::dontSendNotification); l.setJustificationType(juce::Justification::centredLeft); l.setColour(juce::Label::textColourId, juce::Colours::white.withAlpha(0.85f)); }

    juce::Slider amount,sensitivity,emphasisHF,emphasisMid,emphasisLF,blend,outputTrim; juce::ComboBox profile,focus; juce::ToggleButton bypass;
    juce::Label lblAmount,lblProfile,lblFocus,lblSens,lblHF,lblMid,lblLF,lblBlend,lblTrim;
};
