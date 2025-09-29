#include "AlignSubwindow.h"

AlignSubwindow::AlignSubwindow()
    : ProcessSubwindow("PHASE ALIGNMENT", "Creates spatial coherence and clearer stereo imaging")
{
}

void AlignSubwindow::createControls()
{
    // Delay controls
    delayKnob = createKnob("Delay", -100.0f, 100.0f, 0.0f);
    
    juce::StringArray delayUnits = {"Samples", "ms"};
    delayUnitsCombo = createComboBox("Delay Units", delayUnits);
    delayUnitsCombo->setSelectedItemIndex(1); // Default to ms
    
    // Polarity invert
    polarityButton = createToggleButton("Polarity Invert");
    
    // Phase and tilt controls
    phaseRotateKnob = createKnob("Phase Rotate", -180.0f, 180.0f, 0.0f);
    tiltAPKnob = createKnob("Tilt AP", -12.0f, 12.0f, 0.0f);
    crossoverKnob = createKnob("Crossover", 20.0f, 20000.0f, 1000.0f);
    
    // Auto functions
    autoAlignButton = createToggleButton("Auto-Align");
    linkButton = createToggleButton("Link L/R");
}

void AlignSubwindow::layoutControls(juce::Rectangle<int> controlArea)
{
    // Top row: Delay controls
    auto topRow = controlArea.removeFromTop(80);
    auto delayLeft = topRow.removeFromLeft(topRow.getWidth() / 2);
    auto delayRight = topRow;
    
    delayKnob->setBounds(delayLeft.reduced(10));
    delayUnitsCombo->setBounds(delayRight.removeFromTop(40).reduced(10));
    polarityButton->setBounds(delayRight.reduced(10));
    
    controlArea.removeFromTop(10); // Gap
    
    // Middle row: Phase controls
    auto middleRow = controlArea.removeFromTop(80);
    auto knobWidth = middleRow.getWidth() / 3;
    
    phaseRotateKnob->setBounds(middleRow.removeFromLeft(knobWidth).reduced(10));
    tiltAPKnob->setBounds(middleRow.removeFromLeft(knobWidth).reduced(10));
    crossoverKnob->setBounds(middleRow.reduced(10));
    
    controlArea.removeFromTop(10); // Gap
    
    // Bottom row: Auto controls
    auto bottomRow = controlArea;
    auto buttonWidth = bottomRow.getWidth() / 2;
    
    autoAlignButton->setBounds(bottomRow.removeFromLeft(buttonWidth).reduced(10));
    linkButton->setBounds(bottomRow.reduced(10));
}

void AlignSubwindow::paintMeters(juce::Graphics& g, juce::Rectangle<int> meterArea)
{
    // Meter background
    g.setColour(juce::Colour(0xFF0f3460));
    g.fillRoundedRectangle(meterArea.toFloat().reduced(5), 6.0f);
    
    auto area = meterArea.reduced(10);
    
    // Correlation meter
    auto corrArea = area.removeFromTop(area.getHeight() / 4);
    g.setColour(juce::Colours::white);
    g.setFont(juce::Font("Arial", 9.0f, juce::Font::bold));
    g.drawText("CORRELATION", corrArea.removeFromTop(15), juce::Justification::centred);
    
    g.setColour(juce::Colour(0xFF333333));
    g.fillRoundedRectangle(corrArea.toFloat().reduced(2), 2.0f);
    
    float corrLevel = 0.85f; // Good correlation
    auto corrActive = corrArea.removeFromLeft((int)(corrArea.getWidth() * corrLevel));
    g.setColour(juce::Colour(0xFF44ff44));
    g.fillRoundedRectangle(corrActive.toFloat().reduced(2), 2.0f);
    
    area.removeFromTop(10); // Gap
    
    // Phase scope (circular display)
    auto phaseArea = area.removeFromTop(area.getHeight() / 3);
    g.setColour(juce::Colours::white);
    g.drawText("PHASE SCOPE", phaseArea.removeFromTop(15), juce::Justification::centred);
    
    auto scopeRect = phaseArea.reduced(5);
    g.setColour(juce::Colour(0xFF333333));
    g.fillEllipse(scopeRect.toFloat());
    
    // Draw phase scope pattern
    g.setColour(juce::Colour(0xFF44ff44));
    auto center = scopeRect.getCentre();
    float radius = juce::jmin(scopeRect.getWidth(), scopeRect.getHeight()) * 0.3f;
    
    // Draw a simple Lissajous pattern
    for (int i = 0; i < 360; i += 10)
    {
        float angle = i * juce::MathConstants<float>::pi / 180.0f;
        float x = center.x + radius * std::cos(angle);
        float y = center.y + radius * std::sin(angle * 1.2f) * 0.7f; // Slightly elliptical
        g.fillEllipse(x - 1, y - 1, 2, 2);
    }
    
    area.removeFromTop(10); // Gap
    
    // IACC Timeline (simplified)
    auto iaccArea = area.removeFromTop(area.getHeight() / 2);
    g.setColour(juce::Colours::white);
    g.drawText("IACC", iaccArea.removeFromTop(15), juce::Justification::centred);
    
    g.setColour(juce::Colour(0xFF333333));
    g.fillRoundedRectangle(iaccArea.toFloat().reduced(2), 2.0f);
    
    // Draw timeline bars
    int numBars = 10;
    int barWidth = iaccArea.getWidth() / numBars;
    
    for (int i = 0; i < numBars; ++i)
    {
        auto barArea = iaccArea.removeFromLeft(barWidth).reduced(1);
        float height = 0.4f + (std::sin(i * 0.5f) * 0.3f); // Varying IACC over time
        auto activeBar = barArea.removeFromBottom((int)(barArea.getHeight() * height));
        
        g.setColour(juce::Colour(0xFF4444ff));
        g.fillRect(activeBar);
    }
    
    area.removeFromTop(10); // Gap
    
    // Group Delay
    g.setColour(juce::Colours::white);
    g.drawText("GROUP DELAY", area.removeFromTop(15), juce::Justification::centred);
    
    g.setColour(juce::Colour(0xFF333333));
    g.fillRoundedRectangle(area.toFloat().reduced(2), 2.0f);
    
    float delayLevel = 0.3f; // Moderate group delay
    auto delayActive = area.removeFromLeft((int)(area.getWidth() * delayLevel));
    g.setColour(juce::Colour(0xFFffaa44));
    g.fillRoundedRectangle(delayActive.toFloat().reduced(2), 2.0f);
}
