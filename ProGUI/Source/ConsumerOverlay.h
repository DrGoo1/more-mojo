#pragma once

#include <juce_core/juce_core.h>
#include <juce_events/juce_events.h>
#include <juce_graphics/juce_graphics.h>
#include <juce_gui_basics/juce_gui_basics.h>

// Simple consumer-style overlay with translucent chrome and placeholder meters
class ConsumerOverlay : public juce::Component
{
public:
    ConsumerOverlay() {}
    ~ConsumerOverlay() override = default;

    void paint(juce::Graphics& g) override
    {
        auto b = getLocalBounds().toFloat();
        // semi-transparent overlay
        g.setColour(juce::Colours::black.withAlpha(0.25f));
        g.fillRoundedRectangle(b, 8.0f);

        // header bar
        g.setColour(juce::Colours::white.withAlpha(0.08f));
        g.fillRect(b.removeFromTop(36.0f));

        // decorative left/right meters placeholder
        float meterWidth = 12.0f;
        float gap = 6.0f;
        auto h = (float)getHeight();
        auto x0 = 10.0f;
        g.setColour(juce::Colours::limegreen.withAlpha(0.7f));
        g.fillRect(x0, h * (1.0f - leftLevel), meterWidth, h * leftLevel);
        g.setColour(juce::Colours::deepskyblue.withAlpha(0.7f));
        g.fillRect(x0 + meterWidth + gap, h * (1.0f - rightLevel), meterWidth, h * rightLevel);

        // border
        g.setColour(juce::Colours::white.withAlpha(0.15f));
        g.drawRoundedRectangle(getLocalBounds().toFloat().reduced(0.5f), 8.0f, 1.0f);
    }

    void setLevels(float l, float r)
    {
        leftLevel = juce::jlimit(0.0f, 1.0f, l);
        rightLevel = juce::jlimit(0.0f, 1.0f, r);
        repaint();
    }

private:
    float leftLevel = 0.0f;
    float rightLevel = 0.0f;
};
