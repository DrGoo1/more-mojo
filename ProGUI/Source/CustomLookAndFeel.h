#pragma once

#include "../../JUCE/modules/juce_core/juce_core.h"
#include "../../JUCE/modules/juce_events/juce_events.h"
#include "../../JUCE/modules/juce_graphics/juce_graphics.h"
#include "../../JUCE/modules/juce_gui_basics/juce_gui_basics.h"
#include "../../JUCE/modules/juce_gui_extra/juce_gui_extra.h"

//==============================================================================
/**
    Custom look and feel for professional 3D audio controls
*/
class CustomLookAndFeel : public juce::LookAndFeel_V4
{
public:
    CustomLookAndFeel();
    
    // Override rotary slider drawing for 3D knobs
    void drawRotarySlider(juce::Graphics& g, int x, int y, int width, int height, 
                          float sliderPos, float rotaryStartAngle, float rotaryEndAngle, 
                          juce::Slider& slider) override;
    
    // Override linear slider drawing for high-quality vertical sliders
    void drawLinearSlider(juce::Graphics& g, int x, int y, int width, int height,
                          float sliderPos, float minSliderPos, float maxSliderPos,
                          const juce::Slider::SliderStyle style, juce::Slider& slider) override;
    
    // Override button drawing for 3D buttons
    void drawButtonBackground(juce::Graphics& g, juce::Button& button, 
                              const juce::Colour& backgroundColour,
                              bool shouldDrawButtonAsHighlighted, 
                              bool shouldDrawButtonAsDown) override;
    
    // Override tab button drawing
    void drawTabButton(juce::TabBarButton& button, juce::Graphics& g, 
                       bool isMouseOver, bool isMouseDown) override;
    
    // Override tab area drawing
    void drawTabAreaBehindFrontButton(juce::TabbedButtonBar& bar, juce::Graphics& g, 
                                     int w, int h) override;

private:
    // Helper methods for drawing 3D effects
    void drawMetalKnob(juce::Graphics& g, int x, int y, int diameter, float sliderPos, 
                      const juce::Colour& knobColor, bool isEnabled);
    
    void draw3DButton(juce::Graphics& g, int x, int y, int width, int height, 
                     const juce::Colour& buttonColor, bool isHighlighted, bool isDown);
    
    void drawGloss(juce::Graphics& g, const juce::Rectangle<float>& area, float opacity);
    
    void drawInnerShadow(juce::Graphics& g, const juce::Rectangle<float>& area, float depth);
};
