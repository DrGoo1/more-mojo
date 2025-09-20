#pragma once
#include <JuceHeader.h>
#include "../components/MojoMeters.h"
#include "../components/MojoSlider.h"
#include "../../metrics/GDSComputer.h"
#include "../../metrics/IACCAnalyzer.h"
#include "../../metrics/TEIAnalyzer.h"

// Forward declaration
class MoreMojoAudioProcessor;

namespace moremojo {
namespace mlar {
namespace mojoUI {

/**
 * DetailWindow - Base class for all detail windows
 * 
 * Provides common functionality and styling for detail windows
 */
class DetailWindow : public juce::Component {
public:
    explicit DetailWindow(const juce::String& header = "Details")
        : headerText(header) {
        // Set up header label
        addAndMakeVisible(headerLabel);
        headerLabel.setText(header, juce::dontSendNotification);
        headerLabel.setFont(juce::Font(18.0f, juce::Font::bold));
        
        // Set up close button
        addAndMakeVisible(closeButton);
        closeButton.setButtonText("×");
        closeButton.onClick = [this]() {
            if (onClose) onClose();
        };
    }
    
    ~DetailWindow() override = default;
    
    void paint(juce::Graphics& g) override {
        // Fill background
        g.fillAll(juce::Colour(15, 15, 20));
        
        // Draw panel background
        juce::Rectangle<float> bounds = getLocalBounds().toFloat().reduced(6.0f);
        LNF::card(g, bounds, juce::Colour(24, 24, 32));
    }
    
    void resized() override {
        auto bounds = getLocalBounds().reduced(12);
        
        // Position header and close button
        auto headerRow = bounds.removeFromTop(28);
        headerLabel.setBounds(headerRow.removeFromLeft(headerRow.getWidth() - 28));
        closeButton.setBounds(headerRow.withSizeKeepingCentre(28, 28));
        
        // Add spacing
        bounds.removeFromTop(8);
        
        // Child classes should use the remaining bounds for their content
    }
    
    /**
     * Set callback for when close button is clicked
     */
    void setOnClose(std::function<void()> callback) {
        onClose = std::move(callback);
    }
    
protected:
    juce::String headerText;
    juce::Label headerLabel;
    juce::TextButton closeButton;
    
    std::function<void()> onClose;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(DetailWindow)
};

/**
 * ResamplerDetail - Detailed controls for the resampler module
 */
class ResamplerDetail : public DetailWindow {
public:
    explicit ResamplerDetail(MoreMojoAudioProcessor& processor);
    ~ResamplerDetail() override = default;
    
    void resized() override;
    
private:
    MoreMojoAudioProcessor& processor;
    
    // Basic controls
    juce::ToggleButton interpToggle{"Intersample Protection"};
    juce::ToggleButton ditherToggle{"Dither"};
    MojoSlider qualitySlider;
    juce::Label qualityLabel{"", "Resampler Quality"};
    
    // ISP margin control
    MojoSlider ispMarginSlider;
    juce::Label ispMarginLabel{"", "ISP Margin (dB)"};
    
    // Reconstruction filter controls
    MojoSlider rfTapsSlider;
    juce::Label rfTapsLabel{"", "RF Taps"};
    juce::ComboBox rfWindowCombo;
    juce::Label rfWindowLabel{"", "RF Window"};
    MojoSlider rfRippleSlider;
    juce::Label rfRippleLabel{"", "RF Ripple (dB)"};
    
    // Parameter attachments
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> interpAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> ditherAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> qualityAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> ispMarginAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> rfTapsAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> rfWindowAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> rfRippleAttachment;
    
    // Meters
    TruePeakMeter peakMeter;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ResamplerDetail)
};

/**
 * TransientDetail - Detailed controls for the transient detector
 */
class TransientDetail : public DetailWindow {
public:
    explicit TransientDetail(MoreMojoAudioProcessor& processor);
    ~TransientDetail() override = default;
    
    void resized() override;
    
private:
    MoreMojoAudioProcessor& processor;
    
    // Controls
    MojoSlider sensitivitySlider;
    juce::Label sensitivityLabel{"", "Transient Sensitivity"};
    
    MojoSlider bandsSlider;
    juce::Label bandsLabel{"", "Analysis Bands"};
    
    // Parameter attachments
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> sensitivityAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> bandsAttachment;
    
    // Meters
    TransientActivityMeter activityMeter;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(TransientDetail)
};

/**
 * AlignDetail - Detailed controls for the alignment module
 */
class AlignDetail : public DetailWindow {
public:
    explicit AlignDetail(MoreMojoAudioProcessor& processor);
    ~AlignDetail() override = default;
    
    void resized() override;
    
private:
    MoreMojoAudioProcessor& processor;
    
    // Controls
    MojoSlider amountSlider;
    juce::Label amountLabel{"", "Align Amount"};
    
    MojoSlider bandsSlider;
    juce::Label bandsLabel{"", "Crossover Bands"};
    
    // Per-band delay knobs
    std::vector<std::unique_ptr<MojoSlider>> bandDelaySliders;
    std::vector<std::unique_ptr<juce::Label>> bandDelayLabels;
    
    // Parameter attachments
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> amountAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> bandsAttachment;
    std::vector<std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment>> bandDelayAttachments;
    
    // Meters
    PhaseCorrelationMeter phaseMeter;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(AlignDetail)
};

/**
 * AnalogDetail - Detailed controls for the analog continuity module
 */
class AnalogDetail : public DetailWindow {
public:
    explicit AnalogDetail(MoreMojoAudioProcessor& processor);
    ~AnalogDetail() override = default;
    
    void resized() override;
    
private:
    MoreMojoAudioProcessor& processor;
    
    // Controls
    MojoSlider mixSlider;
    juce::Label mixLabel{"", "Analog Mix"};
    
    // Parameter attachments
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> mixAttachment;
    
    // Description
    juce::Label descriptionLabel;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(AnalogDetail)
};

/**
 * LimiterDetail - Detailed controls for the limiter module
 */
class LimiterDetail : public DetailWindow {
public:
    explicit LimiterDetail(MoreMojoAudioProcessor& processor);
    ~LimiterDetail() override = default;
    
    void resized() override;
    
private:
    MoreMojoAudioProcessor& processor;
    
    // Controls
    MojoSlider ceilingSlider;
    juce::Label ceilingLabel{"", "Ceiling (dB)"};
    
    MojoSlider lookAheadSlider;
    juce::Label lookAheadLabel{"", "Look-ahead (ms)"};
    
    // Target LUFS controls
    MojoSlider targetLUFSSlider;
    juce::Label targetLUFSLabel{"", "Target LUFS"};
    
    // Parameter attachments
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> ceilingAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> lookAheadAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> targetLUFSAttachment;
    
    // Meters
    LUFSMeter lufsMeter;
    LUFSMeter targetLUFSMeter;
    GainReductionMeter grMeter;
    
    // ΔLUFS display
    class DeltaLUFSDisplay : public juce::Component {
    public:
        DeltaLUFSDisplay() {
            deltaLUFS = 0.0f;
        }
        
        void paint(juce::Graphics& g) override {
            auto bounds = getLocalBounds().toFloat().reduced(2.0f);
            
            // Draw background
            g.setColour(juce::Colour(32, 32, 38));
            g.fillRoundedRectangle(bounds, 4.0f);
            
            // Draw delta value text with appropriate color
            juce::Colour textColor;
            if (std::abs(deltaLUFS) < 0.1f) {
                // Green for very close match (within 0.1 LU)
                textColor = juce::Colour(80, 220, 100);
            } else if (std::abs(deltaLUFS) < 0.5f) {
                // Yellow for acceptable match (within 0.5 LU)
                textColor = juce::Colour(240, 220, 40);
            } else {
                // Red for poor match
                textColor = juce::Colour(240, 60, 60);
            }
            
            g.setColour(textColor);
            g.setFont(juce::Font(16.0f, juce::Font::bold));
            
            juce::String deltaText;
            if (deltaLUFS > 0.0f) {
                deltaText = "+" + juce::String(deltaLUFS, 1) + " LU";
            } else {
                deltaText = juce::String(deltaLUFS, 1) + " LU";
            }
            
            g.drawText(deltaText, bounds, juce::Justification::centred, true);
            
            // Draw label
            g.setColour(juce::Colours::white);
            g.setFont(juce::Font(12.0f));
            g.drawText("ΔLUFS", bounds.removeFromBottom(14.0f), juce::Justification::centred, true);
        }
        
        void setDeltaLUFS(float delta) {
            if (std::abs(deltaLUFS - delta) > 0.01f) {
                deltaLUFS = delta;
                repaint();
            }
        }
        
    private:
        float deltaLUFS;
    };
    
    DeltaLUFSDisplay deltaLUFSDisplay;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(LimiterDetail)
};

/**
 * JitterDetail - Detailed controls for the jitter reduction module
 */
class JitterDetail : public DetailWindow {
public:
    explicit JitterDetail(MoreMojoAudioProcessor& processor);
    ~JitterDetail() override = default;
    
    void resized() override;
    
private:
    MoreMojoAudioProcessor& processor;
    
    // Controls
    juce::ToggleButton jitterToggle{"Jitter Control"};
    juce::ComboBox precisionCombo;
    juce::Label precisionLabel{"Precision", "Accumulator Precision"};
    
    // Parameter attachments
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> jitterAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> precisionAttachment;
    
    // Phase noise meter
    class PhaseNoiseMeter : public juce::Component {
    public:
        PhaseNoiseMeter() {
            startTimerHz(30);
        }
        
        ~PhaseNoiseMeter() override {
            stopTimer();
        }
        
        void paint(juce::Graphics& g) override {
            auto bounds = getLocalBounds().toFloat().reduced(2.0f);
            
            // Draw background
            g.setColour(juce::Colour(32, 32, 38));
            g.fillRoundedRectangle(bounds, 4.0f);
            
            // Draw grid lines
            g.setColour(juce::Colours::white.withAlpha(0.2f));
            const int numGridLines = 5;
            for (int i = 1; i < numGridLines; ++i) {
                float y = bounds.getY() + bounds.getHeight() * i / numGridLines;
                g.drawHorizontalLine(static_cast<int>(y), bounds.getX(), bounds.getRight());
                
                float x = bounds.getX() + bounds.getWidth() * i / numGridLines;
                g.drawVerticalLine(static_cast<int>(x), bounds.getY(), bounds.getBottom());
            }
            
            // Draw noise waveform
            g.setColour(juce::Colour(80, 180, 220));
            juce::Path noisePath;
            const int numPoints = bounds.getWidth();
            noisePath.startNewSubPath(bounds.getX(), bounds.getCentreY() + noiseValues[0] * bounds.getHeight() * 0.4f);
            
            for (int i = 1; i < juce::jmin(numPoints, static_cast<int>(noiseValues.size())); ++i) {
                noisePath.lineTo(bounds.getX() + i, 
                               bounds.getCentreY() + noiseValues[i] * bounds.getHeight() * 0.4f);
            }
            
            g.strokePath(noisePath, juce::PathStrokeType(1.5f));
            
            // Draw noise level indicator
            float noiseLevel = 0.0f;
            for (float val : noiseValues) {
                noiseLevel += std::abs(val);
            }
            noiseLevel /= static_cast<float>(noiseValues.size());
            
            g.setColour(juce::Colour(220, 180, 80));
            g.drawText(juce::String(noiseLevel * 100.0f, 1) + "% noise",
                      bounds, juce::Justification::bottomRight, true);
        }
        
        void timerCallback() {
            // Generate new noise values with some persistence
            if (noiseValues.size() < 256) {
                noiseValues.resize(256);
            }
            
            // Shift values left
            for (size_t i = 0; i < noiseValues.size() - 1; ++i) {
                noiseValues[i] = noiseValues[i + 1];
            }
            
            // Generate new value with filtered noise
            currentNoise = currentNoise * 0.7f + (random.nextFloat() * 2.0f - 1.0f) * 0.3f;
            noiseValues[noiseValues.size() - 1] = currentNoise;
            
            repaint();
        }
        
        void setNoiseLevel(float level) {
            noiseMultiplier = juce::jlimit(0.0f, 1.0f, level);
        }
        
    private:
        juce::Random random;
        std::vector<float> noiseValues;
        float currentNoise = 0.0f;
        float noiseMultiplier = 0.5f;
    };
    
    PhaseNoiseMeter noiseMeter;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(JitterDetail)
};

/**
 * ConsumerMacroDetail - Detailed description of macro mapping
 */
class ConsumerMacroDetail : public DetailWindow {
public:
    explicit ConsumerMacroDetail(MoreMojoAudioProcessor& processor);
    ~ConsumerMacroDetail() override = default;
    
    void resized() override;
    
private:
    MoreMojoAudioProcessor& processor;
    
    // Controls
    MojoSlider macroSlider;
    juce::Label macroLabel{"", "More Like The Record"};
    
    // Parameter attachment
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> macroAttachment;
    
    // Mapping visualization
    juce::Label mappingLabel;
    juce::DrawableRectangle mappingVisualization;
    
    // Update mapping visualization based on slider value
    void updateMappingVisualization(float value);
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ConsumerMacroDetail)
};

/**
 * MetricsHubDetail - Detailed metrics visualization
 */
class MetricsHubDetail : public DetailWindow {
public:
    explicit MetricsHubDetail(MoreMojoAudioProcessor& processor);
    ~MetricsHubDetail() override = default;
    
    void resized() override;
    void paint(juce::Graphics& g) override;
    
private:
    MoreMojoAudioProcessor& processor;
    
    // GDS Heatmap
    juce::Image gdsHeatmapImage;
    juce::Rectangle<int> gdsHeatmapBounds;
    
    // IACC Timeline
    std::vector<float> iaccTimeline;
    juce::Rectangle<int> iaccTimelineBounds;
    
    // TEI Metrics
    float teiRiseUs = 0.0f;
    float teiPreRingDb = 0.0f;
    float teiOvershootPct = 0.0f;
    juce::Rectangle<int> teiOverlayBounds;
    
    // Residual controls
    juce::ToggleButton residualToggle{"Residual Mode"};
    MojoSlider residualGainSlider;
    juce::Label residualGainLabel{"", "Residual Gain (dB)"};
    
    // Export buttons
    juce::TextButton exportCSVButton{"Export CSV"};
    juce::TextButton exportPNGButton{"Export PNG"};
    
    // Residual spectrum display
    class ResidualSpectrumView : public juce::Component {
    public:
        ResidualSpectrumView() {
            startTimerHz(15);
        }
        
        ~ResidualSpectrumView() override {
            stopTimer();
        }
        
        void paint(juce::Graphics& g) override {
            auto bounds = getLocalBounds().toFloat().reduced(2.0f);
            
            // Draw background
            g.setColour(juce::Colour(32, 32, 38));
            g.fillRoundedRectangle(bounds, 4.0f);
            
            // Draw grid lines
            g.setColour(juce::Colours::white.withAlpha(0.2f));
            const int numVerticalLines = 10;
            const int numHorizontalLines = 5;
            
            // Vertical lines (frequency markers)
            for (int i = 1; i < numVerticalLines; ++i) {
                float x = bounds.getX() + bounds.getWidth() * i / numVerticalLines;
                g.drawVerticalLine(static_cast<int>(x), bounds.getY(), bounds.getBottom());
                
                // Draw frequency labels
                float freq = 20.0f * std::pow(10.0f, static_cast<float>(i) / numVerticalLines * 3.0f); // 20Hz to 20kHz
                juce::String freqLabel;
                
                if (freq < 1000.0f) {
                    freqLabel = juce::String(static_cast<int>(freq)) + "Hz";
                } else {
                    freqLabel = juce::String(freq / 1000.0f, 1) + "kHz";
                }
                
                g.setFont(10.0f);
                g.drawText(freqLabel, static_cast<int>(x - 20), static_cast<int>(bounds.getBottom() - 15), 
                          40, 15, juce::Justification::centred, true);
            }
            
            // Horizontal lines (amplitude markers)
            for (int i = 1; i < numHorizontalLines; ++i) {
                float y = bounds.getY() + bounds.getHeight() * i / numHorizontalLines;
                g.drawHorizontalLine(static_cast<int>(y), bounds.getX(), bounds.getRight());
                
                // Draw dB labels
                float db = -i * 12.0f;
                juce::String dbLabel = juce::String(static_cast<int>(db)) + "dB";
                
                g.setFont(10.0f);
                g.drawText(dbLabel, static_cast<int>(bounds.getX()), static_cast<int>(y - 10),
                          30, 20, juce::Justification::centredLeft, true);
            }
            
            // Draw spectrum if we have data
            if (!spectrumData.empty()) {
                g.setColour(juce::Colour(240, 80, 60));
                
                juce::Path spectrumPath;
                spectrumPath.startNewSubPath(bounds.getX(), bounds.getBottom());
                
                const int numPoints = static_cast<int>(spectrumData.size());
                const float pointWidth = bounds.getWidth() / numPoints;
                
                for (int i = 0; i < numPoints; ++i) {
                    float x = bounds.getX() + i * pointWidth;
                    
                    // Convert to dB and limit to -60 dB minimum
                    float dbValue = 20.0f * std::log10(spectrumData[i] + 1.0e-6f);
                    dbValue = juce::jmax(dbValue, -60.0f);
                    
                    // Map dB to y coordinate (0dB at top, -60dB at bottom)
                    float y = juce::jmap(dbValue, 0.0f, -60.0f, bounds.getY(), bounds.getBottom());
                    
                    spectrumPath.lineTo(x, y);
                }
                
                // Complete the path to bottom-right and bottom-left to create filled area
                spectrumPath.lineTo(bounds.getRight(), bounds.getBottom());
                spectrumPath.closeSubPath();
                
                // Fill with semi-transparent color
                g.setColour(juce::Colour(240, 80, 60).withAlpha(0.4f));
                g.fillPath(spectrumPath);
                
                // Draw the line with solid color
                g.setColour(juce::Colour(240, 80, 60));
                g.strokePath(spectrumPath, juce::PathStrokeType(1.5f));
            }
            
            // Draw title
            g.setColour(juce::Colours::white);
            g.setFont(juce::Font(14.0f, juce::Font::bold));
            g.drawText("Residual Spectrum", bounds.toNearestInt(), juce::Justification::topLeft, true);
        }
        
        void timerCallback() {
            repaint();
        }
        
        void setSpectrumData(const std::vector<float>& data) {
            spectrumData = data;
            repaint();
        }
        
    private:
        std::vector<float> spectrumData;
    };
    
    ResidualSpectrumView residualSpectrum;
    
    // Parameter attachments
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> residualAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> residualGainAttachment;
    
    // Update metrics display
    void updateMetrics();
    
    // Draw IACC timeline
    void drawIACCTimeline(juce::Graphics& g, const juce::Rectangle<int>& bounds, 
                        const std::vector<float>& timeline);
    
    // Draw TEI overlay
    void drawTEIOverlay(juce::Graphics& g, const juce::Rectangle<int>& bounds,
                      float riseUs, float preRingDb, float overshootPct);
    
    // Update GDS heatmap image
    void updateGDSHeatmap(const std::vector<float>& data, int width, int height);
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MetricsHubDetail)
};

}}} // namespace moremojo::mlar::mojoUI
