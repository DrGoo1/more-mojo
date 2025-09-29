#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_gui_extra/juce_gui_extra.h>

class AudioGraphicsComponent : public juce::Component, private juce::Timer
{
public:
    enum GraphType
    {
        FrequencyAliasing,
        SamplingRate,
        FilterResponse,
        Waveform
    };
    
    AudioGraphicsComponent()
    {
        // Create graph type selector
        addAndMakeVisible(graphSelector);
        graphSelector.addItem("Frequency Aliasing", 1);
        graphSelector.addItem("Sampling Rate", 2);
        graphSelector.addItem("Filter Response", 3);
        graphSelector.addItem("Waveform Analysis", 4);
        graphSelector.setSelectedId(1);
        graphSelector.onChange = [this]() { repaint(); };
        
        // Animation control
        addAndMakeVisible(animateButton);
        animateButton.setButtonText("Animate");
        animateButton.setToggleable(true);
        animateButton.onClick = [this]() {
            if (animateButton.getToggleState())
                startTimerHz(30);
            else
                stopTimer();
        };
        
        setSize(800, 600);
        startTimerHz(30); // Start with animation
        animateButton.setToggleState(true, juce::dontSendNotification);
    }
    
    void paint(juce::Graphics& g) override
    {
        // Background gradient
        juce::ColourGradient gradient(juce::Colour(0xFF1a1a2e), 0, 0,
                                     juce::Colour(0xFF16213e), getWidth(), getHeight(), false);
        g.setGradientFill(gradient);
        g.fillAll();
        
        // Title
        g.setColour(juce::Colours::white);
        g.setFont(24.0f);
        g.drawText("Audio Processing Graphics", 10, 10, getWidth()-20, 40, juce::Justification::centred);
        
        // Draw selected graph
        auto graphArea = getLocalBounds().reduced(50).removeFromBottom(getHeight() - 100);
        
        switch (graphSelector.getSelectedId())
        {
            case 1: drawFrequencyAliasing(g, graphArea); break;
            case 2: drawSamplingRate(g, graphArea); break;
            case 3: drawFilterResponse(g, graphArea); break;
            case 4: drawWaveformAnalysis(g, graphArea); break;
        }
    }
    
    void resized() override
    {
        graphSelector.setBounds(50, 60, 200, 25);
        animateButton.setBounds(270, 60, 100, 25);
    }
    
private:
    void timerCallback() override
    {
        animationTime += 0.05f;
        repaint();
    }
    
    void drawFrequencyAliasing(juce::Graphics& g, juce::Rectangle<int> area)
    {
        g.setColour(juce::Colours::yellow);
        g.setFont(20.0f);
        g.drawText("Frequency Aliasing", area.getX(), area.getY(), area.getWidth(), 30, juce::Justification::centred);
        
        auto graphArea = area.reduced(0, 40);
        
        // Draw axes
        g.setColour(juce::Colours::white.withAlpha(0.3f));
        g.drawLine(graphArea.getX(), graphArea.getCentreY(), graphArea.getRight(), graphArea.getCentreY(), 1.0f);
        g.drawLine(graphArea.getCentreX(), graphArea.getY(), graphArea.getCentreX(), graphArea.getBottom(), 1.0f);
        
        // Draw true signal (high frequency)
        juce::Path truePath;
        g.setColour(juce::Colours::magenta);
        
        float frequency = 8.0f; // High frequency
        bool firstPoint = true;
        
        for (int x = 0; x < graphArea.getWidth(); x += 2)
        {
            float t = (float)x / graphArea.getWidth() * 4.0f * juce::MathConstants<float>::pi + animationTime;
            float y = graphArea.getCentreY() + std::sin(frequency * t) * graphArea.getHeight() * 0.3f;
            
            if (firstPoint)
            {
                truePath.startNewSubPath(graphArea.getX() + x, y);
                firstPoint = false;
            }
            else
            {
                truePath.lineTo(graphArea.getX() + x, y);
            }
        }
        
        g.strokePath(truePath, juce::PathStrokeType(2.0f));
        
        // Draw aliased signal (low frequency)
        juce::Path aliasedPath;
        g.setColour(juce::Colours::yellow);
        
        float aliasedFreq = 2.0f; // Low frequency alias
        firstPoint = true;
        
        for (int x = 0; x < graphArea.getWidth(); x += 2)
        {
            float t = (float)x / graphArea.getWidth() * 4.0f * juce::MathConstants<float>::pi + animationTime;
            float y = graphArea.getCentreY() + std::sin(aliasedFreq * t) * graphArea.getHeight() * 0.25f;
            
            if (firstPoint)
            {
                aliasedPath.startNewSubPath(graphArea.getX() + x, y);
                firstPoint = false;
            }
            else
            {
                aliasedPath.lineTo(graphArea.getX() + x, y);
            }
        }
        
        g.strokePath(aliasedPath, juce::PathStrokeType(3.0f));
        
        // Draw sample points
        g.setColour(juce::Colours::white);
        int sampleSpacing = 40;
        for (int x = 0; x < graphArea.getWidth(); x += sampleSpacing)
        {
            float t = (float)x / graphArea.getWidth() * 4.0f * juce::MathConstants<float>::pi + animationTime;
            float trueY = graphArea.getCentreY() + std::sin(frequency * t) * graphArea.getHeight() * 0.3f;
            float aliasedY = graphArea.getCentreY() + std::sin(aliasedFreq * t) * graphArea.getHeight() * 0.25f;
            
            g.fillEllipse(graphArea.getX() + x - 3, trueY - 3, 6, 6);
            g.fillEllipse(graphArea.getX() + x - 3, aliasedY - 3, 6, 6);
        }
        
        // Labels
        g.setColour(juce::Colours::magenta);
        g.setFont(14.0f);
        g.drawText("true signal at frequency ω", graphArea.getRight() - 200, graphArea.getY() + 50, 180, 20, juce::Justification::left);
        
        g.setColour(juce::Colours::yellow);
        g.drawText("low frequency (ω/2) erroneously", graphArea.getX() + 20, graphArea.getBottom() - 60, 300, 20, juce::Justification::left);
        g.drawText("assigned to signal because", graphArea.getX() + 20, graphArea.getBottom() - 40, 300, 20, juce::Justification::left);
        g.drawText("sampling rate is too low", graphArea.getX() + 20, graphArea.getBottom() - 20, 300, 20, juce::Justification::left);
    }
    
    void drawSamplingRate(juce::Graphics& g, juce::Rectangle<int> area)
    {
        g.setColour(juce::Colours::cyan);
        g.setFont(20.0f);
        g.drawText("Digital Sampling", area.getX(), area.getY(), area.getWidth(), 30, juce::Justification::centred);
        
        auto graphArea = area.reduced(0, 40);
        
        // Draw continuous waveform
        juce::Path wavePath;
        g.setColour(juce::Colours::cyan);
        
        bool firstPoint = true;
        for (int x = 0; x < graphArea.getWidth(); x += 2)
        {
            float t = (float)x / graphArea.getWidth() * 4.0f * juce::MathConstants<float>::pi + animationTime * 0.5f;
            float y = graphArea.getCentreY() + std::sin(t) * graphArea.getHeight() * 0.3f;
            
            if (firstPoint)
            {
                wavePath.startNewSubPath(graphArea.getX() + x, y);
                firstPoint = false;
            }
            else
            {
                wavePath.lineTo(graphArea.getX() + x, y);
            }
        }
        
        g.strokePath(wavePath, juce::PathStrokeType(2.0f));
        
        // Draw sample bars
        int sampleSpacing = 20;
        for (int x = 0; x < graphArea.getWidth(); x += sampleSpacing)
        {
            float t = (float)x / graphArea.getWidth() * 4.0f * juce::MathConstants<float>::pi + animationTime * 0.5f;
            float y = graphArea.getCentreY() + std::sin(t) * graphArea.getHeight() * 0.3f;
            
            // Sample bar
            g.setColour(juce::Colours::white.withAlpha(0.8f));
            g.drawLine(graphArea.getX() + x, graphArea.getCentreY(), graphArea.getX() + x, y, 2.0f);
            
            // Sample point
            g.setColour(juce::Colours::yellow);
            g.fillEllipse(graphArea.getX() + x - 2, y - 2, 4, 4);
        }
        
        // Draw baseline
        g.setColour(juce::Colours::white.withAlpha(0.3f));
        g.drawLine(graphArea.getX(), graphArea.getCentreY(), graphArea.getRight(), graphArea.getCentreY(), 1.0f);
    }
    
    void drawFilterResponse(juce::Graphics& g, juce::Rectangle<int> area)
    {
        g.setColour(juce::Colours::orange);
        g.setFont(20.0f);
        g.drawText("Filter Frequency Response", area.getX(), area.getY(), area.getWidth(), 30, juce::Justification::centred);
        
        auto graphArea = area.reduced(0, 40);
        
        // Draw axes
        g.setColour(juce::Colours::white.withAlpha(0.3f));
        g.drawLine(graphArea.getX(), graphArea.getBottom(), graphArea.getRight(), graphArea.getBottom(), 1.0f);
        g.drawLine(graphArea.getX(), graphArea.getY(), graphArea.getX(), graphArea.getBottom(), 1.0f);
        
        // Draw filter response curve
        juce::Path responsePath;
        g.setColour(juce::Colours::lime);
        
        bool firstPoint = true;
        for (int x = 0; x < graphArea.getWidth(); x += 2)
        {
            float freq = (float)x / graphArea.getWidth(); // 0 to 1
            float cutoff = 0.3f + 0.2f * std::sin(animationTime); // Animated cutoff
            float response;
            
            if (freq < cutoff)
                response = 1.0f; // Passband
            else
                response = 1.0f / (1.0f + std::pow((freq - cutoff) * 10.0f, 4.0f)); // Rolloff
            
            float y = graphArea.getBottom() - response * graphArea.getHeight() * 0.8f;
            
            if (firstPoint)
            {
                responsePath.startNewSubPath(graphArea.getX() + x, y);
                firstPoint = false;
            }
            else
            {
                responsePath.lineTo(graphArea.getX() + x, y);
            }
        }
        
        g.strokePath(responsePath, juce::PathStrokeType(3.0f));
        
        // Labels
        g.setColour(juce::Colours::white);
        g.setFont(12.0f);
        g.drawText("Frequency", graphArea.getCentreX() - 30, graphArea.getBottom() + 10, 60, 20, juce::Justification::centred);
        g.drawText("Gain", graphArea.getX() - 40, graphArea.getCentreY() - 10, 30, 20, juce::Justification::centred);
    }
    
    void drawWaveformAnalysis(juce::Graphics& g, juce::Rectangle<int> area)
    {
        g.setColour(juce::Colours::pink);
        g.setFont(20.0f);
        g.drawText("Waveform Analysis", area.getX(), area.getY(), area.getWidth(), 30, juce::Justification::centred);
        
        auto graphArea = area.reduced(0, 40);
        
        // Draw multiple frequency components
        std::vector<juce::Colour> colors = {juce::Colours::red, juce::Colours::green, juce::Colours::blue, juce::Colours::yellow};
        std::vector<float> frequencies = {1.0f, 2.0f, 3.0f, 0.5f};
        std::vector<float> amplitudes = {0.3f, 0.2f, 0.15f, 0.25f};
        
        for (size_t i = 0; i < frequencies.size(); ++i)
        {
            juce::Path componentPath;
            g.setColour(colors[i].withAlpha(0.7f));
            
            bool firstPoint = true;
            for (int x = 0; x < graphArea.getWidth(); x += 2)
            {
                float t = (float)x / graphArea.getWidth() * 6.0f * juce::MathConstants<float>::pi + animationTime;
                float y = graphArea.getCentreY() + std::sin(frequencies[i] * t) * graphArea.getHeight() * amplitudes[i];
                
                if (firstPoint)
                {
                    componentPath.startNewSubPath(graphArea.getX() + x, y);
                    firstPoint = false;
                }
                else
                {
                    componentPath.lineTo(graphArea.getX() + x, y);
                }
            }
            
            g.strokePath(componentPath, juce::PathStrokeType(2.0f));
        }
        
        // Draw composite waveform
        juce::Path compositePath;
        g.setColour(juce::Colours::white);
        
        bool firstPoint = true;
        for (int x = 0; x < graphArea.getWidth(); x += 2)
        {
            float t = (float)x / graphArea.getWidth() * 6.0f * juce::MathConstants<float>::pi + animationTime;
            float y = graphArea.getCentreY();
            
            // Sum all components
            for (size_t i = 0; i < frequencies.size(); ++i)
            {
                y += std::sin(frequencies[i] * t) * graphArea.getHeight() * amplitudes[i];
            }
            
            if (firstPoint)
            {
                compositePath.startNewSubPath(graphArea.getX() + x, y);
                firstPoint = false;
            }
            else
            {
                compositePath.lineTo(graphArea.getX() + x, y);
            }
        }
        
        g.strokePath(compositePath, juce::PathStrokeType(3.0f));
        
        // Draw baseline
        g.setColour(juce::Colours::white.withAlpha(0.3f));
        g.drawLine(graphArea.getX(), graphArea.getCentreY(), graphArea.getRight(), graphArea.getCentreY(), 1.0f);
    }
    
    juce::ComboBox graphSelector;
    juce::TextButton animateButton;
    float animationTime = 0.0f;
};

class MainWindow : public juce::DocumentWindow
{
public:
    MainWindow(juce::String name) : DocumentWindow(name,
                                                   juce::Desktop::getInstance().getDefaultLookAndFeel()
                                                                               .findColour(juce::ResizableWindow::backgroundColourId),
                                                   DocumentWindow::allButtons)
    {
        setUsingNativeTitleBar(true);
        setContentOwned(new AudioGraphicsComponent(), true);
        setResizable(true, true);
        centreWithSize(800, 600);
        setVisible(true);
    }

    void closeButtonPressed() override
    {
        std::exit(0);
    }

private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MainWindow)
};

class AudioGraphicsTestApp : public juce::JUCEApplication
{
public:
    const juce::String getApplicationName() override { return "Audio Graphics Test"; }
    const juce::String getApplicationVersion() override { return "1.0.0"; }
    
    void initialise(const juce::String& commandLine) override
    {
        juce::ignoreUnused(commandLine);
        mainWindow.reset(new MainWindow(getApplicationName()));
    }
    
    void shutdown() override
    {
        mainWindow = nullptr;
    }

    void systemRequestedQuit() override
    {
        quit();
    }

    void anotherInstanceStarted(const juce::String& commandLine) override
    {
        juce::ignoreUnused(commandLine);
    }
    
private:
    std::unique_ptr<MainWindow> mainWindow;
};

START_JUCE_APPLICATION(AudioGraphicsTestApp)
