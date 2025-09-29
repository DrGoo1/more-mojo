#include "LedBarMeter.h"

LedBarMeter::LedBarMeter() : LedBarMeter(Config{}) {}

LedBarMeter::LedBarMeter(const Config& cfg) : config(cfg)
{
    setOpaque(true);
    startTimerHz(60);
}

void LedBarMeter::setSpriteImage(const juce::Image& img)
{
    sprite = img;
    for (int i = 0; i < 4; ++i) spriteFrames[i] = {};
    if (sprite.isValid())
    {
        const int fw = juce::jmax(1, sprite.getWidth() / 4);
        const int fh = sprite.getHeight();
        for (int i = 0; i < 4; ++i)
            spriteFrames[i] = { i * fw, 0, fw, fh };
    }
    repaint();
}

void LedBarMeter::setPeakNorm(float norm)
{
    norm = juce::jlimit(0.0f, 1.0f, norm);
    // Update peak & hold
    float current = peakNorm.load(std::memory_order_relaxed);
    if (norm >= current) {
        peakNorm.store(norm, std::memory_order_relaxed);
        heldPeak  = norm;
        holdTEnd  = juce::Time::getMillisecondCounterHiRes() / 1000.0 + config.peakHoldSeconds;
    } else {
        // falling handled in timer
        peakNorm.store(juce::jmax(norm, current), std::memory_order_relaxed);
    }
}

void LedBarMeter::setRmsNorm(float norm)
{
    rmsNorm.store(juce::jlimit(0.0f, 1.0f, norm), std::memory_order_relaxed);
}

void LedBarMeter::resized()
{
    segmentRects.clear();
    auto r = getLocalBounds().toFloat().reduced(6.0f);

    const int N = juce::jmax(4, config.numSegments);
    if (config.orientation == Vertical)
    {
        const float gap = 1.5f;
        float segH = (r.getHeight() - gap * (N - 1)) / (float) N;
        float y = r.getBottom() - segH;
        for (int i = 0; i < N; ++i) {
            segmentRects.add(juce::Rectangle<float>(r.getX(), y, r.getWidth(), segH));
            y -= (segH + gap);
        }
    }
    else
    {
        const float gap = 1.5f;
        float segW = (r.getWidth() - gap * (N - 1)) / (float) N;
        float x = r.getX();
        for (int i = 0; i < N; ++i) {
            segmentRects.add(juce::Rectangle<float>(x, r.getY(), segW, r.getHeight()));
            x += (segW + gap);
        }
    }
}

void LedBarMeter::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colour(0xFF0E1015));

    // optional inner card
    auto inner = getLocalBounds().reduced(3).toFloat();
    g.setColour(juce::Colour(0xFF19202A));
    g.fillRoundedRectangle(inner, 8.0f);
    g.setColour(juce::Colours::white.withAlpha(0.06f));
    g.drawRoundedRectangle(inner, 8.0f, 1.0f);

    const float p  = peakNorm.load(std::memory_order_relaxed);
    const float r  = rmsNorm.load (std::memory_order_relaxed);

    const int N = juce::jmax(4, config.numSegments);
    int litCountPeak = (int) std::round(p * N);
    int litCountRms  = (int) std::round(r * N);
    litCountPeak = juce::jlimit(0, N, litCountPeak);
    litCountRms  = juce::jlimit(0, N, litCountRms);

    // Draw segments (from bottom/left to top/right)
    for (int i = 0; i < N; ++i)
    {
        const bool lit = (i < litCountPeak);
        const int  k   = litKindForSegment(i);
        auto segR = segmentRects[i];

        if (sprite.isValid())
            drawSegment(g, segR, lit, k);
        else
        {
            // Vector fallback
            juce::Colour c = (k == 3 ? juce::Colours::red
                          : k == 2 ? juce::Colours::yellow
                                   : juce::Colours::limegreen);
            drawVectorSegment(g, segR, lit, c);
        }

        // RMS overlay: faint inner fill up to RMS count
        if (config.showRmsOverlay && i < litCountRms)
        {
            auto overlay = segR.reduced(segR.getHeight() * 0.18f, segR.getWidth() * 0.18f);
            g.setColour(juce::Colours::white.withAlpha(0.10f));
            g.fillRoundedRectangle(overlay, juce::jmin(overlay.getWidth(), overlay.getHeight()) * 0.25f);
        }
    }

    // Peak hold marker (thin white bar) at heldPeak position
    const int holdSeg = juce::jlimit(0, N-1, (int) std::round(heldPeak * N));
    if (holdSeg >= 0 && holdSeg < N)
    {
        auto segR = segmentRects[holdSeg];
        g.setColour(juce::Colours::white.withAlpha(0.9f));
        if (config.orientation == Vertical)
            g.fillRect(segR.withHeight(2.0f));
        else
            g.fillRect(segR.withWidth(2.0f));
    }
}

void LedBarMeter::timerCallback()
{
    // Simple ballistic fall on peak and peak hold expiration
    const double now = juce::Time::getMillisecondCounterHiRes() / 1000.0;
    float p = peakNorm.load(std::memory_order_relaxed);

    // fall peak (in normalized domain, approximated)
    const float fallPerFrame = (config.fallRateDbPerSec / 60.0f) / 60.0f; // small step
    p = juce::jlimit(0.0f, 1.0f, p - fallPerFrame);
    peakNorm.store(p, std::memory_order_relaxed);

    if (now > holdTEnd)
        heldPeak = juce::jmax(heldPeak - fallPerFrame * 2.0f, p);

    repaint();
}

// 1=green, 2=yellow, 3=red based on the *top* of segment's normalized position
int LedBarMeter::litKindForSegment(int segIndex) const
{
    const int N = juce::jmax(4, config.numSegments);
    // normalized top of this segment
    float norm = (segIndex + 1) / (float) N;
    if (norm >= config.redStartNorm)    return 3;
    if (norm >= config.yellowStartNorm) return 2;
    return 1;
}

void LedBarMeter::drawSegment(juce::Graphics& g, juce::Rectangle<float> r, bool lit, int litKind)
{
    if (!sprite.isValid()) return;

    const int idx = juce::jlimit(0, 3, lit ? litKind : 0);
    const juce::Rectangle<int> src = spriteFrames[idx];
    // Maintain aspect ratio of sprite frame when fitting into segment cell
    auto dst = r;
    const float spriteAspect = (float) src.getWidth() / (float) src.getHeight();
    const float cellAspect   = r.getWidth() / r.getHeight();

    if (config.orientation == Vertical)
    {
        if (spriteAspect > cellAspect)
        {
            // fit width
            float h = r.getWidth() / spriteAspect;
            dst = juce::Rectangle<float>(r.getX(), r.getCentreY() - h*0.5f, r.getWidth(), h);
        }
        else
        {
            // fit height
            float w = r.getHeight() * spriteAspect;
            dst = juce::Rectangle<float>(r.getCentreX() - w*0.5f, r.getY(), w, r.getHeight());
        }
    }
    else
    {
        if (spriteAspect < cellAspect)
        {
            // fit height
            float w = r.getHeight() * spriteAspect;
            dst = juce::Rectangle<float>(r.getX(), r.getY(), w, r.getHeight());
        }
        else
        {
            float h = r.getWidth() / spriteAspect;
            dst = juce::Rectangle<float>(r.getX(), r.getCentreY() - h*0.5f, r.getWidth(), h);
        }
    }

    g.drawImage(sprite,
                (int) dst.getX(), (int) dst.getY(), (int) dst.getWidth(), (int) dst.getHeight(),
                src.getX(), src.getY(), src.getWidth(), src.getHeight(),
                false);
}

void LedBarMeter::drawVectorSegment(juce::Graphics& g, juce::Rectangle<float> r, bool lit, juce::Colour litColour)
{
    const float radius = juce::jmin(r.getWidth(), r.getHeight()) * 0.35f;
    // base glass
    juce::Colour glass = juce::Colour(0xFF1A212C);
    g.setColour(glass);
    g.fillRoundedRectangle(r, radius);

    // inner rim
    g.setColour(juce::Colours::black.withAlpha(0.35f));
    g.drawRoundedRectangle(r, radius, 1.5f);

    if (lit)
    {
        auto inner = r.reduced(r.getHeight()*0.18f, r.getWidth()*0.18f);
        juce::Colour glow = litColour.withAlpha(0.95f);
        juce::Colour core = juce::Colours::white.withAlpha(0.30f);

        // glow grad
        juce::ColourGradient grad(glow, inner.getCentreX(), inner.getCentreY(),
                                  glow.darker(0.4f), inner.getRight(), inner.getBottom(), true);
        g.setGradientFill(grad);
        g.fillRoundedRectangle(inner, radius * 0.6f);

        // specular line
        g.setColour(core);
        if (inner.getWidth() < inner.getHeight())
            g.fillRect(inner.withHeight(2.0f));
        else
            g.fillRect(inner.withWidth(2.0f));
    }
}
