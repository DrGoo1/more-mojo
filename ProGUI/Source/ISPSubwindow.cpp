#include "ISPSubwindow.h"

ISPSubwindow::ISPSubwindow()
    : ProcessSubwindow("INTERSAMPLE PROCESSING", "Reduces aliasing, pre-ringing, and intersample distortion for cleaner reconstruction")
{
    createControls();
}

void ISPSubwindow::createControls()
{
    // OS Factor dropdown
    juce::StringArray osFactors = {"2×", "4×", "8×"};
    osFactorCombo = createComboBox("OS Factor", osFactors);
    osFactorCombo->setSelectedItemIndex(1); // Default to 4×
    
    // Filter Type dropdown
    juce::StringArray filterTypes = {"Linear", "Minimum Phase", "Polyphase"};
    filterTypeCombo = createComboBox("Filter Type", filterTypes);
    filterTypeCombo->setSelectedItemIndex(2); // Default to Polyphase
    
    // Control knobs
    passbandRolloffKnob = createKnob("Passband Rolloff", 0.0f, 3.0f, 0.5f);
    stopbandAttenKnob = createKnob("Stopband Atten", 60.0f, 120.0f, 90.0f);
    tpCeilingKnob = createKnob("TP Ceiling", -6.0f, 0.0f, -1.0f);
    lookaheadKnob = createKnob("Lookahead", 0.0f, 10.0f, 2.0f);
}

void ISPSubwindow::layoutControls(juce::Rectangle<int> controlArea)
{
    // Top row: Dropdowns
    auto topRow = controlArea.removeFromTop(60);
    osFactorCombo->setBounds(topRow.removeFromLeft(120).reduced(5));
    filterTypeCombo->setBounds(topRow.removeFromLeft(150).reduced(5));
    
    controlArea.removeFromTop(10); // Gap
    
    // Knob grid: 2 rows × 2 columns
    auto knobHeight = (controlArea.getHeight() - 10) / 2;
    
    auto firstRow = controlArea.removeFromTop(knobHeight);
    auto knobWidth = firstRow.getWidth() / 2;
    
    passbandRolloffKnob->setBounds(firstRow.removeFromLeft(knobWidth).reduced(10));
    stopbandAttenKnob->setBounds(firstRow.reduced(10));
    
    controlArea.removeFromTop(10); // Gap between rows
    
    auto secondRow = controlArea;
    tpCeilingKnob->setBounds(secondRow.removeFromLeft(knobWidth).reduced(10));
    lookaheadKnob->setBounds(secondRow.reduced(10));
}

void ISPSubwindow::paintMeters(juce::Graphics& g, juce::Rectangle<int> meterArea)
{
    // Meter background
    g.setColour(juce::Colour(0xFF0f3460));
    g.fillRoundedRectangle(meterArea.toFloat().reduced(5), 6.0f);
    
    // Meter labels and bars
    auto area = meterArea.reduced(10);
    
    // True-Peak meter
    auto tpArea = area.removeFromTop(area.getHeight() / 3);
    g.setColour(juce::Colours::white);
    g.setFont(juce::Font("Arial", 10.0f, juce::Font::bold));
    g.drawText("TRUE-PEAK", tpArea.removeFromTop(15), juce::Justification::centred);
    
    g.setColour(juce::Colour(0xFF333333));
    g.fillRoundedRectangle(tpArea.toFloat().reduced(2), 2.0f);
    
    // Simulated meter level (would be real-time in actual implementation)
    float tpLevel = 0.7f; // 70% of scale
    auto tpActive = tpArea.removeFromLeft((int)(tpArea.getWidth() * tpLevel));
    g.setColour(juce::Colour(0xFFff4444));
    g.fillRoundedRectangle(tpActive.toFloat().reduced(2), 2.0f);
    
    area.removeFromTop(10); // Gap
    
    // ISP Count meter
    auto ispArea = area.removeFromTop(area.getHeight() / 2);
    g.setColour(juce::Colours::white);
    g.drawText("ISP COUNT", ispArea.removeFromTop(15), juce::Justification::centred);
    
    g.setColour(juce::Colour(0xFF333333));
    g.fillRoundedRectangle(ispArea.toFloat().reduced(2), 2.0f);
    
    float ispLevel = 0.4f; // 40% activity
    auto ispActive = ispArea.removeFromLeft((int)(ispArea.getWidth() * ispLevel));
    g.setColour(juce::Colour(0xFF44ff44));
    g.fillRoundedRectangle(ispActive.toFloat().reduced(2), 2.0f);
    
    area.removeFromTop(10); // Gap
    
    // Ceiling Margin meter
    g.setColour(juce::Colours::white);
    g.drawText("CEILING MARGIN", area.removeFromTop(15), juce::Justification::centred);
    
    g.setColour(juce::Colour(0xFF333333));
    g.fillRoundedRectangle(area.toFloat().reduced(2), 2.0f);
    
    float marginLevel = 0.8f; // Good margin
    auto marginActive = area.removeFromLeft((int)(area.getWidth() * marginLevel));
    g.setColour(juce::Colour(0xFF4444ff));
    g.fillRoundedRectangle(marginActive.toFloat().reduced(2), 2.0f);
}
