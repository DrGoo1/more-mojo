#pragma once

#include <juce_core/juce_core.h>
#include <juce_events/juce_events.h>
#include <juce_graphics/juce_graphics.h>
#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_gui_extra/juce_gui_extra.h>
#include "ProEQComponent.h"
#include "ProCompressorComponent.h"
#include "CustomLookAndFeel.h"

//==============================================================================
/**
    Main component that contains all the Pro GUI elements
*/
class MainComponent : public juce::Component,
                      public juce::ChangeListener
{
public:
    MainComponent();
    ~MainComponent() override;

    void paint(juce::Graphics& g) override;
    void resized() override;
    
    // ChangeListener
    void changeListenerCallback(juce::ChangeBroadcaster* source) override;

private:
    // Custom look and feel for professional 3D controls
    CustomLookAndFeel customLookAndFeel;
    
    // Main tab component to organize different processor sections
    std::unique_ptr<juce::TabbedComponent> tabComponent;
    
    // Individual processor components
    std::unique_ptr<ProEQComponent> eqComponent;
    std::unique_ptr<ProCompressorComponent> compressorComponent;
    
    // Main control bar components
    juce::Label titleLabel;
    juce::TextButton settingsButton;
    juce::TextButton presetButton;
    
    // Status bar components
    juce::Label statusLabel;
    juce::Label cpuUsageLabel;
    
    // Initialize components
    void initializeComponents();
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainComponent)
};
