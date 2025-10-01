#include <juce_gui_extra/juce_gui_extra.h>
#include "../../Source/Components/ConsumerGUI.h"
#include "../../Source/UIDemoComponent.h"

class ConsumerApplication : public juce::JUCEApplication {
public:
    const juce::String getApplicationName() override { return "MoreMojo"; }
    const juce::String getApplicationVersion() override { return "1.0.0"; }
    
    void initialise(const juce::String&) override {
        mainWindow.reset(new MainWindow(getApplicationName()));
    }
    
    void shutdown() override {
        stealMojoWindow = nullptr;
        proWindow = nullptr;
        mainWindow = nullptr;
    }
    
    void showProGUI(const juce::File& audioFile) {
        if (!proWindow) {
            proWindow.reset(new ProWindow("MoreMojo - Professional Controls"));
            
            // If audio file is loaded in Consumer GUI, load it in Pro GUI
            if (audioFile.existsAsFile()) {
                // TODO: Load audio file into Pro GUI waveform
                DBG("Loading audio into Pro GUI: " + audioFile.getFullPathName());
            }
        }
        
        if (proWindow) {
            proWindow->setVisible(true);
            proWindow->toFront(true);
        }
    }
    
    void showStealMojoGUI() {
        if (!stealMojoWindow) {
            stealMojoWindow.reset(new StealMojoWindow("Steal The Mojo"));
        }
        
        if (stealMojoWindow) {
            stealMojoWindow->setVisible(true);
            stealMojoWindow->toFront(true);
        }
    }
    
private:
    class MainWindow : public juce::DocumentWindow {
    public:
        MainWindow(juce::String name)
            : DocumentWindow(name,
                           juce::Colours::black,
                           DocumentWindow::allButtons) {
            setUsingNativeTitleBar(true);
            
            auto* consumerGUI = new ConsumerGUI();
            
            // Set up Pro Controls button callback
            consumerGUI->onProControlsClicked = [this, consumerGUI]() {
                auto* app = dynamic_cast<ConsumerApplication*>(juce::JUCEApplication::getInstance());
                if (app) {
                    app->showProGUI(consumerGUI->getLoadedFile());
                }
            };
            
            // Set up Steal The Mojo button callback
            consumerGUI->onStealMojoClicked = []() {
                auto* app = dynamic_cast<ConsumerApplication*>(juce::JUCEApplication::getInstance());
                if (app) {
                    app->showStealMojoGUI();
                }
            };
            
            setContentOwned(consumerGUI, true);
            setResizable(true, true);
            centreWithSize(getWidth(), getHeight());
            setVisible(true);
        }
        
        void closeButtonPressed() override {
            juce::JUCEApplication::getInstance()->systemRequestedQuit();
        }
    };
    
    class ProWindow : public juce::DocumentWindow {
    public:
        ProWindow(juce::String name)
            : DocumentWindow(name,
                           juce::Colours::darkgrey,
                           DocumentWindow::allButtons) {
            setUsingNativeTitleBar(true);
            setContentOwned(new UIDemoComponent(), true);
            setResizable(true, true);
            centreWithSize(1200, 900);
            setVisible(true);
        }
        
        void closeButtonPressed() override {
            setVisible(false);
        }
    };
    
    class StealMojoWindow : public juce::DocumentWindow {
    public:
        StealMojoWindow(juce::String name)
            : DocumentWindow(name,
                           PsychedelicTheme::Colors::psychPurple,
                           DocumentWindow::allButtons) {
            setUsingNativeTitleBar(true);
            setContentOwned(new StealMojoComponent(), true);
            setResizable(false, false);
            centreWithSize(700, 1000);
            setVisible(true);
        }
        
        void closeButtonPressed() override {
            setVisible(false);
        }
    };
    
    std::unique_ptr<MainWindow> mainWindow;
    std::unique_ptr<ProWindow> proWindow;
    std::unique_ptr<StealMojoWindow> stealMojoWindow;
};

START_JUCE_APPLICATION(ConsumerApplication)
