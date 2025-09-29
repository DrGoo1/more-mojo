#pragma once
#include <juce_graphics/juce_graphics.h>
#include <juce_gui_basics/juce_gui_basics.h>

class WaveformView : public juce::Component, private juce::Timer {
public:
    WaveformView();
    ~WaveformView() override = default;

    // Provide simple waveform data (normalized -1..+1). Not realtime-optimized; demo only.
    void setSamples(const juce::Array<float>& data);

    // Playhead [0..1]
    void setPlayhead(float norm);

    // Timeline seconds (for grid labelling)
    void setDurationSeconds(double secs);

    // Zoom API (1.0 = full view). Range clamped to [1.0, 16.0].
    void setZoom(float factor);
    float getZoom() const { return zoom; }
    void zoomIn();
    void zoomOut();

    void paint(juce::Graphics& g) override;
    void resized() override;

    // Mouse wheel to zoom horizontally
    void mouseWheelMove (const juce::MouseEvent& e, const juce::MouseWheelDetails& wheel) override;

private:
    void timerCallback() override;

    juce::Array<float> samples; // mono, downsampled for view
    float playheadNorm { 0.0f };
    double durationSec { 10.0 };

    float zoom { 1.0f }; // 1.0 = entire duration visible
};
