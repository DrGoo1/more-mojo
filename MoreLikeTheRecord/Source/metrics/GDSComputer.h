#pragma once
#include <JuceHeader.h>

/**
 * GDSComputer - Group Delay Spread computation from STFT phase derivatives
 * 
 * Uses Short-Time Fourier Transform to analyze phase changes across frequency
 * and computes the group delay spread as a measure of timing coherence
 */
class GDSComputer {
public:
    GDSComputer() = default;
    
    /**
     * Prepare the GDS computer with sample rate
     * 
     * @param sampleRate The audio sample rate
     */
    void prepare(double sampleRate) {
        this->sampleRate = sampleRate;
        
        // Set up FFT
        fftSize = 1024;
        fftOverlap = fftSize / 2;
        
        // Initialize forward FFT
        forwardFFT = std::make_unique<juce::dsp::FFT>(std::log2(fftSize));
        
        // Initialize window function
        window = std::make_unique<juce::dsp::WindowingFunction<float>>(
            fftSize, juce::dsp::WindowingFunction<float>::hann);
        
        // Initialize buffers
        fftBuffer.resize(fftSize * 2, 0.0f); // Complex data (real/imag)
        lastPhase.resize(fftSize / 2, 0.0f);
        groupDelay.resize(fftSize / 2, 0.0f);
        
        // Initialize heatmap grid
        gdsWidth = 256;   // Frequency bins (x)
        gdsHeight = 128;  // Group delay values (y)
        gdsData.resize(gdsWidth * gdsHeight, 0.0f);
        
        reset();
    }
    
    /**
     * Reset the GDS computer state
     */
    void reset() {
        std::fill(fftBuffer.begin(), fftBuffer.end(), 0.0f);
        std::fill(lastPhase.begin(), lastPhase.end(), 0.0f);
        std::fill(groupDelay.begin(), groupDelay.end(), 0.0f);
        std::fill(gdsData.begin(), gdsData.end(), 0.0f);
        
        // Reset metrics
        gdsScore = 0.5f;
    }
    
    /**
     * Compute GDS heatmap from an audio buffer
     * 
     * @param buffer The audio buffer to analyze
     * @param width Output parameter for heatmap width
     * @param height Output parameter for heatmap height
     * @return 2D grid (as flattened vector) representing the GDS heatmap
     */
    std::vector<float> compute(const juce::AudioBuffer<float>& buffer, int& width, int& height) {
        // Use only first channel for GDS analysis
        if (buffer.getNumChannels() == 0 || buffer.getNumSamples() == 0) {
            // Return empty grid with dimensions
            width = gdsWidth;
            height = gdsHeight;
            return gdsData;
        }
        
        const float* channelData = buffer.getReadPointer(0);
        const int numSamples = buffer.getNumSamples();
        
        // Copy data to FFT buffer
        for (int i = 0; i < std::min(numSamples, fftSize); ++i) {
            fftBuffer[i * 2] = channelData[i];
            fftBuffer[i * 2 + 1] = 0.0f; // Imaginary part
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
            float phaseDiff = phase - lastPhase[bin];
            
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
            lastPhase[bin] = phase;
            
            // Update GDS heatmap
            updateGDSHeatmap(bin, groupDelaySec, magnitude);
        }
        
        // Calculate GDS score from heatmap
        calculateGDSScore();
        
        // Return the heatmap grid with dimensions
        width = gdsWidth;
        height = gdsHeight;
        return gdsData;
    }
    
    /**
     * Get the current GDS score (0-1)
     * Higher scores indicate better timing coherence
     */
    float getScore() const {
        return gdsScore;
    }
    
private:
    // Update the GDS heatmap with new group delay data
    void updateGDSHeatmap(int freqBin, float groupDelaySec, float magnitude) {
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
        const int x = juce::jlimit(0, gdsWidth - 1, static_cast<int>(normX * gdsWidth));
        
        // Convert group delay to heatmap Y coordinate (linear scale)
        const float maxDelay = 0.005f; // 5 ms
        const float normDelay = juce::jlimit(0.0f, 1.0f, groupDelaySec / maxDelay);
        
        // Invert Y coordinate (0 at top, maxDelay at bottom)
        const int y = juce::jlimit(0, gdsHeight - 1, static_cast<int>((1.0f - normDelay) * gdsHeight));
        
        // Index in the flattened 2D array
        const int index = y * gdsWidth + x;
        
        // Update heatmap with exponential moving average, weighted by magnitude
        const float alpha = 0.2f; // Smoothing factor
        const float weight = juce::jlimit(0.0f, 1.0f, magnitude * 10.0f);
        
        if (index >= 0 && index < static_cast<int>(gdsData.size())) {
            gdsData[index] = gdsData[index] * (1.0f - alpha) + weight * alpha;
        }
    }
    
    // Calculate GDS score from heatmap data
    void calculateGDSScore() {
        float totalSpread = 0.0f;
        int countBins = 0;
        
        // For each frequency column
        for (int x = 0; x < gdsWidth; ++x) {
            // Find min and max group delay bins with non-zero energy
            int minY = gdsHeight;
            int maxY = 0;
            bool hasEnergy = false;
            
            for (int y = 0; y < gdsHeight; ++y) {
                const int index = y * gdsWidth + x;
                if (gdsData[index] > 0.05f) {
                    minY = juce::jmin(minY, y);
                    maxY = juce::jmax(maxY, y);
                    hasEnergy = true;
                }
            }
            
            // Calculate spread for this frequency
            if (hasEnergy && maxY > minY) {
                const float spread = static_cast<float>(maxY - minY) / static_cast<float>(gdsHeight);
                totalSpread += spread;
                ++countBins;
            }
        }
        
        // Calculate average spread and convert to score (lower spread = higher score)
        if (countBins > 0) {
            const float avgSpread = totalSpread / static_cast<float>(countBins);
            gdsScore = 1.0f - juce::jlimit(0.0f, 1.0f, avgSpread * 2.0f);
        } else {
            gdsScore = 0.5f; // Default neutral score
        }
    }
    
    // Instance variables
    double sampleRate = 44100.0;
    int fftSize = 1024;
    int fftOverlap = 512;
    
    std::unique_ptr<juce::dsp::FFT> forwardFFT;
    std::unique_ptr<juce::dsp::WindowingFunction<float>> window;
    
    std::vector<float> fftBuffer;
    std::vector<float> lastPhase;
    std::vector<float> groupDelay;
    
    int gdsWidth = 256;
    int gdsHeight = 128;
    std::vector<float> gdsData;
    
    float gdsScore = 0.5f;
};
