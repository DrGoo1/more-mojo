#include "HorizontalBarMeter.h"

HorizontalBarMeter::HorizontalBarMeter(const juce::String& label)
    : labelText(label)
{
}

void HorizontalBarMeter::setLevel(float newLevel)
{
    currentLevel = juce::jlimit(0.0f, 1.0f, newLevel);
    repaint();
}

void HorizontalBarMeter::setLabel(const juce::String& newLabel)
{
    labelText = newLabel;
    repaint();
}

void HorizontalBarMeter::paint(juce::Graphics& g)
{
    auto bounds = getLocalBounds();
    
    // Label area (left side)
    auto labelArea = bounds.removeFromLeft(60);
    auto meterArea = bounds.reduced(2);
    
    // Draw label
    g.setColour(juce::Colours::white);
    g.setFont(juce::Font(10.0f, juce::Font::bold));
    g.drawText(labelText, labelArea, juce::Justification::centredRight);
    
    // Draw meter background
    g.setColour(juce::Colour(0xFF333333));
    g.fillRoundedRectangle(meterArea.toFloat(), 2.0f);
    
    // Draw meter level (single cyan color)
    if (currentLevel > 0.0f)
    {
        auto levelArea = meterArea.removeFromLeft((int)(meterArea.getWidth() * currentLevel));
        
        // Single cyan color for all levels
        juce::Colour levelColor = juce::Colour(0xFF00d4aa); // Cyan
        
        g.setColour(levelColor);
        g.fillRoundedRectangle(levelArea.toFloat(), 2.0f);
        
        // Add highlight
        g.setColour(levelColor.brighter(0.3f));
        auto highlightArea = levelArea.removeFromTop(levelArea.getHeight() / 3);
        g.fillRoundedRectangle(highlightArea.toFloat(), 2.0f);
    }
    
    // Draw border
    g.setColour(juce::Colours::white.withAlpha(0.3f));
    g.drawRoundedRectangle(meterArea.toFloat(), 2.0f, 1.0f);
}
