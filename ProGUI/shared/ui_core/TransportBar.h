#pragma once
#include <juce_gui_basics/juce_gui_basics.h>

class TransportBar : public juce::Component {
public:
    TransportBar() {
        addAndMakeVisible(btnPlay);
        addAndMakeVisible(btnPause);
        addAndMakeVisible(btnStop);
        btnPlay.setButtonText(">");
        btnPause.setButtonText("||");
        btnStop.setButtonText("[]");
        for (auto* b : { &btnPlay, &btnPause, &btnStop }) {
            b->setColour(juce::TextButton::buttonColourId, juce::Colour(0xFF4a90e2));
            b->setColour(juce::TextButton::buttonOnColourId, juce::Colour(0xFF2171b5));
            b->setColour(juce::TextButton::textColourOffId, juce::Colours::white);
            b->setColour(juce::TextButton::textColourOnId, juce::Colours::white);
        }
    }

    std::function<void()> onPlay;
    std::function<void()> onPause;
    std::function<void()> onStop;

    void resized() override {
        auto r = getLocalBounds().reduced(8, 4);
        const int gap = 8;
        const int w = 50; // wider buttons for text
        const int h = r.getHeight();
        
        // Calculate total width needed for 3 buttons
        int totalWidth = w * 3 + gap * 2;
        int startX = r.getX() + (r.getWidth() - totalWidth) / 2;
        
        btnPlay.setBounds(startX, r.getY(), w, h);
        btnPause.setBounds(startX + w + gap, r.getY(), w, h);
        btnStop.setBounds(startX + (w + gap) * 2, r.getY(), w, h);
    }

    void paint(juce::Graphics& g) override {
        auto r = getLocalBounds().toFloat();
        // Subtle transport background
        g.setColour(juce::Colours::white.withAlpha(0.06f));
        g.fillRoundedRectangle(r, 6.0f);
        g.setColour(juce::Colours::white.withAlpha(0.12f));
        g.drawRoundedRectangle(r, 6.0f, 1.0f);
    }

private:
    juce::TextButton btnPlay, btnPause, btnStop;
};
