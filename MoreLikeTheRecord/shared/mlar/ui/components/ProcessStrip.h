#pragma once
#include <JuceHeader.h>
#include "../LookAndFeel_Mojo.h"

namespace moremojo {
namespace mlar {
namespace mojoUI {

/**
 * ProcessStrip - UI component representing a single process module
 * 
 * Features:
 * - Enable/disable toggle
 * - Main parameter control
 * - Open detail button
 * - Status indicator
 */
class ProcessStrip : public juce::Component {
public:
    ProcessStrip(const juce::String& name = "Process") : processName(name) {
        // Set up toggle button
        addAndMakeVisible(enableToggle);
        enableToggle.setButtonText(name);
        
        // Set up main slider
        addAndMakeVisible(mainSlider);
        mainSlider.setSliderStyle(juce::Slider::LinearHorizontal);
        mainSlider.setTextBoxStyle(juce::Slider::NoTextBox, true, 0, 0);
        mainSlider.setRange(0.0, 100.0);
        mainSlider.setValue(50.0);
        
        // Set up detail button
        addAndMakeVisible(detailButton);
        detailButton.setButtonText("⊞");
        detailButton.setTooltip("Open " + name + " details");
        
        // Register for slider changes to update status color
        mainSlider.onValueChange = [this]() {
            updateStatusColor();
            repaint();
        };
        
        // Register for toggle changes to update status
        enableToggle.onClick = [this]() {
            updateStatusColor();
            repaint();
        };
    }
    
    ~ProcessStrip() override = default;
    
    void paint(juce::Graphics& g) override {
        // Draw background
        juce::Rectangle<float> bounds = getLocalBounds().toFloat().reduced(2.0f);
        
        // Draw card with shadow
        mojoUI::LNF::card(g, bounds, juce::Colour(28, 28, 36));
        
        // Draw status indicator (colored circle)
        juce::Rectangle<float> statusBounds = bounds.removeFromRight(16.0f)
                                            .withSizeKeepingCentre(8.0f, 8.0f);
        
        g.setColour(statusColor);
        g.fillEllipse(statusBounds);
        g.setColour(juce::Colours::white.withAlpha(0.5f));
        g.drawEllipse(statusBounds, 1.0f);
    }
    
    void resized() override {
        auto bounds = getLocalBounds().reduced(8);
        
        // Position enable toggle button on left
        enableToggle.setBounds(bounds.removeFromLeft(80));
        
        // Position detail button on right
        detailButton.setBounds(bounds.removeFromRight(24).withSizeKeepingCentre(24, 24));
        
        // Position main slider in remaining space
        bounds.removeFromRight(20);  // Add space between slider and detail button
        mainSlider.setBounds(bounds);
    }
    
    /**
     * Set the module name
     * 
     * @param name New process module name
     */
    void setProcessName(const juce::String& name) {
        processName = name;
        enableToggle.setButtonText(name);
        detailButton.setTooltip("Open " + name + " details");
    }
    
    /**
     * Get the toggle button for parameter attachment
     */
    juce::ToggleButton& getToggleButton() { return enableToggle; }
    
    /**
     * Get the main slider for parameter attachment
     */
    juce::Slider& getSlider() { return mainSlider; }
    
    /**
     * Get the detail button to hook up click handler
     */
    juce::TextButton& getDetailButton() { return detailButton; }
    
    /**
     * Set the callback function for when detail button is clicked
     * 
     * @param callback Function to call when detail button is clicked
     */
    void setDetailCallback(std::function<void()> callback) {
        detailButton.onClick = std::move(callback);
    }
    
    /**
     * Set status color based on external metrics
     * 
     * @param value Status value (0.0-1.0, where 1.0 is good)
     */
    void setStatusValue(float value) {
        statusValue = juce::jlimit(0.0f, 1.0f, value);
        updateStatusColor();
        repaint();
    }
    
private:
    // Update status indicator color based on toggle state and value
    void updateStatusColor() {
        if (!enableToggle.getToggleState()) {
            // Grey when disabled
            statusColor = juce::Colours::darkgrey;
        } else {
            // Green to red based on status value
            float hue = statusValue * 0.3f;  // 0.3 = green, 0.0 = red
            statusColor = juce::Colour::fromHSV(hue, 0.8f, 0.9f, 1.0f);
        }
    }
    
    juce::String processName;
    juce::ToggleButton enableToggle;
    juce::Slider mainSlider;
    juce::TextButton detailButton;
    
    float statusValue = 0.5f;
    juce::Colour statusColor = juce::Colours::darkgrey;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ProcessStrip)
};

}}} // namespace moremojo::mlar::mojoUI
