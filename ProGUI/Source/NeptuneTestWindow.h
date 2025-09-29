#pragma once
#include <juce_gui_basics/juce_gui_basics.h>

class NeptuneKnobDemo; // fwd decl

class NeptuneTestWindow : public juce::DocumentWindow
{
public:
    NeptuneTestWindow();                      // <-- must match .cpp exactly
    ~NeptuneTestWindow() override;

    void closeButtonPressed() override;

private:
    std::unique_ptr<NeptuneKnobDemo> content;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (NeptuneTestWindow)
};
