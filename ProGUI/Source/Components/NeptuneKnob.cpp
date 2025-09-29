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
    juce::File knobDir("/Volumes/MP 1/MoreMojo/Knobs/knob-77/assets");
    
    if (!knobDir.exists())
    {
        juce::Logger::writeToLog("Knob-77 directory not found: " + knobDir.getFullPathName());
        return;
    }
    
    // Load frames 001.png to 128.png in order
    for (int i = 1; i <= 128; ++i)
    {
        juce::String filename = juce::String::formatted("%03d.png", i);
        juce::File frameFile = knobDir.getChildFile(filename);
        
        if (frameFile.exists())
        {
            auto image = juce::ImageFileFormat::loadFrom(frameFile);
            if (image.isValid())
            {
                neptuneFrames.add(image);
            }
        }
    }
    
    juce::Logger::writeToLog("Knob-77: Loaded " + juce::String(neptuneFrames.size()) + " frames");
}
