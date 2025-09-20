#pragma once
#include <JuceHeader.h>

namespace moremojo {
namespace mlar {

/**
 * AnalogContinuity - Subtle analog-like processing for stereo image cohesion
 * 
 * Features:
 * - Mid/side elliptical filtering for warm, analog-like sound
 * - Frequency-dependent stereo crosstalk
 * - Subtle spatial enhancements for improved stereo imaging
 */
class AnalogContinuity {
public:
    AnalogContinuity() = default;
    ~AnalogContinuity() = default;
    
    /**
     * Prepare for processing
     * 
     * @param sampleRate The current sample rate
     * @param blockSize The maximum expected block size
     */
    void prepare(double sampleRate, int blockSize) {
        this->sampleRate = sampleRate;
        this->blockSize = blockSize;
        
        // Initialize filters for mid and side processing
        initFilters();
    }
    
    /**
     * Reset internal state
     */
    void reset() {
        // Reset all filters
        for (auto& filter : filters) {
            filter.reset();
        }
    }
    
    /**
     * Process a buffer through the analog continuity processor
     * 
     * @param buffer The audio buffer to process
     * @param apvts Parameter state containing the processor settings
     */
    void process(juce::AudioBuffer<float>& buffer, juce::AudioProcessorValueTreeState& apvts) {
        // Check if analog continuity is enabled
        bool analogEnabled = apvts.getRawParameterValue("mlar_analog_on")->load() > 0.5f;
        
        if (!analogEnabled) {
            // Early return if disabled
            return;
        }
        
        // Get mix parameter (0.0-1.0)
        float mix = apvts.getRawParameterValue("mlar_analog_mix")->load() * 0.01f;
        mix = juce::jlimit(0.0f, 1.0f, mix);
        
        // Early return if mix is zero
        if (mix <= 0.0f) {
            return;
        }
        
        // Process only if we have stereo audio
        if (buffer.getNumChannels() >= 2) {
            // Store a dry copy for mixing later
            juce::AudioBuffer<float> dryBuffer;
            dryBuffer.makeCopyOf(buffer);
            
            // Process with mid/side elliptical filtering
            processElliptical(buffer, mix);
            
            // Apply subtle crosstalk if mix is high enough
            if (mix > 0.3f) {
                processCrosstalk(buffer, (mix - 0.3f) / 0.7f);
            }
            
            // Final mix between dry and processed
            mixDryWet(dryBuffer, buffer, mix);
        }
    }
    
private:
    /**
     * Initialize filters for mid/side processing
     */
    void initFilters() {
        filters.clear();
        
        // Create mid-range EQ filter
        juce::dsp::IIR::Filter<float> midFilter;
        *midFilter.state = *juce::dsp::IIR::Coefficients<float>::makePeakFilter(
            sampleRate, 1200.0f, 1.0f, 1.2f);  // Slight mid boost
        filters.push_back(midFilter);
        
        // Create low shelf filter for side channel
        juce::dsp::IIR::Filter<float> sideFilter;
        *sideFilter.state = *juce::dsp::IIR::Coefficients<float>::makeLowShelf(
            sampleRate, 250.0f, 0.7f, 0.9f);  // Subtle side channel low taming
        filters.push_back(sideFilter);
    }
    
    /**
     * Process buffer with mid/side elliptical filtering
     * 
     * @param buffer The buffer to process
     * @param mix The mix amount (0.0-1.0)
     */
    void processElliptical(juce::AudioBuffer<float>& buffer, float mix) {
        const float* leftIn = buffer.getReadPointer(0);
        const float* rightIn = buffer.getReadPointer(1);
        float* leftOut = buffer.getWritePointer(0);
        float* rightOut = buffer.getWritePointer(1);
        const int numSamples = buffer.getNumSamples();
        
        // Calculate tilt factor based on mix (more mix = more tilt)
        float tiltFactor = 0.95f + 0.04f * mix;  // 0.95 to 0.99
        
        // Process each sample
        for (int i = 0; i < numSamples; ++i) {
            // Convert to mid/side
            float mid = 0.5f * (leftIn[i] + rightIn[i]);
            float side = leftIn[i] - rightIn[i];
            
            // Apply mid filter
            mid = filters[0].processSample(mid);
            
            // Apply side filter and tilt
            side = filters[1].processSample(side) * tiltFactor;
            
            // Convert back to left/right
            leftOut[i] = mid + 0.5f * side;
            rightOut[i] = mid - 0.5f * side;
        }
    }
    
    /**
     * Process buffer with subtle crosstalk
     * 
     * @param buffer The buffer to process
     * @param amount The crosstalk amount (0.0-1.0)
     */
    void processCrosstalk(juce::AudioBuffer<float>& buffer, float amount) {
        const float* leftIn = buffer.getReadPointer(0);
        const float* rightIn = buffer.getReadPointer(1);
        float* leftOut = buffer.getWritePointer(0);
        float* rightOut = buffer.getWritePointer(1);
        const int numSamples = buffer.getNumSamples();
        
        // Calculate crosstalk factor (very subtle)
        float crosstalkFactor = 0.05f * amount;  // Max 5% crosstalk
        
        // Process each sample
        for (int i = 0; i < numSamples; ++i) {
            // Apply crosstalk
            float leftMix = leftIn[i] + rightIn[i] * crosstalkFactor;
            float rightMix = rightIn[i] + leftIn[i] * crosstalkFactor;
            
            // Normalize level
            float normalizer = 1.0f / (1.0f + crosstalkFactor);
            leftOut[i] = leftMix * normalizer;
            rightOut[i] = rightMix * normalizer;
        }
    }
    
    /**
     * Mix dry and wet signals
     * 
     * @param dry The dry buffer
     * @param wet The wet buffer (will be overwritten with result)
     * @param mix The mix amount (0.0-1.0)
     */
    void mixDryWet(const juce::AudioBuffer<float>& dry, juce::AudioBuffer<float>& wet, float mix) {
        const int numChannels = juce::jmin(dry.getNumChannels(), wet.getNumChannels());
        const int numSamples = wet.getNumSamples();
        
        for (int ch = 0; ch < numChannels; ++ch) {
            const float* dryData = dry.getReadPointer(ch);
            float* wetData = wet.getWritePointer(ch);
            
            for (int i = 0; i < numSamples; ++i) {
                wetData[i] = dryData[i] * (1.0f - mix) + wetData[i] * mix;
            }
        }
    }
    
    // Instance variables
    double sampleRate = 44100.0;
    int blockSize = 512;
    
    // Filters for mid/side processing
    std::vector<juce::dsp::IIR::Filter<float>> filters;
};

}} // namespace moremojo::mlar
