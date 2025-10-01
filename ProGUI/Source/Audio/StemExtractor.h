#pragma once
#include <juce_audio_basics/juce_audio_basics.h>
#include <juce_core/juce_core.h>
#include <functional>

// Hybrid stem extraction system for Steal The Mojo
// Supports both AI-based (Demucs via Python) and DSP-based methods
class StemExtractor {
public:
    enum class InstrumentType {
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
    
    enum class ExtractionMethod {
        Auto,           // Automatically choose best available method
        AI_Demucs,      // Python + Demucs (highest quality)
        DSP_Frequency,  // Traditional frequency-based separation
        DSP_Enhanced    // Enhanced DSP with harmonic/percussive separation
    };
    
    struct ExtractionResult {
        bool success;
        juce::File stemFile;
        juce::String errorMessage;
        ExtractionMethod methodUsed;
        double processingTimeSeconds;
    };
    
    StemExtractor();
    ~StemExtractor();
    
    // Main extraction interface
    void extractStem(
        const juce::File& inputAudioFile,
        InstrumentType instrumentType,
        ExtractionMethod method,
        std::function<void(ExtractionResult)> completionCallback,
        std::function<void(float)> progressCallback = nullptr
    );
    
    // Check what methods are available
    bool isAIExtractionAvailable() const;
    bool isDSPExtractionAvailable() const;
    ExtractionMethod selectBestMethod() const;
    
    // Cancel ongoing extraction
    void cancelExtraction();
    
private:
    class Impl;
    std::unique_ptr<Impl> impl;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(StemExtractor)
};
