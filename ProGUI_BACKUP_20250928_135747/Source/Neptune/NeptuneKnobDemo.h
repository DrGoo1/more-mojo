#pragma once
#include <juce_gui_basics/juce_gui_basics.h>

// A simple filmstrip/sequence-driven knob Component + some basic controls preview
class NeptuneKnobDemo : public juce::Component, private juce::Timer
{
public:
    NeptuneKnobDemo(const juce::File& bigDir,
                    const juce::File& medDir,
                    const juce::File& smallDir);

    void resized() override;
    void paint (juce::Graphics&) override;

private:
    // Loads PNG sequence from a directory (e.g., the 257 frames)
    struct KnobSeq {
        juce::OwnedArray<juce::Image> frames;
        int frameCount = 0;
        int current = 0;
        int targetW = 120, targetH = 120; // draw size
        void loadFromDir(const juce::File& dir);
        void draw(juce::Graphics& g, juce::Rectangle<int> r, float normPos);
    };

    KnobSeq big, med, small;

    // some basic controls to verify JUCE widgets render fine
    juce::TextButton  testButton   { "Test" };
    juce::Slider      testSlider   { juce::Slider::LinearHorizontal, juce::Slider::TextBoxBelow };
    juce::ComboBox    testCombo;

    float valueNorm = 0.5f;

    void timerCallback() override; // small animation to prove frames render

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (NeptuneKnobDemo)
};
