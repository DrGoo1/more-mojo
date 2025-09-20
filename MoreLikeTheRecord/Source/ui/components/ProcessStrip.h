#pragma once
#include <JuceHeader.h>
#include "../LookAndFeel_Mojo.h"
#include "MojoIcon.h"

namespace mojo {

/**
 * Process strip component for the Pro UI
 * Displays a single processing module with controls and meter
 */
class ProcessStrip : public juce::Component {
public:
    ProcessStrip(juce::String title, std::unique_ptr<juce::Drawable> icon) 
    : name(std::move(title)) {
        // Add and make visible the icon component
        addAndMakeVisible(iconComp = std::make_unique<MojoIcon>(std::move(icon)));
        
        // Set up power button
        addAndMakeVisible(power);
        power.setButtonText("ON");
        power.setClickingTogglesState(true);
        
        // Set up amount slider
        addAndMakeVisible(amount);
        amount.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
        amount.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 56, 20);
        amount.setRange(0.0, 100.0, 0.1);
        
        // Set up detail button
        addAndMakeVisible(openDetail);
        openDetail.setButtonText("Details");
        openDetail.onClick = [this] { 
            if (onOpenDetail) onOpenDetail(); 
        };
    }
    
    /** Callback function for when the detail button is clicked */
    std::function<void()> onOpenDetail;
    
    void paint(juce::Graphics& g) override {
        // Draw the background card
        LookAndFeel_Mojo::drawCard(g, getLocalBounds().toFloat(), 
                                  juce::Colour(30, 30, 40), 12.f, 10.f);
        
        // Draw the title
        g.setColour(juce::Colours::white);
        g.setFont(juce::Font(16.f, juce::Font::bold));
        g.drawFittedText(name, getLocalBounds().reduced(12).removeFromTop(22), 
                        juce::Justification::left, 1);
    }
    
    void resized() override {
        auto bounds = getLocalBounds().reduced(12);
        bounds.removeFromTop(26); // Space for title
        
        // Layout for icon
        auto left = bounds.removeFromLeft(60);
        iconComp->setBounds(left);
        
        // Layout for controls
        auto controls = bounds.reduced(6);
        power.setBounds(controls.removeFromLeft(80));
        amount.setBounds(controls.removeFromLeft(160));
        openDetail.setBounds(controls.removeFromRight(90));
    }
    
    // Public controls for parameter attachment
    juce::ToggleButton power;
    juce::Slider amount;
    juce::TextButton openDetail;
    
private:
    juce::String name;
    std::unique_ptr<MojoIcon> iconComp;
};

}  // namespace mojo
