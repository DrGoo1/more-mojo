#pragma once
#include <JuceHeader.h>
#include <array>
#include <vector>
#include <complex>

namespace mojoDSP {

/**
 * TransientDetect - Multi-band transient detection using phase deviation
 * 
 * Features:
 * - Phase deviation detection across frequency bands
 * - Adaptive sensitivity and hysteresis
 * - Transient time and confidence estimation
 */
class TransientDetect {
public:
    TransientDetect() = default;
    
    struct TransientInfo {
        float time = 0.0f;      // Time offset within block
        float confidence = 0.0f; // Detection confidence (0-1)
        int band = 0;           // Band index where detected
    };
    
    void prepare(double sampleRate, int maxBlockSize) {
        this->sampleRate = sampleRate;
        
        // Initialize FFT
        fftSize = 1024;
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
        
        // Initialize band filters
        setupBands();
        
        // Reset state
        reset();
    }
    
    void reset() {
        // Clear buffers
        std::fill(fftBuffer.begin(), fftBuffer.end(), 0.0f);
        std::fill(lastPhase.begin(), lastPhase.end(), 0.0f);
        std::fill(phaseDeviation.begin(), phaseDeviation.end(), 0.0f);
        
        // Reset band information
        for (auto& band : bands) {
            band.energy = 0.0f;
            band.lastEnergy = 0.0f;
            band.phaseDev = 0.0f;
            band.transientConfidence = 0.0f;
        }
        
        // Clear detection results
        lastTransients.clear();
    }
    
    void setParameters(int numBands, float sensitivity, float hysteresis, float smoothing) {
        this->numBands = juce::jlimit(4, 12, numBands);
        this->sensitivity = juce::jlimit(0.0f, 1.0f, sensitivity);
        this->hysteresis = juce::jlimit(0.0f, 1.0f, hysteresis);
        this->smoothing = juce::jlimit(0.0f, 0.5f, smoothing);
        
        // Update band settings if needed
        if (this->numBands != bands.size()) {
            setupBands();
        }
    }
    
    void process(juce::AudioBuffer<float>& buffer, juce::AudioProcessorValueTreeState& apvts) {
        const int numChannels = buffer.getNumChannels();
        const int numSamples = buffer.getNumSamples();
        
        // Get parameters from APVTS
        const int numBands = static_cast<int>(*apvts.getRawParameterValue("transient_bands"));
        const float sensitivity = apvts.getRawParameterValue("transient_sensitivity")->load() * 0.01f; // 0-100 to 0-1
        const float hysteresis = apvts.getRawParameterValue("transient_hysteresis")->load() * 0.01f; // 0-100 to 0-1
        const float smoothing = apvts.getRawParameterValue("transient_smoothing")->load() * 0.01f; // 0-50 to 0-0.5
        
        // Update parameters
        setParameters(numBands, sensitivity, hysteresis, smoothing);
        
        // Process only the first channel (or mix channels for better detection)
        if (numChannels > 0) {
            // Get the first channel for analysis
            const float* channelData = buffer.getReadPointer(0);
            
            // Copy data to FFT buffer with overlap
            for (int i = 0; i < numSamples; ++i) {
                fftBuffer[i] = channelData[i];
            }
            
            // Apply window function
            window->multiplyWithWindowingTable(fftBuffer.data(), fftSize);
            
            // Perform FFT
            forwardFFT->performFrequencyOnlyForwardTransform(fftBuffer.data());
            
            // Calculate phase deviation and detect transients
            calculatePhaseDeviation();
            detectTransients(numSamples);
            
            // Store FFT results for next time
            storeFFTResults();
        }
    }
    
    // Get detected transients from the last processed block
    const std::vector<TransientInfo>& getTransients() const {
        return lastTransients;
    }
    
    // Get transient onsets by band (for UI visualization)
    const std::array<float, 12>& getBandTransientConfidence() const {
        std::array<float, 12> result = {};
        for (size_t i = 0; i < bands.size() && i < result.size(); ++i) {
            result[i] = bands[i].transientConfidence;
        }
        return result;
    }
    
private:
    struct Band {
        float minFreq;           // Minimum frequency
        float maxFreq;           // Maximum frequency
        int minBin;              // Minimum FFT bin
        int maxBin;              // Maximum FFT bin
        float energy;            // Current energy
        float lastEnergy;        // Previous energy
        float phaseDev;          // Phase deviation
        float transientConfidence; // Confidence of transient detection
    };
    
    // Set up frequency bands with quasi-logarithmic spacing
    void setupBands() {
        bands.resize(numBands);
        
        // Set up frequency bands with quasi-logarithmic spacing
        const float minFreq = 20.0f;
        const float maxFreq = static_cast<float>(sampleRate) / 2.0f;
        const float freqRatio = std::pow(maxFreq / minFreq, 1.0f / numBands);
        
        for (int i = 0; i < numBands; ++i) {
            Band& band = bands[i];
            
            // Calculate band frequency range
            band.minFreq = minFreq * std::pow(freqRatio, i);
            band.maxFreq = minFreq * std::pow(freqRatio, i + 1);
            
            // Map to FFT bins
            band.minBin = juce::jlimit(0, fftSize / 2 - 1, 
                             static_cast<int>(band.minFreq * fftSize / sampleRate));
            band.maxBin = juce::jlimit(band.minBin, fftSize / 2 - 1, 
                             static_cast<int>(band.maxFreq * fftSize / sampleRate));
            
            // Initialize energy and phase
            band.energy = 0.0f;
            band.lastEnergy = 0.0f;
            band.phaseDev = 0.0f;
            band.transientConfidence = 0.0f;
        }
    }
    
    // Calculate phase deviation from the FFT data
    void calculatePhaseDeviation() {
        // Process each frequency band
        for (auto& band : bands) {
            float bandEnergy = 0.0f;
            float bandPhaseDev = 0.0f;
            
            // Sum energy and phase deviation across bins in this band
            for (int bin = band.minBin; bin <= band.maxBin; ++bin) {
                // Magnitude at this bin
                const float magnitude = fftBuffer[bin];
                
                // Calculate phase at this bin
                const float phase = std::atan2(fftBuffer[bin*2+1], fftBuffer[bin*2]);
                
                // Unwrap phase and calculate phase deviation
                float phaseDiff = phase - lastPhase[bin];
                if (phaseDiff > juce::MathConstants<float>::pi)
                    phaseDiff -= 2.0f * juce::MathConstants<float>::pi;
                else if (phaseDiff < -juce::MathConstants<float>::pi)
                    phaseDiff += 2.0f * juce::MathConstants<float>::pi;
                
                // Store phase deviation weighted by magnitude
                phaseDeviation[bin] = magnitude * std::abs(phaseDiff);
                
                // Accumulate for the band
                bandEnergy += magnitude;
                bandPhaseDev += phaseDeviation[bin];
            }
            
            // Normalize band results
            const int numBins = band.maxBin - band.minBin + 1;
            if (numBins > 0) {
                bandEnergy /= static_cast<float>(numBins);
                bandPhaseDev /= static_cast<float>(numBins);
            }
            
            // Apply smoothing
            band.energy = band.energy * smoothing + bandEnergy * (1.0f - smoothing);
            band.phaseDev = band.phaseDev * smoothing + bandPhaseDev * (1.0f - smoothing);
        }
    }
    
    // Detect transients based on energy and phase deviation
    void detectTransients(int numSamples) {
        // Clear previous detections
        lastTransients.clear();
        
        // Process each band
        for (int bandIdx = 0; bandIdx < static_cast<int>(bands.size()); ++bandIdx) {
            auto& band = bands[bandIdx];
            
            // Calculate energy derivative
            float energyDerivative = band.energy - band.lastEnergy;
            
            // Detect transients using energy derivative and phase deviation
            float transientScore = energyDerivative * band.phaseDev;
            
            // Apply sensitivity threshold
            const float threshold = 0.1f * (1.0f - sensitivity);
            
            // Detect onset with hysteresis
            if (transientScore > threshold && 
                band.transientConfidence < 0.5f - hysteresis * 0.25f) {
                // Transient detected
                band.transientConfidence = 1.0f;
                
                // Create transient info
                TransientInfo info;
                info.time = static_cast<float>(numSamples / 2) / static_cast<float>(sampleRate);
                info.confidence = juce::jlimit(0.0f, 1.0f, transientScore / (threshold * 10.0f));
                info.band = bandIdx;
                
                // Add to results
                lastTransients.push_back(info);
            } else {
                // Decay confidence
                band.transientConfidence *= 0.5f;
            }
            
            // Store current energy for next time
            band.lastEnergy = band.energy;
        }
    }
    
    // Store FFT results for the next block
    void storeFFTResults() {
        // Store phase for next time
        for (int bin = 0; bin < fftSize / 2; ++bin) {
            lastPhase[bin] = std::atan2(fftBuffer[bin*2+1], fftBuffer[bin*2]);
        }
    }
    
    // Instance variables
    double sampleRate = 44100.0;
    int fftSize = 1024;
    int fftOverlap = 512;
    int numBands = 6;
    float sensitivity = 0.5f;
    float hysteresis = 0.5f;
    float smoothing = 0.2f;
    
    std::unique_ptr<juce::dsp::FFT> forwardFFT;
    std::unique_ptr<juce::dsp::WindowingFunction<float>> window;
    
    std::vector<float> fftBuffer;
    std::vector<float> lastPhase;
    std::vector<float> phaseDeviation;
    std::vector<Band> bands;
    
    std::vector<TransientInfo> lastTransients;
};

} // namespace mojoDSP
