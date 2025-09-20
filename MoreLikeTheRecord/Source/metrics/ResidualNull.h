#pragma once
#include <JuceHeader.h>

/**
 * ResidualNull - Utility for creating residual signals by subtracting audio buffers
 */
struct ResidualNull {
    /**
     * Make residual signal by subtracting buffer A from buffer B
     * 
     * @param a Input buffer A
     * @param b Input buffer B
     * @param out Output buffer (B-A)
     */
    static void makeResidual(const juce::AudioBuffer<float>& a, 
                            const juce::AudioBuffer<float>& b, 
                            juce::AudioBuffer<float>& out) {
        // Copy B to output
        out.makeCopyOf(b);
        
        // Subtract A from output (B-A)
        for (int ch = 0; ch < out.getNumChannels(); ++ch) {
            for (int i = 0; i < out.getNumSamples(); ++i) {
                out.setSample(ch, i, b.getSample(ch, i) - a.getSample(ch, i));
            }
        }
    }
};
