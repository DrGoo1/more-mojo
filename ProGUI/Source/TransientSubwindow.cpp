#include "TransientSubwindow.h"

TransientSubwindow::TransientSubwindow()
    : ProcessSubwindow("TRANSIENT SHAPING", "Adds punch & presence while preventing smearing of attacks")
{
}

void TransientSubwindow::createControls()
{
    // Sensitivity and timing controls
    sensitivityKnob = createKnob("Sensitivity", 0.0f, 100.0f, 50.0f);
    attackKnob = createKnob("Attack", 0.1f, 100.0f, 1.0f);
    holdKnob = createKnob("Hold", 0.1f, 1000.0f, 10.0f);
    releaseKnob = createKnob("Release", 1.0f, 5000.0f, 100.0f);
    
    // Shaping controls
    boostSoftenKnob = createKnob("Boost/Soften", -100.0f, 100.0f, 0.0f);
    hfFocusKnob = createKnob("HF Focus", 0.0f, 100.0f, 30.0f);
    
    // Band mode
    juce::StringArray bandModes = {"Full Band", "Split Band"};
    bandModeCombo = createComboBox("Band Mode", bandModes);
}

void TransientSubwindow::layoutControls(juce::Rectangle<int> controlArea)
{
    // Top row: Timing controls
    auto topRow = controlArea.removeFromTop(100);
    auto timingWidth = topRow.getWidth() / 4;
    
    sensitivityKnob->setBounds(topRow.removeFromLeft(timingWidth).reduced(10));
    attackKnob->setBounds(topRow.removeFromLeft(timingWidth).reduced(10));
    holdKnob->setBounds(topRow.removeFromLeft(timingWidth).reduced(10));
    releaseKnob->setBounds(topRow.reduced(10));
    
    controlArea.removeFromTop(20); // Gap
    
    // Bottom row: Shaping controls
    auto bottomRow = controlArea;
    auto shapingWidth = bottomRow.getWidth() / 3;
    
    boostSoftenKnob->setBounds(bottomRow.removeFromLeft(shapingWidth).reduced(10));
    hfFocusKnob->setBounds(bottomRow.removeFromLeft(shapingWidth).reduced(10));
    bandModeCombo->setBounds(bottomRow.reduced(10));
}

void TransientSubwindow::paintMeters(juce::Graphics& g, juce::Rectangle<int> meterArea)
{
    // Meter background
    g.setColour(juce::Colour(0xFF0f3460));
    g.fillRoundedRectangle(meterArea.toFloat().reduced(5), 6.0f);
    
    auto area = meterArea.reduced(10);
    
    // Transient Timeline
    auto timelineArea = area.removeFromTop(area.getHeight() / 3);
    g.setColour(juce::Colours::white);
    g.setFont(juce::Font("Arial", 9.0f, juce::Font::bold));
    g.drawText("TRANSIENT", timelineArea.removeFromTop(12), juce::Justification::centred);
    g.drawText("TIMELINE", timelineArea.removeFromTop(12), juce::Justification::centred);
    
    // Draw transient activity over time
    g.setColour(juce::Colour(0xFF333333));
    g.fillRoundedRectangle(timelineArea.toFloat().reduced(2), 2.0f);
    
    // Simulate transient detection spikes
    int numSpikes = 15;
    int spikeWidth = timelineArea.getWidth() / numSpikes;
    
    for (int i = 0; i < numSpikes; ++i)
    {
        auto spikeArea = timelineArea.removeFromLeft(spikeWidth);
        // Random spike heights to simulate transient activity
        float height = (i % 4 == 0) ? 0.8f : ((i % 7 == 0) ? 0.6f : 0.2f);
        auto activeSpike = spikeArea.removeFromBottom((int)(spikeArea.getHeight() * height));
        
        g.setColour(juce::Colour(0xFFff4444));
        g.fillRect(activeSpike.reduced(1));
    }
    
    area.removeFromTop(15); // Gap
    
    // Before/After Waveform comparison
    auto waveformArea = area.removeFromTop(area.getHeight() / 2);
    g.setColour(juce::Colours::white);
    g.drawText("BEFORE/AFTER", waveformArea.removeFromTop(15), juce::Justification::centred);
    
    auto beforeArea = waveformArea.removeFromTop(waveformArea.getHeight() / 2);
    auto afterArea = waveformArea;
    
    // Before waveform (softer transients)
    g.setColour(juce::Colour(0xFF666666));
    g.fillRoundedRectangle(beforeArea.toFloat().reduced(2), 1.0f);
    g.setColour(juce::Colour(0xFFaaaaaa));
    
    for (int i = 0; i < beforeArea.getWidth(); i += 3)
    {
        float amplitude = std::sin(i * 0.1f) * 0.3f + 0.5f;
        int y = beforeArea.getY() + (int)(beforeArea.getHeight() * amplitude);
        g.drawVerticalLine(beforeArea.getX() + i, (float)beforeArea.getY(), (float)y);
    }
    
    // After waveform (enhanced transients)
    g.setColour(juce::Colour(0xFF333333));
    g.fillRoundedRectangle(afterArea.toFloat().reduced(2), 1.0f);
    g.setColour(juce::Colour(0xFF44ff44));
    
    for (int i = 0; i < afterArea.getWidth(); i += 3)
    {
        float amplitude = std::sin(i * 0.1f) * 0.5f + 0.5f;
        // Enhance peaks for transient effect
        if (i % 20 < 5) amplitude *= 1.3f;
        int y = afterArea.getY() + (int)(afterArea.getHeight() * amplitude);
        g.drawVerticalLine(afterArea.getX() + i, (float)afterArea.getY(), (float)y);
    }
    
    area.removeFromTop(10); // Gap
    
    // Spectral Emphasis
    g.setColour(juce::Colours::white);
    g.drawText("SPECTRAL", area.removeFromTop(12), juce::Justification::centred);
    g.drawText("EMPHASIS", area.removeFromTop(12), juce::Justification::centred);
    
    // Draw frequency emphasis bars
    int numBands = 8;
    int bandWidth = area.getWidth() / numBands;
    
    for (int i = 0; i < numBands; ++i)
    {
        auto bandArea = area.removeFromLeft(bandWidth).reduced(1);
        // Higher frequencies get more emphasis in transient shaping
        float emphasis = 0.2f + (i * 0.1f);
        auto activeBand = bandArea.removeFromBottom((int)(bandArea.getHeight() * emphasis));
        
        g.setColour(juce::Colour(0xFF4444ff));
        g.fillRect(activeBand);
    }
}
