#pragma once
#include <JuceHeader.h>
#include "../LookAndFeel_Mojo.h"

namespace moremojo {
namespace mlar {
namespace mojoUI {

/**
 * BaseMeter - Base class for all meter components
 * 
 * Features:
 * - Smooth value transitions
 * - Customizable title
 * - Modern styling
 */
class BaseMeter : public juce::Component,
                 private juce::Timer {
public:
    BaseMeter() {
        // Set up title label
        addAndMakeVisible(titleLabel);
        titleLabel.setJustificationType(juce::Justification::centred);
        titleLabel.setFont(juce::Font(14.0f, juce::Font::bold));
        
        // Start update timer
        startTimerHz(30);
    }
    
    ~BaseMeter() override {
        stopTimer();
    }
    
    /**
     * Set the target value for the meter
     * 
     * @param newValue New target value (0.0-1.0)
     */
    void setValue(float newValue) {
        targetValue = juce::jlimit(0.0f, 1.0f, newValue);
    }
    
    /**
     * Get the current display value
     */
    float getValue() const {
        return currentValue;
    }
    
    /**
     * Set the meter title
     * 
     * @param title New meter title
     */
    void setTitle(const juce::String& title) {
        titleLabel.setText(title, juce::dontSendNotification);
    }
    
    void resized() override {
        // Position title label at top
        titleLabel.setBounds(getLocalBounds().removeFromTop(20));
    }
    
protected:
    float targetValue = 0.0f;
    float currentValue = 0.0f;
    
    juce::Label titleLabel;
    
private:
    void timerCallback() override {
        // Smooth value transition
        if (std::abs(currentValue - targetValue) > 0.001f) {
            currentValue = currentValue * 0.8f + targetValue * 0.2f;
            repaint();
        }
    }
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(BaseMeter)
};

/**
 * LUFSMeter - Loudness meter with colored segments
 */
class LUFSMeter : public BaseMeter {
public:
    LUFSMeter() {
        setTitle("LUFS");
    }
    
    void paint(juce::Graphics& g) override {
        auto bounds = getLocalBounds();
        bounds.removeFromTop(24); // Remove space for title
        
        // Draw background
        g.setColour(juce::Colour(32, 32, 38));
        g.fillRoundedRectangle(bounds.toFloat(), 4.0f);
        
        // Draw meter segments
        const int numSegments = 30;
        const float segmentWidth = bounds.getWidth() / static_cast<float>(numSegments);
        
        for (int i = 0; i < numSegments; ++i) {
            // Calculate segment position and color
            const float x = bounds.getX() + i * segmentWidth;
            const float segmentHeight = bounds.getHeight() * 0.7f;
            const float y = bounds.getCentreY() - segmentHeight * 0.5f;
            
            const float segmentValue = static_cast<float>(i) / numSegments;
            const bool isActive = segmentValue <= currentValue;
            
            // Get color based on position
            juce::Colour segmentColor;
            if (segmentValue < 0.7f) {
                segmentColor = juce::Colour(80, 220, 100); // Green
            } else if (segmentValue < 0.9f) {
                segmentColor = juce::Colour(240, 220, 40); // Yellow
            } else {
                segmentColor = juce::Colour(240, 60, 60);  // Red
            }
            
            // Draw segment
            g.setColour(isActive ? segmentColor : segmentColor.withAlpha(0.2f));
            g.fillRoundedRectangle(x + 2.0f, y, segmentWidth - 4.0f, segmentHeight, 2.0f);
        }
        
        // Draw current value text
        const float displayDb = -60.0f + currentValue * 60.0f; // Map 0-1 to -60dB to 0dB
        
        g.setColour(juce::Colours::white);
        g.setFont(juce::Font(14.0f, juce::Font::bold));
        g.drawText(juce::String(displayDb, 1) + " dB", 
                  bounds, juce::Justification::centred, true);
    }
};

/**
 * PhaseCorrelationMeter - Stereo correlation meter
 */
class PhaseCorrelationMeter : public BaseMeter {
public:
    PhaseCorrelationMeter() {
        setTitle("Phase");
    }
    
    void paint(juce::Graphics& g) override {
        auto bounds = getLocalBounds();
        bounds.removeFromTop(24); // Remove space for title
        
        // Draw background
        g.setColour(juce::Colour(32, 32, 38));
        g.fillRoundedRectangle(bounds.toFloat(), 4.0f);
        
        // Draw scale markings
        g.setColour(juce::Colours::white.withAlpha(0.3f));
        const float centerX = bounds.getCentreX();
        const float markingsY = bounds.getCentreY() + bounds.getHeight() * 0.25f;
        
        // Draw center line
        g.drawVerticalLine(centerX, bounds.getY() + 5.0f, markingsY);
        
        // Draw -1, 0, +1 markings
        g.setFont(juce::Font(10.0f));
        g.drawText("-1", bounds.getX() + 5, static_cast<int>(markingsY), 20, 15, 
                  juce::Justification::left, true);
        g.drawText("0", static_cast<int>(centerX - 5), static_cast<int>(markingsY), 10, 15, 
                  juce::Justification::centred, true);
        g.drawText("+1", bounds.getRight() - 25, static_cast<int>(markingsY), 20, 15, 
                  juce::Justification::right, true);
        
        // Draw meter needle
        // Map 0-1 to -1 to +1 for display
        const float corrValue = currentValue * 2.0f - 1.0f;
        const float needleX = centerX + corrValue * (centerX - 20.0f);
        
        juce::Path needle;
        needle.startNewSubPath(needleX, bounds.getY() + 10.0f);
        needle.lineTo(needleX - 5.0f, bounds.getCentreY() - 5.0f);
        needle.lineTo(needleX + 5.0f, bounds.getCentreY() - 5.0f);
        needle.closeSubPath();
        
        // Color based on correlation value
        juce::Colour needleColor;
        if (corrValue < -0.5f) {
            needleColor = juce::Colour(240, 60, 60);  // Red
        } else if (corrValue < 0.5f) {
            needleColor = juce::Colour(240, 220, 40); // Yellow
        } else {
            needleColor = juce::Colour(80, 220, 100); // Green
        }
        
        g.setColour(needleColor);
        g.fillPath(needle);
        
        // Draw current value text
        g.setColour(juce::Colours::white);
        g.setFont(juce::Font(14.0f, juce::Font::bold));
        g.drawText(juce::String(corrValue, 2),
                  bounds, juce::Justification::centred, true);
    }
};

/**
 * ConfidenceMeter - Simple circular confidence meter
 */
class ConfidenceMeter : public BaseMeter {
public:
    ConfidenceMeter() {
        setTitle("Confidence");
    }
    
    void paint(juce::Graphics& g) override {
        auto bounds = getLocalBounds();
        bounds.removeFromTop(24); // Remove space for title
        
        // Draw background
        g.setColour(juce::Colour(32, 32, 38));
        g.fillRoundedRectangle(bounds.toFloat(), 4.0f);
        
        // Calculate circle size and position
        float circleDiameter = juce::jmin(bounds.getWidth(), bounds.getHeight()) * 0.7f;
        juce::Rectangle<float> circleArea = bounds.withSizeKeepingCentre(
            circleDiameter, circleDiameter).toFloat();
        
        // Draw background circle
        g.setColour(juce::Colours::darkgrey.withAlpha(0.2f));
        g.fillEllipse(circleArea);
        
        // Draw confidence arc
        const float startAngle = -juce::MathConstants<float>::halfPi;
        const float endAngle = startAngle + juce::MathConstants<float>::twoPi * currentValue;
        
        // Get color based on confidence value
        juce::Colour fillColor;
        if (currentValue < 0.3f) {
            fillColor = juce::Colour(240, 60, 60);  // Red
        } else if (currentValue < 0.7f) {
            fillColor = juce::Colour(240, 220, 40); // Yellow
        } else {
            fillColor = juce::Colour(80, 220, 100); // Green
        }
        
        // Fill arc
        juce::Path arc;
        arc.addPieSegment(circleArea, startAngle, endAngle, 0.0f);
        g.setColour(fillColor);
        g.fillPath(arc);
        
        // Draw percentage text
        g.setColour(juce::Colours::white);
        g.setFont(juce::Font(18.0f, juce::Font::bold));
        g.drawText(juce::String(static_cast<int>(currentValue * 100)) + "%", 
                  bounds, juce::Justification::centred, true);
    }
};

/**
 * TruePeakMeter - Specialized meter for true-peak levels
 */
class TruePeakMeter : public BaseMeter {
public:
    TruePeakMeter() {
        setTitle("True Peak");
    }
    
    void paint(juce::Graphics& g) override {
        auto bounds = getLocalBounds();
        bounds.removeFromTop(24); // Remove space for title
        
        // Draw background
        g.setColour(juce::Colour(32, 32, 38));
        g.fillRoundedRectangle(bounds.toFloat(), 4.0f);
        
        // Draw peak bar
        const float peakWidth = bounds.getWidth() * currentValue;
        juce::Rectangle<float> peakBar(
            bounds.getX(), bounds.getY() + 5.0f,
            peakWidth, bounds.getHeight() - 10.0f);
            
        // Color based on level
        juce::Colour barColor;
        if (currentValue > 0.95f) {
            barColor = juce::Colour(240, 60, 60);  // Red
        } else if (currentValue > 0.8f) {
            barColor = juce::Colour(240, 220, 40); // Yellow
        } else {
            barColor = juce::Colour(80, 220, 100); // Green
        }
        
        g.setColour(barColor);
        g.fillRoundedRectangle(peakBar, 3.0f);
        
        // Draw peak level text
        g.setColour(juce::Colours::white);
        g.setFont(juce::Font(14.0f, juce::Font::bold));
        
        // Format as dBTP
        const float dbTP = currentValue > 0.0f 
                         ? 20.0f * std::log10(currentValue) 
                         : -60.0f;
        
        g.drawText(juce::String(dbTP, 1) + " dBTP", 
                  bounds, juce::Justification::centred, true);
                  
        // Draw ceiling marker at -0.3 dBTP
        const float ceilingX = bounds.getX() + bounds.getWidth() * 0.965f;
        g.setColour(juce::Colours::red);
        g.drawVerticalLine(ceilingX, bounds.getY() + 2.0f, bounds.getBottom() - 2.0f);
        
        g.setFont(juce::Font(10.0f));
        g.drawText("-0.3", 
                  static_cast<int>(ceilingX - 15), bounds.getY() + 2, 
                  30, 12, juce::Justification::centred, true);
    }
};

/**
 * GainReductionMeter - Meter showing gain reduction
 */
class GainReductionMeter : public BaseMeter {
public:
    GainReductionMeter() {
        setTitle("Gain Reduction");
    }
    
    void paint(juce::Graphics& g) override {
        auto bounds = getLocalBounds();
        bounds.removeFromTop(24); // Remove space for title
        
        // Draw background
        g.setColour(juce::Colour(32, 32, 38));
        g.fillRoundedRectangle(bounds.toFloat(), 4.0f);
        
        // Draw reduction bar
        const float reductionWidth = bounds.getWidth() * currentValue;
        juce::Rectangle<float> reductionBar(
            bounds.getX(), bounds.getY() + 5.0f,
            reductionWidth, bounds.getHeight() - 10.0f);
            
        // Color based on reduction amount
        juce::Colour barColor = juce::Colour(240, 60, 60).withAlpha(0.8f);  // Red
        
        g.setColour(barColor);
        g.fillRoundedRectangle(reductionBar, 3.0f);
        
        // Draw reduction text in dB
        g.setColour(juce::Colours::white);
        g.setFont(juce::Font(14.0f, juce::Font::bold));
        
        // Map 0-1 to 0 to -12 dB
        const float dBReduction = -12.0f * currentValue;
        
        g.drawText(juce::String(dBReduction, 1) + " dB", 
                  bounds, juce::Justification::centred, true);
    }
};

/**
 * TransientActivityMeter - Meter showing transient detection activity
 */
class TransientActivityMeter : public BaseMeter {
public:
    TransientActivityMeter() {
        setTitle("Transient Activity");
    }
    
    void paint(juce::Graphics& g) override {
        auto bounds = getLocalBounds();
        bounds.removeFromTop(24); // Remove space for title
        
        // Draw background
        g.setColour(juce::Colour(32, 32, 38));
        g.fillRoundedRectangle(bounds.toFloat(), 4.0f);
        
        // Draw activity dots
        const int numDots = 20;
        const float dotDiameter = (bounds.getWidth() - 20.0f) / numDots - 2.0f;
        
        for (int i = 0; i < numDots; ++i) {
            // Calculate position
            const float x = bounds.getX() + 10.0f + i * (dotDiameter + 2.0f);
            const float y = bounds.getCentreY();
            
            // Calculate activation level for this dot
            const float dotThreshold = static_cast<float>(i) / numDots;
            const float dotActivity = currentValue > dotThreshold ? 
                                    std::min(1.0f, (currentValue - dotThreshold) * numDots) : 
                                    0.0f;
            
            // Get color based on position
            juce::Colour dotColor;
            if (i < numDots * 0.5f) {
                dotColor = juce::Colour(80, 220, 100); // Green for low activity
            } else if (i < numDots * 0.8f) {
                dotColor = juce::Colour(240, 220, 40); // Yellow for medium
            } else {
                dotColor = juce::Colour(240, 60, 60);  // Red for high
            }
            
            // Draw dot
            g.setColour(dotColor.withAlpha(dotActivity * 0.9f + 0.1f));
            g.fillEllipse(x, y - dotDiameter * 0.5f, dotDiameter, dotDiameter);
        }
        
        // Draw activity text
        g.setColour(juce::Colours::white);
        g.setFont(juce::Font(14.0f, juce::Font::bold));
        
        // Convert to percentage
        const int activityPercent = static_cast<int>(currentValue * 100.0f);
        
        g.drawText(juce::String(activityPercent) + "%", 
                  bounds, juce::Justification::centred, true);
    }
};

}}} // namespace moremojo::mlar::mojoUI
