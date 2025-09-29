#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_gui_extra/juce_gui_extra.h>
#include "../../Source/Neptune/NeptuneTestWindow.h"

class NeptuneTestApp : public juce::JUCEApplication
{
public:
    const juce::String getApplicationName() override { return "Neptune Test"; }
    const juce::String getApplicationVersion() override { return "1.0.0"; }

    void initialise(const juce::String&) override
    {
        mainWindow = std::make_unique<NeptuneTestWindow>();
    }

    void shutdown() override
    {
        mainWindow = nullptr;
    }

private:
    std::unique_ptr<NeptuneTestWindow> mainWindow;
};

START_JUCE_APPLICATION(NeptuneTestApp)
