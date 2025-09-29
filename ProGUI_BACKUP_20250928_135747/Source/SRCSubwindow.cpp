#include "SRCSubwindow.h"

SRCSubwindow::SRCSubwindow()
    : ProcessSubwindow("SAMPLE RATE CONVERSION", "Eliminates digital harshness with smooth, musical resampling")
{
}

void SRCSubwindow::createControls()
{
    // Target Sample Rate dropdown
    juce::StringArray sampleRates = {"44.1 kHz", "48 kHz", "88.2 kHz", "96 kHz", "176.4 kHz", "192 kHz"};
    targetSRCombo = createComboBox("Target SR", sampleRates);
    targetSRCombo->setSelectedItemIndex(1); // Default to 48 kHz
    
    // Quality dropdown
    juce::StringArray qualities = {"Fast", "High Quality", "Extreme"};
    qualityCombo = createComboBox("SRC Quality", qualities);
    qualityCombo->setSelectedItemIndex(1); // Default to HQ
    
    // Phase dropdown
    juce::StringArray phases = {"Linear", "Minimum", "Mixed"};
    phaseCombo = createComboBox("Phase", phases);
    phaseCombo->setSelectedItemIndex(0); // Default to Linear
    
    // Control knobs
    passbandRippleKnob = createKnob("Passband Ripple", 0.001f, 0.1f, 0.01f);
    stopbandAttenKnob = createKnob("Stopband Atten", 80.0f, 140.0f, 110.0f);
}

void SRCSubwindow::layoutControls(juce::Rectangle<int> controlArea)
{
    // Top row: Dropdowns
    auto topRow = controlArea.removeFromTop(60);
    auto dropdownWidth = topRow.getWidth() / 3;
    
    targetSRCombo->setBounds(topRow.removeFromLeft(dropdownWidth).reduced(5));
    qualityCombo->setBounds(topRow.removeFromLeft(dropdownWidth).reduced(5));
    phaseCombo->setBounds(topRow.reduced(5));
    
    controlArea.removeFromTop(20); // Gap
    
    // Bottom row: Knobs
    auto knobWidth = controlArea.getWidth() / 2;
    passbandRippleKnob->setBounds(controlArea.removeFromLeft(knobWidth).reduced(10));
    stopbandAttenKnob->setBounds(controlArea.reduced(10));
}

void SRCSubwindow::paintMeters(juce::Graphics& g, juce::Rectangle<int> meterArea)
{
    // Meter background
    g.setColour(juce::Colour(0xFF0f3460));
    g.fillRoundedRectangle(meterArea.toFloat().reduced(5), 6.0f);
    
    auto area = meterArea.reduced(10);
    
    // Aliasing Residual meter
    auto aliasingArea = area.removeFromTop(area.getHeight() / 2);
    g.setColour(juce::Colours::white);
    g.setFont(juce::Font("Arial", 10.0f, juce::Font::bold));
    g.drawText("ALIASING", aliasingArea.removeFromTop(15), juce::Justification::centred);
    g.drawText("RESIDUAL", aliasingArea.removeFromTop(15), juce::Justification::centred);
    
    g.setColour(juce::Colour(0xFF333333));
    g.fillRoundedRectangle(aliasingArea.toFloat().reduced(2), 2.0f);
    
    // Low aliasing is good (inverse meter)
    float aliasingLevel = 0.2f; // Low aliasing
    auto aliasingActive = aliasingArea.removeFromLeft((int)(aliasingArea.getWidth() * aliasingLevel));
    g.setColour(juce::Colour(0xFF44ff44));
    g.fillRoundedRectangle(aliasingActive.toFloat().reduced(2), 2.0f);
    
    area.removeFromTop(20); // Gap
    
    // Latency meter
    g.setColour(juce::Colours::white);
    g.drawText("LATENCY", area.removeFromTop(15), juce::Justification::centred);
    
    g.setColour(juce::Colour(0xFF333333));
    g.fillRoundedRectangle(area.toFloat().reduced(2), 2.0f);
    
    float latencyLevel = 0.3f; // Moderate latency
    auto latencyActive = area.removeFromLeft((int)(area.getWidth() * latencyLevel));
    g.setColour(juce::Colour(0xFFffaa44));
    g.fillRoundedRectangle(latencyActive.toFloat().reduced(2), 2.0f);
    
    // Latency value text
    g.setColour(juce::Colours::white);
    g.setFont(juce::Font("Arial", 9.0f, juce::Font::plain));
    g.drawText("2.3ms", area, juce::Justification::centred);
}
