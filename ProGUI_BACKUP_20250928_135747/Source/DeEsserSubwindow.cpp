#include "DeEsserSubwindow.h"

DeEsserSubwindow::DeEsserSubwindow()
    : ProcessSubwindow("DE-ESSING", "Removes harsh frequencies while preserving natural brightness")
{
}

void DeEsserSubwindow::createControls()
{
    // Frequency and bandwidth controls
    freqKnob = createKnob("Frequency", 2000.0f, 12000.0f, 6000.0f);
    qBandwidthKnob = createKnob("Q/Bandwidth", 0.5f, 10.0f, 2.0f);
    
    // Dynamics controls
    thresholdKnob = createKnob("Threshold", -40.0f, 0.0f, -20.0f);
    amountKnob = createKnob("Amount", 0.0f, 100.0f, 50.0f);
    
    // Mode selection
    juce::StringArray modes = {"Split Band", "Full Band"};
    modeCombo = createComboBox("Mode", modes);
    
    // Listen button for monitoring sibilance
    listenButton = createToggleButton("Listen");
}

void DeEsserSubwindow::layoutControls(juce::Rectangle<int> controlArea)
{
    // Top row: Frequency controls
    auto topRow = controlArea.removeFromTop(100);
    auto freqWidth = topRow.getWidth() / 2;
    
    freqKnob->setBounds(topRow.removeFromLeft(freqWidth).reduced(10));
    qBandwidthKnob->setBounds(topRow.reduced(10));
    
    controlArea.removeFromTop(20); // Gap
    
    // Middle row: Dynamics controls
    auto middleRow = controlArea.removeFromTop(100);
    auto dynamicsWidth = middleRow.getWidth() / 2;
    
    thresholdKnob->setBounds(middleRow.removeFromLeft(dynamicsWidth).reduced(10));
    amountKnob->setBounds(middleRow.reduced(10));
    
    controlArea.removeFromTop(20); // Gap
    
    // Bottom row: Mode and Listen
    auto bottomRow = controlArea;
    auto bottomWidth = bottomRow.getWidth() / 2;
    
    modeCombo->setBounds(bottomRow.removeFromLeft(bottomWidth).reduced(10));
    listenButton->setBounds(bottomRow.reduced(10));
}

void DeEsserSubwindow::paintMeters(juce::Graphics& g, juce::Rectangle<int> meterArea)
{
    // Meter background
    g.setColour(juce::Colour(0xFF0f3460));
    g.fillRoundedRectangle(meterArea.toFloat().reduced(5), 6.0f);
    
    auto area = meterArea.reduced(10);
    
    // Sibilance Activity meter
    auto sibilanceArea = area.removeFromTop(area.getHeight() / 2);
    g.setColour(juce::Colours::white);
    g.setFont(juce::Font("Arial", 9.0f, juce::Font::bold));
    g.drawText("SIBILANCE", sibilanceArea.removeFromTop(12), juce::Justification::centred);
    g.drawText("ACTIVITY", sibilanceArea.removeFromTop(12), juce::Justification::centred);
    
    // Draw sibilance detection over time
    g.setColour(juce::Colour(0xFF333333));
    g.fillRoundedRectangle(sibilanceArea.toFloat().reduced(2), 2.0f);
    
    // Simulate sibilance detection events
    int numSamples = 20;
    int sampleWidth = sibilanceArea.getWidth() / numSamples;
    
    for (int i = 0; i < numSamples; ++i)
    {
        auto sampleArea = sibilanceArea.removeFromLeft(sampleWidth);
        // Simulate sibilance detection (peaks at certain intervals)
        bool isSibilant = (i % 8 == 3) || (i % 12 == 7);
        float activity = isSibilant ? 0.9f : 0.1f;
        
        auto activeSample = sampleArea.removeFromBottom((int)(sampleArea.getHeight() * activity));
        
        juce::Colour color = isSibilant ? juce::Colour(0xFFff4444) : juce::Colour(0xFF444444);
        g.setColour(color);
        g.fillRect(activeSample.reduced(1));
    }
    
    area.removeFromTop(15); // Gap
    
    // HF Spectrum analyzer
    g.setColour(juce::Colours::white);
    g.drawText("HF SPECTRUM", area.removeFromTop(15), juce::Justification::centred);
    
    // Draw frequency spectrum focusing on high frequencies
    int numBands = 12;
    int bandWidth = area.getWidth() / numBands;
    
    for (int i = 0; i < numBands; ++i)
    {
        auto bandArea = area.removeFromLeft(bandWidth).reduced(1);
        
        // Simulate frequency spectrum with emphasis on problem frequencies
        float frequency = 2000.0f + (i * 1000.0f); // 2kHz to 14kHz range
        float amplitude = 0.3f;
        
        // Simulate sibilant frequency peak around 6-8kHz (bands 4-6)
        if (i >= 4 && i <= 6)
        {
            amplitude = 0.7f + (std::sin(i * 0.5f) * 0.2f); // Peak in sibilant range
        }
        else
        {
            amplitude = 0.2f + (std::sin(i * 0.3f) * 0.15f); // Normal spectrum
        }
        
        auto activeBand = bandArea.removeFromBottom((int)(bandArea.getHeight() * amplitude));
        
        // Color coding: red for problem frequencies, green for normal
        juce::Colour color = (i >= 4 && i <= 6) ? juce::Colour(0xFFff6666) : juce::Colour(0xFF44ff44);
        g.setColour(color);
        g.fillRect(activeBand);
    }
    
    // Frequency labels
    g.setColour(juce::Colours::white);
    g.setFont(juce::Font("Arial", 8.0f, juce::Font::plain));
    auto labelArea = meterArea.removeFromBottom(15);
    g.drawText("2k", labelArea.removeFromLeft(labelArea.getWidth() / 4), juce::Justification::centred);
    g.drawText("6k", labelArea.removeFromLeft(labelArea.getWidth() / 3), juce::Justification::centred);
    g.drawText("10k", labelArea.removeFromLeft(labelArea.getWidth() / 2), juce::Justification::centred);
    g.drawText("14k", labelArea, juce::Justification::centred);
}
