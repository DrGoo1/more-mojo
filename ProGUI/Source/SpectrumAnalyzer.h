#pragma once

#include <juce_core/juce_core.h>
#include <juce_events/juce_events.h>
#include <juce_graphics/juce_graphics.h>
#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_gui_extra/juce_gui_extra.h>
#include <juce_dsp/juce_dsp.h>
#include <juce_audio_basics/juce_audio_basics.h>

//==============================================================================
/**
    High-resolution spectrum analyzer component with FFT visualization
*/
class SpectrumAnalyzer : public juce::Component,
                         private juce::Timer
{
public:
    SpectrumAnalyzer();
    ~SpectrumAnalyzer() override;

    void paint(juce::Graphics& g) override;
    void resized() override;
    
    // Push new audio samples for analysis
    void pushSamples(const float* samples, int numChannels);
    
    // FFT settings
    void setFFTSize(int size);
    void setAveraging(float newAveraging);
    
    // Display settings
    void setDecayRate(float decayRate);
    void setPeakHold(bool shouldHoldPeaks);
    void setPeakDecay(float peakDecay);
    
private:
    void timerCallback() override;
    
    // FFT processor
    juce::dsp::FFT fft;
    int fftSize = 2048;
    int fftSizeHalf = fftSize / 2;
    
    // FFT data buffers
    std::vector<float> fftData;
    std::vector<float> fftWindow;
    std::vector<float> averager;
    std::vector<float> peaks;
    
    // Audio input buffer
    juce::AudioBuffer<float> inputBuffer;
    int inputBufferWritePos = 0;
    
    // Analysis settings
    float averaging = 0.5f;
    float decayRate = 0.95f;
    float peakDecay = 0.98f;
    bool holdPeaks = true;
    
    // Visualization colors
    juce::Colour backgroundColour = juce::Colour(0xff101010);
    juce::Colour gridColour = juce::Colour(0xff333333);
    juce::Colour mainGradientColourTop = juce::Colour(0xff4285f4).withAlpha(0.9f);
    juce::Colour mainGradientColourBottom = juce::Colour(0xff4285f4).withAlpha(0.1f);
    juce::Colour peakColour = juce::Colour(0xffea4335);
    
    // Helper methods
    void drawBackground(juce::Graphics& g);
    void drawGridLines(juce::Graphics& g);
    void drawTextLabels(juce::Graphics& g);
    void drawSpectrum(juce::Graphics& g);
    
    // Generate Hann window for FFT
    void createWindow();
    
    // Map frequency to x coordinate
    float getXForFrequency(float frequency, float width);
    
    // Map amplitude to y coordinate
    float getYForAmplitude(float amplitude, float height);
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SpectrumAnalyzer)
};
