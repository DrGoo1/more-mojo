#pragma once

#include <JuceHeader.h>
#include "foleys_gui_magic.h"

/**
 * ProCompressorComponent - Professional Compressor Component using PluginGuiMagic
 * 
 * This component provides a high-quality dynamic range compressor with real-time
 * visualization of compression curve, input/output waveforms, and gain reduction.
 */
class ProCompressorComponent : public juce::Component,
                              private juce::Timer
{
public:
    //==============================================================================
    ProCompressorComponent();
    ~ProCompressorComponent() override;

    //==============================================================================
    void paint(juce::Graphics& g) override;
    void resized() override;
    
    //==============================================================================
    /**
     * Set the compressor threshold
     * 
     * @param thresholdDB The threshold value in dB (-60 to 0)
     */
    void setThreshold(float thresholdDB);
    
    /**
     * Set the compression ratio
     * 
     * @param ratio The compression ratio (1 to 20)
     */
    void setRatio(float ratio);
    
    /**
     * Set the attack time
     * 
     * @param attackMs Attack time in milliseconds (0.1 to 100)
     */
    void setAttack(float attackMs);
    
    /**
     * Set the release time
     * 
     * @param releaseMs Release time in milliseconds (10 to 1000)
     */
    void setRelease(float releaseMs);
    
    /**
     * Set the knee width
     * 
     * @param kneeDB Knee width in dB (0 to 24)
     */
    void setKnee(float kneeDB);
    
    /**
     * Set the makeup gain
     * 
     * @param makeupDB Makeup gain in dB (0 to 24)
     */
    void setMakeupGain(float makeupDB);
    
    //==============================================================================
    /**
     * Get the current threshold value
     * 
     * @return Threshold in dB
     */
    float getThreshold() const;
    
    /**
     * Get the current ratio value
     * 
     * @return Compression ratio
     */
    float getRatio() const;
    
    /**
     * Get the current attack time
     * 
     * @return Attack time in ms
     */
    float getAttack() const;
    
    /**
     * Get the current release time
     * 
     * @return Release time in ms
     */
    float getRelease() const;
    
    /**
     * Get the current knee width
     * 
     * @return Knee width in dB
     */
    float getKnee() const;
    
    /**
     * Get the current makeup gain
     * 
     * @return Makeup gain in dB
     */
    float getMakeupGain() const;
    
    /**
     * Process audio buffer through the compressor to update visualizations
     * 
     * @param buffer The audio buffer to analyze
     */
    void processAudio(const juce::AudioBuffer<float>& buffer);
    
private:
    //==============================================================================
    void timerCallback() override;
    
    // Updates the compression curve display
    void updateCompressionCurve();
    
    // Process a single sample through the compressor model
    float processSample(float inputSample);
    
    // PluginGuiMagic components
    std::unique_ptr<foleys::MagicGUIBuilder> magicBuilder;
    std::unique_ptr<foleys::MagicProcessorState> magicState;
    
    // Visualization components
    foleys::MagicPlotSource* inputWaveform = nullptr;
    foleys::MagicPlotSource* outputWaveform = nullptr;
    foleys::MagicLevelSource* gainReductionMeter = nullptr;
    
    // Custom plot for compression curve
    class CompressionCurvePlot : public foleys::MagicPlotSource
    {
    public:
        CompressionCurvePlot() = default;
        
        void createPlotPaths (juce::Path& path, juce::Path& filledPath, juce::Rectangle<float> bounds, foleys::MagicPlotComponent&) override;
        
        // Update curve parameters
        void setParameters(float threshold, float ratio, float knee);
        
    private:
        float threshold = -20.0f;
        float ratio = 4.0f;
        float knee = 6.0f;
    };
    
    std::unique_ptr<CompressionCurvePlot> compressionCurve;
    
    // Compressor parameters
    float threshold = -20.0f;    // dB
    float ratio = 4.0f;          // ratio:1
    float attack = 10.0f;        // ms
    float release = 100.0f;      // ms
    float knee = 6.0f;           // dB
    float makeupGain = 0.0f;     // dB
    
    // Runtime state
    double sampleRate = 44100.0;
    float currentGainReduction = 0.0f;
    float detectorValue = 0.0f;
    
    // Circular buffers for visualization
    juce::AudioBuffer<float> inputBuffer;
    juce::AudioBuffer<float> outputBuffer;
    int bufferPosition = 0;
    static const int bufferSize = 1024;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ProCompressorComponent)
};
