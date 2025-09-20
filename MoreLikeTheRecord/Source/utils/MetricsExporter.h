#pragma once
#include <JuceHeader.h>
#include "MetricsAnalyzer.h"

namespace mojo {

/**
 * MetricsExporter - Exports metrics data to CSV and PNG files
 */
class MetricsExporter {
public:
    MetricsExporter() = default;
    
    // Export metrics data to CSV file
    bool exportCSV(const juce::File& file, const MetricsFrame& metrics) {
        // Create CSV content
        juce::String csvContent = "Metric,Value\n";
        
        // Add GDS score
        csvContent += "GDS Score," + juce::String(metrics.gdsScore) + "\n";
        
        // Add IACC/ITD metrics
        csvContent += "IACC," + juce::String(metrics.iacc) + "\n";
        csvContent += "ITD (ms)," + juce::String(metrics.itdMs) + "\n";
        csvContent += "ITD Variance (ms)," + juce::String(metrics.itdVariance) + "\n";
        
        // Add TEI metrics
        csvContent += "TEI Rise Time Delta (µs)," + juce::String(metrics.teiRiseTime) + "\n";
        csvContent += "TEI Pre-Ring Energy (dB)," + juce::String(metrics.teiPreRing) + "\n";
        csvContent += "TEI Overshoot (%)," + juce::String(metrics.teiOvershoot) + "\n";
        csvContent += "TEI Score," + juce::String(metrics.teiScore) + "\n";
        
        // Add residual metrics
        csvContent += "Residual RMS (dB)," + juce::String(metrics.residualRMS) + "\n";
        csvContent += "Residual Tilt (dB/oct)," + juce::String(metrics.residualTilt) + "\n";
        csvContent += "Residual Crest Factor (dB)," + juce::String(metrics.residualCrest) + "\n";
        
        // Add level metrics
        csvContent += "LUFS A (dB)," + juce::String(metrics.lufsA) + "\n";
        csvContent += "LUFS B (dB)," + juce::String(metrics.lufsB) + "\n";
        csvContent += "True Peak A (dB)," + juce::String(metrics.truePeakA) + "\n";
        csvContent += "True Peak B (dB)," + juce::String(metrics.truePeakB) + "\n";
        
        // Add composite score
        csvContent += "SCPI Score," + juce::String(metrics.scpiScore) + "\n";
        
        // Write to file
        return file.replaceWithText(csvContent);
    }
    
    // Export GDS heatmap to PNG file
    bool exportGDSHeatmap(const juce::File& file, const MetricsFrame& metrics) {
        // Create image with appropriate size
        const int width = metrics.gdsWidth > 0 ? metrics.gdsWidth : 256;
        const int height = metrics.gdsHeight > 0 ? metrics.gdsHeight : 128;
        
        juce::Image image(juce::Image::RGB, width, height, true);
        juce::Graphics g(image);
        
        // Fill background
        g.fillAll(juce::Colour(24, 24, 32));
        
        // Draw heatmap
        if (!metrics.gdsData.empty()) {
            for (int y = 0; y < height; ++y) {
                for (int x = 0; x < width; ++x) {
                    const int index = y * width + x;
                    if (index < static_cast<int>(metrics.gdsData.size())) {
                        // Get value and convert to color
                        const float value = metrics.gdsData[index];
                        const juce::Colour color = getHeatmapColor(value);
                        
                        // Draw pixel
                        g.setColour(color);
                        g.fillRect(x, y, 1, 1);
                    }
                }
            }
        }
        
        // Add frequency axis labels
        g.setColour(juce::Colours::white);
        g.setFont(juce::Font(10.0f));
        
        const float freqPoints[] = {20.0f, 100.0f, 1000.0f, 10000.0f, 20000.0f};
        const juce::String freqLabels[] = {"20Hz", "100Hz", "1kHz", "10kHz", "20kHz"};
        
        for (int i = 0; i < 5; ++i) {
            // Calculate position (logarithmic)
            const float normX = std::log10(freqPoints[i] / 20.0f) / std::log10(20000.0f / 20.0f);
            const int x = static_cast<int>(normX * width);
            
            // Draw label
            g.drawText(freqLabels[i], x - 15, height - 15, 30, 12, juce::Justification::centred, true);
        }
        
        // Add group delay axis labels
        const float delayPoints[] = {0.0f, 1.0f, 2.0f, 5.0f};
        const juce::String delayLabels[] = {"0ms", "1ms", "2ms", "5ms"};
        
        for (int i = 0; i < 4; ++i) {
            // Calculate position (linear)
            const float normY = 1.0f - (delayPoints[i] / 5.0f);
            const int y = static_cast<int>(normY * height);
            
            // Draw label
            g.drawText(delayLabels[i], 2, y - 6, 30, 12, juce::Justification::left, true);
        }
        
        // Add title
        g.setFont(juce::Font(12.0f, juce::Font::bold));
        g.drawText("Group Delay Spread (GDS)", 0, 2, width, 20, juce::Justification::centred, true);
        g.drawText("Score: " + juce::String(metrics.gdsScore, 2), 0, 20, width, 20, juce::Justification::centred, true);
        
        // Save to file
        juce::PNGImageFormat pngFormat;
        juce::FileOutputStream outputStream(file);
        
        if (outputStream.openedOk()) {
            return pngFormat.writeImageToStream(image, outputStream);
        }
        
        return false;
    }
    
    // Export residual spectrum to PNG file
    bool exportResidualSpectrum(const juce::File& file, const MetricsFrame& metrics) {
        // Create image with appropriate size
        const int width = 512;
        const int height = 256;
        
        juce::Image image(juce::Image::RGB, width, height, true);
        juce::Graphics g(image);
        
        // Fill background
        g.fillAll(juce::Colour(24, 24, 32));
        
        // Draw spectrum
        if (!metrics.residualSpectrum.empty()) {
            const int numBins = static_cast<int>(metrics.residualSpectrum.size());
            
            // Find range
            float minDb = -96.0f;
            float maxDb = 0.0f;
            
            // Draw spectrum path
            juce::Path spectrumPath;
            spectrumPath.startNewSubPath(0, height);
            
            for (int i = 0; i < numBins; ++i) {
                // Calculate x position (logarithmic)
                const float binFreq = i * 44100.0f / (2.0f * numBins);
                const float normX = std::log10(binFreq / 20.0f) / std::log10(20000.0f / 20.0f);
                const int x = static_cast<int>(normX * width);
                
                // Calculate y position (dB)
                const float magnitude = metrics.residualSpectrum[i];
                const float dbValue = 20.0f * std::log10(magnitude + 0.0000001f);
                const float normY = (dbValue - minDb) / (maxDb - minDb);
                const int y = static_cast<int>((1.0f - normY) * height);
                
                // Add point to path
                spectrumPath.lineTo(x, y);
            }
            
            // Complete path to bottom corners
            spectrumPath.lineTo(width, height);
            spectrumPath.lineTo(0, height);
            spectrumPath.closeSubPath();
            
            // Draw spectrum
            g.setColour(juce::Colour(100, 180, 255).withAlpha(0.7f));
            g.fillPath(spectrumPath);
            
            g.setColour(juce::Colour(100, 180, 255));
            g.strokePath(spectrumPath, juce::PathStrokeType(1.5f));
        }
        
        // Add frequency axis labels
        g.setColour(juce::Colours::white);
        g.setFont(juce::Font(10.0f));
        
        const float freqPoints[] = {20.0f, 100.0f, 1000.0f, 10000.0f, 20000.0f};
        const juce::String freqLabels[] = {"20Hz", "100Hz", "1kHz", "10kHz", "20kHz"};
        
        for (int i = 0; i < 5; ++i) {
            // Calculate position (logarithmic)
            const float normX = std::log10(freqPoints[i] / 20.0f) / std::log10(20000.0f / 20.0f);
            const int x = static_cast<int>(normX * width);
            
            // Draw label
            g.drawText(freqLabels[i], x - 15, height - 15, 30, 12, juce::Justification::centred, true);
        }
        
        // Add amplitude axis labels
        const float dbPoints[] = {-96.0f, -72.0f, -48.0f, -24.0f, 0.0f};
        
        for (int i = 0; i < 5; ++i) {
            // Calculate position
            const float normY = (dbPoints[i] - (-96.0f)) / (0.0f - (-96.0f));
            const int y = static_cast<int>((1.0f - normY) * height);
            
            // Draw label
            g.drawText(juce::String(static_cast<int>(dbPoints[i])) + " dB", 
                      width - 40, y - 6, 38, 12, juce::Justification::right, true);
        }
        
        // Add title and metrics
        g.setFont(juce::Font(12.0f, juce::Font::bold));
        g.drawText("Residual Spectrum", 0, 2, width, 20, juce::Justification::centred, true);
        
        // Add metrics
        g.setFont(juce::Font(10.0f));
        g.drawText("RMS: " + juce::String(metrics.residualRMS, 1) + " dB", 
                  10, 24, 120, 16, juce::Justification::left, true);
                  
        g.drawText("Tilt: " + juce::String(metrics.residualTilt, 1) + " dB/oct", 
                  10, 40, 120, 16, juce::Justification::left, true);
                  
        g.drawText("Crest: " + juce::String(metrics.residualCrest, 1) + " dB", 
                  10, 56, 120, 16, juce::Justification::left, true);
        
        // Save to file
        juce::PNGImageFormat pngFormat;
        juce::FileOutputStream outputStream(file);
        
        if (outputStream.openedOk()) {
            return pngFormat.writeImageToStream(image, outputStream);
        }
        
        return false;
    }
    
    // Export IACC/ITD timeline to PNG file
    bool exportIACCTimeline(const juce::File& file, const std::vector<float>& iaccHistory,
                           const std::vector<float>& itdHistory) {
        // Create image with appropriate size
        const int width = 512;
        const int height = 256;
        
        juce::Image image(juce::Image::RGB, width, height, true);
        juce::Graphics g(image);
        
        // Fill background
        g.fillAll(juce::Colour(24, 24, 32));
        
        // Draw grid lines
        g.setColour(juce::Colours::grey.withAlpha(0.4f));
        
        // Horizontal lines (IACC values at 0.0, 0.5, 1.0)
        const int yValues[] = {height, height / 2, 0};
        for (int y : yValues) {
            g.drawLine(0, y, width, y, 1.0f);
        }
        
        // Vertical lines (time divisions)
        for (int i = 0; i <= 4; ++i) {
            const int x = i * width / 4;
            g.drawLine(x, 0, x, height, 1.0f);
        }
        
        // Draw IACC timeline
        if (!iaccHistory.empty()) {
            juce::Path iaccPath;
            const int numPoints = static_cast<int>(iaccHistory.size());
            const float xStep = static_cast<float>(width) / static_cast<float>(numPoints - 1);
            
            iaccPath.startNewSubPath(0, height - iaccHistory[0] * height);
            
            for (int i = 1; i < numPoints; ++i) {
                const float x = i * xStep;
                const float y = height - iaccHistory[i] * height;
                iaccPath.lineTo(x, y);
            }
            
            g.setColour(juce::Colour(80, 180, 255));
            g.strokePath(iaccPath, juce::PathStrokeType(2.0f));
        }
        
        // Draw ITD timeline
        if (!itdHistory.empty()) {
            juce::Path itdPath;
            const int numPoints = static_cast<int>(itdHistory.size());
            const float xStep = static_cast<float>(width) / static_cast<float>(numPoints - 1);
            
            // Find ITD range for normalization
            float maxITD = 0.01f; // Minimum range
            for (float itd : itdHistory) {
                maxITD = juce::jmax(maxITD, std::abs(itd));
            }
            
            itdPath.startNewSubPath(0, height / 2 - itdHistory[0] / maxITD * height / 2);
            
            for (int i = 1; i < numPoints; ++i) {
                const float x = i * xStep;
                const float y = height / 2 - itdHistory[i] / maxITD * height / 2;
                itdPath.lineTo(x, y);
            }
            
            g.setColour(juce::Colour(255, 120, 80));
            g.strokePath(itdPath, juce::PathStrokeType(2.0f));
        }
        
        // Add labels
        g.setColour(juce::Colours::white);
        g.setFont(juce::Font(10.0f));
        
        // IACC labels
        g.drawText("1.0", 2, 2, 20, 12, juce::Justification::left, true);
        g.drawText("0.5", 2, height / 2 - 6, 20, 12, juce::Justification::left, true);
        g.drawText("0.0", 2, height - 14, 20, 12, juce::Justification::left, true);
        
        // Time labels
        g.drawText("0s", width - 20, height - 14, 20, 12, juce::Justification::right, true);
        g.drawText("-1s", 0, height - 14, 20, 12, juce::Justification::left, true);
        
        // Add title
        g.setFont(juce::Font(12.0f, juce::Font::bold));
        g.drawText("Interaural Coherence & Time", 0, 2, width, 20, juce::Justification::centred, true);
        
        // Add legend
        g.setFont(juce::Font(10.0f));
        g.setColour(juce::Colour(80, 180, 255));
        g.drawLine(width - 100, 30, width - 80, 30, 2.0f);
        g.setColour(juce::Colours::white);
        g.drawText("IACC", width - 75, 24, 40, 12, juce::Justification::left, true);
        
        g.setColour(juce::Colour(255, 120, 80));
        g.drawLine(width - 100, 46, width - 80, 46, 2.0f);
        g.setColour(juce::Colours::white);
        g.drawText("ITD", width - 75, 40, 40, 12, juce::Justification::left, true);
        
        // Save to file
        juce::PNGImageFormat pngFormat;
        juce::FileOutputStream outputStream(file);
        
        if (outputStream.openedOk()) {
            return pngFormat.writeImageToStream(image, outputStream);
        }
        
        return false;
    }
    
    // Export all metrics and visualizations to a report folder
    bool exportFullReport(const juce::File& folder, const MetricsFrame& metrics,
                         const std::vector<float>& iaccHistory,
                         const std::vector<float>& itdHistory) {
        // Create folder if it doesn't exist
        if (!folder.exists()) {
            folder.createDirectory();
        }
        
        // Export CSV metrics
        const juce::File csvFile = folder.getChildFile("metrics.csv");
        bool success = exportCSV(csvFile, metrics);
        
        // Export GDS heatmap
        const juce::File gdsFile = folder.getChildFile("gds_heatmap.png");
        success = success && exportGDSHeatmap(gdsFile, metrics);
        
        // Export residual spectrum
        const juce::File residualFile = folder.getChildFile("residual_spectrum.png");
        success = success && exportResidualSpectrum(residualFile, metrics);
        
        // Export IACC/ITD timeline
        const juce::File iaccFile = folder.getChildFile("iacc_itd_timeline.png");
        success = success && exportIACCTimeline(iaccFile, iaccHistory, itdHistory);
        
        // Create HTML report that includes all visualizations
        const juce::File htmlFile = folder.getChildFile("report.html");
        success = success && generateHTMLReport(htmlFile, metrics);
        
        return success;
    }
    
private:
    // Get color for heatmap value (blue to yellow gradient)
    juce::Colour getHeatmapColor(float value) const {
        // Value should be between 0 and 1
        value = juce::jlimit(0.0f, 1.0f, value);
        
        // Blue to cyan to green to yellow gradient
        if (value < 0.25f) {
            // Blue to cyan (0.0 - 0.25)
            return juce::Colour::fromFloatRGBA(0.0f, value * 4.0f, 1.0f, 1.0f);
        } else if (value < 0.5f) {
            // Cyan to green (0.25 - 0.5)
            return juce::Colour::fromFloatRGBA(0.0f, 1.0f, 1.0f - (value - 0.25f) * 4.0f, 1.0f);
        } else if (value < 0.75f) {
            // Green to yellow (0.5 - 0.75)
            return juce::Colour::fromFloatRGBA((value - 0.5f) * 4.0f, 1.0f, 0.0f, 1.0f);
        } else {
            // Yellow to red (0.75 - 1.0)
            return juce::Colour::fromFloatRGBA(1.0f, 1.0f - (value - 0.75f) * 4.0f, 0.0f, 1.0f);
        }
    }
    
    // Generate HTML report
    bool generateHTMLReport(const juce::File& file, const MetricsFrame& metrics) {
        // Create HTML content
        juce::String htmlContent = R"(
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>More Like The Record - Metrics Report</title>
    <style>
        body {
            font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif;
            max-width: 1000px;
            margin: 0 auto;
            padding: 20px;
            background-color: #f5f5f5;
            color: #333;
        }
        h1 {
            color: #2c3e50;
            text-align: center;
            margin-bottom: 30px;
        }
        .metrics-container {
            display: flex;
            justify-content: space-between;
            flex-wrap: wrap;
        }
        .metric-card {
            background-color: white;
            border-radius: 8px;
            box-shadow: 0 2px 10px rgba(0, 0, 0, 0.1);
            padding: 20px;
            margin-bottom: 20px;
            flex: 1 1 200px;
            margin: 10px;
        }
        .metric-title {
            font-size: 16px;
            font-weight: bold;
            margin-bottom: 10px;
            color: #2c3e50;
        }
        .metric-value {
            font-size: 24px;
            font-weight: bold;
        }
        .good { color: #27ae60; }
        .medium { color: #f39c12; }
        .poor { color: #c0392b; }
        .visualization {
            background-color: white;
            border-radius: 8px;
            box-shadow: 0 2px 10px rgba(0, 0, 0, 0.1);
            padding: 20px;
            margin: 20px 0;
        }
        .vis-title {
            font-size: 18px;
            font-weight: bold;
            margin-bottom: 15px;
            color: #2c3e50;
        }
        img {
            max-width: 100%;
            height: auto;
            display: block;
            margin: 0 auto;
        }
    </style>
</head>
<body>
    <h1>More Like The Record - Metrics Report</h1>
    
    <div class="metrics-container">
        <div class="metric-card">
            <div class="metric-title">SCPI Score</div>
            <div class="metric-value )";
        
        // Add class based on score
        if (metrics.scpiScore > 0.7f) {
            htmlContent += "good";
        } else if (metrics.scpiScore > 0.4f) {
            htmlContent += "medium";
        } else {
            htmlContent += "poor";
        }
        
        htmlContent += "\">" + juce::String(static_cast<int>(metrics.scpiScore * 100)) + "</div>\n        </div>\n";
        
        // Add GDS score
        htmlContent += R"(
        <div class="metric-card">
            <div class="metric-title">GDS Score</div>
            <div class="metric-value )";
        
        if (metrics.gdsScore > 0.7f) {
            htmlContent += "good";
        } else if (metrics.gdsScore > 0.4f) {
            htmlContent += "medium";
        } else {
            htmlContent += "poor";
        }
        
        htmlContent += "\">" + juce::String(static_cast<int>(metrics.gdsScore * 100)) + "</div>\n        </div>\n";
        
        // Add IACC
        htmlContent += R"(
        <div class="metric-card">
            <div class="metric-title">IACC</div>
            <div class="metric-value )";
        
        if (metrics.iacc > 0.7f) {
            htmlContent += "good";
        } else if (metrics.iacc > 0.4f) {
            htmlContent += "medium";
        } else {
            htmlContent += "poor";
        }
        
        htmlContent += "\">" + juce::String(metrics.iacc, 2) + "</div>\n        </div>\n";
        
        // Add TEI score
        htmlContent += R"(
        <div class="metric-card">
            <div class="metric-title">TEI Score</div>
            <div class="metric-value )";
        
        if (metrics.teiScore > 0.7f) {
            htmlContent += "good";
        } else if (metrics.teiScore > 0.4f) {
            htmlContent += "medium";
        } else {
            htmlContent += "poor";
        }
        
        htmlContent += "\">" + juce::String(static_cast<int>(metrics.teiScore * 100)) + "</div>\n        </div>\n";
        
        // Close metrics container
        htmlContent += "    </div>\n";
        
        // Add visualizations
        htmlContent += R"(
    <div class="visualization">
        <div class="vis-title">Group Delay Spread (GDS) Heatmap</div>
        <img src="gds_heatmap.png" alt="GDS Heatmap">
    </div>
    
    <div class="visualization">
        <div class="vis-title">Interaural Coherence & Time Timeline</div>
        <img src="iacc_itd_timeline.png" alt="IACC/ITD Timeline">
    </div>
    
    <div class="visualization">
        <div class="vis-title">Residual Spectrum</div>
        <img src="residual_spectrum.png" alt="Residual Spectrum">
    </div>
    
    <div class="metrics-container">
        <div class="metric-card">
            <div class="metric-title">Residual RMS</div>
            <div class="metric-value">)";
        htmlContent += juce::String(metrics.residualRMS, 1) + " dB</div>\n        </div>\n";
        
        htmlContent += R"(
        <div class="metric-card">
            <div class="metric-title">ITD Variance</div>
            <div class="metric-value">)";
        htmlContent += juce::String(metrics.itdVariance, 2) + " ms</div>\n        </div>\n";
        
        htmlContent += R"(
        <div class="metric-card">
            <div class="metric-title">Level Difference</div>
            <div class="metric-value">)";
        htmlContent += juce::String(metrics.lufsB - metrics.lufsA, 1) + " LU</div>\n        </div>\n";
        
        htmlContent += R"(
        <div class="metric-card">
            <div class="metric-title">Date Generated</div>
            <div class="metric-value" style="font-size: 16px;">)";
        htmlContent += juce::Time::getCurrentTime().toString(true, true) + "</div>\n        </div>\n";
        
        // Close HTML
        htmlContent += "    </div>\n</body>\n</html>";
        
        // Write to file
        return file.replaceWithText(htmlContent);
    }
};

} // namespace mojo
