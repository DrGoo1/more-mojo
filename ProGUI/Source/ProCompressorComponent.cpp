#include "ProCompressorComponent.h"

ProCompressorComponent::ProCompressorComponent()
{
    // Initialize waveform buffer
    waveform.inputSamples.resize(waveform.numSamples, 0.0f);
    waveform.outputSamples.resize(waveform.numSamples, 0.0f);
    
    // Create UI controls
    createControls();
    
    // Start timer for visualization updates
    startTimerHz(30);
}

// ===== External Control API =====
void ProCompressorComponent::setThreshold(float v)
{
    settings.threshold = v;
    if (thresholdSlider) thresholdSlider->setValue(v, juce::sendNotificationSync);
    updateCompressionCurve();
}

void ProCompressorComponent::setRatio(float v)
{
    settings.ratio = v;
    if (ratioSlider) ratioSlider->setValue(v, juce::sendNotificationSync);
    updateCompressionCurve();
}

void ProCompressorComponent::setAttack(float v)
{
    settings.attack = v;
    if (attackSlider) attackSlider->setValue(v, juce::sendNotificationSync);
}

void ProCompressorComponent::setRelease(float v)
{
    settings.release = v;
    if (releaseSlider) releaseSlider->setValue(v, juce::sendNotificationSync);
}

void ProCompressorComponent::setKnee(float v)
{
    settings.knee = v;
    if (kneeSlider) kneeSlider->setValue(v, juce::sendNotificationSync);
    updateCompressionCurve();
}

void ProCompressorComponent::setMakeupGain(float v)
{
    settings.makeup = v;
    if (makeupSlider) makeupSlider->setValue(v, juce::sendNotificationSync);
}

void ProCompressorComponent::processAudio(juce::AudioBuffer<float>& buffer)
{
    // Simple visualization: compute level and simulate gain reduction for display
    int numChannels = buffer.getNumChannels();
    int numSamples = buffer.getNumSamples();
    if (numChannels == 0 || numSamples == 0) return;
    
    for (int i = 0; i < numSamples; ++i)
    {
        float in = 0.0f;
        for (int ch = 0; ch < numChannels; ++ch)
            in += std::abs(buffer.getReadPointer(ch)[i]);
        in /= (float) numChannels;
        float inDB = juce::Decibels::gainToDecibels(in + 1.0e-6f);
        float gr = calculateGainReduction(inDB);
        currentGainReduction = gr;
        float makeupGain = juce::Decibels::decibelsToGain(settings.makeup);
        for (int ch = 0; ch < numChannels; ++ch)
        {
            float sample = buffer.getReadPointer(ch)[i];
            float out = sample * makeupGain; // no actual compression applied; visual only
            pushSampleForAnalysis(sample, out);
        }
    }
}

ProCompressorComponent::~ProCompressorComponent()
{
    stopTimer();
}

void ProCompressorComponent::paint(juce::Graphics& g)
{
    // Draw background
    drawBackground(g);
    
    // Draw visualizations
    drawCompressionCurve(g);
    drawWaveform(g);
    drawGainReductionMeter(g);
}

void ProCompressorComponent::resized()
{
    // Layout the component
    auto bounds = getLocalBounds().reduced(10);
    
    // Top area for visualizations
    auto topArea = bounds.removeFromTop(bounds.getHeight() * 0.6f);
    
    // Split visualization area
    visualizationArea = topArea;
    compressionCurveArea = topArea.removeFromLeft(topArea.getWidth() * 0.4f).reduced(5);
    gainReductionMeterArea = topArea.removeFromRight(40).reduced(5, 0);
    
    // Controls area
    auto controlsArea = bounds;
    
    // First row of controls
    auto firstRowArea = controlsArea.removeFromTop(100);
    int thirdWidth = firstRowArea.getWidth() / 3;
    
    thresholdSlider->setBounds(firstRowArea.removeFromLeft(thirdWidth).reduced(10));
    thresholdLabel.setBounds(thresholdSlider->getBounds().removeFromTop(20));
    
    ratioSlider->setBounds(firstRowArea.removeFromLeft(thirdWidth).reduced(10));
    ratioLabel.setBounds(ratioSlider->getBounds().removeFromTop(20));
    
    kneeSlider->setBounds(firstRowArea.removeFromLeft(thirdWidth).reduced(10));
    kneeLabel.setBounds(kneeSlider->getBounds().removeFromTop(20));
    
    // Second row of controls
    auto secondRowArea = controlsArea.removeFromTop(100);
    int halfWidth = secondRowArea.getWidth() / 2;
    
    attackSlider->setBounds(secondRowArea.removeFromLeft(halfWidth).reduced(10));
    attackLabel.setBounds(attackSlider->getBounds().removeFromTop(20));
    
    releaseSlider->setBounds(secondRowArea.removeFromLeft(halfWidth).reduced(10));
    releaseLabel.setBounds(releaseSlider->getBounds().removeFromTop(20));
    
    // Third row
    auto thirdRowArea = controlsArea.removeFromTop(100);
    
    makeupSlider->setBounds(thirdRowArea.removeFromLeft(halfWidth).reduced(10));
    makeupLabel.setBounds(makeupSlider->getBounds().removeFromTop(20));
    
    autoMakeupButton->setBounds(thirdRowArea.removeFromLeft(halfWidth).reduced(10).withHeight(30).withY(thirdRowArea.getY() + 30));
    
    // Update compression curve after resizing
    updateCompressionCurve();
}

void ProCompressorComponent::timerCallback()
{
    // Update parameters from UI controls
    updateParameters();
    
    // Update compression curve if parameters changed
    updateCompressionCurve();
    
    // Update visualizations
    peakGainReduction = peakGainReduction * 0.95f;
    if (currentGainReduction > peakGainReduction)
        peakGainReduction = currentGainReduction;
    
    // Trigger repaint
    repaint();
}

void ProCompressorComponent::pushSampleForAnalysis(float inputSample, float outputSample)
{
    // Store samples in circular buffer
    waveform.inputSamples[waveform.writePosition] = inputSample;
    waveform.outputSamples[waveform.writePosition] = outputSample;
    
    // Update write position
    waveform.writePosition = (waveform.writePosition + 1) % waveform.numSamples;
    
    // Calculate current gain reduction for meter
    float inputLevel = juce::Decibels::gainToDecibels(std::abs(inputSample) + 0.00001f);
    currentGainReduction = calculateGainReduction(inputLevel);
}

void ProCompressorComponent::createControls()
{
    // Create threshold slider
    thresholdSlider.reset(new juce::Slider(juce::Slider::RotaryHorizontalVerticalDrag, juce::Slider::TextBoxBelow));
    thresholdSlider->setRange(-60.0, 0.0, 0.1);
    thresholdSlider->setValue(settings.threshold, juce::dontSendNotification);
    thresholdSlider->setTextValueSuffix(" dB");
    thresholdSlider->setColour(juce::Slider::rotarySliderFillColourId, juce::Colour(0xff4285f4));
    addAndMakeVisible(thresholdSlider.get());
    
    thresholdLabel.setText("Threshold", juce::dontSendNotification);
    thresholdLabel.setFont(juce::Font(14.0f, juce::Font::bold));
    thresholdLabel.setJustificationType(juce::Justification::centred);
    thresholdLabel.setColour(juce::Label::textColourId, juce::Colours::white);
    addAndMakeVisible(thresholdLabel);
    
    // Create ratio slider
    ratioSlider.reset(new juce::Slider(juce::Slider::RotaryHorizontalVerticalDrag, juce::Slider::TextBoxBelow));
    ratioSlider->setRange(1.0, 20.0, 0.1);
    ratioSlider->setValue(settings.ratio, juce::dontSendNotification);
    ratioSlider->setTextValueSuffix(":1");
    ratioSlider->setColour(juce::Slider::rotarySliderFillColourId, juce::Colour(0xffea4335));
    addAndMakeVisible(ratioSlider.get());
    
    ratioLabel.setText("Ratio", juce::dontSendNotification);
    ratioLabel.setFont(juce::Font(14.0f, juce::Font::bold));
    ratioLabel.setJustificationType(juce::Justification::centred);
    ratioLabel.setColour(juce::Label::textColourId, juce::Colours::white);
    addAndMakeVisible(ratioLabel);
    
    // Create attack slider
    attackSlider.reset(new juce::Slider(juce::Slider::RotaryHorizontalVerticalDrag, juce::Slider::TextBoxBelow));
    attackSlider->setRange(0.1, 100.0, 0.1);
    attackSlider->setSkewFactorFromMidPoint(10.0);
    attackSlider->setValue(settings.attack, juce::dontSendNotification);
    attackSlider->setTextValueSuffix(" ms");
    attackSlider->setColour(juce::Slider::rotarySliderFillColourId, juce::Colour(0xfffbbc05));
    addAndMakeVisible(attackSlider.get());
    
    attackLabel.setText("Attack", juce::dontSendNotification);
    attackLabel.setFont(juce::Font(14.0f, juce::Font::bold));
    attackLabel.setJustificationType(juce::Justification::centred);
    attackLabel.setColour(juce::Label::textColourId, juce::Colours::white);
    addAndMakeVisible(attackLabel);
    
    // Create release slider
    releaseSlider.reset(new juce::Slider(juce::Slider::RotaryHorizontalVerticalDrag, juce::Slider::TextBoxBelow));
    releaseSlider->setRange(10.0, 1000.0, 1.0);
    releaseSlider->setSkewFactorFromMidPoint(100.0);
    releaseSlider->setValue(settings.release, juce::dontSendNotification);
    releaseSlider->setTextValueSuffix(" ms");
    releaseSlider->setColour(juce::Slider::rotarySliderFillColourId, juce::Colour(0xff34a853));
    addAndMakeVisible(releaseSlider.get());
    
    releaseLabel.setText("Release", juce::dontSendNotification);
    releaseLabel.setFont(juce::Font(14.0f, juce::Font::bold));
    releaseLabel.setJustificationType(juce::Justification::centred);
    releaseLabel.setColour(juce::Label::textColourId, juce::Colours::white);
    addAndMakeVisible(releaseLabel);
    
    // Create knee slider
    kneeSlider.reset(new juce::Slider(juce::Slider::RotaryHorizontalVerticalDrag, juce::Slider::TextBoxBelow));
    kneeSlider->setRange(0.0, 24.0, 0.1);
    kneeSlider->setValue(settings.knee, juce::dontSendNotification);
    kneeSlider->setTextValueSuffix(" dB");
    kneeSlider->setColour(juce::Slider::rotarySliderFillColourId, juce::Colour(0xffbb86fc));
    addAndMakeVisible(kneeSlider.get());
    
    kneeLabel.setText("Knee", juce::dontSendNotification);
    kneeLabel.setFont(juce::Font(14.0f, juce::Font::bold));
    kneeLabel.setJustificationType(juce::Justification::centred);
    kneeLabel.setColour(juce::Label::textColourId, juce::Colours::white);
    addAndMakeVisible(kneeLabel);
    
    // Create makeup gain slider
    makeupSlider.reset(new juce::Slider(juce::Slider::RotaryHorizontalVerticalDrag, juce::Slider::TextBoxBelow));
    makeupSlider->setRange(0.0, 24.0, 0.1);
    makeupSlider->setValue(settings.makeup, juce::dontSendNotification);
    makeupSlider->setTextValueSuffix(" dB");
    makeupSlider->setColour(juce::Slider::rotarySliderFillColourId, juce::Colour(0xff03a9f4));
    makeupSlider->setEnabled(!settings.autoMakeup);
    addAndMakeVisible(makeupSlider.get());
    
    makeupLabel.setText("Makeup", juce::dontSendNotification);
    makeupLabel.setFont(juce::Font(14.0f, juce::Font::bold));
    makeupLabel.setJustificationType(juce::Justification::centred);
    makeupLabel.setColour(juce::Label::textColourId, juce::Colours::white);
    addAndMakeVisible(makeupLabel);
    
    // Create auto makeup button
    autoMakeupButton.reset(new juce::ToggleButton("Auto Makeup"));
    autoMakeupButton->setToggleState(settings.autoMakeup, juce::dontSendNotification);
    autoMakeupButton->setColour(juce::ToggleButton::textColourId, juce::Colours::white);
    addAndMakeVisible(autoMakeupButton.get());
    
    // Add listeners for controls (omitted in this example for simplicity)
    
    // Initialize the compression curve
    updateCompressionCurve();
}

void ProCompressorComponent::updateParameters()
{
    // Update settings from slider values
    settings.threshold = static_cast<float>(thresholdSlider->getValue());
    settings.ratio = static_cast<float>(ratioSlider->getValue());
    settings.attack = static_cast<float>(attackSlider->getValue());
    settings.release = static_cast<float>(releaseSlider->getValue());
    settings.knee = static_cast<float>(kneeSlider->getValue());
    settings.autoMakeup = autoMakeupButton->getToggleState();
    
    // Only update makeup gain if not in auto mode
    if (!settings.autoMakeup) {
        settings.makeup = static_cast<float>(makeupSlider->getValue());
    } else {
        // Calculate auto makeup gain based on threshold and ratio
        float thresholdCompensation = -settings.threshold;
        float ratioCompensation = (1.0f - (1.0f / settings.ratio)) * thresholdCompensation;
        settings.makeup = ratioCompensation * 0.5f; // Scale down a bit for natural sound
        makeupSlider->setValue(settings.makeup, juce::dontSendNotification);
    }
    
    // Enable/disable makeup slider based on auto mode
    makeupSlider->setEnabled(!settings.autoMakeup);
}

void ProCompressorComponent::updateCompressionCurve()
{
    // Generate compression curve path
    compressionCurve.clear();
    
    auto bounds = compressionCurveArea;
    float width = static_cast<float>(bounds.getWidth());
    float height = static_cast<float>(bounds.getHeight());
    
    // Range for curve display: -60dB to 0dB
    constexpr float minDB = -60.0f;
    constexpr float maxDB = 0.0f;
    
    const int numPoints = 100;
    bool pathStarted = false;
    
    for (int i = 0; i <= numPoints; ++i) {
        // Map x to input level in dB
        float inputDB = juce::jmap(static_cast<float>(i) / numPoints, minDB, maxDB);
        
        // Calculate compressed output level
        float gainReduction = calculateGainReduction(inputDB);
        float outputDB = inputDB + gainReduction + settings.makeup;
        
        // Map to pixel coordinates (inverted y-axis)
        float x = bounds.getX() + width * (inputDB - minDB) / (maxDB - minDB);
        float y = bounds.getBottom() - height * (outputDB - minDB) / (maxDB - minDB);
        
        // Add point to path
        if (!pathStarted) {
            compressionCurve.startNewSubPath(x, y);
            pathStarted = true;
        } else {
            compressionCurve.lineTo(x, y);
        }
    }
}

void ProCompressorComponent::drawBackground(juce::Graphics& g)
{
    // Fill background with dark gradient
    auto bounds = getLocalBounds().toFloat();
    
    g.setGradientFill(juce::ColourGradient(
        juce::Colour(0xff1a1a1a), 0, 0,
        juce::Colour(0xff0f0f0f), 0, bounds.getHeight(),
        false));
    g.fillAll();
    
    // Draw subtle grid pattern
    g.setColour(juce::Colour(0xff222222));
    for (int y = 0; y < bounds.getHeight(); y += 10) {
        g.drawHorizontalLine(y, 0.0f, bounds.getWidth());
    }
    
    for (int x = 0; x < bounds.getWidth(); x += 10) {
        g.drawVerticalLine(x, 0.0f, bounds.getHeight());
    }
}

void ProCompressorComponent::drawCompressionCurve(juce::Graphics& g)
{
    auto bounds = compressionCurveArea;
    
    // Draw background
    g.setColour(juce::Colour(0xff2a2a2a));
    g.fillRoundedRectangle(bounds.toFloat(), 5.0f);
    
    // Draw grid
    g.setColour(juce::Colour(0xff333333));
    
    // Horizontal grid lines
    for (int i = 0; i <= 6; ++i) {
        float y = bounds.getY() + (bounds.getHeight() * i / 6.0f);
        g.drawHorizontalLine(juce::roundToInt(y), static_cast<float>(bounds.getX()), static_cast<float>(bounds.getRight()));
    }
    
    // Vertical grid lines
    for (int i = 0; i <= 6; ++i) {
        float x = bounds.getX() + (bounds.getWidth() * i / 6.0f);
        g.drawVerticalLine(juce::roundToInt(x), static_cast<float>(bounds.getY()), static_cast<float>(bounds.getBottom()));
    }
    
    // Draw 1:1 line
    g.setColour(juce::Colours::white.withAlpha(0.2f));
    g.drawLine(static_cast<float>(bounds.getX()), static_cast<float>(bounds.getBottom()), 
              static_cast<float>(bounds.getRight()), static_cast<float>(bounds.getY()), 1.0f);
    
    // Draw threshold line
    float thresholdX = bounds.getX() + bounds.getWidth() * 
                      (settings.threshold - (-60.0f)) / (0.0f - (-60.0f));
    
    g.setColour(juce::Colour(0xff4285f4).withAlpha(0.7f));
    g.drawVerticalLine(juce::roundToInt(thresholdX), static_cast<float>(bounds.getY()), static_cast<float>(bounds.getBottom()));
    
    // Draw compression curve
    g.setColour(juce::Colour(0xffffffff));
    g.strokePath(compressionCurve, juce::PathStrokeType(2.0f));
    
    // Draw labels
    g.setColour(juce::Colours::white);
    g.setFont(12.0f);
    g.drawText("Input (dB)", bounds.getX(), bounds.getBottom() + 5, bounds.getWidth(), 15,
              juce::Justification::centred, false);
    
    g.addTransform(juce::AffineTransform::rotation(-juce::MathConstants<float>::halfPi,
                                                 bounds.getX() - 20, bounds.getCentreY()));
    g.drawText("Output (dB)", bounds.getX() - 40, bounds.getCentreY() - bounds.getHeight() / 2,
              bounds.getHeight(), 15, juce::Justification::centred, false);
}

void ProCompressorComponent::drawWaveform(juce::Graphics& g)
{
    // Draw waveform visualization between compression curve and gain reduction meter
    auto bounds = visualizationArea;
    bounds.removeFromLeft(compressionCurveArea.getWidth() + 10);
    bounds.removeFromRight(gainReductionMeterArea.getWidth() + 10);
    
    // Draw background
    g.setColour(juce::Colour(0xff2a2a2a));
    g.fillRoundedRectangle(bounds.toFloat(), 5.0f);
    
    // Draw grid
    g.setColour(juce::Colour(0xff333333));
    
    // Horizontal center line
    float centerY = bounds.getY() + bounds.getHeight() / 2.0f;
    g.drawHorizontalLine(juce::roundToInt(centerY), static_cast<float>(bounds.getX()), static_cast<float>(bounds.getRight()));
    
    // Create paths for input and output waveforms
    juce::Path inputWaveform;
    juce::Path outputWaveform;
    
    float width = static_cast<float>(bounds.getWidth());
    float height = static_cast<float>(bounds.getHeight());
    float halfHeight = height / 2.0f;
    
    bool inputPathStarted = false;
    bool outputPathStarted = false;
    
    for (int i = 0; i < waveform.numSamples; ++i) {
        int index = (waveform.writePosition - i + waveform.numSamples) % waveform.numSamples;
        
        float x = bounds.getX() + width * (waveform.numSamples - i) / waveform.numSamples;
        
        // Input waveform
        float inputY = centerY - waveform.inputSamples[index] * halfHeight;
        if (!inputPathStarted) {
            inputWaveform.startNewSubPath(x, inputY);
            inputPathStarted = true;
        } else {
            inputWaveform.lineTo(x, inputY);
        }
        
        // Output waveform
        float outputY = centerY - waveform.outputSamples[index] * halfHeight;
        if (!outputPathStarted) {
            outputWaveform.startNewSubPath(x, outputY);
            outputPathStarted = true;
        } else {
            outputWaveform.lineTo(x, outputY);
        }
    }
    
    // Draw waveforms
    g.setColour(juce::Colour(0xffea4335).withAlpha(0.6f)); // Input in red
    g.strokePath(inputWaveform, juce::PathStrokeType(1.5f));
    
    g.setColour(juce::Colour(0xff4285f4));  // Output in blue
    g.strokePath(outputWaveform, juce::PathStrokeType(1.5f));
    
    // Draw labels
    g.setColour(juce::Colours::white);
    g.setFont(12.0f);
    g.drawText("Waveform", bounds.getX(), bounds.getY() - 20, bounds.getWidth(), 20,
              juce::Justification::centred, false);
    
    g.setColour(juce::Colour(0xffea4335));
    g.fillRect(bounds.getX() + 10, bounds.getBottom() + 5, 10, 10);
    g.setColour(juce::Colours::white);
    g.drawText("Input", bounds.getX() + 25, bounds.getBottom() + 5, 50, 10,
              juce::Justification::centredLeft, false);
    
    g.setColour(juce::Colour(0xff4285f4));
    g.fillRect(bounds.getCentreX(), bounds.getBottom() + 5, 10, 10);
    g.setColour(juce::Colours::white);
    g.drawText("Output", bounds.getCentreX() + 15, bounds.getBottom() + 5, 60, 10,
              juce::Justification::centredLeft, false);
}

void ProCompressorComponent::drawGainReductionMeter(juce::Graphics& g)
{
    auto bounds = gainReductionMeterArea;
    
    // Draw background
    g.setColour(juce::Colour(0xff2a2a2a));
    g.fillRoundedRectangle(bounds.toFloat(), 5.0f);
    
    // Draw scale
    g.setColour(juce::Colours::white);
    g.setFont(10.0f);
    
    // Scale markers
    for (int i = 0; i <= 20; i += 5) {
        float y = bounds.getY() + bounds.getHeight() * i / 20.0f;
        float w = (i % 10 == 0) ? 6.0f : 3.0f;
        g.drawLine(bounds.getX(), y, bounds.getX() + w, y, 1.0f);
        
        if (i % 10 == 0) {
            g.drawText(juce::String(i), bounds.getX() + 8, juce::roundToInt(y) - 5, 20, 10,
                      juce::Justification::left, false);
        }
    }
    
    g.drawText("GR (dB)", bounds.getX(), bounds.getY() - 20, bounds.getWidth(), 20,
              juce::Justification::centred, false);
    
    // Calculate meter position
    float meterHeight = bounds.getHeight() * juce::jlimit(0.0f, 20.0f, -currentGainReduction) / 20.0f;
    float peakHeight = bounds.getHeight() * juce::jlimit(0.0f, 20.0f, -peakGainReduction) / 20.0f;
    
    // Draw meter bar with gradient
    if (meterHeight > 0.0f) {
        g.setGradientFill(juce::ColourGradient(
            juce::Colour(0xff4CAF50), static_cast<float>(bounds.getCentreX()), static_cast<float>(bounds.getY()),
            juce::Colour(0xffF44336), static_cast<float>(bounds.getCentreX()), static_cast<float>(bounds.getY() + bounds.getHeight()),
            false));
            
        g.fillRect(static_cast<float>(bounds.getX()) + 2.0f,
                   static_cast<float>(bounds.getY()),
                   static_cast<float>(bounds.getWidth()) - 4.0f,
                   meterHeight);
    }
    
    // Draw peak marker
    if (peakHeight > 0.0f) {
        g.setColour(juce::Colour(0xffF44336));
        g.drawLine(static_cast<float>(bounds.getX() + 1), 
                  static_cast<float>(bounds.getY() + peakHeight), 
                  static_cast<float>(bounds.getRight() - 1),
                  static_cast<float>(bounds.getY() + peakHeight), 
                  2.0f);
    }
}

float ProCompressorComponent::calculateGainReduction(float inputLevel)
{
    // Simple gain reduction calculation based on input level and compressor parameters
    float threshold = settings.threshold;
    float ratio = settings.ratio;
    float knee = settings.knee;
    
    if (knee <= 0.0f) {
        // Hard knee
        if (inputLevel <= threshold)
            return 0.0f;
        else
            return (threshold - inputLevel) * (1.0f - 1.0f / ratio);
    } else {
        // Soft knee
        if (inputLevel < threshold - knee / 2.0f)
            return 0.0f;
        else if (inputLevel > threshold + knee / 2.0f)
            return (threshold - inputLevel) * (1.0f - 1.0f / ratio);
        else {
            // In knee region
            float kneeInput = inputLevel - (threshold - knee / 2.0f);
            float kneeComp = kneeInput * kneeInput / (2.0f * knee);
            return kneeComp * (1.0f - 1.0f / ratio);
        }
    }
}
