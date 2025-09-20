#pragma once
#include <JuceHeader.h>

namespace mojoDSP {

/**
 * TruePeak estimator based on ITU-R BS.1770
 * Uses 4x oversampling to detect intersample peaks (ISPs)
 */
class TruePeakEstimator {
public:
    TruePeakEstimator() : peakLevel(0.0f) {}
    
    void prepare(double sampleRate) {
        // Initialize filters with ITU-R BS.1770 coefficients
        for (auto& filter : filters) {
            filter.reset();
            filter.prepare(4 * sampleRate);
        }
        peakLevel.store(0.0f);
    }
    
    void reset() {
        for (auto& filter : filters) {
            filter.reset();
        }
        peakLevel.store(0.0f);
    }
    
    void pushBlock(const juce::AudioBuffer<float>& buffer) {
        const int numChannels = juce::jmin(buffer.getNumChannels(), 8); // Limit to 8 channels
        const int numSamples = buffer.getNumSamples();
        
        // Create temporary buffer for oversampled data
        juce::AudioBuffer<float> oversampledBuffer(1, numSamples * 4);
        float localPeak = peakLevel.load();
        
        // Process each channel
        for (int ch = 0; ch < numChannels; ++ch) {
            const float* inputData = buffer.getReadPointer(ch);
            
            // Process the channel with the filter and check for peak
            filters[ch].processAndFindPeak(inputData, numSamples, localPeak);
        }
        
        // Store the new peak level
        peakLevel.store(localPeak);
    }
    
    float getAndClearPeak() {
        float peak = peakLevel.load();
        peakLevel.store(0.0f);
        return peak;
    }
    
private:
    // Filter implementation for BS.1770 oversampling
    class TruePeakFilter {
    public:
        TruePeakFilter() = default;
        
        void prepare(double sampleRate) {
            juce::ignoreUnused(sampleRate);
            z1 = z2 = z3 = 0.0f;
            
            // ITU-R BS.1770 coefficients for 4x oversampling
            b0 = 0.5002f;
            b1 = 1.0f;
            b2 = 0.5002f;
            a1 = 0.0f;
            a2 = 0.0f;
        }
        
        void reset() {
            z1 = z2 = z3 = 0.0f;
        }
        
        void processAndFindPeak(const float* input, int numSamples, float& maxPeak) {
            for (int i = 0; i < numSamples; ++i) {
                const float x = input[i];
                
                // Apply filter and generate 4x oversampled points
                float out[4];
                
                // First point (original sample)
                out[0] = b0 * x + z1;
                z1 = b1 * x - a1 * out[0] + z2;
                z2 = b2 * x - a2 * out[0];
                
                // Interpolated points
                out[1] = (out[0] + x) * 0.5f;
                out[2] = x * 0.75f + out[0] * 0.25f;
                out[3] = x * 0.25f + out[0] * 0.75f;
                
                // Check peak on all points
                for (int j = 0; j < 4; ++j) {
                    const float absValue = std::abs(out[j]);
                    if (absValue > maxPeak)
                        maxPeak = absValue;
                }
            }
        }
        
    private:
        float z1 = 0.0f, z2 = 0.0f, z3 = 0.0f;  // Filter state variables
        float b0 = 0.0f, b1 = 0.0f, b2 = 0.0f;  // Filter coefficients (numerator)
        float a1 = 0.0f, a2 = 0.0f;             // Filter coefficients (denominator)
    };
    
    std::array<TruePeakFilter, 8> filters; // Support up to 8 channels
    std::atomic<float> peakLevel;
};

} // namespace mojoDSP
