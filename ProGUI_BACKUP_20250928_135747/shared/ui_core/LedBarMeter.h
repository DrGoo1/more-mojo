#pragma once
#include <juce_graphics/juce_graphics.h>
#include <juce_gui_basics/juce_gui_basics.h>

class LedBarMeter : public juce::Component, private juce::Timer {
public:
    enum Orientation { Vertical, Horizontal };

    struct Config {
        int   numSegments        = 48;
        float yellowStartNorm    = 0.66f; // >= this → yellow
        float redStartNorm       = 0.85f; // >= this → red
        float peakHoldSeconds    = 0.8f;
        float fallRateDbPerSec   = 18.0f; // peak return speed
        Orientation orientation  = Vertical;
        bool   drawGridShadows   = true;
        bool   showRmsOverlay    = true;
    };

    LedBarMeter();
    explicit LedBarMeter(const Config& cfg);
    ~LedBarMeter() override = default;

    // Meter updates (normalized 0..1)
    void setPeakNorm(float norm);
    void setRmsNorm (float norm);

    // Convenience if you have dB values
    static float dbToNorm(float db, float floorDb = -60.0f) {
        return juce::jlimit(0.0f, 1.0f, 1.0f + (db / juce::jmax(1.0f, -floorDb)));
    }

    // Sprite management: 4 frames horizontally (OFF,G,Y,R)
    void setSpriteImage(const juce::Image& sprite);
    void setOrientation(Orientation o) { config.orientation = o; resized(); repaint(); }
    void setSegments(int n)            { config.numSegments = juce::jmax(4, n); resized(); repaint(); }
    void setThresholds(float yellowStart, float redStart) {
        config.yellowStartNorm = juce::jlimit(0.f, 1.f, yellowStart);
        config.redStartNorm    = juce::jlimit(0.f, 1.f, redStart);
        if (config.redStartNorm < config.yellowStartNorm)
            std::swap(config.redStartNorm, config.yellowStartNorm);
        repaint();
    }

    void paint(juce::Graphics&) override;
    void resized() override;

private:
    void timerCallback() override;
    void drawSegment(juce::Graphics& g, juce::Rectangle<float> r, bool lit, int litKind);
    void drawVectorSegment(juce::Graphics& g, juce::Rectangle<float> r, bool lit, juce::Colour litColour);
    int  litKindForSegment(int segIndex) const; // 1=G, 2=Y, 3=R

    Config config;
    juce::Image sprite;                   // 4 frames (OFF,G,Y,R)
    juce::Rectangle<int> spriteFrames[4]; // cached rects inside sprite

    // Levels
    std::atomic<float> peakNorm { 0.0f }; // 0..1
    std::atomic<float> rmsNorm  { 0.0f }; // 0..1

    // Peak hold state
    float heldPeak  = 0.0f;
    double holdTEnd = 0.0; // seconds

    // Cached layout
    juce::Array<juce::Rectangle<float>> segmentRects;
};
