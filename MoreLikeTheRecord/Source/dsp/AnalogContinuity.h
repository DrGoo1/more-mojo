#pragma once
#include <JuceHeader.h>

namespace mojoDSP {

/**
 * AnalogContinuity - Subtle analog-like enhancements for spatial cues
 * 
 * Features:
 * - Gentle elliptical filter for low frequencies
 * - Stereo crosstalk matrix with frequency tilt
 * - Stylus envelope shaper for micro asymmetry
 */
class AnalogContinuity {
public:
    AnalogContinuity() = default;
    
    void prepare(double sampleRate, int maxBlockSize) {
        this->sampleRate = sampleRate;
        
        // Initialize filters
        for (int ch = 0; ch < 2; ++ch) {
            // Elliptical LF filter
            ellipticalFilter[ch].prepare({sampleRate, static_cast<juce::uint32>(maxBlockSize), 1});
            
            // Crosstalk filters (frequency-dependent crosstalk)
            for (auto& filter : crosstalkFilters[ch]) {
                filter.prepare({sampleRate, static_cast<juce::uint32>(maxBlockSize), 1});
            }
        }
        
        // Set default parameters
        updateFilterCoefficients();
        
        // Initialize buffers
        tempBuffer.setSize(2, maxBlockSize);
        
        reset();
    }
    
    void reset() {
        // Reset filters
        for (int ch = 0; ch < 2; ++ch) {
            ellipticalFilter[ch].reset();
            for (auto& filter : crosstalkFilters[ch]) {
                filter.reset();
            }
        }
        
        // Clear previous state
        phase = 0.0f;
    }
    
    void setParameters(float elliptFreq, int elliptSlope, float xtalkTilt, float asymmetry, float mix) {
        // Validate and update parameters
        this->ellipticalFreq = juce::jlimit(20.0f, 500.0f, elliptFreq);
        this->ellipticalSlope = juce::jlimit(6, 24, elliptSlope);
        this->crosstalkTilt = juce::jlimit(-60.0f, -20.0f, xtalkTilt);
        this->asymmetryAmount = juce::jlimit(0.0f, 10.0f, asymmetry);
        this->wetMix = juce::jlimit(0.0f, 1.0f, mix);
        
        // Update filter coefficients
        updateFilterCoefficients();
    }
    
    void process(juce::AudioBuffer<float>& buffer, juce::AudioProcessorValueTreeState& apvts) {
        const int numChannels = buffer.getNumChannels();
        const int numSamples = buffer.getNumSamples();
        
        // Need stereo for analog continuity processing
        if (numChannels < 2 || numSamples <= 0) {
            return;
        }
        
        // Get parameters from APVTS
        const float elliptFreq = *apvts.getRawParameterValue("analog_elliptical_freq");
        const int elliptSlope = static_cast<int>(*apvts.getRawParameterValue("analog_elliptical_slope")) * 6;
        const float xtalkTilt = *apvts.getRawParameterValue("analog_crosstalk_tilt");
        const float asymmetry = *apvts.getRawParameterValue("analog_asymmetry");
        const float mix = *apvts.getRawParameterValue("analog_mix") * 0.01f; // 0-100 to 0-1
        
        // Update parameters
        setParameters(elliptFreq, elliptSlope, xtalkTilt, asymmetry, mix);
        
        // If mix is zero, do nothing
        if (wetMix <= 0.0f) {
            return;
        }
        
        // Copy original signal to temp buffer for dry/wet mix
        tempBuffer.copyFrom(0, 0, buffer, 0, 0, numSamples);
        tempBuffer.copyFrom(1, 0, buffer, 1, 0, numSamples);
        
        // Process the buffer
        
        // 1. Apply elliptical filter to low frequencies
        applyEllipticalFilter(buffer);
        
        // 2. Apply crosstalk
        applyCrosstalk(buffer);
        
        // 3. Apply stylus asymmetry
        applyAsymmetry(buffer);
        
        // Mix dry/wet
        for (int ch = 0; ch < 2; ++ch) {
            float* data = buffer.getWritePointer(ch);
            const float* dryData = tempBuffer.getReadPointer(ch);
            
            for (int i = 0; i < numSamples; ++i) {
                data[i] = dryData[i] * (1.0f - wetMix) + data[i] * wetMix;
            }
        }
    }
    
private:
    // Update filter coefficients based on current parameters
    void updateFilterCoefficients() {
        // Elliptical filter coefficients
        for (int ch = 0; ch < 2; ++ch) {
            // Create an elliptical low-pass filter for coupling
            *ellipticalFilter[ch].state = *juce::dsp::IIR::Coefficients<float>::makeLowPass(
                sampleRate, ellipticalFreq, 0.7071f);
        }
        
        // Crosstalk filter coefficients (frequency-dependent crosstalk)
        // Low, Mid, and High bands
        const float crossFreqs[3] = {100.0f, 1000.0f, 8000.0f};
        float crossAmounts[3];
        
        // Calculate frequency-dependent crosstalk amounts with tilt
        const float tiltDB = crosstalkTilt; // Negative value for tilt
        
        // Tilt the crosstalk amounts across frequency
        crossAmounts[0] = juce::Decibels::decibelsToGain(tiltDB - 5.0f); // More crosstalk in low frequencies
        crossAmounts[1] = juce::Decibels::decibelsToGain(tiltDB);        // Mid reference point
        crossAmounts[2] = juce::Decibels::decibelsToGain(tiltDB + 5.0f); // Less crosstalk in high frequencies
        
        for (int ch = 0; ch < 2; ++ch) {
            // Set up band-specific filters
            *crosstalkFilters[ch][0].state = *juce::dsp::IIR::Coefficients<float>::makeLowPass(
                sampleRate, crossFreqs[0], 0.7071f);
                
            *crosstalkFilters[ch][1].state = *juce::dsp::IIR::Coefficients<float>::makeBandPass(
                sampleRate, crossFreqs[0], crossFreqs[1], 0.7071f);
                
            *crosstalkFilters[ch][2].state = *juce::dsp::IIR::Coefficients<float>::makeHighPass(
                sampleRate, crossFreqs[2], 0.7071f);
        }
    }
    
    // Apply elliptical filter to couple low frequencies between L and R
    void applyEllipticalFilter(juce::AudioBuffer<float>& buffer) {
        const int numSamples = buffer.getNumSamples();
        float* leftData = buffer.getWritePointer(0);
        float* rightData = buffer.getWritePointer(1);
        
        // Apply elliptical filter
        juce::dsp::AudioBlock<float> leftBlock(&leftData, 1, numSamples);
        juce::dsp::AudioBlock<float> rightBlock(&rightData, 1, numSamples);
        
        juce::dsp::ProcessContextReplacing<float> leftContext(leftBlock);
        juce::dsp::ProcessContextReplacing<float> rightContext(rightBlock);
        
        ellipticalFilter[0].process(leftContext);
        ellipticalFilter[1].process(rightContext);
        
        // Create mid/side components
        for (int i = 0; i < numSamples; ++i) {
            const float left = leftData[i];
            const float right = rightData[i];
            
            // Convert to mid/side
            const float mid = (left + right) * 0.5f;
            const float side = (left - right) * 0.5f;
            
            // Enhance mid at very low frequencies (coupling effect)
            const float enhancedMid = mid * 1.05f;
            
            // Convert back to left/right
            leftData[i] = enhancedMid + side;
            rightData[i] = enhancedMid - side;
        }
    }
    
    // Apply crosstalk between channels with frequency tilt
    void applyCrosstalk(juce::AudioBuffer<float>& buffer) {
        const int numSamples = buffer.getNumSamples();
        
        // Split into frequency bands and apply crosstalk separately
        juce::AudioBuffer<float> bandBuffer;
        bandBuffer.setSize(2, numSamples);
        
        // Process each frequency band
        for (int band = 0; band < 3; ++band) {
            // Copy main buffer to band buffer
            bandBuffer.copyFrom(0, 0, buffer, 0, 0, numSamples);
            bandBuffer.copyFrom(1, 0, buffer, 1, 0, numSamples);
            
            // Apply band filter
            for (int ch = 0; ch < 2; ++ch) {
                float* data = bandBuffer.getWritePointer(ch);
                juce::dsp::AudioBlock<float> block(&data, 1, numSamples);
                juce::dsp::ProcessContextReplacing<float> context(block);
                crosstalkFilters[ch][band].process(context);
            }
            
            // Calculate crosstalk amount for this band
            float crossAmount = 0.0f;
            switch (band) {
                case 0: crossAmount = 0.12f; break; // More crosstalk in low frequencies
                case 1: crossAmount = 0.08f; break; // Medium crosstalk in mid frequencies
                case 2: crossAmount = 0.04f; break; // Less crosstalk in high frequencies
            }
            
            // Apply crosstalk to this band
            float* leftData = bandBuffer.getWritePointer(0);
            float* rightData = bandBuffer.getWritePointer(1);
            
            for (int i = 0; i < numSamples; ++i) {
                const float left = leftData[i];
                const float right = rightData[i];
                
                leftData[i] = left + right * crossAmount;
                rightData[i] = right + left * crossAmount;
            }
            
            // Mix processed band back to main buffer
            buffer.addFrom(0, 0, bandBuffer, 0, 0, numSamples);
            buffer.addFrom(1, 0, bandBuffer, 1, 0, numSamples);
        }
    }
    
    // Apply subtle stylus-like envelope asymmetry
    void applyAsymmetry(juce::AudioBuffer<float>& buffer) {
        if (asymmetryAmount <= 0.0f) {
            return;
        }
        
        const int numSamples = buffer.getNumSamples();
        const float amount = asymmetryAmount * 0.01f; // Scale down for subtle effect
        
        for (int ch = 0; ch < 2; ++ch) {
            float* data = buffer.getWritePointer(ch);
            
            for (int i = 0; i < numSamples; ++i) {
                // Generate a subtle asymmetric envelope
                phase += 0.0001f;
                if (phase > 1.0f) phase -= 1.0f;
                
                // Asymmetric shape that affects positive and negative differently
                float envelope = std::sin(phase * juce::MathConstants<float>::twoPi) * amount;
                
                // Apply subtly different processing to positive and negative parts
                if (data[i] > 0.0f) {
                    // Positive: slightly compress
                    data[i] = data[i] * (1.0f - envelope * 0.1f);
                } else {
                    // Negative: slightly expand
                    data[i] = data[i] * (1.0f + envelope * 0.1f);
                }
            }
        }
    }
    
    // Instance variables
    double sampleRate = 44100.0;
    float ellipticalFreq = 120.0f;
    int ellipticalSlope = 6;
    float crosstalkTilt = -30.0f;
    float asymmetryAmount = 2.0f;
    float wetMix = 0.2f;
    float phase = 0.0f;
    
    juce::AudioBuffer<float> tempBuffer;
    
    std::array<juce::dsp::IIR::Filter<float>, 2> ellipticalFilter;
    std::array<std::array<juce::dsp::IIR::Filter<float>, 3>, 2> crosstalkFilters; // [channel][band]
};

} // namespace mojoDSP
