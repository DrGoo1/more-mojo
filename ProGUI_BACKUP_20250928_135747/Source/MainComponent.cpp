#include "MainComponent.h"

MainComponent::MainComponent()
{
    // Apply custom look and feel
    juce::LookAndFeel::setDefaultLookAndFeel(&customLookAndFeel);
    
    // Initialize all UI components
    initializeComponents();
    
    // Set up the component (after children exist to avoid early resized() crashes)
    setSize(1200, 800);
}

MainComponent::~MainComponent()
{
    // Reset the look and feel to default before destruction
    juce::LookAndFeel::setDefaultLookAndFeel(nullptr);
    
    // Remove listeners
    if (tabComponent)
        tabComponent->getTabbedButtonBar().removeChangeListener(this);
}

void MainComponent::paint(juce::Graphics& g)
{
    // Professional dark gradient background
    g.setGradientFill(juce::ColourGradient(
        juce::Colour(0xff1a1a1a), 0, 0,
        juce::Colour(0xff101010), 0, getHeight(),
        false));
    g.fillAll();
    
    // Add subtle pattern overlay for depth
    g.setOpacity(0.03f);
    for (int y = 0; y < getHeight(); y += 2) {
        g.setColour(juce::Colours::white);
        g.drawHorizontalLine(y, 0, getWidth());
    }
}

void MainComponent::resized()
{
    auto area = getLocalBounds().reduced(12);
    
    // Title bar area
    auto titleArea = area.removeFromTop(50);
    titleLabel.setBounds(titleArea.removeFromLeft(300));
    presetButton.setBounds(titleArea.removeFromRight(120).reduced(5));
    settingsButton.setBounds(titleArea.removeFromRight(120).reduced(5));
    
    // Status bar area
    auto statusArea = area.removeFromBottom(30);
    statusLabel.setBounds(statusArea.removeFromLeft(300));
    cpuUsageLabel.setBounds(statusArea.removeFromRight(150));
    
    // Main content area with tabs
    if (tabComponent)
        tabComponent->setBounds(area);
}

void MainComponent::changeListenerCallback(juce::ChangeBroadcaster* source)
{
    // Handle tab changes
    if (tabComponent && source == &tabComponent->getTabbedButtonBar())
        repaint();
}

void MainComponent::initializeComponents()
{
    // Set up title label
    titleLabel.setText("Audio Processor Pro", juce::dontSendNotification);
    titleLabel.setFont(juce::Font(28.0f, juce::Font::bold));
    titleLabel.setJustificationType(juce::Justification::centredLeft);
    titleLabel.setColour(juce::Label::textColourId, juce::Colours::white);
    addAndMakeVisible(titleLabel);
    
    // Set up buttons
    settingsButton.setButtonText("Settings");
    settingsButton.setColour(juce::TextButton::buttonColourId, juce::Colour(0xff444444));
    settingsButton.setColour(juce::TextButton::textColourOffId, juce::Colours::white);
    addAndMakeVisible(settingsButton);
    
    presetButton.setButtonText("Presets");
    presetButton.setColour(juce::TextButton::buttonColourId, juce::Colour(0xff444444));
    presetButton.setColour(juce::TextButton::textColourOffId, juce::Colours::white);
    addAndMakeVisible(presetButton);
    
    // Set up status indicators
    statusLabel.setText("Processing active", juce::dontSendNotification);
    statusLabel.setFont(juce::Font(14.0f));
    statusLabel.setColour(juce::Label::textColourId, juce::Colour(0xff88ff88));
    addAndMakeVisible(statusLabel);
    
    cpuUsageLabel.setText("CPU: 12% | Latency: 2.1ms", juce::dontSendNotification);
    cpuUsageLabel.setFont(juce::Font(14.0f));
    cpuUsageLabel.setColour(juce::Label::textColourId, juce::Colours::lightgrey);
    addAndMakeVisible(cpuUsageLabel);
    
    // Create processor components
    eqComponent.reset(new ProEQComponent());
    compressorComponent.reset(new ProCompressorComponent());
    
    // Set up tab component
    tabComponent.reset(new juce::TabbedComponent(juce::TabbedButtonBar::TabsAtTop));
    tabComponent->setOutline(0);
    tabComponent->setTabBarDepth(40);
    tabComponent->addTab("EQ", juce::Colours::transparentBlack, eqComponent.get(), true);
    tabComponent->addTab("Compressor", juce::Colours::transparentBlack, compressorComponent.get(), true);
    tabComponent->addTab("Saturation", juce::Colours::transparentBlack, nullptr, false);
    tabComponent->addTab("Limiter", juce::Colours::transparentBlack, nullptr, false);
    tabComponent->addTab("Analyzer", juce::Colours::transparentBlack, nullptr, false);
    tabComponent->getTabbedButtonBar().addChangeListener(this);
    addAndMakeVisible(tabComponent.get());
}
