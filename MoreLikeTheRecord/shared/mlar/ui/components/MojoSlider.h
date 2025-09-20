#pragma once
#include <JuceHeader.h>
#include "../LookAndFeel_Mojo.h"

namespace moremojo {
namespace mlar {
namespace mojoUI {

/**
 * MojoSlider - Customized slider with modern look and feel
 * 
 * Features:
 * - High DPI rendering
 * - Shadow effects
 * - Modern appearance
 */
class MojoSlider : public juce::Slider {
public:
    MojoSlider() {
        // Default configuration
        setSliderStyle(juce::Slider::SliderStyle::LinearHorizontal);
        setTextBoxStyle(juce::Slider::TextBoxBelow, false, 80, 20);
        setRange(0.0, 100.0);
        setValue(50.0);
        setNumDecimalPlacesToDisplay(1);
    }
    
    ~MojoSlider() override = default;
    
    /**
     * Create a rotary-style slider
     */
    static MojoSlider createRotarySlider() {
        MojoSlider slider;
        slider.setSliderStyle(juce::Slider::SliderStyle::RotaryHorizontalVerticalDrag);
        slider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 60, 15);
        return slider;
    }
    
    /**
     * Create a horizontal slider
     */
    static MojoSlider createHorizontalSlider() {
        MojoSlider slider;
        slider.setSliderStyle(juce::Slider::SliderStyle::LinearHorizontal);
        slider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 80, 20);
        return slider;
    }
    
    /**
     * Create a vertical slider
     */
    static MojoSlider createVerticalSlider() {
        MojoSlider slider;
        slider.setSliderStyle(juce::Slider::SliderStyle::LinearVertical);
        slider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 60, 15);
        return slider;
    }
};

}}} // namespace moremojo::mlar::mojoUI
