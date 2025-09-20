#pragma once
#include <JuceHeader.h>
#include "../LookAndFeel_Mojo.h"

namespace mojo {

/**
 * Base class for smooth-animated meters with 60 FPS updates
 */
class SmoothMeterBase : public juce::Component, private juce::Timer {
public:
    SmoothMeterBase() {
        startTimerHz(60); // 60 FPS animation
    }
    
    /** Set target value (0.0 to 1.0) with smoothing */
    void setTarget(float v) {
        target = juce::jlimit(0.f, 1.f, v);
    }
    
protected:
    void timerCallback() override {
        // Smooth the value with 250ms-ish response time (at 60 FPS)
        value += 0.25f * (target - value);
        repaint();
    }
    
    float value = 0.f;
    float target = 0.f;
};

/**
 * LUFS/Level meter with stylish appearance
 */
class LUFSMeter : public SmoothMeterBase {
public:
    void paint(juce::Graphics& g) override {
        auto bounds = getLocalBounds().toFloat();
        LookAndFeel_Mojo::drawCard(g, bounds, juce::Colour(30, 30, 40), 8.f, 6.f);
        
        // Draw meter bar
        auto bar = bounds.reduced(8.f);
        auto height = bar.getHeight() * value;
        juce::Rectangle<float> filled(bar.getX(), bar.getBottom() - height, 
                                     bar.getWidth(), height);
        
        // Use purple-ish color for LUFS meter
        g.setColour(juce::Colour(155, 120, 255));
        g.fillRoundedRectangle(filled, 6.f);
        
        // Draw label
        g.setColour(juce::Colours::white.withAlpha(0.8f));
        g.drawFittedText("LUFS", getLocalBounds().reduced(6), 
                        juce::Justification::bottomRight, 1);
    }
};

/**
 * Correlation meter with dial-style visualization
 */
class CorrelationMeter : public SmoothMeterBase {
public:
    void paint(juce::Graphics& g) override {
        auto bounds = getLocalBounds().toFloat();
        LookAndFeel_Mojo::drawCard(g, bounds, juce::Colour(30, 30, 40), 8.f, 6.f);
        
        // Draw correlation meter as a dial
        auto cx = bounds.getCentreX();
        auto cy = bounds.getCentreY();
        auto radius = juce::jmin(bounds.getWidth(), bounds.getHeight()) * 0.35f;
        
        // Draw meter background circle
        g.setColour(juce::Colours::white.withAlpha(0.08f));
        g.drawEllipse({cx - radius, cy - radius, radius * 2, radius * 2}, 1.5f);
        
        // Draw the needle indicating correlation
        float angle = juce::jmap(value, 0.f, 1.f, 
                               -juce::MathConstants<float>::halfPi, 
                               juce::MathConstants<float>::halfPi);
        
        juce::Path needle;
        needle.addRectangle(-2.f, -radius * 0.85f, 4.f, radius * 0.7f);
        
        g.setColour(juce::Colours::white);
        g.fillPath(needle, juce::AffineTransform::rotation(angle).translated(cx, cy));
        
        // Draw label
        g.setColour(juce::Colours::white.withAlpha(0.8f));
        g.drawFittedText("Correlation", getLocalBounds().reduced(6), 
                        juce::Justification::bottomRight, 1);
    }
};

/**
 * Placeholder for the TEI Table component
 */
class TEITable : public juce::Component {
public:
    void paint(juce::Graphics& g) override {
        LookAndFeel_Mojo::drawCard(g, getLocalBounds().toFloat(), juce::Colour(30, 30, 40));
        g.setColour(juce::Colours::white);
        g.drawFittedText("TEI Metrics (ΔRise, Pre-ring, Overshoot)", 
                        getLocalBounds(), juce::Justification::centred, 1);
    }
};

/**
 * Placeholder for the Residual Spectrum meter
 */
class ResidualSpectrumMeter : public juce::Component {
public:
    void paint(juce::Graphics& g) override {
        LookAndFeel_Mojo::drawCard(g, getLocalBounds().toFloat(), juce::Colour(30, 30, 40));
        g.setColour(juce::Colours::white);
        g.drawFittedText("Residual/Null Spectrum", 
                        getLocalBounds(), juce::Justification::centred, 1);
    }
};

/**
 * Placeholder for the Heatmap meter (will be replaced with OpenGL version)
 */
class HeatmapMeter : public juce::Component {
public:
    void paint(juce::Graphics& g) override {
        LookAndFeel_Mojo::drawCard(g, getLocalBounds().toFloat(), juce::Colour(30, 30, 40));
        g.setColour(juce::Colours::white);
        g.drawFittedText("Group Delay Spread (GDS) Heatmap", 
                        getLocalBounds(), juce::Justification::centred, 1);
    }
};

}  // namespace mojo
