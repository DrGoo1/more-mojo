#pragma once
#include <JuceHeader.h>
#include <array>
#include <vector>

namespace mojoDSP {

/**
 * LinkwitzRiley - Multi-band crossover filter
 * 
 * Implements a Linkwitz-Riley 24 dB/oct (LR4) filter bank
 * for splitting audio into frequency bands with perfect reconstruction
 */
class LinkwitzRiley {
public:
    LinkwitzRiley() = default;
    
    /**
     * Prepare the crossover for processing
     * 
     * @param sampleRate The audio sample rate
     * @param maxBlockSize Maximum expected block size
     * @param numBands Number of bands (3-8)
     */
    void prepare(double sampleRate, int maxBlockSize, int numBands) {
        this->sampleRate = sampleRate;
        this->maxBlockSize = maxBlockSize;
        this->numBands = juce::jlimit(3, 8, numBands);
        
        // Initialize filter state arrays
        for (int ch = 0; ch < 2; ++ch) { // Support stereo
            // Low-pass filters for each crossover frequency
            lowpassFilters[ch].clear();
            
            // High-pass filters for each crossover frequency
            highpassFilters[ch].clear();
        }
        
        // Initialize band buffers
        bandBuffers.clear();
        for (int band = 0; band < numBands; ++band) {
            bandBuffers.push_back(juce::AudioBuffer<float>(2, maxBlockSize));
        }
        
        // Set default crossover frequencies (equally distributed on log scale)
        setCrossoverFrequencies(calculateDefaultFrequencies(numBands));
    }
    
    /**
     * Reset filter states
     */
    void reset() {
        for (int ch = 0; ch < 2; ++ch) {
            for (auto& filter : lowpassFilters[ch]) {
                filter.reset();
            }
            
            for (auto& filter : highpassFilters[ch]) {
                filter.reset();
            }
        }
    }
    
    /**
     * Set crossover frequencies
     * 
     * @param frequencies Vector of crossover frequencies (must be numBands-1 values)
     */
    void setCrossoverFrequencies(const std::vector<float>& frequencies) {
        // Check if we have the correct number of frequencies
        if (static_cast<int>(frequencies.size()) != numBands - 1) {
            return;
        }
        
        crossoverFreqs = frequencies;
        
        // Create filter arrays
        for (int ch = 0; ch < 2; ++ch) {
            lowpassFilters[ch].clear();
            highpassFilters[ch].clear();
            
            for (float freq : crossoverFreqs) {
                // Create Linkwitz-Riley 24 dB/oct low-pass filter
                juce::dsp::IIR::Filter<float> lowpassFilter;
                *lowpassFilter.state = *juce::dsp::IIR::Coefficients<float>::makeLowPass(
                    sampleRate, freq, 0.7071f); // Cascaded twice for LR4
                
                // Create Linkwitz-Riley 24 dB/oct high-pass filter
                juce::dsp::IIR::Filter<float> highpassFilter;
                *highpassFilter.state = *juce::dsp::IIR::Coefficients<float>::makeHighPass(
                    sampleRate, freq, 0.7071f); // Cascaded twice for LR4
                
                lowpassFilters[ch].push_back(lowpassFilter);
                highpassFilters[ch].push_back(highpassFilter);
            }
        }
    }
    
    /**
     * Split input buffer into frequency bands
     * 
     * @param input The input audio buffer
     */
    void split(const juce::AudioBuffer<float>& input) {
        const int numChannels = juce::jmin(input.getNumChannels(), 2);
        const int numSamples = input.getNumSamples();
        
        // Ensure band buffers are cleared
        for (auto& buffer : bandBuffers) {
            buffer.clear();
        }
        
        // Process each channel
        for (int ch = 0; ch < numChannels; ++ch) {
            const float* inputData = input.getReadPointer(ch);
            
            // Copy input to temporary buffer
            juce::AudioBuffer<float> tempBuffer(1, numSamples);
            tempBuffer.copyFrom(0, 0, inputData, numSamples);
            
            // Process first band (low-pass only)
            juce::AudioBuffer<float>& firstBandBuffer = bandBuffers[0];
            float* firstBandData = firstBandBuffer.getWritePointer(ch);
            
            // Apply first low-pass filter twice for LR4
            {
                float* tempData = tempBuffer.getWritePointer(0);
                
                // Apply first-order low-pass filter twice for LR4
                juce::dsp::AudioBlock<float> block(&tempData, 1, numSamples);
                juce::dsp::ProcessContextReplacing<float> context(block);
                
                lowpassFilters[ch][0].process(context);
                lowpassFilters[ch][0].process(context);
                
                // Copy to first band
                juce::FloatVectorOperations::copy(firstBandData, tempData, numSamples);
            }
            
            // Process middle bands (band-pass)
            for (int band = 1; band < numBands - 1; ++band) {
                juce::AudioBuffer<float>& bandBuffer = bandBuffers[band];
                float* bandData = bandBuffer.getWritePointer(ch);
                
                // Copy input to temp buffer
                tempBuffer.copyFrom(0, 0, inputData, numSamples);
                float* tempData = tempBuffer.getWritePointer(0);
                
                // Apply high-pass for lower crossover frequency
                {
                    juce::dsp::AudioBlock<float> block(&tempData, 1, numSamples);
                    juce::dsp::ProcessContextReplacing<float> context(block);
                    
                    highpassFilters[ch][band - 1].process(context);
                    highpassFilters[ch][band - 1].process(context);
                }
                
                // Apply low-pass for upper crossover frequency
                {
                    juce::dsp::AudioBlock<float> block(&tempData, 1, numSamples);
                    juce::dsp::ProcessContextReplacing<float> context(block);
                    
                    lowpassFilters[ch][band].process(context);
                    lowpassFilters[ch][band].process(context);
                }
                
                // Copy to band buffer
                juce::FloatVectorOperations::copy(bandData, tempData, numSamples);
            }
            
            // Process last band (high-pass only)
            if (numBands > 1) {
                juce::AudioBuffer<float>& lastBandBuffer = bandBuffers[numBands - 1];
                float* lastBandData = lastBandBuffer.getWritePointer(ch);
                
                // Copy input to temp buffer
                tempBuffer.copyFrom(0, 0, inputData, numSamples);
                float* tempData = tempBuffer.getWritePointer(0);
                
                // Apply last high-pass filter twice for LR4
                juce::dsp::AudioBlock<float> block(&tempData, 1, numSamples);
                juce::dsp::ProcessContextReplacing<float> context(block);
                
                highpassFilters[ch][numBands - 2].process(context);
                highpassFilters[ch][numBands - 2].process(context);
                
                // Copy to last band
                juce::FloatVectorOperations::copy(lastBandData, tempData, numSamples);
            }
        }
    }
    
    /**
     * Sum processed band buffers back into output buffer
     * 
     * @param output The output buffer to fill with summed bands
     */
    void sum(juce::AudioBuffer<float>& output) {
        const int numChannels = juce::jmin(output.getNumChannels(), 2);
        const int numSamples = output.getNumSamples();
        
        // Clear output buffer
        output.clear();
        
        // Sum all bands into output
        for (int band = 0; band < numBands; ++band) {
            const juce::AudioBuffer<float>& bandBuffer = bandBuffers[band];
            
            for (int ch = 0; ch < numChannels; ++ch) {
                output.addFrom(ch, 0, bandBuffer, ch, 0, numSamples);
            }
        }
    }
    
    /**
     * Access band buffer for processing
     * 
     * @param band Band index (0 to numBands-1)
     * @return Reference to the band buffer
     */
    juce::AudioBuffer<float>& getBandBuffer(int band) {
        return bandBuffers[juce::jlimit(0, numBands - 1, band)];
    }
    
    /**
     * Get the number of bands
     */
    int getNumBands() const {
        return numBands;
    }
    
    /**
     * Get crossover frequencies
     */
    const std::vector<float>& getCrossoverFrequencies() const {
        return crossoverFreqs;
    }
    
private:
    // Calculate default crossover frequencies
    std::vector<float> calculateDefaultFrequencies(int bands) {
        std::vector<float> freqs;
        
        if (bands <= 1) {
            return freqs;
        }
        
        // Log-scale distribution from 100 Hz to 10 kHz
        const float minFreq = 100.0f;
        const float maxFreq = 10000.0f;
        const float factor = std::pow(maxFreq / minFreq, 1.0f / (bands - 1));
        
        for (int i = 0; i < bands - 1; ++i) {
            freqs.push_back(minFreq * std::pow(factor, static_cast<float>(i)));
        }
        
        return freqs;
    }
    
    // Instance variables
    double sampleRate = 44100.0;
    int maxBlockSize = 512;
    int numBands = 6; // Default to 6 bands
    
    std::vector<float> crossoverFreqs; // Crossover frequencies
    
    // Arrays of filters for each channel
    std::array<std::vector<juce::dsp::IIR::Filter<float>>, 2> lowpassFilters;
    std::array<std::vector<juce::dsp::IIR::Filter<float>>, 2> highpassFilters;
    
    // Band buffers
    std::vector<juce::AudioBuffer<float>> bandBuffers;
};

} // namespace mojoDSP
