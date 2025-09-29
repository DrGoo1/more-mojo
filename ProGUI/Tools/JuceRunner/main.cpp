#include <juce_core/juce_core.h>
#include <juce_graphics/juce_graphics.h>
#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_gui_extra/juce_gui_extra.h>
#include "../../Source/UIDemoComponent.h"

class DemoWindow : public juce::DocumentWindow {
public:
    DemoWindow() : juce::DocumentWindow("ProGUI JUCE Runner",
                                        juce::Desktop::getInstance().getDefaultLookAndFeel()
                                            .findColour(juce::ResizableWindow::backgroundColourId),
                                        juce::DocumentWindow::allButtons) {
        setUsingNativeTitleBar(true);
        auto* demo = new UIDemoComponent();
        setContentOwned(demo, true);
        setResizable(true, true);
        // Remove size limits to allow free resizing
        // setResizeLimits(800, 500, 1600, 1200);
        
        centreWithSize(900, 600);
        setVisible(true);
    }

    void closeButtonPressed() override {
        juce::JUCEApplicationBase::quit();
    }
};

class RunnerApp : public juce::JUCEApplication {
public:
    const juce::String getApplicationName() override    { return "ProGUI JUCE Runner"; }
    const juce::String getApplicationVersion() override { return "1.0"; }
    void initialise (const juce::String&) override      { window.reset(new DemoWindow()); }
    void shutdown() override                            { window = nullptr; }
private:
    std::unique_ptr<DemoWindow> window;
};

START_JUCE_APPLICATION (RunnerApp)
