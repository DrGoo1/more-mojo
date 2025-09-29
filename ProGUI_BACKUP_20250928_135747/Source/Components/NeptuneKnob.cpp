#include "NeptuneKnob.h"

NeptuneKnob::NeptuneKnob()
{
    loadNeptuneFrames();
}

void NeptuneKnob::setValue(float newValue)
{
    currentValue = juce::jlimit(0.0f, 1.0f, newValue);
    repaint();
}

void NeptuneKnob::paint(juce::Graphics& g)
{
    if (neptuneFrames.size() > 0)
    {
        int frameIndex = juce::jlimit(0, neptuneFrames.size()-1, (int)(currentValue * (neptuneFrames.size()-1)));
        g.drawImage(neptuneFrames[frameIndex], getLocalBounds().toFloat());
    }
    else
    {
        // Fallback if Neptune frames not loaded
        g.setColour(juce::Colours::darkgrey);
        g.fillEllipse(getLocalBounds().toFloat());
        g.setColour(juce::Colours::white);
        g.drawText("ISP", getLocalBounds(), juce::Justification::centred);
    }
}

void NeptuneKnob::mouseDown(const juce::MouseEvent& event)
{
    lastMousePos = event.getPosition();
}

void NeptuneKnob::mouseDrag(const juce::MouseEvent& event)
{
    auto currentPos = event.getPosition();
    auto delta = lastMousePos.y - currentPos.y; // Inverted: up = increase
    
    float sensitivity = 0.005f;
    float newValue = currentValue + delta * sensitivity;
    setValue(newValue);
    
    if (onValueChanged)
        onValueChanged(currentValue);
    
    lastMousePos = currentPos;
}

void NeptuneKnob::loadNeptuneFrames()
{
    juce::File neptuneKnobDir("/Volumes/MP 1/MoreMojo/NeptuneUI/03 Delivery/VST/Assets/Animation/Oneshots/knob_big");
    
    if (!neptuneKnobDir.exists())
    {
        juce::Logger::writeToLog("Neptune knob directory not found: " + neptuneKnobDir.getFullPathName());
        return;
    }
    
    auto files = neptuneKnobDir.findChildFiles(juce::File::findFiles, false, "*.png");
    files.sort();
    
    for (auto& file : files)
    {
        auto image = juce::ImageFileFormat::loadFrom(file);
        if (image.isValid())
        {
            neptuneFrames.add(image);
        }
    }
    
    juce::Logger::writeToLog("Neptune Knob: Loaded " + juce::String(neptuneFrames.size()) + " frames");
}
