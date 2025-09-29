#include "TransformerSubwindow.h"

TransformerSubwindow::TransformerSubwindow()
    : ProcessSubwindow("TRANSFORMER", "Harmonic driver with analog transformer modeling for warmth and character")
{
}

void TransformerSubwindow::createControls()
{
    // Drive and harmonic controls
    driveKnob = createKnob("Drive", 0.0f, 100.0f, 30.0f);
    evenOddBalanceKnob = createKnob("Even/Odd Balance", -100.0f, 100.0f, 0.0f); // -100 = all even, +100 = all odd
    biasKnob = createKnob("Bias", -10.0f, 10.0f, 0.0f);
    
    // Core material selection
    juce::StringArray coreModes = {"Iron", "Nickel", "Copper"};
    coreModeCombo = createComboBox("Core Mode", coreModes);
    
    // Frequency response controls
    hfEmphasisKnob = createKnob("HF Emphasis", -6.0f, 6.0f, 1.0f);
    lfSagKnob = createKnob("LF Sag", 0.0f, 100.0f, 20.0f);
    
    // Dynamics response
    juce::StringArray dynamics = {"Fast", "Slow"};
    dynamicsCombo = createComboBox("Dynamics", dynamics);
    dynamicsCombo->setSelectedItemIndex(1); // Default to Slow
    
    // Mix and output
    mixKnob = createKnob("Mix", 0.0f, 100.0f, 80.0f);
    transformerOutputTrimKnob = createKnob("Output Trim", -12.0f, 12.0f, 0.0f);
}

void TransformerSubwindow::layoutControls(juce::Rectangle<int> controlArea)
{
    // Top row: Drive and harmonics
    auto topRow = controlArea.removeFromTop(100);
    auto topWidth = topRow.getWidth() / 4;
    
    driveKnob->setBounds(topRow.removeFromLeft(topWidth).reduced(10));
    evenOddBalanceKnob->setBounds(topRow.removeFromLeft(topWidth).reduced(10));
    biasKnob->setBounds(topRow.removeFromLeft(topWidth).reduced(10));
    coreModeCombo->setBounds(topRow.reduced(5));
    
    controlArea.removeFromTop(20); // Gap
    
    // Middle row: Frequency response
    auto middleRow = controlArea.removeFromTop(100);
    auto freqWidth = middleRow.getWidth() / 3;
    
    hfEmphasisKnob->setBounds(middleRow.removeFromLeft(freqWidth).reduced(10));
    lfSagKnob->setBounds(middleRow.removeFromLeft(freqWidth).reduced(10));
    dynamicsCombo->setBounds(middleRow.reduced(5));
    
    controlArea.removeFromTop(20); // Gap
    
    // Bottom row: Mix and output
    auto bottomRow = controlArea;
    auto bottomWidth = bottomRow.getWidth() / 2;
    
    mixKnob->setBounds(bottomRow.removeFromLeft(bottomWidth).reduced(10));
    transformerOutputTrimKnob->setBounds(bottomRow.reduced(10));
}

void TransformerSubwindow::paintMeters(juce::Graphics& g, juce::Rectangle<int> meterArea)
{
    // Meter background
    g.setColour(juce::Colour(0xFF0f3460));
    g.fillRoundedRectangle(meterArea.toFloat().reduced(5), 6.0f);
    
    auto area = meterArea.reduced(10);
    
    // Harmonic Spectrum (H2/H3 bars)
    auto harmonicArea = area.removeFromTop(area.getHeight() / 3);
    g.setColour(juce::Colours::white);
    g.setFont(juce::Font("Arial", 8.0f, juce::Font::bold));
    g.drawText("HARMONICS", harmonicArea.removeFromTop(12), juce::Justification::centred);
    
    auto h2Area = harmonicArea.removeFromTop(harmonicArea.getHeight() / 2);
    auto h3Area = harmonicArea;
    
    // H2 (Even harmonics) bar
    g.setColour(juce::Colours::white);
    g.setFont(juce::Font("Arial", 7.0f, juce::Font::plain));
    g.drawText("H2", h2Area.removeFromLeft(15), juce::Justification::centred);
    
    g.setColour(juce::Colour(0xFF333333));
    g.fillRoundedRectangle(h2Area.toFloat().reduced(2), 2.0f);
    
    float h2Level = 0.6f; // Moderate even harmonic content
    auto h2Active = h2Area.removeFromLeft((int)(h2Area.getWidth() * h2Level));
    g.setColour(juce::Colour(0xFF44ff44));
    g.fillRoundedRectangle(h2Active.toFloat().reduced(2), 2.0f);
    
    // H3 (Odd harmonics) bar
    g.setColour(juce::Colours::white);
    g.drawText("H3", h3Area.removeFromLeft(15), juce::Justification::centred);
    
    g.setColour(juce::Colour(0xFF333333));
    g.fillRoundedRectangle(h3Area.toFloat().reduced(2), 2.0f);
    
    float h3Level = 0.4f; // Lower odd harmonic content
    auto h3Active = h3Area.removeFromLeft((int)(h3Area.getWidth() * h3Level));
    g.setColour(juce::Colour(0xFFff6644));
    g.fillRoundedRectangle(h3Active.toFloat().reduced(2), 2.0f);
    
    area.removeFromTop(15); // Gap
    
    // THD meter
    auto thdArea = area.removeFromTop(area.getHeight() / 3);
    g.setColour(juce::Colours::white);
    g.drawText("THD", thdArea.removeFromTop(15), juce::Justification::centred);
    
    g.setColour(juce::Colour(0xFF333333));
    g.fillRoundedRectangle(thdArea.toFloat().reduced(2), 2.0f);
    
    float thdLevel = 0.35f; // Moderate THD from transformer saturation
    auto thdActive = thdArea.removeFromLeft((int)(thdArea.getWidth() * thdLevel));
    g.setColour(juce::Colour(0xFFffaa44));
    g.fillRoundedRectangle(thdActive.toFloat().reduced(2), 2.0f);
    
    // THD percentage text
    g.setColour(juce::Colours::white);
    g.setFont(juce::Font("Arial", 8.0f, juce::Font::plain));
    g.drawText("2.1%", thdArea, juce::Justification::centred);
    
    area.removeFromTop(15); // Gap
    
    // In/Out meters
    auto ioArea = area.removeFromTop(area.getHeight() / 2);
    g.setColour(juce::Colours::white);
    g.drawText("IN/OUT", ioArea.removeFromTop(15), juce::Justification::centred);
    
    auto inMeter = ioArea.removeFromTop(ioArea.getHeight() / 2);
    auto outMeter = ioArea;
    
    // Input meter
    g.setColour(juce::Colours::white);
    g.setFont(juce::Font("Arial", 7.0f, juce::Font::plain));
    g.drawText("IN", inMeter.removeFromLeft(15), juce::Justification::centred);
    
    g.setColour(juce::Colour(0xFF333333));
    g.fillRoundedRectangle(inMeter.toFloat().reduced(1), 1.0f);
    
    float inLevel = 0.7f;
    auto inActive = inMeter.removeFromLeft((int)(inMeter.getWidth() * inLevel));
    g.setColour(juce::Colour(0xFF44ff44));
    g.fillRoundedRectangle(inActive.toFloat().reduced(1), 1.0f);
    
    // Output meter
    g.setColour(juce::Colours::white);
    g.drawText("OUT", outMeter.removeFromLeft(15), juce::Justification::centred);
    
    g.setColour(juce::Colour(0xFF333333));
    g.fillRoundedRectangle(outMeter.toFloat().reduced(1), 1.0f);
    
    float outLevel = 0.75f; // Slightly higher due to harmonic content
    auto outActive = outMeter.removeFromLeft((int)(outMeter.getWidth() * outLevel));
    g.setColour(juce::Colour(0xFFff6644));
    g.fillRoundedRectangle(outActive.toFloat().reduced(1), 1.0f);
    
    area.removeFromTop(10); // Gap
    
    // Gain Reduction Ring (circular meter)
    g.setColour(juce::Colours::white);
    g.drawText("AMOUNT", area.removeFromTop(12), juce::Justification::centred);
    
    auto ringArea = area.reduced(5);
    auto center = ringArea.getCentre();
    float outerRadius = juce::jmin(ringArea.getWidth(), ringArea.getHeight()) * 0.4f;
    float innerRadius = outerRadius * 0.7f;
    
    // Background ring
    g.setColour(juce::Colour(0xFF333333));
    g.drawEllipse(center.x - outerRadius, center.y - outerRadius, outerRadius * 2, outerRadius * 2, 3.0f);
    
    // Active amount ring (270 degrees max)
    float amountLevel = 0.6f; // 60% transformer saturation
    float startAngle = -juce::MathConstants<float>::halfPi; // Start at top
    float endAngle = startAngle + (amountLevel * juce::MathConstants<float>::pi * 1.5f); // 270 degrees max
    
    juce::Path amountArc;
    amountArc.addCentredArc(center.x, center.y, outerRadius, outerRadius, 0.0f, startAngle, endAngle, true);
    
    g.setColour(juce::Colour(0xFFffaa44));
    g.strokePath(amountArc, juce::PathStrokeType(4.0f));
    
    // Center amount text
    g.setColour(juce::Colours::white);
    g.setFont(juce::Font("Arial", 10.0f, juce::Font::bold));
    g.drawText("60%", juce::Rectangle<int>(center.x - 15, center.y - 8, 30, 16), juce::Justification::centred);
}
