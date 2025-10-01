#include "AudioAnalyzer.h"
#include <juce_events/juce_events.h>
#include <cmath>

AudioAnalyzer::AudioAnalyzer() {
    // Initialize FFT processor
    fft = std::make_unique<juce::dsp::FFT>(juce::roundToInt(std::log2(fftSize)));
    fftData.resize(fftSize * 2, 0.0f);
}

AudioAnalyzer::~AudioAnalyzer() {
}

void AudioAnalyzer::analyzeFile(const juce::File& audioFile, 
                                 std::function<void(bool, const MojoProfile&, const CharacterMetrics&)> callback) {
    
    shouldCancel = false;
    
    // Launch analysis in background thread
    juce::Thread::launch([this, audioFile, callback]() {
        MojoProfile profile{};
        CharacterMetrics metrics{};
        
        // Load audio file
        juce::AudioFormatManager formatManager;
        formatManager.registerBasicFormats();
        
        std::unique_ptr<juce::AudioFormatReader> reader(
            formatManager.createReaderFor(audioFile)
        );
        
        if (!reader || shouldCancel) {
            juce::MessageManager::callAsync([callback, profile, metrics]() {
                callback(false, profile, metrics);
            });
            return;
        }
        
        // Read audio into buffer
        const int numSamples = static_cast<int>(reader->lengthInSamples);
        const int numChannels = static_cast<int>(reader->numChannels);
        const int sampleRate = static_cast<int>(reader->sampleRate);
        
        juce::AudioBuffer<float> buffer(numChannels, numSamples);
        reader->read(&buffer, 0, numSamples, 0, true, true);
        
        if (shouldCancel) {
            juce::MessageManager::callAsync([callback, profile, metrics]() {
                callback(false, profile, metrics);
            });
            return;
        }
        
        // Perform analysis
        performSpectralAnalysis(buffer, profile);
        
        if (shouldCancel) {
            juce::MessageManager::callAsync([callback, profile, metrics]() {
                callback(false, profile, metrics);
            });
            return;
        }
        
        performTransientAnalysis(buffer, profile);
        
        if (shouldCancel) {
            juce::MessageManager::callAsync([callback, profile, metrics]() {
                callback(false, profile, metrics);
            });
            return;
        }
        
        performSpatialAnalysis(buffer, profile);
        
        // Calculate character metrics from profile
        calculateCharacterMetrics(profile, metrics);
        
        // Return results on message thread
        juce::MessageManager::callAsync([callback, profile, metrics]() {
            callback(true, profile, metrics);
        });
    });
}

void AudioAnalyzer::cancelAnalysis() {
    shouldCancel = true;
}

void AudioAnalyzer::performSpectralAnalysis(const juce::AudioBuffer<float>& buffer, MojoProfile& profile) {
    const int numSamples = buffer.getNumSamples();
    const int numChannels = buffer.getNumChannels();
    
    if (numSamples == 0 || numChannels == 0) return;
    
    // Mix to mono for spectral analysis
    std::vector<float> monoBuffer(numSamples, 0.0f);
    for (int ch = 0; ch < numChannels; ++ch) {
        const float* channelData = buffer.getReadPointer(ch);
        for (int i = 0; i < numSamples; ++i) {
            monoBuffer[i] += channelData[i] / numChannels;
        }
    }
    
    // Perform FFT on multiple windows and average
    const int hopSize = fftSize / 4;
    const int numWindows = std::max(1, (numSamples - fftSize) / hopSize);
    
    std::vector<float> avgSpectrum(fftSize / 2, 0.0f);
    
    for (int window = 0; window < numWindows && !shouldCancel; ++window) {
        const int startSample = window * hopSize;
        
        if (startSample + fftSize > numSamples) break;
        
        // Copy data and apply Hann window
        for (int i = 0; i < fftSize; ++i) {
            const float hannWindow = 0.5f * (1.0f - std::cos(2.0f * juce::MathConstants<float>::pi * i / fftSize));
            fftData[i] = monoBuffer[startSample + i] * hannWindow;
        }
        
        // Zero padding
        for (int i = fftSize; i < fftSize * 2; ++i) {
            fftData[i] = 0.0f;
        }
        
        // Perform FFT
        fft->performRealOnlyForwardTransform(fftData.data());
        
        // Accumulate magnitude spectrum
        for (int i = 0; i < fftSize / 2; ++i) {
            const float real = fftData[i * 2];
            const float imag = fftData[i * 2 + 1];
            avgSpectrum[i] += std::sqrt(real * real + imag * imag);
        }
    }
    
    // Average the spectrum
    if (numWindows > 0) {
        for (auto& value : avgSpectrum) {
            value /= numWindows;
        }
    }
    
    // Analyze spectral characteristics
    const int sampleRate = 44100; // Default, could be passed as parameter
    profile.warmth = analyzeWarmth(avgSpectrum, sampleRate);
    profile.shimmer = analyzeShimmer(avgSpectrum, sampleRate);
    profile.clarity = analyzeClarity(avgSpectrum, sampleRate);
    profile.body = analyzeBody(avgSpectrum, sampleRate);
    profile.vintage = analyzeVintage(avgSpectrum, sampleRate);
    profile.vibe = analyzeTonalBalance(avgSpectrum);
}

void AudioAnalyzer::performTransientAnalysis(const juce::AudioBuffer<float>& buffer, MojoProfile& profile) {
    profile.punch = analyzeTransients(buffer);
}

void AudioAnalyzer::performSpatialAnalysis(const juce::AudioBuffer<float>& buffer, MojoProfile& profile) {
    if (buffer.getNumChannels() >= 2) {
        const float stereoWidth = analyzeStereoWidth(buffer);
        const float correlation = analyzePhaseCorrelation(buffer);
        profile.space = (stereoWidth + (1.0f - correlation)) * 0.5f;
    } else {
        profile.space = 0.0f; // Mono has no spatial width
    }
}

void AudioAnalyzer::calculateCharacterMetrics(const MojoProfile& profile, CharacterMetrics& metrics) {
    metrics.warmth = profile.warmth;
    metrics.vintage = profile.vintage;
    metrics.punch = profile.punch;
    metrics.space = profile.space;
    metrics.shimmer = profile.shimmer;
}

// Spectral analysis helpers
float AudioAnalyzer::analyzeWarmth(const std::vector<float>& spectrum, int sampleRate) {
    // Warmth: Energy in 200-800 Hz range
    const int binStart = static_cast<int>((200.0f / sampleRate) * spectrum.size() * 2);
    const int binEnd = static_cast<int>((800.0f / sampleRate) * spectrum.size() * 2);
    
    float warmthEnergy = 0.0f;
    float totalEnergy = 0.0f;
    
    for (size_t i = 0; i < spectrum.size(); ++i) {
        const float energy = spectrum[i] * spectrum[i];
        totalEnergy += energy;
        
        if (i >= binStart && i < binEnd) {
            warmthEnergy += energy;
        }
    }
    
    return totalEnergy > 0.0f ? juce::jlimit(0.0f, 1.0f, warmthEnergy / totalEnergy * 3.0f) : 0.0f;
}

float AudioAnalyzer::analyzeShimmer(const std::vector<float>& spectrum, int sampleRate) {
    // Shimmer: Energy in 8-20 kHz range
    const int binStart = static_cast<int>((8000.0f / sampleRate) * spectrum.size() * 2);
    const int binEnd = static_cast<int>((20000.0f / sampleRate) * spectrum.size() * 2);
    
    float shimmerEnergy = 0.0f;
    float totalEnergy = 0.0f;
    
    for (size_t i = 0; i < spectrum.size(); ++i) {
        const float energy = spectrum[i] * spectrum[i];
        totalEnergy += energy;
        
        if (i >= binStart && i < binEnd) {
            shimmerEnergy += energy;
        }
    }
    
    return totalEnergy > 0.0f ? juce::jlimit(0.0f, 1.0f, shimmerEnergy / totalEnergy * 5.0f) : 0.0f;
}

float AudioAnalyzer::analyzeClarity(const std::vector<float>& spectrum, int sampleRate) {
    // Clarity: Energy in 1-4 kHz range (vocal presence)
    const int binStart = static_cast<int>((1000.0f / sampleRate) * spectrum.size() * 2);
    const int binEnd = static_cast<int>((4000.0f / sampleRate) * spectrum.size() * 2);
    
    float clarityEnergy = 0.0f;
    float totalEnergy = 0.0f;
    
    for (size_t i = 0; i < spectrum.size(); ++i) {
        const float energy = spectrum[i] * spectrum[i];
        totalEnergy += energy;
        
        if (i >= binStart && i < binEnd) {
            clarityEnergy += energy;
        }
    }
    
    return totalEnergy > 0.0f ? juce::jlimit(0.0f, 1.0f, clarityEnergy / totalEnergy * 2.5f) : 0.0f;
}

float AudioAnalyzer::analyzeBody(const std::vector<float>& spectrum, int sampleRate) {
    // Body: Energy in 40-200 Hz range (low-end fullness)
    const int binStart = static_cast<int>((40.0f / sampleRate) * spectrum.size() * 2);
    const int binEnd = static_cast<int>((200.0f / sampleRate) * spectrum.size() * 2);
    
    float bodyEnergy = 0.0f;
    float totalEnergy = 0.0f;
    
    for (size_t i = 0; i < spectrum.size(); ++i) {
        const float energy = spectrum[i] * spectrum[i];
        totalEnergy += energy;
        
        if (i >= binStart && i < binEnd) {
            bodyEnergy += energy;
        }
    }
    
    return totalEnergy > 0.0f ? juce::jlimit(0.0f, 1.0f, bodyEnergy / totalEnergy * 4.0f) : 0.0f;
}

float AudioAnalyzer::analyzeVintage(const std::vector<float>& spectrum, int sampleRate) {
    // Vintage: Harmonic distortion content (odd harmonics, warmth in mids)
    // Detect presence of harmonics by looking for peaks at harmonic intervals
    
    float harmonicContent = 0.0f;
    const int fundamentalBin = static_cast<int>((100.0f / sampleRate) * spectrum.size() * 2);
    
    // Check for harmonic series
    for (int harmonic = 2; harmonic <= 7; ++harmonic) {
        const int harmonicBin = fundamentalBin * harmonic;
        if (harmonicBin < spectrum.size()) {
            harmonicContent += spectrum[harmonicBin];
        }
    }
    
    // Also consider warmth in lower mids (analog character)
    const float warmth = analyzeWarmth(spectrum, sampleRate);
    
    return juce::jlimit(0.0f, 1.0f, (harmonicContent * 0.3f + warmth * 0.7f));
}

float AudioAnalyzer::analyzeTonalBalance(const std::vector<float>& spectrum) {
    // Vibe: Overall tonal balance (how evenly distributed energy is)
    
    // Divide spectrum into octave bands
    const int numBands = 10;
    std::vector<float> bandEnergies(numBands, 0.0f);
    
    for (size_t i = 0; i < spectrum.size(); ++i) {
        const int band = static_cast<int>((std::log2(i + 1) / std::log2(spectrum.size())) * numBands);
        if (band < numBands) {
            bandEnergies[band] += spectrum[i] * spectrum[i];
        }
    }
    
    // Calculate variance (lower variance = better balance = higher vibe)
    float mean = 0.0f;
    for (float energy : bandEnergies) {
        mean += energy;
    }
    mean /= numBands;
    
    float variance = 0.0f;
    for (float energy : bandEnergies) {
        const float diff = energy - mean;
        variance += diff * diff;
    }
    variance /= numBands;
    
    // Normalize to 0-1 (lower variance = higher score)
    const float balance = 1.0f - juce::jlimit(0.0f, 1.0f, std::sqrt(variance) * 0.1f);
    
    return balance;
}

// Transient analysis helpers
float AudioAnalyzer::analyzeTransients(const juce::AudioBuffer<float>& buffer) {
    const int numSamples = buffer.getNumSamples();
    const int numChannels = buffer.getNumChannels();
    
    if (numSamples == 0 || numChannels == 0) return 0.0f;
    
    // Analyze first channel (or mix to mono)
    const float* channelData = buffer.getReadPointer(0);
    
    // Detect transients using envelope follower
    const float attackTime = 0.001f; // 1ms
    const float releaseTime = 0.1f;  // 100ms
    const int sampleRate = 44100;
    
    const float attackCoeff = std::exp(-1.0f / (attackTime * sampleRate));
    const float releaseCoeff = std::exp(-1.0f / (releaseTime * sampleRate));
    
    float envelope = 0.0f;
    float peakEnvelope = 0.0f;
    int transientCount = 0;
    float lastEnvelope = 0.0f;
    
    for (int i = 0; i < numSamples; ++i) {
        const float sample = std::abs(channelData[i]);
        
        // Envelope follower
        if (sample > envelope) {
            envelope = attackCoeff * envelope + (1.0f - attackCoeff) * sample;
        } else {
            envelope = releaseCoeff * envelope + (1.0f - releaseCoeff) * sample;
        }
        
        // Track peak
        if (envelope > peakEnvelope) {
            peakEnvelope = envelope;
        }
        
        // Detect transient (sharp rise in envelope)
        if (envelope > lastEnvelope * 1.5f && envelope > 0.1f) {
            transientCount++;
        }
        
        lastEnvelope = envelope;
    }
    
    // Normalize transient count to 0-1
    const float transientDensity = static_cast<float>(transientCount) / (numSamples / sampleRate);
    const float punch = juce::jlimit(0.0f, 1.0f, transientDensity * 0.1f + peakEnvelope * 0.5f);
    
    return punch;
}

float AudioAnalyzer::calculateEnvelopeFollower(const float* channelData, int numSamples) {
    const float attackTime = 0.01f;
    const float releaseTime = 0.1f;
    const int sampleRate = 44100;
    
    const float attackCoeff = std::exp(-1.0f / (attackTime * sampleRate));
    const float releaseCoeff = std::exp(-1.0f / (releaseTime * sampleRate));
    
    float envelope = 0.0f;
    float peakValue = 0.0f;
    
    for (int i = 0; i < numSamples; ++i) {
        const float sample = std::abs(channelData[i]);
        
        if (sample > envelope) {
            envelope = attackCoeff * envelope + (1.0f - attackCoeff) * sample;
        } else {
            envelope = releaseCoeff * envelope + (1.0f - releaseCoeff) * sample;
        }
        
        if (envelope > peakValue) {
            peakValue = envelope;
        }
    }
    
    return peakValue;
}

// Spatial analysis helpers
float AudioAnalyzer::analyzeStereoWidth(const juce::AudioBuffer<float>& buffer) {
    if (buffer.getNumChannels() < 2) return 0.0f;
    
    const int numSamples = buffer.getNumSamples();
    const float* leftData = buffer.getReadPointer(0);
    const float* rightData = buffer.getReadPointer(1);
    
    float sidePower = 0.0f;
    float midPower = 0.0f;
    
    for (int i = 0; i < numSamples; ++i) {
        const float mid = (leftData[i] + rightData[i]) * 0.5f;
        const float side = (leftData[i] - rightData[i]) * 0.5f;
        
        midPower += mid * mid;
        sidePower += side * side;
    }
    
    const float totalPower = midPower + sidePower;
    if (totalPower == 0.0f) return 0.0f;
    
    return juce::jlimit(0.0f, 1.0f, sidePower / totalPower * 2.0f);
}

float AudioAnalyzer::analyzePhaseCorrelation(const juce::AudioBuffer<float>& buffer) {
    if (buffer.getNumChannels() < 2) return 1.0f;
    
    const int numSamples = buffer.getNumSamples();
    const float* leftData = buffer.getReadPointer(0);
    const float* rightData = buffer.getReadPointer(1);
    
    float correlation = 0.0f;
    float leftPower = 0.0f;
    float rightPower = 0.0f;
    
    for (int i = 0; i < numSamples; ++i) {
        correlation += leftData[i] * rightData[i];
        leftPower += leftData[i] * leftData[i];
        rightPower += rightData[i] * rightData[i];
    }
    
    const float denominator = std::sqrt(leftPower * rightPower);
    if (denominator == 0.0f) return 0.0f;
    
    // Correlation ranges from -1 (out of phase) to +1 (in phase)
    return juce::jlimit(-1.0f, 1.0f, correlation / denominator);
}
