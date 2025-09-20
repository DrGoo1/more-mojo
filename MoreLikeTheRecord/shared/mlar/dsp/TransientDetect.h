#pragma once
#include <JuceHeader.h>
#include <vector>
#include <array>

namespace moremojo {
namespace mlar {

/**
 * TransientDetect - Multi-band phase deviation transient detector
 * 
 * Features:
 * - STFT-based analysis for multi-band detection
 * - Phase deviation measurement for accurate transient identification
 * - Magnitude weighting for perceptually relevant processing
 */
class TransientDetect {
public:
    TransientDetect() = default;
    ~TransientDetect() = default;
    
    /**
     * Prepare for processing
     * 
     * @param sampleRate The current sample rate
     * @param blockSize The maximum expected block size
     */
    void prepare(double sampleRate, int blockSize) {
        this->sampleRate = sampleRate;
        this->blockSize = blockSize;
        
        // Initialize FFT and window
        fftSize = 1024;
        fftOverlap = fftSize / 2;
        
        // Initialize forward FFT
        forwardFFT = std::make_unique<juce::dsp::FFT>(std::log2(fftSize));
        
        // Initialize window function (Hann)
        window = std::make_unique<juce::dsp::WindowingFunction<float>>(
            fftSize, juce::dsp::WindowingFunction<float>::hann);
        
        // Initialize FFT buffers
        fftBuffer.resize(fftSize * 2, 0.0f);  // Complex data (real/imag pairs)
        prevPhase.resize(fftSize / 2, 0.0f);
        
        // Initialize band data
        numBands = 6;  // Default
        bandEnergies.resize(numBands, 0.0f);
        bandPhaseDeviation.resize(numBands, 0.0f);
        
        // Set default band edges (in Hz)
        bandEdges = {20.0f, 250.0f, 800.0f, 2500.0f, 6000.0f, 12000.0f, 20000.0f};
        
        reset();
    }
    
    /**
     * Reset internal state
     */
    void reset() {
        std::fill(fftBuffer.begin(), fftBuffer.end(), 0.0f);
        std::fill(prevPhase.begin(), prevPhase.end(), 0.0f);
        std::fill(bandEnergies.begin(), bandEnergies.end(), 0.0f);
        std::fill(bandPhaseDeviation.begin(), bandPhaseDeviation.end(), 0.0f);
    }
    
    /**
     * Process a buffer through the transient detector
     * 
     * @param buffer The audio buffer to process
     * @param apvts Parameter state containing the detector settings
     */
    void process(juce::AudioBuffer<float>& buffer, juce::AudioProcessorValueTreeState& apvts) {
        // Check if transient detection is enabled
        bool transientEnabled = apvts.getRawParameterValue("mlar_transient_on")->load() > 0.5f;
        
        if (!transientEnabled) {
            // Early return if disabled
            return;
        }
        
        // Get sensitivity parameter (0.0-1.0)
        float sensitivity = apvts.getRawParameterValue("mlar_transient_sense")->load() * 0.01f;
        sensitivity = juce::jlimit(0.0f, 1.0f, sensitivity);
        
        // Update number of bands if parameter exists
        if (auto* bandsParam = apvts.getRawParameterValue("mlar_transient_bands")) {
            int newBands = static_cast<int>(bandsParam->load());
            if (newBands != numBands && newBands >= 2 && newBands <= 8) {
                numBands = newBands;
                updateBandEdges();
                bandEnergies.resize(numBands, 0.0f);
                bandPhaseDeviation.resize(numBands, 0.0f);
            }
        }
        
        // Process mono mix for analysis (stereo processing could be added)
        juce::AudioBuffer<float> monoBuffer(1, buffer.getNumSamples());
        monoBuffer.clear();
        
        // Create mono mix
        for (int ch = 0; ch < buffer.getNumChannels(); ++ch) {
            monoBuffer.addFrom(0, 0, buffer, ch, 0, buffer.getNumSamples(), 1.0f / buffer.getNumChannels());
        }
        
        // Detect transients
        std::vector<TransientInfo> transients = detectTransients(monoBuffer, sensitivity);
        
        // Apply processing to transients if any were detected
        if (!transients.empty()) {
            processTransients(buffer, transients, sensitivity);
        }
    }
    
private:
    /**
     * Structure to hold information about detected transients
     */
    struct TransientInfo {
        int position;        // Sample position within buffer
        float confidence;    // Detection confidence (0.0-1.0)
        std::vector<float> bandStrengths; // Per-band transient strengths
    };
    
    /**
     * Update frequency band edges based on number of bands
     */
    void updateBandEdges() {
        bandEdges.resize(numBands + 1);
        
        // Set edges on logarithmic scale from 20 Hz to 20 kHz
        const float minFreq = 20.0f;
        const float maxFreq = 20000.0f;
        const float factor = std::pow(maxFreq / minFreq, 1.0f / numBands);
        
        for (int i = 0; i <= numBands; ++i) {
            bandEdges[i] = minFreq * std::pow(factor, static_cast<float>(i));
        }
    }
    
    /**
     * Detect transients in a buffer
     * 
     * @param buffer The audio buffer to analyze
     * @param sensitivity Sensitivity parameter (0.0-1.0)
     * @return Vector of detected transients
     */
    std::vector<TransientInfo> detectTransients(const juce::AudioBuffer<float>& buffer, float sensitivity) {
        std::vector<TransientInfo> result;
        const float* data = buffer.getReadPointer(0);
        const int numSamples = buffer.getNumSamples();
        
        // Process in FFT blocks
        for (int position = 0; position < numSamples; position += fftOverlap) {
            // Fill FFT buffer
            for (int i = 0; i < fftSize; ++i) {
                const int samplePos = position + i;
                const float sample = (samplePos < numSamples) ? data[samplePos] : 0.0f;
                fftBuffer[i * 2] = sample;
                fftBuffer[i * 2 + 1] = 0.0f;  // Imaginary part
            }
            
            // Apply window function
            window->multiplyWithWindowingTable(fftBuffer.data(), fftSize);
            
            // Perform FFT
            forwardFFT->performRealOnlyForwardTransform(fftBuffer.data());
            
            // Analyze phase deviation in each band
            analyzeBands();
            
            // Check for transients in each band
            bool transientDetected = false;
            TransientInfo info;
            info.position = position;
            info.confidence = 0.0f;
            info.bandStrengths.resize(numBands, 0.0f);
            
            float totalDeviation = 0.0f;
            float totalEnergy = 0.0f;
            
            for (int band = 0; band < numBands; ++band) {
                // Calculate band strength based on phase deviation and energy
                float strength = bandPhaseDeviation[band] * (0.2f + 0.8f * std::sqrt(bandEnergies[band]));
                
                // Apply sensitivity scaling
                strength *= (0.5f + sensitivity * 1.5f);
                
                info.bandStrengths[band] = strength;
                
                // Accumulate for confidence calculation
                totalDeviation += bandPhaseDeviation[band];
                totalEnergy += bandEnergies[band];
                
                // Check if this band exceeds threshold
                if (strength > 0.7f) {
                    transientDetected = true;
                }
            }
            
            // Calculate overall confidence
            if (transientDetected && totalEnergy > 0.01f) {
                info.confidence = juce::jlimit(0.0f, 1.0f, 
                                             totalDeviation / static_cast<float>(numBands) * 
                                             std::sqrt(totalEnergy) * (1.0f + sensitivity));
                
                if (info.confidence > 0.5f) {
                    result.push_back(info);
                }
            }
        }
        
        return result;
    }
    
    /**
     * Analyze phase deviation and energy in frequency bands
     */
    void analyzeBands() {
        std::fill(bandEnergies.begin(), bandEnergies.end(), 0.0f);
        std::fill(bandPhaseDeviation.begin(), bandPhaseDeviation.end(), 0.0f);
        
        // Count bins per band
        std::vector<int> binCounts(numBands, 0);
        
        // Process each frequency bin
        for (int bin = 1; bin < fftSize / 2; ++bin) {
            // Calculate bin frequency
            const float binFreq = bin * static_cast<float>(sampleRate) / fftSize;
            
            // Find the corresponding band
            int band = 0;
            while (band < numBands && binFreq > bandEdges[band + 1]) {
                ++band;
            }
            
            if (band < numBands) {
                const float real = fftBuffer[bin * 2];
                const float imag = fftBuffer[bin * 2 + 1];
                
                // Calculate magnitude and phase
                const float magnitude = std::sqrt(real * real + imag * imag);
                const float phase = std::atan2(imag, real);
                
                // Calculate phase deviation (unwrapped)
                float phaseDeviation = phase - prevPhase[bin];
                
                // Unwrap phase to [-π, π]
                if (phaseDeviation > juce::MathConstants<float>::pi) {
                    phaseDeviation -= 2.0f * juce::MathConstants<float>::pi;
                } else if (phaseDeviation < -juce::MathConstants<float>::pi) {
                    phaseDeviation += 2.0f * juce::MathConstants<float>::pi;
                }
                
                // Take absolute value of phase deviation
                phaseDeviation = std::abs(phaseDeviation);
                
                // Accumulate weighted phase deviation and energy for this band
                bandPhaseDeviation[band] += magnitude * phaseDeviation;
                bandEnergies[band] += magnitude * magnitude;
                binCounts[band]++;
                
                // Store current phase for next frame
                prevPhase[bin] = phase;
            }
        }
        
        // Normalize by bin count
        for (int band = 0; band < numBands; ++band) {
            if (binCounts[band] > 0) {
                bandPhaseDeviation[band] /= static_cast<float>(binCounts[band]);
                bandEnergies[band] /= static_cast<float>(binCounts[band]);
            }
        }
    }
    
    /**
     * Process detected transients in the audio buffer
     * 
     * @param buffer The audio buffer to process
     * @param transients Vector of detected transients
     * @param sensitivity Sensitivity parameter (0.0-1.0)
     */
    void processTransients(juce::AudioBuffer<float>& buffer, 
                          const std::vector<TransientInfo>& transients,
                          float sensitivity) {
        // In this implementation, we don't actually modify the audio
        // This would be where transient processing is applied
        
        // Example processing (commented out):
        /*
        for (const auto& transient : transients) {
            // Process this transient in all channels
            for (int ch = 0; ch < buffer.getNumChannels(); ++ch) {
                // Get write pointer for this channel
                float* data = buffer.getWritePointer(ch);
                
                // Apply processing around transient position
                int start = juce::jmax(0, transient.position - 5);
                int end = juce::jmin(buffer.getNumSamples() - 1, transient.position + 20);
                
                for (int i = start; i <= end; ++i) {
                    // Apply some processing
                    // Example: Enhance transient by slight gain
                    float factor = 1.0f + 0.1f * sensitivity * transient.confidence;
                    data[i] *= factor;
                }
            }
        }
        */
    }
    
    // Instance variables
    double sampleRate = 44100.0;
    int blockSize = 512;
    int numBands = 6;
    int fftSize = 1024;
    int fftOverlap = 512;
    
    std::vector<float> bandEdges;
    std::vector<float> bandEnergies;
    std::vector<float> bandPhaseDeviation;
    
    std::unique_ptr<juce::dsp::FFT> forwardFFT;
    std::unique_ptr<juce::dsp::WindowingFunction<float>> window;
    std::vector<float> fftBuffer;
    std::vector<float> prevPhase;
};

}} // namespace moremojo::mlar
