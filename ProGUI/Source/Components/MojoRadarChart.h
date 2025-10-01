#pragma once
#include <juce_gui_basics/juce_gui_basics.h>
#include "PsychedelicStyles.h"

class MojoRadarChart : public juce::Component,
                       private juce::Timer {
public:
    MojoRadarChart() {
        // Initialize 8 dimensions
        dimensions = {
            {"Warmth", 0.0f},
            {"Punch", 0.0f},
            {"Space", 0.0f},
            {"Shimmer", 0.0f},
            {"Vintage", 0.0f},
            {"Clarity", 0.0f},
            {"Body", 0.0f},
            {"Vibe", 0.0f}
        };
        
        startTimerHz(30); // Animation timer
    }
    
    void paint(juce::Graphics& g) override {
        auto area = getLocalBounds().toFloat().reduced(10);
        auto center = area.getCentre();
        auto radius = juce::jmin(area.getWidth(), area.getHeight()) / 2.0f;
        
        // Background circles (concentric)
        auto bgGradient = juce::ColourGradient(PsychedelicColors::psychPurple.withAlpha(0.2f),
                                                center.x, center.y,
                                                PsychedelicColors::electricPink.withAlpha(0.1f),
                                                center.x + radius, center.y, true);
        g.setGradientFill(bgGradient);
        
        for (int i = 1; i <= 4; ++i) {
            float r = radius * (i / 4.0f);
            g.drawEllipse(center.x - r, center.y - r, r * 2, r * 2, 1.0f);
        }
        
        // Draw axes
        int numDimensions = dimensions.size();
        float angleStep = juce::MathConstants<float>::twoPi / numDimensions;
        
        g.setColour(PsychedelicColors::skyBlue.withAlpha(0.3f));
        for (int i = 0; i < numDimensions; ++i) {
            float angle = angleStep * i - juce::MathConstants<float>::halfPi;
            auto endPoint = center + juce::Point<float>(std::cos(angle) * radius,
                                                         std::sin(angle) * radius);
            g.drawLine(center.x, center.y, endPoint.x, endPoint.y, 1.0f);
        }
        
        // Draw the Mojo profile shape
        if (isAnalyzed) {
            juce::Path mojoShape;
            bool first = true;
            
            for (int i = 0; i < numDimensions; ++i) {
                float angle = angleStep * i - juce::MathConstants<float>::halfPi;
                float value = dimensions[i].second;
                float r = radius * value * animationProgress;
                auto point = center + juce::Point<float>(std::cos(angle) * r,
                                                          std::sin(angle) * r);
                
                if (first) {
                    mojoShape.startNewSubPath(point);
                    first = false;
                } else {
                    mojoShape.lineTo(point);
                }
            }
            
            mojoShape.closeSubPath();
            
            // Fill with gradient
            auto fillGradient = juce::ColourGradient(PsychedelicColors::groovyGreen.withAlpha(0.4f),
                                                      center.x, center.y,
                                                      PsychedelicColors::sunshineYellow.withAlpha(0.2f),
                                                      center.x + radius, center.y, true);
            g.setGradientFill(fillGradient);
            g.fillPath(mojoShape);
            
            // Stroke with glow
            g.setColour(PsychedelicColors::groovyGreen.brighter());
            g.strokePath(mojoShape, juce::PathStrokeType(2.5f));
            
            // Glow effect
            g.setColour(PsychedelicColors::groovyGreen.withAlpha(0.3f));
            g.strokePath(mojoShape, juce::PathStrokeType(5.0f));
        }
        
        // Draw labels
        g.setFont(juce::Font(12.0f, juce::Font::bold));
        for (int i = 0; i < numDimensions; ++i) {
            float angle = angleStep * i - juce::MathConstants<float>::halfPi;
            float labelRadius = radius + 20;
            auto labelPos = center + juce::Point<float>(std::cos(angle) * labelRadius,
                                                         std::sin(angle) * labelRadius);
            
            g.setColour(PsychedelicColors::sunshineYellow);
            auto textArea = juce::Rectangle<float>(50, 20).withCentre(labelPos);
            g.drawText(dimensions[i].first, textArea.toNearestInt(), juce::Justification::centred);
        }
        
        // Center text
        if (!isAnalyzed) {
            g.setColour(juce::Colours::white.withAlpha(0.5f));
            g.setFont(juce::Font(14.0f));
            g.drawText("Click Analyze\nto extract Mojo", area.toNearestInt(), juce::Justification::centred);
        }
    }
    
    void timerCallback() override {
        if (isAnimating && animationProgress < 1.0f) {
            animationProgress += 0.05f;
            if (animationProgress >= 1.0f) {
                animationProgress = 1.0f;
                isAnimating = false;
            }
            repaint();
        }
    }
    
    void setMojoValues(const std::vector<std::pair<juce::String, float>>& values) {
        if (values.size() == dimensions.size()) {
            dimensions = values;
            isAnalyzed = true;
            isAnimating = true;
            animationProgress = 0.0f;
            repaint();
        }
    }
    
    void reset() {
        for (auto& dim : dimensions) {
            dim.second = 0.0f;
        }
        isAnalyzed = false;
        animationProgress = 0.0f;
        repaint();
    }
    
private:
    std::vector<std::pair<juce::String, float>> dimensions;
    bool isAnalyzed = false;
    bool isAnimating = false;
    float animationProgress = 0.0f;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MojoRadarChart)
};
