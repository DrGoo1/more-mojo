#pragma once
#include <JuceHeader.h>
#include "../dsp/TruePeak.h"

namespace mojo {

/**
 * MetricsFrame - Container for metrics data at a point in time
 */
struct MetricsFrame {
    // Group Delay Spread (GDS) data
    std::vector<float> gdsData;  // 2D grid as 1D vector (width*height)
    int gdsWidth = 0;
    int gdsHeight = 0;
    float gdsScore = 0.0f;      // Overall GDS score (0-1)
    
    // Interaural Coherence & Time (IACC/ITD) data
    float iacc = 0.0f;          // Interaural coherence (0-1)
    float itdMs = 0.0f;         // Interaural time difference (ms)
    float itdVariance = 0.0f;   // ITD variance (ms)
    
    // Transient Edge Integrity (TEI) data
    float teiRiseTime = 0.0f;   // Rise time delta (µs)
    float teiPreRing = 0.0f;    // Pre-ring energy (dB)
    float teiOvershoot = 0.0f;  // Overshoot percentage (%)
    float teiScore = 0.0f;      // Overall TEI score (0-1)
    
    // Residual/Null data
    std::vector<float> residualSpectrum;  // FFT spectrum of residual
    float residualRMS = -120.0f;          // RMS level (dB)
    float residualTilt = 0.0f;            // Spectral tilt (dB/oct)
    float residualCrest = 0.0f;           // Crest factor (dB)
    
    // Level matching data
    float lufsA = -23.0f;       // LUFS level for A
    float lufsB = -23.0f;       // LUFS level for B
    float truePeakA = -1.0f;    // True peak for A (dB)
    float truePeakB = -1.0f;    // True peak for B (dB)
    
    // Composite score (SCPI)
    float scpiScore = 0.5f;     // Stereo Spatial Cue Preservation Index (0-1)
};

/**
 * MetricsAnalyzer - Analyzes audio for spatial and timing metrics
 */
class MetricsAnalyzer {
public:
    MetricsAnalyzer() = default;
    
    void prepare(double sampleRate, int maxBlockSize) {
        this->sampleRate = sampleRate;
        this->maxBlockSize = maxBlockSize;
        
        // Initialize FFT
        fftSize = 2048;
        fftOverlap = fftSize / 2;
        
        // Find next power of 2 for FFT
        while (fftSize < maxBlockSize * 2) {
            fftSize *= 2;
        }
        
        // Set up FFT objects
        forwardFFT = std::make_unique<juce::dsp::FFT>(std::log2(fftSize));
        window = std::make_unique<juce::dsp::WindowingFunction<float>>(
            fftSize, juce::dsp::WindowingFunction<float>::hann);
        
        // Initialize buffers
        fftBuffer.resize(fftSize * 2); // Complex data (real/imag pairs)
        lastPhase.resize(fftSize / 2);
        phaseDeviation.resize(fftSize / 2);
        
        // Initialize group delay heatmap
        gdsWidth = 256;   // Frequency bins
        gdsHeight = 128;  // Time bins
        gdsData.resize(gdsWidth * gdsHeight, 0.0f);
        
        // Initialize residual spectrum
        residualSpectrum.resize(fftSize / 2, 0.0f);
        
        // Initialize true peak detector
        truePeak.prepare(sampleRate);
        
        reset();
    }
    
    void reset() {
        // Clear buffers
        std::fill(fftBuffer.begin(), fftBuffer.end(), 0.0f);
        std::fill(lastPhase.begin(), lastPhase.end(), 0.0f);
        std::fill(phaseDeviation.begin(), phaseDeviation.end(), 0.0f);
        std::fill(gdsData.begin(), gdsData.end(), 0.0f);
        std::fill(residualSpectrum.begin(), residualSpectrum.end(), 0.0f);
        
        // Reset metrics
        metrics = MetricsFrame();
        
        // Reset true peak detector
        truePeak.reset();
    }
    
    // Process a buffer of audio for metrics analysis
    void processBlock(const juce::AudioBuffer<float>& buffer) {
        const int numChannels = buffer.getNumChannels();
        const int numSamples = buffer.getNumSamples();
        
        if (numChannels < 2 || numSamples <= 0) {
            return;
        }
        
        // Analyze phase/group delay
        analyzeGroupDelay(buffer);
        
        // Analyze IACC/ITD
        analyzeInteraural(buffer);
        
        // Analyze transients
        analyzeTransients(buffer);
        
        // Measure true peak
        truePeak.pushBlock(buffer);
        
        // Update composite SCPI score
        updateSCPI();
    }
    
    // Process A/B comparison for residual analysis
    void processComparison(const juce::AudioBuffer<float>& bufferA, 
                           const juce::AudioBuffer<float>& bufferB) {
        const int numChannels = juce::jmin(bufferA.getNumChannels(), bufferB.getNumChannels());
        const int numSamples = juce::jmin(bufferA.getNumSamples(), bufferB.getNumSamples());
        
        if (numChannels < 1 || numSamples <= 0) {
            return;
        }
        
        // Create residual buffer (A - B)
        juce::AudioBuffer<float> residual(numChannels, numSamples);
        
        for (int ch = 0; ch < numChannels; ++ch) {
            const float* aData = bufferA.getReadPointer(ch);
            const float* bData = bufferB.getReadPointer(ch);
            float* rData = residual.getWritePointer(ch);
            
            for (int i = 0; i < numSamples; ++i) {
                rData[i] = aData[i] - bData[i];
            }
        }
        
        // Analyze residual spectrum
        analyzeResidual(residual);
    }
    
    // Get the current metrics frame
    const MetricsFrame& getMetrics() const {
        return metrics;
    }
    
    // Get the group delay heatmap data
    const std::vector<float>& getGDSData() const {
        return gdsData;
    }
    
    // Get the residual spectrum data
    const std::vector<float>& getResidualSpectrum() const {
        return residualSpectrum;
    }
    
    // Get the true peak level (dB)
    float getTruePeak() const {
        float peak = truePeak.getAndClearPeak();
        return peak > 0.0f ? juce::Decibels::gainToDecibels(peak) : -120.0f;
    }
    
private:
    // Analyze phase/group delay
    void analyzeGroupDelay(const juce::AudioBuffer<float>& buffer) {
        const int numChannels = buffer.getNumChannels();
        const int numSamples = buffer.getNumSamples();
        
        // Process only the first channel for GDS analysis
        if (numChannels > 0) {
            const float* channelData = buffer.getReadPointer(0);
            
            // Copy data to FFT buffer
            for (int i = 0; i < juce::jmin(numSamples, fftSize); ++i) {
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
                
                // Calculate phase
                const float phase = std::atan2(imag, real);
                
                // Calculate unwrapped phase difference (derivative)
                float phaseDiff = phase - lastPhase[bin];
                if (phaseDiff > juce::MathConstants<float>::pi)
                    phaseDiff -= 2.0f * juce::MathConstants<float>::pi;
                else if (phaseDiff < -juce::MathConstants<float>::pi)
                    phaseDiff += 2.0f * juce::MathConstants<float>::pi;
                
                // Convert to group delay (negative phase derivative)
                const float groupDelay = -phaseDiff / (2.0f * juce::MathConstants<float>::pi) * fftSize / sampleRate;
                
                // Store phase for next block
                lastPhase[bin] = phase;
                
                // Update GDS heatmap
                updateGDSHeatmap(bin, groupDelay);
            }
            
            // Calculate GDS score from heatmap
            calculateGDSScore();
        }
    }
    
    // Update the GDS heatmap with new group delay data
    void updateGDSHeatmap(int freqBin, float groupDelaySec) {
        // Convert frequency bin to heatmap X coordinate
        const int x = juce::jlimit(0, gdsWidth - 1, 
                                 static_cast<int>(static_cast<float>(freqBin) / 
                                               static_cast<float>(fftSize / 2) * 
                                               static_cast<float>(gdsWidth)));
        
        // Convert group delay to heatmap Y coordinate (0-5ms range)
        const float maxDelay = 0.005f; // 5 ms
        const float normDelay = juce::jlimit(0.0f, 1.0f, groupDelaySec / maxDelay);
        const int y = juce::jlimit(0, gdsHeight - 1, 
                                 static_cast<int>((1.0f - normDelay) * static_cast<float>(gdsHeight)));
        
        // Update heatmap with smoothing (exponential moving average)
        const int index = y * gdsWidth + x;
        const float alpha = 0.2f; // Smoothing factor
        
        if (index >= 0 && index < static_cast<int>(gdsData.size())) {
            gdsData[index] = gdsData[index] * (1.0f - alpha) + alpha;
        }
    }
    
    // Calculate GDS score from heatmap
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
            metrics.gdsScore = 1.0f - juce::jlimit(0.0f, 1.0f, avgSpread * 2.0f);
        } else {
            metrics.gdsScore = 0.5f; // Default neutral score
        }
        
        // Copy heatmap data to metrics
        metrics.gdsData = gdsData;
        metrics.gdsWidth = gdsWidth;
        metrics.gdsHeight = gdsHeight;
    }
    
    // Analyze interaural coherence and time difference
    void analyzeInteraural(const juce::AudioBuffer<float>& buffer) {
        const int numChannels = buffer.getNumChannels();
        const int numSamples = buffer.getNumSamples();
        
        if (numChannels < 2 || numSamples <= 0) {
            return;
        }
        
        // Get channel data
        const float* leftData = buffer.getReadPointer(0);
        const float* rightData = buffer.getReadPointer(1);
        
        // Calculate cross-correlation
        float sumL2 = 0.0f;
        float sumR2 = 0.0f;
        float sumLR = 0.0f;
        
        for (int i = 0; i < numSamples; ++i) {
            sumL2 += leftData[i] * leftData[i];
            sumR2 += rightData[i] * rightData[i];
            sumLR += leftData[i] * rightData[i];
        }
        
        // Calculate IACC
        const float iacc = sumLR / std::sqrt(sumL2 * sumR2 + 0.0000001f);
        metrics.iacc = juce::jlimit(0.0f, 1.0f, std::abs(iacc));
        
        // Calculate ITD using simple peak finding
        float maxCorr = 0.0f;
        int maxLag = 0;
        const int maxOffset = juce::jmin(48, numSamples / 4); // Max 1ms at 48kHz
        
        for (int lag = -maxOffset; lag <= maxOffset; ++lag) {
            float corr = 0.0f;
            int count = 0;
            
            for (int i = 0; i < numSamples; ++i) {
                const int j = i + lag;
                if (j >= 0 && j < numSamples) {
                    corr += leftData[i] * rightData[j];
                    ++count;
                }
            }
            
            if (count > 0) {
                corr /= static_cast<float>(count);
                if (std::abs(corr) > std::abs(maxCorr)) {
                    maxCorr = corr;
                    maxLag = lag;
                }
            }
        }
        
        // Convert lag to milliseconds
        metrics.itdMs = static_cast<float>(maxLag) / sampleRate * 1000.0f;
        
        // Update ITD variance with exponential moving average
        const float alpha = 0.2f;
        const float diff = metrics.itdMs - lastITD;
        metrics.itdVariance = metrics.itdVariance * (1.0f - alpha) + std::abs(diff) * alpha;
        
        // Store current ITD for next time
        lastITD = metrics.itdMs;
    }
    
    // Analyze transients for TEI metrics
    void analyzeTransients(const juce::AudioBuffer<float>& buffer) {
        const int numChannels = buffer.getNumChannels();
        const int numSamples = buffer.getNumSamples();
        
        if (numChannels < 1 || numSamples <= 0) {
            return;
        }
        
        // For simplicity, we'll just analyze the first channel
        const float* data = buffer.getReadPointer(0);
        
        // Find the sharpest transient in the buffer
        int transientIndex = 0;
        float maxRise = 0.0f;
        
        for (int i = 1; i < numSamples; ++i) {
            const float rise = data[i] - data[i - 1];
            if (rise > maxRise) {
                maxRise = rise;
                transientIndex = i;
            }
        }
        
        // If a significant transient is found
        if (maxRise > 0.05f) {
            // Calculate rise time (10% to 90%)
            float riseTime10 = 0.0f;
            float riseTime90 = 0.0f;
            bool found10 = false;
            bool found90 = false;
            
            const float baseline = data[juce::jmax(0, transientIndex - 5)];
            const float peak = data[juce::jmin(numSamples - 1, transientIndex + 5)];
            const float rise = peak - baseline;
            const float threshold10 = baseline + 0.1f * rise;
            const float threshold90 = baseline + 0.9f * rise;
            
            for (int i = juce::jmax(0, transientIndex - 10); 
                 i < juce::jmin(numSamples, transientIndex + 10); ++i) {
                if (!found10 && data[i] >= threshold10) {
                    riseTime10 = static_cast<float>(i);
                    found10 = true;
                }
                if (!found90 && data[i] >= threshold90) {
                    riseTime90 = static_cast<float>(i);
                    found90 = true;
                }
                if (found10 && found90) break;
            }
            
            // Calculate rise time in microseconds
            if (found10 && found90) {
                const float riseTimeSamples = riseTime90 - riseTime10;
                metrics.teiRiseTime = riseTimeSamples / sampleRate * 1000000.0f;
            }
            
            // Calculate pre-ring energy (energy before the transient)
            float preEnergy = 0.0f;
            const int preRingWindow = 10; // 10 samples before transient
            
            for (int i = juce::jmax(0, transientIndex - preRingWindow); i < transientIndex; ++i) {
                preEnergy += data[i] * data[i];
            }
            
            // Convert to dB
            metrics.teiPreRing = 10.0f * std::log10(preEnergy / static_cast<float>(preRingWindow) + 0.0000001f);
            
            // Calculate overshoot
            float overshoot = 0.0f;
            const int overshootWindow = 10; // 10 samples after transient
            
            for (int i = transientIndex; i < juce::jmin(numSamples, transientIndex + overshootWindow); ++i) {
                overshoot = juce::jmax(overshoot, data[i] - peak);
            }
            
            metrics.teiOvershoot = 100.0f * overshoot / juce::jmax(0.0001f, rise);
            
            // Calculate TEI score (higher is better)
            const float riseTimeScore = std::exp(-std::abs(metrics.teiRiseTime - 20.0f) / 50.0f);
            const float preRingScore = std::exp(-std::abs(metrics.teiPreRing + 40.0f) / 20.0f);
            const float overshootScore = std::exp(-std::abs(metrics.teiOvershoot) / 20.0f);
            
            metrics.teiScore = riseTimeScore * 0.4f + preRingScore * 0.4f + overshootScore * 0.2f;
        }
    }
    
    // Analyze residual for spectral characteristics
    void analyzeResidual(const juce::AudioBuffer<float>& residual) {
        const int numChannels = residual.getNumChannels();
        const int numSamples = residual.getNumSamples();
        
        if (numChannels < 1 || numSamples <= 0) {
            return;
        }
        
        // Use the first channel for spectral analysis
        const float* data = residual.getReadPointer(0);
        
        // Copy data to FFT buffer
        for (int i = 0; i < juce::jmin(numSamples, fftSize); ++i) {
            fftBuffer[i * 2] = data[i];
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
        forwardFFT->performFrequencyOnlyForwardTransform(fftBuffer.data());
        
        // Calculate spectrum and metrics
        float sumSquares = 0.0f;
        float peakValue = 0.0f;
        float lowSum = 0.0f;
        float highSum = 0.0f;
        
        // Define frequency bands for tilt calculation
        const int lowBand = fftSize / 8;
        const int highBand = fftSize * 3/4;
        
        // Extract magnitudes and calculate metrics
        for (int i = 0; i < fftSize / 2; ++i) {
            // Extract magnitude
            const float mag = fftBuffer[i];
            residualSpectrum[i] = mag;
            
            // Accumulate for RMS
            sumSquares += mag * mag;
            
            // Find peak
            peakValue = juce::jmax(peakValue, mag);
            
            // Accumulate for spectral tilt
            if (i < lowBand) {
                lowSum += mag;
            } else if (i >= highBand) {
                highSum += mag;
            }
        }
        
        // Calculate RMS
        const float rms = std::sqrt(sumSquares / static_cast<float>(fftSize / 2));
        metrics.residualRMS = 20.0f * std::log10(rms + 0.0000001f);
        
        // Calculate crest factor
        metrics.residualCrest = 20.0f * std::log10((peakValue / rms) + 0.0000001f);
        
        // Calculate spectral tilt
        if (lowSum > 0.0f && highSum > 0.0f) {
            const float lowAvg = lowSum / static_cast<float>(lowBand);
            const float highAvg = highSum / static_cast<float>(fftSize / 2 - highBand);
            metrics.residualTilt = 20.0f * std::log10(highAvg / lowAvg);
        }
        
        // Store spectrum in metrics
        metrics.residualSpectrum = residualSpectrum;
    }
    
    // Update composite SCPI (Stereo Spatial Cue Preservation Index) score
    void updateSCPI() {
        // Weighted combination of individual metrics
        // Higher is better for GDS and TEI
        // Lower is better for ITD variance and residual
        
        // Scale ITD variance to 0-1 (lower is better)
        const float itdVarScore = std::exp(-metrics.itdVariance / 0.2f);
        
        // Scale residual to 0-1 (lower is better)
        const float residualScore = std::exp(-std::abs(metrics.residualRMS + 60.0f) / 30.0f);
        
        // Calculate composite score
        metrics.scpiScore = metrics.gdsScore * 0.3f +
                           metrics.iacc * 0.2f +
                           itdVarScore * 0.2f +
                           metrics.teiScore * 0.2f +
                           residualScore * 0.1f;
    }
    
    // Instance variables
    double sampleRate = 44100.0;
    int maxBlockSize = 512;
    int fftSize = 2048;
    int fftOverlap = 1024;
    
    std::unique_ptr<juce::dsp::FFT> forwardFFT;
    std::unique_ptr<juce::dsp::WindowingFunction<float>> window;
    
    std::vector<float> fftBuffer;
    std::vector<float> lastPhase;
    std::vector<float> phaseDeviation;
    
    int gdsWidth = 256;
    int gdsHeight = 128;
    std::vector<float> gdsData;
    
    std::vector<float> residualSpectrum;
    
    float lastITD = 0.0f;
    
    MetricsFrame metrics;
    mojoDSP::TruePeakEstimator truePeak;
};

} // namespace mojo
