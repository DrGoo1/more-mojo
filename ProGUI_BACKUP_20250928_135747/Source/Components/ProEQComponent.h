#pragma once

#include <JuceHeader.h>
#include "foleys_gui_magic.h"

/**
 * ProEQComponent - Professional Equalizer Component using PluginGuiMagic
 * 
 * This component provides a high-quality parametric EQ with spectrum analyzer
 * visualization for audio processing tasks. It features multiple filter bands
 * with interactive control points.
 */
class ProEQComponent : public juce::Component,
                       private juce::Timer
{
public:
    //==============================================================================
    ProEQComponent();
    ~ProEQComponent() override;

    //==============================================================================
    void paint(juce::Graphics& g) override;
    void resized() override;
    
    //==============================================================================
    /**
     * Set the frequency for a specific EQ band
     * 
     * @param bandIndex The index of the band (0-4)
     * @param frequency The frequency value in Hz (20-20000)
     */
    void setBandFrequency(int bandIndex, float frequency);
    
    /**
     * Set the gain for a specific EQ band
     * 
     * @param bandIndex The index of the band (0-4)
     * @param gainDB The gain value in dB (-24 to +24)
     */
    void setBandGain(int bandIndex, float gainDB);
    
    /**
     * Set the Q factor for a specific EQ band
     * 
     * @param bandIndex The index of the band (0-4)
     * @param q The Q factor value (0.1 to 10)
     */
    void setBandQ(int bandIndex, float q);
    
    /**
     * Get the frequency for a specific EQ band
     * 
     * @param bandIndex The index of the band (0-4)
     * @return The frequency in Hz
     */
    float getBandFrequency(int bandIndex) const;
    
    /**
     * Get the gain for a specific EQ band
     * 
     * @param bandIndex The index of the band (0-4)
     * @return The gain in dB
     */
    float getBandGain(int bandIndex) const;
    
    /**
     * Get the Q factor for a specific EQ band
     * 
     * @param bandIndex The index of the band (0-4)
     * @return The Q factor
     */
    float getBandQ(int bandIndex) const;
    
    /**
     * Process audio buffer through the EQ to update visualizations
     * 
     * @param buffer The audio buffer to analyze
     */
    void processAudio(const juce::AudioBuffer<float>& buffer);
    
private:
    //==============================================================================
    // Enumeration of filter types
    enum FilterType
    {
        LowShelf = 0,
        LowPass,
        Peak,
        HighShelf,
        HighPass,
        NumFilterTypes
    };
    
    // Called periodically to update the visuals
    void timerCallback() override;
    
    // Update the filter responses based on current parameters
    void updateFilters();
    
    // PluginGuiMagic components
    std::unique_ptr<foleys::MagicGUIBuilder> magicBuilder;
    std::unique_ptr<foleys::MagicProcessorState> magicState;
    
    // Filter components
    struct FilterBand
    {
        float frequency = 1000.0f;  // Hz
        float gain = 0.0f;          // dB
        float quality = 1.0f;       // Q factor
        FilterType type = Peak;     // Filter type
        bool active = true;         // Is band active
        
        juce::dsp::IIR::Coefficients<float>::Ptr coefficients;
    };
    
    std::array<FilterBand, 5> bands;
    
    // Parameters
    double sampleRate = 44100.0;
    float maxFreq = 20000.0f;
    float minFreq = 20.0f;
    float maxGain = 24.0f;
    float minGain = -24.0f;
    
    // Visuals
    foleys::MagicPlotSource* spectrumAnalyzer = nullptr;
    foleys::MagicFilterPlot* responseGraph = nullptr;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ProEQComponent)
};
