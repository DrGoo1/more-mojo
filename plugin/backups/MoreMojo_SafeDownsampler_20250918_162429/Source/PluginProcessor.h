#pragma once

// Include JUCE modules directly instead of using JuceHeader.h
#include "../../JUCE/modules/juce_audio_basics/juce_audio_basics.h"
#include "../../JUCE/modules/juce_audio_devices/juce_audio_devices.h"
#include "../../JUCE/modules/juce_audio_formats/juce_audio_formats.h"
#include "../../JUCE/modules/juce_audio_plugin_client/juce_audio_plugin_client.h"
#include "../../JUCE/modules/juce_audio_processors/juce_audio_processors.h"
#include "../../JUCE/modules/juce_audio_utils/juce_audio_utils.h"
#include "../../JUCE/modules/juce_core/juce_core.h"
#include "../../JUCE/modules/juce_data_structures/juce_data_structures.h"
#include "../../JUCE/modules/juce_dsp/juce_dsp.h"
#include "../../JUCE/modules/juce_events/juce_events.h"
#include "../../JUCE/modules/juce_graphics/juce_graphics.h"
#include "../../JUCE/modules/juce_gui_basics/juce_gui_basics.h"
#include "../../JUCE/modules/juce_gui_extra/juce_gui_extra.h"

// Include MLAR module
#include "../../shared/mlar/MLAR.h"

// Forward declarations
class Oversampler;
class AnalogProcessor;

//==============================================================================
class MoreMojoAudioProcessor  : public juce::AudioProcessor
{
public:
    //==============================================================================
    MoreMojoAudioProcessor();
    ~MoreMojoAudioProcessor() override;

    //==============================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;

    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    //==============================================================================
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    //==============================================================================
    const juce::String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    //==============================================================================
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override;
    void changeProgramName (int index, const juce::String& newName) override;

    //==============================================================================
    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    //==============================================================================
    enum InterpolationMode {
        LiveHB4x = 0,
        HQSinc8x,
        TransientSpline4x,
        Adaptive,
        AIAnalogHook
    };
    
    // Parameter IDs
    static const juce::String PARAM_DRIVE;
    static const juce::String PARAM_CHARACTER;
    static const juce::String PARAM_SATURATION;
    static const juce::String PARAM_PRESENCE;
    static const juce::String PARAM_MIX;
    static const juce::String PARAM_OUTPUT;
    static const juce::String PARAM_INTERP_MODE;
    static const juce::String PARAM_MLAR_ENABLED;

private:
    //==============================================================================
    // Parameters
    juce::AudioProcessorValueTreeState parameters;
    std::atomic<float>* driveParameter = nullptr;
    std::atomic<float>* characterParameter = nullptr;
    std::atomic<float>* saturationParameter = nullptr;
    std::atomic<float>* presenceParameter = nullptr;
    std::atomic<float>* mixParameter = nullptr;
    std::atomic<float>* outputParameter = nullptr;
    std::atomic<float>* interpModeParameter = nullptr;
    
    // DSP objects
    std::unique_ptr<Oversampler> oversampler;
    std::unique_ptr<AnalogProcessor> analogProcessor;
    
    // MLAR integration
    moremojo::mlar::Processor mlarProcessor;
    bool mlarEnabled = false;
    
    // Preset handling
    juce::StringArray presetNames;
    void loadPresets();
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MoreMojoAudioProcessor)
};
