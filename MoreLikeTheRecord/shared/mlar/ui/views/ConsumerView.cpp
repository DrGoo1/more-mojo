#include "ConsumerView.h"
#include "../../MLAR.h"

namespace moremojo {
namespace mlar {
namespace mojoUI {

ConsumerView::ConsumerView() {
    // Set up macro slider
    addAndMakeVisible(macroSlider);
    macroSlider.setSliderStyle(juce::Slider::LinearHorizontal);
    macroSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 80, 20);
    macroSlider.setRange(0.0, 100.0);
    macroSlider.setValue(0.0);
    macroSlider.setTextValueSuffix("%");
    macroSlider.setNumDecimalPlacesToDisplay(1);
    macroSlider.setDoubleClickReturnValue(true, 0.0);
    macroSlider.setColour(juce::Slider::trackColourId, juce::Colour(80, 110, 255));
    
    // Set up macro label
    addAndMakeVisible(macroLabel);
    macroLabel.setText("More Like The Record", juce::dontSendNotification);
    macroLabel.setJustificationType(juce::Justification::centred);
    macroLabel.setFont(juce::Font(20.0f, juce::Font::bold));
    
    // Set up tooltip label
    addAndMakeVisible(tooltipLabel);
    tooltipLabel.setJustificationType(juce::Justification::centred);
    tooltipLabel.setFont(juce::Font(14.0f));
    
    // Set up meters
    addAndMakeVisible(lufsMeter);
    lufsMeter.setTitle("LUFS");
    
    addAndMakeVisible(confidenceMeter);
    confidenceMeter.setTitle("Confidence");
    
    // Initialize macro descriptions
    macroDescriptions = {
        {0.0f, "Off - Original signal unprocessed"},
        {20.0f, "Light - Subtle transient enhancement"},
        {50.0f, "Medium - Balanced timing and phase correction"},
        {75.0f, "High - Robust alignment and analog continuity"},
        {100.0f, "Maximum - Full correction and analog warmth"}
    };
    
    // Set up slider value change listener
    macroSlider.onValueChange = [this]() {
        updateTooltip(static_cast<float>(macroSlider.getValue()));
    };
    
    // Initialize tooltip with default value
    updateTooltip(0.0f);
}

void ConsumerView::paint(juce::Graphics& g) {
    // Fill background
    g.fillAll(juce::Colour(15, 15, 20));
    
    // Draw shadow under the main panel
    juce::Rectangle<float> panelBounds = getLocalBounds().toFloat().reduced(20.0f);
    
    // Draw panel background
    juce::Colour panelColor(24, 24, 32);
    LNF::card(g, panelBounds, panelColor);
}

void ConsumerView::resized() {
    auto bounds = getLocalBounds().reduced(40);
    
    // Position label at top
    macroLabel.setBounds(bounds.removeFromTop(40));
    
    // Add some spacing
    bounds.removeFromTop(20);
    
    // Position meters side by side
    auto meterRow = bounds.removeFromTop(80);
    lufsMeter.setBounds(meterRow.removeFromLeft(meterRow.getWidth() / 2).reduced(10));
    confidenceMeter.setBounds(meterRow.reduced(10));
    
    // Add some spacing
    bounds.removeFromTop(30);
    
    // Position macro slider
    macroSlider.setBounds(bounds.removeFromTop(60));
    
    // Position tooltip below slider
    bounds.removeFromTop(20);
    tooltipLabel.setBounds(bounds.removeFromTop(30));
}

void ConsumerView::updateMeters(const struct MetricsFrame& metrics) {
    // Update LUFS meter with true peak value
    lufsMeter.setValue(metrics.truePeak);
    
    // Calculate confidence based on IACC stability and TEI score
    // IACC close to 1.0 and low ITD variance indicates good stereo image
    float confidence = 0.5f;
    
    if (metrics.iacc > 0.0f) {
        // Scale IACC from 0-1 to contribute 50% of confidence
        float iaccScore = metrics.iacc * 0.5f;
        
        // Low ITD variance (capped at 0.5) contributes remaining 50%
        float itdScore = 0.5f * (1.0f - juce::jlimit(0.0f, 1.0f, metrics.itdVar * 2.0f));
        
        confidence = iaccScore + itdScore;
    }
    
    confidenceMeter.setValue(confidence);
    
    // Trigger repaint
    repaint();
}

void ConsumerView::updateTooltip(float value) {
    // Find the closest description
    juce::String description;
    float minDistance = 100.0f;
    
    for (const auto& desc : macroDescriptions) {
        float distance = std::abs(desc.first - value);
        if (distance < minDistance) {
            minDistance = distance;
            description = desc.second;
        }
    }
    
    tooltipLabel.setText(description, juce::dontSendNotification);
}

}}} // namespace moremojo::mlar::mojoUI
