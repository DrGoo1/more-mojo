#include "ProEQComponent.h"

ProEQComponent::ProEQComponent()
{
    // Initialize spectrum analyzer
    spectrumAnalyzer.reset(new SpectrumAnalyzer());
    addAndMakeVisible(spectrumAnalyzer.get());
    
    // Set up band colors
    bandColours.add(juce::Colour(0xff4285f4)); // Blue
    bandColours.add(juce::Colour(0xffea4335)); // Red
    bandColours.add(juce::Colour(0xfffbbc05)); // Yellow
    bandColours.add(juce::Colour(0xff34a853)); // Green
    bandColours.add(juce::Colour(0xffbb86fc)); // Purple
    
    // Create EQ bands
    createBands();
    
    // Start timer for visual updates
    startTimerHz(30);
}

// ===== External Control API =====
void ProEQComponent::setBandGain(int bandIndex, float gainDB)
{
    if (bandIndex < 0 || bandIndex >= numBands) return;
    auto& band = bands[bandIndex];
    band.gain = gainDB;
    if (band.gainSlider)
        band.gainSlider->setValue(gainDB, juce::sendNotificationSync);
    updateResponseCurve();
    repaint();
}

void ProEQComponent::setBandFrequency(int bandIndex, float frequency)
{
    if (bandIndex < 0 || bandIndex >= numBands) return;
    auto& band = bands[bandIndex];
    band.frequency = frequency;
    if (band.frequencySlider)
        band.frequencySlider->setValue(frequency, juce::sendNotificationSync);
    updateResponseCurve();
    repaint();
}

void ProEQComponent::setBandQ(int bandIndex, float q)
{
    if (bandIndex < 0 || bandIndex >= numBands) return;
    auto& band = bands[bandIndex];
    band.q = q;
    if (band.qSlider)
        band.qSlider->setValue(q, juce::sendNotificationSync);
    updateResponseCurve();
    repaint();
}

float ProEQComponent::getBandGain(int bandIndex) const
{
    if (bandIndex < 0 || bandIndex >= numBands) return 0.0f;
    return bands[bandIndex].gain;
}

float ProEQComponent::getBandFrequency(int bandIndex) const
{
    if (bandIndex < 0 || bandIndex >= numBands) return 0.0f;
    return bands[bandIndex].frequency;
}

float ProEQComponent::getBandQ(int bandIndex) const
{
    if (bandIndex < 0 || bandIndex >= numBands) return 0.0f;
    return bands[bandIndex].q;
}

void ProEQComponent::processAudio(juce::AudioBuffer<float>& buffer)
{
    // Feed the spectrum analyzer with downsampled frames from the buffer
    const int numChannels = buffer.getNumChannels();
    const int numSamples = buffer.getNumSamples();
    const int step = 1; // use every sample; adjust if needed
    for (int i = 0; i < numSamples; i += step)
    {
        // Collect one frame of samples across channels
        float frame[8] = {0};
        const int chCount = juce::jmin(numChannels, 8);
        for (int ch = 0; ch < chCount; ++ch)
            frame[ch] = buffer.getReadPointer(ch)[i];
        spectrumAnalyzer->pushSamples(frame, chCount);
    }
}

ProEQComponent::~ProEQComponent()
{
    stopTimer();
}

void ProEQComponent::paint(juce::Graphics& g)
{
    // Draw professional dark background
    drawBackground(g);
    
    // Draw frequency and gain scales
    drawFrequencyScale(g);
    drawGainScale(g);
    
    // Draw grid lines for frequency markers
    drawFrequencyMarkers(g);
    
    // Draw response curve
    drawResponseCurve(g);
}

void ProEQComponent::resized()
{
    // Calculate area for frequency response display
    auto area = getLocalBounds().reduced(10);
    
    // Top area for spectrum analyzer
    auto spectrumArea = area.removeFromTop(area.getHeight() * 0.6);
    spectrumAnalyzer->setBounds(spectrumArea);
    
    // Bottom area for EQ bands
    auto eqControlsArea = area;
    
    // Define frequency response area for reference in drawing
    frequencyResponseArea = spectrumArea.reduced(2);
    
    // Layout EQ band controls horizontally
    int bandWidth = eqControlsArea.getWidth() / numBands;
    
    for (int i = 0; i < numBands; ++i) {
        auto& band = bands[i];
        auto bandArea = eqControlsArea.removeFromLeft(bandWidth);
        
        // Layout vertically
        auto labelArea = bandArea.removeFromTop(20);
        band.nameLabel.setBounds(labelArea);
        
        auto typeArea = bandArea.removeFromTop(25);
        band.typeSelector->setBounds(typeArea.reduced(2));
        
        auto freqArea = bandArea.removeFromTop(80);
        band.frequencySlider->setBounds(freqArea.reduced(5));
        
        auto gainArea = bandArea.removeFromTop(80);
        band.gainSlider->setBounds(gainArea.reduced(5));
        
        auto qArea = bandArea.removeFromTop(80);
        band.qSlider->setBounds(qArea.reduced(5));
    }
    
    // Update response curve
    updateResponseCurve();
}

void ProEQComponent::timerCallback()
{
    // Update spectrum analyzer
    spectrumAnalyzer->repaint();
    
    // Update response curve if needed
    bool needsUpdate = false;
    
    for (auto& band : bands) {
        // Check if any control has changed
        if (band.frequency != static_cast<float>(band.frequencySlider->getValue()) ||
            band.gain != static_cast<float>(band.gainSlider->getValue()) ||
            band.q != static_cast<float>(band.qSlider->getValue()) ||
            band.type != band.typeSelector->getSelectedId() - 1) {
            
            // Update band parameters
            band.frequency = static_cast<float>(band.frequencySlider->getValue());
            band.gain = static_cast<float>(band.gainSlider->getValue());
            band.q = static_cast<float>(band.qSlider->getValue());
            band.type = band.typeSelector->getSelectedId() - 1;
            
            needsUpdate = true;
        }
    }
    
    if (needsUpdate) {
        updateResponseCurve();
        repaint();
    }
}

void ProEQComponent::pushNextSampleIntoAnalyzer(const float* inputChannelData, int numChannels)
{
    // Forward audio data to spectrum analyzer
    spectrumAnalyzer->pushSamples(inputChannelData, numChannels);
}

void ProEQComponent::createBands()
{
    // Default band settings for a 5-band EQ
    static const struct {
        const char* name;
        float defaultFreq;
        float defaultGain;
        float defaultQ;
        int defaultType;
    } bandDefaults[numBands] = {
        { "Low", 100.0f, 0.0f, 0.7f, 1 },     // Low Shelf
        { "Low Mid", 300.0f, 0.0f, 1.0f, 0 }, // Bell
        { "Mid", 1000.0f, 0.0f, 1.0f, 0 },    // Bell
        { "High Mid", 3000.0f, 0.0f, 1.0f, 0 },// Bell
        { "High", 10000.0f, 0.0f, 0.7f, 2 }   // High Shelf
    };
    
    // Create sliders and controls for each band
    for (int i = 0; i < numBands; ++i) {
        auto& band = bands[i];
        band.colour = bandColours[i];
        
        // Create and set up frequency slider
        band.frequencySlider.reset(new juce::Slider(juce::Slider::RotaryHorizontalVerticalDrag, juce::Slider::TextBoxBelow));
        band.frequencySlider->setTextValueSuffix(" Hz");
        band.frequencySlider->setRange(20.0, 20000.0, 1.0);
        band.frequencySlider->setSkewFactorFromMidPoint(1000.0);
        band.frequencySlider->setValue(bandDefaults[i].defaultFreq, juce::dontSendNotification);
        band.frequencySlider->setTextBoxStyle(juce::Slider::TextBoxBelow, false, 80, 20);
        band.frequencySlider->setColour(juce::Slider::rotarySliderFillColourId, band.colour);
        band.frequencySlider->setTooltip("Frequency");
        addAndMakeVisible(band.frequencySlider.get());
        
        // Create and set up gain slider
        band.gainSlider.reset(new juce::Slider(juce::Slider::RotaryHorizontalVerticalDrag, juce::Slider::TextBoxBelow));
        band.gainSlider->setTextValueSuffix(" dB");
        band.gainSlider->setRange(-24.0, 24.0, 0.1);
        band.gainSlider->setValue(bandDefaults[i].defaultGain, juce::dontSendNotification);
        band.gainSlider->setTextBoxStyle(juce::Slider::TextBoxBelow, false, 80, 20);
        band.gainSlider->setColour(juce::Slider::rotarySliderFillColourId, band.colour);
        band.gainSlider->setTooltip("Gain");
        addAndMakeVisible(band.gainSlider.get());
        
        // Create and set up Q slider
        band.qSlider.reset(new juce::Slider(juce::Slider::RotaryHorizontalVerticalDrag, juce::Slider::TextBoxBelow));
        band.qSlider->setTextValueSuffix("");
        band.qSlider->setRange(0.1, 10.0, 0.01);
        band.qSlider->setSkewFactorFromMidPoint(1.0);
        band.qSlider->setValue(bandDefaults[i].defaultQ, juce::dontSendNotification);
        band.qSlider->setTextBoxStyle(juce::Slider::TextBoxBelow, false, 80, 20);
        band.qSlider->setColour(juce::Slider::rotarySliderFillColourId, band.colour);
        band.qSlider->setTooltip("Q");
        addAndMakeVisible(band.qSlider.get());
        
        // Create and set up band type selector
        band.typeSelector.reset(new juce::ComboBox());
        band.typeSelector->addItem("Bell", 1);
        band.typeSelector->addItem("Low Shelf", 2);
        band.typeSelector->addItem("High Shelf", 3);
        band.typeSelector->addItem("Low Pass", 4);
        band.typeSelector->addItem("High Pass", 5);
        band.typeSelector->addItem("Notch", 6);
        band.typeSelector->setSelectedId(bandDefaults[i].defaultType + 1, juce::dontSendNotification);
        band.typeSelector->setColour(juce::ComboBox::backgroundColourId, juce::Colour(0xff333333));
        band.typeSelector->setColour(juce::ComboBox::textColourId, juce::Colours::white);
        band.typeSelector->setTooltip("Filter Type");
        addAndMakeVisible(band.typeSelector.get());
        
        // Set up band name label
        band.nameLabel.setText(bandDefaults[i].name, juce::dontSendNotification);
        band.nameLabel.setFont(juce::Font(14.0f, juce::Font::bold));
        band.nameLabel.setJustificationType(juce::Justification::centred);
        band.nameLabel.setColour(juce::Label::textColourId, band.colour);
        addAndMakeVisible(band.nameLabel);
        
        // Store initial values
        band.frequency = bandDefaults[i].defaultFreq;
        band.gain = bandDefaults[i].defaultGain;
        band.q = bandDefaults[i].defaultQ;
        band.type = bandDefaults[i].defaultType;
    }
    
    // Initialize response curve
    updateResponseCurve();
}

void ProEQComponent::updateResponseCurve()
{
    auto responseArea = frequencyResponseArea.toFloat();
    
    responseCurve.clear();
    
    const int numPoints = 200;
    
    for (int i = 0; i < numPoints; ++i) {
        // Map x position to frequency using logarithmic scale
        float proportion = static_cast<float>(i) / static_cast<float>(numPoints - 1);
        float freq = 20.0f * std::pow(1000.0f, proportion);
        
        // Calculate combined magnitude response in dB
        float magnitudeDB = 0.0f;
        
        for (const auto& band : bands) {
            if (band.isActive) {
                magnitudeDB += getFilterMagnitudeForFrequency(band, freq);
            }
        }
        
        // Map to y position
        float x = getXForFrequency(freq);
        float y = getYForGain(magnitudeDB);
        
        if (i == 0)
            responseCurve.startNewSubPath(x, y);
        else
            responseCurve.lineTo(x, y);
    }
}

float ProEQComponent::getFilterMagnitudeForFrequency(const EQBand& band, float frequency)
{
    // Simple magnitude response approximations for different filter types
    
    // Scale for octave distance calculation
    float octaveDistance = std::log2(frequency / band.frequency);
    
    switch (band.type) {
        case 0: // Bell
        {
            // Bell filter response approximation
            float response = band.gain / (1.0f + std::pow(octaveDistance * band.q, 2));
            return response;
        }
        
        case 1: // Low Shelf
        {
            if (frequency < band.frequency)
                return band.gain;
            
            // Transition slope based on Q
            float transition = band.gain / (1.0f + std::pow(octaveDistance * band.q * 2.0f, 2));
            return transition;
        }
        
        case 2: // High Shelf
        {
            if (frequency > band.frequency)
                return band.gain;
            
            // Transition slope based on Q
            float transition = band.gain / (1.0f + std::pow(-octaveDistance * band.q * 2.0f, 2));
            return transition;
        }
        
        case 3: // Low Pass
        {
            if (frequency > band.frequency)
                return -24.0f * octaveDistance * band.q;
            return 0.0f;
        }
        
        case 4: // High Pass
        {
            if (frequency < band.frequency)
                return 24.0f * -octaveDistance * band.q;
            return 0.0f;
        }
        
        case 5: // Notch
        {
            // Narrow notch response
            float notchWidth = 0.5f / band.q;
            if (std::abs(octaveDistance) < notchWidth)
                return -24.0f * (1.0f - std::abs(octaveDistance) / notchWidth);
            return 0.0f;
        }
        
        default:
            return 0.0f;
    }
}

void ProEQComponent::drawBackground(juce::Graphics& g)
{
    auto bounds = getLocalBounds();
    
    // Fill background with dark gradient
    g.setGradientFill(juce::ColourGradient(
        juce::Colour(0xff1a1a1a), 0, 0,
        juce::Colour(0xff0f0f0f), 0, (float)bounds.getHeight(),
        false));
    g.fillAll();
    
    // Draw subtle grid pattern
    g.setColour(juce::Colour(0xff242424));
    for (int y = 0; y < bounds.getHeight(); y += 10) {
        g.drawHorizontalLine(y, 0.0f, (float)bounds.getWidth());
    }
    
    for (int x = 0; x < bounds.getWidth(); x += 10) {
        g.drawVerticalLine(x, 0.0f, (float)bounds.getHeight());
    }
}

void ProEQComponent::drawFrequencyScale(juce::Graphics& g)
{
    auto bounds = frequencyResponseArea;
    
    // Frequency scale at bottom
    int fontHeight = 12;
    g.setFont((float)fontHeight);
    g.setColour(juce::Colours::silver);
    
    // Standard frequencies for audio
    const float frequencies[] = { 20.0f, 50.0f, 100.0f, 200.0f, 500.0f, 
                                1000.0f, 2000.0f, 5000.0f, 10000.0f, 20000.0f };
                                
    for (auto freq : frequencies) {
        float x = getXForFrequency(freq);
        
        g.setColour(juce::Colours::darkgrey);
        g.drawVerticalLine(juce::roundToInt(x), (float)bounds.getY(), (float)bounds.getBottom());
        
        g.setColour(juce::Colours::silver);
        g.drawText(formatFrequency(freq), juce::roundToInt(x) - 10, bounds.getBottom(), 20, fontHeight,
                  juce::Justification::centred, false);
    }
}

void ProEQComponent::drawGainScale(juce::Graphics& g)
{
    auto bounds = frequencyResponseArea;
    
    // Gain scale on left side
    int fontHeight = 12;
    g.setFont((float)fontHeight);
    g.setColour(juce::Colours::silver);
    
    // Draw gain markers
    for (int gain = -24; gain <= 24; gain += 6) {
        float y = getYForGain(static_cast<float>(gain));
        
        g.setColour(juce::Colours::darkgrey);
        g.drawHorizontalLine(juce::roundToInt(y), (float)bounds.getX(), (float)bounds.getRight());
        
        g.setColour(juce::Colours::silver);
        juce::String gainText = (gain > 0 ? "+" : "") + juce::String(gain) + " dB";
        g.drawText(gainText, bounds.getX() - 35, juce::roundToInt(y) - fontHeight / 2, 
                  30, fontHeight, juce::Justification::centredRight, false);
    }
    
    // Center line (0 dB) in brighter color
    float y0dB = getYForGain(0.0f);
    g.setColour(juce::Colours::darkgrey.brighter(0.3f));
    g.drawHorizontalLine(juce::roundToInt(y0dB), (float)bounds.getX(), (float)bounds.getRight());
}

void ProEQComponent::drawFrequencyMarkers(juce::Graphics& g)
{
    auto bounds = frequencyResponseArea;
    
    g.setColour(juce::Colours::darkgrey.darker(0.3f));
    
    // Vertical grid lines (octaves)
    for (float freq = 20.0f; freq <= 20000.0f; freq *= 2.0f) {
        float x = getXForFrequency(freq);
        g.drawVerticalLine(juce::roundToInt(x), (float)bounds.getY(), (float)bounds.getBottom());
    }
    
    // Horizontal grid lines (gain)
    for (int gain = -24; gain <= 24; gain += 3) {
        float y = getYForGain(static_cast<float>(gain));
        g.drawHorizontalLine(juce::roundToInt(y), (float)bounds.getX(), (float)bounds.getRight());
    }
}

void ProEQComponent::drawResponseCurve(juce::Graphics& g)
{
    // Set up clip region
    g.saveState();
    g.reduceClipRegion(frequencyResponseArea);
    
    // Draw overall response curve
    g.setColour(juce::Colour(0xffffffff));
    g.strokePath(responseCurve, juce::PathStrokeType(2.0f));
    
    // Draw band frequencies as markers
    for (const auto& band : bands) {
        if (!band.isActive)
            continue;
        
        float x = getXForFrequency(band.frequency);
        float y = getYForGain(band.gain);
        
        // Draw vertical line at frequency
        g.setColour(band.colour.withAlpha(0.5f));
        g.drawVerticalLine(juce::roundToInt(x), (float)frequencyResponseArea.getY(), (float)frequencyResponseArea.getBottom());
        
        // Draw handle for dragging
        g.setColour(band.colour);
        g.fillEllipse(x - 5.0f, y - 5.0f, 10.0f, 10.0f);
        g.setColour(juce::Colours::white);
        g.drawEllipse(x - 5.0f, y - 5.0f, 10.0f, 10.0f, 1.0f);
    }
    
    g.restoreState();
}

float ProEQComponent::getXForFrequency(float frequency)
{
    auto bounds = frequencyResponseArea.toFloat();
    
    // Log scale from 20Hz to 20kHz
    constexpr float minFreq = 20.0f;
    constexpr float maxFreq = 20000.0f;
    
    // Convert to log scale and normalize to 0-1
    float normalisedX = std::log10(frequency / minFreq) / std::log10(maxFreq / minFreq);
    
    // Map to pixel position
    return bounds.getX() + bounds.getWidth() * normalisedX;
}

float ProEQComponent::getYForGain(float gainDB)
{
    auto bounds = frequencyResponseArea.toFloat();
    
    // Linear scale from -24dB to +24dB
    constexpr float minGain = -24.0f;
    constexpr float maxGain = 24.0f;
    
    // Normalize gain to 0-1 range (inverted)
    float normalisedY = 1.0f - ((gainDB - minGain) / (maxGain - minGain));
    
    // Map to pixel position
    return bounds.getY() + bounds.getHeight() * normalisedY;
}

float ProEQComponent::getFrequencyForX(float x)
{
    auto bounds = frequencyResponseArea.toFloat();
    
    // Normalize x position to 0-1
    float normalizedX = (x - bounds.getX()) / bounds.getWidth();
    
    // Constrain to valid range
    normalizedX = juce::jlimit(0.0f, 1.0f, normalizedX);
    
    // Convert from log scale
    constexpr float minFreq = 20.0f;
    constexpr float maxFreq = 20000.0f;
    
    return minFreq * std::pow(maxFreq / minFreq, normalizedX);
}

float ProEQComponent::getGainForY(float y)
{
    auto bounds = frequencyResponseArea.toFloat();
    
    // Normalize y position to 0-1 (inverted)
    float normalizedY = 1.0f - (y - bounds.getY()) / bounds.getHeight();
    
    // Constrain to valid range
    normalizedY = juce::jlimit(0.0f, 1.0f, normalizedY);
    
    // Convert to dB
    constexpr float minGain = -24.0f;
    constexpr float maxGain = 24.0f;
    
    return minGain + normalizedY * (maxGain - minGain);
}

juce::String ProEQComponent::formatFrequency(float freq)
{
    if (freq >= 1000.0f)
        return juce::String(freq / 1000.0f) + "k";
    return juce::String(freq);
}
