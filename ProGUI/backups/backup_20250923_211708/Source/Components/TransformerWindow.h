#pragma once
#include <juce_gui_basics/juce_gui_basics.h>
#include "../../shared/ui_core/ProcessSubwindow.h"

class TransformerWindow : public ProcessSubwindow {
public:
    TransformerWindow() : ProcessSubwindow("Transformer — Harmonic Driver")
    {
        addAndMakeVisible(drive);       cfgSlider(drive, 0,100," %",30);
        addAndMakeVisible(balance);     cfgSlider(balance, 0,1,"",0.5);
        addAndMakeVisible(bias);        cfgSlider(bias, -1,1,"",0);
        addAndMakeVisible(coreMode);    coreMode.addItem("Iron",1); coreMode.addItem("Nickel",2); coreMode.addItem("Copper",3); coreMode.setSelectedId(1);
        addAndMakeVisible(hfEmphasis);  cfgSlider(hfEmphasis, 0,1,"",0.25);
        addAndMakeVisible(lfSag);       cfgSlider(lfSag, 0,1,"",0.35);
        addAndMakeVisible(dynamics);    dynamics.addItem("Fast",1); dynamics.addItem("Slow",2); dynamics.setSelectedId(1);
        addAndMakeVisible(mix);         cfgSlider(mix, 0,100," %",50);
        addAndMakeVisible(outputTrim);  cfgSlider(outputTrim, -24,24," dB",0);
        addAndMakeVisible(bypass);      bypass.setButtonText("Bypass");

        addLabel(lblDrive,   "Drive");
        addLabel(lblBalance, "Even/Odd Balance (H2/H3)");
        addLabel(lblBias,    "Bias");
        addLabel(lblCore,    "Core Mode");
        addLabel(lblHF,      "HF Emphasis");
        addLabel(lblLF,      "LF Sag");
        addLabel(lblDyn,     "Dynamics");
        addLabel(lblMix,     "Mix");
        addLabel(lblTrim,    "Output Trim");
    }

    void resized() override
    {
        ProcessSubwindow::resized(); auto r=body().getBounds().reduced(12); const int h=28,g=10,w=220;
        auto row=r.removeFromTop(h); lblDrive.setBounds(row.removeFromLeft(w)); drive.setBounds(row); r.removeFromTop(g);
        row=r.removeFromTop(h); lblBalance.setBounds(row.removeFromLeft(w)); balance.setBounds(row); r.removeFromTop(g);
        row=r.removeFromTop(h); lblBias.setBounds(row.removeFromLeft(w)); bias.setBounds(row); r.removeFromTop(g);
        row=r.removeFromTop(h); lblCore.setBounds(row.removeFromLeft(w)); coreMode.setBounds(row.removeFromLeft(180)); r.removeFromTop(g);
        row=r.removeFromTop(h); lblHF.setBounds(row.removeFromLeft(w)); hfEmphasis.setBounds(row); r.removeFromTop(g);
        row=r.removeFromTop(h); lblLF.setBounds(row.removeFromLeft(w)); lfSag.setBounds(row); r.removeFromTop(g);
        row=r.removeFromTop(h); lblDyn.setBounds(row.removeFromLeft(w)); dynamics.setBounds(row.removeFromLeft(160)); r.removeFromTop(g);
        row=r.removeFromTop(h); lblMix.setBounds(row.removeFromLeft(w)); mix.setBounds(row); r.removeFromTop(g);
        row=r.removeFromTop(h); lblTrim.setBounds(row.removeFromLeft(w)); outputTrim.setBounds(row); r.removeFromTop(g);
        row=r.removeFromTop(h); bypass.setBounds(row.removeFromLeft(100));
    }

private:
    static void cfgSlider(juce::Slider& s, double mn,double mx, juce::String suf,double v){ s.setRange(mn,mx,(mx-mn)/10000.0); s.setSliderStyle(juce::Slider::LinearHorizontal); s.setTextBoxStyle(juce::Slider::TextBoxRight,false,72,20); s.setTextValueSuffix(suf); s.setValue(v); }
    void addLabel(juce::Label& l, const juce::String& t){ addAndMakeVisible(l); l.setText(t, juce::dontSendNotification); l.setJustificationType(juce::Justification::centredLeft); l.setColour(juce::Label::textColourId, juce::Colours::white.withAlpha(0.85f)); }

    juce::Slider drive,balance,bias,hfEmphasis,lfSag,mix,outputTrim; juce::ComboBox coreMode,dynamics; juce::ToggleButton bypass;
    juce::Label lblDrive,lblBalance,lblBias,lblCore,lblHF,lblLF,lblDyn,lblMix,lblTrim;
};
