#pragma once
#include <juce_audio_basics/juce_audio_basics.h>
#include <juce_dsp/juce_dsp.h>

// DSP-based frequency separation fallback
// Uses spectral filtering and harmonic/percussive separation
class DSPSeparator {
public:
    enum class ProfileType {
        Vocal,
        Drums,
        Bass,
        Guitar,
        Piano,
        Strings,
        Brass,
        Synth,
        FullMix
    };
    
    struct FrequencyProfile {
        float subBass;      // 20-60 Hz
        float bass;         // 60-250 Hz
        float lowMid;       // 250-500 Hz
        float mid;          // 500-2k Hz
        float highMid;      // 2k-8k Hz
        float presence;     // 8k-12k Hz
        float brilliance;   // 12k-20k Hz
        
        bool emphasizeTransients;
        bool emphasizeHarmonic;
        float stereoWidthFactor;
    };
    
    DSPSeparator();
    ~DSPSeparator();
    
    // Separate audio using frequency profile
    juce::AudioBuffer<float> separateByProfile(
        const juce::AudioBuffer<float>& input,
        ProfileType profile,
        double sampleRate,
        std::function<void(float)> progressCallback = nullptr
    );
    
    // Get frequency profile for instrument type
    static FrequencyProfile getProfileForType(ProfileType type);
    
private:
    void applyFrequencyMask(
        juce::AudioBuffer<float>& buffer,
        const FrequencyProfile& profile,
        double sampleRate
    );
    
    void enhanceTransients(juce::AudioBuffer<float>& buffer);
    void isolateHarmonic(juce::AudioBuffer<float>& buffer, double sampleRate);
    void adjustStereoWidth(juce::AudioBuffer<float>& buffer, float factor);
    
    static constexpr int fftOrder = 12; // 4096 samples
    static constexpr int fftSize = 1 << fftOrder;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(DSPSeparator)
};
