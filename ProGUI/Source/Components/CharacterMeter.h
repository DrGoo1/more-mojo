#pragma once
#include <juce_gui_basics/juce_gui_basics.h>
#include "PsychedelicStyles.h"

class CharacterMeter : public juce::Component,
                       private juce::Timer {
public:
    CharacterMeter(const juce::String& labelText, const juce::String& emoji)
        : label(labelText), emojiIcon(emoji) {
        setSize(400, 30);
        startTimerHz(30);
    }
    
    void paint(juce::Graphics& g) override {
        auto area = getLocalBounds().toFloat();
        
        // Label
        g.setFont(juce::Font(14.0f, juce::Font::bold));
        g.setColour(juce::Colours::white);
        auto labelArea = area.removeFromLeft(80);
        g.drawText(label, labelArea.toNearestInt(), juce::Justification::centredLeft);
        
        // Meter background
        auto meterArea = area.removeFromLeft(area.getWidth() - 80);
        meterArea = meterArea.reduced(2, 4);
        
        g.setColour(juce::Colours::black.withAlpha(0.3f));
        g.fillRoundedRectangle(meterArea, 4.0f);
        
        // Meter fill with rainbow gradient based on value
        if (currentValue > 0.01f) {
            auto fillArea = meterArea.withWidth(meterArea.getWidth() * currentValue * animationProgress);
            
            juce::Colour fillColor;
            if (currentValue < 0.33f) {
                fillColor = juce::Colour::fromHSV(0.55f + currentValue * 0.15f, 0.8f, 0.9f, 1.0f); // Blue to Cyan
            } else if (currentValue < 0.66f) {
                fillColor = juce::Colour::fromHSV(0.33f - (currentValue - 0.33f) * 0.3f, 0.8f, 0.9f, 1.0f); // Green to Yellow
            } else {
                fillColor = juce::Colour::fromHSV(0.08f - (currentValue - 0.66f) * 0.08f, 0.9f, 0.95f, 1.0f); // Orange to Pink
            }
            
            auto gradient = juce::ColourGradient(fillColor.brighter(0.3f), fillArea.getX(), fillArea.getCentreY(),
                                                  fillColor, fillArea.getRight(), fillArea.getCentreY(), false);
            g.setGradientFill(gradient);
            g.fillRoundedRectangle(fillArea, 4.0f);
            
            // Flowing animation overlay
            auto flowOffset = flowPosition * fillArea.getWidth();
            auto flowGradient = juce::ColourGradient(juce::Colours::white.withAlpha(0.0f), fillArea.getX() + flowOffset - 30, fillArea.getCentreY(),
                                                       juce::Colours::white.withAlpha(0.3f), fillArea.getX() + flowOffset, fillArea.getCentreY(), false);
            flowGradient.addColour(1.0, juce::Colours::white.withAlpha(0.0f));
            g.setGradientFill(flowGradient);
            g.fillRoundedRectangle(fillArea, 4.0f);
        }
        
        // Border
        g.setColour(juce::Colours::white.withAlpha(0.2f));
        g.drawRoundedRectangle(meterArea, 4.0f, 1.0f);
        
        // Percentage text
        g.setColour(juce::Colours::white);
        g.setFont(juce::Font(12.0f, juce::Font::bold));
        auto textArea = meterArea.removeFromRight(50);
        g.drawText(juce::String(static_cast<int>(currentValue * 100)) + "%", textArea.toNearestInt(), juce::Justification::centredRight);
        
        // Emoji icon
        auto emojiArea = area.removeFromRight(30);
        g.setFont(juce::Font(18.0f));
        g.drawText(emojiIcon, emojiArea.toNearestInt(), juce::Justification::centred);
    }
    
    void timerCallback() override {
        // Animate the meter fill
        if (isAnimating && animationProgress < 1.0f) {
            animationProgress += 0.04f;
            if (animationProgress >= 1.0f) {
                animationProgress = 1.0f;
                isAnimating = false;
            }
            repaint();
        }
        
        // Flow animation
        flowPosition += 0.02f;
        if (flowPosition > 1.5f) flowPosition = -0.5f;
        
        if (currentValue > 0.01f) {
            repaint();
        }
    }
    
    void setValue(float newValue) {
        targetValue = juce::jlimit(0.0f, 1.0f, newValue);
        currentValue = targetValue;
        isAnimating = true;
        animationProgress = 0.0f;
        repaint();
    }
    
    float getValue() const { return currentValue; }
    
private:
    juce::String label;
    juce::String emojiIcon;
    float currentValue = 0.0f;
    float targetValue = 0.0f;
    float animationProgress = 0.0f;
    float flowPosition = 0.0f;
    bool isAnimating = false;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(CharacterMeter)
};
