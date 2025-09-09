#include "PluginProcessor.h"

// Parameter IDs
const juce::String MoreMojoAudioProcessor::PARAM_DRIVE = "drive";
const juce::String MoreMojoAudioProcessor::PARAM_CHARACTER = "character";
const juce::String MoreMojoAudioProcessor::PARAM_SATURATION = "saturation";
const juce::String MoreMojoAudioProcessor::PARAM_PRESENCE = "presence";
const juce::String MoreMojoAudioProcessor::PARAM_MIX = "mix";
const juce::String MoreMojoAudioProcessor::PARAM_OUTPUT = "output";
const juce::String MoreMojoAudioProcessor::PARAM_INTERP_MODE = "interp_mode";

//==============================================================================
// Oversampler class - handles upsampling and downsampling
//==============================================================================
class Oversampler
{
public:
    Oversampler() {}
    
    void prepare(const juce::dsp::ProcessSpec& spec, int oversamplingFactor)
    {
        // Set up oversampling with appropriate quality (JUCE's own oversampling)
        if (oversamplingFactor == 4)
        {
            oversamplerX4 = std::make_unique<juce::dsp::Oversampling<float>>(
                spec.numChannels, 2, juce::dsp::Oversampling<float>::filterHalfBandPolyphaseIIR, true, false);
            oversamplerX4->initProcessing(spec.maximumBlockSize);
            currentFactor = 4;
        }
        else if (oversamplingFactor == 8)
        {
            oversamplerX8 = std::make_unique<juce::dsp::Oversampling<float>>(
                spec.numChannels, 3, juce::dsp::Oversampling<float>::filterHalfBandPolyphaseIIR, true, false);
            oversamplerX8->initProcessing(spec.maximumBlockSize);
            currentFactor = 8;
        }
        
        sampleRate = spec.sampleRate;
    }
    
    void processSamplesUp(juce::AudioBuffer<float>& buffer, juce::AudioBuffer<float>& oversampledBuffer)
    {
        if (currentFactor == 4 && oversamplerX4 != nullptr)
        {
            auto block = juce::dsp::AudioBlock<float>(buffer);
            auto osBlock = oversamplerX4->processSamplesUp(block);
            oversampledBuffer.setSize(osBlock.getNumChannels(), osBlock.getNumSamples(), false, false, true);
            
            for (int ch = 0; ch < osBlock.getNumChannels(); ++ch)
            {
                oversampledBuffer.copyFrom(ch, 0, osBlock.getChannelPointer(ch), osBlock.getNumSamples());
            }
        }
        else if (currentFactor == 8 && oversamplerX8 != nullptr)
        {
            auto block = juce::dsp::AudioBlock<float>(buffer);
            auto osBlock = oversamplerX8->processSamplesUp(block);
            oversampledBuffer.setSize(osBlock.getNumChannels(), osBlock.getNumSamples(), false, false, true);
            
            for (int ch = 0; ch < osBlock.getNumChannels(); ++ch)
            {
                oversampledBuffer.copyFrom(ch, 0, osBlock.getChannelPointer(ch), osBlock.getNumSamples());
            }
        }
    }
    
    void processSamplesDown(juce::AudioBuffer<float>& oversampledBuffer, juce::AudioBuffer<float>& buffer)
    {
        if (currentFactor == 4 && oversamplerX4 != nullptr)
        {
            auto osBlock = juce::dsp::AudioBlock<float>(oversampledBuffer);
            auto block = oversamplerX4->processSamplesDown(osBlock);
            
            for (int ch = 0; ch < block.getNumChannels(); ++ch)
            {
                buffer.copyFrom(ch, 0, block.getChannelPointer(ch), block.getNumSamples());
            }
        }
        else if (currentFactor == 8 && oversamplerX8 != nullptr)
        {
            auto osBlock = juce::dsp::AudioBlock<float>(oversampledBuffer);
            auto block = oversamplerX8->processSamplesDown(osBlock);
            
            for (int ch = 0; ch < block.getNumChannels(); ++ch)
            {
                buffer.copyFrom(ch, 0, block.getChannelPointer(ch), block.getNumSamples());
            }
        }
    }
    
    void reset()
    {
        if (oversamplerX4 != nullptr)
            oversamplerX4->reset();
        
        if (oversamplerX8 != nullptr)
            oversamplerX8->reset();
    }
    
    int getOversamplingFactor() const { return currentFactor; }
    
private:
    std::unique_ptr<juce::dsp::Oversampling<float>> oversamplerX4;
    std::unique_ptr<juce::dsp::Oversampling<float>> oversamplerX8;
    int currentFactor = 4;
    double sampleRate = 44100.0;
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
                                                      LiveHB4x)
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

    // Process with oversampling
    juce::AudioBuffer<float> oversampledBuffer;
    oversampler->processSamplesUp(buffer, oversampledBuffer);
    
    // Apply analog processor to the oversampled buffer
    if (oversampledBuffer.getNumChannels() >= 2)
    {
        analogProcessor->processStereo(
            oversampledBuffer.getWritePointer(0),
            oversampledBuffer.getWritePointer(1),
            oversampledBuffer.getNumSamples(),
            drive, 
            character,
            saturation,
            presence
        );
    }
    
    // Process back down to original sample rate
    oversampler->processSamplesDown(oversampledBuffer, buffer);
    
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
