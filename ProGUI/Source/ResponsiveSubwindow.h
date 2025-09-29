#pragma once

#include <JuceHeader.h>

/**
 * Base class for responsive professional subwindows
 * Provides consistent styling, layout, and responsive behavior for all process subwindows
 */
class ResponsiveSubwindow : public juce::Component, public juce::Timer
{
public:
    ResponsiveSubwindow(const juce::String& processName, const juce::String& subtitle);
    ~ResponsiveSubwindow() override = default;

    // Override these in derived classes
    virtual void paintPresetLibrary(juce::Graphics& g, float scaleX, float scaleY, float minScale) = 0;
    virtual void paintGlobalControls(juce::Graphics& g, float scaleX, float scaleY, float minScale) = 0;
    virtual void paintProcessingControls(juce::Graphics& g, float scaleX, float scaleY, float minScale) = 0;
    virtual void resizeControls(float scaleX, float scaleY) = 0;

    // Common implementation
    void paint(juce::Graphics& g) override;
    void resized() override;
    void timerCallback() override;

protected:
    // Common styling methods
    void paintBackground(juce::Graphics& g);
    void paintHeader(juce::Graphics& g, float minScale);
    
    // Layout constants
    static constexpr int DEFAULT_WIDTH = 1100;
    static constexpr int DEFAULT_HEIGHT = 1000;
    static constexpr int PRESET_Y = 80;
    static constexpr int GLOBAL_Y = 200;
    static constexpr int PROCESSING_Y = 330;

    juce::String processName;
    juce::String subtitle;
};
