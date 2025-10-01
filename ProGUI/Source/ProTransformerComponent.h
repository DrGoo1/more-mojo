#pragma once

#include <juce_core/juce_core.h>
#include <juce_events/juce_events.h>
#include <juce_graphics/juce_graphics.h>
#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_gui_extra/juce_gui_extra.h>
#include <juce_audio_basics/juce_audio_basics.h>

/**
    Transformer (color) component UI skeleton with simple parameter API.
 */
class ProTransformerComponent : public juce::Component,
                               private juce::Timer
{
public:
    ProTransformerComponent();
    ~ProTransformerComponent() override;

    void paint(juce::Graphics& g) override;
    void resized() override;

    // Simple processing for meter animation
    void processAudio(juce::AudioBuffer<float>& buffer);

    // Parameter API (used by wrapper/Swift bridge)
    void setEnabled(bool on) { enabled = on; repaint(); }
    bool getEnabled() const { return enabled; }

    void setModel(const juce::String& m) { model = m; }
    juce::String getModel() const { return model; }

    void setDrive(float d) { drive = d; }
    float getDrive() const { return drive; }

    void setZSrc(int z) { zsrc = z; }
    int getZSrc() const { return zsrc; }

    void setZLoad(int z) { zload = z; }
    int getZLoad() const { return zload; }

    void setH2(float v) { h2 = v; }
    float getH2() const { return h2; }

    void setH3(float v) { h3 = v; }
    float getH3() const { return h3; }

    void setXtalk(float db) { xtalkDB = db; }
    float getXtalk() const { return xtalkDB; }

    void setOvershoot(float pct) { overshootPct = pct; }
    float getOvershoot() const { return overshootPct; }

    void setMix(float pct) { mixPct = pct; }
    float getMix() const { return mixPct; }

private:
    // UI controls
    juce::ComboBox modelBox;
    juce::TextButton loadBtn { "Load CSV…" };
    juce::Slider driveSlider, zsrcSlider, zloadSlider, h2Slider, h3Slider, xtalkSlider, overshootSlider, mixSlider;

    // Simple harmonic meter placeholder
    struct HarmonicMeter : public juce::Component {
        void setValues(float v2, float v3) { H2 = v2; H3 = v3; repaint(); }
        void paint(juce::Graphics& g) override {
            auto r = getLocalBounds().reduced(8);
            g.fillAll(juce::Colour(28,28,36));
            g.setColour(juce::Colours::white);
            g.drawFittedText("H2: " + juce::String(H2,2) + "  H3: " + juce::String(H3,2), r, juce::Justification::centred, 1);
        }
        float H2 {0.0f}, H3 {0.0f};
    };
    HarmonicMeter meter;

    // Parameters
    bool enabled { true };
    juce::String model { "LL5402" };
    float drive { 0.0f };      // dBu
    int zsrc { 150 };          // Ohms
    int zload { 10000 };       // Ohms
    float h2 { 0.2f };
    float h3 { 0.06f };
    float xtalkDB { -40.0f };
    float overshootPct { 1.0f };
    float mixPct { 30.0f };

    void timerCallback() override;
};
