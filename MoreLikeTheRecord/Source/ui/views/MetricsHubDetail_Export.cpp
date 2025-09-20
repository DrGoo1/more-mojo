#include <JuceHeader.h>
#include "MetricsHubDetail.h"

namespace mojo {

/**
 * Export metrics data to CSV file
 * 
 * @param data The data grid to export
 * @param W Grid width
 * @param H Grid height
 * @param file Target file
 */
void exportCSV(const std::vector<float>& data, int W, int H, const juce::File& file) {
    // Create file output stream
    juce::FileOutputStream os(file);
    
    if (!os.openedOk()) {
        // Failed to open file
        return;
    }
    
    // Write CSV header for frequency values
    os << "Time\\Freq";
    for (int x = 0; x < W; ++x) {
        // Map x coordinate to frequency (log scale)
        const float freq = 20.0f * std::pow(10.0f, static_cast<float>(x) / static_cast<float>(W) * 3.0f); // 20 Hz to 20 kHz
        os << "," << juce::String(freq, 1);
    }
    os << "\n";
    
    // Write data rows
    for (int y = 0; y < H; ++y) {
        // Map y coordinate to group delay (0-5 ms)
        const float delay = 5.0f * static_cast<float>(H - y - 1) / static_cast<float>(H);
        os << juce::String(delay, 3);
        
        for (int x = 0; x < W; ++x) {
            os << "," << juce::String(data[static_cast<size_t>(y) * W + x]);
        }
        
        os << "\n";
    }
}

/**
 * Export component visualization as PNG image
 * 
 * @param component The component to capture
 * @param file Target file
 */
void exportPNG(juce::Component& component, const juce::File& file) {
    // Create an image with the component's size
    juce::Image image(juce::Image::ARGB, 
                     component.getWidth(), 
                     component.getHeight(), 
                     true);
    
    // Create graphics context for drawing
    juce::Graphics g(image);
    
    // Draw the component onto the image
    component.paintEntireComponent(g, false);
    
    // Save as PNG
    juce::PNGImageFormat pngFormat;
    std::unique_ptr<juce::OutputStream> os(file.createOutputStream());
    
    if (os != nullptr) {
        pngFormat.writeImageToStream(image, *os);
    }
}

} // namespace mojo
