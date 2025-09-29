#pragma once
// Explicit JUCE module includes (avoid JuceHeader.h); use module includes
#include <juce_core/juce_core.h>
#include <juce_events/juce_events.h>
#include <juce_graphics/juce_graphics.h>
#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_gui_extra/juce_gui_extra.h>
// Optional skin manager (stub) for loading assets
#include "../shared/ui_core/SkinManager.h"
#include "../shared/ui_core/Theme.h"
#include "../shared/ui_core/Primitives.h"
#include "../shared/ui_core/PillToggle.h"
#include "../shared/ui_core/LedBarMeter.h"
#include "../shared/ui_core/TransportBar.h"
#include "Components/ProMasterComponent.h"
#include "Components/StealMojoComponent.h"
#include "../shared/ui_core/SkinnedKnob.h"
#include "../shared/ui_core/SkinnedSlider.h"

// Fallback: draw vector controls directly in paint() for guaranteed visibility
#ifndef PROGUI_VECTOR_FALLBACK
#define PROGUI_VECTOR_FALLBACK 0
#endif

class UIDemoComponent : public juce::Component, private juce::Timer
{
public:
    UIDemoComponent();
    void paint(juce::Graphics& g) override;
    void resized() override;

private:
    enum class MeterMode { VU, LED };
    MeterMode meterMode { MeterMode::VU };

    static void proguiLog(const juce::String& s);
    juce::Slider demoKnob;
    juce::Slider demoSlider;
    juce::TextButton demoButton;
    ui::PillToggle demoSwitch;
    juce::TextButton loadSkinButton;
    juce::TextButton btnAction;
    juce::TextButton btnLoadSkin;
    juce::TextButton btnProSkin;
    juce::TextButton btnConsumerSkin;
    juce::TextButton btnConsumerAltSkin;
    juce::TextButton btnFantasmaSkin; // Neptune Fantasma kit
    juce::TextButton btnVUMode;
    juce::TextButton btnLEDMode;
    juce::TextButton btnProMaster { "Pro Master…" };
    juce::TextButton btnStealMojo { "Steal The Mojo…" };
    juce::Image skinImage;

    // Shell: master bypass and timeline zoom controls
    juce::ToggleButton masterBypass { "Bypass" };
    juce::TextButton zoomMinus { "-" };
    juce::TextButton zoomPlus  { "+" };
    bool isBypassed { false };

    // LED meters (constructed in .cpp)
    std::unique_ptr<LedBarMeter> ledMeter1;
    std::unique_ptr<LedBarMeter> ledMeter2;
    // IO trims (Vertical sliders) and labels
    SkinnedSlider inputTrim;
    SkinnedSlider outputTrim;
    juce::Label lblIn { {}, "Input" };
    juce::Label lblOut{ {}, "Output" };
    // Waveform view (constructed in .cpp)
    std::unique_ptr<class WaveformView> waveform;
    // Transport bar (UI-only for now)
    std::unique_ptr<TransportBar> transport;

    float level {0.25f};
    float level2 {0.75f};

    void timerCallback() override;
    static void drawMeter(juce::Graphics& g, juce::Rectangle<int> area, float value, juce::Colour col, MeterMode mode);
};
