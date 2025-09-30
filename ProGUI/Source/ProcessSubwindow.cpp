#include "ProcessSubwindow.h"
#include "ControlInfoWindow.h"

ProcessSubwindow::ProcessSubwindow(const juce::String& name, const juce::String& desc)
    : processName(name), processDescription(desc)
{
    createGlobalControls();
    // Don't call createControls() here - subclasses will call it in their constructor
}

void ProcessSubwindow::paint(juce::Graphics& g)
{
    auto bounds = getLocalBounds();
    
    // Background
    g.fillAll(juce::Colour(0xFF1a1a2e));
    
    // Header area
    auto headerArea = bounds.removeFromTop(80);
    paintHeader(g, headerArea);
    
    // Meter column (right side)
    auto meterArea = bounds.removeFromRight(120);
    paintMeters(g, meterArea);
    
    // Footer area
    auto footerArea = bounds.removeFromBottom(100);
    paintFooter(g, footerArea);
    
    // Main control area background
    g.setColour(juce::Colour(0xFF16213e));
    g.fillRoundedRectangle(bounds.toFloat(), 8.0f);
}

void ProcessSubwindow::resized()
{
    auto bounds = getLocalBounds();
    
    // Header layout
    auto headerArea = bounds.removeFromTop(80);
    layoutHeader(headerArea);
    
    // Meter column
    auto meterArea = bounds.removeFromRight(120);
    // Meters are painted, not components
    
    // Footer layout
    auto footerArea = bounds.removeFromBottom(100);
    layoutFooter(footerArea);
    
    // Main control area for subclasses
    auto controlArea = bounds.reduced(10);
    layoutControls(controlArea);
}

std::unique_ptr<SkinnedKnob> ProcessSubwindow::createKnob(const juce::String& name, float min, float max, float defaultVal)
{
    auto knob = std::make_unique<SkinnedKnob>();
    knob->setRange(min, max, 0.01);
    knob->setValue(defaultVal);
    knob->setName(name);
    addAndMakeVisible(*knob);
    return knob;
}

std::unique_ptr<SkinnedSlider> ProcessSubwindow::createSlider(const juce::String& name, float min, float max, float defaultVal)
{
    auto slider = std::make_unique<SkinnedSlider>();
    slider->setRange(min, max, 0.01);
    slider->setValue(defaultVal);
    slider->setName(name);
    addAndMakeVisible(*slider);
    return slider;
}

std::unique_ptr<juce::ComboBox> ProcessSubwindow::createComboBox(const juce::String& name, const juce::StringArray& items)
{
    auto combo = std::make_unique<juce::ComboBox>(name);
    combo->addItemList(items, 1);
    combo->setSelectedItemIndex(0);
    addAndMakeVisible(*combo);
    return combo;
}

std::unique_ptr<juce::ToggleButton> ProcessSubwindow::createToggleButton(const juce::String& name)
{
    auto button = std::make_unique<juce::ToggleButton>(name);
    addAndMakeVisible(*button);
    return button;
}

void ProcessSubwindow::createGlobalControls()
{
    // Preset selector
    juce::StringArray presets = {"Instrument", "Buss", "Master", "Other"};
    presetCombo = createComboBox("Preset", presets);
    
    // Engine selector
    juce::StringArray engines = {"Auto", "CPU", "GPU"};
    engineCombo = createComboBox("Engine", engines);
    
    // Quality selector
    juce::StringArray qualities = {"Realtime", "Offline"};
    qualityCombo = createComboBox("Quality", qualities);
    
    // Amount knob
    amountKnob = createKnob("Amount", 0.0f, 100.0f, 50.0f);
    
    // Bypass button
    bypassButton = createToggleButton("Bypass");
    
    // Bake button
    bakeButton = std::make_unique<juce::TextButton>("Bake It In");
    addAndMakeVisible(*bakeButton);
    
    // I/O trims
    inputTrimSlider = createSlider("Input Trim", -24.0f, 24.0f, 0.0f);
    outputTrimSlider = createSlider("Output Trim", -24.0f, 24.0f, 0.0f);
    
    // Info button
    infoButton = std::make_unique<juce::TextButton>("INFO");
    infoButton->setColour(juce::TextButton::buttonColourId, juce::Colour(0xFF4a90e2));
    infoButton->setColour(juce::TextButton::textColourOffId, juce::Colours::white);
    infoButton->onClick = [this]() { showInfoWindow(); };
    addAndMakeVisible(*infoButton);
}

void ProcessSubwindow::layoutHeader(juce::Rectangle<int> headerArea)
{
    auto leftSide = headerArea.removeFromLeft(headerArea.getWidth() / 2);
    auto rightSide = headerArea;
    
    // Left side: Title and description
    // (painted, not components)
    
    // Right side: Preset, Engine, Quality, Amount, Bypass
    auto topRow = rightSide.removeFromTop(40);
    auto bottomRow = rightSide;
    
    presetCombo->setBounds(topRow.removeFromLeft(120).reduced(2));
    engineCombo->setBounds(topRow.removeFromLeft(80).reduced(2));
    qualityCombo->setBounds(topRow.removeFromLeft(100).reduced(2));
    
    // INFO button in top row
    infoButton->setBounds(topRow.removeFromLeft(60).reduced(2));
    
    amountKnob->setBounds(bottomRow.removeFromLeft(60).reduced(5));
    bypassButton->setBounds(bottomRow.removeFromLeft(80).reduced(2));
}

void ProcessSubwindow::layoutFooter(juce::Rectangle<int> footerArea)
{
    auto leftSide = footerArea.removeFromLeft(100);
    auto rightSide = footerArea.removeFromRight(100);
    auto centerArea = footerArea;
    
    // Left: Input trim
    inputTrimSlider->setBounds(leftSide.reduced(10));
    
    // Right: Output trim
    outputTrimSlider->setBounds(rightSide.reduced(10));
    
    // Center: Waveform, Transport, Bake button
    auto waveformArea = centerArea.removeFromTop(40);
    auto transportArea = centerArea.removeFromTop(30);
    auto bakeArea = centerArea;
    
    bakeButton->setBounds(bakeArea.reduced(10));
}

void ProcessSubwindow::paintHeader(juce::Graphics& g, juce::Rectangle<int> headerArea)
{
    auto leftSide = headerArea.removeFromLeft(headerArea.getWidth() / 2);
    
    // Process name
    g.setColour(juce::Colours::white);
    g.setFont(juce::Font("Arial", 18.0f, juce::Font::bold));
    auto titleArea = leftSide.removeFromTop(30);
    g.drawText(processName, titleArea, juce::Justification::centredLeft);
    
    // Process description
    g.setColour(juce::Colour(0xFF00ffff));
    g.setFont(juce::Font("Arial", 12.0f, juce::Font::plain));
    g.drawText(processDescription, leftSide, juce::Justification::topLeft, true);
}

void ProcessSubwindow::paintFooter(juce::Graphics& g, juce::Rectangle<int> footerArea)
{
    auto leftSide = footerArea.removeFromLeft(100);
    auto rightSide = footerArea.removeFromRight(100);
    auto centerArea = footerArea;
    
    // Input/Output labels
    g.setColour(juce::Colours::white);
    g.setFont(juce::Font("Arial", 10.0f, juce::Font::bold));
    g.drawText("INPUT", leftSide.removeFromTop(15), juce::Justification::centred);
    g.drawText("OUTPUT", rightSide.removeFromTop(15), juce::Justification::centred);
    
    // Waveform placeholder
    auto waveformArea = centerArea.removeFromTop(40);
    g.setColour(juce::Colour(0xFF333333));
    g.fillRoundedRectangle(waveformArea.toFloat().reduced(5), 4.0f);
    g.setColour(juce::Colours::white);
    g.setFont(juce::Font("Arial", 10.0f, juce::Font::plain));
    g.drawText("Waveform", waveformArea, juce::Justification::centred);
    
    // Transport placeholder
    auto transportArea = centerArea.removeFromTop(30);
    g.setColour(juce::Colour(0xFF444444));
    g.fillRoundedRectangle(transportArea.toFloat().reduced(5), 4.0f);
    g.drawText("Transport", transportArea, juce::Justification::centred);
}

void ProcessSubwindow::showInfoWindow()
{
    // Create basic info structure - subclasses can override this method for specific info
    ControlInfoWindow::ControlInfo info;
    info.title = processName + " - PROFESSIONAL CONTROLS";
    info.overview = processDescription;
    info.technicalDetails = "This is a professional audio processing module with advanced controls.";
    info.practicalUsage = "Use this process to enhance your audio with professional-grade processing.";
    info.commonSettings = "Adjust controls based on your specific audio material and desired outcome.";
    info.accentColor = juce::Colour(0xFF4a90e2);
    
    // Create and show ControlInfoWindow
    auto* infoWindow = new ControlInfoWindow(info);
    
    juce::DialogWindow::LaunchOptions opts;
    opts.content.setOwned(infoWindow);
    opts.dialogTitle = processName + " - Information";
    opts.componentToCentreAround = this;
    opts.escapeKeyTriggersCloseButton = true;
    opts.useNativeTitleBar = true;
    opts.resizable = false;
    opts.launchAsync();
}
