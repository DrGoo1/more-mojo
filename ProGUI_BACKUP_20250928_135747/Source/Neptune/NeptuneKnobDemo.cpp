#include "NeptuneKnobDemo.h"

using namespace juce;

static void loadPngsFromDir (const File& dir, OwnedArray<Image>& outFrames)
{
    outFrames.clear(true);
    if (!dir.isDirectory()) return;

    Array<File> files;
    dir.findChildFiles(files, File::findFiles, false, "*.png");

    files.sort();
    for (auto& f : files)
    {
        Image img = ImageFileFormat::loadFrom(f);
        if (img.isValid())
            outFrames.add(new Image(std::move(img)));
    }
}

void NeptuneKnobDemo::KnobSeq::loadFromDir (const File& dir)
{
    frames.clear(true);
    loadPngsFromDir(dir, frames);
    frameCount = frames.size();
}

void NeptuneKnobDemo::KnobSeq::draw(Graphics& g, Rectangle<int> r, float normPos)
{
    if (frameCount <= 0) { g.drawFittedText("no frames", r, Justification::centred, 1); return; }

    normPos = jlimit(0.0f, 1.0f, normPos);
    const int idx = jlimit(0, frameCount - 1, roundToInt(normPos * float(frameCount - 1)));

    Image* frame = frames[idx];
    if (frame == nullptr || !frame->isValid()) return;

    auto dst = r.withSizeKeepingCentre(targetW, targetH);
    g.drawImage(*frame, dst.toFloat());
}

NeptuneKnobDemo::NeptuneKnobDemo(const File& bigDir,
                                 const File& medDir,
                                 const File& smallDir)
{
    big.targetW = big.targetH = 120;
    med.targetW = med.targetH = 80;
    small.targetW = small.targetH = 60;

    big.loadFromDir(bigDir);
    med.loadFromDir(medDir);
    small.loadFromDir(smallDir);

    addAndMakeVisible(testButton);
    addAndMakeVisible(testSlider);
    addAndMakeVisible(testCombo);

    testSlider.setRange(0.0, 1.0, 0.0001);
    testSlider.setValue(valueNorm);
    testSlider.onValueChange = [this] { valueNorm = (float)testSlider.getValue(); repaint(); };

    testCombo.addItem("Option A", 1);
    testCombo.addItem("Option B", 2);
    testCombo.addItem("Option C", 3);
    testCombo.setSelectedId(1);

    startTimerHz(30); // animate a tiny bit so you can see frames changing if desired
}

void NeptuneKnobDemo::resized()
{
    auto bounds = getLocalBounds().reduced(10);

    // top: the three knobs
    auto top = bounds.removeFromTop(140);
    auto third = top.getWidth() / 3;
    auto r1 = top.removeFromLeft(third).reduced(10);
    auto r2 = top.removeFromLeft(third).reduced(10);
    auto r3 = top.reduced(10);

    // bottom: widgets
    auto row = bounds.removeFromTop(80);
    testButton.setBounds(row.removeFromLeft(140).reduced(10));
    testCombo.setBounds(row.removeFromLeft(180).reduced(10));
    testSlider.setBounds(bounds.removeFromTop(60).reduced(10));

    // store knob rects using component coordinates via repaint in paint()
    // (We'll just repaint using getLocalBounds() again.)
}

void NeptuneKnobDemo::paint(Graphics& g)
{
    g.fillAll(Colours::black.withAlpha(0.8f));

    auto area = getLocalBounds().reduced(10);
    auto top = area.removeFromTop(140);
    auto third = top.getWidth() / 3;
    auto r1 = top.removeFromLeft(third).reduced(10);
    auto r2 = top.removeFromLeft(third).reduced(10);
    auto r3 = top.reduced(10);

    // Draw three sizes with the same normalized value
    big.draw(g, r1, valueNorm);
    med.draw(g, r2, valueNorm);
    small.draw(g, r3, valueNorm);

    g.setColour(Colours::white.withAlpha(0.8f));
    g.drawFittedText("Neptune Knob Demo (120 / 80 / 60)", area.removeFromTop(20), Justification::centred, 1);
}

void NeptuneKnobDemo::timerCallback()
{
    // If you want subtle animation uncomment:
    // valueNorm = fmod (valueNorm + 0.002f, 1.0f);
    // repaint();
}
