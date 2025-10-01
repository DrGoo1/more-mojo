#pragma once

#include <juce_core/juce_core.h>
#include <juce_events/juce_events.h>
#include <juce_graphics/juce_graphics.h>
#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_gui_extra/juce_gui_extra.h>
#include <juce_audio_basics/juce_audio_basics.h>

//==============================================================================
/**
    Professional compressor component with dynamic waveform visualization
    showing compression in real-time
*/
class ProCompressorComponent : public juce::Component,
                              private juce::Timer
{
public:
    ProCompressorComponent();
    ~ProCompressorComponent() override;

    void paint(juce::Graphics& g) override;
    void resized() override;
    
    void timerCallback() override;
    
    // Push audio for visualization
    void pushSampleForAnalysis(float inputSample, float outputSample);

    // API for external control (used by wrapper/Swift bridge)
    void setThreshold(float v);
    void setRatio(float v);
    void setAttack(float v);
    void setRelease(float v);
    void setKnee(float v);
    void setMakeupGain(float v);
    float getThreshold() const { return settings.threshold; }
    float getRatio() const { return settings.ratio; }
    float getAttack() const { return settings.attack; }
    float getRelease() const { return settings.release; }
    float getKnee() const { return settings.knee; }
    float getMakeupGain() const { return settings.makeup; }
    
    // Simple processing stub for visualization
    void processAudio(juce::AudioBuffer<float>& buffer);
    
private:
    // Compressor parameters
    struct CompressorSettings {
        float threshold = -20.0f;    // dB
        float ratio = 4.0f;          // compression ratio
        float attack = 20.0f;        // ms
        float release = 200.0f;      // ms
        float makeup = 0.0f;         // dB
        float knee = 6.0f;           // dB
        bool autoMakeup = true;
    };
    
    CompressorSettings settings;
    
    // Parameter controls
    std::unique_ptr<juce::Slider> thresholdSlider;
    std::unique_ptr<juce::Slider> ratioSlider;
    std::unique_ptr<juce::Slider> attackSlider;
    std::unique_ptr<juce::Slider> releaseSlider;
    std::unique_ptr<juce::Slider> makeupSlider;
    std::unique_ptr<juce::Slider> kneeSlider;
    std::unique_ptr<juce::ToggleButton> autoMakeupButton;
    
    // Labels
    juce::Label thresholdLabel;
    juce::Label ratioLabel;
    juce::Label attackLabel;
    juce::Label releaseLabel;
    juce::Label makeupLabel;
    juce::Label kneeLabel;
    
    // Visualization data
    juce::Rectangle<int> visualizationArea;
    
    // Waveform display
    struct WaveformDisplay {
        std::vector<float> inputSamples;
        std::vector<float> outputSamples;
        int writePosition = 0;
        int numSamples = 1024;
    };
    
    WaveformDisplay waveform;
    
    // Gain reduction meter
    float currentGainReduction = 0.0f;
    float peakGainReduction = 0.0f;
    juce::Rectangle<int> gainReductionMeterArea;
    
    // Compression curve
    juce::Path compressionCurve;
    juce::Rectangle<int> compressionCurveArea;
    
    // Create and position controls
    void createControls();
    
    // Update parameters from sliders
    void updateParameters();
    
    // Update compression curve visualization
    void updateCompressionCurve();
    
    // Drawing methods
    void drawWaveform(juce::Graphics& g);
    void drawGainReductionMeter(juce::Graphics& g);
    void drawCompressionCurve(juce::Graphics& g);
    void drawBackground(juce::Graphics& g);
    
    // Helper to calculate gain reduction for visualization
    float calculateGainReduction(float inputLevel);
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ProCompressorComponent)
};
