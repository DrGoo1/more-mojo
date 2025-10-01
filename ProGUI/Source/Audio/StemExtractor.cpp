#include "StemExtractor.h"
#include "PythonBridge.h"
#include "DSPSeparator.h"
#include <juce_audio_formats/juce_audio_formats.h>

// Implementation class (PIMPL pattern)
class StemExtractor::Impl {
public:
    Impl() 
        : pythonBridge(std::make_unique<PythonBridge>()),
          dspSeparator(std::make_unique<DSPSeparator>())
    {
        formatManager.registerBasicFormats();
    }
    
    void extractStem(
        const juce::File& inputFile,
        InstrumentType type,
        ExtractionMethod method,
        std::function<void(ExtractionResult)> callback,
        std::function<void(float)> progressCallback
    ) {
        ExtractionResult result;
        result.methodUsed = method;
        
        auto startTime = juce::Time::getMillisecondCounterHiRes();
        
        // Auto-select method if needed
        if (method == ExtractionMethod::Auto) {
            method = selectBestAvailableMethod();
            result.methodUsed = method;
        }
        
        // Try AI extraction first
        if (method == ExtractionMethod::AI_Demucs) {
            if (pythonBridge->isPythonAvailable()) {
                extractWithAI(inputFile, type, callback, progressCallback);
                return;
            } else {
                // Fallback to DSP
                method = ExtractionMethod::DSP_Enhanced;
            }
        }
        
        // DSP extraction
        if (method == ExtractionMethod::DSP_Frequency || 
            method == ExtractionMethod::DSP_Enhanced) {
            extractWithDSP(inputFile, type, method, callback, progressCallback);
            return;
        }
        
        // Should never reach here
        result.success = false;
        result.errorMessage = "No extraction method available";
        callback(result);
    }
    
    bool isAIAvailable() const {
        return pythonBridge->isPythonAvailable() && 
               pythonBridge->areDemucsModelsAvailable();
    }
    
    bool isDSPAvailable() const {
        return true; // Always available
    }
    
    ExtractionMethod selectBestAvailableMethod() const {
        if (isAIAvailable())
            return ExtractionMethod::AI_Demucs;
        else
            return ExtractionMethod::DSP_Enhanced;
    }
    
    void cancel() {
        if (pythonBridge)
            pythonBridge->cancel();
    }
    
private:
    void extractWithAI(
        const juce::File& inputFile,
        InstrumentType type,
        std::function<void(ExtractionResult)> callback,
        std::function<void(float)> progressCallback
    ) {
        auto stemType = instrumentTypeToDemucsName(type);
        auto outputPath = juce::File::createTempFile("stem_output.wav");
        
        auto startTime = juce::Time::getMillisecondCounterHiRes();
        
        pythonBridge->extractStem(
            inputFile,
            stemType,
            outputPath,
            [callback, startTime](PythonBridge::StemResult pyResult) {
                ExtractionResult result;
                result.success = pyResult.success;
                result.stemFile = pyResult.outputFile;
                result.errorMessage = pyResult.errorMessage;
                result.methodUsed = ExtractionMethod::AI_Demucs;
                result.processingTimeSeconds = 
                    (juce::Time::getMillisecondCounterHiRes() - startTime) / 1000.0;
                callback(result);
            },
            progressCallback
        );
    }
    
    void extractWithDSP(
        const juce::File& inputFile,
        InstrumentType type,
        ExtractionMethod method,
        std::function<void(ExtractionResult)> callback,
        std::function<void(float)> progressCallback
    ) {
        // Run DSP extraction on a background thread to avoid blocking UI
        juce::Thread::launch([this, inputFile, type, method, callback, progressCallback]() {
            auto startTime = juce::Time::getMillisecondCounterHiRes();
            
            // Load audio file
            std::unique_ptr<juce::AudioFormatReader> reader(
                formatManager.createReaderFor(inputFile)
            );
            
            if (!reader) {
                juce::MessageManager::callAsync([callback, method]() {
                    ExtractionResult result;
                    result.success = false;
                    result.errorMessage = "Could not read audio file";
                    result.methodUsed = method;
                    callback(result);
                });
                return;
            }
            
            // Read into buffer
            juce::AudioBuffer<float> inputBuffer(
                reader->numChannels,
                static_cast<int>(reader->lengthInSamples)
            );
            reader->read(&inputBuffer, 0, static_cast<int>(reader->lengthInSamples), 0, true, true);
            
            // Perform DSP separation
            auto profile = instrumentTypeToDSPProfile(type);
            auto separatedBuffer = dspSeparator->separateByProfile(
                inputBuffer,
                profile,
                reader->sampleRate,
                [progressCallback](float progress) {
                    if (progressCallback) {
                        juce::MessageManager::callAsync([progressCallback, progress]() {
                            progressCallback(progress);
                        });
                    }
                }
            );
            
            // Write output file
            auto outputFile = juce::File::createTempFile("stem_dsp.wav");
            std::unique_ptr<juce::FileOutputStream> outStream(outputFile.createOutputStream());
            
            if (outStream) {
                juce::WavAudioFormat wavFormat;
                std::unique_ptr<juce::AudioFormatWriter> writer(
                    wavFormat.createWriterFor(
                        outStream.get(),
                        reader->sampleRate,
                        separatedBuffer.getNumChannels(),
                        24, // bit depth
                        {},
                        0
                    )
                );
                
                if (writer) {
                    outStream.release(); // writer takes ownership
                    writer->writeFromAudioSampleBuffer(separatedBuffer, 0, separatedBuffer.getNumSamples());
                    writer.reset();
                    
                    juce::MessageManager::callAsync([callback, outputFile, method, startTime]() {
                        ExtractionResult result;
                        result.success = true;
                        result.stemFile = outputFile;
                        result.methodUsed = method;
                        result.processingTimeSeconds = 
                            (juce::Time::getMillisecondCounterHiRes() - startTime) / 1000.0;
                        callback(result);
                    });
                    return;
                }
            }
            
            juce::MessageManager::callAsync([callback, method]() {
                ExtractionResult result;
                result.success = false;
                result.errorMessage = "Could not write output file";
                result.methodUsed = method;
                callback(result);
            });
        });
    }
    
    juce::String instrumentTypeToDemucsName(InstrumentType type) const {
        switch (type) {
            case InstrumentType::Vocal:   return "vocals";
            case InstrumentType::Drums:   return "drums";
            case InstrumentType::Bass:    return "bass";
            default:                      return "other";
        }
    }
    
    DSPSeparator::ProfileType instrumentTypeToDSPProfile(InstrumentType type) const {
        switch (type) {
            case InstrumentType::Vocal:   return DSPSeparator::ProfileType::Vocal;
            case InstrumentType::Drums:   return DSPSeparator::ProfileType::Drums;
            case InstrumentType::Bass:    return DSPSeparator::ProfileType::Bass;
            case InstrumentType::Guitar:  return DSPSeparator::ProfileType::Guitar;
            case InstrumentType::Piano:   return DSPSeparator::ProfileType::Piano;
            case InstrumentType::Strings: return DSPSeparator::ProfileType::Strings;
            case InstrumentType::Brass:   return DSPSeparator::ProfileType::Brass;
            case InstrumentType::Synth:   return DSPSeparator::ProfileType::Synth;
            case InstrumentType::FullMix: return DSPSeparator::ProfileType::FullMix;
            default:                      return DSPSeparator::ProfileType::FullMix;
        }
    }
    
    std::unique_ptr<PythonBridge> pythonBridge;
    std::unique_ptr<DSPSeparator> dspSeparator;
    juce::AudioFormatManager formatManager;
};

// StemExtractor implementation
StemExtractor::StemExtractor() : impl(std::make_unique<Impl>()) {}
StemExtractor::~StemExtractor() = default;

void StemExtractor::extractStem(
    const juce::File& inputAudioFile,
    InstrumentType instrumentType,
    ExtractionMethod method,
    std::function<void(ExtractionResult)> completionCallback,
    std::function<void(float)> progressCallback
) {
    impl->extractStem(inputAudioFile, instrumentType, method, completionCallback, progressCallback);
}

bool StemExtractor::isAIExtractionAvailable() const {
    return impl->isAIAvailable();
}

bool StemExtractor::isDSPExtractionAvailable() const {
    return impl->isDSPAvailable();
}

StemExtractor::ExtractionMethod StemExtractor::selectBestMethod() const {
    return impl->selectBestAvailableMethod();
}

void StemExtractor::cancelExtraction() {
    impl->cancel();
}
