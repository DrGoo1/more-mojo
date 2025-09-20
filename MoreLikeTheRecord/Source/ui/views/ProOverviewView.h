#pragma once
#include <JuceHeader.h>
#include "../components/ProcessStrip.h"
#include "../components/MojoMeters.h"

namespace mojo {

/**
 * Pro UI overview showing all processing modules
 * Provides access to detailed controls via detail windows
 */
class ProOverviewView : public juce::Component {
public:
    ProOverviewView() {
        // Helper function to create SVG icons
        auto fromSvg = [&](const void* data, size_t size) {
            return std::unique_ptr<juce::Drawable>(juce::Drawable::createFromImageData(data, size));
        };
        
        // Add all process strips with their icons
        // Note: In a real implementation, we'd use BinaryData with embedded SVG assets
        strips.add(addStrip("More Like The Record", createDummyIcon("record")));
        strips.add(addStrip("TimingSafeResampler", createDummyIcon("filter")));
        strips.add(addStrip("TransientDetect", createDummyIcon("transient")));
        strips.add(addStrip("FractionalDelayAlign", createDummyIcon("clock")));
        strips.add(addStrip("AnalogContinuity", createDummyIcon("vinyl")));
        strips.add(addStrip("Limiter + LevelMatch", createDummyIcon("limiter")));
        strips.add(addStrip("Metrics Hub", createDummyIcon("metrics")));
        
        // Add master meters
        addAndMakeVisible(masterLUFS);
        addAndMakeVisible(masterCorr);
    }
    
    /** Callback for when a process detail should be opened */
    std::function<void(int)> onOpenDetail;
    
    void resized() override {
        auto bounds = getLocalBounds().reduced(16);
        
        // Left side: process strips
        auto left = bounds.removeFromLeft(juce::jmax(420, getWidth() / 2));
        for (int i = 0; i < strips.size(); ++i) {
            strips[i]->setBounds(left.removeFromTop(110).reduced(8));
        }
        
        // Right side: master meters
        bounds.removeFromTop(8);
        auto meters = bounds.removeFromTop(150);
        masterLUFS.setBounds(meters.removeFromLeft(140).reduced(6));
        masterCorr.setBounds(meters.removeFromLeft(200).reduced(6));
    }
    
private:
    juce::OwnedArray<ProcessStrip> strips;
    LUFSMeter masterLUFS;
    CorrelationMeter masterCorr;
    
    // Helper method to add a process strip
    ProcessStrip* addStrip(juce::String name, std::unique_ptr<juce::Drawable> icon) {
        auto* strip = new ProcessStrip(std::move(name), std::move(icon));
        addAndMakeVisible(strip);
        
        // Set up callback to open detail window
        strip->onOpenDetail = [this, strip] {
            if (onOpenDetail) {
                onOpenDetail(strips.indexOf(strip));
            }
        };
        
        return strip;
    }
    
    // Temporary helper to create placeholder icons
    // In a real implementation, we'd use BinaryData with proper SVG icons
    std::unique_ptr<juce::Drawable> createDummyIcon(const juce::String& name) {
        auto image = juce::Image(juce::Image::ARGB, 48, 48, true);
        juce::Graphics g(image);
        g.setColour(juce::Colours::white);
        g.drawEllipse(4.0f, 4.0f, 40.0f, 40.0f, 2.0f);
        g.drawFittedText(name.substring(0, 1).toUpperCase(), 
                         juce::Rectangle<int>(0, 0, 48, 48), 
                         juce::Justification::centred, 1);
        
        return juce::Drawable::createFromImage(image);
    }
};

}  // namespace mojo
