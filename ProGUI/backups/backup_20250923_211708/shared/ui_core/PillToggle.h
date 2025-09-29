#pragma once
#include "../../JUCE/modules/juce_graphics/juce_graphics.h"
#include "../../JUCE/modules/juce_gui_basics/juce_gui_basics.h"
#include "Theme.h"

namespace ui {
class PillToggle : public juce::ToggleButton {
public:
    PillToggle() { setClickingTogglesState(true); }

    void paintButton(juce::Graphics& g, bool over, bool down) override {
        auto r = getLocalBounds().toFloat().reduced(2.0f, 4.0f);
        const float radius = r.getHeight() * 0.5f;
        const bool on = getToggleState();

        // Background
        juce::Colour base = on ? theme().accent.withAlpha(0.9f) : theme().panel.darker(0.15f);
        if (down) base = base.darker(0.12f); else if (over) base = base.brighter(0.06f);
        g.setColour(base);
        g.fillRoundedRectangle(r, radius);

        // Border glow
        g.setColour(juce::Colours::white.withAlpha(0.08f));
        g.drawRoundedRectangle(r, radius, 1.5f);

        // Knob
        float knobD = r.getHeight() - 6.0f;
        float knobX = on ? (r.getRight() - knobD - 3.0f) : (r.getX() + 3.0f);
        auto knob = juce::Rectangle<float>(knobX, r.getY() + 3.0f, knobD, knobD);
        // shadow
        g.setColour(juce::Colours::black.withAlpha(0.35f));
        g.fillEllipse(knob.translated(0, 1.0f));
        // body gradient
        auto topCol = theme().card.brighter(0.20f);
        auto botCol = theme().card.darker(0.15f);
        g.setGradientFill(juce::ColourGradient(topCol, knob.getX(), knob.getY(), botCol, knob.getRight(), knob.getBottom(), false));
        g.fillEllipse(knob);
        g.setColour(juce::Colours::black.withAlpha(0.45f));
        g.drawEllipse(knob, 1.0f);

        // Text
        g.setColour(theme().text.withAlpha(on ? 1.0f : 0.8f));
        g.setFont(theme().body);
        g.drawFittedText(getButtonText(), getLocalBounds().reduced(10, 0), juce::Justification::centred, 1);
    }
};
} // namespace ui
