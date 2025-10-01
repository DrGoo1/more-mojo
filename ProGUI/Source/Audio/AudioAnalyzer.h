#pragma once

#include <juce_audio_formats/juce_audio_formats.h>
#include <juce_dsp/juce_dsp.h>
#include <vector>
#include <memory>
#include <functional>

/**
 * AudioAnalyzer - Real-time audio analysis for Mojo extraction
 * 
 * Analyzes extracted stem files to determine their sonic characteristics:
 * - Spectral content (frequency distribution, harmonics)
 * - Transient characteristics (attack, punch, dynamics)
 * - Spatial properties (stereo width, correlation)
 * - Character profile (warmth, vintage, shimmer, etc.)
 */
class AudioAnalyzer {
public:
    /**
     * MojoProfile - 8-dimensional sonic character representation
     */
    struct MojoProfile {
        float warmth;      // 0-1: Low-mid frequency energy (200-800 Hz)
        float punch;       // 0-1: Transient impact and attack characteristics
        float space;       // 0-1: Stereo width and spatial depth
        float shimmer;     // 0-1: High-frequency sparkle (8-20 kHz)
        float vintage;     // 0-1: Harmonic distortion and analog character
        float clarity;     // 0-1: Mid-range definition (1-4 kHz)
        float body;        // 0-1: Low-end fullness (40-200 Hz)
        float vibe;        // 0-1: Overall tonal balance and coherence
        
        // Convert to vector for radar chart
        std::vector<float> toVector() const {
            return {warmth, punch, space, shimmer, vintage, clarity, body, vibe};
        }
    };
    
    /**
     * CharacterMetrics - 5 key characteristics for meters
     */
    struct CharacterMetrics {
        float warmth;      // 0-1: 🔥 Warmth
        float vintage;     // 0-1: 📻 Vintage character
        float punch;       // 0-1: 💥 Punch/impact
        float space;       // 0-1: 🌌 Spatial depth
        float shimmer;     // 0-1: ✨ High-end sparkle
    };
    
    AudioAnalyzer();
    ~AudioAnalyzer();
    
    /**
     * Analyze an audio file and extract its Mojo profile
     * @param audioFile The audio file to analyze (WAV, AIFF, etc.)
     * @param callback Completion callback with analysis results
     */
    void analyzeFile(const juce::File& audioFile, 
                     std::function<void(bool success, const MojoProfile&, const CharacterMetrics&)> callback);
    
    /**
     * Cancel ongoing analysis
     */
    void cancelAnalysis();
    
private:
    // Analysis components
    void performSpectralAnalysis(const juce::AudioBuffer<float>& buffer, MojoProfile& profile);
    void performTransientAnalysis(const juce::AudioBuffer<float>& buffer, MojoProfile& profile);
    void performSpatialAnalysis(const juce::AudioBuffer<float>& buffer, MojoProfile& profile);
    void calculateCharacterMetrics(const MojoProfile& profile, CharacterMetrics& metrics);
    
    // Spectral analysis helpers
    float analyzeWarmth(const std::vector<float>& spectrum, int sampleRate);
    float analyzeShimmer(const std::vector<float>& spectrum, int sampleRate);
    float analyzeClarity(const std::vector<float>& spectrum, int sampleRate);
    float analyzeBody(const std::vector<float>& spectrum, int sampleRate);
    float analyzeVintage(const std::vector<float>& spectrum, int sampleRate);
    float analyzeTonalBalance(const std::vector<float>& spectrum);
    
    // Transient analysis helpers
    float analyzeTransients(const juce::AudioBuffer<float>& buffer);
    float calculateEnvelopeFollower(const float* channelData, int numSamples);
    
    // Spatial analysis helpers
    float analyzeStereoWidth(const juce::AudioBuffer<float>& buffer);
    float analyzePhaseCorrelation(const juce::AudioBuffer<float>& buffer);
    
    // FFT processor
    std::unique_ptr<juce::dsp::FFT> fft;
    std::vector<float> fftData;
    int fftSize = 8192;
    
    // Analysis state
    std::atomic<bool> shouldCancel{false};
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(AudioAnalyzer)
};
