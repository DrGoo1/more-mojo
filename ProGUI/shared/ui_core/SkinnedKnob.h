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
    
    // Add setRange for compatibility with ProcessSubwindow (maps to internal 0-1 slider)
    void setRange(double min, double max, double interval = 0.0) { 
        rangeMin = min; 
        rangeMax = max; 
    }
    
    double getRangeMin() const { return rangeMin; }
    double getRangeMax() const { return rangeMax; }

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
            // Draw Neptune knob PLAIN (no depression) with colorful center tinting
            int n = (int) skin.knobFrames.size();
            int idx = juce::jlimit(0, n-1, (int) std::round(t * (n-1)));
            auto img = skin.knobFrames[(size_t) idx];
            float scale = juce::jmin(r.getWidth() / img.getWidth(), r.getHeight() / img.getHeight()) * 1.0f;
            auto dest = juce::Rectangle<float>(img.getWidth() * scale, img.getHeight() * scale).withCentre(c);
            
            // Draw the base knob
            g.drawImage(img, dest);
            
            // Apply colorful tint to the center area
            g.setColour(juce::Colour(0xFFFF6B35).withAlpha(0.3f)); // Bright orange tint
            auto centerRadius = dest.getWidth() * 0.25f; // 25% of knob size
            juce::ColourGradient centerGlow(
                juce::Colour(0xFFFFD166).withAlpha(0.5f), // Bright yellow center
                c.x, c.y,
                juce::Colour(0xFFFF6B35).withAlpha(0.2f), // Orange fade out
                c.x + centerRadius, c.y + centerRadius,
                true
            );
            g.setGradientFill(centerGlow);
            g.fillEllipse(c.x - centerRadius, c.y - centerRadius, centerRadius * 2, centerRadius * 2);
            
            // Add a bright highlight in the very center
            auto highlightRadius = centerRadius * 0.3f;
            g.setColour(juce::Colour(0xFFFFFFFF).withAlpha(0.4f));
            g.fillEllipse(c.x - highlightRadius, c.y - highlightRadius, highlightRadius * 2, highlightRadius * 2);
            
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
    double rangeMin = 0.0;
    double rangeMax = 1.0;
};
