#pragma once
#include <JuceHeader.h>

namespace moremojo {
namespace mlar {
namespace mojoUI {

/**
 * LNF - Custom Look and Feel for the MoreMojo UI
 * 
 * Features:
 * - High-DPI shaded controls with modern appearance
 * - Custom rendering for sliders, buttons, and meters
 * - Consistent color scheme and styling
 */
class LNF : public juce::LookAndFeel_V4 {
public:
    LNF();
    ~LNF() override = default;
    
    // Slider drawing methods
    void drawRotarySlider(juce::Graphics&, int x, int y, int width, int height, 
                          float sliderPos, float rotaryStartAngle, float rotaryEndAngle, 
                          juce::Slider&) override;
    
    void drawLinearSlider(juce::Graphics&, int x, int y, int width, int height, 
                          float sliderPos, float minSliderPos, float maxSliderPos, 
                          const juce::Slider::SliderStyle, juce::Slider&) override;
    
    // Button drawing methods
    void drawToggleButton(juce::Graphics&, juce::ToggleButton&, 
                         bool shouldDrawButtonAsHighlighted, 
                         bool shouldDrawButtonAsDown) override;
    
    // Combo box drawing methods
    void drawComboBox(juce::Graphics&, int width, int height, bool isButtonDown, 
                     int buttonX, int buttonY, int buttonW, int buttonH, 
                     juce::ComboBox&) override;
    
    // Popup menu drawing
    void drawPopupMenuBackground(juce::Graphics&, int width, int height) override;
    
    void drawPopupMenuItem(juce::Graphics&, const juce::Rectangle<int>&, 
                          bool isSeparator, bool isActive, bool isHighlighted, 
                          bool isTicked, bool hasSubMenu, const juce::String& text, 
                          const juce::String& shortcutKeyText, 
                          const juce::Drawable* icon, const juce::Colour* textColour) override;
    
    // Tab button drawing
    void drawTabButton(juce::Graphics&, juce::TabBarButton&, 
                      bool isMouseOver, bool isMouseDown) override;
    
    // Group component drawing
    void drawGroupComponentOutline(juce::Graphics&, int w, int h, const juce::String& text, 
                                  const juce::Justification&, juce::GroupComponent&) override;
    
    // Utility drawing methods
    static void card(juce::Graphics& g, juce::Rectangle<float> bounds, juce::Colour color);
    static void drawShadow(juce::Graphics& g, juce::Rectangle<float> bounds, float depth);
    static void drawInnerShadow(juce::Graphics& g, juce::Rectangle<float> bounds, float depth);
    
    // Color scheme
    juce::Colour getBackgroundColor() const { return backgroundColor; }
    juce::Colour getForegroundColor() const { return foregroundColor; }
    juce::Colour getHighlightColor() const { return highlightColor; }
    juce::Colour getAccentColor() const { return accentColor; }
    
private:
    // Color scheme
    juce::Colour backgroundColor { 15, 15, 20 };
    juce::Colour foregroundColor { 230, 230, 235 };
    juce::Colour highlightColor { 80, 110, 255 };
    juce::Colour accentColor { 230, 100, 90 };
    
    // Slider knob image cache
    juce::Image knobImage;
    
    // Helper to create knob image
    void createKnobImage();
};

}}} // namespace moremojo::mlar::mojoUI
