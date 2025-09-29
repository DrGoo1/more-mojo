#include "JitterSubwindow.h"

JitterSubwindow::JitterSubwindow()
    : ProcessSubwindow("JITTER & ACCUMULATION", "Ensures ultra-stable timing for improved depth and spatial stability")
{
}

void JitterSubwindow::createControls()
{
    // Jitter controls
    jitterRMSKnob = createKnob("Jitter RMS", 0.0f, 1000.0f, 100.0f);
    
    juce::StringArray spectrums = {"White", "Pink", "Brown", "Blue"};
    jitterSpectrumCombo = createComboBox("Jitter Spectrum", spectrums);
    
    // Accumulator bit depth
    juce::StringArray bitDepths = {"16-bit", "20-bit", "24-bit", "32-bit", "Auto"};
    accumBitDepthCombo = createComboBox("Accum Bit-Depth", bitDepths);
    accumBitDepthCombo->setSelectedItemIndex(4); // Default to Auto
    
    // Quantization mode
    juce::StringArray quantModes = {"Round", "Truncate", "Shaped"};
    quantModeCombo = createComboBox("Quant Mode", quantModes);
    quantModeCombo->setSelectedItemIndex(2); // Default to Shaped
    
    // Dither controls
    juce::StringArray ditherTypes = {"None", "TPDF", "RPDF", "Shaped", "Advanced"};
    ditherTypeCombo = createComboBox("Dither Type", ditherTypes);
    ditherTypeCombo->setSelectedItemIndex(3); // Default to Shaped
    
    ditherLevelKnob = createKnob("Dither Level", -6.0f, 6.0f, 0.0f);
    noiseFloorKnob = createKnob("Noise Floor Target", -120.0f, -60.0f, -96.0f);
}

void JitterSubwindow::layoutControls(juce::Rectangle<int> controlArea)
{
    // Top section: Jitter controls
    auto jitterSection = controlArea.removeFromTop(80);
    auto jitterLeft = jitterSection.removeFromLeft(jitterSection.getWidth() / 2);
    
    jitterRMSKnob->setBounds(jitterLeft.reduced(10));
    jitterSpectrumCombo->setBounds(jitterSection.reduced(10));
    
    controlArea.removeFromTop(10); // Gap
    
    // Middle section: Bit depth and quantization
    auto middleSection = controlArea.removeFromTop(60);
    auto middleLeft = middleSection.removeFromLeft(middleSection.getWidth() / 2);
    
    accumBitDepthCombo->setBounds(middleLeft.reduced(5));
    quantModeCombo->setBounds(middleSection.reduced(5));
    
    controlArea.removeFromTop(10); // Gap
    
    // Bottom section: Dither controls
    auto bottomSection = controlArea;
    auto ditherWidth = bottomSection.getWidth() / 3;
    
    ditherTypeCombo->setBounds(bottomSection.removeFromLeft(ditherWidth).reduced(5));
    ditherLevelKnob->setBounds(bottomSection.removeFromLeft(ditherWidth).reduced(10));
    noiseFloorKnob->setBounds(bottomSection.reduced(10));
}

void JitterSubwindow::paintMeters(juce::Graphics& g, juce::Rectangle<int> meterArea)
{
    // Meter background
    g.setColour(juce::Colour(0xFF0f3460));
    g.fillRoundedRectangle(meterArea.toFloat().reduced(5), 6.0f);
    
    auto area = meterArea.reduced(10);
    
    // Noise Spectrum meter
    auto noiseArea = area.removeFromTop(area.getHeight() / 3);
    g.setColour(juce::Colours::white);
    g.setFont(juce::Font("Arial", 9.0f, juce::Font::bold));
    g.drawText("NOISE", noiseArea.removeFromTop(12), juce::Justification::centred);
    g.drawText("SPECTRUM", noiseArea.removeFromTop(12), juce::Justification::centred);
    
    // Draw spectrum bars
    auto spectrumArea = noiseArea.reduced(2);
    int numBars = 8;
    int barWidth = spectrumArea.getWidth() / numBars;
    
    for (int i = 0; i < numBars; ++i)
    {
        auto barArea = spectrumArea.removeFromLeft(barWidth).reduced(1);
        float height = 0.3f + (i * 0.1f); // Varying heights
        auto activeBar = barArea.removeFromBottom((int)(barArea.getHeight() * height));
        
        g.setColour(juce::Colour(0xFF44ff44));
        g.fillRect(activeBar);
    }
    
    area.removeFromTop(10); // Gap
    
    // THD+N meter
    auto thdArea = area.removeFromTop(area.getHeight() / 2);
    g.setColour(juce::Colours::white);
    g.drawText("THD+N", thdArea.removeFromTop(15), juce::Justification::centred);
    
    g.setColour(juce::Colour(0xFF333333));
    g.fillRoundedRectangle(thdArea.toFloat().reduced(2), 2.0f);
    
    float thdLevel = 0.15f; // Low THD+N is good
    auto thdActive = thdArea.removeFromLeft((int)(thdArea.getWidth() * thdLevel));
    g.setColour(juce::Colour(0xFF44ff44));
    g.fillRoundedRectangle(thdActive.toFloat().reduced(2), 2.0f);
    
    area.removeFromTop(10); // Gap
    
    // Time-variance meter
    g.setColour(juce::Colours::white);
    g.drawText("TIME-VAR", area.removeFromTop(15), juce::Justification::centred);
    
    g.setColour(juce::Colour(0xFF333333));
    g.fillRoundedRectangle(area.toFloat().reduced(2), 2.0f);
    
    float varianceLevel = 0.1f; // Low variance is good
    auto varianceActive = area.removeFromLeft((int)(area.getWidth() * varianceLevel));
    g.setColour(juce::Colour(0xFF4444ff));
    g.fillRoundedRectangle(varianceActive.toFloat().reduced(2), 2.0f);
}
