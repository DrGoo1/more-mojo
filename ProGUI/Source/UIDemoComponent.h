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
#include "Components/NeptuneMixKnob.h"
#include "../shared/ui_core/SkinnedKnob.h"
#include "../shared/ui_core/SkinnedSlider.h"

// Forward declarations for subwindow classes
class ProcessSubwindow;
class ISPSubwindow;
class SRCSubwindow;
class JitterSubwindow;
class AlignSubwindow;
class TransientSubwindow;
class DeEsserSubwindow;
class MLARSubwindow;
class TransformerSubwindow;

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
    void mouseDown(const juce::MouseEvent& event) override;
    void openProcessSubwindow(int processIndex);

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
    
    // Mix knob and horizontal bar meters
    std::unique_ptr<class NeptuneMixKnob> mixKnob;
    std::unique_ptr<class HorizontalBarMeter> originalMeter;
    std::unique_ptr<class HorizontalBarMeter> processedMeter;
    
    // Process controls (8 integrated process components)
    std::unique_ptr<class ProcessControl> processControls[8];
    
    // Enhanced placeholder component class that looks like ProcessControl
    class PlaceholderComponent : public juce::Component
    {
    public:
        PlaceholderComponent(const juce::String& name) : componentName(name), description(""), currentLevel(0.5f) 
        {
            // Create real Neptune knob
            neptuneKnob = std::make_unique<NeptuneMixKnob>();
            neptuneKnob->setValue(0.5f);
            neptuneKnob->onValueChanged = [this](float value) {
                currentLevel = value;
                repaint(); // Update cyan bar
            };
            addAndMakeVisible(*neptuneKnob);
        }
        
        void setDescription(const juce::String& desc) { description = desc; }
        
        void resized() override
        {
            auto bounds = getLocalBounds().reduced(8);
            bounds.removeFromTop(10 + 8); // Skip level bar and gap
            
            auto mainArea = bounds.removeFromTop(55);
            
            // Position the real Neptune knob on the far left
            auto knobArea = mainArea.removeFromLeft(45);
            knobArea = knobArea.withSizeKeepingCentre(45, 45);
            neptuneKnob->setBounds(knobArea);
        }
        
        void paint(juce::Graphics& g) override
        {
            auto bounds = getLocalBounds();
            
            // Background with subtle border (like ProcessControl)
            g.setColour(juce::Colour(0xFF1a1a1a));
            g.fillRoundedRectangle(bounds.toFloat(), 4.0f);
            g.setColour(juce::Colour(0xFF333333));
            g.drawRoundedRectangle(bounds.toFloat(), 4.0f, 1.0f);
            
            // Layout areas
            auto contentArea = bounds.reduced(8);
            
            // Cyan level bar at top (full width)
            auto levelArea = contentArea.removeFromTop(10).reduced(0, 1);
            
            // Background for level bar
            g.setColour(juce::Colour(0xFF333333));
            g.fillRoundedRectangle(levelArea.toFloat(), 2.0f);
            
            // Active level (cyan)
            auto activeLevelArea = levelArea.removeFromLeft((int)(levelArea.getWidth() * currentLevel));
            g.setColour(juce::Colour(0xFF00d4aa)); // Cyan color
            g.fillRoundedRectangle(activeLevelArea.toFloat(), 2.0f);
            
            contentArea.removeFromTop(8); // Gap after level bar
            
            // Main content area with knob on left, text on right
            auto mainArea = contentArea.removeFromTop(55); // Even larger area for text and knob
            
            // Skip knob area since we have a real Neptune knob
            mainArea.removeFromLeft(45 + 15); // knob width + gap
            
            // Text area (process name and description)
            auto textArea = mainArea;
            
            // Process name (main title) - aligned with knob center
            auto nameArea = textArea.removeFromTop(22);
            g.setColour(juce::Colours::white);
            g.setFont(13.0f); // Larger font for bigger components
            g.drawText(componentName, nameArea, juce::Justification::centredLeft);
            
            // Description (subtitle)
            if (description.isNotEmpty())
            {
                auto descArea = textArea.removeFromTop(20);
                g.setColour(juce::Colour(0xFF00ffff)); // Cyan text
                g.setFont(10.0f); // Larger subtitle font
                g.drawText(description, descArea, juce::Justification::centredLeft);
            }
            
            // Larger button at the bottom right
            auto buttonArea = contentArea.removeFromBottom(25).removeFromRight(110);
            g.setColour(juce::Colour(0xFF2a2a2a));
            g.fillRoundedRectangle(buttonArea.toFloat(), 4.0f);
            g.setColour(juce::Colour(0xFF666666));
            g.drawRoundedRectangle(buttonArea.toFloat(), 4.0f, 1.0f);
            g.setColour(juce::Colours::white);
            g.setFont(8.0f); // Larger button font
            g.drawText("OPEN CONTROLS", buttonArea, juce::Justification::centred);
        }
        
        void mouseDown(const juce::MouseEvent& event) override
        {
            // Forward mouse events to parent UIDemoComponent
            if (auto* parent = getParentComponent())
            {
                // Convert local coordinates to parent coordinates
                auto parentEvent = event.getEventRelativeTo(parent);
                parent->mouseDown(parentEvent);
            }
        }
        
    private:
        juce::String componentName;
        juce::String description;
        float currentLevel;
        std::unique_ptr<NeptuneMixKnob> neptuneKnob;
    };
    
    // Temporary placeholder components for testing architecture
    std::unique_ptr<PlaceholderComponent> placeholderComponents[8];
    // Waveform view (constructed in .cpp)
    std::unique_ptr<class WaveformView> waveform;
    // Transport bar (UI-only for now)
    std::unique_ptr<TransportBar> transport;

    float level {0.25f};
    float level2 {0.75f};
    float originalLevel {0.3f};
    float processedLevel {0.6f};
    float mixValue {0.5f};
    float processLevels[8] = {0.3f, 0.4f, 0.5f, 0.6f, 0.7f, 0.8f, 0.9f, 0.35f}; // Individual process levels
    // Debug variables removed - knobs positioned correctly

    void timerCallback() override;
    void forceNeptuneKnobPositioning();
    static void drawMeter(juce::Graphics& g, juce::Rectangle<int> area, float value, juce::Colour col, MeterMode mode);
};
