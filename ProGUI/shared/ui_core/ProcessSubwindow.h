#pragma once
#include <juce_gui_basics/juce_gui_basics.h>
#include "InfoOverlay.h"
#include "AmountKnobWithRing.h"

// Generic process subwindow shell (UI-only)
// Header: Title + Info button
// Body: container area exposed via body()
class ProcessSubwindow : public juce::Component {
public:
    explicit ProcessSubwindow(const juce::String& titleText)
    {
        setOpaque(false);
        title.setText(titleText, juce::dontSendNotification);
        title.setJustificationType(juce::Justification::centredLeft);
        title.setColour(juce::Label::textColourId, juce::Colours::white);
        addAndMakeVisible(title);

        infoBtn.setButtonText("i");
        addAndMakeVisible(infoBtn);
        infoBtn.onClick = [this]{ overlay.show(); };
        infoBtn.setColour(juce::TextButton::buttonColourId, juce::Colours::transparentBlack);
        infoBtn.setColour(juce::TextButton::textColourOffId, juce::Colours::white.withAlpha(0.9f));

        addAndMakeVisible(overlay);
        overlay.setVisible(false);
        overlay.onDismiss = [this]{ overlay.setVisible(false); };

        // Body container
        addAndMakeVisible(bodyContainer);

        // Preset dropdown (basic categories)
        preset.addItem("Instrument", 1);
        preset.addItem("Buss", 2);
        preset.addItem("Master", 3);
        preset.addItem("Other", 4);
        preset.setSelectedId(1);
        addAndMakeVisible(preset);
        preset.onChange = [this]{ if (onPresetChanged) onPresetChanged(preset.getText()); };

        // Header amount knob (small)
        addAndMakeVisible(headerAmount);
        headerAmount.onAmountChanged = [this](float a){ if (onHeaderAmountChanged) onHeaderAmountChanged(a); };
    }

    void setInfoText(const juce::String& t)
    {
        overlay.setTitle("Process Info");
        overlay.setBodyText(t);
    }

    void resized() override
    {
        auto r = getLocalBounds().reduced(12);
        auto header = r.removeFromTop(48);
        // Layout header: Title (left), Preset (middle), Amount (right-48), Info button (top-right)
        auto headerInner = header.reduced(4, 4);
        auto right = headerInner.removeFromRight(64);
        infoBtn.setBounds(right.removeFromRight(28).withSizeKeepingCentre(24, 24));
        headerAmount.setBounds(right.withSizeKeepingCentre(44, 44));
        auto mid = headerInner.removeFromRight(240);
        preset.setBounds(mid.reduced(8, 10));
        title.setBounds(headerInner.reduced(4, 8));

        r.removeFromTop(8);
        bodyContainer.setBounds(r);

        overlay.setBounds(getLocalBounds());
    }

    void paint(juce::Graphics& g) override
    {
        auto b = getLocalBounds().toFloat();
        g.setColour(juce::Colour(0xFF101419));
        g.fillRoundedRectangle(b, 8.0f);
        g.setColour(juce::Colours::white.withAlpha(0.08f));
        g.drawRoundedRectangle(b, 8.0f, 1.0f);
    }

private:
    juce::Label title;
    juce::TextButton infoBtn;
    juce::Component bodyContainer;
    InfoOverlay overlay;
    juce::ComboBox preset;
    AmountKnobWithRing headerAmount;

public:
    juce::Component& body() noexcept { return bodyContainer; }
    void setHeaderAmount(float a) { headerAmount.setAmount(a); }
    float getHeaderAmount() const { return headerAmount.getAmount(); }
    juce::String getPresetText() const { return preset.getText(); }
    void setPresetByName(const juce::String& t) { preset.setText(t, juce::dontSendNotification); }
    std::function<void(const juce::String&)> onPresetChanged;
    std::function<void(float)> onHeaderAmountChanged;
};
