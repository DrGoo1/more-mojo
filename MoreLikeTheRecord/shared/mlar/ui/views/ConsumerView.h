#pragma once
#include <JuceHeader.h>
#include "../components/MojoSlider.h"
#include "../components/MojoMeters.h"

namespace moremojo {
namespace mlar {
namespace mojoUI {

/**
 * ConsumerView - Simple UI view with a single "More Like The Record" macro slider
 * 
 * Features:
 * - Large macro slider for easy control
 * - Simple meters for immediate feedback
 * - Minimal, user-friendly design
 */
class ConsumerView : public juce::Component {
public:
    ConsumerView();
    ~ConsumerView() override = default;
    
    void paint(juce::Graphics&) override;
    void resized() override;
    
    /**
     * Get reference to the macro slider for parameter attachment
     */
    juce::Slider& getMacroSlider() { return macroSlider; }
    
    /**
     * Update meters with current metrics
     * 
     * @param metrics Current metrics frame
     */
    void updateMeters(const struct MetricsFrame& metrics);
    
private:
    // Main macro slider
    juce::Slider macroSlider;
    juce::Label macroLabel;
    
    // Simple meters
    LUFSMeter lufsMeter;
    ConfidenceMeter confidenceMeter;
    
    // Macro slider tooltip
    juce::Label tooltipLabel;
    
    // Macro descriptions for different positions
    std::vector<std::pair<float, juce::String>> macroDescriptions;
    
    // Update tooltip based on slider value
    void updateTooltip(float value);
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ConsumerView)
};

}}} // namespace moremojo::mlar::mojoUI
