#include "SpectrumAnalyzer.h"
#include <cmath>

SpectrumAnalyzer::SpectrumAnalyzer() : fft(11) // 2^11 = 2048 points
{
    setOpaque(true);
    
    // Initialize FFT data buffers
    fftSize = 2048;
    fftSizeHalf = fftSize / 2;
    
    fftData.resize(fftSize * 2, 0.0f); // Real and imaginary parts
    fftWindow.resize(fftSize, 0.0f);
    averager.resize(fftSizeHalf, 0.0f);
    peaks.resize(fftSizeHalf, 0.0f);
    
    // Create Hann window function for FFT
    createWindow();
    
    // Set up audio input buffer
    inputBuffer.setSize(2, fftSize * 2);
    inputBuffer.clear();
    
    // Start timer for visualization updates
    startTimerHz(30);
}

SpectrumAnalyzer::~SpectrumAnalyzer()
{
    stopTimer();
}

void SpectrumAnalyzer::paint(juce::Graphics& g)
{
    drawBackground(g);
    drawGridLines(g);
    drawSpectrum(g);
    drawTextLabels(g);
}

void SpectrumAnalyzer::resized()
{
    // Nothing special to do here
}

void SpectrumAnalyzer::pushSamples(const float* samples, int numChannels)
{
    if (numChannels == 0)
        return;
    
    // Mix down to mono if stereo
    float monoSample = 0.0f;
    for (int ch = 0; ch < numChannels; ++ch)
        monoSample += samples[ch];
    
    monoSample /= static_cast<float>(numChannels);
    
    // Add to circular buffer
    inputBuffer.setSample(0, inputBufferWritePos, monoSample);
    
    // Increment and wrap write position
    inputBufferWritePos = (inputBufferWritePos + 1) % inputBuffer.getNumSamples();
}

void SpectrumAnalyzer::timerCallback()
{
    // Process FFT when timer fires
    if (inputBuffer.getNumSamples() >= fftSize) {
        // Copy samples from input buffer to FFT buffer
        for (int i = 0; i < fftSize; ++i) {
            int pos = (inputBufferWritePos - fftSize + i + inputBuffer.getNumSamples()) % inputBuffer.getNumSamples();
            
            // Apply window function and copy to FFT buffer
            float sample = inputBuffer.getSample(0, pos) * fftWindow[i];
            
            // Real part
            fftData[i * 2] = sample;
            // Imaginary part
            fftData[i * 2 + 1] = 0.0f;
        }
        
        // Perform FFT
        fft.performFrequencyOnlyForwardTransform(fftData.data());
        
        // Update analyzer data
        for (int i = 0; i < fftSizeHalf; ++i) {
            // Convert to magnitude
            float magnitude = fftData[i];
            
            // Convert to dB scale and normalize
            float dB = juce::jlimit(-100.0f, 0.0f, 
                                     juce::Decibels::gainToDecibels(magnitude) - juce::Decibels::gainToDecibels(0.5f));
            
            // Normalize to 0-1
            float normalizedMagnitude = juce::jmap(dB, -100.0f, 0.0f, 0.0f, 1.0f);
            
            // Apply averaging with previous values
            averager[i] = averager[i] * averaging + normalizedMagnitude * (1.0f - averaging);
            
            // Update peak values
            if (holdPeaks && averager[i] > peaks[i]) {
                peaks[i] = averager[i];
            } else {
                peaks[i] = peaks[i] * peakDecay + averager[i] * (1.0f - peakDecay);
            }
        }
        
        // Trigger repaint to show new data
        repaint();
    }
}

void SpectrumAnalyzer::setFFTSize(int size)
{
    // FFT size must be a power of 2
    if (size != fftSize && juce::isPowerOfTwo(size)) {
        fftSize = size;
        fftSizeHalf = fftSize / 2;
        
        // Update FFT processor (order = log2(N))
        fft = juce::dsp::FFT(static_cast<int>(std::log2(static_cast<double>(fftSize))));
        
        // Resize data buffers
        fftData.resize(fftSize * 2, 0.0f);
        fftWindow.resize(fftSize, 0.0f);
        averager.resize(fftSizeHalf, 0.0f);
        peaks.resize(fftSizeHalf, 0.0f);
        
        // Create new window function
        createWindow();
        
        // Resize audio buffer
        inputBuffer.setSize(2, fftSize * 2);
        inputBuffer.clear();
        inputBufferWritePos = 0;
    }
}

void SpectrumAnalyzer::setAveraging(float newAveraging)
{
    averaging = juce::jlimit(0.0f, 0.99f, newAveraging);
}

void SpectrumAnalyzer::setDecayRate(float newDecayRate)
{
    decayRate = juce::jlimit(0.0f, 0.99f, newDecayRate);
}

void SpectrumAnalyzer::setPeakHold(bool shouldHoldPeaks)
{
    holdPeaks = shouldHoldPeaks;
}

void SpectrumAnalyzer::setPeakDecay(float newPeakDecay)
{
    peakDecay = juce::jlimit(0.0f, 0.99f, newPeakDecay);
}

void SpectrumAnalyzer::drawBackground(juce::Graphics& g)
{
    auto bounds = getLocalBounds().toFloat();
    
    // Draw gradient background
    g.setGradientFill(juce::ColourGradient(
        juce::Colour(0xff141414), bounds.getX(), bounds.getY(),
        juce::Colour(0xff0a0a0a), bounds.getX(), bounds.getBottom(),
        false));
    g.fillRect(bounds);
}

void SpectrumAnalyzer::drawGridLines(juce::Graphics& g)
{
    auto bounds = getLocalBounds().toFloat();
    float width = bounds.getWidth();
    float height = bounds.getHeight();
    
    g.setColour(gridColour);
    
    // Vertical grid lines (octaves)
    for (float freq = 31.25f; freq < 20000.0f; freq *= 2.0f) {
        float x = getXForFrequency(freq, width);
        g.drawVerticalLine(juce::roundToInt(x), bounds.getY(), bounds.getBottom());
    }
    
    // Horizontal grid lines (amplitude)
    const int numHorizontalLines = 5;
    for (int i = 0; i <= numHorizontalLines; ++i) {
        float normalizedAmplitude = static_cast<float>(i) / numHorizontalLines;
        float y = getYForAmplitude(normalizedAmplitude, height);
        g.drawHorizontalLine(juce::roundToInt(y), bounds.getX(), bounds.getRight());
    }
}

void SpectrumAnalyzer::drawTextLabels(juce::Graphics& g)
{
    auto bounds = getLocalBounds().toFloat();
    float width = bounds.getWidth();
    float height = bounds.getHeight();
    
    g.setColour(juce::Colours::lightgrey);
    g.setFont(12.0f);
    
    // Frequency labels
    const float frequencies[] = { 20.0f, 50.0f, 100.0f, 200.0f, 500.0f, 
                                1000.0f, 2000.0f, 5000.0f, 10000.0f, 20000.0f };
    
    for (auto freq : frequencies) {
        float x = getXForFrequency(freq, width);
        
        // Format frequency text
        juce::String freqText;
        if (freq >= 1000.0f)
            freqText = juce::String(freq / 1000.0f) + "k";
        else
            freqText = juce::String(freq);
        
        g.drawText(freqText, juce::roundToInt(x) - 10, bounds.getBottom() - 16, 20, 12,
                  juce::Justification::centred, false);
    }
    
    // Amplitude labels (dB)
    g.setColour(juce::Colours::lightgrey);
    
    const int numAmplitudeLabels = 5;
    for (int i = 0; i <= numAmplitudeLabels; ++i) {
        float normalizedAmplitude = static_cast<float>(i) / numAmplitudeLabels;
        float dB = juce::jmap(normalizedAmplitude, 0.0f, 1.0f, -100.0f, 0.0f);
        float y = getYForAmplitude(normalizedAmplitude, height);
        
        juce::String dbText = juce::String(juce::roundToInt(dB)) + " dB";
        g.drawText(dbText, bounds.getX() + 2, juce::roundToInt(y) - 6, 40, 12,
                  juce::Justification::left, false);
    }
}

void SpectrumAnalyzer::drawSpectrum(juce::Graphics& g)
{
    auto bounds = getLocalBounds().toFloat();
    float width = bounds.getWidth();
    float height = bounds.getHeight();
    
    // Draw spectrum path
    juce::Path spectrumPath;
    bool isFirstPoint = true;
    
    for (int i = 0; i < fftSizeHalf; ++i) {
        // Map frequency bin to x coordinate (logarithmic)
        float binFrequency = static_cast<float>(i) * 44100.0f / static_cast<float>(fftSize);
        float x = getXForFrequency(binFrequency, width);
        
        // Map amplitude to y coordinate
        float y = getYForAmplitude(averager[i], height);
        
        if (isFirstPoint) {
            spectrumPath.startNewSubPath(x, y);
            isFirstPoint = false;
        }
        else {
            spectrumPath.lineTo(x, y);
        }
    }
    
    // Close the path at the bottom for filling
    spectrumPath.lineTo(width, height);
    spectrumPath.lineTo(0, height);
    spectrumPath.closeSubPath();
    
    // Fill with gradient
    g.setGradientFill(juce::ColourGradient(
        mainGradientColourTop, 0, 0,
        mainGradientColourBottom, 0, height,
        false));
    g.fillPath(spectrumPath);
    
    // Draw peaks
    g.setColour(peakColour);
    
    for (int i = 1; i < fftSizeHalf; ++i) {
        // Only draw if we have valid data
        if (peaks[i] <= 0.0f)
            continue;
            
        // Map frequency bin to x coordinate
        float binFrequency = static_cast<float>(i) * 44100.0f / static_cast<float>(fftSize);
        float x = getXForFrequency(binFrequency, width);
        
        // Map amplitude to y coordinate
        float y = getYForAmplitude(peaks[i], height);
        
        // Draw peak line
        g.drawHorizontalLine(juce::roundToInt(y), x - 1.0f, x + 1.0f);
    }
}

void SpectrumAnalyzer::createWindow()
{
    // Generate Hann window for smoothing
    for (int i = 0; i < fftSize; ++i)
        fftWindow[i] = 0.5f - 0.5f * std::cos(2.0f * juce::MathConstants<float>::pi * i / static_cast<float>(fftSize - 1));
}

float SpectrumAnalyzer::getXForFrequency(float frequency, float width)
{
    // Log scale mapping from 20Hz to 20kHz
    constexpr float minFreq = 20.0f;
    constexpr float maxFreq = 20000.0f;
    
    // Constrain frequency to valid range
    frequency = juce::jlimit(minFreq, maxFreq, frequency);
    
    // Convert to log scale and normalize to 0-1
    float normalisedX = std::log10(frequency / minFreq) / std::log10(maxFreq / minFreq);
    
    // Map to pixel position
    return width * normalisedX;
}

float SpectrumAnalyzer::getYForAmplitude(float amplitude, float height)
{
    // Invert Y since higher amplitude = lower Y position
    return height * (1.0f - amplitude);
}
