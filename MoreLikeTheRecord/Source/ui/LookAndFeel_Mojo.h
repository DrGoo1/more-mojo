#pragma once
#include <JuceHeader.h>

namespace mojo {

/**
 * Custom Look and Feel class for the More Like The Record plugin
 * Provides stylish, shadowed UI elements with high-DPI support
 */
class LookAndFeel_Mojo : public juce::LookAndFeel_V4 {
public:
    LookAndFeel_Mojo();
    
    // Utility method to draw a card with shadow and gradient
    static void drawCard(juce::Graphics& g, juce::Rectangle<float> bounds, 
                         juce::Colour fill, float cornerRadius = 10.f, float elevation = 8.f);
    
    // Override LookAndFeel methods for UI elements
    void drawRotarySlider(juce::Graphics&, int x, int y, int width, int height,
                          float sliderPos, float rotaryStartAngle, float rotaryEndAngle,
                          juce::Slider&) override;
    
    void drawButtonBackground(juce::Graphics&, juce::Button&, const juce::Colour& backgroundColour,
                              bool shouldDrawButtonAsHighlighted, bool shouldDrawButtonAsDown) override;
    
    void drawToggleButton(juce::Graphics&, juce::ToggleButton&,
                          bool shouldDrawButtonAsHighlighted, bool shouldDrawButtonAsDown) override;
    
    // Palette structure for consistent colors
    struct Palette {
        juce::Colour bg = juce::Colour(18, 18, 24);
        juce::Colour card = juce::Colour(30, 30, 40);
        juce::Colour panel = juce::Colour(24, 24, 32);
        juce::Colour accent = juce::Colour(120, 80, 255);
        juce::Colour ok = juce::Colour(80, 220, 100);
        juce::Colour warning = juce::Colour(255, 180, 50);
        juce::Colour textMain = juce::Colours::white;
        juce::Colour textDim = juce::Colours::lightgrey;
    };
    
    Palette palette;
};

}  // namespace mojo
