#pragma once
#include <JuceHeader.h>
#include "../LookAndFeel_Mojo.h"
#include "../components/MojoMeters.h"

namespace mojo {

/**
 * Consumer UI view with simplified "More Like The Record" controls
 * Provides a single macro slider and minimal meters
 */
class ConsumerView : public juce::Component {
public:
    ConsumerView() {
        // Set up title label
        addAndMakeVisible(title);
        title.setText("More Like The Record", juce::dontSendNotification);
        title.setJustificationType(juce::Justification::centredLeft);
        title.setFont(juce::Font(20.f, juce::Font::bold));
        
        // Set up macro slider
        addAndMakeVisible(macro);
        macro.setSliderStyle(juce::Slider::LinearHorizontal);
        macro.setRange(0.0, 100.0, 0.1);
        macro.setTextValueSuffix(" %");
        macro.setTextBoxStyle(juce::Slider::TextBoxRight, false, 60, 24);
        
        // Set up A/B comparison button
        addAndMakeVisible(abBtn);
        abBtn.setButtonText("A/B");
        
        // Set up confidence indicator
        addAndMakeVisible(confidence);
        confidence.setText("◎", juce::dontSendNotification);
        confidence.setJustificationType(juce::Justification::centred);
        confidence.setFont(juce::Font(24.f));
        
        // Set up meters
        addAndMakeVisible(lufs);
        addAndMakeVisible(corr);
    }
    
    // Access to controls for parameter attachment
    juce::Slider& getMacro() { return macro; }
    juce::TextButton& getAB() { return abBtn; }
    
    void paint(juce::Graphics& g) override {
        // Fill background
        g.fillAll(findColour(juce::ResizableWindow::backgroundColourId));
        
        // Draw main card
        LookAndFeel_Mojo::drawCard(g, getLocalBounds().toFloat().reduced(8), 
                                 juce::Colour(24, 24, 32), 16.f, 12.f);
    }
    
    void resized() override {
        auto bounds = getLocalBounds().reduced(20);
        
        // Title at the top
        title.setBounds(bounds.removeFromTop(28));
        bounds.removeFromTop(10);
        
        // Macro slider
        macro.setBounds(bounds.removeFromTop(56));
        bounds.removeFromTop(10);
        
        // A/B button
        abBtn.setBounds(bounds.removeFromTop(32).removeFromLeft(80));
        bounds.removeFromTop(8);
        
        // Meters and confidence indicator
        auto meterRow = bounds.removeFromTop(120);
        lufs.setBounds(meterRow.removeFromLeft(120).reduced(4));
        corr.setBounds(meterRow.removeFromLeft(160).reduced(4));
        confidence.setBounds(meterRow.removeFromLeft(60));
    }
    
    // Update confidence indicator based on processing quality
    void setConfidenceLevel(float level) {
        // 0.0 = amber, 1.0 = green
        if (level > 0.8f) {
            confidence.setColour(juce::Label::textColourId, juce::Colour(80, 220, 100)); // Green
        } else if (level > 0.4f) {
            confidence.setColour(juce::Label::textColourId, juce::Colour(255, 200, 50)); // Amber
        } else {
            confidence.setColour(juce::Label::textColourId, juce::Colours::grey); // Grey
        }
    }
    
private:
    juce::Label title, confidence;
    juce::Slider macro;
    juce::TextButton abBtn;
    
    // Meters
    LUFSMeter lufs;
    CorrelationMeter corr;
};

}  // namespace mojo
