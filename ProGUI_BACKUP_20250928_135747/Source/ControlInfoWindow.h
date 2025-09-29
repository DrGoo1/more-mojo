#pragma once

#include <juce_gui_extra/juce_gui_extra.h>
#include <juce_gui_basics/juce_gui_basics.h>

/**
 * Detailed information window for processing controls
 * Provides comprehensive technical information with large, readable fonts
 */
class ControlInfoWindow : public juce::Component
{
public:
    struct ControlInfo
    {
        juce::String title;
        juce::String overview;
        juce::String technicalDetails;
        juce::String practicalUsage;
        juce::String commonSettings;
        juce::String troubleshooting;
        juce::String range;
        juce::Colour accentColor;
    };

    ControlInfoWindow(const ControlInfo& info);
    ~ControlInfoWindow() override = default;

    void paint(juce::Graphics& g) override;
    void resized() override;

private:
    ControlInfo controlInfo;
    
    // UI Components
    std::unique_ptr<juce::TextButton> closeButton;
    
    // Visual representation methods
    void drawVisualRepresentation(juce::Graphics& g, juce::Rectangle<int> area);
    void drawPassbandRolloffGraphic(juce::Graphics& g, juce::Rectangle<int> area);
    void drawStopbandAttenuationGraphic(juce::Graphics& g, juce::Rectangle<int> area);
    void drawTruePeakCeilingGraphic(juce::Graphics& g, juce::Rectangle<int> area);
    void drawLookaheadTimeGraphic(juce::Graphics& g, juce::Rectangle<int> area);
    void drawOversamplingFactorGraphic(juce::Graphics& g, juce::Rectangle<int> area);
    void drawAntiAliasingFilterGraphic(juce::Graphics& g, juce::Rectangle<int> area);
    
    // Layout constants
    static constexpr int WINDOW_WIDTH = 900;
    static constexpr int WINDOW_HEIGHT = 800;
    static constexpr int MARGIN = 30;
    static constexpr int SECTION_SPACING = 20;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ControlInfoWindow)
};
