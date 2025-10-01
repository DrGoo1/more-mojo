#pragma once
#include <juce_gui_basics/juce_gui_basics.h>
#include "SkinManager.h"

// Skinned vertical slider using SkinManager filmstrip when available; falls back to vector slider.
class SkinnedSlider : public juce::Component {
public:
    SkinnedSlider()
    {
        addAndMakeVisible(slider);
        slider.setRange(-12.0, 12.0, 0.1);  // dB range
        slider.setValue(0.0);  // Zero position
        slider.setSliderStyle(juce::Slider::LinearVertical);
        slider.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
        slider.onValueChange = [this]{ repaint(); if (onValueChanged) onValueChanged((float)slider.getValue()); };
        
        // Make slider completely invisible - we draw our own graphics
        slider.setColour(juce::Slider::backgroundColourId, juce::Colours::transparentBlack);
        slider.setColour(juce::Slider::trackColourId, juce::Colours::transparentBlack);
        slider.setColour(juce::Slider::thumbColourId, juce::Colours::transparentBlack);
        slider.setColour(juce::Slider::textBoxTextColourId, juce::Colours::transparentBlack);
        slider.setColour(juce::Slider::textBoxBackgroundColourId, juce::Colours::transparentBlack);
        slider.setColour(juce::Slider::textBoxOutlineColourId, juce::Colours::transparentBlack);
    }

    void setValue(float v, juce::NotificationType nt = juce::dontSendNotification) { 
        slider.setValue(juce::jlimit(-12.0, 12.0, (double)v), nt); 
    }
    float getValue() const { return (float) slider.getValue(); }
    
    // Add setRange for compatibility with ProcessSubwindow
    void setRange(double min, double max, double interval = 0.0) { 
        slider.setRange(min, max, interval);
        rangeMin = min;
        rangeMax = max;
    }
    
    double getRangeMin() const { return rangeMin; }
    double getRangeMax() const { return rangeMax; }

    void resized() override
    {
        slider.setBounds(getLocalBounds());
    }

    void paint(juce::Graphics& g) override
    {
        auto r = getLocalBounds().toFloat();
        const auto& skin = SkinManager::instance().skin();
        
        // Convert dB value (-12 to +12) to normalized 0-1 range for frame selection
        float dbValue = (float) slider.getValue();
        float normalizedValue = (dbValue + 12.0f) / 24.0f; // -12dB=0.0, 0dB=0.5, +12dB=1.0
        
        // Priority: vertical slider filmstrip frames first, then vector fallback
        bool drewSlider = false;
        
        if (! skin.sliderVertFrames.empty())
        {
            // Select frame based on normalized value
            int frameIdx = (int)(normalizedValue * (skin.sliderVertFrames.size() - 1));
            frameIdx = juce::jlimit(0, (int)skin.sliderVertFrames.size() - 1, frameIdx);
            
            auto img = skin.sliderVertFrames[frameIdx];
            if (img.isValid())
            {
                // Scale to fit bounds while maintaining aspect ratio
                float scale = juce::jmin(r.getWidth() / (float)img.getWidth(), 
                                       r.getHeight() / (float)img.getHeight());
                auto dest = juce::Rectangle<float>(img.getWidth() * scale, img.getHeight() * scale)
                              .withCentre(r.getCentre());
                g.drawImage(img, dest);
                drewSlider = true;
            }
        }
        
        // Vector fallback if no filmstrip available
        if (!drewSlider)
        {
            // Draw simple vertical slider
            auto trackArea = r.reduced(r.getWidth() * 0.3f, 4.0f);
            
            // Track background
            g.setColour(juce::Colour(0xFF2a2a2a));
            g.fillRoundedRectangle(trackArea, 2.0f);
            
            // Track border
            g.setColour(juce::Colour(0xFF1a1a1a));
            g.drawRoundedRectangle(trackArea, 2.0f, 1.0f);
            
            // Thumb position (0dB at 3/4 from top)
            float thumbY = trackArea.getY() + trackArea.getHeight() * (1.0f - normalizedValue);
            auto thumbArea = juce::Rectangle<float>(r.getWidth() * 0.8f, 8.0f)
                               .withCentre(juce::Point<float>(r.getCentreX(), thumbY));
            
            // Thumb
            g.setColour(juce::Colour(0xFF4a90e2));
            g.fillRoundedRectangle(thumbArea, 2.0f);
            g.setColour(juce::Colour(0xFF6bb6ff));
            g.drawRoundedRectangle(thumbArea, 2.0f, 1.0f);
            
            // Zero line at 3/4 height (0dB position)
            float zeroY = trackArea.getY() + trackArea.getHeight() * 0.5f; // 0dB position
            g.setColour(juce::Colour(0xFF888888));
            g.drawHorizontalLine((int)zeroY, trackArea.getX() - 2, trackArea.getRight() + 2);
        }
    }

    std::function<void(float)> onValueChanged;

private:
    juce::Slider slider;
    double rangeMin = -12.0;
    double rangeMax = 12.0;
};
