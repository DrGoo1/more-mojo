#pragma once
#include <JuceHeader.h>
#include <vector>
#include <complex>

namespace moremojo {
namespace mlar {

/**
 * GDSComputer - Group Delay Spread calculation and heatmap generation
 * 
 * Features:
 * - FFT-based phase derivative analysis across frequencies
 * - Time-frequency heatmap generation
 * - Frequency-weighted group delay calculation
 */
class GDSComputer {
public:
    GDSComputer() = default;
    ~GDSComputer() = default;
    
    /**
     * Prepare for processing
     * 
     * @param sampleRate The current sample rate
     */
    void prepare(double sampleRate) {
        this->sampleRate = sampleRate;
        
        // Set up FFT parameters
        fftSize = 1024;
        fftOverlap = fftSize / 2;
        
        // Initialize FFT
        forwardFFT = std::make_unique<juce::dsp::FFT>(std::log2(fftSize));
        
        // Initialize window function (Hann)
        window = std::make_unique<juce::dsp::WindowingFunction<float>>(
            fftSize, juce::dsp::WindowingFunction<float>::hann);
        
        // Initialize buffers
        fftBuffer.resize(fftSize * 2, 0.0f);  // Complex data (real/imag pairs)
        prevPhase.resize(fftSize / 2, 0.0f);
        groupDelay.resize(fftSize / 2, 0.0f);
        
        // Initialize heatmap grid
        heatmapWidth = 256;  // Frequency bins (x)
        heatmapHeight = 128;  // Group delay values (y)
        heatmapData.resize(heatmapWidth * heatmapHeight, 0.0f);
        
        reset();
    }
    
    /**
     * Reset internal state
     */
    void reset() {
        std::fill(fftBuffer.begin(), fftBuffer.end(), 0.0f);
        std::fill(prevPhase.begin(), prevPhase.end(), 0.0f);
        std::fill(groupDelay.begin(), groupDelay.end(), 0.0f);
        std::fill(heatmapData.begin(), heatmapData.end(), 0.0f);
        
        // Reset metrics
        gdsScore = 0.5f;
    }
    
    /**
     * Compute GDS heatmap from an audio buffer
     * 
     * @param buffer The audio buffer to analyze
     * @param width Output parameter for heatmap width
     * @param height Output parameter for heatmap height
     * @return Vector containing the flattened heatmap data
     */
    std::vector<float> compute(const juce::AudioBuffer<float>& buffer, int& width, int& height) {
        // Set output dimensions
        width = heatmapWidth;
        height = heatmapHeight;
        
        // Use only first channel for GDS analysis
        if (buffer.getNumChannels() == 0 || buffer.getNumSamples() == 0) {
            // Return empty grid with dimensions
            return heatmapData;
        }
        
        // Get channel data
        const float* channelData = buffer.getReadPointer(0);
        const int numSamples = buffer.getNumSamples();
        
        // Copy data to FFT buffer
        for (int i = 0; i < std::min(numSamples, fftSize); ++i) {
            fftBuffer[i * 2] = channelData[i];        // Real part
            fftBuffer[i * 2 + 1] = 0.0f;              // Imaginary part
        }
        
        // Zero-pad if needed
        for (int i = numSamples; i < fftSize; ++i) {
            fftBuffer[i * 2] = 0.0f;
            fftBuffer[i * 2 + 1] = 0.0f;
        }
        
        // Apply window function
        window->multiplyWithWindowingTable(fftBuffer.data(), fftSize);
        
        // Perform FFT
        forwardFFT->performRealOnlyForwardTransform(fftBuffer.data());
        
        // Calculate phase and phase derivative (group delay)
        for (int bin = 0; bin < fftSize / 2; ++bin) {
            const float real = fftBuffer[bin * 2];
            const float imag = fftBuffer[bin * 2 + 1];
            
            // Calculate magnitude and phase
            const float magnitude = std::sqrt(real * real + imag * imag);
            const float phase = std::atan2(imag, real);
            
            // Skip bins with very low magnitude
            if (magnitude < 0.001f) {
                groupDelay[bin] = 0.0f;
                continue;
            }
            
            // Calculate unwrapped phase difference (derivative)
            float phaseDiff = phase - prevPhase[bin];
            
            // Unwrap phase to [-π, π]
            if (phaseDiff > juce::MathConstants<float>::pi)
                phaseDiff -= 2.0f * juce::MathConstants<float>::pi;
            else if (phaseDiff < -juce::MathConstants<float>::pi)
                phaseDiff += 2.0f * juce::MathConstants<float>::pi;
            
            // Convert to group delay (negative phase derivative)
            // Units: seconds (phase difference / frequency difference)
            const float groupDelaySec = -phaseDiff / (2.0f * juce::MathConstants<float>::pi) * fftSize / sampleRate;
            
            // Store group delay and phase
            groupDelay[bin] = groupDelaySec;
            prevPhase[bin] = phase;
            
            // Update GDS heatmap
            updateHeatmap(bin, groupDelaySec, magnitude);
        }
        
        // Calculate GDS score from heatmap
        calculateGDSScore();
        
        // Return the heatmap grid with dimensions
        return heatmapData;
    }
    
    /**
     * Get current GDS score (0.0-1.0)
     * Higher scores indicate better timing coherence
     * 
     * @return GDS score
     */
    float getScore() const {
        return gdsScore;
    }
    
private:
    /**
     * Update the heatmap with new group delay data
     * 
     * @param freqBin Frequency bin
     * @param groupDelaySec Group delay in seconds
     * @param magnitude Magnitude (for weighting)
     */
    void updateHeatmap(int freqBin, float groupDelaySec, float magnitude) {
        // Convert frequency bin to heatmap X coordinate (log-scale)
        const float binFreq = static_cast<float>(freqBin) * sampleRate / fftSize;
        const float minFreq = 20.0f;
        const float maxFreq = sampleRate / 2.0f;
        
        // Skip frequencies outside range of interest
        if (binFreq < minFreq || binFreq > maxFreq) {
            return;
        }
        
        // Log-scale mapping to X coordinate
        const float normX = std::log10(binFreq / minFreq) / std::log10(maxFreq / minFreq);
        const int x = juce::jlimit(0, heatmapWidth - 1, static_cast<int>(normX * heatmapWidth));
        
        // Convert group delay to heatmap Y coordinate (linear scale)
        const float maxDelay = 0.005f;  // 5 ms
        const float normDelay = juce::jlimit(0.0f, 1.0f, std::abs(groupDelaySec) / maxDelay);
        
        // Invert Y coordinate (0 at top, maxDelay at bottom)
        const int y = juce::jlimit(0, heatmapHeight - 1, static_cast<int>((1.0f - normDelay) * heatmapHeight));
        
        // Index in the flattened 2D array
        const int index = y * heatmapWidth + x;
        
        // Update heatmap with exponential moving average, weighted by magnitude
        const float alpha = 0.2f;  // Smoothing factor
        const float weight = juce::jlimit(0.0f, 1.0f, magnitude * 10.0f);
        
        if (index >= 0 && index < static_cast<int>(heatmapData.size())) {
            heatmapData[index] = heatmapData[index] * (1.0f - alpha) + weight * alpha;
        }
    }
    
    /**
     * Calculate GDS score from heatmap data
     */
    void calculateGDSScore() {
        float totalSpread = 0.0f;
        int countBins = 0;
        
        // For each frequency column
        for (int x = 0; x < heatmapWidth; ++x) {
            // Find min and max group delay bins with non-zero energy
            int minY = heatmapHeight;
            int maxY = 0;
            bool hasEnergy = false;
            
            for (int y = 0; y < heatmapHeight; ++y) {
                const int index = y * heatmapWidth + x;
                if (heatmapData[index] > 0.05f) {
                    minY = juce::jmin(minY, y);
                    maxY = juce::jmax(maxY, y);
                    hasEnergy = true;
                }
            }
            
            // Calculate spread for this frequency
            if (hasEnergy && maxY > minY) {
                const float spread = static_cast<float>(maxY - minY) / static_cast<float>(heatmapHeight);
                totalSpread += spread;
                ++countBins;
            }
        }
        
        // Calculate average spread and convert to score (lower spread = higher score)
        if (countBins > 0) {
            const float avgSpread = totalSpread / static_cast<float>(countBins);
            gdsScore = 1.0f - juce::jlimit(0.0f, 1.0f, avgSpread * 2.0f);
        } else {
            gdsScore = 0.5f;  // Default neutral score
        }
    }
    
    // Instance variables
    double sampleRate = 44100.0;
    int fftSize = 1024;
    int fftOverlap = 512;
    
    std::unique_ptr<juce::dsp::FFT> forwardFFT;
    std::unique_ptr<juce::dsp::WindowingFunction<float>> window;
    
    std::vector<float> fftBuffer;
    std::vector<float> prevPhase;
    std::vector<float> groupDelay;
    
    int heatmapWidth = 256;
    int heatmapHeight = 128;
    std::vector<float> heatmapData;
    
    float gdsScore = 0.5f;
};

}} // namespace moremojo::mlar
