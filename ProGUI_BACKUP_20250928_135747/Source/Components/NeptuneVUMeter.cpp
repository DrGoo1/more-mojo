#include "NeptuneVUMeter.h"

NeptuneVUMeter::NeptuneVUMeter()
{
    loadNeptuneFrames();
}

void NeptuneVUMeter::setLevel(float newLevel)
{
    currentLevel = juce::jlimit(0.0f, 1.0f, newLevel);
    repaint();
}

void NeptuneVUMeter::paint(juce::Graphics& g)
{
    if (neptuneFrames.size() > 0)
    {
        int frameIndex = juce::jlimit(0, neptuneFrames.size()-1, (int)(currentLevel * (neptuneFrames.size()-1)));
        g.drawImage(neptuneFrames[frameIndex], getLocalBounds().toFloat());
    }
    else
    {
        // Fallback if Neptune frames not loaded
        g.setColour(juce::Colours::darkgrey);
        g.fillAll();
        g.setColour(juce::Colours::white);
        g.drawText("VU", getLocalBounds(), juce::Justification::centred);
    }
}

void NeptuneVUMeter::loadNeptuneFrames()
{
    juce::File neptuneMeterDir("/Volumes/MP 1/MoreMojo/NeptuneUI/03 Delivery/VST/Assets/Animation/Oneshots/vu_meter");
    
    if (!neptuneMeterDir.exists())
    {
        juce::Logger::writeToLog("Neptune VU meter directory not found: " + neptuneMeterDir.getFullPathName());
        return;
    }
    
    auto files = neptuneMeterDir.findChildFiles(juce::File::findFiles, false, "*.png");
    files.sort();
    
    for (auto& file : files)
    {
        auto image = juce::ImageFileFormat::loadFrom(file);
        if (image.isValid())
        {
            neptuneFrames.add(image);
        }
    }
    
    juce::Logger::writeToLog("Neptune VU Meter: Loaded " + juce::String(neptuneFrames.size()) + " frames");
}
