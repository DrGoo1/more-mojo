#pragma once
#include <juce_gui_basics/juce_gui_basics.h>
#include "../../shared/ui_core/AmountKnobWithRing.h"

// UI-only: Steal The Mojo window
// - Upload file button
// - Instrument dropdown (source profile to extract)
// - Amount control (knob with ring)
// - Apply/Render button (no DSP yet)
class StealMojoComponent : public juce::Component {
public:
    StealMojoComponent()
    {
        // Title label
        title.setText("Steal The Mojo", juce::dontSendNotification);
        title.setJustificationType(juce::Justification::centred);
        title.setColour(juce::Label::textColourId, juce::Colours::white);
        addAndMakeVisible(title);

        // Upload
        btnUpload.setButtonText("Upload Audio…");
        addAndMakeVisible(btnUpload);
        btnUpload.onClick = [this]
        {
            auto flags = juce::FileBrowserComponent::openMode | juce::FileBrowserComponent::canSelectFiles;
            auto chooser = std::make_shared<juce::FileChooser>("Select audio file", juce::File(), "*.wav;*.aif;*.aiff;*.mp3;*.flac");
            chooser->launchAsync(flags, [this, chooser](const juce::FileChooser& fc){
                auto f = fc.getResult();
                if (! f.existsAsFile()) return;
                selectedFile = f;
                fileLabel.setText(f.getFileName(), juce::dontSendNotification);
                repaint();
            });
        };

        // File label
        fileLabel.setJustificationType(juce::Justification::centredLeft);
        fileLabel.setColour(juce::Label::textColourId, juce::Colours::white.withAlpha(0.85f));
        addAndMakeVisible(fileLabel);

        // Instruments dropdown
        addAndMakeVisible(instruments);
        instruments.addItem("Vocal", 1);
        instruments.addItem("Drums", 2);
        instruments.addItem("Bass", 3);
        instruments.addItem("Guitar", 4);
        instruments.addItem("Piano/Keys", 5);
        instruments.addItem("Strings", 6);
        instruments.addItem("Brass/Winds", 7);
        instruments.addItem("Synth", 8);
        instruments.addItem("Full Mix", 9);
        instruments.onChange = [this]{ selectedInstrument = instruments.getText(); };

        // Amount control
        addAndMakeVisible(amount);
        amount.onAmountChanged = [this](float a){ amountValue = a; };

        // Apply/Render
        btnApply.setButtonText("Apply Mojo");
        addAndMakeVisible(btnApply);
        btnApply.onClick = [this]
        {
            // UI-only placeholder. In future: run extraction + render to new file.
            lastAction = juce::String("[StealMojo] Apply requested — file=") + selectedFile.getFileName()
                       + ", instrument=" + selectedInstrument
                       + ", amount=" + juce::String(amountValue, 2);
            repaint();
        };

        setOpaque(false);
    }

    void resized() override
    {
        auto r = getLocalBounds().reduced(16);
        title.setBounds(r.removeFromTop(28));
        r.removeFromTop(8);

        auto row = r.removeFromTop(32);
        btnUpload.setBounds(row.removeFromLeft(160));
        row.removeFromLeft(8);
        fileLabel.setBounds(row);

        r.removeFromTop(12);
        auto row2 = r.removeFromTop(32);
        instruments.setBounds(row2.removeFromLeft(220));

        r.removeFromTop(12);
        auto mid = r.removeFromTop(140);
        auto knobArea = juce::Rectangle<int>(0,0,120,120).withCentre(mid.getCentre());
        amount.setBounds(knobArea);

        r.removeFromTop(8);
        btnApply.setBounds(r.removeFromTop(32).withSizeKeepingCentre(140, 32));
    }

    void paint(juce::Graphics& g) override
    {
        auto b = getLocalBounds().toFloat();
        g.setColour(juce::Colour(0xFF101419));
        g.fillRoundedRectangle(b, 8.0f);
        g.setColour(juce::Colours::white.withAlpha(0.08f));
        g.drawRoundedRectangle(b, 8.0f, 1.0f);

        if (lastAction.isNotEmpty())
        {
            g.setColour(juce::Colours::white.withAlpha(0.7f));
            g.setFont(13.0f);
            g.drawFittedText(lastAction, getLocalBounds().reduced(12).removeFromBottom(40), juce::Justification::centred, 1);
        }
    }

private:
    juce::Label title;
    juce::TextButton btnUpload;
    juce::Label fileLabel;
    juce::ComboBox instruments;
    AmountKnobWithRing amount;
    juce::TextButton btnApply;

    juce::File selectedFile;
    juce::String selectedInstrument { "Vocal" };
    float amountValue { 0.5f };
    juce::String lastAction;
};
