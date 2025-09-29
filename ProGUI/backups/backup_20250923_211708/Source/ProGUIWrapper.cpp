#include "ProGUIWrapper.h"

static void progui_log (const juce::String& msg)
{
    juce::File f ("/tmp/progui_demo.log");
    auto ts = juce::Time::getCurrentTime().toString (true, true);
    f.appendText ("[C++] " + ts + " " + msg + "\n");
}

ProGUIWrapper::ProGUIWrapper(ComponentType type)
    : activeComponentType(type)
{
    progui_log ("ProGUIWrapper ctor, type=" + juce::String ((int) type));
    // Create the requested component type
    switch (type)
    {
        case EQComponent:
            eqComponent = std::make_unique<ProEQComponent>();
            addAndMakeVisible(eqComponent.get());
            break;
            
        case CompressorComponent:
            compressorComponent = std::make_unique<ProCompressorComponent>();
            addAndMakeVisible(compressorComponent.get());
            break;

        case DemoComponent:
            demoComponent = std::make_unique<UIDemoComponent>();
            addAndMakeVisible(demoComponent.get());
            progui_log ("Created UIDemoComponent and added to wrapper");
            break;
    }
    // Create overlay (initially hidden)
    consumerOverlay = std::make_unique<ConsumerOverlay>();
    consumerOverlay->setInterceptsMouseClicks(false, false);
    consumerOverlay->setVisible(false);
    addAndMakeVisible(consumerOverlay.get());

    // Defaults: hide overlay so demo controls are unobscured; enable test signal for meters
    setOverlayVisible(false);
    setTestSignalEnabled(true);
}

ProGUIWrapper::~ProGUIWrapper()
{
    // Clean up components
    eqComponent = nullptr;
    compressorComponent = nullptr;
    nativeWrapper = nullptr;
}

void ProGUIWrapper::paint(juce::Graphics& g)
{
    // Fill background with dark gray
    g.fillAll(juce::Colour(0xff202020));
    // Visible debug border and label to confirm wrapper is painting
    g.setColour(juce::Colours::red);
    g.drawRect(getLocalBounds(), 2);
    g.setColour(juce::Colours::white);
    g.drawText("[C++] ProGUIWrapper painting", getLocalBounds().removeFromTop(20), juce::Justification::centred);
}

void ProGUIWrapper::resized()
{
    auto bounds = getLocalBounds();
    progui_log ("ProGUIWrapper::resized bounds=" + bounds.toString());
    
    // Resize the active component
    switch (activeComponentType)
    {
        case EQComponent:
            if (eqComponent)
                eqComponent->setBounds(bounds);
            break;
            
        case CompressorComponent:
            if (compressorComponent)
                compressorComponent->setBounds(bounds);
            break;

        case DemoComponent:
            if (demoComponent)
                demoComponent->setBounds(bounds);
            break;
    }
    // Layout overlay on top
    if (consumerOverlay)
        consumerOverlay->setBounds(bounds);
}

void ProGUIWrapper::setParameterValue(const char* parameterId, float value)
{
    juce::String paramId(parameterId);
    
    // UI parameters handled here
    if (paramId == "ui.overlayVisible")
    {
        setOverlayVisible(value > 0.5f);
        return;
    }
    if (paramId == "ui.testSignal")
    {
        setTestSignalEnabled(value > 0.5f);
        return;
    }

    // Route parameter to appropriate component
    if (activeComponentType == EQComponent && eqComponent)
    {
        // Parse EQ parameters (format: eq.band1.gain, eq.band2.freq, etc.)
        if (paramId.startsWith("eq.band"))
        {
            int bandIndex = paramId.substring(7, 8).getIntValue() - 1; // Extract band number
            
            if (paramId.contains("gain"))
                eqComponent->setBandGain(bandIndex, value);
            else if (paramId.contains("freq"))
                eqComponent->setBandFrequency(bandIndex, value);
            else if (paramId.contains("q"))
                eqComponent->setBandQ(bandIndex, value);
        }
    }
    else if (activeComponentType == CompressorComponent && compressorComponent)
    {
        // Parse compressor parameters
        if (paramId == "comp.threshold")
            compressorComponent->setThreshold(value);
        else if (paramId == "comp.ratio")
            compressorComponent->setRatio(value);
        else if (paramId == "comp.attack")
            compressorComponent->setAttack(value);
        else if (paramId == "comp.release")
            compressorComponent->setRelease(value);
        else if (paramId == "comp.knee")
            compressorComponent->setKnee(value);
        else if (paramId == "comp.makeup")
            compressorComponent->setMakeupGain(value);
    }
}

float ProGUIWrapper::getParameterValue(const char* parameterId)
{
    juce::String paramId(parameterId);
    
    // Get parameter from appropriate component
    if (activeComponentType == EQComponent && eqComponent)
    {
        // Parse EQ parameters
        if (paramId.startsWith("eq.band"))
        {
            int bandIndex = paramId.substring(7, 8).getIntValue() - 1; // Extract band number
            
            if (paramId.contains("gain"))
                return eqComponent->getBandGain(bandIndex);
            else if (paramId.contains("freq"))
                return eqComponent->getBandFrequency(bandIndex);
            else if (paramId.contains("q"))
                return eqComponent->getBandQ(bandIndex);
        }
    }
    else if (activeComponentType == CompressorComponent && compressorComponent)
    {
        // Parse compressor parameters
        if (paramId == "comp.threshold")
            return compressorComponent->getThreshold();
        else if (paramId == "comp.ratio")
            return compressorComponent->getRatio();
        else if (paramId == "comp.attack")
            return compressorComponent->getAttack();
        else if (paramId == "comp.release")
            return compressorComponent->getRelease();
        else if (paramId == "comp.knee")
            return compressorComponent->getKnee();
        else if (paramId == "comp.makeup")
            return compressorComponent->getMakeupGain();
    }
    
    return 0.0f;
}

void ProGUIWrapper::processAudio(float** audioData, int numChannels, int numSamples)
{
    // Create a JUCE audio buffer from the raw data
    juce::AudioBuffer<float> buffer(audioData, numChannels, numSamples);
    
    // Process audio through the active component
    switch (activeComponentType)
    {
        case EQComponent:
            if (eqComponent)
                eqComponent->processAudio(buffer);
            break;
            
        case CompressorComponent:
            if (compressorComponent)
                compressorComponent->processAudio(buffer);
            break;
    }

    // Update overlay meters (simple peak meter)
    if (consumerOverlay)
    {
        float peakL = 0.0f, peakR = 0.0f;
        if (numChannels > 0)
        {
            auto* l = buffer.getReadPointer(0);
            for (int i = 0; i < numSamples; ++i) peakL = std::max(peakL, std::abs(l[i]));
        }
        if (numChannels > 1)
        {
            auto* r = buffer.getReadPointer(1);
            for (int i = 0; i < numSamples; ++i) peakR = std::max(peakR, std::abs(r[i]));
        }
        consumerOverlay->setLevels(juce::jlimit(0.0f, 1.0f, peakL), juce::jlimit(0.0f, 1.0f, peakR));
    }
}

void* ProGUIWrapper::getNativeView()
{
    // Return the native NSView* for the active child component
    if (activeComponentType == EQComponent && eqComponent)
    {
        if (eqComponent->getPeer() == nullptr)
            eqComponent->addToDesktop(0);
        if (auto* peer = eqComponent->getPeer())
            return peer->getNativeHandle();
    }
    else if (activeComponentType == CompressorComponent && compressorComponent)
    {
        if (compressorComponent->getPeer() == nullptr)
            compressorComponent->addToDesktop(0);
        if (auto* peer = compressorComponent->getPeer())
            return peer->getNativeHandle();
    }
    else if (activeComponentType == DemoComponent && demoComponent)
    {
        if (demoComponent->getPeer() == nullptr)
            demoComponent->addToDesktop(0);
        if (auto* peer = demoComponent->getPeer())
            return peer->getNativeHandle();
    }
    return nullptr;
}

void ProGUIWrapper::attachToParent(void* parentNSView)
{
    // Attach the active child component to the provided NSView so it renders inside SwiftUI container
    juce::Component* child = nullptr;
    switch (activeComponentType)
    {
        case EQComponent:          child = eqComponent.get(); break;
        case CompressorComponent:  child = compressorComponent.get(); break;
        case DemoComponent:        child = demoComponent.get(); break;
    }
    if (child == nullptr)
        return;

    // Ensure child has correct bounds
    child->setBounds(getLocalBounds());
    progui_log ("attachToParent called. Child bounds=" + child->getBounds().toString());

    if (child->getPeer() == nullptr)
    {
        // Attach to parent NSView, not creating a top-level window
        child->addToDesktop(0, parentNSView);
        progui_log ("child->addToDesktop executed (no peer previously)");
        child->setVisible(true);
        child->toFront(true);
    }
    else
    {
        // Ensure visible if peer already exists
        if (auto* peer = child->getPeer())
        {
            peer->setVisible(true);
            progui_log ("child already had peer; setVisible(true)");
        }
        child->setVisible(true);
        child->toFront(true);
    }
}

// ===== Overlay Controls =====
void ProGUIWrapper::setOverlayVisible(bool shouldShow)
{
    overlayVisible = shouldShow;
    if (consumerOverlay)
    {
        consumerOverlay->setVisible(overlayVisible);
        if (overlayVisible)
            consumerOverlay->toFront(false);
    }
}

// ===== Test Signal Controls =====
void ProGUIWrapper::setTestSignalEnabled(bool enabled)
{
    if (testSignalEnabled == enabled) return;
    testSignalEnabled = enabled;
    if (testSignalEnabled)
        startTimerHz(30);
    else
        stopTimer();
}

void ProGUIWrapper::timerCallback()
{
    if (!testSignalEnabled)
        return;

    constexpr int numChannels = 2;
    constexpr int numSamples = 512;
    juce::AudioBuffer<float> buffer(numChannels, numSamples);
    auto* l = buffer.getWritePointer(0);
    auto* r = buffer.getWritePointer(1);

    const double twoPi = juce::MathConstants<double>::twoPi;
    const double inc = twoPi * testFrequency / testSampleRate;

    for (int i = 0; i < numSamples; ++i)
    {
        float sample = (float) std::sin(testPhase);
        testPhase += inc;
        if (testPhase > twoPi) testPhase -= twoPi;
        l[i] = sample * 0.2f;
        r[i] = sample * 0.2f;
    }

    if (activeComponentType == EQComponent && eqComponent)
        eqComponent->processAudio(buffer);
    else if (activeComponentType == CompressorComponent && compressorComponent)
        compressorComponent->processAudio(buffer);

    float peakL = 0.0f, peakR = 0.0f;
    for (int i = 0; i < numSamples; ++i) { peakL = std::max(peakL, std::abs(l[i])); }
    for (int i = 0; i < numSamples; ++i) { peakR = std::max(peakR, std::abs(r[i])); }
    if (consumerOverlay)
        consumerOverlay->setLevels(peakL, peakR);
}
