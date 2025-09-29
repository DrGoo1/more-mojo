#pragma once
#include <juce_gui_basics/juce_gui_basics.h>

// A medium-size amount knob with a circumferential coloured meter ring.
// - Uses an internal Slider for interaction
// - Exposes setAmount/getAmount in [0..1]
// - Colour transitions: green -> yellow -> red as amount increases
class AmountKnobWithRing : public juce::Component {
public:
    AmountKnobWithRing()
    {
        addAndMakeVisible(knob);
        knob.setRange(0.0, 1.0, 0.001);
        knob.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
        knob.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
        knob.onValueChange = [this]{ repaint(); if (onAmountChanged) onAmountChanged((float)knob.getValue()); };
    }

    void setAmount(float a) { knob.setValue(juce::jlimit(0.0, 1.0, (double)a), juce::dontSendNotification); repaint(); }
    float getAmount() const { return (float) knob.getValue(); }

    std::function<void(float)> onAmountChanged;

    void resized() override
    {
        auto r = getLocalBounds().reduced(4);
        int d = juce::jmin(r.getWidth(), r.getHeight());
        auto c = juce::Rectangle<int>(0,0,d,d).withCentre(r.getCentre());
        knob.setBounds(c);
    }

    void paint(juce::Graphics& g) override
    {
        auto r = getLocalBounds().toFloat().reduced(4.0f);
        float amount = (float) knob.getValue();

        // Background ring
        g.setColour(juce::Colours::white.withAlpha(0.10f));
        g.drawEllipse(r.reduced(4.0f), 3.0f);

        // Amount arc (green -> yellow -> red)
        auto toColour = [](float t){
            if (t < 0.66f) {
                float u = t / 0.66f; // 0..1
                return juce::Colour::fromFloatRGBA(0.10f + 0.40f*u, 0.85f, 0.20f, 1.0f); // green -> lime
            } else {
                float u = (t - 0.66f) / 0.34f; // 0..1
                return juce::Colour::fromFloatRGBA(0.95f, 0.80f - 0.40f*u, 0.15f, 1.0f); // yellow -> red
            }
        };
        juce::Path arc;
        auto c = r.getCentre();
        float radius = juce::jmin(r.getWidth(), r.getHeight()) * 0.5f - 3.0f;
        float start = juce::MathConstants<float>::pi * 1.2f;
        float span  = juce::MathConstants<float>::pi * 1.2f * amount;
        arc.addCentredArc(c.x, c.y, radius, radius, 0.0f, start, start + span, true);
        g.setColour(toColour(amount));
        g.strokePath(arc, juce::PathStrokeType(5.0f, juce::PathStrokeType::beveled, juce::PathStrokeType::rounded));
    }

private:
    juce::Slider knob;
};
