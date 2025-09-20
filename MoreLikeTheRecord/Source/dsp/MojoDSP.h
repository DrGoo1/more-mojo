#pragma once
#include <JuceHeader.h>
#include "TimingSafeResampler.h"
#include "TransientDetect.h"
#include "FractionalDelayAlign.h"
#include "AnalogContinuity.h"
#include "LimiterLevelMatch.h"

namespace mojoDSP {

/**
 * MojoDSP - Main processing pipeline for "More Like The Record"
 * 
 * Integrates all DSP modules into a complete processing chain:
 * 1. TimingSafeResampler - High-quality oversampling and ISP handling
 * 2. TransientDetect - Multi-band phase deviation detection
 * 3. FractionalDelayAlign - Micro-timing restoration
 * 4. AnalogContinuity - Subtle analog-like enhancements
 * 5. LimiterLevelMatch - Level matching and protection
 */
class MojoDSP {
public:
    MojoDSP() = default;
    
    void prepare(double sampleRate, int maxBlockSize, int numChannels) {
        // Initialize all modules
        resampler.prepare(sampleRate, maxBlockSize);
        transient.prepare(sampleRate, maxBlockSize);
        align.prepare(sampleRate, maxBlockSize);
        analog.prepare(sampleRate, maxBlockSize);
        limiter.prepare(sampleRate, maxBlockSize);
        
        // Store settings
        this->sampleRate = sampleRate;
        this->maxBlockSize = maxBlockSize;
        this->numChannels = numChannels;
    }
    
    void reset() {
        // Reset all modules
        resampler.reset();
        transient.reset();
        align.reset();
        analog.reset();
        limiter.reset();
    }
    
    void process(juce::AudioBuffer<float>& buffer, juce::AudioProcessorValueTreeState& apvts) {
        // Get the macro value
        const float macro = apvts.getRawParameterValue("macro")->load();
        
        // Process the modules in sequence, based on their enabled state
        
        // 1. TimingSafeResampler
        if (apvts.getRawParameterValue("resampler_on")->load() > 0.5f) {
            resampler.process(buffer, apvts);
        }
        
        // 2. TransientDetect
        if (apvts.getRawParameterValue("transient_on")->load() > 0.5f) {
            transient.process(buffer, apvts);
        }
        
        // 3. FractionalDelayAlign
        if (apvts.getRawParameterValue("align_on")->load() > 0.5f) {
            align.process(buffer, apvts);
        }
        
        // 4. AnalogContinuity
        if (apvts.getRawParameterValue("analog_on")->load() > 0.5f) {
            analog.process(buffer, apvts);
        }
        
        // 5. LimiterLevelMatch (always active for safety)
        limiter.process(buffer, apvts);
    }
    
    // Access to individual modules for metrics retrieval
    TimingSafeResampler& getResampler() { return resampler; }
    TransientDetect& getTransientDetect() { return transient; }
    FractionalDelayAlign& getAlign() { return align; }
    AnalogContinuity& getAnalog() { return analog; }
    LimiterLevelMatch& getLimiter() { return limiter; }
    
private:
    // DSP modules
    TimingSafeResampler resampler;
    TransientDetect transient;
    FractionalDelayAlign align;
    AnalogContinuity analog;
    LimiterLevelMatch limiter;
    
    // Settings
    double sampleRate = 44100.0;
    int maxBlockSize = 512;
    int numChannels = 2;
};

} // namespace mojoDSP
