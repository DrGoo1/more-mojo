#pragma once
#include <juce_core/juce_core.h>
#include <juce_events/juce_events.h>

// Python bridge for AI-based stem extraction using Demucs
class PythonBridge {
public:
    struct StemResult {
        bool success;
        juce::File outputFile;
        juce::String errorMessage;
        int sampleRate;
    };
    
    PythonBridge();
    ~PythonBridge();
    
    // Extract stem using Python/Demucs
    // stemType: "vocals", "drums", "bass", "other"
    void extractStem(
        const juce::File& inputAudio,
        const juce::String& stemType,
        const juce::File& outputPath,
        std::function<void(StemResult)> callback,
        std::function<void(float)> progressCallback = nullptr
    );
    
    // Check if Python and dependencies are available
    bool isPythonAvailable() const;
    bool areDemucsModelsAvailable() const;
    
    // Cancel ongoing extraction
    void cancel();
    
private:
    juce::File findPythonExecutable() const;
    juce::File getStemExtractorScript() const;
    juce::File getTempOutputPath() const;
    
    void monitorProcess(
        juce::ChildProcess& process,
        std::function<void(StemResult)> callback
    );
    
    std::unique_ptr<juce::ChildProcess> currentProcess;
    std::atomic<bool> shouldCancel{false};
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PythonBridge)
};
