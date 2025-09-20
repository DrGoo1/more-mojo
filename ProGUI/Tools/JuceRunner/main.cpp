#include <JuceHeader.h>
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
