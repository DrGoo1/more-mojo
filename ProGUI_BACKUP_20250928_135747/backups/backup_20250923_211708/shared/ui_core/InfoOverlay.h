#pragma once
#include <juce_gui_basics/juce_gui_basics.h>

// Minimal, modern info overlay with dismiss '×' and fade animation.
// Usage:
//   overlay.setTitle("Process Info");
//   overlay.setBodyText("What it does...\nPractical use...\nAudible result...\nBest on...");
//   overlay.show();
class InfoOverlay : public juce::Component, private juce::Timer {
public:
    InfoOverlay()
    {
        setInterceptsMouseClicks(true, true);
        addAndMakeVisible(closeButton);
        closeButton.setButtonText("×");
        closeButton.onClick = [this]{ dismiss(); if (onDismiss) onDismiss(); };

        title.setJustificationType(juce::Justification::centredLeft);
        title.setColour(juce::Label::textColourId, juce::Colours::white);
        addAndMakeVisible(title);

        body.setJustificationType(juce::Justification::topLeft);
        body.setColour(juce::Label::textColourId, juce::Colours::white.withAlpha(0.9f));
        body.setMinimumHorizontalScale(1.0f);
        body.setFont(juce::Font(13.0f));
        addAndMakeVisible(body);

        setVisible(false);
        setAlpha(0.0f);
    }

    void setTitle(const juce::String& t) { title.setText(t, juce::dontSendNotification); repaint(); }
    void setBodyText(const juce::String& t) { body.setText(t, juce::dontSendNotification); repaint(); }

    std::function<void()> onDismiss;

    void show()  { if (! isVisible()) setVisible(true); fadingIn=true;  fadingOut=false; fade=0.0f; setAlpha(fade); startTimerHz(60); }
    void dismiss(){ if (! isVisible()) return;          fadingIn=false; fadingOut=true;  fade=getAlpha();        startTimerHz(60); }

    void paint(juce::Graphics& g) override
    {
        // Dim the background
        auto full = getLocalBounds().toFloat();
        g.setColour(juce::Colours::black.withAlpha(0.45f));
        g.fillRect(full);

        // Panel
        auto p = full.reduced(juce::jmax(24, getWidth()/6), juce::jmax(24, getHeight()/6));
        g.setColour(juce::Colour(0xFF101419));
        g.fillRoundedRectangle(p, 10.0f);
        g.setColour(juce::Colours::white.withAlpha(0.10f));
        g.drawRoundedRectangle(p, 10.0f, 1.0f);
    }

    void resized() override
    {
        auto r = getLocalBounds();
        auto p = r.reduced(juce::jmax(24, getWidth()/6), juce::jmax(24, getHeight()/6));

        auto header = p.removeFromTop(36).reduced(12, 8);
        closeButton.setBounds(p.removeFromTop(0)); // set later
        auto cbW = 28; auto cbH = 24;
        closeButton.setBounds(p.getRight() - cbW - 12, header.getY(), cbW, cbH);
        title.setBounds(header.withTrimmedRight(cbW + 16));

        p.removeFromTop(8);
        body.setBounds(p.reduced(12));
    }

    void mouseUp (const juce::MouseEvent& e) override
    {
        auto full = getLocalBounds();
        auto panel = full.reduced(juce::jmax(24, getWidth()/6), juce::jmax(24, getHeight()/6));
        if (! panel.contains(e.getPosition())) dismiss();
    }

private:
    void timerCallback() override
    {
        const float step = 0.12f;
        if (fadingIn)
        {
            fade = juce::jmin(1.0f, fade + step);
            setAlpha(fade);
            if (fade >= 1.0f) { fadingIn = false; stopTimer(); }
        }
        else if (fadingOut)
        {
            fade = juce::jmax(0.0f, fade - step);
            setAlpha(fade);
            if (fade <= 0.0f) { fadingOut = false; stopTimer(); setVisible(false); }
        }
    }

    juce::TextButton closeButton;
    juce::Label title { {}, "Process Info" };
    juce::Label body  { {}, "" };
    bool fadingIn { false }, fadingOut { false };
    float fade { 0.0f };
};
