#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_gui_extra/juce_gui_extra.h>

// Simple LedBarMeter implementation for testing
class LedBarMeter : public juce::Component, private juce::Timer
{
public:
    LedBarMeter()
    {
        startTimerHz(30); // 30 FPS animation
    }
    
    void setLevel(float newLevel)
    {
        targetLevel = juce::jlimit(0.0f, 1.0f, newLevel);
    }
    
    void paint(juce::Graphics& g) override
    {
        auto bounds = getLocalBounds().toFloat();
        
        // Background
        g.setColour(juce::Colour(0xFF1a1a1a));
        g.fillRoundedRectangle(bounds, 2.0f);
        
        // Border
        g.setColour(juce::Colour(0xFF333333));
        g.drawRoundedRectangle(bounds, 2.0f, 1.0f);
        
        // LED segments
        int numSegments = 20;
        float segmentHeight = (bounds.getHeight() - 4) / numSegments;
        float segmentWidth = bounds.getWidth() - 4;
        
        for (int i = 0; i < numSegments; ++i)
        {
            float segmentY = bounds.getBottom() - 2 - (i + 1) * segmentHeight;
            auto segmentRect = juce::Rectangle<float>(bounds.getX() + 2, segmentY, segmentWidth, segmentHeight - 1);
            
            float segmentLevel = (float)(i + 1) / numSegments;
            bool isLit = currentLevel >= segmentLevel;
            
            if (isLit)
            {
                // Color based on level
                juce::Colour segmentColor;
                if (segmentLevel < 0.6f)
                    segmentColor = juce::Colours::green;
                else if (segmentLevel < 0.8f)
                    segmentColor = juce::Colours::yellow;
                else
                    segmentColor = juce::Colours::red;
                
                g.setColour(segmentColor);
                g.fillRoundedRectangle(segmentRect, 1.0f);
                
                // Highlight
                g.setColour(segmentColor.brighter(0.3f));
                g.fillRoundedRectangle(segmentRect.reduced(0, segmentRect.getHeight() * 0.3f), 1.0f);
            }
            else
            {
                // Unlit segment
                g.setColour(juce::Colour(0xFF2a2a2a));
                g.fillRoundedRectangle(segmentRect, 1.0f);
            }
        }
        
        // Peak hold indicator
        if (peakLevel > 0.0f)
        {
            int peakSegment = (int)(peakLevel * numSegments);
            if (peakSegment > 0 && peakSegment <= numSegments)
            {
                float peakY = bounds.getBottom() - 2 - peakSegment * segmentHeight;
                auto peakRect = juce::Rectangle<float>(bounds.getX() + 2, peakY, segmentWidth, segmentHeight - 1);
                
                g.setColour(juce::Colours::white);
                g.fillRoundedRectangle(peakRect, 1.0f);
            }
        }
    }
    
private:
    void timerCallback() override
    {
        // Smooth level animation
        float diff = targetLevel - currentLevel;
        if (std::abs(diff) > 0.001f)
        {
            currentLevel += diff * 0.1f; // Smooth interpolation
            repaint();
        }
        
        // Peak hold decay
        if (targetLevel > peakLevel)
        {
            peakLevel = targetLevel;
            peakHoldTime = 0;
        }
        else
        {
            peakHoldTime++;
            if (peakHoldTime > 60) // Hold for 2 seconds at 30fps
            {
                peakLevel *= 0.95f; // Slow decay
                if (peakLevel < 0.01f) peakLevel = 0.0f;
            }
        }
    }
    
    float currentLevel = 0.0f;
    float targetLevel = 0.0f;
    float peakLevel = 0.0f;
    int peakHoldTime = 0;
};

class LedBarMeterTest : public juce::Component, private juce::Timer
{
public:
    LedBarMeterTest()
    {
        // Create multiple meters
        for (int i = 0; i < 4; ++i)
        {
            auto meter = std::make_unique<LedBarMeter>();
            addAndMakeVisible(*meter);
            meters.add(std::move(meter));
        }
        
        // Create control sliders
        for (int i = 0; i < 4; ++i)
        {
            auto slider = std::make_unique<juce::Slider>();
            slider->setRange(0.0, 1.0, 0.01);
            slider->setValue(0.5);
            slider->onValueChange = [this, i]() {
                if (meters[i])
                    meters[i]->setLevel((float)sliders[i]->getValue());
            };
            addAndMakeVisible(*slider);
            sliders.add(std::move(slider));
        }
        
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
        
        setSize(600, 400);
    }
    
    void paint(juce::Graphics& g) override
    {
        g.fillAll(juce::Colour(0xFF2a2a3e));
        
        // Title
        g.setColour(juce::Colours::white);
        g.setFont(20.0f);
        g.drawText("LED Bar Meter Test", 10, 10, getWidth()-20, 30, juce::Justification::centred);
        
        // Labels
        g.setFont(12.0f);
        for (int i = 0; i < 4; ++i)
        {
            g.drawText("Meter " + juce::String(i+1), 20 + i * 140, 50, 100, 20, juce::Justification::left);
        }
    }
    
    void resized() override
    {
        // Position meters
        for (int i = 0; i < 4; ++i)
        {
            if (meters[i])
                meters[i]->setBounds(50 + i * 140, 80, 30, 200);
        }
        
        // Position sliders
        for (int i = 0; i < 4; ++i)
        {
            if (sliders[i])
                sliders[i]->setBounds(90 + i * 140, 80, 30, 200);
        }
        
        // Position animate button
        animateButton.setBounds(250, 300, 100, 30);
    }
    
private:
    void timerCallback() override
    {
        // Auto-animate the meters with different patterns
        float time = juce::Time::getMillisecondCounterHiRes() * 0.001f;
        
        for (int i = 0; i < 4; ++i)
        {
            if (meters[i])
            {
                float level = 0.5f + 0.4f * std::sin(time * (1.0f + i * 0.3f));
                meters[i]->setLevel(level);
                
                if (sliders[i])
                    sliders[i]->setValue(level, juce::dontSendNotification);
            }
        }
    }
    
    juce::OwnedArray<LedBarMeter> meters;
    juce::OwnedArray<juce::Slider> sliders;
    juce::TextButton animateButton;
};

class LedBarMeterTestApp : public juce::JUCEApplication
{
public:
    const juce::String getApplicationName() override { return "LED Bar Meter Test"; }
    const juce::String getApplicationVersion() override { return "1.0.0"; }
    
    void initialise(const juce::String&) override
    {
        mainWindow = std::make_unique<juce::DocumentWindow>("LED Bar Meter Test", 
                                                           juce::Colours::darkgrey, 
                                                           juce::DocumentWindow::allButtons);
        
        auto* content = new LedBarMeterTest();
        mainWindow->setContentOwned(content, true);
        mainWindow->centreWithSize(600, 400);
        mainWindow->setVisible(true);
    }
    
    void shutdown() override
    {
        mainWindow = nullptr;
    }
    
private:
    std::unique_ptr<juce::DocumentWindow> mainWindow;
};

START_JUCE_APPLICATION(LedBarMeterTestApp)
