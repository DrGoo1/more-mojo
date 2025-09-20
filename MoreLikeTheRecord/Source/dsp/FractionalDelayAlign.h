#pragma once
#include <JuceHeader.h>
#include <array>
#include <vector>
#include <atomic>

namespace mojoDSP {

/**
 * GCCPHAT - Generalized Cross-Correlation Phase Transform
 * Used for estimating the delay between two channels
 */
class GCCPHAT {
public:
    GCCPHAT() = default;
    
    void prepare(double sampleRate, int maxBlockSize) {
        this->sampleRate = sampleRate;
        
        // Initialize FFT
        fftSize = 1024;
        
        // Find next power of 2 for FFT
        while (fftSize < maxBlockSize * 2) {
            fftSize *= 2;
        }
        
        // Set up FFT objects
        forwardFFT = std::make_unique<juce::dsp::FFT>(std::log2(fftSize));
        inverseFFT = std::make_unique<juce::dsp::FFT>(std::log2(fftSize));
        window = std::make_unique<juce::dsp::WindowingFunction<float>>(
            fftSize, juce::dsp::WindowingFunction<float>::hann);
        
        // Initialize buffers
        bufferL.resize(fftSize * 2); // Complex data (real/imag pairs)
        bufferR.resize(fftSize * 2);
        correlation.resize(fftSize * 2);
        
        // Reset state
        reset();
    }
    
    void reset() {
        std::fill(bufferL.begin(), bufferL.end(), 0.0f);
        std::fill(bufferR.begin(), bufferR.end(), 0.0f);
        std::fill(correlation.begin(), correlation.end(), 0.0f);
    }
    
    // Calculate ITD (Interaural Time Difference) in samples
    float calculateITD(const float* leftChannel, const float* rightChannel, int numSamples) {
        if (numSamples <= 0 || !forwardFFT) {
            return 0.0f;
        }
        
        // Copy data to FFT buffers
        for (int i = 0; i < numSamples && i < fftSize; ++i) {
            bufferL[i * 2] = leftChannel[i];
            bufferL[i * 2 + 1] = 0.0f; // Imaginary part
            bufferR[i * 2] = rightChannel[i];
            bufferR[i * 2 + 1] = 0.0f; // Imaginary part
        }
        
        // Zero-pad if needed
        for (int i = numSamples; i < fftSize; ++i) {
            bufferL[i * 2] = 0.0f;
            bufferL[i * 2 + 1] = 0.0f;
            bufferR[i * 2] = 0.0f;
            bufferR[i * 2 + 1] = 0.0f;
        }
        
        // Apply window
        window->multiplyWithWindowingTable(bufferL.data(), fftSize);
        window->multiplyWithWindowingTable(bufferR.data(), fftSize);
        
        // Perform FFT
        forwardFFT->performRealOnlyForwardTransform(bufferL.data(), true);
        forwardFFT->performRealOnlyForwardTransform(bufferR.data(), true);
        
        // Calculate cross-power spectrum
        for (int i = 0; i < fftSize; ++i) {
            const float realL = bufferL[i * 2];
            const float imagL = bufferL[i * 2 + 1];
            const float realR = bufferR[i * 2];
            const float imagR = bufferR[i * 2 + 1];
            
            // Complex multiplication (L * conj(R))
            const float real = realL * realR + imagL * imagR;
            const float imag = imagL * realR - realL * imagR;
            
            // Normalize (PHAT transform)
            const float magnitude = std::sqrt(real * real + imag * imag);
            if (magnitude > 1e-8f) {
                correlation[i * 2] = real / magnitude;
                correlation[i * 2 + 1] = imag / magnitude;
            } else {
                correlation[i * 2] = 0.0f;
                correlation[i * 2 + 1] = 0.0f;
            }
        }
        
        // Inverse FFT to get time-domain correlation
        inverseFFT->performRealOnlyInverseTransform(correlation.data());
        
        // Find the peak of the correlation
        float maxVal = 0.0f;
        int maxIndex = 0;
        
        for (int i = 0; i < fftSize; ++i) {
            const float value = std::abs(correlation[i]);
            if (value > maxVal) {
                maxVal = value;
                maxIndex = i;
            }
        }
        
        // Adjust index for circular nature of FFT
        if (maxIndex > fftSize / 2) {
            maxIndex -= fftSize;
        }
        
        // Return ITD in samples
        return static_cast<float>(maxIndex);
    }
    
private:
    double sampleRate = 44100.0;
    int fftSize = 1024;
    
    std::unique_ptr<juce::dsp::FFT> forwardFFT;
    std::unique_ptr<juce::dsp::FFT> inverseFFT;
    std::unique_ptr<juce::dsp::WindowingFunction<float>> window;
    
    std::vector<float> bufferL;
    std::vector<float> bufferR;
    std::vector<float> correlation;
};

/**
 * FractionalDelayAlign - Micro-timing restoration and phase alignment
 * 
 * Features:
 * - Per-band fractional delay for phase/timing alignment
 * - ITD (Interaural Time Difference) stabilization
 * - Sub-millisecond delay resolution (microsecond-level)
 */
class FractionalDelayAlign {
public:
    FractionalDelayAlign() = default;
    
    void prepare(double sampleRate, int maxBlockSize) {
        this->sampleRate = sampleRate;
        
        // Initialize bands and delay lines
        setupBands(numBands);
        
        // Initialize GCC-PHAT
        gcc.prepare(sampleRate, maxBlockSize);
        
        // Reset state
        reset();
    }
    
    void reset() {
        // Reset delay lines
        for (auto& delayLine : delay) {
            delayLine->reset();
        }
        
        // Reset GCC-PHAT
        gcc.reset();
    }
    
    void setParameters(int numBands, float maxCorrection, float stepSize, 
                       float itdTarget, float widthProtect, float combAvoid) {
        // Update parameters
        this->numBands = juce::jlimit(1, 12, numBands);
        this->maxCorrectionMs = juce::jlimit(0.0f, 2.0f, maxCorrection);
        this->stepSizeMs = juce::jlimit(0.01f, 0.2f, stepSize);
        this->itdTarget = juce::jlimit(-1.0f, 1.0f, itdTarget);
        this->widthProtect = juce::jlimit(0.0f, 1.0f, widthProtect);
        this->combAvoid = juce::jlimit(0.0f, 1.0f, combAvoid);
        
        // Update bands if needed
        if (this->numBands != static_cast<int>(bands.size())) {
            setupBands(this->numBands);
        }
    }
    
    void process(juce::AudioBuffer<float>& buffer, juce::AudioProcessorValueTreeState& apvts) {
        const int numChannels = buffer.getNumChannels();
        const int numSamples = buffer.getNumSamples();
        
        // Need at least stereo for alignment
        if (numChannels < 2 || numSamples <= 0) {
            return;
        }
        
        // Get parameters from APVTS
        const int numBands = static_cast<int>(*apvts.getRawParameterValue("align_bands"));
        const float maxCorrection = *apvts.getRawParameterValue("align_max_correction");
        const float stepSize = *apvts.getRawParameterValue("align_step_size");
        const float itdTarget = *apvts.getRawParameterValue("align_itd_target");
        const float widthProtect = *apvts.getRawParameterValue("align_width_protect") * 0.01f; // 0-100 to 0-1
        const float combAvoid = *apvts.getRawParameterValue("align_comb_avoid") * 0.01f; // 0-100 to 0-1
        
        // Update parameters
        setParameters(numBands, maxCorrection, stepSize, itdTarget, widthProtect, combAvoid);
        
        // Create temporary buffer for band filtering
        juce::AudioBuffer<float> tempBuffer(numChannels, numSamples);
        
        // Calculate ITD between L/R channels
        float itdSamples = gcc.calculateITD(
            buffer.getReadPointer(0), 
            buffer.getReadPointer(1), 
            numSamples);
        
        // Store for reporting
        currentITD.store(itdSamples);
        
        // Calculate target ITD (0 = centered, negative = left leads, positive = right leads)
        float targetITDSamples = itdTarget * 0.001f * static_cast<float>(sampleRate);
        
        // Calculate correction amount while respecting width protection
        float itdCorrection = (itdSamples - targetITDSamples) * (1.0f - widthProtect);
        
        // Apply ITD correction to achieve target ITD
        if (std::abs(itdCorrection) > 0.01f) {
            // Convert to sample delay amount
            float delayL = 0.0f;
            float delayR = 0.0f;
            
            // Determine which channel to delay
            if (itdCorrection > 0) {
                // Left channel is ahead, delay it
                delayL = itdCorrection;
            } else {
                // Right channel is ahead, delay it
                delayR = -itdCorrection;
            }
            
            // Limit to max correction
            const float maxDelaySamples = maxCorrectionMs * 0.001f * static_cast<float>(sampleRate);
            delayL = juce::jlimit(0.0f, maxDelaySamples, delayL);
            delayR = juce::jlimit(0.0f, maxDelaySamples, delayR);
            
            // Apply fractional delay
            if (delayL > 0.0f) {
                applyFractionalDelay(buffer, 0, delayL);
            }
            if (delayR > 0.0f) {
                applyFractionalDelay(buffer, 1, delayR);
            }
        }
        
        // TODO: Implement per-band alignment (requires more complex filtering)
    }
    
    std::atomic<float> currentITD { 0.0f };
    
private:
    struct Band {
        float minFreq;     // Minimum frequency
        float maxFreq;     // Maximum frequency
        float delayAmount; // Delay amount in samples
    };
    
    // Set up frequency bands with quasi-logarithmic spacing
    void setupBands(int numBands) {
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
            band.delayAmount = 0.0f;
        }
        
        // Initialize delay lines (one per channel, up to 8 channels)
        delay.clear();
        for (int i = 0; i < 8; ++i) {
            delay.push_back(std::make_unique<juce::dsp::DelayLine<float>>(
                static_cast<int>(2.0f * sampleRate))); // Max 2 seconds
            delay.back()->prepare({sampleRate, static_cast<juce::uint32>(maxBlockSize), 1});
            delay.back()->setDelay(0.0f);
        }
    }
    
    // Apply fractional delay to a channel
    void applyFractionalDelay(juce::AudioBuffer<float>& buffer, int channel, float delaySamples) {
        if (channel >= buffer.getNumChannels() || channel >= static_cast<int>(delay.size())) {
            return;
        }
        
        // Set delay
        delay[channel]->setDelay(delaySamples);
        
        // Get channel data
        float* data = buffer.getWritePointer(channel);
        const int numSamples = buffer.getNumSamples();
        
        // Apply delay
        for (int i = 0; i < numSamples; ++i) {
            const float input = data[i];
            data[i] = delay[channel]->popSample(0);
            delay[channel]->pushSample(0, input);
        }
    }
    
    // Instance variables
    double sampleRate = 44100.0;
    int maxBlockSize = 512;
    int numBands = 6;
    float maxCorrectionMs = 0.6f;
    float stepSizeMs = 0.05f;
    float itdTarget = 0.0f;
    float widthProtect = 0.5f;
    float combAvoid = 0.5f;
    
    GCCPHAT gcc;
    std::vector<Band> bands;
    std::vector<std::unique_ptr<juce::dsp::DelayLine<float>>> delay;
};

} // namespace mojoDSP
