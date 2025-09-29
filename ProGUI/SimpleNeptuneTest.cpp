#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_gui_extra/juce_gui_extra.h>

class SimpleNeptuneTest : public juce::Component
{
public:
    SimpleNeptuneTest()
    {
        // Load Neptune images directly
        loadNeptuneFrames();
        
        // Create a simple slider to control knob position
        addAndMakeVisible(testSlider);
        testSlider.setRange(0.0, 1.0, 0.01);
        testSlider.setValue(0.5);
        testSlider.onValueChange = [this]() { repaint(); };
        
        setSize(600, 400);
    }
    
    void paint(juce::Graphics& g) override
    {
        g.fillAll(juce::Colours::darkgrey);
        
        // Draw title
        g.setColour(juce::Colours::white);
        g.setFont(20.0f);
        g.drawText("Neptune Knob Test", 10, 10, getWidth()-20, 30, juce::Justification::centred);
        
        // Draw Neptune knob
        if (neptuneFrames.size() > 0)
        {
            float value = (float)testSlider.getValue();
            int frameIndex = juce::jlimit(0, neptuneFrames.size()-1, (int)(value * (neptuneFrames.size()-1)));
            
            auto knobArea = juce::Rectangle<int>(250, 100, 100, 100);
            g.drawImage(neptuneFrames[frameIndex], knobArea.toFloat());
            
            // Show frame info
            g.setFont(12.0f);
            g.drawText("Frame: " + juce::String(frameIndex+1) + "/" + juce::String(neptuneFrames.size()), 
                      10, 350, 200, 20, juce::Justification::left);
            g.drawText("Value: " + juce::String(value, 2), 
                      220, 350, 200, 20, juce::Justification::left);
        }
        else
        {
            g.setColour(juce::Colours::red);
            g.drawText("Neptune frames not loaded!", 10, 100, getWidth()-20, 30, juce::Justification::centred);
        }
    }
    
    void resized() override
    {
        testSlider.setBounds(50, 250, getWidth()-100, 30);
    }
    
private:
    void loadNeptuneFrames()
    {
        juce::File neptuneDir("/Volumes/MP 1/MoreMojo/NeptuneUI/03 Delivery/VST/Assets/Animation/Oneshots/knob_big");
        
        if (!neptuneDir.exists())
        {
            juce::Logger::writeToLog("Neptune directory not found: " + neptuneDir.getFullPathName());
            return;
        }
        
        auto files = neptuneDir.findChildFiles(juce::File::findFiles, false, "*.png");
        files.sort();
        
        for (auto& file : files)
        {
            auto image = juce::ImageFileFormat::loadFrom(file);
            if (image.isValid())
            {
                neptuneFrames.add(image);
            }
        }
        
        juce::Logger::writeToLog("Loaded " + juce::String(neptuneFrames.size()) + " Neptune frames");
    }
    
    juce::Array<juce::Image> neptuneFrames;
    juce::Slider testSlider;
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
        setContentOwned(new SimpleNeptuneTest(), true);

#if JUCE_IOS || JUCE_ANDROID
        setFullScreen(true);
#else
        setResizable(true, true);
        centreWithSize(getContentComponent()->getWidth(), getContentComponent()->getHeight());
#endif

        setVisible(true);
    }

    void closeButtonPressed() override
    {
        std::exit(0);
    }

private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MainWindow)
};

class NeptuneTestApp : public juce::JUCEApplication
{
public:
    const juce::String getApplicationName() override { return "Neptune Test"; }
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

START_JUCE_APPLICATION(NeptuneTestApp)
