#pragma once
#include <JuceHeader.h>
#include "../components/ProcessStrip.h"
#include "../components/MojoMeters.h"

namespace moremojo {
namespace mlar {
namespace mojoUI {

/**
 * ProOverviewView - Advanced UI view with individual process control strips
 * 
 * Features:
 * - Process strips for each DSP module
 * - Meters for real-time feedback
 * - Access to detailed control panels
 */
class ProOverviewView : public juce::Component {
public:
    ProOverviewView();
    ~ProOverviewView() override = default;
    
    void paint(juce::Graphics&) override;
    void resized() override;
    
    /**
     * Update meters with current metrics
     * 
     * @param metrics Current metrics frame
     */
    void updateMeters(const struct MetricsFrame& metrics);
    
    /**
     * Set callback for opening detail windows
     * 
     * @param callback Function called with the module index when detail is requested
     */
    void onOpenDetail(std::function<void(int)> callback) {
        detailCallback = std::move(callback);
    }
    
    /**
     * Access individual process strips
     * 
     * @param index Process strip index
     * @return Reference to the process strip
     */
    ProcessStrip& getProcessStrip(int index);
    
    /**
     * Get the total number of process strips
     */
    int getNumProcessStrips() const { return static_cast<int>(processStrips.size()); }
    
private:
    // Process strips for each module
    std::vector<std::unique_ptr<ProcessStrip>> processStrips;
    
    // Meters
    LUFSMeter lufsMeter;
    PhaseCorrelationMeter phaseMeter;
    
    // Special macro strip
    std::unique_ptr<ProcessStrip> macroStrip;
    
    // Detail window callback
    std::function<void(int)> detailCallback;
    
    // Master indicators
    bool limitingActive = false;
    float transientCount = 0.0f;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ProOverviewView)
};

}}} // namespace moremojo::mlar::mojoUI
