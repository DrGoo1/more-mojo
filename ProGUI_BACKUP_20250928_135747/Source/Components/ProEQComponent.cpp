#include "ProEQComponent.h"

ProEQComponent::ProEQComponent()
{
    // Initialize the magic state for PluginGuiMagic
    magicState = std::make_unique<foleys::MagicProcessorState>();
    magicBuilder = std::make_unique<foleys::MagicGUIBuilder>(*magicState);
    
    // Create spectrum analyzer
    spectrumAnalyzer = magicState->createAndAddObject<foleys::MagicAnalyser>("spectrum");
    
    // Create response graph for filter visualization
    responseGraph = magicState->createAndAddObject<foleys::MagicFilterPlot>("response");
    
    // Set up default filter bands
    bands[0].frequency = 100.0f;
    bands[0].type = LowShelf;
    
    bands[1].frequency = 400.0f;
    bands[1].type = Peak;
    
    bands[2].frequency = 1000.0f;
    bands[2].type = Peak;
    
    bands[3].frequency = 3500.0f;
    bands[3].type = Peak;
    
    bands[4].frequency = 8000.0f;
    bands[4].type = HighShelf;
    
    // Add the response graph component
    addAndMakeVisible(magicBuilder->createPluginComponent());
    
    // Start timer to periodically update visuals
    startTimerHz(30);
    
    // Initial update
    updateFilters();
}

ProEQComponent::~ProEQComponent()
{
    stopTimer();
}

void ProEQComponent::paint(juce::Graphics& g)
{
    // Draw background
    g.fillAll(juce::Colour(0xff202020));
}

void ProEQComponent::resized()
{
    auto area = getLocalBounds();
    
    // Set the size of the magic builder component
    if (auto* component = magicBuilder->getGuiRootComponent())
        component->setBounds(area);
}

void ProEQComponent::setBandFrequency(int bandIndex, float frequency)
{
    if (bandIndex >= 0 && bandIndex < static_cast<int>(bands.size()))
    {
        bands[bandIndex].frequency = juce::jlimit(minFreq, maxFreq, frequency);
        updateFilters();
    }
}

void ProEQComponent::setBandGain(int bandIndex, float gainDB)
{
    if (bandIndex >= 0 && bandIndex < static_cast<int>(bands.size()))
    {
        bands[bandIndex].gain = juce::jlimit(minGain, maxGain, gainDB);
        updateFilters();
    }
}

void ProEQComponent::setBandQ(int bandIndex, float q)
{
    if (bandIndex >= 0 && bandIndex < static_cast<int>(bands.size()))
    {
        bands[bandIndex].quality = juce::jlimit(0.1f, 10.0f, q);
        updateFilters();
    }
}

float ProEQComponent::getBandFrequency(int bandIndex) const
{
    if (bandIndex >= 0 && bandIndex < static_cast<int>(bands.size()))
        return bands[bandIndex].frequency;
    
    return 1000.0f; // Default frequency
}

float ProEQComponent::getBandGain(int bandIndex) const
{
    if (bandIndex >= 0 && bandIndex < static_cast<int>(bands.size()))
        return bands[bandIndex].gain;
    
    return 0.0f; // Default gain
}

float ProEQComponent::getBandQ(int bandIndex) const
{
    if (bandIndex >= 0 && bandIndex < static_cast<int>(bands.size()))
        return bands[bandIndex].quality;
    
    return 1.0f; // Default Q
}

void ProEQComponent::processAudio(const juce::AudioBuffer<float>& buffer)
{
    // Update the spectrum analyzer with the latest audio data
    if (spectrumAnalyzer)
        spectrumAnalyzer->pushSamples(buffer);
}

void ProEQComponent::updateFilters()
{
    if (sampleRate < 20.0)
        return;
    
    // Clear existing coefficients from the response graph
    std::vector<juce::dsp::IIR::Coefficients<float>::Ptr> allCoefficients;
    
    // Calculate coefficients for each band
    for (auto& band : bands)
    {
        if (!band.active)
            continue;
            
        switch (band.type)
        {
            case LowShelf:
                band.coefficients = juce::dsp::IIR::Coefficients<float>::makeLowShelf(
                    sampleRate, band.frequency, band.quality, 
                    juce::Decibels::decibelsToGain(band.gain));
                break;
                
            case LowPass:
                band.coefficients = juce::dsp::IIR::Coefficients<float>::makeLowPass(
                    sampleRate, band.frequency, band.quality);
                break;
                
            case Peak:
                band.coefficients = juce::dsp::IIR::Coefficients<float>::makePeakFilter(
                    sampleRate, band.frequency, band.quality, 
                    juce::Decibels::decibelsToGain(band.gain));
                break;
                
            case HighShelf:
                band.coefficients = juce::dsp::IIR::Coefficients<float>::makeHighShelf(
                    sampleRate, band.frequency, band.quality, 
                    juce::Decibels::decibelsToGain(band.gain));
                break;
                
            case HighPass:
                band.coefficients = juce::dsp::IIR::Coefficients<float>::makeHighPass(
                    sampleRate, band.frequency, band.quality);
                break;
                
            default:
                break;
        }
        
        if (band.coefficients)
            allCoefficients.push_back(band.coefficients);
    }
    
    // Update the response graph with all active coefficients
    if (responseGraph)
        responseGraph->setIIRCoefficients(1.0f, allCoefficients, 24.0f);
}

void ProEQComponent::timerCallback()
{
    // This is called periodically to update UI components
    repaint();
}
