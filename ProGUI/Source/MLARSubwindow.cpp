#include "MLARSubwindow.h"

MLARSubwindow::MLARSubwindow()
    : ProcessSubwindow("MORE LIKE A RECORD", "Vintage analog character with subtle compression, saturation, and frequency response")
{
}

void MLARSubwindow::createControls()
{
    // Main amount control
    mlarAmountKnob = createKnob("MLAR Amount", 0.0f, 100.0f, 50.0f);
    
    // Profile selection
    juce::StringArray profiles = {"Instrument", "Buss", "Master"};
    profileCombo = createComboBox("Profile", profiles);
    profileCombo->setSelectedItemIndex(1); // Default to Buss
    
    // Focus selection
    juce::StringArray focuses = {"Timing", "Phase", "Transient"};
    focusCombo = createComboBox("Focus", focuses);
    
    // Sensitivity control
    sensitivityKnob = createKnob("Sensitivity", 0.0f, 100.0f, 70.0f);
    
    // Frequency emphasis controls
    hfEmphasisKnob = createKnob("HF Emphasis", -12.0f, 12.0f, 2.0f);
    midEmphasisKnob = createKnob("Mid Emphasis", -12.0f, 12.0f, 1.0f);
    lfEmphasisKnob = createKnob("LF Emphasis", -12.0f, 12.0f, 3.0f);
    
    // Blend and output
    blendKnob = createKnob("Dry/Wet", 0.0f, 100.0f, 75.0f);
    outputTrimKnob = createKnob("Output Trim", -12.0f, 12.0f, 0.0f);
}

void MLARSubwindow::layoutControls(juce::Rectangle<int> controlArea)
{
    // Top row: Main controls
    auto topRow = controlArea.removeFromTop(100);
    auto topWidth = topRow.getWidth() / 4;
    
    mlarAmountKnob->setBounds(topRow.removeFromLeft(topWidth).reduced(10));
    profileCombo->setBounds(topRow.removeFromLeft(topWidth).reduced(5));
    focusCombo->setBounds(topRow.removeFromLeft(topWidth).reduced(5));
    sensitivityKnob->setBounds(topRow.reduced(10));
    
    controlArea.removeFromTop(20); // Gap
    
    // Middle row: Frequency emphasis
    auto middleRow = controlArea.removeFromTop(100);
    auto emphasisWidth = middleRow.getWidth() / 3;
    
    hfEmphasisKnob->setBounds(middleRow.removeFromLeft(emphasisWidth).reduced(10));
    midEmphasisKnob->setBounds(middleRow.removeFromLeft(emphasisWidth).reduced(10));
    lfEmphasisKnob->setBounds(middleRow.reduced(10));
    
    controlArea.removeFromTop(20); // Gap
    
    // Bottom row: Blend and output
    auto bottomRow = controlArea;
    auto bottomWidth = bottomRow.getWidth() / 2;
    
    blendKnob->setBounds(bottomRow.removeFromLeft(bottomWidth).reduced(10));
    outputTrimKnob->setBounds(bottomRow.reduced(10));
}

void MLARSubwindow::paintMeters(juce::Graphics& g, juce::Rectangle<int> meterArea)
{
    // Meter background
    g.setColour(juce::Colour(0xFF0f3460));
    g.fillRoundedRectangle(meterArea.toFloat().reduced(5), 6.0f);
    
    auto area = meterArea.reduced(10);
    
    // Before/After Waveform comparison
    auto waveformArea = area.removeFromTop(area.getHeight() / 4);
    g.setColour(juce::Colours::white);
    g.setFont(juce::Font("Arial", 8.0f, juce::Font::bold));
    g.drawText("BEFORE/AFTER", waveformArea.removeFromTop(12), juce::Justification::centred);
    
    auto beforeWave = waveformArea.removeFromTop(waveformArea.getHeight() / 2);
    auto afterWave = waveformArea;
    
    // Before waveform (digital/harsh)
    g.setColour(juce::Colour(0xFF666666));
    g.fillRoundedRectangle(beforeWave.toFloat().reduced(1), 1.0f);
    g.setColour(juce::Colour(0xFFaaaaaa));
    
    for (int i = 0; i < beforeWave.getWidth(); i += 2)
    {
        float amplitude = std::sin(i * 0.15f) * 0.4f + 0.5f;
        int y = beforeWave.getY() + (int)(beforeWave.getHeight() * amplitude);
        g.drawVerticalLine(beforeWave.getX() + i, (float)beforeWave.getY(), (float)y);
    }
    
    // After waveform (analog/smooth)
    g.setColour(juce::Colour(0xFF333333));
    g.fillRoundedRectangle(afterWave.toFloat().reduced(1), 1.0f);
    g.setColour(juce::Colour(0xFFffaa44));
    
    for (int i = 0; i < afterWave.getWidth(); i += 2)
    {
        float amplitude = std::sin(i * 0.15f) * 0.45f + 0.5f;
        // Add subtle analog-style saturation curve
        amplitude = std::tanh(amplitude * 1.2f) * 0.8f + 0.1f;
        int y = afterWave.getY() + (int)(afterWave.getHeight() * amplitude);
        g.drawVerticalLine(afterWave.getX() + i, (float)afterWave.getY(), (float)y);
    }
    
    area.removeFromTop(10); // Gap
    
    // Correlation/Phase meter
    auto corrPhaseArea = area.removeFromTop(area.getHeight() / 3);
    g.setColour(juce::Colours::white);
    g.drawText("CORRELATION", corrPhaseArea.removeFromTop(12), juce::Justification::centred);
    
    auto corrMeter = corrPhaseArea.removeFromTop(15);
    g.setColour(juce::Colour(0xFF333333));
    g.fillRoundedRectangle(corrMeter.toFloat().reduced(2), 2.0f);
    
    float correlation = 0.92f; // High correlation (good)
    auto corrActive = corrMeter.removeFromLeft((int)(corrMeter.getWidth() * correlation));
    g.setColour(juce::Colour(0xFF44ff44));
    g.fillRoundedRectangle(corrActive.toFloat().reduced(2), 2.0f);
    
    // Phase meter (circular)
    auto phaseMeter = corrPhaseArea.reduced(2);
    g.setColour(juce::Colour(0xFF333333));
    g.fillEllipse(phaseMeter.toFloat());
    
    // Draw phase correlation dot
    auto center = phaseMeter.getCentre();
    float phaseRadius = juce::jmin(phaseMeter.getWidth(), phaseMeter.getHeight()) * 0.3f;
    g.setColour(juce::Colour(0xFF44ff44));
    g.fillEllipse(center.x - 2, center.y - 2, 4, 4);
    
    area.removeFromTop(10); // Gap
    
    // Residual Difference meter
    auto residualArea = area.removeFromTop(area.getHeight() / 2);
    g.setColour(juce::Colours::white);
    g.drawText("RESIDUAL", residualArea.removeFromTop(12), juce::Justification::centred);
    g.drawText("DIFFERENCE", residualArea.removeFromTop(12), juce::Justification::centred);
    
    g.setColour(juce::Colour(0xFF333333));
    g.fillRoundedRectangle(residualArea.toFloat().reduced(2), 2.0f);
    
    float residual = 0.25f; // Moderate residual difference
    auto residualActive = residualArea.removeFromLeft((int)(residualArea.getWidth() * residual));
    g.setColour(juce::Colour(0xFF4444ff));
    g.fillRoundedRectangle(residualActive.toFloat().reduced(2), 2.0f);
    
    area.removeFromTop(10); // Gap
    
    // Activity Timeline
    g.setColour(juce::Colours::white);
    g.drawText("ACTIVITY", area.removeFromTop(12), juce::Justification::centred);
    
    // Draw activity timeline
    int numSamples = 15;
    int sampleWidth = area.getWidth() / numSamples;
    
    for (int i = 0; i < numSamples; ++i)
    {
        auto sampleArea = area.removeFromLeft(sampleWidth).reduced(1);
        
        // Simulate MLAR processing activity
        float activity = 0.3f + (std::sin(i * 0.4f) * 0.4f);
        activity = juce::jmax(0.1f, activity);
        
        auto activeSample = sampleArea.removeFromBottom((int)(sampleArea.getHeight() * activity));
        
        g.setColour(juce::Colour(0xFFffaa44));
        g.fillRect(activeSample);
    }
}
