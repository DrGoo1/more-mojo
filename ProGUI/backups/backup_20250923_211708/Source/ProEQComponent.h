#pragma once

#include "../../JUCE/modules/juce_core/juce_core.h"
#include "../../JUCE/modules/juce_events/juce_events.h"
#include "../../JUCE/modules/juce_graphics/juce_graphics.h"
#include "../../JUCE/modules/juce_gui_basics/juce_gui_basics.h"
#include "../../JUCE/modules/juce_gui_extra/juce_gui_extra.h"
#include "../../JUCE/modules/juce_audio_basics/juce_audio_basics.h"
#include "SpectrumAnalyzer.h"

//==============================================================================
/**
    Professional EQ component with spectrum analyzer and 
    customizable parametric EQ bands
*/
class ProEQComponent : public juce::Component,
                       private juce::Timer
{
public:
    ProEQComponent();
    ~ProEQComponent() override;

    void paint(juce::Graphics& g) override;
    void resized() override;
    
    void timerCallback() override;
    
    // Called when processing audio to update the spectrum analyzer
    void pushNextSampleIntoAnalyzer(const float* inputChannelData, int numChannels);
    
    // API for external control (used by wrapper/Swift bridge)
    void setBandGain(int bandIndex, float gainDB);
    void setBandFrequency(int bandIndex, float frequency);
    void setBandQ(int bandIndex, float q);
    float getBandGain(int bandIndex) const;
    float getBandFrequency(int bandIndex) const;
    float getBandQ(int bandIndex) const;
    
    // Simple pass-through processing for visualization
    void processAudio(juce::AudioBuffer<float>& buffer);
    
private:
    // Spectrum analyzer
    std::unique_ptr<SpectrumAnalyzer> spectrumAnalyzer;
    
    // EQ parameters
    struct EQBand {
        std::unique_ptr<juce::Slider> frequencySlider;
        std::unique_ptr<juce::Slider> gainSlider;
        std::unique_ptr<juce::Slider> qSlider;
        std::unique_ptr<juce::ComboBox> typeSelector;
        juce::Label nameLabel;
        juce::Colour colour;
        bool isActive = true;
        
        // Filter parameters
        float frequency = 1000.0f;
        float gain = 0.0f;
        float q = 1.0f;
        int type = 0; // 0 = Bell, 1 = Low Shelf, 2 = High Shelf, 3 = Low Pass, 4 = High Pass, 5 = Notch
    };
    
    // EQ bands
    static const int numBands = 5;
    std::array<EQBand, numBands> bands;
    
    // Response curve calculation
    juce::Path responseCurve;
    void updateResponseCurve();
    
    // Band colors for visualization
    juce::Array<juce::Colour> bandColours;
    
    // EQ frequency response area
    juce::Rectangle<int> frequencyResponseArea;
    
    // Helper methods
    void createBands();
    void drawFrequencyScale(juce::Graphics& g);
    void drawGainScale(juce::Graphics& g);
    void drawResponseCurve(juce::Graphics& g);
    void drawFrequencyMarkers(juce::Graphics& g);
    void drawBackground(juce::Graphics& g);
    
    // Map frequency to x position
    float getXForFrequency(float frequency);
    
    // Map gain to y position
    float getYForGain(float gainDB);
    
    // Map x position to frequency
    float getFrequencyForX(float x);
    
    // Map y position to gain
    float getGainForY(float y);
    
    // Helper to format frequency text
    static juce::String formatFrequency(float freq);
    
    // Calculate filter response at a given frequency
    float getFilterMagnitudeForFrequency(const EQBand& band, float frequency);
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ProEQComponent)
};
