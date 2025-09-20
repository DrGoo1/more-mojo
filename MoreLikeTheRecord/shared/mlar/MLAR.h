#pragma once
/**
 * MLAR.h - Umbrella header for "More Like The Record" module
 * 
 * This header provides simplified access to the MLAR module's
 * components for integration into the larger MoreMojo app/plugin.
 */

#include "dsp/TimingSafeResampler.h"
#include "dsp/TransientDetect.h"
#include "dsp/FractionalDelayAlign.h"
#include "dsp/AnalogContinuity.h"
#include "dsp/LimiterLevelMatch.h"
#include "metrics/GDSComputer.h"
#include "metrics/IACCComputer.h"
#include "metrics/TEIAnalyzer.h"
#include "metrics/ResidualNull.h"

namespace moremojo {
namespace mlar {

/**
 * MetricsFrame - Container for metrics data at a point in time
 */
struct MetricsFrame {
    // True peak level
    float truePeak{0.0f};
    
    // Interaural coherence value
    float iacc{0.0f};
    
    // Interaural time difference variance
    float itdVar{0.0f};
    
    // Transient Edge Integrity rise time in microseconds
    float teiRiseUs{0.0f};
    
    // IACC timeline data
    std::vector<float> iaccLine;
    
    // Group delay spread heatmap dimensions
    int gdsWidth{0};
    int gdsHeight{0};
    
    // Group delay spread heatmap data
    std::vector<float> gdsData;
};

/**
 * Processor - Main MLAR processing class
 * 
 * Provides a simplified interface for the entire MLAR processing chain
 * to integrate easily with the existing MoreMojo processor.
 */
class Processor {
public:
    Processor() = default;
    
    /**
     * Prepare the processor for playback
     * 
     * @param sampleRate The audio sample rate
     * @param blockSize Maximum expected block size
     */
    void prepare(double sampleRate, int blockSize) {
        resampler.prepare(sampleRate, blockSize);
        transient.prepare(sampleRate, blockSize);
        align.prepare(sampleRate, blockSize);
        analog.prepare(sampleRate, blockSize);
        limiter.prepare(sampleRate, blockSize);
        
        gds.prepare(sampleRate);
        iacc.prepare(2048, 1024);
        tei.prepare(sampleRate);
        
        // Initialize buffers for pre/post processing
        preBuf.setSize(2, blockSize);
        postBuf.setSize(2, blockSize);
    }
    
    /**
     * Reset processor state
     */
    void reset() {
        resampler.reset();
        transient.reset();
        align.reset();
        analog.reset();
        limiter.reset();
    }
    
    /**
     * Process an audio buffer through the MLAR chain
     * 
     * @param buffer Audio buffer to process
     * @param apvts AudioProcessorValueTreeState containing MLAR parameters
     */
    void process(juce::AudioBuffer<float>& buffer, juce::AudioProcessorValueTreeState& apvts) {
        // Store pre-processing buffer
        preBuf.makeCopyOf(buffer);
        
        // Process each module in sequence (if enabled)
        if (apvts.getRawParameterValue("mlar_resampler_on")->load() > 0.5f) {
            resampler.process(buffer, apvts);
        }
        
        if (apvts.getRawParameterValue("mlar_transient_on")->load() > 0.5f) {
            transient.process(buffer, apvts);
        }
        
        if (apvts.getRawParameterValue("mlar_align_on")->load() > 0.5f) {
            align.process(buffer, apvts);
        }
        
        if (apvts.getRawParameterValue("mlar_analog_on")->load() > 0.5f) {
            analog.process(buffer, apvts);
        }
        
        // Limiter is always active for safety
        float ceiling = apvts.getRawParameterValue("mlar_limit_ceiling")->load();
        limiter.process(buffer, ceiling);
        
        // Store post-processing buffer
        postBuf.makeCopyOf(buffer);
        
        // Analyze transient edge integrity
        tei.analyze(preBuf, postBuf);
    }
    
    /**
     * Snapshot current metrics into a MetricsFrame
     * 
     * @param frame Output frame for metrics
     */
    void snapshotMetrics(MetricsFrame& frame) {
        // Get true peak level
        frame.truePeak = limiter.truePeak();
        
        // Compute IACC and timeline
        std::vector<float> line;
        iacc.compute(postBuf, line);
        
        // Store IACC value and line
        frame.iacc = line.empty() ? 1.0f : line.back();
        frame.iaccLine = std::move(line);
        
        // Compute GDS heatmap
        frame.gdsData = gds.compute(postBuf, frame.gdsWidth, frame.gdsHeight);
        
        // Get ITD variance
        frame.itdVar = align.itdVar.load();
        
        // Get TEI rise time
        frame.teiRiseUs = tei.riseDeltaUs;
    }
    
    /**
     * Get the current processing latency in samples
     */
    int latencySamples() const {
        return static_cast<int>(limiter.getLookAheadSamples());
    }
    
    /**
     * Access the pre-processing buffer
     */
    const juce::AudioBuffer<float>& getPreBuffer() const {
        return preBuf;
    }
    
    /**
     * Access the post-processing buffer
     */
    const juce::AudioBuffer<float>& getPostBuffer() const {
        return postBuf;
    }
    
    /**
     * Create parameter layout for MLAR
     * 
     * @return Parameter layout for MLAR module
     */
    static juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout() {
        using Range = juce::NormalisableRange<float>;
        
        std::vector<std::unique_ptr<juce::RangedAudioParameter>> params;
        
        // Main macro control
        params.push_back(std::make_unique<juce::AudioParameterFloat>(
            "mlar_macro", "More Like The Record", Range(0.0f, 100.0f), 0.0f));
        
        // Resampler parameters
        params.push_back(std::make_unique<juce::AudioParameterBool>(
            "mlar_resampler_on", "Resampler On", true));
        params.push_back(std::make_unique<juce::AudioParameterBool>(
            "mlar_interp", "Intersample Interpolation", true));
        params.push_back(std::make_unique<juce::AudioParameterFloat>(
            "mlar_resampler_quality", "Resampler Quality", Range(0.0f, 100.0f), 80.0f));
        params.push_back(std::make_unique<juce::AudioParameterBool>(
            "mlar_resampler_dither", "Resampler Dither", false));
        params.push_back(std::make_unique<juce::AudioParameterFloat>(
            "mlar_resampler_isp_margin", "ISP Margin dB", Range(-2.0f, 2.0f, 0.01f), 0.5f));
            
        // Reconstruction Filter parameters
        params.push_back(std::make_unique<juce::AudioParameterInt>(
            "mlar_rf_taps", "RF Taps", 63, 511, 127));
        params.push_back(std::make_unique<juce::AudioParameterChoice>(
            "mlar_rf_window", "RF Window", juce::StringArray{"Blackman-Harris", "Kaiser", "Hann"}, 0));
        params.push_back(std::make_unique<juce::AudioParameterFloat>(
            "mlar_rf_ripple", "RF Ripple dB", Range(0.01f, 0.10f, 0.01f), 0.02f));
            
        // Jitter & Accumulator parameters
        params.push_back(std::make_unique<juce::AudioParameterBool>(
            "mlar_jitter_on", "Jitter Control", false));
        params.push_back(std::make_unique<juce::AudioParameterChoice>(
            "mlar_accum_precision", "Accumulator Precision", juce::StringArray{"Auto", "High"}, 0));
        
        // Transient detector parameters
        params.push_back(std::make_unique<juce::AudioParameterBool>(
            "mlar_transient_on", "Transient On", true));
        params.push_back(std::make_unique<juce::AudioParameterFloat>(
            "mlar_transient_sense", "Transient Sens", Range(0.0f, 100.0f), 50.0f));
        params.push_back(std::make_unique<juce::AudioParameterFloat>(
            "mlar_transient_bands", "Transient Bands", Range(2.0f, 8.0f), 6.0f));
        
        // Alignment parameters
        params.push_back(std::make_unique<juce::AudioParameterBool>(
            "mlar_align_on", "Align On", true));
        params.push_back(std::make_unique<juce::AudioParameterFloat>(
            "mlar_align_amount", "Align Amount", Range(0.0f, 100.0f), 50.0f));
        params.push_back(std::make_unique<juce::AudioParameterFloat>(
            "mlar_align_bands", "Align Bands", Range(1.0f, 6.0f), 6.0f));
        // Per-band delay parameters (6 bands max)
        for (int i = 0; i < 6; ++i) {
            params.push_back(std::make_unique<juce::AudioParameterFloat>(
                "mlar_band" + juce::String(i) + "_delay", "Band " + juce::String(i+1) + " Delay", 
                Range(-0.6f, 0.6f), 0.0f));
        }
        
        // Analog parameters
        params.push_back(std::make_unique<juce::AudioParameterBool>(
            "mlar_analog_on", "Analog On", false));
        params.push_back(std::make_unique<juce::AudioParameterFloat>(
            "mlar_analog_mix", "Analog Mix", Range(0.0f, 100.0f), 20.0f));
        
        // Limiter parameters
        params.push_back(std::make_unique<juce::AudioParameterFloat>(
            "mlar_limit_ceiling", "Limiter Ceiling dB", Range(-6.0f, 0.0f), -0.3f));
        params.push_back(std::make_unique<juce::AudioParameterFloat>(
            "mlar_look_ahead", "Look-ahead (ms)", Range(0.0f, 5.0f), 1.0f));
        params.push_back(std::make_unique<juce::AudioParameterFloat>(
            "mlar_target_lufs", "Target LUFS", Range(-24.0f, -8.0f, 0.1f), -14.0f));
            
        // Residual/Null parameters
        params.push_back(std::make_unique<juce::AudioParameterBool>(
            "mlar_residual_on", "Residual On", false));
        params.push_back(std::make_unique<juce::AudioParameterFloat>(
            "mlar_residual_gain", "Residual Gain dB", Range(0.0f, 12.0f, 0.1f), 6.0f));
        
        return { params.begin(), params.end() };
    }
    
    // Direct access to DSP modules (for metrics and parameter access)
    TimingSafeResampler resampler;
    TransientDetect transient;
    FractionalDelayAlign align;
    AnalogContinuity analog;
    LimiterLevelMatch limiter;
    
    // Metrics analyzers
    GDSComputer gds;
    IACCComputer iacc;
    TEIAnalyzer tei;
    
private:
    // Pre/post processing buffers for analysis
    juce::AudioBuffer<float> preBuf;
    juce::AudioBuffer<float> postBuf;
};

} // namespace mlar
} // namespace moremojo
