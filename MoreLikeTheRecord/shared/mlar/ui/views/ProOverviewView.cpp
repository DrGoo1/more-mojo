#include "ProOverviewView.h"
#include "../../MLAR.h"

namespace moremojo {
namespace mlar {
namespace mojoUI {

ProOverviewView::ProOverviewView() {
    // Create process strips
    processStrips.clear();
    
    // Resampler strip
    auto resamplerStrip = std::make_unique<ProcessStrip>("Resampler");
    resamplerStrip->setDetailCallback([this]() {
        if (detailCallback) detailCallback(0);
    });
    processStrips.push_back(std::move(resamplerStrip));
    
    // Transient strip
    auto transientStrip = std::make_unique<ProcessStrip>("Transient");
    transientStrip->setDetailCallback([this]() {
        if (detailCallback) detailCallback(1);
    });
    processStrips.push_back(std::move(transientStrip));
    
    // Align strip
    auto alignStrip = std::make_unique<ProcessStrip>("Align");
    alignStrip->setDetailCallback([this]() {
        if (detailCallback) detailCallback(2);
    });
    processStrips.push_back(std::move(alignStrip));
    
    // Analog strip
    auto analogStrip = std::make_unique<ProcessStrip>("Analog");
    analogStrip->setDetailCallback([this]() {
        if (detailCallback) detailCallback(3);
    });
    processStrips.push_back(std::move(analogStrip));
    
    // Limiter strip
    auto limiterStrip = std::make_unique<ProcessStrip>("Limiter");
    limiterStrip->setDetailCallback([this]() {
        if (detailCallback) detailCallback(4);
    });
    processStrips.push_back(std::move(limiterStrip));
    
    // Add all strips to component
    for (auto& strip : processStrips) {
        addAndMakeVisible(*strip);
    }
    
    // Create macro strip
    macroStrip = std::make_unique<ProcessStrip>("Macro");
    macroStrip->setDetailCallback([this]() {
        if (detailCallback) detailCallback(5);
    });
    addAndMakeVisible(*macroStrip);
    
    // Set up meters
    addAndMakeVisible(lufsMeter);
    lufsMeter.setTitle("LUFS");
    
    addAndMakeVisible(phaseMeter);
    phaseMeter.setTitle("Phase");
    
    // Add metrics hub button (using a custom process strip)
    auto metricsStrip = std::make_unique<ProcessStrip>("Metrics Hub");
    metricsStrip->setDetailCallback([this]() {
        if (detailCallback) detailCallback(6);
    });
    processStrips.push_back(std::move(metricsStrip));
}

void ProOverviewView::paint(juce::Graphics& g) {
    // Fill background
    g.fillAll(juce::Colour(15, 15, 20));
    
    // Draw shadow under the main panel
    juce::Rectangle<float> panelBounds = getLocalBounds().toFloat().reduced(20.0f);
    
    // Draw panel background
    juce::Colour panelColor(24, 24, 32);
    LNF::card(g, panelBounds, panelColor);
    
    // Draw limiting indicator if active
    if (limitingActive) {
        g.setColour(juce::Colours::red.withAlpha(0.7f));
        juce::Rectangle<float> indicatorBounds = panelBounds.getTopRight()
                                              .translated(-30.0f, 30.0f)
                                              .withSizeKeepingCentre(10.0f, 10.0f);
        g.fillEllipse(indicatorBounds);
    }
    
    // Draw transient activity indicator
    if (transientCount > 0.0f) {
        g.setColour(juce::Colours::yellow.withAlpha(transientCount));
        juce::Rectangle<float> indicatorBounds = panelBounds.getTopRight()
                                              .translated(-50.0f, 30.0f)
                                              .withSizeKeepingCentre(10.0f, 10.0f);
        g.fillEllipse(indicatorBounds);
        
        // Decay transient indicator
        transientCount *= 0.9f;
        if (transientCount < 0.01f) {
            transientCount = 0.0f;
        } else {
            // Trigger repaint for animation
            juce::Timer::callAfterDelay(50, [this]() { repaint(); });
        }
    }
}

void ProOverviewView::resized() {
    auto bounds = getLocalBounds().reduced(40);
    
    // Position macro strip at top
    if (macroStrip) {
        macroStrip->setBounds(bounds.removeFromTop(50));
        bounds.removeFromTop(10);  // Add spacing
    }
    
    // Position meters side by side
    auto meterRow = bounds.removeFromTop(80);
    lufsMeter.setBounds(meterRow.removeFromLeft(meterRow.getWidth() / 2).reduced(10));
    phaseMeter.setBounds(meterRow.reduced(10));
    
    // Add some spacing
    bounds.removeFromTop(20);
    
    // Position process strips
    const int stripHeight = 50;
    const int stripSpacing = 10;
    
    for (auto& strip : processStrips) {
        strip->setBounds(bounds.removeFromTop(stripHeight));
        bounds.removeFromTop(stripSpacing);
    }
}

void ProOverviewView::updateMeters(const struct MetricsFrame& metrics) {
    // Update LUFS meter with true peak value
    lufsMeter.setValue(metrics.truePeak);
    
    // Update phase correlation meter with IACC value
    // Scale from -1...1 to 0...1
    phaseMeter.setValue((metrics.iacc + 1.0f) * 0.5f);
    
    // Update process strip status indicators
    if (processStrips.size() >= 5) {
        // Resampler - status based on intersample peak detection
        processStrips[0]->setStatusValue(metrics.truePeak < 0.95f ? 1.0f : 0.0f);
        
        // Transient - status based on TEI score
        float teiScore = std::exp(-std::abs(metrics.teiRiseUs) / 500.0f);
        processStrips[1]->setStatusValue(teiScore);
        
        // Align - status based on ITD variance (lower is better)
        float itdScore = 1.0f - juce::jlimit(0.0f, 1.0f, metrics.itdVar * 2.0f);
        processStrips[2]->setStatusValue(itdScore);
        
        // Analog - fixed status (no quality metric)
        processStrips[3]->setStatusValue(0.5f);
        
        // Limiter - status based on headroom
        float headroomScore = 1.0f - juce::jlimit(0.0f, 1.0f, metrics.truePeak);
        processStrips[4]->setStatusValue(headroomScore);
        
        // Metrics Hub - overall score (average of others)
        if (processStrips.size() >= 6) {
            float overallScore = (teiScore + itdScore + headroomScore) / 3.0f;
            processStrips[5]->setStatusValue(overallScore);
        }
    }
    
    // Check if limiting is active
    limitingActive = metrics.truePeak > 0.98f;
    
    // Check for transient detection
    if (std::abs(metrics.teiRiseUs) > 1.0f) {
        transientCount = 1.0f;  // Activate transient indicator
    }
    
    // Trigger repaint
    repaint();
}

ProcessStrip& ProOverviewView::getProcessStrip(int index) {
    jassert(index >= 0 && index < static_cast<int>(processStrips.size()));
    return *processStrips[juce::jlimit(0, static_cast<int>(processStrips.size()) - 1, index)];
}

}}} // namespace moremojo::mlar::mojoUI
