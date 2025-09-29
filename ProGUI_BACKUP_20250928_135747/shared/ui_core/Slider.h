#pragma once
#include "../../JUCE/modules/juce_graphics/juce_graphics.h"
#include "../../JUCE/modules/juce_gui_basics/juce_gui_basics.h"
#include "Theme.h"

namespace ui {
class Slider : public juce::Slider {
public:
    Slider() {
        setSliderStyle (juce::Slider::LinearHorizontal);
        setTextBoxStyle (juce::Slider::TextBoxRight, false, 56, 20);
        setRange (0.0, 100.0, 0.01);
    }
    void paint (juce::Graphics& g) override {
        auto r = getLocalBounds().toFloat().reduced(8.0f, 6.0f);
        auto trackH = juce::jmin(8.0f, r.getHeight());
        auto track = juce::Rectangle<float>(r.getX(), r.getCentreY() - trackH*0.5f, r.getWidth(), trackH);

        // Background track
        auto bg = theme().panel.darker(0.20f);
        g.setColour (bg);
        g.fillRoundedRectangle (track, trackH/2.0f);

        // Value fill with subtle gradient
        auto t = (float) juce::jlimit (0.0, 1.0, proportionOfLengthToValue (getValue()));
        auto fill = track.withWidth (track.getWidth() * t);
        juce::Colour a0 = theme().accent.withAlpha(0.85f);
        juce::Colour a1 = theme().accent2.withAlpha(0.95f);
        g.setGradientFill (juce::ColourGradient (a0, fill.getX(), fill.getY(), a1, fill.getRight(), fill.getBottom(), false));
        g.fillRoundedRectangle (fill, trackH/2.0f);

        // Glow on the filled portion
        g.setColour (a1.withAlpha(0.20f));
        g.drawRoundedRectangle (fill.expanded(1.5f), trackH/2.0f + 1.5f, 2.0f);

        // Thumb (circular)
        auto thumbR = juce::jlimit (8.0f, 12.0f, trackH * 1.2f);
        auto cx = fill.getRight();
        auto cy = track.getCentreY();
        juce::Rectangle<float> thumb (cx - thumbR, cy - thumbR, thumbR*2.0f, thumbR*2.0f);
        // shadow
        g.setColour (juce::Colours::black.withAlpha(0.35f));
        g.fillEllipse (thumb.translated(0, 1.5f));
        // body
        auto topCol = theme().card.brighter(0.20f);
        auto botCol = theme().card.darker(0.15f);
        g.setGradientFill (juce::ColourGradient (topCol, thumb.getX(), thumb.getY(), botCol, thumb.getRight(), thumb.getBottom(), false));
        g.fillEllipse (thumb);
        g.setColour (juce::Colours::black.withAlpha(0.45f));
        g.drawEllipse (thumb, 1.0f);
    }
};
} // namespace ui
