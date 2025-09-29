#pragma once
#include "../../JUCE/modules/juce_graphics/juce_graphics.h"
#include "../../JUCE/modules/juce_gui_basics/juce_gui_basics.h"
#include "Theme.h"

namespace ui {
class Knob : public juce::Slider {
public:
    Knob() {
        setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
        setTextBoxStyle (juce::Slider::TextBoxBelow, false, 56, 20);
        setRange (0.0, 100.0, 0.1);
    }
    void paint (juce::Graphics& g) override {
        auto r = getLocalBounds().toFloat().reduced (6);
        auto rad = juce::jmin (r.getWidth(), r.getHeight()) * 0.5f;
        auto c = r.getCentre();

        auto body = theme().panel;
        g.setGradientFill (juce::ColourGradient (body.brighter(0.12f), c.x - rad*0.4f, c.y - rad*0.4f,
                                                 body.darker(0.15f),   c.x + rad*0.6f, c.y + rad*0.6f, true));
        g.fillEllipse (r);
        g.setColour (juce::Colours::black.withAlpha(0.35f));
        g.drawEllipse (r, 2.0f);

        auto sa = juce::MathConstants<float>::pi * 1.2f;
        auto ea = juce::MathConstants<float>::pi * 2.8f;
        auto ang = sa + (float)proportionOfLengthToValue (getValue()) * (ea - sa);
        juce::Path arc; arc.addCentredArc (c.x, c.y, rad-6, rad-6, 0, sa, ang, true);
        g.setColour (theme().accent);
        g.strokePath (arc, juce::PathStrokeType (3.0f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

        juce::Path needle; needle.addRoundedRectangle (-2.f, -rad+10.f, 4.f, rad*0.55f, 1.5f);
        g.setColour (juce::Colours::white.withAlpha(0.9f));
        g.fillPath (needle, juce::AffineTransform::rotation(ang).translated (c.x, c.y));
    }
};
} // namespace ui
