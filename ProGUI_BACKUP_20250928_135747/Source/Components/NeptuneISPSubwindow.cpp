#include "NeptuneISPSubwindow.h"

NeptuneISPSubwindow::NeptuneISPSubwindow()
{
    setSize(600, 500);
    
    // Create OS Factor dropdown
    osFactorCombo = std::make_unique<juce::ComboBox>("OS Factor");
    osFactorCombo->addItem("2×", 1);
    osFactorCombo->addItem("4×", 2);
    osFactorCombo->addItem("8×", 3);
    osFactorCombo->setSelectedId(2); // Default to 4×
    addAndMakeVisible(*osFactorCombo);
    
    osFactorLabel = std::make_unique<juce::Label>("", "OS Factor");
    osFactorLabel->setJustificationType(juce::Justification::centred);
    osFactorLabel->setColour(juce::Label::textColourId, juce::Colours::white);
    addAndMakeVisible(*osFactorLabel);
    
    // Create Filter Type dropdown
    filterTypeCombo = std::make_unique<juce::ComboBox>("Filter Type");
    filterTypeCombo->addItem("Linear Phase", 1);
    filterTypeCombo->addItem("Minimum Phase", 2);
    filterTypeCombo->addItem("Polyphase", 3);
    filterTypeCombo->setSelectedId(3); // Default to Polyphase
    addAndMakeVisible(*filterTypeCombo);
    
    filterTypeLabel = std::make_unique<juce::Label>("", "Filter Type");
    filterTypeLabel->setJustificationType(juce::Justification::centred);
    filterTypeLabel->setColour(juce::Label::textColourId, juce::Colours::white);
    addAndMakeVisible(*filterTypeLabel);
    
    // Create Neptune knobs (2x2 grid)
    passbandRolloffKnob = std::make_unique<NeptuneKnob>();
    passbandRolloffKnob->setValue(0.17f); // 0.5/3.0 normalized
    addAndMakeVisible(*passbandRolloffKnob);
    
    passbandLabel = std::make_unique<juce::Label>("", "Passband Rolloff");
    passbandLabel->setJustificationType(juce::Justification::centred);
    passbandLabel->setColour(juce::Label::textColourId, juce::Colours::white);
    addAndMakeVisible(*passbandLabel);
    
    stopbandAttenKnob = std::make_unique<NeptuneKnob>();
    stopbandAttenKnob->setValue(0.5f); // 90/120 normalized
    addAndMakeVisible(*stopbandAttenKnob);
    
    stopbandLabel = std::make_unique<juce::Label>("", "Stopband Atten");
    stopbandLabel->setJustificationType(juce::Justification::centred);
    stopbandLabel->setColour(juce::Label::textColourId, juce::Colours::white);
    addAndMakeVisible(*stopbandLabel);
    
    tpCeilingKnob = std::make_unique<NeptuneKnob>();
    tpCeilingKnob->setValue(0.83f); // -1.0/-6.0 normalized
    addAndMakeVisible(*tpCeilingKnob);
    
    tpCeilingLabel = std::make_unique<juce::Label>("", "TP Ceiling");
    tpCeilingLabel->setJustificationType(juce::Justification::centred);
    tpCeilingLabel->setColour(juce::Label::textColourId, juce::Colours::white);
    addAndMakeVisible(*tpCeilingLabel);
    
    lookaheadKnob = std::make_unique<NeptuneKnob>();
    lookaheadKnob->setValue(0.2f); // 2.0/10.0 normalized
    addAndMakeVisible(*lookaheadKnob);
    
    lookaheadLabel = std::make_unique<juce::Label>("", "Lookahead");
    lookaheadLabel->setJustificationType(juce::Justification::centred);
    lookaheadLabel->setColour(juce::Label::textColourId, juce::Colours::white);
    addAndMakeVisible(*lookaheadLabel);
}

void NeptuneISPSubwindow::paint(juce::Graphics& g)
{
    // Professional dark background
    auto bg = juce::ColourGradient(juce::Colour(0xFF1a1a2e), 0, 0,
                                  juce::Colour(0xFF16213e), getWidth(), getHeight(), false);
    g.setGradientFill(bg);
    g.fillAll();
    
    // Title
    g.setColour(juce::Colours::white);
    g.setFont(juce::Font(20.0f, juce::Font::bold));
    g.drawText("ISP - Intersample Processing", 20, 20, getWidth()-40, 30, juce::Justification::centred);
    
    // Section headers
    g.setFont(juce::Font(14.0f, juce::Font::bold));
    g.setColour(juce::Colours::cyan);
    g.drawText("OVERSAMPLING", 50, 80, 200, 20, juce::Justification::left);
    g.drawText("CONTROLS", 50, 200, 200, 20, juce::Justification::left);
    
    // Professional meters (placeholder)
    auto meterArea = juce::Rectangle<int>(400, 200, 150, 250);
    g.setColour(juce::Colour(0xFF333333));
    g.fillRoundedRectangle(meterArea.toFloat(), 5.0f);
    
    g.setColour(juce::Colours::white);
    g.setFont(juce::Font(12.0f, juce::Font::bold));
    g.drawText("METERS", meterArea.getX(), meterArea.getY() - 20, meterArea.getWidth(), 20, juce::Justification::centred);
    
    // Meter labels
    g.setFont(juce::Font(10.0f));
    g.setColour(juce::Colours::red);
    g.drawText("True-Peak", meterArea.getX() + 10, meterArea.getY() + 20, 80, 15, juce::Justification::left);
    g.setColour(juce::Colours::green);
    g.drawText("ISP Count", meterArea.getX() + 10, meterArea.getY() + 80, 80, 15, juce::Justification::left);
    g.setColour(juce::Colours::blue);
    g.drawText("Ceiling Margin", meterArea.getX() + 10, meterArea.getY() + 140, 80, 15, juce::Justification::left);
}

void NeptuneISPSubwindow::resized()
{
    // Dropdown area (top)
    auto dropdownArea = getLocalBounds().removeFromTop(180).reduced(50, 60);
    
    // OS Factor dropdown
    auto osArea = dropdownArea.removeFromLeft(dropdownArea.getWidth() / 2).reduced(10);
    osFactorLabel->setBounds(osArea.removeFromTop(20));
    osFactorCombo->setBounds(osArea.removeFromTop(25));
    
    // Filter Type dropdown  
    auto filterArea = dropdownArea.reduced(10);
    filterTypeLabel->setBounds(filterArea.removeFromTop(20));
    filterTypeCombo->setBounds(filterArea.removeFromTop(25));
    
    // Neptune knobs area (2x2 grid)
    auto knobArea = getLocalBounds().removeFromBottom(280).removeFromLeft(350).reduced(50, 20);
    
    int knobSize = 80;
    int spacing = 20;
    
    // Top row
    auto topRow = knobArea.removeFromTop(knobSize + 30);
    auto leftKnob = topRow.removeFromLeft((knobArea.getWidth() / 2));
    auto rightKnob = topRow;
    
    // Passband Rolloff (top-left)
    passbandRolloffKnob->setBounds(leftKnob.withSizeKeepingCentre(knobSize, knobSize));
    passbandLabel->setBounds(leftKnob.removeFromBottom(20));
    
    // Stopband Atten (top-right)
    stopbandAttenKnob->setBounds(rightKnob.withSizeKeepingCentre(knobSize, knobSize));
    stopbandLabel->setBounds(rightKnob.removeFromBottom(20));
    
    // Bottom row
    auto bottomRow = knobArea.removeFromTop(knobSize + 30);
    auto leftKnob2 = bottomRow.removeFromLeft((knobArea.getWidth() / 2));
    auto rightKnob2 = bottomRow;
    
    // TP Ceiling (bottom-left)
    tpCeilingKnob->setBounds(leftKnob2.withSizeKeepingCentre(knobSize, knobSize));
    tpCeilingLabel->setBounds(leftKnob2.removeFromBottom(20));
    
    // Lookahead (bottom-right)
    lookaheadKnob->setBounds(rightKnob2.withSizeKeepingCentre(knobSize, knobSize));
    lookaheadLabel->setBounds(rightKnob2.removeFromBottom(20));
}
