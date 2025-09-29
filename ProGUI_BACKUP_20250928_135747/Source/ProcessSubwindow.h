#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include "../shared/ui_core/SkinManager.h"
#include "../shared/ui_core/SkinnedKnob.h"
#include "../shared/ui_core/SkinnedSlider.h"

/**
 * Base class for all Professional Mojo process subwindows
 * Template: Header (title/preset/amount) + Body (controls) + Meter column + Footer (waveform/transport/IO/Bake)
 */
class ProcessSubwindow : public juce::Component
{
public:
    ProcessSubwindow(const juce::String& processName, const juce::String& description);
    ~ProcessSubwindow() override = default;

    void paint(juce::Graphics& g) override;
    void resized() override;

protected:
    // Virtual methods for subclasses to implement
    virtual void createControls() = 0;
    virtual void layoutControls(juce::Rectangle<int> controlArea) = 0;
    virtual void paintMeters(juce::Graphics& g, juce::Rectangle<int> meterArea) = 0;

    // Helper methods for creating common controls
    std::unique_ptr<SkinnedKnob> createKnob(const juce::String& name, float min, float max, float defaultVal);
    std::unique_ptr<SkinnedSlider> createSlider(const juce::String& name, float min, float max, float defaultVal);
    std::unique_ptr<juce::ComboBox> createComboBox(const juce::String& name, const juce::StringArray& items);
    std::unique_ptr<juce::ToggleButton> createToggleButton(const juce::String& name);

    // Common UI elements
    juce::String processName;
    juce::String processDescription;
    
    // Global controls (in every subwindow)
    std::unique_ptr<juce::ComboBox> presetCombo;
    std::unique_ptr<juce::ComboBox> engineCombo;
    std::unique_ptr<juce::ComboBox> qualityCombo;
    std::unique_ptr<SkinnedKnob> amountKnob;
    std::unique_ptr<juce::ToggleButton> bypassButton;
    std::unique_ptr<juce::TextButton> bakeButton;
    
    // I/O controls
    std::unique_ptr<SkinnedSlider> inputTrimSlider;
    std::unique_ptr<SkinnedSlider> outputTrimSlider;

private:
    void createGlobalControls();
    void layoutHeader(juce::Rectangle<int> headerArea);
    void layoutFooter(juce::Rectangle<int> footerArea);
    void paintHeader(juce::Graphics& g, juce::Rectangle<int> headerArea);
    void paintFooter(juce::Graphics& g, juce::Rectangle<int> footerArea);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ProcessSubwindow)
};
