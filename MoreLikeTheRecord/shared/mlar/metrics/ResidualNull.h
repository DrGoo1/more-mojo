#pragma once
#include <JuceHeader.h>

namespace moremojo {
namespace mlar {

/**
 * ResidualNull - Utility for creating and analyzing residual signals
 * 
 * Features:
 * - Creates residual signal by subtracting original from processed audio
 * - Calculates spectrum and energy metrics of the residual
 * - Provides analysis tools for comparing original and processed audio
 */
class ResidualNull {
public:
    ResidualNull() = default;
    ~ResidualNull() = default;
    
    /**
     * Make a residual signal by subtracting original from processed audio
     * 
     * @param original Original (input) audio buffer
     * @param processed Processed (output) audio buffer
     * @param residual Output buffer to store the residual signal
     */
    static void makeResidual(const juce::AudioBuffer<float>& original, 
                            const juce::AudioBuffer<float>& processed, 
                            juce::AudioBuffer<float>& residual) {
        // Make sure residual is properly sized
        const int numChannels = juce::jmin(original.getNumChannels(), 
                                         processed.getNumChannels());
        const int numSamples = juce::jmin(original.getNumSamples(), 
                                        processed.getNumSamples());
        
        residual.setSize(numChannels, numSamples, false, true, false);
        
        // For each channel, calculate processed - original
        for (int ch = 0; ch < numChannels; ++ch) {
            const float* originalData = original.getReadPointer(ch);
            const float* processedData = processed.getReadPointer(ch);
            float* residualData = residual.getWritePointer(ch);
            
            for (int i = 0; i < numSamples; ++i) {
                residualData[i] = processedData[i] - originalData[i];
            }
        }
    }
    
    /**
     * Calculate RMS level of a buffer
     * 
     * @param buffer Audio buffer to analyze
     * @return RMS level in linear gain
     */
    static float calculateRMS(const juce::AudioBuffer<float>& buffer) {
        float sum = 0.0f;
        int totalSamples = 0;
        
        for (int ch = 0; ch < buffer.getNumChannels(); ++ch) {
            const float* data = buffer.getReadPointer(ch);
            
            for (int i = 0; i < buffer.getNumSamples(); ++i) {
                sum += data[i] * data[i];
            }
            
            totalSamples += buffer.getNumSamples();
        }
        
        if (totalSamples > 0) {
            return std::sqrt(sum / static_cast<float>(totalSamples));
        }
        
        return 0.0f;
    }
    
    /**
     * Calculate crest factor (peak/RMS) of a buffer
     * 
     * @param buffer Audio buffer to analyze
     * @return Crest factor (dimensionless)
     */
    static float calculateCrestFactor(const juce::AudioBuffer<float>& buffer) {
        float peak = 0.0f;
        
        for (int ch = 0; ch < buffer.getNumChannels(); ++ch) {
            const float* data = buffer.getReadPointer(ch);
            
            for (int i = 0; i < buffer.getNumSamples(); ++i) {
                peak = juce::jmax(peak, std::abs(data[i]));
            }
        }
        
        float rms = calculateRMS(buffer);
        
        if (rms > 1.0e-8f) {
            return peak / rms;
        }
        
        return 1.0f;  // Default for very quiet signals
    }
    
    /**
     * Calculate spectral tilt of a buffer
     * 
     * @param buffer Audio buffer to analyze
     * @param sampleRate Sample rate in Hz
     * @return Spectral tilt in dB/octave
     */
    static float calculateSpectralTilt(const juce::AudioBuffer<float>& buffer, double sampleRate) {
        // FFT size
        const int fftSize = 2048;
        
        // Calculate FFT for first channel only for simplicity
        if (buffer.getNumChannels() < 1 || buffer.getNumSamples() < fftSize) {
            return 0.0f;  // Default for short buffers
        }
        
        // Create FFT object
        juce::dsp::FFT fft(std::log2(fftSize));
        
        // Create buffer for FFT (real/imaginary pairs)
        std::vector<float> fftData(fftSize * 2, 0.0f);
        
        // Copy audio data to FFT buffer (real part)
        const float* audioData = buffer.getReadPointer(0);
        for (int i = 0; i < fftSize; ++i) {
            fftData[i * 2] = audioData[i];
            fftData[i * 2 + 1] = 0.0f;  // Imaginary part
        }
        
        // Apply window function
        juce::dsp::WindowingFunction<float> window(fftSize, 
                                                 juce::dsp::WindowingFunction<float>::hann);
        window.multiplyWithWindowingTable(fftData.data(), fftSize);
        
        // Perform FFT
        fft.performRealOnlyForwardTransform(fftData.data());
        
        // Calculate magnitude spectrum
        std::vector<float> magnitudes(fftSize / 2);
        for (int i = 0; i < fftSize / 2; ++i) {
            float real = fftData[i * 2];
            float imag = fftData[i * 2 + 1];
            magnitudes[i] = std::sqrt(real * real + imag * imag);
        }
        
        // Calculate spectral tilt using linear regression
        // Bin frequencies
        std::vector<float> freqs(fftSize / 2);
        std::vector<float> logMags(fftSize / 2);
        std::vector<float> logFreqs(fftSize / 2);
        
        // Only use bins from 50 Hz to Nyquist/2
        int startBin = static_cast<int>(50.0f * fftSize / sampleRate);
        int endBin = fftSize / 4;  // Nyquist/2
        
        int validBins = 0;
        
        for (int i = startBin; i < endBin; ++i) {
            freqs[validBins] = static_cast<float>(i) * static_cast<float>(sampleRate) / fftSize;
            
            // Log-frequency (in octaves) and log-magnitude
            logFreqs[validBins] = std::log2(freqs[validBins]);
            
            if (magnitudes[i] > 1.0e-8f) {
                logMags[validBins] = 20.0f * std::log10(magnitudes[i]);
            } else {
                logMags[validBins] = -120.0f;  // -120 dB floor
            }
            
            ++validBins;
        }
        
        if (validBins < 10) {
            return 0.0f;  // Not enough valid bins
        }
        
        // Linear regression: y = ax + b
        // We want to find 'a' which is the slope (tilt in dB/octave)
        float sumX = 0.0f, sumY = 0.0f, sumXY = 0.0f, sumX2 = 0.0f;
        
        for (int i = 0; i < validBins; ++i) {
            sumX += logFreqs[i];
            sumY += logMags[i];
            sumXY += logFreqs[i] * logMags[i];
            sumX2 += logFreqs[i] * logFreqs[i];
        }
        
        float denominator = validBins * sumX2 - sumX * sumX;
        
        if (std::abs(denominator) < 1.0e-8f) {
            return 0.0f;  // Avoid division by near-zero
        }
        
        // Calculate slope (dB/octave)
        float slope = (validBins * sumXY - sumX * sumY) / denominator;
        
        return slope;
    }
};

}} // namespace moremojo::mlar
