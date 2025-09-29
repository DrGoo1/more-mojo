#include "ProcessControl.h"

ProcessControl::ProcessControl(const juce::String& processName, const juce::String& description)
    : processName(processName), description(description)
{
    // Create Neptune knob
    knob = std::make_unique<NeptuneKnob>();
    knob->setValue(0.5f);
    knob->onValueChanged = [this](float value) {
        currentLevel = value;
        repaint(); // Repaint to update cyan bar
        if (onKnobChanged)
            onKnobChanged(value);
    };
    addAndMakeVisible(*knob);
    
    // Create OPEN CONTROLS button
    openButton = std::make_unique<juce::TextButton>("OPEN CONTROLS");
    openButton->setColour(juce::TextButton::buttonColourId, juce::Colour(0xFF2a2a2a));
    openButton->setColour(juce::TextButton::textColourOffId, juce::Colours::white);
    openButton->onClick = [this]() {
        if (onOpenControls)
            onOpenControls();
    };
    addAndMakeVisible(*openButton);
}

void ProcessControl::paint(juce::Graphics& g)
{
    auto bounds = getLocalBounds();
    
    // Background with subtle border
    g.setColour(juce::Colour(0xFF1a1a1a));
    g.fillRoundedRectangle(bounds.toFloat(), 4.0f);
    g.setColour(juce::Colour(0xFF333333));
    g.drawRoundedRectangle(bounds.toFloat(), 4.0f, 1.0f);
    
    // Layout areas
    auto contentArea = bounds.reduced(8);
    
    // Cyan level bar at top
    auto levelArea = contentArea.removeFromTop(12).reduced(0, 2);
    
    // Background for level bar
    g.setColour(juce::Colour(0xFF333333));
    g.fillRoundedRectangle(levelArea.toFloat(), 2.0f);
    
    // Active level (cyan)
    auto activeLevelArea = levelArea.removeFromLeft((int)(levelArea.getWidth() * currentLevel));
    g.setColour(juce::Colour(0xFF00d4aa)); // Cyan color
    g.fillRoundedRectangle(activeLevelArea.toFloat(), 2.0f);
    
    contentArea.removeFromTop(8); // Gap after level bar
    
    // Process name (main title)
    auto nameArea = contentArea.removeFromTop(20);
    g.setColour(juce::Colours::white);
    g.setFont(juce::Font(14.0f, juce::Font::bold));
    g.drawText(processName, nameArea, juce::Justification::centredLeft);
    
    // Description (subtitle)
    auto descArea = contentArea.removeFromTop(16);
    g.setColour(juce::Colour(0xFF00ffff)); // Cyan text
    g.setFont(juce::Font(11.0f, juce::Font::plain));
    g.drawText(description, descArea, juce::Justification::centredLeft);
}

void ProcessControl::resized()
{
    auto bounds = getLocalBounds().reduced(8);
    
    // Skip level bar and text areas
    bounds.removeFromTop(12 + 8 + 20 + 16 + 8); // level + gap + name + desc + gap
    
    // Bottom area for knob and button
    auto controlArea = bounds;
    
    // Knob on the left (square area)
    int knobSize = juce::jmin(50, controlArea.getHeight());
    auto knobArea = controlArea.removeFromLeft(knobSize);
    knobArea = knobArea.withHeight(knobSize);
    knob->setBounds(knobArea);
    
    controlArea.removeFromLeft(8); // Gap
    
    // Button on the right
    auto buttonArea = controlArea.removeFromRight(100);
    buttonArea = buttonArea.withHeight(25).withY(knobArea.getY() + (knobArea.getHeight() - 25) / 2);
    openButton->setBounds(buttonArea);
}

void ProcessControl::setLevel(float level)
{
    currentLevel = juce::jlimit(0.0f, 1.0f, level);
    repaint();
}

float ProcessControl::getKnobValue() const
{
    return currentLevel;
}
