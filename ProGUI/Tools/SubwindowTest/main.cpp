#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_gui_extra/juce_gui_extra.h>
#include "../../Source/ProcessSubwindow.h"
#include "../../Source/ISPSubwindow.h"
#include "../../Source/SRCSubwindow.h"
#include "../../Source/JitterSubwindow.h"
#include "../../Source/AlignSubwindow.h"
#include "../../Source/TransientSubwindow.h"
#include "../../Source/DeEsserSubwindow.h"
#include "../../Source/MLARSubwindow.h"
#include "../../Source/TransformerSubwindow.h"

class SubwindowTestApp : public juce::JUCEApplication
{
public:
    const juce::String getApplicationName() override { return "Subwindow Test"; }
    const juce::String getApplicationVersion() override { return "1.0.0"; }
    
    void initialise(const juce::String&) override
    {
        mainWindow = std::make_unique<MainWindow>("Subwindow Test", new TestComponent(), *this);
    }
    
    void shutdown() override
    {
        mainWindow = nullptr;
    }
    
private:
    class TestComponent : public juce::Component
    {
    public:
        TestComponent()
        {
            setSize(400, 300);
            
            // Create buttons for each subwindow
            addAndMakeVisible(ispButton);
            ispButton.setButtonText("ISP Subwindow");
            ispButton.onClick = [this] { openSubwindow<ISPSubwindow>("ISP Controls"); };
            
            addAndMakeVisible(srcButton);
            srcButton.setButtonText("SRC Subwindow");
            srcButton.onClick = [this] { openSubwindow<SRCSubwindow>("SRC Controls"); };
            
            addAndMakeVisible(jitterButton);
            jitterButton.setButtonText("Jitter Subwindow");
            jitterButton.onClick = [this] { openSubwindow<JitterSubwindow>("Jitter Controls"); };
            
            addAndMakeVisible(alignButton);
            alignButton.setButtonText("Align Subwindow");
            alignButton.onClick = [this] { openSubwindow<AlignSubwindow>("Align Controls"); };
            
            addAndMakeVisible(transientButton);
            transientButton.setButtonText("Transient Subwindow");
            transientButton.onClick = [this] { openSubwindow<TransientSubwindow>("Transient Controls"); };
            
            addAndMakeVisible(deesserButton);
            deesserButton.setButtonText("De-esser Subwindow");
            deesserButton.onClick = [this] { openSubwindow<DeEsserSubwindow>("De-esser Controls"); };
            
            addAndMakeVisible(mlarButton);
            mlarButton.setButtonText("MLAR Subwindow");
            mlarButton.onClick = [this] { openSubwindow<MLARSubwindow>("MLAR Controls"); };
            
            addAndMakeVisible(transformerButton);
            transformerButton.setButtonText("Transformer Subwindow");
            transformerButton.onClick = [this] { openSubwindow<TransformerSubwindow>("Transformer Controls"); };
        }
        
        void paint(juce::Graphics& g) override
        {
            g.fillAll(juce::Colour(0xFF1a1a2e));
            g.setColour(juce::Colours::white);
            g.setFont(juce::Font(16.0f, juce::Font::bold));
            g.drawText("Click buttons to test subwindows:", getLocalBounds().removeFromTop(40), juce::Justification::centred);
        }
        
        void resized() override
        {
            auto area = getLocalBounds().reduced(20);
            area.removeFromTop(40); // Skip title area
            
            int buttonHeight = 30;
            int gap = 5;
            
            ispButton.setBounds(area.removeFromTop(buttonHeight));
            area.removeFromTop(gap);
            srcButton.setBounds(area.removeFromTop(buttonHeight));
            area.removeFromTop(gap);
            jitterButton.setBounds(area.removeFromTop(buttonHeight));
            area.removeFromTop(gap);
            alignButton.setBounds(area.removeFromTop(buttonHeight));
            area.removeFromTop(gap);
            transientButton.setBounds(area.removeFromTop(buttonHeight));
            area.removeFromTop(gap);
            deesserButton.setBounds(area.removeFromTop(buttonHeight));
            area.removeFromTop(gap);
            mlarButton.setBounds(area.removeFromTop(buttonHeight));
            area.removeFromTop(gap);
            transformerButton.setBounds(area.removeFromTop(buttonHeight));
        }
        
    private:
        template<typename SubwindowType>
        void openSubwindow(const juce::String& title)
        {
            auto* subwindow = new SubwindowType();
            
            juce::DialogWindow::LaunchOptions options;
            options.content.setOwned(subwindow);
            options.dialogTitle = title;
            options.componentToCentreAround = this;
            options.escapeKeyTriggersCloseButton = true;
            options.useNativeTitleBar = true;
            options.resizable = true;
            options.launchAsync();
        }
        
        juce::TextButton ispButton, srcButton, jitterButton, alignButton;
        juce::TextButton transientButton, deesserButton, mlarButton, transformerButton;
    };
    
    class MainWindow : public juce::DocumentWindow
    {
    public:
        MainWindow(juce::String name, juce::Component* c, JUCEApplication& a)
            : DocumentWindow(name, juce::Colour(0xFF1a1a2e), DocumentWindow::allButtons), app(a)
        {
            setUsingNativeTitleBar(true);
            setContentOwned(c, true);
            setResizable(true, true);
            centreWithSize(getWidth(), getHeight());
            setVisible(true);
        }
        
        void closeButtonPressed() override
        {
            app.systemRequestedQuit();
        }
        
    private:
        JUCEApplication& app;
    };
    
    std::unique_ptr<MainWindow> mainWindow;
};

START_JUCE_APPLICATION(SubwindowTestApp)
