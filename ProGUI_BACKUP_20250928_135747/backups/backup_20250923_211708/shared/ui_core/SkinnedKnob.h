#pragma once
#include <juce_gui_basics/juce_gui_basics.h>
#include "SkinManager.h"

// Skinned 3D knob using SkinManager filmstrip when available; falls back to vector knob.
class SkinnedKnob : public juce::Component {
public:
    SkinnedKnob()
    {
        addAndMakeVisible(slider);
        slider.setRange(0.0, 1.0, 0.001);
        slider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
        slider.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
        slider.onValueChange = [this]{ repaint(); if (onValueChanged) onValueChanged((float)slider.getValue()); };
        
        // Make slider completely invisible - we draw our own graphics
        slider.setColour(juce::Slider::rotarySliderFillColourId, juce::Colours::transparentBlack);
        slider.setColour(juce::Slider::rotarySliderOutlineColourId, juce::Colours::transparentBlack);
        slider.setColour(juce::Slider::thumbColourId, juce::Colours::transparentBlack);
        slider.setColour(juce::Slider::trackColourId, juce::Colours::transparentBlack);
        slider.setColour(juce::Slider::backgroundColourId, juce::Colours::transparentBlack);
    }

    void setValue(float v, juce::NotificationType nt = juce::dontSendNotification) { slider.setValue(juce::jlimit(0.0,1.0,(double)v), nt); }
    float getValue() const { return (float) slider.getValue(); }

    void resized() override
    {
        auto r = getLocalBounds();
        int d = juce::jmin(r.getWidth(), r.getHeight());
        slider.setBounds(juce::Rectangle<int>(0,0,d,d).withCentre(r.getCentre()));
    }

    void paint(juce::Graphics& g) override
    {
        auto r = getLocalBounds().toFloat();
        const auto& skin = SkinManager::instance().skin();
        float t = (float) slider.getValue();
        auto c = r.getCentre();

        // Priority: filmstrip frames first, then single knob image, then vector fallback
        bool drewKnob = false;
        
        if (! skin.knobFrames.empty())
        {
            // Draw realistic 3D depression with light bouncing
            float depthRadius = juce::jmin(r.getWidth(), r.getHeight()) * 0.55f;
            auto depthArea = juce::Rectangle<float>(depthRadius * 2, depthRadius * 2).withCentre(c);
            
            // Deep outer shadow (very dark)
            g.setColour(juce::Colour(0xFF000000).withAlpha(0.8f));
            g.fillEllipse(depthArea.expanded(6));
            
            // Medium shadow
            g.setColour(juce::Colour(0xFF222222).withAlpha(0.6f));
            g.fillEllipse(depthArea.expanded(3));
            
            // Inner recessed area (dark gray)
            g.setColour(juce::Colour(0xFF333333));
            g.fillEllipse(depthArea);
            
            // Realistic light reflection - only on upper-left arc (not full ring)
            juce::Path lightArc;
            float startAngle = juce::MathConstants<float>::pi * 1.2f; // upper-left
            float endAngle = juce::MathConstants<float>::pi * 1.8f;   // upper-right
            lightArc.addCentredArc(c.x, c.y, depthRadius - 2, depthRadius - 2, 0, startAngle, endAngle, true);
            g.setColour(juce::Colour(0xFFffffff).withAlpha(0.5f));
            g.strokePath(lightArc, juce::PathStrokeType(3.0f));
            
            // Inner light reflection - smaller arc
            juce::Path innerLightArc;
            innerLightArc.addCentredArc(c.x, c.y, depthRadius - 8, depthRadius - 8, 0, startAngle + 0.2f, endAngle - 0.2f, true);
            g.setColour(juce::Colour(0xFFffffff).withAlpha(0.3f));
            g.strokePath(innerLightArc, juce::PathStrokeType(2.0f));
            
            // Draw Neptune knob (slightly smaller to fit in depression)
            int n = (int) skin.knobFrames.size();
            int idx = juce::jlimit(0, n-1, (int) std::round(t * (n-1)));
            auto img = skin.knobFrames[(size_t) idx];
            float scale = juce::jmin(r.getWidth() / img.getWidth(), r.getHeight() / img.getHeight()) * 0.9f;
            auto dest = juce::Rectangle<float>(img.getWidth() * scale, img.getHeight() * scale).withCentre(c);
            g.drawImage(img, dest);
            drewKnob = true;
        }
        else if (skin.knobImage.isValid())
        {
            auto img = skin.knobImage;
            float scale = juce::jmin(r.getWidth() / img.getWidth(), r.getHeight() / img.getHeight()) * 0.9f;
            auto dest = juce::Rectangle<float>(img.getWidth() * scale, img.getHeight() * scale).withCentre(c);
            g.drawImage(img, dest);
            drewKnob = true;
        }
        
        // Neptune knobs should NEVER use vector fallback - they have filmstrips
        // Vector fallback completely disabled when Neptune skin is loaded
        
        // NEVER draw any overlay on Neptune knobs - they are complete as filmstrips
    }

    std::function<void(float)> onValueChanged;

private:
    juce::Slider slider;
};
