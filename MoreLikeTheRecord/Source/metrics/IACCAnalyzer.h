#pragma once
#include <JuceHeader.h>
#include <vector>

/**
 * IACCAnalyzer - Interaural Cross-Correlation Analyzer
 * 
 * Calculates correlation between left and right channels over time
 * for stereo image stability measurement
 */
class IACCAnalyzer {
public:
    IACCAnalyzer() = default;
    
    /**
     * Prepare the analyzer with configuration
     * 
     * @param frameSize Size of analysis frames
     * @param hopSize Hop size between frames
     */
    void prepare(int frameSize, int hopSize) {
        this->frameSize = frameSize;
        this->hopSize = hopSize;
        
        // Initialize buffers
        leftBuffer.resize(frameSize, 0.0f);
        rightBuffer.resize(frameSize, 0.0f);
        
        // Initialize history
        iaccHistory.clear();
        
        // Pre-allocate some history
        iaccHistory.reserve(100);
    }
    
    /**
     * Reset the analyzer state
     */
    void reset() {
        std::fill(leftBuffer.begin(), leftBuffer.end(), 0.0f);
        std::fill(rightBuffer.begin(), rightBuffer.end(), 0.0f);
        
        iaccHistory.clear();
        bufferPos = 0;
    }
    
    /**
     * Compute IACC timeline from a stereo buffer
     * 
     * @param buffer Stereo audio buffer to analyze
     * @param timeline Output vector for IACC timeline
     */
    void compute(const juce::AudioBuffer<float>& buffer, std::vector<float>& timeline) {
        // Need stereo for IACC
        if (buffer.getNumChannels() < 2) {
            timeline = iaccHistory;
            return;
        }
        
        const float* leftData = buffer.getReadPointer(0);
        const float* rightData = buffer.getReadPointer(1);
        const int numSamples = buffer.getNumSamples();
        
        // Process samples in blocks
        for (int i = 0; i < numSamples; ++i) {
            // Store samples in buffers
            leftBuffer[bufferPos] = leftData[i];
            rightBuffer[bufferPos] = rightData[i];
            
            // Increment buffer position
            ++bufferPos;
            
            // Process a frame when we've collected enough samples
            if (bufferPos >= frameSize) {
                processFrame();
                
                // Reset buffer position with overlap
                bufferPos = frameSize - hopSize;
                
                // Shift samples for overlap
                for (int j = 0; j < hopSize; ++j) {
                    leftBuffer[j] = leftBuffer[j + hopSize];
                    rightBuffer[j] = rightBuffer[j + hopSize];
                }
            }
        }
        
        // Return the IACC history
        timeline = iaccHistory;
        
        // Limit history size
        const int maxHistory = 240; // Maximum history to maintain
        if (iaccHistory.size() > maxHistory) {
            iaccHistory.erase(iaccHistory.begin(), iaccHistory.begin() + (iaccHistory.size() - maxHistory));
        }
    }
    
    /**
     * Get the latest IACC value
     */
    float getLatestIACC() const {
        if (!iaccHistory.empty()) {
            return iaccHistory.back();
        }
        return 0.0f;
    }
    
    /**
     * Get the IACC variance (stability measure)
     */
    float getIACCVariance() const {
        if (iaccHistory.size() < 2) {
            return 0.0f;
        }
        
        // Calculate mean
        float sum = 0.0f;
        for (float iacc : iaccHistory) {
            sum += iacc;
        }
        float mean = sum / static_cast<float>(iaccHistory.size());
        
        // Calculate variance
        float sumSq = 0.0f;
        for (float iacc : iaccHistory) {
            float diff = iacc - mean;
            sumSq += diff * diff;
        }
        
        return std::sqrt(sumSq / static_cast<float>(iaccHistory.size()));
    }
    
private:
    // Process a complete frame of audio
    void processFrame() {
        // Calculate IACC
        float sumLR = 0.0f;
        float sumL2 = 0.0f;
        float sumR2 = 0.0f;
        
        for (int i = 0; i < frameSize; ++i) {
            sumLR += leftBuffer[i] * rightBuffer[i];
            sumL2 += leftBuffer[i] * leftBuffer[i];
            sumR2 += rightBuffer[i] * rightBuffer[i];
        }
        
        // Normalize correlation
        float iacc = 0.0f;
        float denominator = std::sqrt(sumL2 * sumR2);
        
        if (denominator > 1.0e-8f) {
            iacc = sumLR / denominator;
        }
        
        // Limit to valid range [-1, 1]
        iacc = juce::jlimit(-1.0f, 1.0f, iacc);
        
        // Store in history
        iaccHistory.push_back(iacc);
    }
    
    // Instance variables
    int frameSize = 1024;
    int hopSize = 512;
    int bufferPos = 0;
    
    std::vector<float> leftBuffer;
    std::vector<float> rightBuffer;
    std::vector<float> iaccHistory;
};
