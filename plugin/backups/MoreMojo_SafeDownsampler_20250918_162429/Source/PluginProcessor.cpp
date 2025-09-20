#include "PluginProcessor.h"
#include "JuceInit_fix.h"
#include "SafeDownsampler.h"

// Parameter IDs
const juce::String MoreMojoAudioProcessor::PARAM_DRIVE = "drive";
const juce::String MoreMojoAudioProcessor::PARAM_CHARACTER = "character";
const juce::String MoreMojoAudioProcessor::PARAM_SATURATION = "saturation";
const juce::String MoreMojoAudioProcessor::PARAM_PRESENCE = "presence";
const juce::String MoreMojoAudioProcessor::PARAM_MIX = "mix";
const juce::String MoreMojoAudioProcessor::PARAM_OUTPUT = "output";
const juce::String MoreMojoAudioProcessor::PARAM_INTERP_MODE = "interp_mode";
const juce::String MoreMojoAudioProcessor::PARAM_MLAR_ENABLED = "mlar_enabled";

//==============================================================================
// Oversampler class - handles upsampling and downsampling with JUCE
// With extra memory management protections
//==============================================================================
class Oversampler
{
public:
    Oversampler() : currentFactor(4), sampleRate(44100.0), isInitialized(false) 
    {
        // Pre-allocate with safe values to prevent null pointer issues
        juce::dsp::ProcessSpec safeSpec;
        safeSpec.sampleRate = 44100.0;
        safeSpec.maximumBlockSize = 512;
        safeSpec.numChannels = 2;
        
        try {
            // Pre-initialize to avoid memory issues on first use
            ensureInitialized(safeSpec);
            
#if MLAR_USE_SAFE_DOWNSAMPLER
            // Initialize safe downsamplers with default settings
            safeDownsamplerX2.prepare(safeSpec.sampleRate, 
                                  static_cast<uint32_t>(safeSpec.maximumBlockSize), 
                                  static_cast<uint32_t>(safeSpec.numChannels),
                                  moremojo::dsp::SafeDownsampler::Quality::HQ127, 
                                  moremojo::dsp::SafeDownsampler::Phase::MinPhase);
#endif
        } catch (...) {
            // Silently continue - we'll retry during prepare
        }
    }
    
    // Ensure oversampler objects are properly initialized
    void ensureInitialized(const juce::dsp::ProcessSpec& spec)
    {
        // Ensure we have valid parameters
        const uint32_t safeNumChannels = spec.numChannels > 0 ? spec.numChannels : 2;
        const uint32_t safeBlockSize = spec.maximumBlockSize > 0 ? spec.maximumBlockSize : 512;
        const double safeSampleRate = spec.sampleRate > 0 ? spec.sampleRate : 44100.0;
        
        juce::ScopedJuceInitialiser_GUI scopedJuce; // Ensure JUCE is initialized for thread safety
        
        try {
            // Create 4x oversampler if needed
            if (!oversamplerX4)
            {
                oversamplerX4 = std::make_unique<juce::dsp::Oversampling<float>>(
                    safeNumChannels, 2, juce::dsp::Oversampling<float>::filterHalfBandPolyphaseIIR, true, false);
                oversamplerX4->initProcessing(safeBlockSize);
            }
            
            // Create 8x oversampler if needed
            if (!oversamplerX8)
            {
                oversamplerX8 = std::make_unique<juce::dsp::Oversampling<float>>(
                    safeNumChannels, 3, juce::dsp::Oversampling<float>::filterHalfBandPolyphaseIIR, true, false);
                oversamplerX8->initProcessing(safeBlockSize);
            }
            
            isInitialized = true;
        }
        catch (std::exception& e) {
            DBG("Oversampler initialization error: " + juce::String(e.what()));
            isInitialized = false;
        }
    }
    
    void prepare(const juce::dsp::ProcessSpec& spec, int oversamplingFactor)
    {
        // Ensure we have valid parameters
        const uint32_t safeNumChannels = spec.numChannels > 0 ? spec.numChannels : 2;
        const uint32_t safeBlockSize = spec.maximumBlockSize > 0 ? spec.maximumBlockSize : 512;
        const double safeSampleRate = spec.sampleRate > 0 ? spec.sampleRate : 44100.0;
        
        // Initialize the oversamplers if needed
        ensureInitialized(spec);
        
        // Set the current factor based on parameter (default to 4x if 8x requested but not available)
        currentFactor = (oversamplingFactor == 8 && oversamplerX8) ? 8 : 4;
        sampleRate = safeSampleRate;
        
#if MLAR_USE_SAFE_DOWNSAMPLER
        // Initialize safe downsamplers with high quality settings
        safeDownsamplerX2.prepare(safeSampleRate, 
                              static_cast<uint32_t>(safeBlockSize), 
                              static_cast<uint32_t>(safeNumChannels),
                              moremojo::dsp::SafeDownsampler::Quality::HQ127, 
                              moremojo::dsp::SafeDownsampler::Phase::MinPhase);
        
        // For 8x oversampling, we'll need both stages
        if (currentFactor == 8) {
            safeDownsamplerX4.prepare(safeSampleRate/2.0, 
                                static_cast<uint32_t>(safeBlockSize/2), 
                                static_cast<uint32_t>(safeNumChannels),
                                moremojo::dsp::SafeDownsampler::Quality::HQ127,
                                moremojo::dsp::SafeDownsampler::Phase::MinPhase);
        }
        
        // Report latency to main processor
        downsamplerId = juce::String(currentFactor) + "x SafeDownsampler";
#else
        downsamplerId = juce::String(currentFactor) + "x JUCE downsampler";
#endif
    }
    
    int getOversamplingFactor() const
    {
        return currentFactor;
    }
    
    void processSamplesUp(juce::AudioBuffer<float>& buffer, juce::AudioBuffer<float>& oversampledBuffer)
    {
        // Ensure initialization
        if (!isInitialized)
        {
            juce::dsp::ProcessSpec spec;
            spec.sampleRate = sampleRate;
            spec.maximumBlockSize = buffer.getNumSamples() > 0 ? buffer.getNumSamples() : 512;
            spec.numChannels = buffer.getNumChannels() > 0 ? buffer.getNumChannels() : 2;
            ensureInitialized(spec);
        }
        
        // Safety checks
        if ((currentFactor == 4 && oversamplerX4 == nullptr) ||
            (currentFactor == 8 && oversamplerX8 == nullptr) ||
            buffer.getNumChannels() == 0 || buffer.getNumSamples() == 0)
        {
            // Can't process without valid data, create safe empty buffer
            oversampledBuffer.setSize(buffer.getNumChannels() > 0 ? buffer.getNumChannels() : 2,
                                     buffer.getNumSamples() > 0 ? buffer.getNumSamples() * 4 : 512, 
                                     false, true);
            oversampledBuffer.clear();
            return;
        }
        
        try {
            // Use JUCE's implementation but with robust safety checks
            juce::dsp::AudioBlock<float> block(buffer);
            
            if (currentFactor == 4 && oversamplerX4 != nullptr)
            {
                auto osBlock = oversamplerX4->processSamplesUp(block);
                
                // Safe buffer sizing and copying
                const int destNumSamples = static_cast<int>(osBlock.getNumSamples());
                if (destNumSamples > 0) {
                    oversampledBuffer.setSize(static_cast<int>(osBlock.getNumChannels()), 
                                            destNumSamples, false, false, true);
                
                    // Copy data safely with bounds checking
                    for (int ch = 0; ch < static_cast<int>(osBlock.getNumChannels()); ++ch)
                    {
                        if (ch < oversampledBuffer.getNumChannels())
                        {
                            oversampledBuffer.copyFrom(ch, 0, osBlock.getChannelPointer(static_cast<size_t>(ch)), destNumSamples);
                        }
                    }
                }
            }
            else if (currentFactor == 8 && oversamplerX8 != nullptr)
            {
                auto osBlock = oversamplerX8->processSamplesUp(block);
                
                // Safe buffer sizing and copying
                const int destNumSamples = static_cast<int>(osBlock.getNumSamples());
                if (destNumSamples > 0) {
                    oversampledBuffer.setSize(static_cast<int>(osBlock.getNumChannels()), 
                                            destNumSamples, false, false, true);
                    
                    // Copy data safely with bounds checking
                    for (int ch = 0; ch < static_cast<int>(osBlock.getNumChannels()); ++ch)
                    {
                        if (ch < oversampledBuffer.getNumChannels())
                        {
                            oversampledBuffer.copyFrom(ch, 0, osBlock.getChannelPointer(static_cast<size_t>(ch)), destNumSamples);
                        }
                    }
                }
            }
            else
            {
                // Fallback if we couldn't use either oversampler
                oversampledBuffer.setSize(buffer.getNumChannels(), buffer.getNumSamples() * 4, false, true);
                oversampledBuffer.clear();
                
                // Copy the original data with gain scaling (simple upsampling)
                for (int ch = 0; ch < buffer.getNumChannels(); ++ch)
                {
                    for (int i = 0; i < buffer.getNumSamples(); ++i)
                    {
                        oversampledBuffer.setSample(ch, i * 4, buffer.getSample(ch, i) * 4.0f);
                    }
                }
            }
        }
        catch (std::exception& e) {
            // Handle any exceptions gracefully with fallback
            DBG("Upsampling error: " + juce::String(e.what()));
            oversampledBuffer.setSize(buffer.getNumChannels(), buffer.getNumSamples() * 4, false, true);
            oversampledBuffer.clear();
        }
    }
    
    void processSamplesDown(juce::AudioBuffer<float>& oversampledBuffer, juce::AudioBuffer<float>& buffer)
    {
        // Ensure initialization
        if (!isInitialized)
        {
            juce::dsp::ProcessSpec spec;
            spec.sampleRate = sampleRate;
            spec.maximumBlockSize = buffer.getNumSamples() > 0 ? buffer.getNumSamples() : 512;
            spec.numChannels = buffer.getNumChannels() > 0 ? buffer.getNumChannels() : 2;
            ensureInitialized(spec);
        }
        
        // Safety checks
        if (oversampledBuffer.getNumChannels() == 0 || buffer.getNumChannels() == 0 || 
            oversampledBuffer.getNumSamples() == 0 || buffer.getNumSamples() == 0)
        {
            buffer.clear(); // Safety - ensure clean output
            return;
        }
        
        try {
#if MLAR_USE_SAFE_DOWNSAMPLER
            // Use the SafeDownsampler for high-quality downsampling
            if (currentFactor == 4)
            {
                // For 4x, use one SafeDownsampler stage (/2) twice
                juce::AudioBuffer<float> tempBuffer;
                tempBuffer.setSize(oversampledBuffer.getNumChannels(), oversampledBuffer.getNumSamples() / 2 + 4);
                
                // First stage: oversampledBuffer -> tempBuffer
                int n2 = safeDownsamplerX2.processBlock(oversampledBuffer, tempBuffer);
                tempBuffer.setSize(tempBuffer.getNumChannels(), n2, true, true, true);
                
                // Second stage: tempBuffer -> buffer 
                safeDownsamplerX2.processBlock(tempBuffer, buffer);
            }
            else if (currentFactor == 8)
            {
                // For 8x, use 2-stage approach (/2, then /4)
                juce::AudioBuffer<float> tempBuffer;
                tempBuffer.setSize(oversampledBuffer.getNumChannels(), oversampledBuffer.getNumSamples() / 2 + 4);
                
                // First stage: oversampledBuffer -> tempBuffer
                int n2 = safeDownsamplerX2.processBlock(oversampledBuffer, tempBuffer);
                tempBuffer.setSize(tempBuffer.getNumChannels(), n2, true, true, true);
                
                // Second stage: tempBuffer -> buffer (using /4 downsampler)
                safeDownsamplerX4.processBlock(tempBuffer, buffer);
            }
#else
            // Original safe approach with JUCE downsampling
            if ((currentFactor == 4 && oversamplerX4 == nullptr) ||
                (currentFactor == 8 && oversamplerX8 == nullptr))
            {
                buffer.clear();
                return;
            }
            
            // Create a properly sized working buffer that we control
            const int numChannels = juce::jmin(buffer.getNumChannels(), oversampledBuffer.getNumChannels());
            
            // Create an intermediate buffer for safe processing
            juce::AudioBuffer<float> workBuffer;
            workBuffer.setSize(numChannels, oversampledBuffer.getNumSamples(), false, true);
            
            // Copy data to our working buffer
            for (int ch = 0; ch < numChannels; ++ch)
            {
                workBuffer.copyFrom(ch, 0, oversampledBuffer, ch, 0, 
                                   juce::jmin(oversampledBuffer.getNumSamples(), workBuffer.getNumSamples()));
            }
            
            // Create a dsp::AudioBlock using our working buffer
            juce::dsp::AudioBlock<float> audioBlock(workBuffer);
            
            // Clear the output buffer before processing
            buffer.clear();
            
            // Process with appropriate oversampler
            if (currentFactor == 4 && oversamplerX4 != nullptr)
            {
                oversamplerX4->processSamplesDown(audioBlock);
                
                // Copy the result safely to output
                const int outSamples = juce::jmin(buffer.getNumSamples(), workBuffer.getNumSamples() / currentFactor);
                
                for (int ch = 0; ch < numChannels; ++ch)
                {
                    buffer.copyFrom(ch, 0, workBuffer, ch, 0, outSamples);
                }
            }
            else if (currentFactor == 8 && oversamplerX8 != nullptr)
            {
                oversamplerX8->processSamplesDown(audioBlock);
                
                // Copy the result safely to output
                const int outSamples = juce::jmin(buffer.getNumSamples(), workBuffer.getNumSamples() / currentFactor);
                
                for (int ch = 0; ch < numChannels; ++ch)
                {
                    buffer.copyFrom(ch, 0, workBuffer, ch, 0, outSamples);
                }
            }
#endif
        }
        catch (std::exception& e) {
            // Handle any exceptions with simple fallback
            DBG("Downsampling error: " + juce::String(e.what()));
            
            // Very simple decimation in case of any failure
            buffer.clear();
            const int numChannels = juce::jmin(buffer.getNumChannels(), oversampledBuffer.getNumChannels());
            const int outSamples = juce::jmin(buffer.getNumSamples(), 
                                            oversampledBuffer.getNumSamples() / currentFactor);
            
            for (int ch = 0; ch < numChannels; ++ch)
            {
                const float* inData = oversampledBuffer.getReadPointer(ch);
                float* outData = buffer.getWritePointer(ch);
                
                for (int i = 0; i < outSamples; ++i)
                {
                    const int srcIdx = i * currentFactor;
                    if (srcIdx < oversampledBuffer.getNumSamples())
                    {
                        outData[i] = inData[srcIdx] * 0.9f;
                    }
                }
            }
        }
    }
    
    void reset()
    {
        try {
            if (oversamplerX4 != nullptr)
                oversamplerX4->reset();
            
            if (oversamplerX8 != nullptr)
                oversamplerX8->reset();
                
#if MLAR_USE_SAFE_DOWNSAMPLER
            // No explicit reset needed for SafeDownsampler
#endif
        }
        catch (...) {
            // Protect against any exceptions during reset
            DBG("Exception in oversampler reset");
        }
    }
    
    // Return the downsampler ID for diagnostic purposes
    juce::String getDownsamplerId() const
    {
        return downsamplerId;
    }
    
    // Return the latency introduced by downsampling (in samples)
    int getLatencySamples() const
    {
#if MLAR_USE_SAFE_DOWNSAMPLER
        // Two stages for 4x, each with its own latency
        if (currentFactor == 4)
            return safeDownsamplerX2.getLatencySamples() * 2;
        else if (currentFactor == 8)
            return safeDownsamplerX2.getLatencySamples() + safeDownsamplerX4.getLatencySamples();
        return 0;
#else
        return 0; // JUCE doesn't report latency easily
#endif
    }

private:
    std::unique_ptr<juce::dsp::Oversampling<float>> oversamplerX4;
    std::unique_ptr<juce::dsp::Oversampling<float>> oversamplerX8;
    int currentFactor;
    double sampleRate;
    bool isInitialized;
    juce::String downsamplerId;
    
#if MLAR_USE_SAFE_DOWNSAMPLER
    // High-quality safe downsamplers
    moremojo::dsp::SafeDownsampler safeDownsamplerX2; // For /2 stages
    moremojo::dsp::SafeDownsampler safeDownsamplerX4; // For /4 stage
#endif
};

//==============================================================================
// AnalogProcessor class - handles analog shaping simulation
//==============================================================================
class AnalogProcessor
{
public:
    AnalogProcessor() {}
    
    void prepare(double sampleRate, int numChannels)
    {
        this->sampleRate = sampleRate;
    }
    
    void processStereo(float* left, float* right, int numSamples, 
                       float drive, float character, float saturation, float presence)
    {
        // ADAA (Anti-derivatives anti-aliasing) saturator for low aliasing
        for (int i = 0; i < numSamples; ++i)
        {
            // Shape the input with drive and character
            float shapeL = processSample(left[i], drive, character, saturation, presence);
            float shapeR = processSample(right[i], drive, character, saturation, presence);
            
            // Apply the shaped signal
            left[i] = shapeL;
            right[i] = shapeR;
        }
    }
    
    float processSample(float input, float drive, float character, float saturation, float presence)
    {
        // Adjust the input gain based on drive (0.0-1.0)
        const float driveGain = 1.0f + (drive * 10.0f);
        input *= driveGain;
        
        // Apply character - changes the shape of the distortion
        // Higher character = more asymmetrical, tube-like distortion
        const float characterAmount = character * 0.9f; 
        input += characterAmount * std::sin(input);
        
        // Apply soft saturation
        const float saturationAmount = saturation * 0.5f + 0.5f;
        input = std::tanh(input * saturationAmount) / std::tanh(saturationAmount);
        
        // Apply presence - high frequency enhancement
        const float presenceFactor = presence * 0.6f;
        static float lastInput = 0.0f;
        const float highFreq = input - lastInput;
        input += highFreq * presenceFactor;
        lastInput = input;
        
        return input;
    }
    
private:
    double sampleRate = 44100.0;
};

//==============================================================================
// Presets implementation
//==============================================================================
struct PresetParameter {
    float drive;
    float character;
    float saturation;
    float presence;
    float mix;
    float output;
    int interpMode;
};

struct Preset {
    juce::String name;
    PresetParameter params;
};

// Preset definitions - mirroring the app's presets
static std::vector<Preset> getPresets()
{
    return {
        // Vocals presets
        { "Vocal - Mojo", { 0.4f, 0.5f, 0.4f, 0.6f, 0.8f, 0.0f, MoreMojoAudioProcessor::LiveHB4x } },
        { "Vocal - More Mojo", { 0.6f, 0.55f, 0.55f, 0.7f, 1.0f, 0.2f, MoreMojoAudioProcessor::Adaptive } },
        { "Vocal - Most Mojo", { 0.7f, 0.6f, 0.7f, 0.8f, 1.0f, 0.5f, MoreMojoAudioProcessor::HQSinc8x } },
        
        // Instrument presets
        { "Instrument - Mojo", { 0.5f, 0.45f, 0.5f, 0.5f, 0.7f, 0.0f, MoreMojoAudioProcessor::LiveHB4x } },
        { "Instrument - More Mojo", { 0.65f, 0.6f, 0.6f, 0.6f, 0.9f, 0.3f, MoreMojoAudioProcessor::Adaptive } },
        { "Instrument - Most Mojo", { 0.8f, 0.7f, 0.75f, 0.7f, 1.0f, 0.5f, MoreMojoAudioProcessor::HQSinc8x } },
        
        // Bus presets
        { "Bus - Mojo", { 0.3f, 0.4f, 0.4f, 0.5f, 0.6f, 0.0f, MoreMojoAudioProcessor::LiveHB4x } },
        { "Bus - More Mojo", { 0.5f, 0.5f, 0.5f, 0.6f, 0.8f, 0.2f, MoreMojoAudioProcessor::TransientSpline4x } },
        { "Bus - Most Mojo", { 0.7f, 0.6f, 0.6f, 0.7f, 1.0f, 0.4f, MoreMojoAudioProcessor::HQSinc8x } },
        
        // Master presets
        { "Master - Mojo", { 0.25f, 0.3f, 0.3f, 0.4f, 0.5f, 0.0f, MoreMojoAudioProcessor::LiveHB4x } },
        { "Master - More Mojo", { 0.4f, 0.45f, 0.45f, 0.5f, 0.7f, 0.2f, MoreMojoAudioProcessor::Adaptive } },
        { "Master - Most Mojo", { 0.6f, 0.55f, 0.55f, 0.6f, 0.9f, 0.3f, MoreMojoAudioProcessor::HQSinc8x } }
    };
}

//==============================================================================
// AudioProcessor implementation
//==============================================================================
MoreMojoAudioProcessor::MoreMojoAudioProcessor()
    : AudioProcessor (BusesProperties()
                     .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                     .withOutput ("Output", juce::AudioChannelSet::stereo(), true)),
      parameters(*this, nullptr, "Parameters", {
          std::make_unique<juce::AudioParameterFloat>(PARAM_DRIVE, "Drive", 0.0f, 1.0f, 0.5f),
          std::make_unique<juce::AudioParameterFloat>(PARAM_CHARACTER, "Character", 0.0f, 1.0f, 0.5f),
          std::make_unique<juce::AudioParameterFloat>(PARAM_SATURATION, "Saturation", 0.0f, 1.0f, 0.5f),
          std::make_unique<juce::AudioParameterFloat>(PARAM_PRESENCE, "Presence", 0.0f, 1.0f, 0.5f),
          std::make_unique<juce::AudioParameterFloat>(PARAM_MIX, "Mix", 0.0f, 1.0f, 1.0f),
          std::make_unique<juce::AudioParameterFloat>(PARAM_OUTPUT, "Output", -12.0f, 12.0f, 0.0f),
          std::make_unique<juce::AudioParameterChoice>(PARAM_INTERP_MODE, "Quality", 
                                                      juce::StringArray {"Live (4×)", "HQ (8×)", "Transient (4×)", "Adaptive", "AI (8×)"},
                                                      LiveHB4x),
          std::make_unique<juce::AudioParameterBool>(PARAM_MLAR_ENABLED, "MLAR", false)
      })
{
    driveParameter = parameters.getRawParameterValue(PARAM_DRIVE);
    characterParameter = parameters.getRawParameterValue(PARAM_CHARACTER);
    saturationParameter = parameters.getRawParameterValue(PARAM_SATURATION);
    presenceParameter = parameters.getRawParameterValue(PARAM_PRESENCE);
    mixParameter = parameters.getRawParameterValue(PARAM_MIX);
    outputParameter = parameters.getRawParameterValue(PARAM_OUTPUT);
    interpModeParameter = parameters.getRawParameterValue(PARAM_INTERP_MODE);
    
    oversampler = std::make_unique<Oversampler>();
    analogProcessor = std::make_unique<AnalogProcessor>();
    
    loadPresets();
}

MoreMojoAudioProcessor::~MoreMojoAudioProcessor() {}

void MoreMojoAudioProcessor::loadPresets()
{
    auto presets = getPresets();
    presetNames.clear();
    
    for (const auto& preset : presets)
    {
        presetNames.add(preset.name);
    }
}

const juce::String MoreMojoAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool MoreMojoAudioProcessor::acceptsMidi() const
{
    return false;
}

bool MoreMojoAudioProcessor::producesMidi() const
{
    return false;
}

bool MoreMojoAudioProcessor::isMidiEffect() const
{
    return false;
}

double MoreMojoAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int MoreMojoAudioProcessor::getNumPrograms()
{
    return presetNames.size();
}

int MoreMojoAudioProcessor::getCurrentProgram()
{
    return 0; // Placeholder, to be implemented
}

void MoreMojoAudioProcessor::setCurrentProgram(int index)
{
    if (index >= 0 && index < presetNames.size())
    {
        auto presets = getPresets();
        const auto& preset = presets[index];
        
        parameters.getParameter(PARAM_DRIVE)->setValueNotifyingHost(preset.params.drive);
        parameters.getParameter(PARAM_CHARACTER)->setValueNotifyingHost(preset.params.character);
        parameters.getParameter(PARAM_SATURATION)->setValueNotifyingHost(preset.params.saturation);
        parameters.getParameter(PARAM_PRESENCE)->setValueNotifyingHost(preset.params.presence);
        parameters.getParameter(PARAM_MIX)->setValueNotifyingHost(preset.params.mix);
        parameters.getParameter(PARAM_OUTPUT)->setValueNotifyingHost(juce::jmap(preset.params.output, -12.0f, 12.0f, 0.0f, 1.0f));
        parameters.getParameter(PARAM_INTERP_MODE)->setValueNotifyingHost((float)preset.params.interpMode / 4.0f);
    }
}

const juce::String MoreMojoAudioProcessor::getProgramName(int index)
{
    return index >= 0 && index < presetNames.size() ? presetNames[index] : "";
}

void MoreMojoAudioProcessor::changeProgramName(int index, const juce::String& newName)
{
    // Not implemented
}

void MoreMojoAudioProcessor::prepareToPlay(double sampleRate, int samplesPerBlock)
{
    // Determine whether to use 4× or 8× oversampling
    int interpMode = (int)*interpModeParameter;
    int oversamplingFactor = (interpMode == HQSinc8x || interpMode == AIAnalogHook) ? 8 : 4;
    
    // If offline rendering, force HQ (8×)
    if (isNonRealtime())
    {
        oversamplingFactor = 8;
    }
    
    // Initialize the oversampler
    juce::dsp::ProcessSpec spec;
    spec.sampleRate = sampleRate;
    spec.maximumBlockSize = samplesPerBlock;
    spec.numChannels = getTotalNumOutputChannels();
    
    oversampler->prepare(spec, oversamplingFactor);
    analogProcessor->prepare(sampleRate * oversamplingFactor, getTotalNumOutputChannels());
    
    // Initialize MLAR processor
    mlarProcessor.prepare(sampleRate, samplesPerBlock);
}

void MoreMojoAudioProcessor::releaseResources()
{
    oversampler->reset();
}

bool MoreMojoAudioProcessor::isBusesLayoutSupported(const BusesLayout& layouts) const
{
    return layouts.getMainOutputChannelSet() == juce::AudioChannelSet::stereo();
}

void MoreMojoAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    // Clear any unused output channels
    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear(i, 0, buffer.getNumSamples());
    
    // Store the dry signal for mix
    juce::AudioBuffer<float> dryBuffer;
    dryBuffer.makeCopyOf(buffer);

    // Get current parameters
    float drive = *driveParameter;
    float character = *characterParameter;
    float saturation = *saturationParameter;
    float presence = *presenceParameter;
    float mix = *mixParameter;
    float output = std::pow(10.0f, *outputParameter / 20.0f); // dB to gain
    
    // Check if MLAR is enabled
    mlarEnabled = *parameters.getRawParameterValue(PARAM_MLAR_ENABLED) > 0.5f;
    
    // Determine whether to use 4× or 8× oversampling
    int interpMode = (int)*interpModeParameter;
    
    // If offline rendering, force HQ (8×)
    if (isNonRealtime() && oversampler->getOversamplingFactor() != 8)
    {
        // Re-initialize the oversampler for 8× processing
        juce::dsp::ProcessSpec spec;
        spec.sampleRate = getSampleRate();
        spec.maximumBlockSize = buffer.getNumSamples();
        spec.numChannels = totalNumOutputChannels;
        
        oversampler->prepare(spec, 8);
    }

    // Store a dry copy of the buffer for wet/dry mixing
    dryBuffer.makeCopyOf(buffer);
    
    // Process with MLAR if enabled
    if (mlarEnabled) {
        // Use the MLAR processor directly with proper buffer protection
        try {
            // Process only up to stereo channels for MLAR
            const int numChannels = juce::jmin(2, buffer.getNumChannels());
            const int numSamples = buffer.getNumSamples();
            
            // Create a working copy to avoid in-place buffer issues
            juce::AudioBuffer<float> workBuffer(numChannels, numSamples);
            workBuffer.makeCopyOf(buffer);
            
            // Process each channel through MLAR with proper error handling
            for (int channel = 0; channel < numChannels; ++channel) {
                float* channelData = workBuffer.getWritePointer(channel);
                if (channelData != nullptr && numSamples > 0) {
                    // Process through MLAR
                    mlarProcessor.process(channelData, 1, numSamples);
                }
            }
            
            // Copy processed data back to the main buffer
            for (int ch = 0; ch < numChannels; ++ch) {
                buffer.copyFrom(ch, 0, workBuffer, ch, 0, numSamples);
            }
        } 
        catch (std::exception& e) {
            DBG("MLAR processing error: " + juce::String(e.what()));
            // In case of error, return the unprocessed buffer
            buffer.makeCopyOf(dryBuffer);
        }
    }
    else {
        // Standard processing with oversampling
        try {
            // Extra safety checks for buffer validity
            if (buffer.getNumChannels() > 0 && buffer.getNumSamples() > 0 && oversampler != nullptr) {
                // Pre-allocate oversampled buffer with correct size
                const int oversampledSamples = buffer.getNumSamples() * oversampler->getOversamplingFactor();
                const int numChannels = buffer.getNumChannels();
                
                juce::AudioBuffer<float> oversampledBuffer(numChannels, oversampledSamples);
                oversampledBuffer.clear();
                
                // Process samples up to higher sample rate with error handling
                try {
                    oversampler->processSamplesUp(buffer, oversampledBuffer);
                    
                    // Extra validation before analog processing
                    if (analogProcessor != nullptr && 
                        oversampledBuffer.getNumChannels() >= 2 && 
                        oversampledBuffer.getNumSamples() > 0)
                    {
                        // Get direct pointers to channels with validation
                        float* leftChannel = oversampledBuffer.getWritePointer(0);
                        float* rightChannel = oversampledBuffer.getWritePointer(1);
                        
                        if (leftChannel != nullptr && rightChannel != nullptr) {
                            // Process through analog processor
                            analogProcessor->processStereo(
                                leftChannel,
                                rightChannel,
                                oversampledBuffer.getNumSamples(),
                                drive, 
                                character,
                                saturation,
                                presence
                            );
                        }
                    }
                    
                    // Process back down to original sample rate with error handling
                    try {
                        oversampler->processSamplesDown(oversampledBuffer, buffer);
                    }
                    catch (std::exception& e) {
                        DBG("Downsampling error: " + juce::String(e.what()));
                        // Restore the dry signal on error
                        buffer.makeCopyOf(dryBuffer);
                    }
                }
                catch (std::exception& e) {
                    DBG("Upsampling error: " + juce::String(e.what()));
                    // Restore the dry signal on error
                    buffer.makeCopyOf(dryBuffer);
                }
            }
        }
        catch (std::exception& e) {
            DBG("Standard processing error: " + juce::String(e.what()));
            // In case of error, return the unprocessed buffer
            buffer.makeCopyOf(dryBuffer);
        }
    }
    
    // Apply wet/dry mix and output gain
    for (int channel = 0; channel < totalNumOutputChannels; ++channel)
    {
        auto* channelData = buffer.getWritePointer(channel);
        auto* dryData = dryBuffer.getReadPointer(channel);
        
        for (int sample = 0; sample < buffer.getNumSamples(); ++sample)
        {
            // Mix dry and wet
            channelData[sample] = (1.0f - mix) * dryData[sample] + mix * channelData[sample];
            
            // Apply output gain
            channelData[sample] *= output;
        }
    }
}

bool MoreMojoAudioProcessor::hasEditor() const
{
    return true;
}

juce::AudioProcessorEditor* MoreMojoAudioProcessor::createEditor()
{
    return new juce::GenericAudioProcessorEditor(*this);
}

void MoreMojoAudioProcessor::getStateInformation(juce::MemoryBlock& destData)
{
    auto state = parameters.copyState();
    std::unique_ptr<juce::XmlElement> xml(state.createXml());
    copyXmlToBinary(*xml, destData);
}

void MoreMojoAudioProcessor::setStateInformation(const void* data, int sizeInBytes)
{
    std::unique_ptr<juce::XmlElement> xmlState(getXmlFromBinary(data, sizeInBytes));
    
    if (xmlState.get() != nullptr)
        if (xmlState->hasTagName(parameters.state.getType()))
            parameters.replaceState(juce::ValueTree::fromXml(*xmlState));
}

// This creates new instances of the plugin
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new MoreMojoAudioProcessor();
}
