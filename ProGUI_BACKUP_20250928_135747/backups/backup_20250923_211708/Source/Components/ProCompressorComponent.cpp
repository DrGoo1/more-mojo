#include "ProCompressorComponent.h"

ProCompressorComponent::ProCompressorComponent()
    : inputBuffer(2, bufferSize),
      outputBuffer(2, bufferSize)
{
    // Initialize the magic state for PluginGuiMagic
    magicState = std::make_unique<foleys::MagicProcessorState>();
    magicBuilder = std::make_unique<foleys::MagicGUIBuilder>(*magicState);
    
    // Create waveform visualizers
    inputWaveform = magicState->createAndAddObject<foleys::MagicWaveformPlot>("inputWaveform");
    outputWaveform = magicState->createAndAddObject<foleys::MagicWaveformPlot>("outputWaveform");
    
    // Create gain reduction meter
    gainReductionMeter = magicState->createAndAddObject<foleys::MagicLevelSource>("gainReduction");
    if (gainReductionMeter)
        gainReductionMeter->setupSource(1, sampleRate, 200);
    
    // Create compression curve visualizer
    compressionCurve = std::make_unique<CompressionCurvePlot>();
    compressionCurve->setParameters(threshold, ratio, knee);
    magicState->createAndAddObject("compressionCurve", compressionCurve.get());
    
    // Add the GUI component
    addAndMakeVisible(magicBuilder->createPluginComponent());
    
    // Clear buffers
    inputBuffer.clear();
    outputBuffer.clear();
    
    // Start timer to periodically update visuals
    startTimerHz(30);
    
    // Create GUI layout from XML
    auto guiTree = juce::ValueTree::fromXml(R"(
        <magic>
            <View id="root" flex-direction="column" padding="5">
                <View flex-direction="row" flex-grow="1" margin="5">
                    <View flex-grow="1" margin="5">
                        <Plot source="compressionCurve" 
                              plot-color="ff2a77d4"
                              background-color="ff222222" 
                              line-thickness="2.0"
                              margin="5" />
                    </View>
                    <View flex-grow="1" flex-direction="column" margin="5">
                        <View flex-grow="0.5">
                            <Plot source="inputWaveform" 
                                  plot-color="ff22aa22"
                                  background-color="ff222222" 
                                  line-thickness="1.5"
                                  margin="5" />
                        </View>
                        <View flex-grow="0.5">
                            <Plot source="outputWaveform" 
                                  plot-color="ff2a77d4"
                                  background-color="ff222222" 
                                  line-thickness="1.5"
                                  margin="5" />
                        </View>
                    </View>
                </View>
                <View flex-grow="0.2" margin="5">
                    <MeterSource source="gainReduction"
                                background-color="ff222222"
                                gradient="ff550000, ffaa0000, ffaaaa00, ff00aa00"
                                margin="5" />
                </View>
            </View>
        </magic>
    )");
    
    magicBuilder->createGUI(guiTree);
    
    // Initial update
    updateCompressionCurve();
}

ProCompressorComponent::~ProCompressorComponent()
{
    stopTimer();
}

void ProCompressorComponent::paint(juce::Graphics& g)
{
    // Draw background
    g.fillAll(juce::Colour(0xff202020));
}

void ProCompressorComponent::resized()
{
    auto area = getLocalBounds();
    
    // Set the size of the magic builder component
    if (auto* component = magicBuilder->getGuiRootComponent())
        component->setBounds(area);
}

void ProCompressorComponent::setThreshold(float thresholdDB)
{
    threshold = juce::jlimit(-60.0f, 0.0f, thresholdDB);
    updateCompressionCurve();
}

void ProCompressorComponent::setRatio(float newRatio)
{
    ratio = juce::jlimit(1.0f, 20.0f, newRatio);
    updateCompressionCurve();
}

void ProCompressorComponent::setAttack(float attackMs)
{
    attack = juce::jlimit(0.1f, 100.0f, attackMs);
}

void ProCompressorComponent::setRelease(float releaseMs)
{
    release = juce::jlimit(10.0f, 1000.0f, releaseMs);
}

void ProCompressorComponent::setKnee(float kneeDB)
{
    knee = juce::jlimit(0.0f, 24.0f, kneeDB);
    updateCompressionCurve();
}

void ProCompressorComponent::setMakeupGain(float makeupDB)
{
    makeupGain = juce::jlimit(0.0f, 24.0f, makeupDB);
}

float ProCompressorComponent::getThreshold() const
{
    return threshold;
}

float ProCompressorComponent::getRatio() const
{
    return ratio;
}

float ProCompressorComponent::getAttack() const
{
    return attack;
}

float ProCompressorComponent::getRelease() const
{
    return release;
}

float ProCompressorComponent::getKnee() const
{
    return knee;
}

float ProCompressorComponent::getMakeupGain() const
{
    return makeupGain;
}

void ProCompressorComponent::processAudio(const juce::AudioBuffer<float>& buffer)
{
    const int numChannels = buffer.getNumChannels();
    const int numSamples = buffer.getNumSamples();
    
    // Process each sample and update visualization buffers
    for (int i = 0; i < numSamples; ++i)
    {
        // Get input sample (average of all channels)
        float inputSample = 0.0f;
        for (int channel = 0; channel < numChannels; ++channel)
            inputSample += std::abs(buffer.getSample(channel, i));
        
        inputSample /= numChannels;
        
        // Process through compressor model
        float outputSample = processSample(inputSample);
        
        // Store in circular buffer for visualization
        inputBuffer.setSample(0, bufferPosition, inputSample);
        outputBuffer.setSample(0, bufferPosition, outputSample);
        
        // Update buffer position
        bufferPosition = (bufferPosition + 1) % bufferSize;
    }
    
    // Update visualizers with new data
    if (inputWaveform)
        inputWaveform->pushSamples(inputBuffer);
        
    if (outputWaveform)
        outputWaveform->pushSamples(outputBuffer);
        
    // Update gain reduction meter
    if (gainReductionMeter)
    {
        juce::AudioBuffer<float> grBuffer(1, 1);
        grBuffer.setSample(0, 0, currentGainReduction);
        gainReductionMeter->pushSamples(grBuffer);
    }
}

float ProCompressorComponent::processSample(float inputSample)
{
    // Convert to dB
    float inputDB = inputSample > 0.000001f ? juce::Decibels::gainToDecibels(inputSample) : -100.0f;
    
    // Calculate the amount of gain reduction using the soft knee
    float gainReduction = 0.0f;
    
    // Below threshold - knee/2
    if (inputDB < threshold - knee / 2.0f)
    {
        gainReduction = 0.0f;
    }
    // Within the knee range
    else if (inputDB < threshold + knee / 2.0f)
    {
        float kneeInput = inputDB - threshold + knee / 2.0f;
        float kneeRatio = 1.0f + (ratio - 1.0f) * kneeInput * kneeInput / (2.0f * knee);
        gainReduction = (threshold - inputDB) * (1.0f - 1.0f / kneeRatio);
    }
    // Above threshold + knee/2
    else
    {
        gainReduction = (threshold - inputDB) * (1.0f - 1.0f / ratio);
    }
    
    // Smooth the gain reduction with attack and release times
    const float attackCoef = std::exp(-1.0f / (sampleRate * attack / 1000.0f));
    const float releaseCoef = std::exp(-1.0f / (sampleRate * release / 1000.0f));
    
    // Determine if we're in attack or release phase
    float coef = gainReduction < detectorValue ? attackCoef : releaseCoef;
    
    // Smooth the gain reduction
    detectorValue = gainReduction + coef * (detectorValue - gainReduction);
    
    // Save the current gain reduction for metering
    currentGainReduction = -detectorValue;
    
    // Apply the gain reduction and makeup gain
    float outputGain = juce::Decibels::decibelsToGain(detectorValue + makeupGain);
    
    // Return processed sample
    return inputSample * outputGain;
}

void ProCompressorComponent::updateCompressionCurve()
{
    if (compressionCurve)
    {
        compressionCurve->setParameters(threshold, ratio, knee);
        
        // Mark the plot as needing to be redrawn
        auto* plot = magicState->getObjectWithType<foleys::MagicPlotComponent>("compressionCurve");
        if (plot)
            plot->repaint();
    }
}

void ProCompressorComponent::timerCallback()
{
    // Periodic updates for UI
    repaint();
}

//==============================================================================
// CompressionCurvePlot implementation
//==============================================================================
void ProCompressorComponent::CompressionCurvePlot::createPlotPaths(juce::Path& path, 
                                                                 juce::Path& filledPath, 
                                                                 juce::Rectangle<float> bounds, 
                                                                 foleys::MagicPlotComponent&)
{
    path.clear();
    filledPath.clear();
    
    const float width = bounds.getWidth();
    const float height = bounds.getHeight();
    
    // Set up scales
    const float dbMin = -60.0f;
    const float dbMax = 10.0f;
    
    // Function to map dB to x position
    auto dbToX = [&](float db)
    {
        return bounds.getX() + juce::jmap(db, dbMin, dbMax, 0.0f, width);
    };
    
    // Function to map dB to y position
    auto dbToY = [&](float db)
    {
        return bounds.getBottom() - juce::jmap(db, dbMin, dbMax, 0.0f, height);
    };
    
    // Start the path at the minimum dB
    path.startNewSubPath(dbToX(dbMin), dbToY(dbMin));
    
    // Create the compression curve
    for (float inputDB = dbMin; inputDB <= dbMax; inputDB += 0.5f)
    {
        float outputDB = inputDB;
        
        // Below threshold - knee/2
        if (inputDB < threshold - knee / 2.0f)
        {
            outputDB = inputDB;
        }
        // Within the knee range
        else if (inputDB < threshold + knee / 2.0f)
        {
            float kneeInput = inputDB - threshold + knee / 2.0f;
            float kneeRatio = 1.0f + (ratio - 1.0f) * kneeInput * kneeInput / (2.0f * knee);
            float diff = (inputDB - threshold) / kneeRatio;
            outputDB = threshold + diff;
        }
        // Above threshold + knee/2
        else
        {
            float diff = (inputDB - threshold) / ratio;
            outputDB = threshold + diff;
        }
        
        path.lineTo(dbToX(inputDB), dbToY(outputDB));
    }
    
    // Create the filled path (for visualization)
    filledPath = path;
    filledPath.lineTo(dbToX(dbMax), dbToY(dbMin));
    filledPath.lineTo(dbToX(dbMin), dbToY(dbMin));
    filledPath.closeSubPath();
}

void ProCompressorComponent::CompressionCurvePlot::setParameters(float newThreshold, float newRatio, float newKnee)
{
    threshold = newThreshold;
    ratio = newRatio;
    knee = newKnee;
}
