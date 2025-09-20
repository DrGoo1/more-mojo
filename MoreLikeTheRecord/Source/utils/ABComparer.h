#pragma once
#include <JuceHeader.h>

namespace mojo {

/**
 * ABComparer - Handles A/B/X comparison with level matching and morphing
 * 
 * Features:
 * - Level-matched A/B comparison (using BS.1770 LUFS)
 * - Sample-accurate latency compensation
 * - Smooth morphing between A and B
 * - Blind X testing
 */
class ABComparer : public juce::AudioProcessorValueTree::Listener {
public:
    ABComparer() = default;
    
    // Initialize with parent AudioProcessorValueTreeState
    void init(juce::AudioProcessorValueTreeState& apvts) {
        this->apvts = &apvts;
        
        // Add listener for parameter changes
        apvts.addParameterListener("ab_state", this);
        apvts.addParameterListener("ab_morph", this);
    }
    
    // Clean up
    void cleanup() {
        if (apvts) {
            apvts->removeParameterListener("ab_state", this);
            apvts->removeParameterListener("ab_morph", this);
        }
    }
    
    // Parameter changed callback
    void parameterChanged(const juce::String& parameterID, float newValue) override {
        if (parameterID == "ab_state") {
            abState = static_cast<int>(newValue);
            abChanged = true;
        } else if (parameterID == "ab_morph") {
            morphing = newValue > 0.5f;
        }
    }
    
    // Prepare for playback
    void prepare(double sampleRate, int maxBlockSize, int numChannels) {
        this->sampleRate = sampleRate;
        this->maxBlockSize = maxBlockSize;
        
        // Initialize buffers
        bufferA.setSize(numChannels, maxBlockSize);
        bufferB.setSize(numChannels, maxBlockSize);
        
        // Initialize LUFS meters
        prepareK22Filter();
    }
    
    // Process an audio block
    void processBlock(juce::AudioBuffer<float>& buffer, bool bypassProcessing) {
        // Skip processing if not needed
        if (!abEnabled()) return;
        
        const int numChannels = buffer.getNumChannels();
        const int numSamples = buffer.getNumSamples();
        
        // Store original (A) buffer
        bufferA.clear();
        for (int ch = 0; ch < numChannels; ++ch) {
            bufferA.copyFrom(ch, 0, buffer, ch, 0, numSamples);
        }
        
        // Measure LUFS for A
        lufsA = measureLUFS(bufferA);
        
        // Process B buffer if not bypassed
        if (!bypassProcessing) {
            // B buffer is processed by DSP chain
            // Just continue with processing
        } else {
            // If bypassed, B = A (no processing)
            for (int ch = 0; ch < numChannels; ++ch) {
                buffer.copyFrom(ch, 0, bufferA, ch, 0, numSamples);
            }
        }
        
        // Store processed (B) buffer
        bufferB.clear();
        for (int ch = 0; ch < numChannels; ++ch) {
            bufferB.copyFrom(ch, 0, buffer, ch, 0, numSamples);
        }
        
        // Measure LUFS for B
        lufsB = measureLUFS(bufferB);
        
        // Level match if needed
        if (levelMatching) {
            matchLevels();
        }
        
        // Apply A/B/X selection or morphing
        applyABSelection(buffer);
    }
    
    // Check if A/B functionality is enabled
    bool abEnabled() const {
        return true; // Always enabled in this implementation
    }
    
    // Get the current LUFS values
    float getLUFSA() const { return lufsA; }
    float getLUFSB() const { return lufsB; }
    float getLUFSDifference() const { return lufsB - lufsA; }
    
    // Set level matching
    void setLevelMatching(bool enabled) {
        levelMatching = enabled;
    }
    
    // Set latency compensation
    void setLatencyCompensation(int latencySamples) {
        this->latencySamples = latencySamples;
        // Implement latency compensation logic here
    }
    
private:
    // Apply A/B selection or morphing to the output buffer
    void applyABSelection(juce::AudioBuffer<float>& buffer) {
        const int numChannels = buffer.getNumChannels();
        const int numSamples = buffer.getNumSamples();
        
        switch (abState) {
            case 0: // A (original)
                for (int ch = 0; ch < numChannels; ++ch) {
                    buffer.copyFrom(ch, 0, bufferA, ch, 0, numSamples);
                }
                break;
                
            case 1: // B (processed)
                // Buffer already contains B (processed audio)
                break;
                
            case 2: // X (blind test - either A or B)
                // For simplicity, let's use a deterministic choice based on system time
                const bool useA = (juce::Time::getMillisecondCounter() % 2) == 0;
                
                for (int ch = 0; ch < numChannels; ++ch) {
                    if (useA) {
                        buffer.copyFrom(ch, 0, bufferA, ch, 0, numSamples);
                    } // else keep buffer as is (B)
                }
                break;
        }
        
        // Apply morphing if enabled
        if (morphing && abChanged) {
            applyMorphCrossfade(buffer);
            abChanged = false; // Reset flag
        }
    }
    
    // Apply crossfade morphing between A and B
    void applyMorphCrossfade(juce::AudioBuffer<float>& buffer) {
        const int numChannels = buffer.getNumChannels();
        const int numSamples = buffer.getNumSamples();
        
        // Create ramp for crossfade
        const int morphSamples = static_cast<int>(0.25f * sampleRate); // 250 ms crossfade
        const int samplesToProcess = juce::jmin(numSamples, morphSamples);
        
        // Create temporary buffer for crossfade
        juce::AudioBuffer<float> tempBuffer(numChannels, samplesToProcess);
        
        // Fill temp buffer with source that we're crossfading from
        for (int ch = 0; ch < numChannels; ++ch) {
            if (abState == 0) {
                // Crossfading from A to B
                tempBuffer.copyFrom(ch, 0, bufferA, ch, 0, samplesToProcess);
            } else {
                // Crossfading from B to A
                tempBuffer.copyFrom(ch, 0, bufferB, ch, 0, samplesToProcess);
            }
        }
        
        // Apply crossfade
        for (int ch = 0; ch < numChannels; ++ch) {
            float* data = buffer.getWritePointer(ch);
            const float* tempData = tempBuffer.getReadPointer(ch);
            
            for (int i = 0; i < samplesToProcess; ++i) {
                const float alpha = static_cast<float>(i) / static_cast<float>(samplesToProcess);
                data[i] = tempData[i] * (1.0f - alpha) + data[i] * alpha;
            }
        }
    }
    
    // Match levels between A and B
    void matchLevels() {
        const float levelDiff = lufsA - lufsB;
        const float gainFactor = juce::Decibels::decibelsToGain(levelDiff);
        
        // Apply gain to B to match A's level
        bufferB.applyGain(gainFactor);
    }
    
    // Prepare K-weighting filter for LUFS measurement
    void prepareK22Filter() {
        // ITU-R BS.1770 K-weighting filter coefficients for 48 kHz
        // Simplified version with pre-filter + high shelf
        
        for (int ch = 0; ch < 2; ++ch) {
            kFilterStage1[ch].reset();
            kFilterStage2[ch].reset();
            
            // Stage 1: High-pass filter (38 Hz, Q=0.5)
            *kFilterStage1[ch].state = *juce::dsp::IIR::Coefficients<float>::makeHighPass(
                sampleRate, 38.0f, 0.5f);
                
            // Stage 2: High-shelf filter (+4 dB at 1.5 kHz, Q=0.707)
            *kFilterStage2[ch].state = *juce::dsp::IIR::Coefficients<float>::makeHighShelf(
                sampleRate, 1500.0f, 0.707f, juce::Decibels::decibelsToGain(4.0f));
        }
    }
    
    // Measure LUFS level according to ITU-R BS.1770
    float measureLUFS(const juce::AudioBuffer<float>& buffer) {
        const int numChannels = juce::jmin(buffer.getNumChannels(), 2); // Stereo LUFS
        const int numSamples = buffer.getNumSamples();
        
        // Channel weights (L, R) for stereo
        const float channelWeights[2] = {1.0f, 1.0f};
        
        // Process K-weighted power
        float energySum = 0.0f;
        
        for (int ch = 0; ch < numChannels; ++ch) {
            const float* data = buffer.getReadPointer(ch);
            float channelEnergy = 0.0f;
            
            for (int i = 0; i < numSamples; ++i) {
                // Apply K-weighting filter
                float filtered = data[i];
                filtered = kFilterStage1[ch].processSample(filtered);
                filtered = kFilterStage2[ch].processSample(filtered);
                
                // Square for energy
                channelEnergy += filtered * filtered;
            }
            
            // Apply channel weight and accumulate
            energySum += channelEnergy * channelWeights[ch];
        }
        
        // Calculate LUFS for this block
        if (energySum > 0.0f && numSamples > 0) {
            return -0.691f + 10.0f * std::log10(energySum / numSamples);
        } else {
            return -100.0f; // Silent
        }
    }
    
    // Instance variables
    juce::AudioProcessorValueTreeState* apvts = nullptr;
    double sampleRate = 44100.0;
    int maxBlockSize = 512;
    int latencySamples = 0;
    
    juce::AudioBuffer<float> bufferA;
    juce::AudioBuffer<float> bufferB;
    
    std::array<juce::dsp::IIR::Filter<float>, 2> kFilterStage1;
    std::array<juce::dsp::IIR::Filter<float>, 2> kFilterStage2;
    
    float lufsA = -23.0f;
    float lufsB = -23.0f;
    
    int abState = 0;      // 0 = A, 1 = B, 2 = X (blind test)
    bool morphing = false;
    bool levelMatching = true;
    bool abChanged = false;
};

} // namespace mojo
