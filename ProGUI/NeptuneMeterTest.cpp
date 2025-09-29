#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_gui_extra/juce_gui_extra.h>

class SimpleNeptuneMeterTest : public juce::Component, private juce::Timer
{
public:
    SimpleNeptuneMeterTest()
    {
        // Load Neptune VU meter images directly
        loadNeptuneMeterFrames();
        
        // Create control sliders
        addAndMakeVisible(testSlider);
        testSlider.setRange(0.0, 1.0, 0.01);
        testSlider.setValue(0.5);
        testSlider.onValueChange = [this]() { repaint(); };
        
        // Auto-animate button
        animateButton.setButtonText("Auto Animate");
        animateButton.setToggleable(true);
        animateButton.onClick = [this]() {
            if (animateButton.getToggleState())
                startTimerHz(30);
            else
                stopTimer();
        };
        addAndMakeVisible(animateButton);
        
        setSize(800, 500);
    }
    
    void paint(juce::Graphics& g) override
    {
        g.fillAll(juce::Colour(0xFF2a2a3e));
        
        // Draw title
        g.setColour(juce::Colours::white);
        g.setFont(20.0f);
        g.drawText("Neptune VU Meter Test", 10, 10, getWidth()-20, 30, juce::Justification::centred);
        
        // Draw Neptune VU meters
        if (neptuneMeterFrames.size() > 0)
        {
            float value = (float)testSlider.getValue();
            int frameIndex = juce::jlimit(0, neptuneMeterFrames.size()-1, (int)(value * (neptuneMeterFrames.size()-1)));
            
            // Draw main meter (large)
            auto meterArea1 = juce::Rectangle<int>(50, 80, 300, 200);
            g.drawImage(neptuneMeterFrames[frameIndex], meterArea1.toFloat());
            
            // Draw second meter (medium)
            auto meterArea2 = juce::Rectangle<int>(400, 100, 200, 133);
            g.drawImage(neptuneMeterFrames[frameIndex], meterArea2.toFloat());
            
            // Draw third meter (small)
            auto meterArea3 = juce::Rectangle<int>(650, 120, 100, 67);
            g.drawImage(neptuneMeterFrames[frameIndex], meterArea3.toFloat());
            
            // Show frame info
            g.setFont(12.0f);
            g.drawText("Frame: " + juce::String(frameIndex+1) + "/" + juce::String(neptuneMeterFrames.size()), 
                      10, 400, 200, 20, juce::Justification::left);
            g.drawText("Value: " + juce::String(value, 2), 
                      220, 400, 200, 20, juce::Justification::left);
            g.drawText("Auto: " + juce::String(animateButton.getToggleState() ? "ON" : "OFF"), 
                      430, 400, 200, 20, juce::Justification::left);
                      
            // Labels
            g.setFont(14.0f);
            g.drawText("Large", 50, 290, 100, 20, juce::Justification::left);
            g.drawText("Medium", 400, 240, 100, 20, juce::Justification::left);
            g.drawText("Small", 650, 195, 100, 20, juce::Justification::left);
        }
        else
        {
            g.setColour(juce::Colours::red);
            g.drawText("Neptune VU meter frames not loaded!", 10, 100, getWidth()-20, 30, juce::Justification::centred);
        }
    }
    
    void resized() override
    {
        testSlider.setBounds(50, 350, getWidth()-200, 30);
        animateButton.setBounds(getWidth()-120, 350, 100, 30);
    }
    
private:
    void timerCallback() override
    {
        // Auto-animate the meter with a sine wave pattern
        float time = juce::Time::getMillisecondCounterHiRes() * 0.001f;
        float level = 0.5f + 0.4f * std::sin(time * 2.0f);
        testSlider.setValue(level, juce::dontSendNotification);
        repaint();
    }
    
    void loadNeptuneMeterFrames()
    {
        juce::File neptuneMeterDir("/Volumes/MP 1/MoreMojo/NeptuneUI/03 Delivery/VST/Assets/Animation/Oneshots/vu_meter");
        
        if (!neptuneMeterDir.exists())
        {
            juce::Logger::writeToLog("Neptune VU meter directory not found: " + neptuneMeterDir.getFullPathName());
            return;
        }
        
        auto files = neptuneMeterDir.findChildFiles(juce::File::findFiles, false, "*.png");
        files.sort();
        
        for (auto& file : files)
        {
            auto image = juce::ImageFileFormat::loadFrom(file);
            if (image.isValid())
            {
                neptuneMeterFrames.add(image);
            }
        }
        
        juce::Logger::writeToLog("Loaded " + juce::String(neptuneMeterFrames.size()) + " Neptune VU meter frames");
    }
    
    juce::Array<juce::Image> neptuneMeterFrames;
    juce::Slider testSlider;
    juce::TextButton animateButton;
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
        setContentOwned(new SimpleNeptuneMeterTest(), true);
        setResizable(true, true);
        centreWithSize(800, 500);
        setVisible(true);
    }

    void closeButtonPressed() override
    {
        std::exit(0);
    }

private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MainWindow)
};

class NeptuneMeterTestApp : public juce::JUCEApplication
{
public:
    const juce::String getApplicationName() override { return "Neptune VU Meter Test"; }
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

START_JUCE_APPLICATION(NeptuneMeterTestApp)
