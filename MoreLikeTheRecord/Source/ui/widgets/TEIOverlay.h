#pragma once
#include <JuceHeader.h>

/**
 * TEIOverlay - Waveform overlay showing pre/post transients and TEI stats
 * Visualizes the changes in transient edge integrity between original and processed audio
 */
class TEIOverlay : public juce::Component {
public:
    TEIOverlay() = default;
    
    /**
     * Set the waveform data and TEI metrics to display
     * 
     * @param pre Original (pre-processing) audio buffer
     * @param post Processed (post-processing) audio buffer
     * @param riseUs Rise time difference in microseconds
     * @param preDb Pre-ring energy difference in dB
     * @param overPct Overshoot percentage
     */
    void setData(juce::AudioBuffer<float> pre, juce::AudioBuffer<float> post,
                float riseUs, float preDb, float overPct) {
        originalBuffer = std::move(pre);
        processedBuffer = std::move(post);
        riseDeltaUs = riseUs;
        preRingDeltaDb = preDb;
        overshootPct = overPct;
        repaint();
    }
    
    /**
     * Paint the waveform overlay with metrics
     */
    void paint(juce::Graphics& g) override {
        auto bounds = getLocalBounds().reduced(8).toFloat();
        
        // Draw background
        g.fillAll(juce::Colour(24, 24, 32));
        
        // Draw waveform overlays
        drawWaveform(g, originalBuffer, juce::Colours::white.withAlpha(0.7f), bounds);
        drawWaveform(g, processedBuffer, juce::Colour(155, 120, 255), bounds);
        
        // Draw metrics text
        g.setColour(juce::Colours::white);
        g.drawFittedText(
            "ΔRise: " + juce::String(riseDeltaUs, 1) + " µs   " + 
            "ΔPre-ring: " + juce::String(preRingDeltaDb, 2) + " dB   " + 
            "Overshoot: " + juce::String(overshootPct, 1) + "%",
            bounds.toNearestInt(), juce::Justification::bottomLeft, 1
        );
        
        // Draw TEI score color indicator based on metrics
        float score = calculateTEIScore();
        juce::Colour scoreColor = getScoreColor(score);
        g.setColour(scoreColor);
        
        juce::Rectangle<float> scoreBox(bounds.getRight() - 40.0f, bounds.getBottom() - 20.0f, 30.0f, 15.0f);
        g.fillRoundedRectangle(scoreBox, 5.0f);
        
        g.setColour(juce::Colours::black);
        g.drawText(juce::String(static_cast<int>(score * 100)),
                  scoreBox.toNearestInt(), juce::Justification::centred, false);
    }
    
private:
    // Draw a waveform from audio buffer
    void drawWaveform(juce::Graphics& g, const juce::AudioBuffer<float>& buffer,
                     juce::Colour color, juce::Rectangle<float> bounds) {
        if (buffer.getNumSamples() == 0) return;
        
        g.setColour(color);
        
        juce::Path path;
        int numSamples = buffer.getNumSamples();
        
        // Create path for waveform
        path.startNewSubPath(bounds.getX(), bounds.getCentreY());
        
        for (int i = 0; i < numSamples; ++i) {
            float y = bounds.getCentreY() - bounds.getHeight() * 0.4f * buffer.getSample(0, i);
            float x = bounds.getX() + bounds.getWidth() * static_cast<float>(i) / static_cast<float>(numSamples - 1);
            
            if (i == 0) {
                path.startNewSubPath(x, y);
            } else {
                path.lineTo(x, y);
            }
        }
        
        // Draw the path
        g.strokePath(path, juce::PathStrokeType(1.5f));
        
        // Highlight transient area
        int transientIndex = findTransientIndex(buffer);
        if (transientIndex > 0 && transientIndex < buffer.getNumSamples()) {
            float x = bounds.getX() + bounds.getWidth() * static_cast<float>(transientIndex) / static_cast<float>(numSamples - 1);
            g.setColour(color.withAlpha(0.3f));
            g.drawVerticalLine(static_cast<int>(x), bounds.getY(), bounds.getBottom());
        }
    }
    
    // Find the likely transient onset point in a buffer
    int findTransientIndex(const juce::AudioBuffer<float>& buffer) {
        if (buffer.getNumSamples() < 3) return 0;
        
        int maxIndex = 0;
        float maxDiff = 0.0f;
        
        // Simple detection - find maximum sample-to-sample change
        for (int i = 1; i < buffer.getNumSamples(); ++i) {
            float diff = std::abs(buffer.getSample(0, i) - buffer.getSample(0, i - 1));
            if (diff > maxDiff) {
                maxDiff = diff;
                maxIndex = i;
            }
        }
        
        return maxIndex;
    }
    
    // Calculate an overall TEI score (0-1) from the metrics
    float calculateTEIScore() const {
        // Lower values are better for all metrics
        const float riseTimeScore = std::exp(-std::abs(riseDeltaUs) / 500.0f);
        const float preRingScore = std::exp(-std::abs(preRingDeltaDb) / 20.0f);
        const float overshootScore = std::exp(-std::abs(overshootPct) / 20.0f);
        
        return (riseTimeScore * 0.4f + preRingScore * 0.4f + overshootScore * 0.2f);
    }
    
    // Get color based on score (red to green gradient)
    juce::Colour getScoreColor(float score) const {
        // Red to yellow to green
        if (score < 0.5f) {
            // Red to yellow
            return juce::Colour::fromHSV(0.15f * score * 2.0f, 0.9f, 0.9f, 1.0f);
        } else {
            // Yellow to green
            return juce::Colour::fromHSV(0.15f + 0.25f * (score - 0.5f) * 2.0f, 0.9f, 0.9f, 1.0f);
        }
    }
    
    juce::AudioBuffer<float> originalBuffer;
    juce::AudioBuffer<float> processedBuffer;
    float riseDeltaUs{0.0f};
    float preRingDeltaDb{0.0f};
    float overshootPct{0.0f};
};
