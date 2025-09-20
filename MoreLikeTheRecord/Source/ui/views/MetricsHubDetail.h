#pragma once
#include <JuceHeader.h>
#include "DetailWindows.h"
#include "../opengl/HeatmapGL.h"
#include "../widgets/TEIOverlay.h"
#include "../components/MojoMeters.h"
#include "../../metrics/GDSComputer.h"
#include "../../metrics/IACCAnalyzer.h"
#include "../../metrics/TEIAnalyzer.h"

// Forward declaration of processor class
class MoreMojoAudioProcessor;

namespace mojo {

// Function declarations for CSV and PNG export
void exportCSV(const std::vector<float>& data, int W, int H, const juce::File& file);
void exportPNG(juce::Component& component, const juce::File& file);

/**
 * MetricsHubDetail - Detailed metrics visualization window
 * 
 * Features:
 * - GDS heatmap with OpenGL acceleration
 * - IACC timeline visualization
 * - TEI waveform overlay with metrics
 * - Export functionality for CSV data and PNG screenshots
 */
class MetricsHubDetail : public DetailWindow, private juce::Timer {
public:
    /**
     * Constructor
     * 
     * @param p Reference to the plugin processor for data access
     */
    MetricsHubDetail(MoreMojoAudioProcessor& p) : DetailWindow("Metrics Hub"), processor(p) {
        // Initialize OpenGL heatmap and attach
        addAndMakeVisible(gdsHeatmap);
        gdsHeatmap.attach();
        
        // Add IACC HUD meter
        addAndMakeVisible(iaccHUD);
        
        // Add TEI waveform overlay
        addAndMakeVisible(teiOverlay);
        
        // Add export buttons
        addAndMakeVisible(exportCSVBtn);
        exportCSVBtn.setButtonText("Export CSV");
        
        addAndMakeVisible(exportPNGBtn);
        exportPNGBtn.setButtonText("Export PNG");
        
        // Set up button callbacks
        exportCSVBtn.onClick = [this]() {
            // Get GDS data from processor
            std::vector<float> grid;
            int width = 0, height = 0;
            
            // Assuming processor has a method to get this data
            processor.getLatestGDS(grid, width, height);
            
            if (!grid.empty() && width > 0 && height > 0) {
                // Ask user for save location
                juce::FileChooser chooser("Save GDS data as CSV", 
                                        juce::File::getSpecialLocation(juce::File::userDesktopDirectory)
                                        .getChildFile("MoreMojo_GDS.csv"), 
                                        "*.csv");
                
                if (chooser.browseForFileToSave(true)) {
                    // Export data to selected file
                    exportCSV(grid, width, height, chooser.getResult());
                }
            }
        };
        
        exportPNGBtn.onClick = [this]() {
            // Ask user for save location
            juce::FileChooser chooser("Save metrics as PNG",
                                    juce::File::getSpecialLocation(juce::File::userDesktopDirectory)
                                    .getChildFile("MoreMojo_Metrics.png"),
                                    "*.png");
            
            if (chooser.browseForFileToSave(true)) {
                // Export current view to selected file
                exportPNG(*this, chooser.getResult());
            }
        };
        
        // Start the update timer (30 FPS)
        startTimerHz(30);
    }
    
    /**
     * Destructor - ensure OpenGL resources are released
     */
    ~MetricsHubDetail() override {
        // Detach OpenGL context
        gdsHeatmap.detach();
        
        // Stop timer
        stopTimer();
    }
    
    /**
     * Timer callback - update visualization from processor data
     */
    void timerCallback() override {
        // Pull metrics data from processor
        
        // 1. Update GDS heatmap
        std::vector<float> gdsGrid;
        int gdsWidth = 0, gdsHeight = 0;
        
        // Assuming processor has methods to get this data
        processor.getLatestGDS(gdsGrid, gdsWidth, gdsHeight);
        
        if (!gdsGrid.empty() && gdsWidth > 0 && gdsHeight > 0) {
            gdsHeatmap.setData(std::move(gdsGrid), gdsWidth, gdsHeight);
        }
        
        // 2. Update IACC HUD meter
        std::vector<float> iaccLine;
        processor.getLatestIACC(iaccLine);
        
        if (!iaccLine.empty()) {
            float latestIACC = iaccLine.back();
            // Convert from -1...1 to 0...1 for meter display
            iaccHUD.setTarget(juce::jlimit(0.0f, 1.0f, (latestIACC + 1.0f) * 0.5f));
        }
        
        // 3. Update TEI waveform overlay
        // Assuming processor has these buffer and TEI fields
        const TEIAnalyzer& tei = processor.getTEI();
        teiOverlay.setData(
            processor.getLastPreBuffer(),
            processor.getLastPostBuffer(),
            tei.riseDeltaUs,
            tei.preRingDeltaDb,
            tei.overshootPct
        );
        
        // Trigger repaint
        repaint();
    }
    
    /**
     * Layout components
     */
    void resized() override {
        // Call base class implementation first
        DetailWindow::resized();
        
        auto bounds = getLocalBounds().reduced(16);
        bounds.removeFromTop(28);  // Space for title
        
        // GDS heatmap (group delay spread visualization)
        gdsHeatmap.setBounds(bounds.removeFromTop(200));
        bounds.removeFromTop(8);  // Spacing
        
        // IACC HUD meter
        iaccHUD.setBounds(bounds.removeFromTop(120));
        bounds.removeFromTop(8);  // Spacing
        
        // TEI waveform overlay
        teiOverlay.setBounds(bounds.removeFromTop(160));
        bounds.removeFromTop(8);  // Spacing
        
        // Export buttons
        auto buttonRow = bounds.removeFromTop(28);
        exportCSVBtn.setBounds(buttonRow.removeFromLeft(120));
        buttonRow.removeFromLeft(8);  // Spacing
        exportPNGBtn.setBounds(buttonRow.removeFromLeft(120));
    }
    
private:
    MoreMojoAudioProcessor& processor;
    
    // Visualization components
    mojoGL::HeatmapGL gdsHeatmap;
    LUFSMeter iaccHUD;
    TEIOverlay teiOverlay;
    
    // Export buttons
    juce::TextButton exportCSVBtn;
    juce::TextButton exportPNGBtn;
};

} // namespace mojo
