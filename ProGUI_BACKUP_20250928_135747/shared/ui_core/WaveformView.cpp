#include "WaveformView.h"

WaveformView::WaveformView()
{
    setOpaque(false);
    startTimerHz(30);
    // Default placeholder: simple decaying sine for 10s
    const int N = 4096;
    juce::Array<float> tmp; tmp.ensureStorageAllocated(N);
    for (int i = 0; i < N; ++i) {
        float t = (float)i / (float)(N - 1);
        float s = std::sin(juce::MathConstants<float>::twoPi * 5.0f * t) * (1.0f - t);
        tmp.add(s);
    }
    setSamples(tmp);
    setDurationSeconds(10.0);
}

void WaveformView::setSamples(const juce::Array<float>& data)
{
    samples = data;
    repaint();
}

void WaveformView::setPlayhead(float norm)
{
    playheadNorm = juce::jlimit(0.0f, 1.0f, norm);
    repaint();
}

void WaveformView::setDurationSeconds(double secs)
{
    durationSec = juce::jmax(0.1, secs);
    repaint();
}

void WaveformView::setZoom(float factor)
{
    float clamped = juce::jlimit(1.0f, 16.0f, factor);
    if (juce::approximatelyEqual(clamped, zoom)) return;
    zoom = clamped;
    repaint();
}

void WaveformView::zoomIn()
{
    setZoom(zoom * 2.0f);
}

void WaveformView::zoomOut()
{
    setZoom(zoom * 0.5f);
}

void WaveformView::resized()
{
    // nothing to cache yet (lightweight)
}

void WaveformView::timerCallback()
{
    // For demo, animate playhead slowly
    float p = playheadNorm + 0.005f;
    if (p > 1.0f) p = 0.0f;
    setPlayhead(p);
}

void WaveformView::paint(juce::Graphics& g)
{
    auto b = getLocalBounds().toFloat();
    // Background card
    g.setColour(juce::Colour(0xFF151922));
    g.fillRoundedRectangle(b, 8.0f);
    g.setColour(juce::Colours::white.withAlpha(0.06f));
    g.drawRoundedRectangle(b, 8.0f, 1.0f);

    // Timeline grid
    g.setColour(juce::Colours::white.withAlpha(0.06f));
    double visible = durationSec / juce::jmax(1.0f, zoom);
    const int seconds = (int) std::ceil(visible);
    for (int s = 0; s <= seconds; ++s) {
        float x = b.getX() + (float)s / (float)juce::jmax(1, seconds) * b.getWidth();
        g.drawLine(x, b.getY(), x, b.getBottom(), 1.0f);
        if (s < seconds) {
            juce::String label = juce::String(s) + "s";
            g.setColour(juce::Colours::white.withAlpha(0.35f));
            g.setFont(12.0f);
            g.drawFittedText(label, juce::Rectangle<int>((int)x+2, (int)b.getY()+2, 40, 16), juce::Justification::left, 1);
            g.setColour(juce::Colours::white.withAlpha(0.06f));
        }
    }

    // Waveform path
    if (samples.size() > 1) {
        juce::Path p;
        float midY = b.getCentreY();
        float half = b.getHeight() * 0.4f;
        // Draw portion of samples that fits visible window (no pan: anchored at start)
        int N = samples.size();
        int visibleSamples = (int) juce::jmax(2.0, (double)N / juce::jmax(1.0f, zoom));
        for (int i = 0; i < visibleSamples; ++i) {
            float t = (float)i / (float)(visibleSamples - 1);
            float x = b.getX() + t * b.getWidth();
            float y = midY - samples.getUnchecked(i) * half;
            if (i == 0) p.startNewSubPath(x, y);
            else p.lineTo(x, y);
        }
        g.setColour(juce::Colour(0xFF5AA8FF));
        g.strokePath(p, juce::PathStrokeType(1.5f));
    }

    // Playhead cursor
    float x = b.getX() + playheadNorm * b.getWidth();
    g.setColour(juce::Colours::white.withAlpha(0.85f));
    g.drawLine(x, b.getY(), x, b.getBottom(), 1.5f);
}

void WaveformView::mouseWheelMove (const juce::MouseEvent&, const juce::MouseWheelDetails& wheel)
{
    if (wheel.deltaY > 0.0f) zoomIn();
    else if (wheel.deltaY < 0.0f) zoomOut();
}
