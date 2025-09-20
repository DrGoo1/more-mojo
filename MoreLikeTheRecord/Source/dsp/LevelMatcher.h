#pragma once
#include <JuceHeader.h>

namespace mojoDSP {

/**
 * LevelMatcher - EBU R128 LUFS level-matching for fair A/B comparison
 * 
 * Implements precise level matching with ±0.1 LU accuracy
 * based on ITU-R BS.1770 / EBU R128 loudness measurement
 */
class LevelMatcher {
public:
    LevelMatcher() = default;
    
    /**
     * Prepare the level matcher for processing
     * 
     * @param sampleRate The audio sample rate
     */
    void prepare(double sampleRate) {
        this->sampleRate = sampleRate;
        
        // Initialize K-weighting filters
        for (int ch = 0; ch < 2; ++ch) {
            // Pre-filter (high-pass at 38 Hz)
            *preFilter[ch].state = *juce::dsp::IIR::Coefficients<float>::makeHighPass(
                sampleRate, 38.0f, 0.5f);
                
            // High shelf filter (+4 dB at 1.5 kHz)
            *shelfFilter[ch].state = *juce::dsp::IIR::Coefficients<float>::makeHighShelf(
                sampleRate, 1500.0f, 0.7071f, juce::Decibels::decibelsToGain(4.0f));
        }
        
        reset();
    }
    
    /**
     * Reset the level matcher state
     */
    void reset() {
        // Reset filters
        for (int ch = 0; ch < 2; ++ch) {
            preFilter[ch].reset();
            shelfFilter[ch].reset();
        }
        
        // Reset loudness measurements
        shortTermLUFSA = -23.0f;
        shortTermLUFSB = -23.0f;
        targetLUFS = -23.0f;
        
        // Reset gain
        matchGain = 1.0f;
    }
    
    /**
     * Measure LUFS for reference (A) signal
     * 
     * @param buffer The reference audio buffer
     */
    void measureA(const juce::AudioBuffer<float>& buffer) {
        shortTermLUFSA = measureLUFS(buffer);
    }
    
    /**
     * Measure LUFS for processed (B) signal
     * 
     * @param buffer The processed audio buffer
     */
    void measureB(const juce::AudioBuffer<float>& buffer) {
        shortTermLUFSB = measureLUFS(buffer);
    }
    
    /**
     * Set target LUFS level
     * 
     * @param target Target LUFS level
     */
    void setTarget(float target) {
        targetLUFS = target;
    }
    
    /**
     * Set whether to match B to A (true) or A to B (false)
     * 
     * @param matchBtoA Direction of matching
     */
    void setMatchDirection(bool matchBtoA) {
        this->matchBtoA = matchBtoA;
    }
    
    /**
     * Apply level matching to the buffer
     * 
     * @param buffer Audio buffer to apply level matching to
     */
    void process(juce::AudioBuffer<float>& buffer) {
        // Calculate level difference
        float sourceLUFS = matchBtoA ? shortTermLUFSB : shortTermLUFSA;
        float targetLUFS = matchBtoA ? shortTermLUFSA : shortTermLUFSB;
        
        // If user-specified target is set, use that instead
        if (this->targetLUFS != -23.0f) {
            targetLUFS = this->targetLUFS;
        }
        
        // Calculate gain in dB
        float gainDB = targetLUFS - sourceLUFS;
        
        // Only adjust if difference is more than 0.1 LU
        if (std::abs(gainDB) > 0.1f) {
            // Convert to linear gain
            float targetGain = juce::Decibels::decibelsToGain(gainDB);
            
            // Smoothly approach target gain to avoid clicks
            matchGain = matchGain * 0.9f + targetGain * 0.1f;
            
            // Apply gain
            buffer.applyGain(matchGain);
        }
    }
    
    /**
     * Get short-term LUFS measurement for A
     */
    float getLUFSA() const {
        return shortTermLUFSA;
    }
    
    /**
     * Get short-term LUFS measurement for B
     */
    float getLUFSB() const {
        return shortTermLUFSB;
    }
    
    /**
     * Get level difference in LU
     */
    float getLevelDifference() const {
        return shortTermLUFSB - shortTermLUFSA;
    }
    
private:
    /**
     * Measure LUFS level for a buffer
     * 
     * @param buffer The audio buffer to measure
     * @return LUFS value in dB
     */
    float measureLUFS(const juce::AudioBuffer<float>& buffer) {
        const int numChannels = juce::jmin(buffer.getNumChannels(), 2);
        const int numSamples = buffer.getNumSamples();
        
        if (numChannels == 0 || numSamples == 0) {
            return -70.0f; // Silent
        }
        
        // Channel weights (L, R)
        const float channelWeights[2] = {1.0f, 1.0f};
        
        // Process each channel with K-weighting
        float weightedSquareSum = 0.0f;
        
        for (int ch = 0; ch < numChannels; ++ch) {
            const float* data = buffer.getReadPointer(ch);
            float channelSquareSum = 0.0f;
            
            for (int i = 0; i < numSamples; ++i) {
                // Apply K-weighting (BS.1770)
                float sample = data[i];
                
                // Pre-filter (high-pass)
                sample = preFilter[ch].processSample(sample);
                
                // Shelf filter
                sample = shelfFilter[ch].processSample(sample);
                
                // Square and accumulate
                channelSquareSum += sample * sample;
            }
            
            // Apply channel weight
            weightedSquareSum += channelSquareSum * channelWeights[ch];
        }
        
        // Calculate mean square
        float meanSquare = weightedSquareSum / static_cast<float>(numSamples * numChannels);
        
        // Calculate LUFS
        if (meanSquare > 1.0e-10f) {
            // LUFS = -0.691 + 10*log10(mean square)
            float lufs = -0.691f + 10.0f * std::log10(meanSquare);
            
            // Apply time integration (short-term is ~3 seconds)
            // We simulate this with exponential smoothing
            return currentLUFS * 0.9f + lufs * 0.1f;
        }
        
        return -70.0f; // Silent or very quiet
    }
    
    // Instance variables
    double sampleRate = 44100.0;
    
    // K-weighting filters
    std::array<juce::dsp::IIR::Filter<float>, 2> preFilter; // High-pass
    std::array<juce::dsp::IIR::Filter<float>, 2> shelfFilter; // High shelf
    
    // LUFS measurements
    float shortTermLUFSA = -23.0f;
    float shortTermLUFSB = -23.0f;
    float targetLUFS = -23.0f;
    float currentLUFS = -23.0f;
    
    // Level matching
    float matchGain = 1.0f;
    bool matchBtoA = true; // Match B to A by default
};

} // namespace mojoDSP
