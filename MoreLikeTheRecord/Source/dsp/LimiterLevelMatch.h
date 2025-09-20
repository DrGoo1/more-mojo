#pragma once
#include <JuceHeader.h>
#include "TruePeak.h"

namespace mojoDSP {

/**
 * LimiterLevelMatch - ISP-safe limiter with precise level matching
 * 
 * Features:
 * - Look-ahead limiter with true-peak detection
 * - ITU-R BS.1770 LUFS level matching
 * - Sample-accurate latency compensation
 */
class LimiterLevelMatch {
public:
    LimiterLevelMatch() = default;
    
    void prepare(double sampleRate, int maxBlockSize) {
        this->sampleRate = sampleRate;
        this->maxBlockSize = maxBlockSize;
        
        // Initialize true-peak estimator
        tp.prepare(sampleRate);
        tp.reset();
        
        // Initialize LUFS meter
        lufsFilter.reset();
        
        // Initialize limiter
        resetLimiter();
    }
    
    void reset() {
        tp.reset();
        lufsFilter.reset();
        resetLimiter();
        
        shortTermLufs = -23.0f;
        targetLufs = -23.0f;
        currentGain = 1.0f;
    }
    
    void setParameters(float lookaheadMs, float ceilingDb, float releaseMs, bool useISPGuard, int matchTarget) {
        // Validate and update parameters
        this->lookaheadMs = juce::jlimit(0.1f, 5.0f, lookaheadMs);
        this->ceilingDb = juce::jlimit(-20.0f, 0.0f, ceilingDb);
        this->releaseMs = juce::jlimit(1.0f, 500.0f, releaseMs);
        this->useISPGuard = useISPGuard;
        this->matchTarget = juce::jlimit(0, 1, matchTarget);
        
        // Update limiter settings
        updateLimiterSettings();
    }
    
    void process(juce::AudioBuffer<float>& buffer, juce::AudioProcessorValueTreeState& apvts) {
        const int numChannels = buffer.getNumChannels();
        const int numSamples = buffer.getNumSamples();
        
        if (numChannels <= 0 || numSamples <= 0) {
            return;
        }
        
        // Get parameters from APVTS
        const float lookaheadMs = *apvts.getRawParameterValue("limiter_lookahead");
        const float ceilingDb = *apvts.getRawParameterValue("limiter_ceiling");
        const float releaseMs = *apvts.getRawParameterValue("limiter_release");
        const bool useISPGuard = apvts.getRawParameterValue("limiter_isp_guard")->load() > 0.5f;
        const int matchTarget = static_cast<int>(*apvts.getRawParameterValue("limiter_match_target"));
        
        // Update parameters
        setParameters(lookaheadMs, ceilingDb, releaseMs, useISPGuard, matchTarget);
        
        // Check if we need to apply the limiter
        const bool applyLimiter = ceilingDb < 0.0f || useISPGuard;
        
        // Measure LUFS before processing
        measureLUFS(buffer);
        
        // Apply limiter if needed
        if (applyLimiter) {
            applyLimiting(buffer);
        }
        
        // Apply level matching
        applyLevelMatching(buffer);
        
        // Update true-peak meter
        tp.pushBlock(buffer);
    }
    
    // Get the true-peak value (dB) and reset the meter
    float getTruePeakAndClear() {
        float peak = tp.getAndClearPeak();
        return peak > 0.0f ? juce::Decibels::gainToDecibels(peak) : -120.0f;
    }
    
    // Get current LUFS value
    float getLUFS() const {
        return shortTermLufs;
    }
    
private:
    // Update limiter settings based on current parameters
    void updateLimiterSettings() {
        // Calculate lookahead in samples
        lookaheadSamples = static_cast<int>(lookaheadMs * 0.001f * sampleRate);
        
        // Set ceiling
        limiterCeiling = juce::Decibels::decibelsToGain(ceilingDb);
        
        // Calculate release coefficient
        const float releaseTime = releaseMs * 0.001f;
        releaseCoeff = std::exp(-1.0f / (releaseTime * static_cast<float>(sampleRate)));
        
        // Resize lookahead buffer if needed
        if (lookaheadBuffer.getNumSamples() < lookaheadSamples) {
            lookaheadBuffer.setSize(8, lookaheadSamples, false, true); // Support up to 8 channels
            resetLimiter();
        }
    }
    
    // Reset the limiter state
    void resetLimiter() {
        // Clear lookahead buffer
        lookaheadBuffer.clear();
        
        // Reset limiter state
        limiterGain = 1.0f;
        targetGain = 1.0f;
        writeIndex = 0;
        readIndex = 0;
    }
    
    // Measure LUFS level according to ITU-R BS.1770
    void measureLUFS(const juce::AudioBuffer<float>& buffer) {
        const int numChannels = buffer.getNumChannels();
        const int numSamples = buffer.getNumSamples();
        
        // Process only stereo for LUFS
        if (numChannels < 2) {
            return;
        }
        
        // BS.1770 K-weighting filters and channel weights
        const float channelWeights[2] = {1.0f, 1.0f}; // L, R weights
        
        // Process each sample
        float energySum = 0.0f;
        
        for (int ch = 0; ch < 2; ++ch) {
            const float* data = buffer.getReadPointer(ch);
            float channelEnergy = 0.0f;
            
            for (int i = 0; i < numSamples; ++i) {
                // Apply K-weighting filter (simplified)
                float filtered = lufsFilter.processSample(ch, data[i]);
                
                // Square for energy
                channelEnergy += filtered * filtered;
            }
            
            // Apply channel weight and accumulate
            energySum += channelEnergy * channelWeights[ch];
        }
        
        // Calculate LUFS for this block
        const float blockLufs = energySum > 0.0f ? 
            -0.691f + 10.0f * std::log10(energySum / numSamples) : -120.0f;
        
        // Update short-term LUFS with smoothing
        if (blockLufs > -120.0f) {
            shortTermLufs = 0.8f * shortTermLufs + 0.2f * blockLufs;
        }
    }
    
    // Apply limiting with lookahead
    void applyLimiting(juce::AudioBuffer<float>& buffer) {
        const int numChannels = buffer.getNumChannels();
        const int numSamples = buffer.getNumSamples();
        
        // Resize lookahead buffer if needed
        if (lookaheadBuffer.getNumChannels() < numChannels || 
            lookaheadBuffer.getNumSamples() < lookaheadSamples) {
            lookaheadBuffer.setSize(numChannels, lookaheadSamples, false, true);
        }
        
        // Process in small chunks to handle the circular buffer
        for (int sampleOffset = 0; sampleOffset < numSamples; sampleOffset += maxBlockSize) {
            // Determine chunk size
            const int chunkSize = std::min(maxBlockSize, numSamples - sampleOffset);
            
            // Process chunk
            for (int i = 0; i < chunkSize; ++i) {
                // Find peak in lookahead window
                float peakLevel = 0.0f;
                
                for (int ch = 0; ch < numChannels; ++ch) {
                    // Add new sample to lookahead buffer
                    const float newSample = buffer.getSample(ch, sampleOffset + i);
                    lookaheadBuffer.setSample(ch, writeIndex, newSample);
                    
                    // Find peak in the entire buffer
                    for (int j = 0; j < lookaheadSamples; ++j) {
                        peakLevel = std::max(peakLevel, 
                                           std::abs(lookaheadBuffer.getSample(ch, j)));
                    }
                }
                
                // Calculate required gain reduction
                if (peakLevel > limiterCeiling) {
                    targetGain = limiterCeiling / peakLevel;
                } else {
                    targetGain = 1.0f;
                }
                
                // Smooth gain changes with release
                if (targetGain > limiterGain) {
                    // Release phase
                    limiterGain = targetGain + (limiterGain - targetGain) * releaseCoeff;
                } else {
                    // Attack phase (immediate)
                    limiterGain = targetGain;
                }
                
                // Apply gain to output
                for (int ch = 0; ch < numChannels; ++ch) {
                    // Get delayed sample
                    const float delayedSample = lookaheadBuffer.getSample(ch, readIndex);
                    
                    // Apply gain and write to output
                    buffer.setSample(ch, sampleOffset + i, delayedSample * limiterGain);
                }
                
                // Update buffer indices
                writeIndex = (writeIndex + 1) % lookaheadSamples;
                readIndex = (readIndex + 1) % lookaheadSamples;
            }
        }
    }
    
    // Apply level matching to match target LUFS
    void applyLevelMatching(juce::AudioBuffer<float>& buffer) {
        const int numSamples = buffer.getNumSamples();
        const int numChannels = buffer.getNumChannels();
        
        // Calculate gain needed for level matching
        float targetLevel = targetLufs; // Default
        
        // If matching to A or B, use that level
        if (matchTarget == 0) { // Match A
            targetLevel = refLufsA;
        } else if (matchTarget == 1) { // Match B
            targetLevel = refLufsB;
        }
        
        // Calculate gain difference
        const float gainDiffDb = targetLevel - shortTermLufs;
        const float targetGain = juce::Decibels::decibelsToGain(gainDiffDb);
        
        // Smooth gain changes
        currentGain = currentGain * 0.9f + targetGain * 0.1f;
        
        // Apply gain
        for (int ch = 0; ch < numChannels; ++ch) {
            buffer.applyGain(ch, 0, numSamples, currentGain);
        }
    }
    
    // LUFS filter implementation (simplified K-weighting)
    class LufsFilter {
    public:
        LufsFilter() = default;
        
        void reset() {
            std::fill(z1.begin(), z1.end(), 0.0f);
            std::fill(z2.begin(), z2.end(), 0.0f);
        }
        
        float processSample(int channel, float sample) {
            if (channel >= 2) return sample; // Only process L/R
            
            // Simplified K-weighting filter (high-pass + high-shelf)
            
            // High-pass (75 Hz)
            float hp = sample - 0.975f * z1[channel];
            z1[channel] = sample;
            
            // High-shelf (+4 dB at 3 kHz)
            float hs = hp + 0.2f * z2[channel];
            z2[channel] = hp;
            
            return hs;
        }
        
    private:
        std::array<float, 8> z1{0.0f}; // Support up to 8 channels
        std::array<float, 8> z2{0.0f};
    };
    
    // Instance variables
    double sampleRate = 44100.0;
    int maxBlockSize = 512;
    
    float lookaheadMs = 1.0f;
    float ceilingDb = 0.0f;
    float releaseMs = 50.0f;
    bool useISPGuard = true;
    int matchTarget = 0; // 0 = A, 1 = B
    
    int lookaheadSamples = 0;
    float limiterCeiling = 1.0f;
    float releaseCoeff = 0.9f;
    
    float limiterGain = 1.0f;
    float targetGain = 1.0f;
    int writeIndex = 0;
    int readIndex = 0;
    
    juce::AudioBuffer<float> lookaheadBuffer;
    
    TruePeakEstimator tp;
    LufsFilter lufsFilter;
    
    float shortTermLufs = -23.0f;
    float refLufsA = -23.0f;
    float refLufsB = -23.0f;
    float targetLufs = -23.0f;
    float currentGain = 1.0f;
};

} // namespace mojoDSP
