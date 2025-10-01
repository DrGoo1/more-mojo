#include "DSPSeparator.h"
#include <cmath>

DSPSeparator::DSPSeparator() {}
DSPSeparator::~DSPSeparator() {}

juce::AudioBuffer<float> DSPSeparator::separateByProfile(
    const juce::AudioBuffer<float>& input,
    ProfileType profile,
    double sampleRate,
    std::function<void(float)> progressCallback
) {
    // Create output buffer
    juce::AudioBuffer<float> output(input.getNumChannels(), input.getNumSamples());
    output.makeCopyOf(input);
    
    // Get frequency profile
    auto freqProfile = getProfileForType(profile);
    
    if (progressCallback) progressCallback(0.1f);
    
    // Apply frequency-based filtering
    applyFrequencyMask(output, freqProfile, sampleRate);
    
    if (progressCallback) progressCallback(0.5f);
    
    // Enhance transients if needed
    if (freqProfile.emphasizeTransients) {
        enhanceTransients(output);
    }
    
    if (progressCallback) progressCallback(0.7f);
    
    // Isolate harmonic content if needed
    if (freqProfile.emphasizeHarmonic) {
        isolateHarmonic(output, sampleRate);
    }
    
    if (progressCallback) progressCallback(0.9f);
    
    // Adjust stereo width
    if (std::abs(freqProfile.stereoWidthFactor - 1.0f) > 0.01f) {
        adjustStereoWidth(output, freqProfile.stereoWidthFactor);
    }
    
    if (progressCallback) progressCallback(1.0f);
    
    return output;
}

DSPSeparator::FrequencyProfile DSPSeparator::getProfileForType(ProfileType type) {
    FrequencyProfile profile;
    
    switch (type) {
        case ProfileType::Vocal:
            profile = {0.0f, 0.2f, 0.6f, 1.0f, 0.9f, 0.6f, 0.3f, false, true, 0.8f};
            break;
            
        case ProfileType::Drums:
            profile = {0.4f, 0.8f, 0.7f, 0.5f, 0.6f, 0.9f, 0.4f, true, false, 1.2f};
            break;
            
        case ProfileType::Bass:
            profile = {1.0f, 1.0f, 0.6f, 0.3f, 0.1f, 0.0f, 0.0f, false, true, 0.6f};
            break;
            
        case ProfileType::Guitar:
            profile = {0.1f, 0.4f, 0.7f, 0.9f, 0.8f, 0.5f, 0.3f, true, true, 1.0f};
            break;
            
        case ProfileType::Piano:
            profile = {0.2f, 0.5f, 0.8f, 0.9f, 0.7f, 0.4f, 0.3f, true, true, 1.0f};
            break;
            
        case ProfileType::Strings:
            profile = {0.1f, 0.3f, 0.6f, 0.9f, 0.8f, 0.6f, 0.4f, false, true, 1.1f};
            break;
            
        case ProfileType::Brass:
            profile = {0.0f, 0.2f, 0.5f, 0.9f, 1.0f, 0.7f, 0.3f, false, true, 0.9f};
            break;
            
        case ProfileType::Synth:
            profile = {0.3f, 0.5f, 0.7f, 0.8f, 0.9f, 0.8f, 0.6f, false, true, 1.0f};
            break;
            
        case ProfileType::FullMix:
        default:
            profile = {0.5f, 0.5f, 0.5f, 0.5f, 0.5f, 0.5f, 0.5f, false, false, 1.0f};
            break;
    }
    
    return profile;
}

void DSPSeparator::applyFrequencyMask(
    juce::AudioBuffer<float>& buffer,
    const FrequencyProfile& profile,
    double sampleRate
) {
    // Use 7-band EQ to apply frequency profile
    struct Band {
        float freq;
        float gain;
    };
    
    Band bands[] = {
        {40.0f,   profile.subBass},
        {150.0f,  profile.bass},
        {375.0f,  profile.lowMid},
        {1000.0f, profile.mid},
        {4000.0f, profile.highMid},
        {10000.0f, profile.presence},
        {16000.0f, profile.brilliance}
    };
    
    // Apply simple IIR filters for each band
    for (auto& band : bands) {
        // Skip if gain is near 1.0 (no change needed)
        if (std::abs(band.gain - 1.0f) < 0.01f)
            continue;
        
        // Create peaking filter
        juce::dsp::IIR::Coefficients<float>::Ptr coeffs = 
            juce::dsp::IIR::Coefficients<float>::makePeakFilter(
                sampleRate,
                band.freq,
                0.7f, // Q factor
                juce::Decibels::gainToDecibels(band.gain)
            );
        
        juce::dsp::IIR::Filter<float> filter(coeffs);
        juce::dsp::AudioBlock<float> block(buffer);
        juce::dsp::ProcessContextReplacing<float> context(block);
        filter.process(context);
    }
}

void DSPSeparator::enhanceTransients(juce::AudioBuffer<float>& buffer) {
    // Simple transient enhancer using envelope following
    const int numChannels = buffer.getNumChannels();
    const int numSamples = buffer.getNumSamples();
    
    for (int ch = 0; ch < numChannels; ++ch) {
        auto* data = buffer.getWritePointer(ch);
        
        float envelope = 0.0f;
        const float attackCoeff = 0.1f;
        const float releaseCoeff = 0.999f;
        
        for (int i = 0; i < numSamples; ++i) {
            float input = std::abs(data[i]);
            
            // Envelope follower
            if (input > envelope)
                envelope = envelope * attackCoeff + input * (1.0f - attackCoeff);
            else
                envelope = envelope * releaseCoeff;
            
            // Enhance transients
            float transient = input - envelope;
            if (transient > 0.0f) {
                data[i] += transient * 0.3f; // Boost transients by 30%
            }
        }
    }
}

void DSPSeparator::isolateHarmonic(juce::AudioBuffer<float>& buffer, double sampleRate) {
    // Simple harmonic/percussive separation using median filtering
    // This is a simplified version - full implementation would use STFT
    
    const int numChannels = buffer.getNumChannels();
    const int numSamples = buffer.getNumSamples();
    const int windowSize = 17; // Median filter window
    
    for (int ch = 0; ch < numChannels; ++ch) {
        auto* data = buffer.getWritePointer(ch);
        juce::AudioBuffer<float> temp(1, numSamples);
        auto* tempData = temp.getWritePointer(0);
        
        // Copy data
        temp.copyFrom(0, 0, buffer, ch, 0, numSamples);
        
        // Apply median filter to emphasize harmonic content
        for (int i = windowSize / 2; i < numSamples - windowSize / 2; ++i) {
            std::vector<float> window;
            for (int j = -windowSize / 2; j <= windowSize / 2; ++j) {
                window.push_back(std::abs(tempData[i + j]));
            }
            std::sort(window.begin(), window.end());
            float median = window[windowSize / 2];
            
            // Keep samples close to median (harmonic), reduce outliers (percussive)
            float diff = std::abs(std::abs(data[i]) - median);
            if (diff > median * 0.5f) {
                data[i] *= 0.3f; // Reduce percussive content
            }
        }
    }
}

void DSPSeparator::adjustStereoWidth(juce::AudioBuffer<float>& buffer, float factor) {
    if (buffer.getNumChannels() < 2)
        return;
    
    const int numSamples = buffer.getNumSamples();
    auto* left = buffer.getWritePointer(0);
    auto* right = buffer.getWritePointer(1);
    
    for (int i = 0; i < numSamples; ++i) {
        // Mid-side processing
        float mid = (left[i] + right[i]) * 0.5f;
        float side = (left[i] - right[i]) * 0.5f;
        
        // Adjust side signal
        side *= factor;
        
        // Convert back to L-R
        left[i] = mid + side;
        right[i] = mid - side;
    }
}
