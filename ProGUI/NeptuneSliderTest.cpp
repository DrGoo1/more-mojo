#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_gui_extra/juce_gui_extra.h>

class SimpleNeptuneSliderTest : public juce::Component
{
public:
    SimpleNeptuneSliderTest()
    {
        // Load Neptune slider images directly
        loadNeptuneSliderFrames();
        
        // Create a simple slider to control slider position
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
        g.drawText("Neptune Slider Test", 10, 10, getWidth()-20, 30, juce::Justification::centred);
        
        // Draw Neptune vertical slider
        if (neptuneSliderFrames.size() > 0)
        {
            float value = (float)testSlider.getValue();
            int frameIndex = juce::jlimit(0, neptuneSliderFrames.size()-1, (int)(value * (neptuneSliderFrames.size()-1)));
            
            auto sliderArea = juce::Rectangle<int>(200, 80, 80, 200);
            g.drawImage(neptuneSliderFrames[frameIndex], sliderArea.toFloat());
            
            // Show frame info
            g.setFont(12.0f);
            g.drawText("Frame: " + juce::String(frameIndex+1) + "/" + juce::String(neptuneSliderFrames.size()), 
                      10, 350, 200, 20, juce::Justification::left);
            g.drawText("Value: " + juce::String(value, 2), 
                      220, 350, 200, 20, juce::Justification::left);
        }
        else
        {
            g.setColour(juce::Colours::red);
            g.drawText("Neptune slider frames not loaded!", 10, 100, getWidth()-20, 30, juce::Justification::centred);
        }
        
        // Draw horizontal slider for comparison
        if (neptuneHorSliderFrames.size() > 0)
        {
            float value = (float)testSlider.getValue();
            int frameIndex = juce::jlimit(0, neptuneHorSliderFrames.size()-1, (int)(value * (neptuneHorSliderFrames.size()-1)));
            
            auto horSliderArea = juce::Rectangle<int>(320, 150, 200, 80);
            g.drawImage(neptuneHorSliderFrames[frameIndex], horSliderArea.toFloat());
            
            g.setFont(12.0f);
            g.drawText("Horizontal: " + juce::String(frameIndex+1) + "/" + juce::String(neptuneHorSliderFrames.size()), 
                      320, 240, 200, 20, juce::Justification::left);
        }
    }
    
    void resized() override
    {
        testSlider.setBounds(50, 300, getWidth()-100, 30);
    }
    
private:
    void loadNeptuneSliderFrames()
    {
        // Load vertical slider frames
        juce::File neptuneVertDir("/Volumes/MP 1/MoreMojo/NeptuneUI/03 Delivery/VST/Assets/Animation/Oneshots/slider_ver");
        
        if (neptuneVertDir.exists())
        {
            auto files = neptuneVertDir.findChildFiles(juce::File::findFiles, false, "*.png");
            files.sort();
            
            for (auto& file : files)
            {
                auto image = juce::ImageFileFormat::loadFrom(file);
                if (image.isValid())
                {
                    neptuneSliderFrames.add(image);
                }
            }
            
            juce::Logger::writeToLog("Loaded " + juce::String(neptuneSliderFrames.size()) + " Neptune vertical slider frames");
        }
        
        // Load horizontal slider frames
        juce::File neptuneHorDir("/Volumes/MP 1/MoreMojo/NeptuneUI/03 Delivery/VST/Assets/Animation/Oneshots/slider_hor");
        
        if (neptuneHorDir.exists())
        {
            auto files = neptuneHorDir.findChildFiles(juce::File::findFiles, false, "*.png");
            files.sort();
            
            for (auto& file : files)
            {
                auto image = juce::ImageFileFormat::loadFrom(file);
                if (image.isValid())
                {
                    neptuneHorSliderFrames.add(image);
                }
            }
            
            juce::Logger::writeToLog("Loaded " + juce::String(neptuneHorSliderFrames.size()) + " Neptune horizontal slider frames");
        }
    }
    
    juce::Array<juce::Image> neptuneSliderFrames;
    juce::Array<juce::Image> neptuneHorSliderFrames;
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
        setContentOwned(new SimpleNeptuneSliderTest(), true);
        setResizable(true, true);
        centreWithSize(600, 400);
        setVisible(true);
    }

    void closeButtonPressed() override
    {
        std::exit(0);
    }

private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MainWindow)
};

class NeptuneSliderTestApp : public juce::JUCEApplication
{
public:
    const juce::String getApplicationName() override { return "Neptune Slider Test"; }
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

START_JUCE_APPLICATION(NeptuneSliderTestApp)
