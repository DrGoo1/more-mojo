#pragma once
#include <JuceHeader.h>

namespace moremojo {
namespace mlar {

/**
 * TEIAnalyzer - Transient Edge Integrity analyzer
 * 
 * Features:
 * - Analyzes transient properties between original and processed audio
 * - Measures rise time differences (microseconds)
 * - Measures pre-ring energy differences (dB)
 * - Calculates overshoot percentage
 */
class TEIAnalyzer {
public:
    TEIAnalyzer() = default;
    ~TEIAnalyzer() = default;
    
    // Public metrics for easy access
    float riseDeltaUs = 0.0f;      // Rise time difference in microseconds
    float preRingDeltaDb = 0.0f;   // Pre-ring energy difference in dB
    float overshootPct = 0.0f;     // Overshoot percentage
    
    /**
     * Prepare for processing
     * 
     * @param sampleRate The current sample rate
     */
    void prepare(double sampleRate) {
        this->sampleRate = sampleRate;
        microSecondsPerSample = 1000000.0 / sampleRate;
    }
    
    /**
     * Reset the analyzer
     */
    void reset() {
        riseDeltaUs = 0.0f;
        preRingDeltaDb = 0.0f;
        overshootPct = 0.0f;
    }
    
    /**
     * Analyze transients in pre/post audio buffers
     * 
     * @param pre Original (pre-processing) audio buffer
     * @param post Processed (post-processing) audio buffer
     */
    void analyze(const juce::AudioBuffer<float>& pre, const juce::AudioBuffer<float>& post) {
        // Skip if buffers are empty or different sizes
        if (pre.getNumSamples() == 0 || post.getNumSamples() == 0 ||
            pre.getNumSamples() != post.getNumSamples()) {
            return;
        }
        
        // Extract transients from both buffers
        int preIndex = 0;
        int postIndex = 0;
        float preRiseTime = 0.0f;
        float postRiseTime = 0.0f;
        
        findTransient(pre, preIndex, preRiseTime);
        findTransient(post, postIndex, postRiseTime);
        
        // Calculate rise time difference (in microseconds)
        riseDeltaUs = (postRiseTime - preRiseTime) * static_cast<float>(microSecondsPerSample);
        
        // Calculate pre-ring energy difference
        float prePreRingEnergy = calculatePreRingEnergy(pre, preIndex);
        float postPreRingEnergy = calculatePreRingEnergy(post, postIndex);
        
        preRingDeltaDb = 10.0f * std::log10((postPreRingEnergy + 1.0e-9f) / (prePreRingEnergy + 1.0e-9f));
        
        // Calculate overshoot
        float preMax = findMaxAfterTransient(pre, preIndex);
        float postMax = findMaxAfterTransient(post, postIndex);
        
        if (preMax > 0.01f) {
            overshootPct = 100.0f * (postMax / preMax - 1.0f);
        } else {
            overshootPct = 0.0f;
        }
        
        // Limit values to reasonable ranges
        overshootPct = juce::jlimit(-100.0f, 100.0f, overshootPct);
        preRingDeltaDb = juce::jlimit(-60.0f, 60.0f, preRingDeltaDb);
        riseDeltaUs = juce::jlimit(-1000.0f, 1000.0f, riseDeltaUs);
    }
    
    /**
     * Get overall TEI score based on metrics (0-1)
     * Higher scores indicate better transient preservation
     */
    float getScore() const {
        // Compute score from metrics (lower values for metrics are better)
        const float riseTimeScore = std::exp(-std::abs(riseDeltaUs) / 500.0f);
        const float preRingScore = std::exp(-std::abs(preRingDeltaDb) / 20.0f);
        const float overshootScore = std::exp(-std::abs(overshootPct) / 20.0f);
        
        // Weighted combination
        return (riseTimeScore * 0.4f + preRingScore * 0.4f + overshootScore * 0.2f);
    }
    
private:
    /**
     * Find transient onset in audio buffer
     * 
     * @param buffer Audio buffer to analyze
     * @param transientIndex Output parameter for transient index
     * @param riseTime Output parameter for rise time (in samples)
     */
    void findTransient(const juce::AudioBuffer<float>& buffer, int& transientIndex, float& riseTime) {
        const int numSamples = buffer.getNumSamples();
        const float* data = buffer.getReadPointer(0);  // Use first channel
        
        // Find maximum derivative (steepest increase)
        float maxDerivative = 0.0f;
        transientIndex = 0;
        
        for (int i = 1; i < numSamples - 1; ++i) {
            const float derivative = data[i + 1] - data[i - 1];  // Central difference
            if (derivative > maxDerivative) {
                maxDerivative = derivative;
                transientIndex = i;
            }
        }
        
        // Calculate rise time (10% to 90% of step)
        int startIndex = transientIndex;
        int endIndex = transientIndex;
        
        // Find peak level
        float peakLevel = data[transientIndex];
        for (int i = transientIndex; i < juce::jmin(transientIndex + 20, numSamples); ++i) {
            if (data[i] > peakLevel) {
                peakLevel = data[i];
            }
        }
        
        // Find baseline level
        float baselineLevel = data[juce::jmax(0, transientIndex - 20)];
        for (int i = juce::jmax(0, transientIndex - 20); i < transientIndex; ++i) {
            if (data[i] < baselineLevel) {
                baselineLevel = data[i];
            }
        }
        
        // Calculate 10% and 90% threshold
        const float range = peakLevel - baselineLevel;
        const float threshold10 = baselineLevel + 0.1f * range;
        const float threshold90 = baselineLevel + 0.9f * range;
        
        // Find 10% point
        for (int i = juce::jmax(0, transientIndex - 10); i <= transientIndex; ++i) {
            if (data[i] >= threshold10) {
                startIndex = i;
                break;
            }
        }
        
        // Find 90% point
        for (int i = transientIndex; i < juce::jmin(transientIndex + 10, numSamples); ++i) {
            if (data[i] >= threshold90) {
                endIndex = i;
                break;
            }
        }
        
        // Calculate rise time in samples
        riseTime = static_cast<float>(endIndex - startIndex);
        if (riseTime < 0.1f) riseTime = 0.1f;  // Avoid division by zero
    }
    
    /**
     * Calculate pre-ring energy before transient onset
     * 
     * @param buffer Audio buffer
     * @param transientIndex Index of transient onset
     * @return Pre-ring energy value
     */
    float calculatePreRingEnergy(const juce::AudioBuffer<float>& buffer, int transientIndex) {
        const float* data = buffer.getReadPointer(0);
        const int start = juce::jmax(0, transientIndex - 10);  // 10 samples before transient
        
        float energy = 0.0f;
        
        for (int i = start; i < transientIndex; ++i) {
            energy += data[i] * data[i];
        }
        
        return energy / static_cast<float>(transientIndex - start);
    }
    
    /**
     * Find maximum level after transient onset
     * 
     * @param buffer Audio buffer
     * @param transientIndex Index of transient onset
     * @return Maximum sample value
     */
    float findMaxAfterTransient(const juce::AudioBuffer<float>& buffer, int transientIndex) {
        const float* data = buffer.getReadPointer(0);
        const int numSamples = buffer.getNumSamples();
        const int end = juce::jmin(transientIndex + 20, numSamples);  // 20 samples after transient
        
        float maxValue = 0.0f;
        
        for (int i = transientIndex; i < end; ++i) {
            if (std::abs(data[i]) > maxValue) {
                maxValue = std::abs(data[i]);
            }
        }
        
        return maxValue;
    }
    
    // Instance variables
    double sampleRate = 44100.0;
    double microSecondsPerSample = 22.68;  // 1000000 / 44100
};

}} // namespace moremojo::mlar
