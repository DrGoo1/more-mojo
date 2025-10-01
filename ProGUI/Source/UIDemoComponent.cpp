#include "UIDemoComponent.h"
#include "Components/NeptuneVUMeter.h"
#include "Components/NeptuneKnob.h"
#include "Components/NeptuneMixKnob.h"
#include "Components/HorizontalBarMeter.h"
#include "Components/ProcessControl.h"
#include "Components/NeptuneISPSubwindow.h"
#include "Components/PsychedelicStealMojoComponent.h"
// #include "ControlInfoWindow.h" // Temporarily disabled due to linking issues
// Professional subwindow includes temporarily disabled - using simple inline approach
// #include "ProcessSubwindow.h"
// Individual subwindows temporarily disabled - using inline test class
// #include "ISPSubwindow.h"
// Other subwindows temporarily disabled until linking issues resolved
// #include "SRCSubwindow.h"
// #include "JitterSubwindow.h"
// #include "AlignSubwindow.h"
// #include "TransientSubwindow.h"
// #include "DeEsserSubwindow.h"
// #include "MLARSubwindow.h"
// #include "TransformerSubwindow.h"
#include "../shared/ui_core/LedBarMeter.h"
#include "../shared/ui_core/WaveformView.h"
// Ensure LedBarMeter gets compiled in this TU for the demo build
#include "../shared/ui_core/LedBarMeter.cpp"
// Ensure WaveformView gets compiled in this TU for the demo build
#include "../shared/ui_core/WaveformView.cpp"

// Keep fallback flag consistent
#ifndef PROGUI_VECTOR_FALLBACK
#define PROGUI_VECTOR_FALLBACK 0
#endif

UIDemoComponent::UIDemoComponent()
{
    // Hide demo components to prevent overlap with Neptune knobs
    // Knob
    demoKnob.setRange(0.0, 100.0, 0.1);
    demoKnob.onValueChange = [this]{ level = (float) (demoKnob.getValue() / 100.0); repaint(); };
    demoKnob.setVisible(false); // HIDDEN

    // Slider
    demoSlider.setRange(0.0, 100.0, 0.1);
    demoSlider.onValueChange = [this]{ level2 = (float) (demoSlider.getValue() / 100.0); repaint(); };
    demoSlider.setVisible(false); // HIDDEN

    // Button
    demoButton.setButtonText("Button");
    demoButton.setVisible(false); // HIDDEN

    // Switch
    demoSwitch.setButtonText("Switch");
    demoSwitch.setClickingTogglesState(true);
    demoSwitch.setVisible(false); // HIDDEN

    this->addAndMakeVisible(btnAction);
    btnAction.setButtonText("Action");
    btnAction.onClick = [this]{ proguiLog("[UI] Action clicked"); };

    this->addAndMakeVisible(btnLoadSkin);
    btnLoadSkin.setButtonText("Load Skin…");
    btnLoadSkin.onClick = [this]
    {
        auto flags = juce::FileBrowserComponent::openMode | juce::FileBrowserComponent::canSelectDirectories;
        juce::FileChooser chooser ("Select a skin folder",
                                   juce::File::getSpecialLocation(juce::File::userHomeDirectory),
                                   "*");
        chooser.launchAsync(flags, [this](const juce::FileChooser& fc)
        {
            auto dir = fc.getResult();
            if (! dir.isDirectory()) return;
            SkinManager::instance().loadSkin(dir);
            proguiLog("[UI] Loaded skin folder: " + dir.getFullPathName());
            this->repaint();
        });
    };

    // Load Skin button
    loadSkinButton.setButtonText("Load Skin…");
    loadSkinButton.onClick = [this]
    {
        auto flags = juce::FileBrowserComponent::openMode | juce::FileBrowserComponent::canSelectFiles;
        auto initial = juce::File("/Volumes/MP 1/MoreMojo/Knobs");
        auto chooser = std::make_shared<juce::FileChooser>(
            "Select knob/slider skin (zip or image)", initial, "*.png;*.jpg;*.jpeg;*.zip");
        chooser->launchAsync(flags, [this, chooser](const juce::FileChooser& fc)
        {
            auto f = fc.getResult();
            if (! f.existsAsFile()) return;
            if (f.hasFileExtension("zip"))
            {
                juce::ZipFile zip (f);
                for (int i = 0; i < zip.getNumEntries(); ++i)
                {
                    auto* e = zip.getEntry(i);
                    if (e != nullptr && e->filename.toLowerCase().endsWith(".png"))
                    {
                        std::unique_ptr<juce::InputStream> in (zip.createStreamForEntry(i));
                        if (in != nullptr)
                        {
                            auto img = juce::ImageFileFormat::loadFrom (*in);
                            if (img.isValid()) { skinImage = img; break; }
                        }
                    }
                }
            }
            else
            {
                auto img = juce::ImageFileFormat::loadFrom (f);
                if (img.isValid()) skinImage = img;
            }
            this->repaint();
        });
    };
    this->addAndMakeVisible(loadSkinButton);

    // Pro/Consumer skin toggles
    this->addAndMakeVisible(btnProSkin);
    btnProSkin.setButtonText("Pro: kit-07");
    btnProSkin.onClick = [this]{ SkinManager::instance().loadDefaultProSkin(); meterMode = MeterMode::VU; repaint(); };

    this->addAndMakeVisible(btnConsumerSkin);
    btnConsumerSkin.setButtonText("Consumer: knob-67");
    btnConsumerSkin.onClick = [this]{ SkinManager::instance().loadDefaultConsumerSkinPrimary(); repaint(); };

    this->addAndMakeVisible(btnConsumerAltSkin);
    btnConsumerAltSkin.setButtonText("Consumer: knob-70");
    btnConsumerAltSkin.onClick = [this]{ SkinManager::instance().loadDefaultConsumerSkinAlt(); repaint(); };

    // Fantasma (Neptune) skin toggle
    this->addAndMakeVisible(btnFantasmaSkin);
    btnFantasmaSkin.setButtonText("Fantasma (Neptune)");
    btnFantasmaSkin.onClick = [this]{
        bool ok = SkinManager::instance().loadFantasmaAt("/Volumes/MP 1/MoreMojo/NeptuneUI/03 Delivery/VST/Assets/Animation");
        proguiLog(juce::String("[UI] Fantasma skin load ") + (ok ? "OK" : "FAILED"));
        repaint();
    };

    // Meter mode toggles
    this->addAndMakeVisible(btnVUMode);
    btnVUMode.setButtonText("Meter: VU");
    btnVUMode.onClick = [this]{ meterMode = MeterMode::VU; SkinManager::instance().loadDefaultVUMeter(); repaint(); };

    this->addAndMakeVisible(btnLEDMode);
    btnLEDMode.setButtonText("Meter: LED");
    btnLEDMode.onClick = [this]{ meterMode = MeterMode::LED; if (ledMeter1) ledMeter1->setVisible(true); if (ledMeter2) ledMeter2->setVisible(true); repaint(); };

    // Pro Master / Steal The Mojo entry points
    addAndMakeVisible(btnProMaster);
    addAndMakeVisible(btnStealMojo);
    // Helper: info text by process short label
    auto processInfoText = [](const juce::String& name)->juce::String
    {
        if (name == "ISP") return "Intersample Interpolation / True‑Peak (ISP)\n\nPractical use: Reduces aliasing, pre‑ringing, and intersample distortion.\nAudible result: Cleaner reconstruction, more analog‑like transients.\nBest on: Tracks and master.";
        if (name == "SRC") return "Resampler / SRC\n\nPractical use: Trade‑off between CPU and accuracy via reconstruction filters.\nAudible result: Lower pre‑ringing, more natural decay.\nBest on: Master and high‑fidelity tracks.";
        if (name == "Quant/Dither") return "Jitter Control & Phase Accumulator / Quant & Dither\n\nPractical use: Ensures ultra‑stable timing; control dither/noise shaping.\nAudible result: Subtle depth and spatial stability; controlled noise floor.\nBest on: Master.";
        if (name == "Align") return "FractionalDelayAlign (Phase/Time)\n\nPractical use: Per‑band delay and stereo ITD stabilizer improve coherence.\nAudible result: Clearer stereo imaging, improved coherence.\nBest on: Master or stereo busses.";
        if (name == "Transient") return "TransientDetect / Shaper\n\nPractical use: Prevents smearing of attacks; maintains percussive clarity.\nAudible result: Tighter, clearer attacks.\nBest on: Busses and master.";
        if (name == "De-esser") return "De‑esser / HF Control\n\nPractical use: Controls sibilance; band/amount controls.\nAudible result: Smoother highs without harshness.\nBest on: Vocals/busses.";
        if (name == "MLAR") return "More Like The Record (MLAR)\n\nPractical use: Macro improves timing/phase/transients with profile presets.\nAudible result: Commercial polish; cohesive timing/phase.\nBest on: Tracks, busses, master.";
        if (name == "Transformer") return "Transformer (Harmonic Driver)\n\nPractical use: Even/odd harmonic drive, bias; core modes with dynamics.\nAudible result: Harmonic richness (H2/H3), musical density.\nBest on: Tracks/busses/master.";
        return name + "\n\nSee documentation for details.";
    };

    btnProMaster.onClick = [this, processInfoText]
    {
        auto* comp = new ProMasterComponent();
        comp->onOpenProcess = [this, processInfoText](const juce::String& name)
        {
            // For now, show info dialog until subwindows are fully implemented
            juce::AlertWindow::showMessageBoxAsync(juce::AlertWindow::InfoIcon, 
                name + " Process", 
                processInfoText(name), 
                "OK");
        };
        juce::DialogWindow::LaunchOptions opts;
        opts.content.setOwned(comp);
        opts.dialogTitle = "Professional — Master";
        opts.componentToCentreAround = this;
        opts.escapeKeyTriggersCloseButton = true;
        opts.useNativeTitleBar = true;
        opts.resizable = true;
        opts.launchAsync();
    };
    btnStealMojo.onClick = [this]
    {
        auto* comp = new PsychedelicStealMojoComponent();
        comp->setSize(700, 900);
        juce::DialogWindow::LaunchOptions opts;
        opts.content.setOwned(comp);
        opts.dialogTitle = "✨ Steal The Mojo ✨";
        opts.componentToCentreAround = this;
        opts.escapeKeyTriggersCloseButton = true;
        opts.useNativeTitleBar = true;
        opts.resizable = true;
        opts.launchAsync();
    };

    // Master bypass (UI only)
    addAndMakeVisible(masterBypass);
    masterBypass.onClick = [this]{ isBypassed = masterBypass.getToggleState(); repaint(); };

    // Zoom controls (UI only)
    addAndMakeVisible(zoomMinus);
    addAndMakeVisible(zoomPlus);
    zoomMinus.onClick = [this]{ if (waveform) waveform->zoomOut(); };
    zoomPlus.onClick  = [this]{ if (waveform) waveform->zoomIn();  };

    // Default demo: try Fantasma (Neptune) first; fall back to Pro skin + VU meter
    if (! SkinManager::instance().loadFantasmaAt("/Volumes/MP 1/MoreMojo/NeptuneUI/03 Delivery/VST/Assets/Animation"))
    {
        SkinManager::instance().loadDefaultProSkin();
        SkinManager::instance().loadDefaultVUMeter();
    }
    
    // Configure I/O sliders - SkinnedSlider handles configuration internally
    inputTrim.setValue(0.0);   // Zero position (0dB)
    outputTrim.setValue(0.0);  // Zero position (0dB)
    meterMode = MeterMode::LED; // show LED meters by default for visibility

    // Create LED meters (hidden by default unless LED mode)
    LedBarMeter::Config cfg; cfg.numSegments = 56; cfg.orientation = LedBarMeter::Vertical;
    ledMeter1 = std::make_unique<LedBarMeter>(cfg);
    ledMeter2 = std::make_unique<LedBarMeter>(cfg);
    addAndMakeVisible(*ledMeter1);
    addAndMakeVisible(*ledMeter2);
    ledMeter1->setVisible(true);
    ledMeter2->setVisible(true);

    // Create waveform view (center panel)
    waveform = std::make_unique<WaveformView>();
    addAndMakeVisible(*waveform);

    // Create transport bar (UI only)
    transport = std::make_unique<TransportBar>();
    addAndMakeVisible(*transport);

    // Create Neptune mix knob (pure Neptune, no ring)
    mixKnob = std::make_unique<NeptuneMixKnob>();
    mixKnob->setValue(0.5f); // 50% mix
    mixKnob->onValueChanged = [this](float value) { 
        mixValue = value; 
        repaint(); 
    };
    addAndMakeVisible(*mixKnob);

    // Create horizontal bar meters for original and processed signals
    originalMeter = std::make_unique<HorizontalBarMeter>("ORIGINAL");
    processedMeter = std::make_unique<HorizontalBarMeter>("PROCESSED");
    addAndMakeVisible(*originalMeter);
    addAndMakeVisible(*processedMeter);

    // Create enhanced placeholder components that look like ProcessControl (temporary workaround)
    const char* processNames[] = {"IMAGE SIGNAL PROCESSING", "SAMPLE RATE CONVERSION", "JITTER & ACCUMULATION", "PHASE ALIGNMENT", "TRANSIENT SHAPING", "DE-ESSING", "MORE LIKE A RECORD", "TRANSFORMER"};
    const char* descriptions[] = {"Restores Digital Warmth", "Eliminates Digital Harshness", "Reduces Digital Artifacts", "Creates Spatial Coherence", "Adds Punch & Presence", "Removes Harsh Frequencies", "Vintage Analog Character", "Analog Saturation"};
    
    for (int i = 0; i < 8; ++i)
    {
        // Create enhanced placeholder components
        placeholderComponents[i] = std::make_unique<PlaceholderComponent>(processNames[i]);
        
        // Store descriptions for later use
        placeholderComponents[i]->setDescription(descriptions[i]);
        
        // Keep ProcessControl as nullptr for now due to linking issues
        processControls[i] = nullptr;
        
        // Add the enhanced placeholder to show the positioning works
        addAndMakeVisible(*placeholderComponents[i]);
    }

    // IO trims and labels
    addAndMakeVisible(inputTrim);
    addAndMakeVisible(outputTrim);
    addAndMakeVisible(lblIn);
    addAndMakeVisible(lblOut);
    lblIn.setJustificationType(juce::Justification::centred);
    lblOut.setJustificationType(juce::Justification::centred);
    lblIn.setColour(juce::Label::textColourId, juce::Colours::white.withAlpha(0.85f));
    lblOut.setColour(juce::Label::textColourId, juce::Colours::white.withAlpha(0.85f));

    this->startTimerHz(30);
    // Ensure initial layout even if no resize event yet
    this->resized();
}

void UIDemoComponent::paint(juce::Graphics& g)
{
    // Use skin/theme colours if available
    auto& skin = SkinManager::instance().skin();
    auto BG   = skin.panel.isTransparent() ? ui::theme().bg : skin.panel;
    auto TXT  = skin.text.isTransparent()  ? ui::theme().text : skin.text;
    auto ACC  = skin.accent.isTransparent()? ui::theme().accent : skin.accent;
    auto ACC2 = skin.accent2.isTransparent()? ui::theme().accent2 : skin.accent2;
    auto CARD = skin.panel.isTransparent() ? ui::theme().card : skin.panel.brighter(0.07f);

    g.fillAll(BG);
    // Clean surface; removed debug stripes, guides, and oversized diagnostic banner
    // Top-right status: removed for clean interface

#if 0 // PROGUI_VECTOR_FALLBACK - DISABLED to prevent overlap with Neptune knobs
    // Draw minimal modern-looking controls directly (fallback visuals)
    juce::Rectangle<int> knobR (32, 64, 160, 160);
{{ ... }}
    juce::Rectangle<int> sldR  (32, 240, juce::jmax(220, this->getWidth() - 200), 26);
    juce::Rectangle<int> btnR  (32, 280, 120, 28);
    juce::Rectangle<int> togR  (168, 280, 120, 28);

    // Knob: use filmstrip frames if provided; else single PNG; else vector 3D fallback
    {
        auto rf = knobR.toFloat();
        auto c = rf.getCentre();
        float rad = juce::jmin(rf.getWidth(), rf.getHeight()) * 0.5f;
        if (! skin.knobFrames.empty())
        {
            auto n = (int) skin.knobFrames.size();
            int idx = juce::jlimit(0, n-1, (int) juce::roundToInt(level * (float)(n - 1)));
            const auto& frame = skin.knobFrames[(size_t) idx];
            g.drawImageWithin(frame, knobR.getX(), knobR.getY(), knobR.getWidth(), knobR.getHeight(), juce::RectanglePlacement::centred, false);
        }
        else if (skin.knobImage.isValid())
        {
            g.drawImageWithin(skin.knobImage, knobR.getX(), knobR.getY(), knobR.getWidth(), knobR.getHeight(), juce::RectanglePlacement::centred, false);
        }
        else
        {
            // Vector 3D body
            juce::Colour bodyTop = CARD.brighter(0.20f);
            juce::Colour bodyBot = CARD.darker(0.20f);
            juce::ColourGradient bodyGrad(bodyTop, rf.getX()+rf.getWidth()*0.35f, rf.getY()+rf.getHeight()*0.35f,
                                          bodyBot, rf.getRight(), rf.getBottom(), true);
            g.setGradientFill(bodyGrad);
            g.fillEllipse(rf);
            // Rim
            g.setColour(juce::Colours::black.withAlpha(0.4f));
            g.drawEllipse(rf, 1.5f);
            // Inner shadow ring
            juce::Path inner; inner.addEllipse(rf.reduced(6.0f));
            g.saveState();
            g.reduceClipRegion(inner);
            juce::ColourGradient shadowGrad(juce::Colours::black.withAlpha(0.25f), rf.getRight(), rf.getBottom(),
                                            juce::Colours::transparentBlack, c.x, c.y, true);
            g.setGradientFill(shadowGrad);
            g.fillEllipse(rf);
            g.restoreState();
            // Specular highlight
            auto hl = juce::Rectangle<float>(rf.getX()+rf.getWidth()*0.18f, rf.getY()+rf.getHeight()*0.18f,
                                             rf.getWidth()*0.40f, rf.getHeight()*0.22f);
            g.setColour(juce::Colours::white.withAlpha(0.22f));
            g.fillEllipse(hl);
        }
        // Depth arc (value)
        float start = juce::MathConstants<float>::pi * 1.2f;
        float end   = juce::MathConstants<float>::pi * (1.2f + 1.2f);
        float ang   = start + (end - start) * juce::jlimit(0.0f, 1.0f, level);
        juce::Path arc; arc.addCentredArc(c.x, c.y, rad-12.f, rad-12.f, 0, start, ang, true);
        g.setColour(ACC.withAlpha(0.95f));
        g.strokePath(arc, juce::PathStrokeType(4.0f, juce::PathStrokeType::beveled, juce::PathStrokeType::rounded));
        // Arc glow
        g.setColour(ACC.withAlpha(0.18f));
        g.strokePath(arc, juce::PathStrokeType(8.0f));
    }

    // Slider (3D: beveled track + glossy fill)
    {
        auto rs = sldR.toFloat();
        auto track = juce::Rectangle<float>(rs.getX(), rs.getCentreY() - 5.f, rs.getWidth(), 10.f);
        // Track bevel
        juce::Colour tTop = CARD.brighter(0.10f);
        juce::Colour tBot = CARD.darker(0.20f);
        g.setGradientFill({tTop, track.getX(), track.getY(), tBot, track.getX(), track.getBottom(), false});
        g.fillRoundedRectangle(track, 5.f);
        g.setColour(juce::Colours::white.withAlpha(0.10f));
        g.drawLine({track.getX()+2, track.getY()+2, track.getRight()-2, track.getY()+2}, 1.0f);
        // Fill with gloss
        auto fill = track.withWidth(track.getWidth() * juce::jlimit(0.f, 1.f, level2));
        juce::Colour f1 = ACC.brighter(0.20f);
        juce::Colour f2 = ACC.darker(0.15f);
        g.setGradientFill({f1, fill.getX(), fill.getY(), f2, fill.getX(), fill.getBottom(), false});
        g.fillRoundedRectangle(fill, 5.f);
        // Gloss highlight
        auto gloss = fill.withHeight(fill.getHeight() * 0.45f).reduced(2, 1);
        g.setColour(juce::Colours::white.withAlpha(0.12f));
        g.fillRoundedRectangle(gloss, 4.f);
    }

    // Button (use skin filmstrip frames when available)
    {
        auto bf = btnR.toFloat();
        const auto& frames = skin.buttonFrames;
        if (! frames.empty())
        {
            bool pressed = demoButton.isDown();
            int idx = juce::jlimit(0, (int)frames.size()-1, pressed ? 1 : 0);
            const auto& img = frames[(size_t)idx];
            g.drawImageWithin(img, btnR.getX(), btnR.getY(), btnR.getWidth(), btnR.getHeight(), juce::RectanglePlacement::stretchToFit, false);
            g.setColour(juce::Colours::white.withAlpha(0.90f));
            g.drawFittedText("Button", btnR, juce::Justification::centred, 1);
        }
        else
        {
            // Vector fallback
            juce::DropShadow(juce::Colours::black.withAlpha(0.35f), 10, {0,3}).drawForRectangle(g, btnR);
            juce::Colour bTop = CARD.brighter(0.10f);
            juce::Colour bBot = CARD.darker(0.10f);
            g.setGradientFill({bTop, bf.getX(), bf.getY(), bBot, bf.getX(), bf.getBottom(), false});
            g.fillRoundedRectangle(bf, 6.f);
            auto gloss = bf.withHeight(bf.getHeight()*0.45f).reduced(2,1);
            g.setColour(juce::Colours::white.withAlpha(0.10f));
            g.fillRoundedRectangle(gloss, 5.f);
            g.setColour(juce::Colours::white.withAlpha(0.90f));
            g.drawFittedText("Button", btnR, juce::Justification::centred, 1);

        // Mix knob above Professional Mojo Controls
        if (mixKnobArea.getHeight() > 0)
        auto tf = togR.toFloat();
        const auto& frames = skin.switchFrames;
        if (! frames.empty())
        {
            bool on = demoSwitch.getToggleState();
            int idx = juce::jlimit(0, (int)frames.size()-1, on ? 1 : 0);
            const auto& img = frames[(size_t)idx];
            g.drawImageWithin(img, togR.getX(), togR.getY(), togR.getWidth(), togR.getHeight(), juce::RectanglePlacement::stretchToFit, false);
            g.setColour(juce::Colours::white.withAlpha(0.90f));
            g.drawFittedText(on ? "On" : "Off", togR, juce::Justification::centred, 1);
        }
        else
        {
            // Vector fallback
            juce::Colour tTop = CARD.brighter(0.12f);
            juce::Colour tBot = CARD.darker(0.12f);
            g.setGradientFill({tTop, tf.getX(), tf.getY(), tBot, tf.getX(), tf.getBottom(), false});
            g.fillRoundedRectangle(tf, tf.getHeight()/2.f);
            bool on = demoSwitch.getToggleState();
            auto knob = juce::Rectangle<float>(tf.getX()+ (on ? tf.getWidth()-tf.getHeight() : 0.f), tf.getY(), tf.getHeight(), tf.getHeight());
            juce::Colour kTop = CARD.brighter(0.18f);
            juce::Colour kBot = CARD.darker(0.18f);
            g.setGradientFill({kTop, knob.getX(), knob.getY(), kBot, knob.getX(), knob.getBottom(), false});
            g.fillEllipse(knob);
            g.setColour(juce::Colours::white.withAlpha(0.15f));
            g.fillEllipse(knob.reduced(4).withHeight(knob.getHeight()*0.45f));
            g.setColour(juce::Colours::white.withAlpha(0.85f));
            g.drawFittedText(on ? "On" : "Off", togR, juce::Justification::centred, 1);
        }
    }
    juce::Rectangle<int> skinR (304, 280, 140, 28);
    g.drawText("[Load Skin]", skinR, juce::Justification::centred);
#endif // PROGUI_VECTOR_FALLBACK

    // Layout regions: left / center (waveform) / right
    auto outer = this->getLocalBounds().reduced(16);
    const int colW = 44; // very narrow gutters
    const int gap = 8;
    auto leftCol  = outer.removeFromLeft(colW);
    outer.removeFromLeft(gap);
    auto rightCol = outer.removeFromRight(colW);
    outer.removeFromRight(gap);
    auto centerPanel = outer; // waveform area

    if (meterMode == MeterMode::LED)
    {
        // Ensure LED meters are visible and skinned (no extra overlays)
        if (ledMeter1 && ledMeter2)
        {
            const auto& sk = SkinManager::instance().skin();
            if (sk.meterLEDStrip.isValid())
            {
                ledMeter1->setSpriteImage(sk.meterLEDStrip);
                ledMeter2->setSpriteImage(sk.meterLEDStrip);
            }
            ledMeter1->setVisible(true);
            ledMeter2->setVisible(true);
        }
    }
    else
    {
        // Hide LED meters and draw VU/gradient versions
        if (ledMeter1) ledMeter1->setVisible(false);
        if (ledMeter2) ledMeter2->setVisible(false);
        auto m1 = leftCol.reduced(8).removeFromTop(outer.getHeight()/2 - 6);
        auto m2 = rightCol.reduced(8).removeFromTop(outer.getHeight()/2 - 6);
        drawMeter(g, m1, level, ACC, meterMode);
        drawMeter(g, m2, level2, ACC2, meterMode);
    }

    // (Header removed to prevent any overlap at the top)

    // Fill center area with Pro Master controls
    {
        auto outer = this->getLocalBounds().reduced(16);
        const int colW = 44;
        const int gap = 8;
        auto leftCol  = outer.removeFromLeft(colW);
        outer.removeFromLeft(gap);
        auto rightCol = outer.removeFromRight(colW);
        outer.removeFromRight(gap);
        auto centerPanel = outer;
        
        auto workArea = centerPanel.reduced(0, 20); // reduced top margin
        int wfH = juce::jlimit(50, 70, workArea.getHeight() - 25); // smaller waveform to match resized()
        workArea.removeFromTop(wfH); // skip waveform area
        workArea.removeFromTop(35);  // skip transport area (increased size for better visibility)
        
        // Apply same margin reduction to prevent knob overlap
        workArea = workArea.reduced(8, 0); // smaller side margins
        
        // Mix knob above Professional Mojo Controls - balanced size
        auto mixKnobArea = workArea.removeFromTop(100); // balanced size between knob visibility and space for processes
        workArea.removeFromTop(2); // very minimal gap between knob and Mojo window
        
        if (mixKnobArea.getHeight() > 0)
        {
            // Draw labels for meters and mix knob
            auto labelRect = mixKnobArea.removeFromBottom(18);
            
            g.setColour(juce::Colours::white);
            g.setFont(juce::Font("Arial", 10.0f, juce::Font::bold));
            g.drawText("MIX", labelRect, juce::Justification::centred);
        }
        
        auto remainingArea = workArea.reduced(8);
        if (remainingArea.getHeight() > 100)
        {
            // Keep background area in original position
            auto backgroundArea = remainingArea;
            
            // Draw Pro Master section with gradient background - extended left only
            auto bg = juce::ColourGradient(juce::Colour(0xFF1a1a2e), backgroundArea.getX(), backgroundArea.getY(),
                                         juce::Colour(0xFF16213e), backgroundArea.getX(), backgroundArea.getBottom(), false);
            g.setGradientFill(bg);
            g.fillRoundedRectangle(backgroundArea.toFloat(), 12.0f);
            
            // Keep border area in original position
            auto borderArea = remainingArea;
            
            // Stylish border - extended left only to encompass Neptune knobs
            g.setColour(juce::Colour(0xFF0f3460).withAlpha(0.8f));
            g.drawRoundedRectangle(borderArea.toFloat(), 12.0f, 2.0f);
            
            // Header with 3D effect
            auto headerArea = remainingArea.removeFromTop(45);
            // 3D shadow
            g.setColour(juce::Colour(0xFF000000).withAlpha(0.8f));
            g.setFont(juce::Font("Arial Black", 18.0f, juce::Font::bold));
            g.drawText("PROFESSIONAL MOJO CONTROLS", headerArea.translated(2, 2), juce::Justification::centred);
            // Main text with gradient effect
            g.setColour(juce::Colour(0xFFffffff));
            g.drawText("PROFESSIONAL MOJO CONTROLS", headerArea, juce::Justification::centred);
            
            // OLD DRAWING CODE DISABLED - ProcessControl components handle all drawing now
            /*
            const char* processes[] = {"IMAGE SIGNAL PROCESSING", "SAMPLE RATE CONVERSION", "JITTER & ACCUMULATION", "PHASE ALIGNMENT", "TRANSIENT SHAPING", "DE-ESSING", "MORE LIKE A RECORD", "TRANSFORMER"};
            const char* descriptions[] = {"Restores Digital Warmth", "Eliminates Digital Harshness", "Reduces Digital Artifacts", "Creates Spatial Coherence", "Adds Punch & Presence", "Removes Harsh Frequencies", "Vintage Analog Character", "Analog Saturation"};
            
            // Create a 2x4 grid layout for all 8 processes
            int cols = 2, rows = 4;
            int cellW = (remainingArea.getWidth() - 30) / cols;
            int cellH = juce::jmax(120, (remainingArea.getHeight() - 20) / rows);
            
            for (int i = 0; i < 8; ++i)
            {
                int col = i % cols;
                int row = i / cols;
                
                // SHIFT LEFT COLUMN TO THE RIGHT to make room for knobs
                int xOffset = (col == 0) ? 80 : 0; // Move left column 80px to the right
                
                auto cellArea = juce::Rectangle<int>(
                    remainingArea.getX() + col * cellW + 10 + xOffset,
                    remainingArea.getY() + row * cellH + 5,
                    cellW - 20, cellH - 10);

                // Left column content shifted right to make room for knobs
                
                // Skip knob area (now handled by actual Neptune knob components)
                // auto knobArea = cellArea.removeFromLeft(juce::jmax(85, cellArea.getWidth() / 4)); // DISABLED - using positioned Neptune knobs instead
                
                // Neptune button positioning - symmetric layout
                // Left column buttons moved left to compensate for content shift
                int btnOffset = (col == 0) ? -90 : 0; // Only left column buttons adjusted
                auto btnArea = cellArea.removeFromRight(110).reduced(3, 8).translated(btnOffset, 0);
                
                // Use Neptune button if available
                if (! skin.buttonFrames.empty() && skin.buttonFrames.size() >= 2)
                {
                    auto img = skin.buttonFrames[0];
                    float scale = juce::jmin(btnArea.getWidth() / (float)img.getWidth(), 
                                           btnArea.getHeight() / (float)img.getHeight()) * 0.9f;
                    auto dest = juce::Rectangle<float>(img.getWidth() * scale, img.getHeight() * scale)
                                  .withCentre(btnArea.getCentre().toFloat());
                    g.drawImage(img, dest);
                    
                    g.setColour(juce::Colours::white);
                    g.setFont(juce::Font("Arial", 8.0f, juce::Font::bold));
                    g.drawText("OPEN CONTROLS", btnArea, juce::Justification::centred);
                }
                else
                {
                    // Fallback button
                    auto btnGrad = juce::ColourGradient(juce::Colour(0xFF4a90e2), btnArea.getX(), btnArea.getY(),
                                                      juce::Colour(0xFF2171b5), btnArea.getX(), btnArea.getBottom(), false);
                    g.setGradientFill(btnGrad);
                    g.fillRoundedRectangle(btnArea.toFloat(), 6.0f);
                    g.setColour(juce::Colours::white);
                    g.setFont(juce::Font("Arial", 8.0f, juce::Font::bold));
                    g.drawText("OPEN CONTROLS", btnArea, juce::Justification::centred);
                }
                
                // Process labels (center area)
                auto labelArea = cellArea.reduced(8, 5);
                
                // Horizontal level bar FIRST - above process name
                auto levelArea = labelArea.removeFromTop(8).reduced(0, 1);
                
                g.setColour(juce::Colour(0xFF333333));
                g.fillRoundedRectangle(levelArea.toFloat(), 2.0f);
                // Active level (controlled by Neptune knob)
                float level = processLevels[i];
                auto activeLevelArea = levelArea.removeFromLeft((int)(levelArea.getWidth() * level));
                g.setColour(juce::Colour(0xFF00d4aa));
                g.fillRoundedRectangle(activeLevelArea.toFloat(), 2.0f);
                
                // DEBUG: Draw red rectangle around cyan slider area (AFTER drawing slider)
                g.setColour(juce::Colours::red);
                g.drawRect(levelArea.toFloat(), 3.0f);
                
                // DEBUG: Print actual cyan bar coordinates for knob positioning
                std::cout << "Process " << i << " (" << processes[i] << ") cyan bar at: (" 
                          << levelArea.getX() << "," << levelArea.getY() << "," 
                          << levelArea.getWidth() << "," << levelArea.getHeight() << ")" << std::endl;
                
                // Small gap after meter
                labelArea.removeFromTop(3);
                
                
                // Process name with smaller title text
                auto nameRect = labelArea.removeFromTop(22);
                // Simple shadow for depth
                g.setColour(juce::Colour(0xFF000000).withAlpha(0.8f));
                g.setFont(juce::Font("Arial", 15.0f, juce::Font::bold));
                g.drawText(processes[i], nameRect.translated(1, 1), juce::Justification::centredLeft);
                // Main text - bright and clear
                g.setColour(juce::Colour(0xFFffffff));
                g.drawText(processes[i], nameRect, juce::Justification::centredLeft);
                
                // Description with cyan accent - bigger font, single line
                auto descRect = labelArea.removeFromTop(20);
                g.setColour(juce::Colour(0xFF00ffff));
                g.setFont(juce::Font("Arial", 13.0f, juce::Font::bold));
                g.drawText(descriptions[i], descRect, juce::Justification::centredLeft);
                
                // Detailed description - single instance, no wrapping to prevent doubling
                auto detailRect = labelArea;
                g.setColour(juce::Colour(0xFFcccccc));
                g.setFont(juce::Font("Arial", 9.0f, juce::Font::plain));
                g.drawText(descriptions[i], detailRect, juce::Justification::topLeft, false); // false = no wrapping
            }
            */
        }
    }


    // DEBUG DISABLED - Neptune knobs are positioned correctly!
    // The 2x4 grid layout is working perfectly as confirmed by visual inspection

    // Visual bypass overlay
    if (isBypassed)
    {
        auto b = this->getLocalBounds().toFloat();
        g.setColour(juce::Colours::black.withAlpha(0.40f));
        g.fillRoundedRectangle(b.reduced(6), 10.0f);
        g.setColour(juce::Colours::white.withAlpha(0.85f));
        g.setFont(juce::Font(18.0f, juce::Font::bold));
        g.drawFittedText("BYPASSED", this->getLocalBounds(), juce::Justification::centred, 1);
    }
}

void UIDemoComponent::resized()
{
    auto bounds = this->getLocalBounds();
    proguiLog("[UIDemoComponent::resized] Component bounds: " + bounds.toString());
    
    auto outer = bounds.reduced(8, 16);  // Reasonable margins for proper layout
    const int colW = 200; // further extended wide columns to accommodate Neptune knobs positioned further left
    const int gap = 8;
    proguiLog("[DEBUG] Using colW=" + juce::String(colW) + " gap=" + juce::String(gap));
    auto leftCol  = outer.removeFromLeft(colW);
    outer.removeFromLeft(gap);
    auto rightCol = outer.removeFromRight(colW);
    outer.removeFromRight(gap);
    auto centerPanel = outer;

    // Leave space at top for status banner, then place waveform and transport
    auto workArea = centerPanel.reduced(0, 2); // very minimal top margin
    int wfH = juce::jlimit(50, 70, workArea.getHeight() - 25); // slightly smaller waveform
    auto waveformArea  = workArea.removeFromTop(wfH);
    auto transportArea = workArea.removeFromTop(35); // increased transport size for better visibility
    
    // Reserve space for Mix knob above Professional Mojo Controls
    auto mixKnobArea = workArea.removeFromTop(100); // balanced size between knob visibility and space for processes
    workArea.removeFromTop(2); // very minimal gap between knob and Mojo window
    
    // Reduce remaining area to prevent overlap with side knobs
    workArea = workArea.reduced(10, 0); // smaller margin since columns are wider now
    
    if (waveform) waveform->setBounds(waveformArea);
    
    // Position Neptune mix knob and horizontal bar meters
    if (mixKnobArea.getHeight() > 0)
    {
        auto meterAndKnobArea = mixKnobArea.reduced(20, 18); // leave space for labels at bottom
        
        // Divide into three sections: Left meter | Mix Knob | Right meter
        auto leftMeterArea = meterAndKnobArea.removeFromLeft(meterAndKnobArea.getWidth() / 3);
        auto rightMeterArea = meterAndKnobArea.removeFromRight(meterAndKnobArea.getWidth() / 2);
        auto mixKnobAreaCenter = meterAndKnobArea;
        
        // Position horizontal bar meters (on either side of mix knob)
        if (originalMeter)
        {
            auto meterRect = leftMeterArea.reduced(5).withSizeKeepingCentre(leftMeterArea.getWidth()-10, 20);
            originalMeter->setBounds(meterRect);
        }
        
        if (processedMeter)
        {
            auto meterRect = rightMeterArea.reduced(5).withSizeKeepingCentre(rightMeterArea.getWidth()-10, 20);
            processedMeter->setBounds(meterRect);
        }
        
        // Position Neptune mix knob (center)
        if (mixKnob)
        {
            auto knobRect = mixKnobAreaCenter.withSizeKeepingCentre(80, 80);
            mixKnob->setBounds(knobRect);
        }
    }
    
    // Position Neptune process knobs using absolute positioning relative to Professional Mojo Controls
    if (workArea.getHeight() > 100)
    {
        auto processArea = workArea.reduced(8);
        
        // Define absolute positions for each knob (where you want them to be)
        struct KnobPosition { int x, y; };
        KnobPosition knobPositions[8] = {
            // Left column (ISP, JITTER, TRANSIENT, MLAR) - move left and down
            {processArea.getX() + 20, processArea.getY() + 40},   // ISP (0)
            {processArea.getX() + 20, processArea.getY() + 160},  // JITTER (2)  
            {processArea.getX() + 20, processArea.getY() + 280},  // TRANSIENT (4)
            {processArea.getX() + 20, processArea.getY() + 400},  // MLAR (6)
            
            // Right column (SRC, ALIGN, DEESSER, TRANSFORMER) - move right and down  
            {processArea.getX() + processArea.getWidth()/2 + 20, processArea.getY() + 40},   // SRC (1)
            {processArea.getX() + processArea.getWidth()/2 + 20, processArea.getY() + 160},  // ALIGN (3)
            {processArea.getX() + processArea.getWidth()/2 + 20, processArea.getY() + 280},  // DEESSER (5)
            {processArea.getX() + processArea.getWidth()/2 + 20, processArea.getY() + 400}   // TRANSFORMER (7)
        };
        
        // Position placeholder components in 2x4 grid (demonstrating ProcessControl architecture)
        // Make components much larger to be more prominent and easier to use
        int cols = 2, rows = 4;
        int cellW = 300;  // Even wider components for better presence
        int cellH = 100;  // Much taller for better proportions and readability
        int spacingX = 15; // Horizontal spacing between columns (reduced to fit larger components)
        int spacingY = 20; // Vertical spacing between rows (reduced to fit taller components)
        int marginX = 20;  // Side margins
        int marginY = 50;  // Much larger top margin to clear the "MIX" title
        
        // Calculate grid dimensions
        int gridWidth = cols * cellW + (cols - 1) * spacingX;
        int gridHeight = rows * cellH + (rows - 1) * spacingY;
        
        // Ensure the grid fits within the processArea with proper margins
        int totalHeightNeeded = gridHeight + (2 * marginY);
        if (totalHeightNeeded > processArea.getHeight()) {
            // Reduce component height if needed to fit
            cellH = (processArea.getHeight() - (2 * marginY) - (3 * spacingY)) / rows;
            cellH = juce::jmax(40, cellH); // Minimum height
        }
        
        // Center horizontally within processArea, start with top margin
        int startX = processArea.getX() + (processArea.getWidth() - gridWidth) / 2;
        int startY = processArea.getY() + marginY;
        
        for (int i = 0; i < 8; ++i)
        {
            if (placeholderComponents[i])
            {
                int col = i % 2;  // 0 = left column, 1 = right column
                int row = i / 2;  // 0-3 for the 4 rows
                
                auto cellArea = juce::Rectangle<int>(
                    startX + col * (cellW + spacingX),
                    startY + row * (cellH + spacingY),
                    cellW, cellH);
                
                placeholderComponents[i]->setBounds(cellArea);
            }
        }
    }
    
    // Position transport controls properly below waveform
    if (transport)
    {
        // Give transport the full area with minimal margins
        auto transportStrip = transportArea.reduced(4, 2);
        // Center transport, leaving space for zoom buttons
        auto transportWidth = transportStrip.getWidth() - 100; // reserve 100px for zoom buttons
        auto transportX = transportStrip.getX() + 20; // small left margin
        auto transRect = juce::Rectangle<int>(transportX, transportStrip.getY(), transportWidth, transportStrip.getHeight());
        transport->setBounds(transRect);
        
        // Debug log to see what's happening
        proguiLog("[transport] Area: " + transportArea.toString() + " Strip: " + transportStrip.toString() + " Final: " + transRect.toString());
    }
    
    // Place master bypass top-left (outside outer)
    masterBypass.setBounds(16, 8, 90, 24);
    // Place zoom buttons at right edge of transport area, no overlap
    {
        int w = 26, h = 20, gapB = 4;
        int rx = transportArea.getRight() - 4;
        int cy = transportArea.getCentreY() - h/2;
        zoomPlus.setBounds (rx - w, cy, w, h);
        zoomMinus.setBounds(rx - w*2 - gapB, cy, w, h);
    }
    // Visual guide for transport area to verify visibility
    proguiLog("[resized] ledMeter1=" + (ledMeter1 ? ledMeter1->getBounds().toString() : juce::String("<null>"))
              + " ledMeter2=" + (ledMeter2 ? ledMeter2->getBounds().toString() : juce::String("<null>"))
              + " waveform=" + (waveform ? waveform->getBounds().toString() : juce::String("<null>"))
              + " transport=" + (transport ? transport->getBounds().toString() : juce::String("<null>")));

    // Place LED meters directly above vertical sliders
    auto leftArea  = leftCol.reduced(5); // minimal spacing
    auto rightArea = rightCol.reduced(5);
    
    // Leave space at top for bypass button and status banner
    auto leftAreaWithMargin = leftArea.withTrimmedTop(25);
    auto rightAreaWithMargin = rightArea.withTrimmedTop(25);
    
    // Ensure areas have minimum size
    if (leftAreaWithMargin.getWidth() > 0 && leftAreaWithMargin.getHeight() > 200)
    {
        // Left column: dynamic sizing with slider closer to meter
        int availableHeight = leftAreaWithMargin.getHeight();
        int sliderHeight = juce::jmax(140, (int)(availableHeight * 0.45f)); // 45% of available height, min 140px
        int meterHeight = juce::jmax(180, availableHeight - sliderHeight - 30); // less gap between meter and slider
        
        auto leftSliderArea = leftAreaWithMargin.removeFromBottom(sliderHeight + 25);
        auto labelArea = leftSliderArea.removeFromTop(25);
        // Position Input label within the left column area - BACK TO ORIGINAL POSITION
        auto centeredLabelArea = labelArea.withWidth(50).withX(leftAreaWithMargin.getX() + 10); // back to original
        lblIn.setBounds(centeredLabelArea);
        
        // Meter directly above slider area - taller and responsive, closer to slider - BACK TO ORIGINAL
        auto meterArea = leftAreaWithMargin.removeFromBottom(meterHeight + 10); // extend meter closer to slider
        if (ledMeter1) {
            auto meterRect = meterArea.withWidth(50).withX(leftAreaWithMargin.getX() + 10); // back to original
            ledMeter1->setBounds(meterRect);
        }
        
        // Slider below meter - responsive sizing, closer to meter - BACK TO ORIGINAL
        auto sliderRect = leftSliderArea.withWidth(60).withX(leftAreaWithMargin.getX() + 5); // back to original
        inputTrim.setBounds(sliderRect);
    }
    
    if (rightAreaWithMargin.getWidth() > 0 && rightAreaWithMargin.getHeight() > 200)
    {
        // Right column: dynamic sizing with slider closer to meter
        int availableHeight = rightAreaWithMargin.getHeight();
        int sliderHeight = juce::jmax(140, (int)(availableHeight * 0.45f)); // 45% of available height, min 140px
        int meterHeight = juce::jmax(180, availableHeight - sliderHeight - 30); // less gap between meter and slider
        
        auto rightSliderArea = rightAreaWithMargin.removeFromBottom(sliderHeight + 25);
        auto labelArea = rightSliderArea.removeFromTop(25);
        // Center the Output label under the meter
        auto centeredLabelArea = labelArea.withWidth(50).withX(rightAreaWithMargin.getRight() - 60); // same X as meter
        lblOut.setBounds(centeredLabelArea);
        
        // Meter directly above slider area - taller and responsive, closer to slider
        auto meterArea = rightAreaWithMargin.removeFromBottom(meterHeight + 10); // extend meter closer to slider
        if (ledMeter2) {
            auto meterRect = meterArea.withWidth(50).withX(rightAreaWithMargin.getRight() - 60);
            ledMeter2->setBounds(meterRect);
        }
        
        // Slider below meter - responsive sizing, closer to meter
        auto sliderRect = rightSliderArea.withWidth(60).withX(rightAreaWithMargin.getRight() - 65);
        outputTrim.setBounds(sliderRect);
    }
    // Hide most control buttons to clean up interface - only keep essential ones
    loadSkinButton.setBounds(0, 0, 0, 0); // hidden
    btnAction.setBounds(0, 0, 0, 0); // hidden
    btnLoadSkin.setBounds(0, 0, 0, 0); // hidden
    btnProSkin.setBounds(0, 0, 0, 0); // hidden
    btnConsumerSkin.setBounds(0, 0, 0, 0); // hidden
    btnConsumerAltSkin.setBounds(0, 0, 0, 0); // hidden
    btnFantasmaSkin.setBounds(0, 0, 0, 0); // hidden
    btnVUMode.setBounds(0, 0, 0, 0); // hidden
    btnLEDMode.setBounds(0, 0, 0, 0); // hidden
    
    // Show Consumer button in top-right, hide Pro Master
    btnProMaster.setBounds(0, 0, 0, 0); // hidden
    btnStealMojo.setBounds(bounds.getWidth() - 150, 10, 140, 35); // Consumer mode button
    
    // ProcessControl components handle their own positioning
    
    // Force repaint to ensure visual changes appear
    this->repaint();
    
    // ProcessControl components are positioned in resized() method
    if (ledMeter1) ledMeter1->repaint();
    if (ledMeter2) ledMeter2->repaint();
    if (waveform) waveform->repaint();
    if (transport) transport->repaint();
}

// forceNeptuneKnobPositioning method removed - using ProcessControl components instead

void UIDemoComponent::timerCallback()
{
    // DISABLE continuous positioning to see if timer is causing interference
    // forceNeptuneKnobPositioning();
    
    // Drive LED meters with demo levels
    if (ledMeter1)
    {
        ledMeter1->setPeakNorm(level);
        ledMeter1->setRmsNorm(level * 0.82f);
    }
    if (ledMeter2)
    {
        ledMeter2->setPeakNorm(level2);
        ledMeter2->setRmsNorm(level2 * 0.82f);
    }
    
    // Animate the Neptune Mix knob to show all frames
    if (mixKnob)
    {
        float time = juce::Time::getMillisecondCounterHiRes() * 0.0005f;
        mixValue = 0.5f + 0.4f * std::sin(time);
        mixKnob->setValue(mixValue);
    }
    
    // Animate Neptune VU meters with original and processed signals
    float time = juce::Time::getMillisecondCounterHiRes() * 0.001f;
    originalLevel = 0.3f + 0.25f * std::sin(time * 1.5f); // Original signal (lower level)
    processedLevel = 0.6f + 0.3f * std::sin(time * 2.0f + 1.0f); // Processed signal (higher level, different phase)
    
    if (originalMeter)
        originalMeter->setLevel(originalLevel);
    
    if (processedMeter)
        processedMeter->setLevel(processedLevel);
    
    repaint();
}

void UIDemoComponent::drawMeter(juce::Graphics& g, juce::Rectangle<int> area, float value, juce::Colour col, MeterMode mode)
{
    auto r = area.toFloat();
    // If VU mode with frame provided, draw it as background; else draw beveled card
    const auto& sk = SkinManager::instance().skin();
    if (mode == MeterMode::VU && (! sk.meterVUFrames.empty() || sk.meterVUFrame.isValid()))
    {
        if (! sk.meterVUFrames.empty())
        {
            auto n = (int) sk.meterVUFrames.size();
            int idx = juce::jlimit(0, n-1, (int) juce::roundToInt(value * (float)(n - 1)));
            const auto& frame = sk.meterVUFrames[(size_t) idx];
            g.drawImageWithin(frame, area.getX(), area.getY(), area.getWidth(), area.getHeight(), juce::RectanglePlacement::stretchToFit, false);
        }
        else
        {
            g.drawImageWithin(sk.meterVUFrame, area.getX(), area.getY(), area.getWidth(), area.getHeight(), juce::RectanglePlacement::stretchToFit, false);
        }
    }
    else
    {
        juce::DropShadow (juce::Colours::black.withAlpha(0.45f), 10, {0,5}).drawForRectangle(g, area);
        auto top = ui::theme().card.brighter(0.10f);
        auto bot = ui::theme().card.darker(0.15f);
        g.setGradientFill(juce::ColourGradient(top, r.getCentreX(), r.getY(), bot, r.getCentreX(), r.getBottom(), false));
        g.fillRoundedRectangle(r, ui::theme().r_md);
        g.setColour(juce::Colours::white.withAlpha(0.20f));
        g.drawRoundedRectangle(r, ui::theme().r_md, 2.0f);
    }

    // bar
    auto inner = r.reduced(12);
    auto bar = inner.withHeight(20);
    if (mode == MeterMode::LED && sk.meterLEDStrip.isValid())
    {
        // Draw LED strip cropped by value
        int w = (int) juce::roundToInt(bar.getWidth() * juce::jlimit(0.0f, 1.0f, value));
        if (w > 0) {
            juce::Rectangle<int> dest((int)bar.getX(), (int)bar.getY(), w, (int)bar.getHeight());
            auto src = sk.meterLEDStrip.getBounds().withWidth(juce::jmax(1, (int)(sk.meterLEDStrip.getWidth() * (float)w / (float)bar.getWidth())));
            g.drawImage(sk.meterLEDStrip, dest.toFloat(), juce::RectanglePlacement::stretchToFit);
        }
    }
    else
    {
        g.setColour(ui::theme().panel.darker(0.25f));
        g.fillRoundedRectangle(bar, 10.0f);
        auto fill = bar.withWidth(bar.getWidth() * juce::jlimit(0.0f, 1.0f, value));
        juce::Colour m1 = col.darker(0.10f);
        juce::Colour m2 = col;
        juce::Colour m3 = col.brighter(0.25f);
        // Multi-stop horizontal gradient
        juce::ColourGradient metGrad(m1, fill.getX(), fill.getY(), m3, fill.getRight(), fill.getBottom(), false);
        metGrad.addColour(0.5, m2);
        g.setGradientFill(metGrad);
        g.fillRoundedRectangle(fill, 10.0f);
    }
    // Gloss streak
    g.setColour(juce::Colours::white.withAlpha(0.10f));
    g.fillRoundedRectangle(bar.withHeight(8).translated(0, -4), 6.0f);
    // Label
    g.setColour(juce::Colours::white.withAlpha(0.8f));
    g.setFont(13.0f);
    g.drawFittedText("Meter", area.withHeight(18).translated(0, -24), juce::Justification::centred, 1);
}

/*static*/ void UIDemoComponent::proguiLog(const juce::String& s)
{
    juce::File("/tmp/progui_demo.log").appendText(s + "\n");
}

void UIDemoComponent::mouseDown(const juce::MouseEvent& event)
{
    // Click detection for "OPEN CONTROLS" buttons in our Neptune knob components
    auto clickPos = event.getPosition();
    proguiLog("[DEBUG] Mouse click at: " + juce::String(clickPos.x) + ", " + juce::String(clickPos.y));
    
    // Check if click is within any of our placeholder components' button areas
    for (int i = 0; i < 8; ++i)
    {
        if (placeholderComponents[i])
        {
            auto componentBounds = placeholderComponents[i]->getBounds();
            
            // Button area is at bottom right of each component (110px x 25px)
            auto buttonArea = juce::Rectangle<int>(
                componentBounds.getRight() - 110 - 8,  // 110px wide + 8px margin
                componentBounds.getBottom() - 25 - 8,  // 25px tall + 8px margin
                110, 25
            );
            
            proguiLog("[DEBUG] Component " + juce::String(i) + " button bounds: " + 
                     juce::String(buttonArea.getX()) + " " + juce::String(buttonArea.getY()) + " " + 
                     juce::String(buttonArea.getWidth()) + " " + juce::String(buttonArea.getHeight()));
                     
            if (buttonArea.contains(clickPos))
            {
                juce::String processNames[] = {"ISP", "SRC", "JITTER", "ALIGN", "TRANSIENT", "DEESSER", "MLAR", "TRANSFORMER"};
                proguiLog("[UI] Opening " + processNames[i] + " subwindow!");
                openProcessSubwindow(i);
                return;
            }
        }
    }
}

void UIDemoComponent::openProcessSubwindow(int processIndex)
{
    juce::String processNames[] = {"ISP", "SRC", "JITTER", "ALIGN", "TRANSIENT", "DEESSER", "MLAR", "TRANSFORMER"};
    
    if (processIndex < 0 || processIndex >= 8)
        return;
        
    juce::String processName = processNames[processIndex];
    
    // For now, create a simple test control window to demonstrate the 3-layer system
    // This will show: Main UI -> Control Window (with knobs + INFO button) -> Info Window
    
    // Clean knob component without depression for control windows
    class CleanKnob : public juce::Component
    {
    public:
        CleanKnob()
        {
            addAndMakeVisible(slider);
            slider.setRange(0.0, 1.0, 0.001);
            slider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
            slider.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
            slider.onValueChange = [this]{ 
                repaint(); 
                if (onValueChanged) 
                    onValueChanged(getValue()); 
            };
            
            // Make slider invisible - we draw our own graphics
            slider.setColour(juce::Slider::rotarySliderFillColourId, juce::Colours::transparentBlack);
            slider.setColour(juce::Slider::rotarySliderOutlineColourId, juce::Colours::transparentBlack);
            slider.setColour(juce::Slider::thumbColourId, juce::Colours::transparentBlack);
            slider.setColour(juce::Slider::trackColourId, juce::Colours::transparentBlack);
            slider.setColour(juce::Slider::backgroundColourId, juce::Colours::transparentBlack);
        }
        
        void setValue(float v) { slider.setValue(juce::jlimit(0.0, 1.0, (double)v)); }
        float getValue() const { return (float)slider.getValue(); }
        
        std::function<void(float)> onValueChanged;
        
        void resized() override
        {
            slider.setBounds(getLocalBounds());
        }
        
        void paint(juce::Graphics& g) override
        {
            auto r = getLocalBounds().toFloat();
            const auto& skin = SkinManager::instance().skin();
            float t = (float)slider.getValue();
            auto c = r.getCentre();
            
            // Draw clean Neptune knob WITHOUT depression
            if (!skin.knobFrames.empty())
            {
                int n = (int)skin.knobFrames.size();
                int idx = juce::jlimit(0, n-1, (int)std::round(t * (n-1)));
                auto img = skin.knobFrames[(size_t)idx];
                float scale = juce::jmin(r.getWidth() / img.getWidth(), r.getHeight() / img.getHeight()) * 0.9f;
                auto dest = juce::Rectangle<float>(img.getWidth() * scale, img.getHeight() * scale).withCentre(c);
                g.drawImage(img, dest);
            }
            else
            {
                // Vector fallback
                g.setColour(juce::Colour(0xFF444444));
                g.fillEllipse(r.reduced(4));
                g.setColour(juce::Colour(0xFF666666));
                g.drawEllipse(r.reduced(4), 2.0f);
                
                // Draw pointer
                auto angle = juce::MathConstants<float>::pi * 1.2f + t * juce::MathConstants<float>::pi * 1.6f;
                auto radius = r.getWidth() * 0.3f;
                auto startX = c.x;
                auto startY = c.y;
                auto endX = c.x + std::cos(angle) * radius;
                auto endY = c.y + std::sin(angle) * radius;
                
                g.setColour(juce::Colours::white);
                g.drawLine(startX, startY, endX, endY, 3.0f);
            }
        }
        
    private:
        juce::Slider slider;
    };
    
// Custom component for ISP info with graphics
class ISPInfoComponent : public juce::Component
{
public:
    ISPInfoComponent(const juce::String& title = "ISP") : windowTitle(title)
    {
        setSize(950, 750);
    }
    
    void paint(juce::Graphics& g) override
    {
        // Background
        g.fillAll(juce::Colour(0xFF1a1a2e));
        
        // Header
        g.setColour(juce::Colours::white);
        g.setFont(20.0f);
        juce::String headerText = windowTitle.toUpperCase();
        if (windowTitle.contains("ISP")) headerText = "ISP - INTERSAMPLE PROCESSING";
        else if (windowTitle.contains("SRC")) headerText = "SRC - SAMPLE RATE CONVERSION";
        else if (windowTitle.contains("JITTER")) headerText = "JITTER ANALYSIS & OPTIMIZATION";
        else if (windowTitle.contains("ALIGN")) headerText = "PHASE & TIME ALIGNMENT";
        g.drawText(headerText, 20, 20, getWidth() - 40, 30, juce::Justification::centred);
        
        // Text area (left side) - Use getProcessInfo()
        g.setColour(juce::Colour(0xFF00ffff));
        g.setFont(10.0f);
        juce::String infoText = getProcessInfo();
        g.drawFittedText(infoText, 20, 60, 420, 600, juce::Justification::topLeft, 100);
        
        // Graphics area (right side)
        drawGraphics(g, 460, 60, 420, 600);
    }
    
private:
    juce::String windowTitle;
    
    juce::String getProcessInfo()
    {
        if (windowTitle.contains("JITTER")) {
            return "JITTER ANALYSIS & OPTIMIZATION\n\nMeasures and displays digital audio clock timing irregularities.\n\nCONTROLS:\n- RMS Jitter Meter: Average clock timing deviation (target: <10ps)\n- Peak Jitter Display: Worst-case timing errors  \n- Spectral Analysis: Jitter across frequency spectrum\n- Dither Type Switch: TPDF / Triangular / Gaussian / None\n- Bit Depth Switch: 16 / 20 / 24 / 32 bit\n\nRESULT:\nLower jitter = cleaner audio with better imaging and transients.\nValues >100ps may cause audible degradation.";
        } else if (windowTitle.contains("SRC")) {
            return "SAMPLE RATE CONVERSION\n\nConverts audio between different sample rates while preserving quality.\n\nCONTROLS:\n- Source Rate Switch: 44.1 / 48 / 88.2 / 96 / 176.4 / 192 kHz\n- Target Rate Switch: 44.1 / 48 / 88.2 / 96 / 176.4 / 192 kHz\n- Quality Mode: Draft / Standard / High / Ultra\n- Passband Ripple: Filter transition sharpness\n- Stopband Attenuation: Aliasing rejection (60-140dB)\n\nRESULT:\nHigher quality settings = more transparent conversion.\nUse Ultra mode for critical mastering work.";
        } else if (windowTitle.contains("ALIGN")) {
            return "PHASE & TIME ALIGNMENT\n\nSynchronizes multiple audio sources to eliminate phase cancellation.\n\nCONTROLS:\n- Time Delay: Sample-accurate compensation (+/-10,000 samples)\n- Phase Rotation Switch: 0 / 90 / 180 / 270 degrees\n- Polarity Switch: Normal / Inverted\n- Correlation Meter: Phase relationship (-1.0 to +1.0)\n- Crossover Frequency: Band split point (20Hz-20kHz)\n\nRESULT:\nProper alignment restores fullness, punch, and clarity.\nCorrelation meter: >+0.7 = good, <0 = phase problems.";
        }
        // Default ISP info
        return "INTERSAMPLE PROCESSING\n\nEliminates intersample peaks that can cause digital distortion.\n\nCONTROLS:\n• OS Factor - Oversampling rate (2x/4x/8x)\n• Filter Type - Linear/Minimum/Polyphase\n• Passband Rolloff - Filter transition sharpness\n• Stopband Rejection - Aliasing suppression (60-120dB)\n• True-Peak Ceiling - Maximum output level\n• Lookahead Time - Peak detection window (0-10ms)\n\nRESULT:\nPrevents intersample overs in final masters.\nEssential for loud mastering and broadcast.";
    }

    
    void drawGraphics(juce::Graphics& g, int x, int y, int width, int height)
    {
        // Graphics section header
        g.setColour(juce::Colour(0xFF87ceeb));
        g.setFont(16.0f);
        g.drawText("VISUAL GRAPHICS", x, y, width, 30, juce::Justification::centred);
        
        if (windowTitle.contains("JITTER")) {
            drawJitterGraphics(g, x, y + 40, width, height - 40);
        } else if (windowTitle.contains("SRC")) {
            drawSRCGraphics(g, x, y + 40, width, height - 40);
        } else if (windowTitle.contains("ALIGN")) {
            drawAlignGraphics(g, x, y + 40, width, height - 40);
        } else {
            // Default ISP graphics
            int graphY = y + 40;
            int graphHeight = (height - 40) / 3;
            drawFrequencyResponse(g, x, graphY, width, graphHeight - 20);
            drawWaveformWithPeaks(g, x, graphY + graphHeight, width, graphHeight - 20);
            drawOversamplingComparison(g, x, graphY + 2 * graphHeight, width, graphHeight - 20);
        }
    }
    
    void drawFrequencyResponse(juce::Graphics& g, int x, int y, int width, int height)
    {
        // Title
        g.setColour(juce::Colours::white);
        g.setFont(12.0f);
        g.drawText("Passband Rolloff & Stopband Attenuation", x, y, width, 20, juce::Justification::centred);
        
        // Graph background
        g.setColour(juce::Colour(0xFF0a0a1a));
        g.fillRect(x + 10, y + 25, width - 20, height - 30);
        g.setColour(juce::Colour(0xFF333333));
        g.drawRect(x + 10, y + 25, width - 20, height - 30);
        
        // Draw frequency response curve
        juce::Path responseCurve;
        float startX = x + 15;
        float endX = x + width - 15;
        float midY = y + 25 + (height - 30) / 2;
        
        responseCurve.startNewSubPath(startX, midY);
        
        // Passband (flat)
        responseCurve.lineTo(startX + (endX - startX) * 0.6f, midY);
        
        // Rolloff region
        for (float i = 0.6f; i <= 1.0f; i += 0.01f)
        {
            float xPos = startX + (endX - startX) * i;
            float rolloff = std::pow((i - 0.6f) / 0.4f, 2.0f); // Quadratic rolloff
            float yPos = midY + rolloff * (height - 50);
            responseCurve.lineTo(xPos, yPos);
        }
        
        g.setColour(juce::Colour(0xFF00d4aa));
        g.strokePath(responseCurve, juce::PathStrokeType(2.0f));
        
        // Labels
        g.setColour(juce::Colour(0xFF888888));
        g.setFont(10.0f);
        g.drawText("0dB", x + 15, midY - 10, 30, 20, juce::Justification::left);
        g.drawText("Passband", x + 15, y + height - 20, 60, 15, juce::Justification::left);
        g.drawText("Stopband", x + width - 70, y + height - 20, 60, 15, juce::Justification::right);
    }
    
    void drawWaveformWithPeaks(juce::Graphics& g, int x, int y, int width, int height)
    {
        // Title
        g.setColour(juce::Colours::white);
        g.setFont(12.0f);
        g.drawText("Intersample Interpolation & True-Peak Detection", x, y, width, 20, juce::Justification::centred);
        
        // Graph background
        g.setColour(juce::Colour(0xFF0a0a1a));
        g.fillRect(x + 10, y + 25, width - 20, height - 30);
        g.setColour(juce::Colour(0xFF333333));
        g.drawRect(x + 10, y + 25, width - 20, height - 30);
        
        float startX = x + 15;
        float endX = x + width - 15;
        float midY = y + 25 + (height - 30) / 2;
        
        // Draw original samples (discrete points)
        g.setColour(juce::Colour(0xFF666666));
        for (int i = 0; i < 12; i++)
        {
            float xPos = startX + (endX - startX) * (i / 11.0f);
            float wave = std::sin(i * 2.0f * juce::MathConstants<float>::pi / 11.0f * 6.0f);
            float yPos = midY + wave * (height - 60) * 0.25f;
            g.fillEllipse(xPos - 2, yPos - 2, 4, 4);
            
            // Connect with straight lines (digital reconstruction)
            if (i > 0)
            {
                float prevXPos = startX + (endX - startX) * ((i-1) / 11.0f);
                float prevWave = std::sin((i-1) * 2.0f * juce::MathConstants<float>::pi / 11.0f * 6.0f);
                float prevYPos = midY + prevWave * (height - 60) * 0.25f;
                g.drawLine(prevXPos, prevYPos, xPos, yPos, 1.0f);
            }
        }
        
        // Draw interpolated waveform (smooth curve)
        juce::Path interpolatedWave;
        interpolatedWave.startNewSubPath(startX, midY);
        
        for (float i = 0; i <= 1.0f; i += 0.002f)
        {
            float xPos = startX + (endX - startX) * i;
            // High-resolution interpolated waveform
            float wave = std::sin(i * 12.0f * juce::MathConstants<float>::pi);
            float yPos = midY + wave * (height - 60) * 0.3f;
            interpolatedWave.lineTo(xPos, yPos);
        }
        
        g.setColour(juce::Colour(0xFF00d4aa));
        g.strokePath(interpolatedWave, juce::PathStrokeType(2.0f));
        
        // Draw true intersample peaks (red dots)
        g.setColour(juce::Colour(0xFFff4444));
        for (int i = 0; i < 6; i++)
        {
            float xPos = startX + (endX - startX) * (0.15f + i * 0.14f);
            float peakY = midY - (height - 60) * 0.38f * (0.9f + 0.1f * std::sin(i));
            g.fillEllipse(xPos - 4, peakY - 4, 8, 8);
            
            // Draw peak indicator lines
            g.drawLine(xPos, peakY + 4, xPos, midY + (height - 60) * 0.4f, 1.0f);
        }
        
        // True-peak ceiling line
        g.setColour(juce::Colour(0xFFf7931e));
        float ceilingY = midY - (height - 60) * 0.35f;
        g.drawLine(startX, ceilingY, endX, ceilingY, 2.0f);
        
        // Lookahead processing window (subtle overlay)
        g.setColour(juce::Colour(0x3087ceeb)); // Semi-transparent blue
        float lookaheadStart = startX + (endX - startX) * 0.7f;
        g.fillRect((int)lookaheadStart, y + 30, (int)((endX - startX) * 0.25f), height - 40);
        g.setColour(juce::Colour(0xFF4a90e2));
        g.drawRect((int)lookaheadStart, y + 30, (int)((endX - startX) * 0.25f), height - 40, 2);
        
        // Labels
        g.setColour(juce::Colour(0xFF888888));
        g.setFont(9.0f);
        g.drawText("Ceiling", x + width - 50, ceilingY - 15, 40, 12, juce::Justification::right);
        g.drawText("Samples", x + 15, y + height - 35, 60, 12, juce::Justification::left);
        g.setColour(juce::Colour(0xFF00d4aa));
        g.drawText("Interpolated", x + 15, y + height - 23, 80, 12, juce::Justification::left);
        g.setColour(juce::Colour(0xFFff4444));
        g.drawText("True Peaks", x + 100, y + height - 23, 80, 12, juce::Justification::left);
        g.setColour(juce::Colour(0xFF87ceeb));
        g.drawText("Lookahead", lookaheadStart + 5, y + 35, 80, 12, juce::Justification::left);
    }
    
    void drawOversamplingComparison(juce::Graphics& g, int x, int y, int width, int height)
    {
        // Title
        g.setColour(juce::Colours::white);
        g.setFont(12.0f);
        g.drawText("Oversampling Factor Comparison", x, y, width, 20, juce::Justification::centred);
        
        // Graph background
        g.setColour(juce::Colour(0xFF0a0a1a));
        g.fillRect(x + 10, y + 25, width - 20, height - 30);
        g.setColour(juce::Colour(0xFF333333));
        g.drawRect(x + 10, y + 25, width - 20, height - 30);
        
        // Draw three resolution examples
        int sectionWidth = (width - 40) / 3;
        
        // 1x (Original)
        drawResolutionExample(g, x + 15, y + 30, sectionWidth - 10, height - 40, 1, "1x Original");
        
        // 4x Oversampling
        drawResolutionExample(g, x + 15 + sectionWidth, y + 30, sectionWidth - 10, height - 40, 4, "4x Oversample");
        
        // 8x Oversampling
        drawResolutionExample(g, x + 15 + 2 * sectionWidth, y + 30, sectionWidth - 10, height - 40, 8, "8x Oversample");
    }
    
    void drawResolutionExample(juce::Graphics& g, int x, int y, int width, int height, int factor, const juce::String& label)
    {
        // Background
        g.setColour(juce::Colour(0xFF1a1a2e));
        g.fillRect(x, y, width, height);
        g.setColour(juce::Colour(0xFF444444));
        g.drawRect(x, y, width, height);
        
        // Draw sample points
        int numSamples = 8 * factor;
        float sampleWidth = (float)(width - 20) / numSamples;
        
        juce::Colour sampleColor = (factor == 1) ? juce::Colour(0xFF666666) : 
                                  (factor == 4) ? juce::Colour(0xFF00d4aa) : 
                                                 juce::Colour(0xFF87ceeb);
        
        g.setColour(sampleColor);
        for (int i = 0; i < numSamples; i++)
        {
            float xPos = x + 10 + i * sampleWidth;
            float yPos = y + height/2 + std::sin(i * 0.5f) * (height - 40) * 0.3f;
            g.fillEllipse(xPos - 1, yPos - 1, 2, 2);
            
            if (i < numSamples - 1)
            {
                float nextYPos = y + height/2 + std::sin((i+1) * 0.5f) * (height - 40) * 0.3f;
                g.drawLine(xPos, yPos, xPos + sampleWidth, nextYPos, 1.0f);
            }
        }
        
        // Label
        g.setColour(juce::Colours::white);
        g.setFont(10.0f);
        g.drawText(label, x, y + height - 20, width, 15, juce::Justification::centred);
    }
    
    void drawJitterGraphics(juce::Graphics& g, int x, int y, int width, int height)
    {
        // Title
        g.setColour(juce::Colour(0xFF00d4aa));
        g.setFont(14.0f);
        g.drawText("JITTER MEASUREMENT", x, y, width, 25, juce::Justification::centred);
        
        // Draw RMS Jitter meter
        int meterY = y + 35;
        int meterHeight = (height - 50) / 2;
        
        g.setColour(juce::Colour(0xFF0a0a1a));
        g.fillRect(x + 10, meterY, width - 20, meterHeight);
        g.setColour(juce::Colour(0xFF333333));
        g.drawRect(x + 10, meterY, width - 20, meterHeight);
        
        // Draw jitter level bars
        g.setColour(juce::Colours::white);
        g.setFont(10.0f);
        g.drawText("RMS Jitter (picoseconds)", x + 20, meterY + 10, width - 40, 15, juce::Justification::left);
        
        // Good range (green)
        g.setColour(juce::Colour(0xFF00ff00));
        g.fillRect(x + 20, meterY + 35, (width - 40) / 3, 20);
        g.setColour(juce::Colours::white);
        g.drawText("<10ps", x + 20, meterY + 60, (width - 40) / 3, 15, juce::Justification::centred);
        
        // Fair range (yellow)
        g.setColour(juce::Colour(0xFFffff00));
        g.fillRect(x + 20 + (width - 40) / 3, meterY + 35, (width - 40) / 3, 20);
        g.setColour(juce::Colours::white);
        g.drawText("10-100ps", x + 20 + (width - 40) / 3, meterY + 60, (width - 40) / 3, 15, juce::Justification::centred);
        
        // Poor range (red)
        g.setColour(juce::Colour(0xFFff0000));
        g.fillRect(x + 20 + 2 * (width - 40) / 3, meterY + 35, (width - 40) / 3, 20);
        g.setColour(juce::Colours::white);
        g.drawText(">100ps", x + 20 + 2 * (width - 40) / 3, meterY + 60, (width - 40) / 3, 15, juce::Justification::centred);
        
        // Draw spectral analysis graph
        int graphY = meterY + meterHeight + 20;
        g.setColour(juce::Colour(0xFF0a0a1a));
        g.fillRect(x + 10, graphY, width - 20, height - (graphY - y) - 10);
        g.setColour(juce::Colour(0xFF333333));
        g.drawRect(x + 10, graphY, width - 20, height - (graphY - y) - 10);
        
        g.setColour(juce::Colours::white);
        g.setFont(10.0f);
        g.drawText("Spectral Distribution", x + 20, graphY + 5, width - 40, 15, juce::Justification::left);
        
        // Draw simplified spectrum
        juce::Path spectrum;
        float startX = x + 20;
        float endX = x + width - 20;
        float midY = graphY + (height - (graphY - y) - 10) / 2;
        spectrum.startNewSubPath(startX, midY);
        for (float i = 0; i <= 1.0f; i += 0.02f)
        {
            float xPos = startX + (endX - startX) * i;
            float noise = std::sin(i * 20.0f) * 15.0f;
            spectrum.lineTo(xPos, midY + noise);
        }
        g.setColour(juce::Colour(0xFF00d4aa));
        g.strokePath(spectrum, juce::PathStrokeType(2.0f));
    }
    
    void drawSRCGraphics(juce::Graphics& g, int x, int y, int width, int height)
    {
        // Title
        g.setColour(juce::Colour(0xFF87ceeb));
        g.setFont(14.0f);
        g.drawText("SAMPLE RATE CONVERSION", x, y, width, 25, juce::Justification::centred);
        
        // Draw input/output sample rate visualization
        int graphY = y + 35;
        g.setColour(juce::Colour(0xFF0a0a1a));
        g.fillRect(x + 10, graphY, width - 20, height - 50);
        g.setColour(juce::Colour(0xFF333333));
        g.drawRect(x + 10, graphY, width - 20, height - 50);
        
        // Input samples (lower rate)
        g.setColour(juce::Colours::white);
        g.setFont(10.0f);
        g.drawText("Input: 44.1 kHz", x + 20, graphY + 10, width / 2 - 30, 15, juce::Justification::left);
        
        int inputY = graphY + 35;
        g.setColour(juce::Colour(0xFF666666));
        for (int i = 0; i < 8; i++)
        {
            float xPos = x + 30 + i * ((width / 2 - 40) / 7.0f);
            g.fillEllipse(xPos - 2, inputY - 2, 4, 4);
        }
        
        // Arrow
        g.setColour(juce::Colour(0xFF00d4aa));
        g.fillRect(x + width / 2 - 20, graphY + 30, 40, 3);
        g.fillRect(x + width / 2 + 15, graphY + 25, 3, 13);
        
        // Output samples (higher rate)
        g.setColour(juce::Colours::white);
        g.drawText("Output: 48 kHz", x + width / 2 + 10, graphY + 10, width / 2 - 30, 15, juce::Justification::left);
        
        int outputY = graphY + 35;
        g.setColour(juce::Colour(0xFF87ceeb));
        for (int i = 0; i < 12; i++)
        {
            float xPos = x + width / 2 + 20 + i * ((width / 2 - 40) / 11.0f);
            g.fillEllipse(xPos - 2, outputY - 2, 4, 4);
        }
        
        // Anti-aliasing filter response
        int filterY = graphY + 80;
        g.setColour(juce::Colours::white);
        g.setFont(10.0f);
        g.drawText("Anti-Aliasing Filter Response", x + 20, filterY, width - 40, 15, juce::Justification::centred);
        
        juce::Path filterCurve;
        float startX = x + 20;
        float endX = x + width - 20;
        float midY = filterY + 50;
        filterCurve.startNewSubPath(startX, midY);
        filterCurve.lineTo(startX + (endX - startX) * 0.7f, midY);
        for (float i = 0.7f; i <= 1.0f; i += 0.01f)
        {
            float xPos = startX + (endX - startX) * i;
            float rolloff = std::pow((i - 0.7f) / 0.3f, 3.0f);
            filterCurve.lineTo(xPos, midY + rolloff * 60);
        }
        g.setColour(juce::Colour(0xFF87ceeb));
        g.strokePath(filterCurve, juce::PathStrokeType(2.0f));
    }
    
    void drawAlignGraphics(juce::Graphics& g, int x, int y, int width, int height)
    {
        // Title
        g.setColour(juce::Colour(0xFFf7931e));
        g.setFont(14.0f);
        g.drawText("PHASE & TIME ALIGNMENT", x, y, width, 25, juce::Justification::centred);
        
        int graphY = y + 35;
        g.setColour(juce::Colour(0xFF0a0a1a));
        g.fillRect(x + 10, graphY, width - 20, height - 50);
        g.setColour(juce::Colour(0xFF333333));
        g.drawRect(x + 10, graphY, width - 20, height - 50);
        
        // Correlation meter
        g.setColour(juce::Colours::white);
        g.setFont(10.0f);
        g.drawText("Phase Correlation Meter", x + 20, graphY + 10, width - 40, 15, juce::Justification::centred);
        
        int meterY = graphY + 35;
        int meterWidth = width - 60;
        
        // Draw correlation scale
        g.setColour(juce::Colour(0xFF333333));
        g.fillRect(x + 30, meterY, meterWidth, 20);
        
        // Color gradient: red (-1) -> yellow (0) -> green (+1)
        for (int i = 0; i < meterWidth; i++)
        {
            float pos = (float)i / meterWidth;
            float correlation = -1.0f + pos * 2.0f;
            juce::Colour color;
            if (correlation < 0)
                color = juce::Colour(0xFFff0000).interpolatedWith(juce::Colour(0xFFffff00), (correlation + 1.0f));
            else
                color = juce::Colour(0xFFffff00).interpolatedWith(juce::Colour(0xFF00ff00), correlation);
            
            g.setColour(color);
            g.fillRect(x + 30 + i, meterY, 1, 20);
        }
        
        // Draw scale labels
        g.setColour(juce::Colours::white);
        g.setFont(9.0f);
        g.drawText("-1.0", x + 20, meterY + 25, 30, 12, juce::Justification::left);
        g.drawText("0", x + 30 + meterWidth / 2 - 10, meterY + 25, 20, 12, juce::Justification::centred);
        g.drawText("+1.0", x + width - 50, meterY + 25, 30, 12, juce::Justification::right);
        
        // Draw waveform alignment example
        int waveY = meterY + 60;
        g.setColour(juce::Colours::white);
        g.setFont(10.0f);
        g.drawText("Waveform Alignment", x + 20, waveY, width - 40, 15, juce::Justification::left);
        
        float startX = x + 30;
        float endX = x + width - 30;
        float midY = waveY + 50;
        
        // Draw two waveforms - one aligned, one offset
        juce::Path wave1;
        juce::Path wave2;
        wave1.startNewSubPath(startX, midY);
        wave2.startNewSubPath(startX + 20, midY); // Offset for alignment visualization
        
        for (float i = 0; i <= 1.0f; i += 0.01f)
        {
            float xPos = startX + (endX - startX) * i;
            float sine = std::sin(i * 8.0f * juce::MathConstants<float>::pi);
            float yOffset = sine * 30.0f;
            wave1.lineTo(xPos, midY + yOffset);
            wave2.lineTo(xPos + 20, midY + yOffset);
        }
        
        g.setColour(juce::Colour(0xFF00d4aa));
        g.strokePath(wave1, juce::PathStrokeType(2.0f));
        g.setColour(juce::Colour(0xFFf7931e).withAlpha(0.6f));
        g.strokePath(wave2, juce::PathStrokeType(2.0f));
        
        // Show time offset
        g.setColour(juce::Colours::white);
        g.setFont(9.0f);
        g.drawText("Time offset →", startX + 5, midY + 40, 100, 12, juce::Justification::left);
    }
};
    class ISPControlWindow : public juce::Component
    {
    public:
        ISPControlWindow(const juce::String& name, const juce::String& desc)
            : processName(name), processDescription(desc)
        {
            setSize(800, 600);
            
            // Create dropdowns
            try {
                // OS Factor dropdown
                osFactorCombo = std::make_unique<juce::ComboBox>();
                if (osFactorCombo) {
                    osFactorCombo->addItem("2x Oversampling", 1);
                    osFactorCombo->addItem("4x Oversampling", 2);
                    osFactorCombo->addItem("8x Oversampling", 3);
                    osFactorCombo->setSelectedId(2); // Default to 4x
                    osFactorCombo->setColour(juce::ComboBox::backgroundColourId, juce::Colour(0xFF2a2a3e));
                    osFactorCombo->setColour(juce::ComboBox::textColourId, juce::Colours::white);
                    addAndMakeVisible(*osFactorCombo);
                }
                
                // Filter Type dropdown
                filterTypeCombo = std::make_unique<juce::ComboBox>();
                if (filterTypeCombo) {
                    filterTypeCombo->addItem("Linear Phase", 1);
                    filterTypeCombo->addItem("Minimum Phase", 2);
                    filterTypeCombo->addItem("Polyphase", 3);
                    filterTypeCombo->setSelectedId(3); // Default to Polyphase
                    filterTypeCombo->setColour(juce::ComboBox::backgroundColourId, juce::Colour(0xFF2a2a3e));
                    filterTypeCombo->setColour(juce::ComboBox::textColourId, juce::Colours::white);
                    addAndMakeVisible(*filterTypeCombo);
                }
                
                // Preset dropdown with instrument and bus presets
                presetCombo = std::make_unique<juce::ComboBox>();
                if (presetCombo) {
                    // Instrument presets
                    presetCombo->addSectionHeading("INSTRUMENTS");
                    presetCombo->addItem("Vocals - Lead", 1);
                    presetCombo->addItem("Vocals - Backing", 2);
                    presetCombo->addItem("Guitar - Electric", 3);
                    presetCombo->addItem("Guitar - Acoustic", 4);
                    presetCombo->addItem("Bass Guitar", 5);
                    presetCombo->addItem("Piano - Acoustic", 6);
                    presetCombo->addItem("Piano - Electric", 7);
                    presetCombo->addItem("Drums - Kick", 8);
                    presetCombo->addItem("Drums - Snare", 9);
                    presetCombo->addItem("Drums - Overheads", 10);
                    presetCombo->addItem("Strings - Violin", 11);
                    presetCombo->addItem("Strings - Cello", 12);
                    presetCombo->addItem("Brass - Trumpet", 13);
                    presetCombo->addItem("Brass - Trombone", 14);
                    presetCombo->addItem("Woodwinds - Flute", 15);
                    presetCombo->addItem("Woodwinds - Saxophone", 16);
                    presetCombo->addItem("Synth - Lead", 17);
                    presetCombo->addItem("Synth - Pad", 18);
                    
                    // Bus presets
                    presetCombo->addSeparator();
                    presetCombo->addSectionHeading("BUSSES");
                    presetCombo->addItem("Drum Bus", 19);
                    presetCombo->addItem("Vocal Bus", 20);
                    presetCombo->addItem("Guitar Bus", 21);
                    presetCombo->addItem("String Bus", 22);
                    presetCombo->addItem("Brass Bus", 23);
                    presetCombo->addItem("Mix Bus", 24);
                    presetCombo->addItem("Master Bus", 25);
                    
                    presetCombo->setSelectedId(1); // Default to Vocals - Lead
                    // Professional blue styling for preset dropdown
                    presetCombo->setColour(juce::ComboBox::backgroundColourId, juce::Colour(0xFF1a2a3a)); // Dark blue background
                    presetCombo->setColour(juce::ComboBox::textColourId, juce::Colour(0xFF87ceeb)); // Soft blue text
                    presetCombo->setColour(juce::ComboBox::outlineColourId, juce::Colour(0xFF4a90e2)); // Professional blue outline
                    presetCombo->setColour(juce::ComboBox::arrowColourId, juce::Colour(0xFF87ceeb)); // Soft blue arrow
                    presetCombo->onChange = [this]() { loadPreset(); };
                    addAndMakeVisible(*presetCombo);
                }
                
                // Create 4 ISP-specific knobs with meter connections
                passbandKnob = std::make_unique<CleanKnob>();
                if (passbandKnob) {
                    passbandKnob->setValue(0.33f); // 1.0dB default (0-3dB range)
                    passbandKnob->onValueChanged = [this](float value) { 
                        passbandMeter = value; 
                        repaint(); 
                    };
                    addAndMakeVisible(*passbandKnob);
                }
                
                stopbandKnob = std::make_unique<CleanKnob>();
                if (stopbandKnob) {
                    stopbandKnob->setValue(0.67f); // 100dB default (60-120dB range)
                    stopbandKnob->onValueChanged = [this](float value) { 
                        stopbandMeter = value; 
                        repaint(); 
                    };
                    addAndMakeVisible(*stopbandKnob);
                }
                
                ceilingKnob = std::make_unique<CleanKnob>();
                if (ceilingKnob) {
                    ceilingKnob->setValue(0.83f); // -1dB default (-6 to 0dB range)
                    ceilingKnob->onValueChanged = [this](float value) { 
                        ceilingMeter = value; 
                        repaint(); 
                    };
                    addAndMakeVisible(*ceilingKnob);
                }
                
                lookaheadKnob = std::make_unique<CleanKnob>();
                if (lookaheadKnob) {
                    lookaheadKnob->setValue(0.2f); // 2ms default (0-10ms range)
                    lookaheadKnob->onValueChanged = [this](float value) { 
                        lookaheadMeter = value; 
                        repaint(); 
                    };
                    addAndMakeVisible(*lookaheadKnob);
                }
                
                // Initialize meter values
                passbandMeter = 0.33f;
                stopbandMeter = 0.67f;
                ceilingMeter = 0.83f;
                lookaheadMeter = 0.2f;
                
                // Create INFO button
                infoButton = std::make_unique<juce::TextButton>("INFO");
                if (infoButton) {
                    infoButton->setColour(juce::TextButton::buttonColourId, juce::Colour(0xFF4a90e2));
                    infoButton->setColour(juce::TextButton::textColourOffId, juce::Colours::white);
                    infoButton->onClick = [this]() { showInfoWindow(); };
                    addAndMakeVisible(*infoButton);
                }
            } catch (...) {
                // If component creation fails, we'll handle it gracefully in resized()
            }
        }
        
        void paint(juce::Graphics& g) override
        {
            g.fillAll(juce::Colour(0xFF1a1a2e));
            
            // Header
            g.setColour(juce::Colours::white);
            g.setFont(18.0f);
            g.drawText(processName, 20, 20, getWidth() - 40, 30, juce::Justification::centred);
            
            g.setColour(juce::Colour(0xFF00ffff));
            g.setFont(12.0f);
            g.drawText(processDescription, 20, 50, getWidth() - 40, 40, juce::Justification::centred, true);
            
            // Preset area background highlight
            g.setColour(juce::Colour(0xFF1a2a3a)); // Subtle dark blue background
            g.fillRoundedRectangle(getWidth()/2 - 90, 95, 180, 60, 8.0f);
            g.setColour(juce::Colour(0xFF4a90e2)); // Professional blue outline
            g.drawRoundedRectangle(getWidth()/2 - 90, 95, 180, 60, 8.0f, 2.0f);
            
            // Preset label (centered, professional styling)
            g.setColour(juce::Colour(0xFF87ceeb)); // Soft blue for readability
            g.setFont(13.0f); // Larger font
            g.drawText("PRESET", getWidth()/2 - 80, 100, 160, 20, juce::Justification::centred);
            
            // Other dropdown labels (moved down more)
            g.setColour(juce::Colours::white);
            g.setFont(11.0f);
            g.drawText("OS Factor", 200, 170, 120, 20, juce::Justification::centred);
            g.drawText("Filter Type", 350, 170, 120, 20, juce::Justification::centred);
            
            // Parameter labels (moved down more)
            g.drawText("Passband\nRolloff", 70, 240, 80, 40, juce::Justification::centred);
            g.drawText("Stopband\nAtten", 190, 240, 80, 40, juce::Justification::centred);
            g.drawText("TP Ceiling", 310, 240, 80, 40, juce::Justification::centred);
            g.drawText("Lookahead\nTime", 430, 240, 80, 40, juce::Justification::centred);
            
            // Draw vertical bar meters (moved down more)
            drawVerticalMeter(g, 110, 280, 20, 100, passbandMeter, juce::Colour(0xFF00d4aa), "Passband");
            drawVerticalMeter(g, 230, 280, 20, 100, stopbandMeter, juce::Colour(0xFFff6b35), "Stopband");
            drawVerticalMeter(g, 350, 280, 20, 100, ceilingMeter, juce::Colour(0xFFf7931e), "Ceiling");
            drawVerticalMeter(g, 470, 280, 20, 100, lookaheadMeter, juce::Colour(0xFF4a90e2), "Lookahead");
            
            // Value displays (moved down more)
            g.setColour(juce::Colour(0xFF00d4aa));
            g.setFont(10.0f);
            if (passbandKnob) {
                float value = passbandKnob->getValue() * 3.0f; // 0-3dB
                g.drawText(juce::String(value, 1) + "dB", 70, 390, 80, 15, juce::Justification::centred);
            }
            if (stopbandKnob) {
                float value = 60.0f + stopbandKnob->getValue() * 60.0f; // 60-120dB
                g.drawText(juce::String(value, 0) + "dB", 190, 390, 80, 15, juce::Justification::centred);
            }
            if (ceilingKnob) {
                float value = -6.0f + ceilingKnob->getValue() * 6.0f; // -6 to 0dB
                g.drawText(juce::String(value, 1) + "dB", 310, 390, 80, 15, juce::Justification::centred);
            }
            if (lookaheadKnob) {
                float value = lookaheadKnob->getValue() * 10.0f; // 0-10ms
                g.drawText(juce::String(value, 1) + "ms", 430, 390, 80, 15, juce::Justification::centred);
            }
        }
        
        void resized() override
        {
            auto area = getLocalBounds().reduced(20);
            area.removeFromTop(100); // Skip header
            
            // Preset dropdown (centered, prominent)
            if (presetCombo)
                presetCombo->setBounds(getWidth()/2 - 80, 120, 160, 30); // Centered, wider, taller
                
            // Other dropdowns (moved down more)
            if (osFactorCombo)
                osFactorCombo->setBounds(200, 190, 120, 25);
            if (filterTypeCombo)
                filterTypeCombo->setBounds(350, 190, 120, 25);
            
            // Bigger knobs positioned below meters (moved down more)
            if (passbandKnob)
                passbandKnob->setBounds(80, 410, 80, 80);
            if (stopbandKnob)
                stopbandKnob->setBounds(200, 410, 80, 80);
            if (ceilingKnob)
                ceilingKnob->setBounds(320, 410, 80, 80);
            if (lookaheadKnob)
                lookaheadKnob->setBounds(440, 410, 80, 80);
            
            // INFO button moved to top right
            if (infoButton)
                infoButton->setBounds(getWidth() - 120, 20, 100, 30);
        }
        
    private:
        void drawVerticalMeter(juce::Graphics& g, int x, int y, int width, int height, float level, juce::Colour color, const juce::String& label)
        {
            const int numSegments = 20; // High resolution with 20 segments
            const int segmentHeight = 4;
            const int segmentGap = 1;
            const int totalSegmentSpace = segmentHeight + segmentGap;
            
            // Draw meter background
            g.setColour(juce::Colour(0xFF1a1a1a));
            g.fillRect(x, y, width, height);
            g.setColour(juce::Colour(0xFF444444));
            g.drawRect(x, y, width, height, 1);
            
            // Calculate how many segments to light up
            int activeSegments = (int)(level * numSegments);
            
            // Draw segments from bottom to top
            for (int i = 0; i < numSegments; i++) {
                int segmentY = y + height - (i + 1) * totalSegmentSpace;
                
                if (i < activeSegments) {
                    // Active segment - use color with intensity based on position
                    float intensity = 1.0f;
                    if (i > numSegments * 0.7f) {
                        // Top 30% - brighter (peak area)
                        intensity = 1.2f;
                        g.setColour(color.brighter(0.4f));
                    } else if (i > numSegments * 0.4f) {
                        // Middle area - normal intensity
                        g.setColour(color);
                    } else {
                        // Bottom area - slightly dimmer
                        g.setColour(color.darker(0.2f));
                    }
                    
                    g.fillRect(x + 2, segmentY, width - 4, segmentHeight);
                    
                    // Add highlight on active segments
                    g.setColour(color.brighter(0.6f));
                    g.fillRect(x + 2, segmentY, width - 4, 1);
                } else {
                    // Inactive segment - dark
                    g.setColour(juce::Colour(0xFF2a2a2a));
                    g.fillRect(x + 2, segmentY, width - 4, segmentHeight);
                }
            }
            
            // Draw scale marks at key positions
            g.setColour(juce::Colour(0xFF666666));
            for (int i = 0; i <= 4; i++) {
                int markY = y + (height * i / 4);
                g.drawHorizontalLine(markY, x - 2, x + width + 2);
            }
        }
        
        void loadPreset()
        {
            if (!presetCombo) return;
            
            int selectedId = presetCombo->getSelectedId();
            
            // Set parameters based on preset
            switch (selectedId) {
                // Instrument presets
                case 1: // Vocals - Lead
                    if (passbandKnob) passbandKnob->setValue(0.2f); // 0.6dB
                    if (stopbandKnob) stopbandKnob->setValue(0.8f); // 108dB
                    if (ceilingKnob) ceilingKnob->setValue(0.83f); // -1.0dB
                    if (lookaheadKnob) lookaheadKnob->setValue(0.3f); // 3.0ms
                    break;
                case 2: // Vocals - Backing
                    if (passbandKnob) passbandKnob->setValue(0.33f); // 1.0dB
                    if (stopbandKnob) stopbandKnob->setValue(0.67f); // 100dB
                    if (ceilingKnob) ceilingKnob->setValue(0.67f); // -2.0dB
                    if (lookaheadKnob) lookaheadKnob->setValue(0.2f); // 2.0ms
                    break;
                case 3: // Guitar - Electric
                    if (passbandKnob) passbandKnob->setValue(0.5f); // 1.5dB
                    if (stopbandKnob) stopbandKnob->setValue(0.75f); // 105dB
                    if (ceilingKnob) ceilingKnob->setValue(0.5f); // -3.0dB
                    if (lookaheadKnob) lookaheadKnob->setValue(0.15f); // 1.5ms
                    break;
                case 8: // Drums - Kick
                    if (passbandKnob) passbandKnob->setValue(0.67f); // 2.0dB
                    if (stopbandKnob) stopbandKnob->setValue(0.5f); // 90dB
                    if (ceilingKnob) ceilingKnob->setValue(0.17f); // -5.0dB
                    if (lookaheadKnob) lookaheadKnob->setValue(0.1f); // 1.0ms
                    break;
                case 24: // Mix Bus
                    if (passbandKnob) passbandKnob->setValue(0.17f); // 0.5dB
                    if (stopbandKnob) stopbandKnob->setValue(0.9f); // 114dB
                    if (ceilingKnob) ceilingKnob->setValue(0.9f); // -0.6dB
                    if (lookaheadKnob) lookaheadKnob->setValue(0.5f); // 5.0ms
                    break;
                case 25: // Master Bus
                    if (passbandKnob) passbandKnob->setValue(0.1f); // 0.3dB
                    if (stopbandKnob) stopbandKnob->setValue(1.0f); // 120dB
                    if (ceilingKnob) ceilingKnob->setValue(1.0f); // 0.0dB
                    if (lookaheadKnob) lookaheadKnob->setValue(0.8f); // 8.0ms
                    break;
                default:
                    // Default preset values
                    if (passbandKnob) passbandKnob->setValue(0.33f); // 1.0dB
                    if (stopbandKnob) stopbandKnob->setValue(0.67f); // 100dB
                    if (ceilingKnob) ceilingKnob->setValue(0.83f); // -1.0dB
                    if (lookaheadKnob) lookaheadKnob->setValue(0.2f); // 2.0ms
                    break;
            }
            
            // Update meters and repaint
            if (passbandKnob) passbandMeter = passbandKnob->getValue();
            if (stopbandKnob) stopbandMeter = stopbandKnob->getValue();
            if (ceilingKnob) ceilingMeter = ceilingKnob->getValue();
            if (lookaheadKnob) lookaheadMeter = lookaheadKnob->getValue();
            repaint();
        }
        
        void showInfoWindow()
        {
            // Create custom graphics-enabled info window with proper closing
            auto* infoWindow = new ISPInfoWindow(processName + " - Professional Information");
            infoWindow->setSize(950, 750);
            infoWindow->centreWithSize(950, 750);
            infoWindow->setVisible(true);
            infoWindow->setAlwaysOnTop(true);
        }
        
        // Custom window class that properly handles closing
        class ISPInfoWindow : public juce::DocumentWindow
        {
        public:
            ISPInfoWindow(const juce::String& title) 
                : juce::DocumentWindow(title, juce::Colour(0xFF2a2a3e), 
                                     juce::DocumentWindow::closeButton | juce::DocumentWindow::minimiseButton)
            {
                setUsingNativeTitleBar(true);
                setDropShadowEnabled(true);
                
                // Create and set content - pass window title for correct info display
                auto infoComponent = std::make_unique<ISPInfoComponent>(title);
                setContentOwned(infoComponent.release(), true);
            }
            
            void closeButtonPressed() override
            {
                // Properly delete the window when close button is pressed
                delete this;
            }
        };
        
        
        juce::String processName, processDescription;
        std::unique_ptr<juce::ComboBox> osFactorCombo, filterTypeCombo, presetCombo;
        std::unique_ptr<CleanKnob> passbandKnob, stopbandKnob, ceilingKnob, lookaheadKnob;
        std::unique_ptr<juce::TextButton> infoButton;
        
        // Meter values (0.0 to 1.0)
        float passbandMeter = 0.0f;
        float stopbandMeter = 0.0f;
        float ceilingMeter = 0.0f;
        float lookaheadMeter = 0.0f;
    };
    
    // SRC Control Window - Sample Rate Conversion
    class SRCControlWindow : public juce::Component
    {
    public:
        SRCControlWindow(const juce::String& name, const juce::String& desc) : processName(name)
        {
            setSize(800, 600);
            
            try {
                // Create preset dropdown
                presetCombo = std::make_unique<juce::ComboBox>();
                if (presetCombo) {
                    presetCombo->addItem("CD Mastering (44.1k)", 1);
                    presetCombo->addItem("Broadcast (48k)", 2);
                    presetCombo->addItem("Hi-Res Audio (96k)", 3);
                    presetCombo->setSelectedId(1);
                    presetCombo->setColour(juce::ComboBox::backgroundColourId, juce::Colour(0xFF1a2a3a));
                    presetCombo->setColour(juce::ComboBox::textColourId, juce::Colour(0xFF87ceeb));
                    addAndMakeVisible(*presetCombo);
                }
                
                // Create Source Rate Switch
                sourceRateCombo = std::make_unique<juce::ComboBox>();
                if (sourceRateCombo) {
                    sourceRateCombo->addItem("44.1 kHz", 1);
                    sourceRateCombo->addItem("48 kHz", 2);
                    sourceRateCombo->addItem("88.2 kHz", 3);
                    sourceRateCombo->addItem("96 kHz", 4);
                    sourceRateCombo->addItem("176.4 kHz", 5);
                    sourceRateCombo->addItem("192 kHz", 6);
                    sourceRateCombo->setSelectedId(1);
                    sourceRateCombo->setColour(juce::ComboBox::backgroundColourId, juce::Colour(0xFF1a2a3a));
                    sourceRateCombo->setColour(juce::ComboBox::textColourId, juce::Colour(0xFF00d4aa));
                    addAndMakeVisible(*sourceRateCombo);
                }
                
                // Create Target Rate Switch
                targetRateCombo = std::make_unique<juce::ComboBox>();
                if (targetRateCombo) {
                    targetRateCombo->addItem("44.1 kHz", 1);
                    targetRateCombo->addItem("48 kHz", 2);
                    targetRateCombo->addItem("88.2 kHz", 3);
                    targetRateCombo->addItem("96 kHz", 4);
                    targetRateCombo->addItem("176.4 kHz", 5);
                    targetRateCombo->addItem("192 kHz", 6);
                    targetRateCombo->setSelectedId(2);
                    targetRateCombo->setColour(juce::ComboBox::backgroundColourId, juce::Colour(0xFF1a2a3a));
                    targetRateCombo->setColour(juce::ComboBox::textColourId, juce::Colour(0xFF87ceeb));
                    addAndMakeVisible(*targetRateCombo);
                }
                
                // Create Quality Mode Switch
                qualityCombo = std::make_unique<juce::ComboBox>();
                if (qualityCombo) {
                    qualityCombo->addItem("Draft", 1);
                    qualityCombo->addItem("Standard", 2);
                    qualityCombo->addItem("High", 3);
                    qualityCombo->addItem("Ultra", 4);
                    qualityCombo->setSelectedId(3);
                    qualityCombo->setColour(juce::ComboBox::backgroundColourId, juce::Colour(0xFF1a2a3a));
                    qualityCombo->setColour(juce::ComboBox::textColourId, juce::Colour(0xFFff6b35));
                    addAndMakeVisible(*qualityCombo);
                }
                
                // Create knobs for Passband Ripple and Stopband Attenuation
                rippleKnob = std::make_unique<CleanKnob>();
                if (rippleKnob) addAndMakeVisible(*rippleKnob);
                
                stopbandKnob = std::make_unique<CleanKnob>();
                if (stopbandKnob) addAndMakeVisible(*stopbandKnob);
                
                // Create INFO button
                infoButton = std::make_unique<juce::TextButton>("INFO");
                if (infoButton) {
                    infoButton->setColour(juce::TextButton::buttonColourId, juce::Colour(0xFF4a4a6a));
                    infoButton->onClick = [this]() { showInfoWindow(); };
                    addAndMakeVisible(*infoButton);
                }
            } catch (const std::exception& e) {
                proguiLog("[ERROR] Exception in SRCControlWindow constructor: " + juce::String(e.what()));
            }
        }
        
        void paint(juce::Graphics& g) override
        {
            g.fillAll(juce::Colour(0xFF1a1a2e));
            g.setColour(juce::Colours::white);
            g.setFont(18.0f);
            g.drawText(processName, 20, 20, getWidth() - 40, 30, juce::Justification::centred);
            
            // Preset area
            g.setColour(juce::Colour(0xFF1a2a3a));
            g.fillRoundedRectangle(getWidth()/2 - 90, 95, 180, 60, 8.0f);
            g.setColour(juce::Colour(0xFF87ceeb));
            g.setFont(13.0f);
            g.drawText("PRESET", getWidth()/2 - 80, 100, 160, 20, juce::Justification::centred);
            
            // Switch labels
            g.setColour(juce::Colours::white);
            g.setFont(11.0f);
            g.drawText("Source Rate", 70, 180, 120, 20, juce::Justification::centred);
            g.drawText("Target Rate", 310, 180, 120, 20, juce::Justification::centred);
            g.drawText("Quality Mode", getWidth()/2 - 75, 270, 150, 20, juce::Justification::centred);
            
            // Knob labels
            g.drawText("Passband Ripple", 70, 360, 120, 20, juce::Justification::centred);
            g.drawText("Stopband Attenuation", 280, 360, 160, 20, juce::Justification::centred);
        }
        
        void resized() override
        {
            if (presetCombo) presetCombo->setBounds(getWidth()/2 - 75, 120, 150, 25);
            if (sourceRateCombo) sourceRateCombo->setBounds(60, 210, 140, 30);
            if (targetRateCombo) targetRateCombo->setBounds(300, 210, 140, 30);
            if (qualityCombo) qualityCombo->setBounds(getWidth()/2 - 70, 300, 140, 30);
            if (rippleKnob) rippleKnob->setBounds(90, 390, 80, 80);
            if (stopbandKnob) stopbandKnob->setBounds(320, 390, 80, 80);
            if (infoButton) infoButton->setBounds(getWidth() - 80, 20, 60, 30);
        }
        
        void drawVerticalMeter(juce::Graphics& g, int x, int y, int width, int height, float value, juce::Colour color, const juce::String& label)
        {
            // Background
            g.setColour(juce::Colour(0xFF2a2a3e));
            g.fillRect(x, y, width, height);
            g.setColour(juce::Colour(0xFF4a4a6a));
            g.drawRect(x, y, width, height);
            
            // Segments
            int numSegments = 20;
            float segmentHeight = (float)height / numSegments;
            int activeSegments = (int)(value * numSegments);
            
            for (int i = 0; i < numSegments; i++)
            {
                float segY = y + height - (i + 1) * segmentHeight;
                if (i < activeSegments)
                {
                    float intensity = (float)i / numSegments;
                    juce::Colour segColor = color.withBrightness(0.3f + intensity * 0.7f);
                    g.setColour(segColor);
                    g.fillRect(x + 2, (int)segY + 1, width - 4, (int)segmentHeight - 2);
                }
            }
            
            // Label
            g.setColour(juce::Colours::white);
            g.setFont(8.0f);
            g.drawText(label, x - 10, y + height + 5, width + 20, 12, juce::Justification::centred);
        }
        
        void showInfoWindow()
        {
            auto* infoComponent = new ISPInfoComponent("SRC");
            
            juce::DialogWindow::LaunchOptions opts;
            opts.content.setOwned(infoComponent);
            opts.dialogTitle = "SRC - Sample Rate Conversion Information";
            opts.componentToCentreAround = this;
            opts.escapeKeyTriggersCloseButton = true;
            opts.useNativeTitleBar = true;
            opts.resizable = true;
            opts.launchAsync();
        }
        
    private:
        juce::String processName;
        std::unique_ptr<juce::ComboBox> presetCombo;
        std::unique_ptr<juce::ComboBox> sourceRateCombo;
        std::unique_ptr<juce::ComboBox> targetRateCombo;
        std::unique_ptr<juce::ComboBox> qualityCombo;
        std::unique_ptr<CleanKnob> rippleKnob, stopbandKnob;
        std::unique_ptr<juce::TextButton> infoButton;
    };
    
    // JITTER Control Window - Jitter & Accumulation
    class JitterControlWindow : public juce::Component
    {
    public:
        JitterControlWindow(const juce::String& name, const juce::String& desc) : processName(name)
        {
            setSize(800, 600);
            
            try {
                // Create preset dropdown
                presetCombo = std::make_unique<juce::ComboBox>();
                if (presetCombo) {
                    presetCombo->addSectionHeading("JITTER PRESETS");
                    presetCombo->addItem("Digital Mastering", 1);
                    presetCombo->addItem("Analog Simulation", 2);
                    presetCombo->addItem("Vintage Console", 3);
                    presetCombo->addItem("Modern Clean", 4);
                    presetCombo->setSelectedId(1);
                    presetCombo->setColour(juce::ComboBox::backgroundColourId, juce::Colour(0xFF1a2a3a));
                    presetCombo->setColour(juce::ComboBox::textColourId, juce::Colour(0xFF87ceeb));
                    addAndMakeVisible(*presetCombo);
                }
                
                // Create Dither Type Switch
                ditherTypeCombo = std::make_unique<juce::ComboBox>();
                if (ditherTypeCombo) {
                    ditherTypeCombo->addItem("TPDF", 1);
                    ditherTypeCombo->addItem("Triangular", 2);
                    ditherTypeCombo->addItem("Gaussian", 3);
                    ditherTypeCombo->addItem("None", 4);
                    ditherTypeCombo->setSelectedId(1);
                    ditherTypeCombo->setColour(juce::ComboBox::backgroundColourId, juce::Colour(0xFF1a2a3a));
                    ditherTypeCombo->setColour(juce::ComboBox::textColourId, juce::Colour(0xFF00d4aa));
                    addAndMakeVisible(*ditherTypeCombo);
                }
                
                // Create Bit Depth Switch
                bitDepthCombo = std::make_unique<juce::ComboBox>();
                if (bitDepthCombo) {
                    bitDepthCombo->addItem("16 bit", 1);
                    bitDepthCombo->addItem("20 bit", 2);
                    bitDepthCombo->addItem("24 bit", 3);
                    bitDepthCombo->addItem("32 bit", 4);
                    bitDepthCombo->setSelectedId(3); // Default to 24-bit
                    bitDepthCombo->setColour(juce::ComboBox::backgroundColourId, juce::Colour(0xFF1a2a3a));
                    bitDepthCombo->setColour(juce::ComboBox::textColourId, juce::Colour(0xFF87ceeb));
                    addAndMakeVisible(*bitDepthCombo);
                }
                
                // Create INFO button
                infoButton = std::make_unique<juce::TextButton>("INFO");
                if (infoButton) {
                    infoButton->setColour(juce::TextButton::buttonColourId, juce::Colour(0xFF4a4a6a));
                    infoButton->onClick = [this]() { showInfoWindow(); };
                    addAndMakeVisible(*infoButton);
                }
            } catch (const std::exception& e) {
                proguiLog("[ERROR] Exception in JitterControlWindow constructor: " + juce::String(e.what()));
            }
        }
        
        void paint(juce::Graphics& g) override
        {
            g.fillAll(juce::Colour(0xFF1a1a2e));
            g.setColour(juce::Colours::white);
            g.setFont(18.0f);
            g.drawText(processName, 20, 20, getWidth() - 40, 30, juce::Justification::centred);
            
            // Preset area
            g.setColour(juce::Colour(0xFF1a2a3a));
            g.fillRoundedRectangle(getWidth()/2 - 90, 95, 180, 60, 8.0f);
            g.setColour(juce::Colour(0xFF87ceeb));
            g.setFont(13.0f);
            g.drawText("PRESET", getWidth()/2 - 80, 100, 160, 20, juce::Justification::centred);
            
            // Draw meters with labels
            g.setColour(juce::Colours::white);
            g.setFont(11.0f);
            
            // RMS Jitter Meter
            g.drawText("RMS Jitter Meter", 50, 180, 150, 20, juce::Justification::centred);
            drawVerticalMeter(g, 100, 200, 30, 120, 0.3f, juce::Colour(0xFF00d4aa), "");
            g.setFont(9.0f);
            g.drawText("<10ps", 50, 325, 50, 15, juce::Justification::left);
            g.drawText("Good", 50, 340, 50, 15, juce::Justification::left);
            
            // Peak Jitter Display  
            g.setFont(11.0f);
            g.drawText("Peak Jitter", 200, 180, 120, 20, juce::Justification::centred);
            drawVerticalMeter(g, 240, 200, 30, 120, 0.5f, juce::Colour(0xFFff6b35), "");
            
            // Spectral Analysis
            g.drawText("Spectral Analysis", 350, 180, 150, 20, juce::Justification::centred);
            drawVerticalMeter(g, 400, 200, 30, 120, 0.4f, juce::Colour(0xFF4a90e2), "");
            
            // Control switches section
            g.setFont(11.0f);
            g.drawText("Dither Type", 100, 380, 120, 20, juce::Justification::centred);
            g.drawText("Bit Depth", 380, 380, 120, 20, juce::Justification::centred);
        }
        
        void resized() override
        {
            if (presetCombo) presetCombo->setBounds(getWidth()/2 - 75, 120, 150, 25);
            if (ditherTypeCombo) ditherTypeCombo->setBounds(60, 410, 180, 30);
            if (bitDepthCombo) bitDepthCombo->setBounds(340, 410, 180, 30);
            if (infoButton) infoButton->setBounds(getWidth() - 80, 20, 60, 30);
        }
        
        void drawVerticalMeter(juce::Graphics& g, int x, int y, int width, int height, float value, juce::Colour color, const juce::String& label)
        {
            // Background
            g.setColour(juce::Colour(0xFF2a2a3e));
            g.fillRect(x, y, width, height);
            g.setColour(juce::Colour(0xFF4a4a6a));
            g.drawRect(x, y, width, height);
            
            // Segments
            int numSegments = 20;
            float segmentHeight = (float)height / numSegments;
            int activeSegments = (int)(value * numSegments);
            
            for (int i = 0; i < numSegments; i++)
            {
                float segY = y + height - (i + 1) * segmentHeight;
                if (i < activeSegments)
                {
                    float intensity = (float)i / numSegments;
                    juce::Colour segColor = color.withBrightness(0.3f + intensity * 0.7f);
                    g.setColour(segColor);
                    g.fillRect(x + 2, (int)segY + 1, width - 4, (int)segmentHeight - 2);
                }
            }
            
            // Label
            g.setColour(juce::Colours::white);
            g.setFont(9.0f);
            g.drawText(label, x - 10, y + height + 5, width + 20, 15, juce::Justification::centred);
        }
        
        void showInfoWindow()
        {
            auto* infoComponent = new ISPInfoComponent("JITTER");
            
            juce::DialogWindow::LaunchOptions opts;
            opts.content.setOwned(infoComponent);
            opts.dialogTitle = "JITTER - Analysis & Optimization Information";
            opts.componentToCentreAround = this;
            opts.escapeKeyTriggersCloseButton = true;
            opts.useNativeTitleBar = true;
            opts.resizable = true;
            opts.launchAsync();
        }
        
    private:
        juce::String processName;
        std::unique_ptr<juce::ComboBox> presetCombo;
        std::unique_ptr<juce::ComboBox> ditherTypeCombo;
        std::unique_ptr<juce::ComboBox> bitDepthCombo;
        std::unique_ptr<juce::TextButton> infoButton;
    };
    
    // ALIGN Control Window - Phase/Time Alignment
    class AlignControlWindow : public juce::Component
    {
    public:
        AlignControlWindow(const juce::String& name, const juce::String& desc) : processName(name)
        {
            setSize(800, 600);
            
            try {
                // Create preset dropdown
                presetCombo = std::make_unique<juce::ComboBox>();
                if (presetCombo) {
                    presetCombo->addSectionHeading("ALIGNMENT PRESETS");
                    presetCombo->addItem("Stereo Imaging", 1);
                    presetCombo->addItem("Phase Correction", 2);
                    presetCombo->addItem("Time Alignment", 3);
                    presetCombo->addItem("Auto Align", 4);
                    presetCombo->setSelectedId(1);
                    presetCombo->setColour(juce::ComboBox::backgroundColourId, juce::Colour(0xFF1a2a3a));
                    presetCombo->setColour(juce::ComboBox::textColourId, juce::Colour(0xFF87ceeb));
                    addAndMakeVisible(*presetCombo);
                }
                
                // Create Time Delay knob
                delayKnob = std::make_unique<CleanKnob>();
                if (delayKnob) addAndMakeVisible(*delayKnob);
                
                // Create Crossover Frequency knob
                crossoverKnob = std::make_unique<CleanKnob>();
                if (crossoverKnob) addAndMakeVisible(*crossoverKnob);
                
                // Create Phase Rotation Switch
                phaseRotationCombo = std::make_unique<juce::ComboBox>();
                if (phaseRotationCombo) {
                    phaseRotationCombo->addItem("0\u00B0", 1);
                    phaseRotationCombo->addItem("90\u00B0", 2);
                    phaseRotationCombo->addItem("180\u00B0", 3);
                    phaseRotationCombo->addItem("270\u00B0", 4);
                    phaseRotationCombo->setSelectedId(1);
                    phaseRotationCombo->setColour(juce::ComboBox::backgroundColourId, juce::Colour(0xFF1a2a3a));
                    phaseRotationCombo->setColour(juce::ComboBox::textColourId, juce::Colour(0xFF4a90e2));
                    addAndMakeVisible(*phaseRotationCombo);
                }
                
                // Create Polarity Switch
                polarityCombo = std::make_unique<juce::ComboBox>();
                if (polarityCombo) {
                    polarityCombo->addItem("Normal", 1);
                    polarityCombo->addItem("Inverted", 2);
                    polarityCombo->setSelectedId(1);
                    polarityCombo->setColour(juce::ComboBox::backgroundColourId, juce::Colour(0xFF1a2a3a));
                    polarityCombo->setColour(juce::ComboBox::textColourId, juce::Colour(0xFFff6b35));
                    addAndMakeVisible(*polarityCombo);
                }
                
                // Create INFO button
                infoButton = std::make_unique<juce::TextButton>("INFO");
                if (infoButton) {
                    infoButton->setColour(juce::TextButton::buttonColourId, juce::Colour(0xFF4a4a6a));
                    infoButton->onClick = [this]() { showInfoWindow(); };
                    addAndMakeVisible(*infoButton);
                }
            } catch (const std::exception& e) {
                proguiLog("[ERROR] Exception in AlignControlWindow constructor: " + juce::String(e.what()));
            }
        }
        
        void paint(juce::Graphics& g) override
        {
            g.fillAll(juce::Colour(0xFF1a1a2e));
            g.setColour(juce::Colours::white);
            g.setFont(18.0f);
            g.drawText(processName, 20, 20, getWidth() - 40, 30, juce::Justification::centred);
            
            // Preset area
            g.setColour(juce::Colour(0xFF1a2a3a));
            g.fillRoundedRectangle(getWidth()/2 - 90, 95, 180, 60, 8.0f);
            g.setColour(juce::Colour(0xFF87ceeb));
            g.setFont(13.0f);
            g.drawText("PRESET", getWidth()/2 - 80, 100, 160, 20, juce::Justification::centred);
            
            // Draw Correlation Meter (center position)
            g.setColour(juce::Colours::white);
            g.setFont(13.0f);
            g.drawText("CORRELATION METER", getWidth()/2 - 100, 190, 200, 25, juce::Justification::centred);
            
            // Horizontal correlation meter (-1.0 to +1.0)
            int meterX = getWidth()/2 - 150;
            int meterY = 220;
            int meterWidth = 300;
            int meterHeight = 40;
            
            g.setColour(juce::Colour(0xFF2a2a3e));
            g.fillRect(meterX, meterY, meterWidth, meterHeight);
            g.setColour(juce::Colour(0xFF4a4a6a));
            g.drawRect(meterX, meterY, meterWidth, meterHeight);
            
            // Draw center line at 0.0
            g.setColour(juce::Colours::white);
            g.drawLine(meterX + meterWidth/2, meterY, meterX + meterWidth/2, meterY + meterHeight, 1.5f);
            
            // Draw correlation value (example: +0.8)
            float correlation = 0.8f; // Placeholder value
            float barWidth = (correlation + 1.0f) / 2.0f * meterWidth;
            g.setColour(correlation > 0.7f ? juce::Colour(0xFF00ff00) : correlation > 0.0f ? juce::Colour(0xFFffff00) : juce::Colour(0xFFff0000));
            g.fillRect((float)meterX, (float)meterY, barWidth, (float)meterHeight);
            
            // Draw scale markers
            g.setColour(juce::Colours::white);
            g.setFont(9.0f);
            g.drawText("-1.0", meterX, meterY + meterHeight + 5, 30, 15, juce::Justification::left);
            g.drawText("0.0", meterX + meterWidth/2 - 15, meterY + meterHeight + 5, 30, 15, juce::Justification::centred);
            g.drawText("+1.0", meterX + meterWidth - 30, meterY + meterHeight + 5, 30, 15, juce::Justification::right);
            
            // Knob and switch labels
            g.setFont(11.0f);
            g.drawText("Time Delay", 60, 300, 120, 20, juce::Justification::centred);
            g.drawText("Crossover Freq", 350, 300, 140, 20, juce::Justification::centred);
            g.drawText("Phase Rotation", 90, 440, 140, 20, juce::Justification::centred);
            g.drawText("Polarity", 380, 440, 100, 20, juce::Justification::centred);
        }
        
        void resized() override
        {
            if (presetCombo) presetCombo->setBounds(getWidth()/2 - 75, 120, 150, 25);
            if (delayKnob) delayKnob->setBounds(80, 330, 80, 80);
            if (crossoverKnob) crossoverKnob->setBounds(370, 330, 80, 80);
            if (phaseRotationCombo) phaseRotationCombo->setBounds(80, 470, 160, 30);
            if (polarityCombo) polarityCombo->setBounds(360, 470, 140, 30);
            if (infoButton) infoButton->setBounds(getWidth() - 80, 20, 60, 30);
        }
        
        void drawVerticalMeter(juce::Graphics& g, int x, int y, int width, int height, float value, juce::Colour color, const juce::String& label)
        {
            // Background
            g.setColour(juce::Colour(0xFF2a2a3e));
            g.fillRect(x, y, width, height);
            g.setColour(juce::Colour(0xFF4a4a6a));
            g.drawRect(x, y, width, height);
            
            // Segments
            int numSegments = 20;
            float segmentHeight = (float)height / numSegments;
            int activeSegments = (int)(value * numSegments);
            
            for (int i = 0; i < numSegments; i++)
            {
                float segY = y + height - (i + 1) * segmentHeight;
                if (i < activeSegments)
                {
                    float intensity = (float)i / numSegments;
                    juce::Colour segColor = color.withBrightness(0.3f + intensity * 0.7f);
                    g.setColour(segColor);
                    g.fillRect(x + 2, (int)segY + 1, width - 4, (int)segmentHeight - 2);
                }
            }
            
            // Label
            g.setColour(juce::Colours::white);
            g.setFont(9.0f);
            g.drawText(label, x - 10, y + height + 5, width + 20, 15, juce::Justification::centred);
        }
        
        void showInfoWindow()
        {
            auto* infoComponent = new ISPInfoComponent("ALIGN");
            
            juce::DialogWindow::LaunchOptions opts;
            opts.content.setOwned(infoComponent);
            opts.dialogTitle = "ALIGN - Phase & Time Alignment Information";
            opts.componentToCentreAround = this;
            opts.escapeKeyTriggersCloseButton = true;
            opts.useNativeTitleBar = true;
            opts.resizable = true;
            opts.launchAsync();
        }
        
    private:
        juce::String processName;
        std::unique_ptr<juce::ComboBox> presetCombo;
        std::unique_ptr<CleanKnob> delayKnob, crossoverKnob;
        std::unique_ptr<juce::ComboBox> phaseRotationCombo;
        std::unique_ptr<juce::ComboBox> polarityCombo;
        std::unique_ptr<juce::TextButton> infoButton;
    };
    
    // Remaining control windows - simplified implementations
    class TransientControlWindow : public juce::Component
    {
    public:
        TransientControlWindow(const juce::String& name, const juce::String& desc) : processName(name) 
        { 
            setSize(800, 600);
            presetCombo = std::make_unique<juce::ComboBox>();
            if (presetCombo) {
                // Set presets based on process type
                if (processName.contains("TRANSFORMER")) {
                    presetCombo->addSectionHeading("CLASSIC TRANSFORMERS");
                    presetCombo->addItem("Neve 1073 (Marinair)", 1);
                    presetCombo->addItem("API 2520 (Jensen)", 2);
                    presetCombo->addItem("Ampex 351 (Peerless)", 3);
                    presetCombo->addItem("Telefunken V72 (BV08)", 4);
                    presetCombo->addItem("Triad A-11J (American)", 5);
                    presetCombo->addItem("Triad A-12J (Vintage)", 6);
                } else if (processName.contains("TRANSIENT")) {
                    presetCombo->addItem("Drum Enhancement", 1);
                    presetCombo->addItem("Vocal Punch", 2);
                } else if (processName.contains("DEESSER")) {
                    presetCombo->addItem("Vocal De-essing", 1);
                    presetCombo->addItem("Broadcast Safe", 2);
                } else if (processName.contains("MLAR")) {
                    presetCombo->addItem("Vintage Warmth", 1);
                    presetCombo->addItem("Analog Character", 2);
                } else {
                    presetCombo->addItem("Default", 1);
                    presetCombo->addItem("Alternative", 2);
                }
                presetCombo->setSelectedId(1);
                presetCombo->setColour(juce::ComboBox::backgroundColourId, juce::Colour(0xFF1a2a3a));
                presetCombo->setColour(juce::ComboBox::textColourId, juce::Colour(0xFF87ceeb));
                addAndMakeVisible(*presetCombo);
            }
            for (int i = 0; i < 4; i++) {
                knobs[i] = std::make_unique<CleanKnob>();
                if (knobs[i]) {
                    // Connect knobs to meters
                    knobs[i]->onValueChanged = [this, i](float value) { 
                        meterValues[i] = value; 
                        repaint(); 
                    };
                    addAndMakeVisible(*knobs[i]);
                }
            }
            infoButton = std::make_unique<juce::TextButton>("INFO");
            if (infoButton) {
                infoButton->setColour(juce::TextButton::buttonColourId, juce::Colour(0xFF4a4a6a));
                infoButton->onClick = [this]() { showProcessInfoWindow(); };
                addAndMakeVisible(*infoButton);
            }
        }
        void paint(juce::Graphics& g) override {
            g.fillAll(juce::Colour(0xFF1a1a2e));
            g.setColour(juce::Colours::white);
            g.setFont(18.0f);
            g.drawText(processName, 20, 20, getWidth() - 40, 30, juce::Justification::centred);
            
            // Preset area
            g.setColour(juce::Colour(0xFF1a2a3a));
            g.fillRoundedRectangle(getWidth()/2 - 90, 95, 180, 60, 8.0f);
            g.setColour(juce::Colour(0xFF87ceeb));
            g.setFont(13.0f);
            // Use "TRANSFORMER TYPE" label for Transformer window, "PRESET" for others
            if (processName.contains("TRANSFORMER")) {
                g.drawText("TRANSFORMER TYPE", getWidth()/2 - 80, 100, 160, 20, juce::Justification::centred);
            } else {
                g.drawText("PRESET", getWidth()/2 - 80, 100, 160, 20, juce::Justification::centred);
            }
            
            // Process-specific labels and meters
            g.setColour(juce::Colours::white);
            g.setFont(11.0f);
            if (processName.contains("TRANSIENT")) {
                g.drawText("Sensitivity", 70, 240, 80, 40, juce::Justification::centred);
                g.drawText("Attack", 190, 240, 80, 40, juce::Justification::centred);
                g.drawText("Release", 310, 240, 80, 40, juce::Justification::centred);
                g.drawText("Boost", 430, 240, 80, 40, juce::Justification::centred);
                
                // Draw meters centered above knobs (knob centers: 120, 240, 360, 480)
                drawVerticalMeter(g, 110, 280, 20, 100, meterValues[0], juce::Colour(0xFF00d4aa), "Sens");
                drawVerticalMeter(g, 230, 280, 20, 100, meterValues[1], juce::Colour(0xFFff6b35), "Att");
                drawVerticalMeter(g, 350, 280, 20, 100, meterValues[2], juce::Colour(0xFF4a90e2), "Rel");
                drawVerticalMeter(g, 470, 280, 20, 100, meterValues[3], juce::Colour(0xFFf7931e), "Boost");
                
            } else if (processName.contains("DEESSER")) {
                g.drawText("Frequency", 70, 240, 80, 40, juce::Justification::centred);
                g.drawText("Q Factor", 190, 240, 80, 40, juce::Justification::centred);
                g.drawText("Threshold", 310, 240, 80, 40, juce::Justification::centred);
                g.drawText("Amount", 430, 240, 80, 40, juce::Justification::centred);
                
                drawVerticalMeter(g, 110, 280, 20, 100, meterValues[0], juce::Colour(0xFF00d4aa), "Freq");
                drawVerticalMeter(g, 230, 280, 20, 100, meterValues[1], juce::Colour(0xFFff6b35), "Q");
                drawVerticalMeter(g, 350, 280, 20, 100, meterValues[2], juce::Colour(0xFF4a90e2), "Thresh");
                drawVerticalMeter(g, 470, 280, 20, 100, meterValues[3], juce::Colour(0xFFf7931e), "Amount");
                
            } else if (processName.contains("MLAR")) {
                g.drawText("Amount", 70, 240, 80, 40, juce::Justification::centred);
                g.drawText("Profile", 190, 240, 80, 40, juce::Justification::centred);
                g.drawText("Focus", 310, 240, 80, 40, juce::Justification::centred);
                g.drawText("Blend", 430, 240, 80, 40, juce::Justification::centred);
                
                drawVerticalMeter(g, 110, 280, 20, 100, meterValues[0], juce::Colour(0xFF00d4aa), "Amt");
                drawVerticalMeter(g, 230, 280, 20, 100, meterValues[1], juce::Colour(0xFFff6b35), "Prof");
                drawVerticalMeter(g, 350, 280, 20, 100, meterValues[2], juce::Colour(0xFF4a90e2), "Focus");
                drawVerticalMeter(g, 470, 280, 20, 100, meterValues[3], juce::Colour(0xFFf7931e), "Blend");
                
            } else if (processName.contains("TRANSFORMER")) {
                g.drawText("Drive", 70, 240, 80, 40, juce::Justification::centred);
                g.drawText("Balance", 190, 240, 80, 40, juce::Justification::centred);
                g.drawText("Bias", 310, 240, 80, 40, juce::Justification::centred);
                g.drawText("Emphasis", 430, 240, 80, 40, juce::Justification::centred);
                
                drawVerticalMeter(g, 110, 280, 20, 100, meterValues[0], juce::Colour(0xFF00d4aa), "Drive");
                drawVerticalMeter(g, 230, 280, 20, 100, meterValues[1], juce::Colour(0xFFff6b35), "Bal");
                drawVerticalMeter(g, 350, 280, 20, 100, meterValues[2], juce::Colour(0xFF4a90e2), "Bias");
                drawVerticalMeter(g, 470, 280, 20, 100, meterValues[3], juce::Colour(0xFFf7931e), "Emph");
            }
        }
        void resized() override {
            if (presetCombo) presetCombo->setBounds(getWidth()/2 - 75, 120, 150, 25);
            for (int i = 0; i < 4; i++) {
                if (knobs[i]) knobs[i]->setBounds(80 + i*120, 410, 80, 80);
            }
            if (infoButton) infoButton->setBounds(getWidth() - 80, 20, 60, 30);
        }
        
        void drawVerticalMeter(juce::Graphics& g, int x, int y, int width, int height, float value, juce::Colour color, const juce::String& label)
        {
            // Background
            g.setColour(juce::Colour(0xFF2a2a3e));
            g.fillRect(x, y, width, height);
            g.setColour(juce::Colour(0xFF4a4a6a));
            g.drawRect(x, y, width, height);
            
            // Segments
            int numSegments = 20;
            float segmentHeight = (float)height / numSegments;
            int activeSegments = (int)(value * numSegments);
            
            for (int i = 0; i < numSegments; i++)
            {
                float segY = y + height - (i + 1) * segmentHeight;
                if (i < activeSegments)
                {
                    float intensity = (float)i / numSegments;
                    juce::Colour segColor = color.withBrightness(0.3f + intensity * 0.7f);
                    g.setColour(segColor);
                    g.fillRect(x + 2, (int)segY + 1, width - 4, (int)segmentHeight - 2);
                }
            }
            
            // Label
            g.setColour(juce::Colours::white);
            g.setFont(9.0f);
            g.drawText(label, x - 10, y + height + 5, width + 20, 15, juce::Justification::centred);
        }
        
        void showProcessInfoWindow()
        {
            auto* infoWindow = new ProcessInfoWindow(processName + " - Professional Information");
            infoWindow->setSize(950, 750);
            infoWindow->centreWithSize(950, 750);
            infoWindow->setVisible(true);
            infoWindow->setAlwaysOnTop(true);
        }
        
        class ProcessInfoWindow : public juce::DocumentWindow
        {
        public:
            ProcessInfoWindow(const juce::String& title) 
                : juce::DocumentWindow(title, juce::Colour(0xFF2a2a3e), 
                                     juce::DocumentWindow::closeButton | juce::DocumentWindow::minimiseButton)
            {
                setUsingNativeTitleBar(true);
                setDropShadowEnabled(true);
                
                auto infoComponent = std::make_unique<ProcessInfoComponent>(title);
                setContentOwned(infoComponent.release(), true);
            }
            
            void closeButtonPressed() override { delete this; }
        };
        
        class ProcessInfoComponent : public juce::Component
        {
        public:
            ProcessInfoComponent(const juce::String& title) : windowTitle(title) { setSize(950, 750); }
            
            void paint(juce::Graphics& g) override
            {
                g.fillAll(juce::Colour(0xFF1a1a2e));
                
                g.setColour(juce::Colours::white);
                g.setFont(20.0f);
                g.drawText(windowTitle, 20, 20, getWidth() - 40, 30, juce::Justification::centred);
                
                g.setColour(juce::Colour(0xFF00ffff));
                g.setFont(14.0f);
                
                juce::String infoText = getProcessInfo();
                g.drawFittedText(infoText, 20, 60, 500, 600, juce::Justification::topLeft, 100);
                
                // Graphics area
                drawProcessGraphics(g, 540, 60, 380, 600);
            }
            
        private:
            juce::String windowTitle;
            
            juce::String getProcessInfo()
            {
                if (windowTitle.contains("JITTER")) {
                    return "OVERVIEW:\nAdvanced jitter analysis and digital audio optimization system that measures, analyzes, and corrects timing irregularities in digital audio systems. This comprehensive tool ensures maximum digital audio fidelity through precise clock analysis and bit-depth optimization.\n\n"
                           "WHAT IS JITTER AND WHY IT MATTERS:\n- Jitter = timing irregularities in digital audio clocks and sample timing\n- Even tiny amounts (picoseconds) can degrade sound quality: harshness, reduced depth, loss of stereo imaging\n- Jitter adds noise, distortion, and timing errors that blur transients and muddy the soundstage\n- Professional systems must minimize jitter for transparent, high-fidelity audio reproduction\n- Critical for mastering, archive transfers, and high-resolution audio production\n\n"
                           "JITTER ANALYSIS IN DETAIL:\n- Real-time RMS jitter measurement with picosecond (0.000000000001 second) accuracy\n- Spectral analysis shows jitter distribution across 20Hz-20kHz audio frequency bands\n- Phase noise detection identifies unstable clock sources and interference\n- Clock stability monitoring tracks drift and temperature-related variations over time\n- Accumulated jitter tracking measures long-term system performance\n- Correlation with audio artifacts helps identify audible quality issues\n\n"
                           "TYPES OF JITTER:\n- Period Jitter: Variation in individual sample clock periods\n- Cycle-to-Cycle Jitter: Differences between consecutive clock cycles\n- Time Interval Error (TIE): Cumulative timing error over multiple samples\n- Phase Jitter: Phase variations in the clock signal\n- Random Jitter: Unpredictable, noise-like timing variations\n- Deterministic Jitter: Repeating, pattern-based timing errors\n\n"
                           "BIT-DEPTH OPTIMIZATION:\n- Dynamic bit allocation intelligently assigns bits based on signal content\n- Noise shaping algorithms push quantization noise to inaudible frequency ranges\n- Multiple dithering algorithms: TPDF (industry standard), Triangular, Gaussian, Blue (ultrasonic optimized)\n- Quantization error analysis measures and minimizes digital artifacts\n- Word length reduction maintains quality when converting to lower bit depths (24-bit to 16-bit)\n- Preserves dynamic range and prevents harsh digital artifacts\n\n"
                           "DIGITAL INTERFACE ANALYSIS:\n- S/PDIF: Consumer digital audio interface jitter measurement and correction\n- AES/EBU: Professional digital audio standard timing analysis\n- USB Audio: Computer interface jitter detection and compensation\n- ADAT: Optical multi-channel digital audio verification\n- Network Audio (Dante, AVB, AES67): Modern networked audio timing verification\n- Sample Rate Converter: Analyzes jitter introduced by SRC processes\n- Clock Recovery: Evaluates PLL (Phase-Locked Loop) performance\n\n"
                           "AUDIBLE EFFECTS OF JITTER:\n- Low Jitter (<10ps): Transparent, clean, three-dimensional soundstage\n- Moderate Jitter (10-100ps): Slight harshness, reduced depth, compressed dynamics\n- High Jitter (>100ps): Obvious harshness, flat soundstage, blurred transients, metallic artifacts\n- The difference between 5ps and 50ps jitter is clearly audible on resolving systems\n- Most critical in high-frequency content: cymbals, vocals, acoustic instruments\n\n"
                           "JITTER SOURCES:\n- Clock oscillator quality and stability\n- Power supply noise and interference\n- Ground loops and EMI (electromagnetic interference)\n- Long cable runs and impedance mismatches\n- Poor PCB (circuit board) design and layout\n- Sample rate conversion artifacts\n- Multiple cascaded digital devices\n\n"
                           "TECHNICAL SPECIFICATIONS:\n- Jitter RMS Range: 0.1ps to 10,000ps with 0.01ps resolution\n- Frequency Analysis: 20Hz-20kHz in 1/3 octave bands\n- Bit Depth Support: 16, 20, 24, 32-bit integer and 32/64-bit float\n- Sample Rates: 44.1kHz to 192kHz (and higher with external clocks)\n- Measurement Accuracy: +/-0.1ps RMS, +/-0.01% frequency\n- Analysis Window: 1ms to 10 seconds selectable\n- Display Update Rate: Real-time with 20ms refresh\n\n"
                           "PROFESSIONAL APPLICATIONS:\n- Mastering: Ensure optimal digital quality before final delivery, verify low jitter throughout chain\n- Digital Transfer: Verify and optimize A/D and D/A conversion quality, minimize generational loss\n- System Design: Analyze and optimize digital audio system performance, identify problem devices\n- Quality Control: Verify digital audio equipment specifications meet manufacturer claims\n- Research & Development: Investigate digital audio artifacts and their causes\n- Broadcast: Ensure compliance with digital audio broadcasting standards and regulations\n- Critical Listening: Validate playback system quality for accurate monitoring\n\n"
                           "MEASUREMENT CONTROLS:\n- Jitter RMS: Displays current and peak jitter levels in picoseconds\n- Spectrum: Shows jitter distribution across frequency bands with color coding\n- Bit Depth: Analyzes effective bit depth and noise floor characteristics\n- Quantization: Measures and displays quantization artifacts and their audibility";
                } else if (windowTitle.contains("SRC")) {
                    return "OVERVIEW:\nSample Rate Conversion - Professional-grade resampling system that converts audio between different sample rates while preserving maximum audio quality. Uses advanced filtering algorithms to eliminate aliasing and maintain frequency response accuracy.\n\n"
                           "WHY SAMPLE RATE CONVERSION MATTERS:\n- Different media formats require different sample rates (CD=44.1k, Broadcast=48k, Hi-Res=96k/192k)\n- Poor conversion causes audible artifacts: aliasing, imaging, pre-ringing, loss of high-frequency detail\n- Professional SRC preserves transients, stereo imaging, and frequency response accuracy\n- Essential for mastering, archive transfers, and multi-format distribution\n\n"
                           "RESAMPLING ALGORITHMS:\n- Multi-stage interpolation and decimation for optimal performance\n- Linear phase FIR filtering for transparent, artifact-free conversion\n- Minimum phase options for zero-latency real-time applications\n- Adaptive filter length automatically adjusts based on quality settings\n- Sophisticated dithering at final output stage for optimal bit depth reduction\n\n"
                           "SAMPLE RATE FAMILIES EXPLAINED:\n- 44.1kHz Family: 44.1, 88.2, 176.4kHz (CD, music production)\n- 48kHz Family: 48, 96, 192, 384kHz (video, broadcast, professional)\n- Converting within families (44.1->88.2) is simpler than cross-family (44.1->48)\n- Cross-family conversion requires more sophisticated algorithms\n- Non-standard rates: 32, 64, 128kHz supported for legacy/specialized equipment\n- Arbitrary custom rates via ratio specification for unique applications\n\n"
                           "QUALITY SETTINGS DETAILED:\n- Draft Mode: Fast preview conversion (80dB stopband, <0.01% CPU)\n- Standard Mode: Balanced quality/speed for general use (100dB stopband, ~0.1% CPU)\n- High Mode: Professional quality for distribution (120dB stopband, ~0.5% CPU)\n- Ultra Mode: Maximum quality for critical mastering (140dB+ stopband, ~2% CPU)\n- Custom Mode: User-defined passband/stopband specifications for specialized needs\n\n"
                           "ANTI-ALIASING FILTER DESIGN:\n- Passband Ripple: Adjustable 0.001dB to 0.1dB variation (flatter = more transparent)\n- Stopband Attenuation: 80dB to 160dB rejection (higher = cleaner conversion)\n- Transition Bandwidth: Optimized for minimal pre-ringing and time-domain accuracy\n- Phase Response Options: Linear (transparent) or Minimum (zero-latency)\n- Filter Length: Automatically calculated for optimal quality/performance balance\n\n"
                           "LATENCY AND PERFORMANCE:\n- Draft: ~1ms latency, suitable for real-time monitoring\n- Standard: ~5ms latency, ideal for mixing and general production\n- High: ~20ms latency, appropriate for mastering and offline processing\n- Ultra: ~50ms latency, designed for critical archive and distribution\n- Processing load scales with quality setting and conversion ratio complexity\n\n"
                           "TECHNICAL SPECIFICATIONS:\n- Passband: DC to 0.4535 x Nyquist frequency (adjustable for different requirements)\n- Stopband Rejection: Up to 160dB (eliminates aliasing artifacts completely)\n- THD+N: <0.0001% at nominal levels (transparent, no added distortion)\n- Frequency Response: +/-0.01dB in passband (maintains tonal balance)\n- Phase Linearity: +/-0.5 degrees in passband for linear phase mode\n- Processing: 64-bit floating point precision throughout signal path\n- Dynamic Range: >140dB when properly dithered\n\n"
                           "COMMON CONVERSION SCENARIOS:\n- CD to Broadcast: 44.1kHz -> 48kHz (cross-family, requires high quality)\n- Hi-Res Mastering: 96kHz -> 44.1kHz for CD (2.177:1 ratio, critical quality)\n- Video Post: Various rates -> 48kHz for video synchronization\n- Archive Preservation: Original rate -> multiple distribution formats\n- Live Sound: Mixing console rate -> output device rate in real-time\n\n"
                           "PROFESSIONAL APPLICATIONS:\n- Mastering: Convert between CD (44.1kHz) and broadcast (48kHz) standards\n- Archive Transfer: Digitize at highest rate, convert for multiple distributions\n- Film/Video Post: Convert audio to match video frame rates (23.976, 24, 25, 29.97, 30fps)\n- Broadcast: Ensure compliance with broadcast sample rate standards\n- Hi-Res Distribution: Create multiple sample rate versions from high-res masters\n- Live Sound: Interface between devices with different clock rates in real-time\n- Podcast Production: Standardize various source sample rates for consistent output\n\n"
                           "AUDIBLE QUALITY DIFFERENCES:\n- Poor SRC: Harsh highs, blurred transients, reduced stereo width, metallic artifacts\n- Good SRC: Maintains original character, preserves detail, transparent conversion\n- The difference is most audible on complex material: drums, vocals, acoustic instruments\n- Critical for preserving high-frequency air and spatial imaging\n\n"
                           "CONTROL DESCRIPTIONS:\n- Target Rate: Destination sample rate selection (standard rates or custom)\n- Quality: Conversion algorithm quality level (Draft/Standard/High/Ultra/Custom)\n- Phase Mode: Linear phase (transparent) or minimum phase (zero latency)\n- Dither: Optional dithering for bit depth reduction during conversion";
                } else if (windowTitle.contains("ALIGN")) {
                    return "OVERVIEW:\nPhase and Time Alignment - Precision synchronization system for multi-source audio that eliminates phase cancellation, comb filtering, and timing issues. Essential for maintaining coherent sound when combining multiple microphones or sources.\n\n"
                           "WHY PHASE ALIGNMENT MATTERS:\n- Out-of-phase signals cause comb filtering, hollowness, and loss of bass response\n- Timing misalignment between mics creates phase cancellation at specific frequencies\n- Proper alignment restores fullness, punch, clarity, and three-dimensional soundstage\n- Critical for multi-mic recordings, stereo imaging, and professional productions\n- The difference between aligned and misaligned signals is immediately audible\n\n"
                           "TIME ALIGNMENT IN DETAIL:\n- Sample-accurate delay compensation (+/-10,000 samples = +/-208ms at 48kHz)\n- Subsample precision using high-quality interpolation (0.01 sample accuracy)\n- Distance-based delay calculator: enter mic distances, get exact delay times\n- Speed of sound compensation accounts for temperature (sound travels faster in warm air)\n- Group delay compensation for different processing chains and speaker crossovers\n- Essential for aligning: overhead mics to close mics, room mics to spot mics, stereo pairs\n\n"
                           "UNDERSTANDING PHASE:\n- Phase describes the timing relationship between two waveforms\n- In-phase (0 degrees): Signals add constructively, increased level and fullness\n- Out-of-phase (180 degrees): Signals cancel, thin sound, loss of bass\n- Partially out-of-phase (90 degrees): Comb filtering, hollow, phasey sound\n- Phase relationships vary with frequency, creating complex interference patterns\n\n"
                           "PHASE ALIGNMENT CONTROLS:\n- Continuous phase rotation (0-360 degrees) with 0.1-degree precision\n- All-pass filter implementation provides frequency-independent phase rotation\n- Phase correlation analysis shows real-time coherence between channels\n- Automatic phase alignment detection analyzes tracks and suggests optimal settings\n- Frequency-dependent phase adjustment for surgical correction\n- Visual phase scope display shows phase relationships in real-time\n\n"
                           "POLARITY CORRECTION:\n- Instant polarity inversion (180-degree phase flip)\n- Automatic polarity detection algorithm identifies inverted signals\n- Visual polarity correlation display (-1 = inverted, +1 = in-phase, 0 = uncorrelated)\n- Per-channel polarity control for multi-track productions\n- Preserves frequency response while correcting phase relationships\n- Essential when one mic is wired backwards or amp polarity is reversed\n\n"
                           "CORRELATION METER EXPLAINED:\n- +1.0: Perfect correlation, signals completely in-phase (mono)\n- +0.7 to +1.0: Good correlation, solid stereo image with good mono compatibility\n- +0.3 to +0.7: Moderate correlation, wide stereo but some phase issues\n- 0.0: No correlation, completely independent signals\n- -0.3 to 0.0: Inverted correlation, serious phase problems, thin sound\n- -1.0: Perfect anti-correlation, signals completely out-of-phase\n\n"
                           "CROSSOVER PROCESSING:\n- Linear phase crossover filters maintain phase coherence across frequency bands\n- Adjustable crossover frequency (20Hz-20kHz) for flexible band separation\n- Slope options: 6dB (gentle), 12dB (standard), 18dB (steep), 24dB/octave (very steep)\n- Linkwitz-Riley alignment ensures flat summed frequency response\n- Independent processing of low and high frequency bands\n- Essential for subwoofer alignment, multi-way speaker systems, and frequency-specific processing\n\n"
                           "AUTO-ALIGN FUNCTION:\n- Analyzes multiple tracks for phase relationships using cross-correlation\n- Automatically calculates optimal delay times for maximum coherence\n- Detects and corrects polarity inversions automatically\n- Maximizes phase coherence across entire frequency spectrum\n- Reports alignment quality metrics and suggests improvements\n- Saves hours of manual trial-and-error adjustment\n\n"
                           "COMMON PHASE PROBLEMS:\n- Drum Overheads vs Close Mics: Overheads capture sound later due to distance\n- Stereo Mic Pair: One mic slightly farther from source causes phase smearing\n- Double-Tracked Guitars: Slightly different timing creates comb filtering\n- Bass DI + Mic: Different signal paths cause phase cancellation\n- Room Mics: Reflections and distance create complex phase relationships\n\n"
                           "AUDIBLE EFFECTS:\n- Good Alignment: Full, punchy, clear, three-dimensional, solid bass\n- Poor Alignment: Thin, hollow, phasey, weak bass, unstable stereo image\n- Comb Filtering: Notches at specific frequencies, robotic or tunnel-like sound\n- Polarity Inversion: Dramatic loss of bass, thin midrange, unstable image\n\n"
                           "TECHNICAL SPECIFICATIONS:\n- Delay Range: +/-10,000 samples (+/-208ms at 48kHz, +/-227ms at 44.1kHz)\n- Delay Resolution: 0.01 sample precision via high-quality interpolation\n- Phase Rotation: 0.1-degree resolution (0-360 degrees continuous)\n- Correlation Meter: -1.0 to +1.0 display range, 0.01 resolution\n- Processing Latency: <1ms added latency (look-ahead mode adds specified delay)\n- Frequency Response: +/-0.01dB when properly aligned\n- Crossover Slopes: 6, 12, 18, 24dB/octave Linkwitz-Riley\n\n"
                           "PROFESSIONAL APPLICATIONS:\n- Multi-Mic Drum Recording: Align overheads, close mics, room mics for solid, punchy drums\n- Stereo Pair Correction: Fix spacing and positioning issues in stereo recordings\n- Double-Tracked Instruments: Phase-align multiple guitar, vocal, or bass takes\n- Bass Recording: Align DI signal with microphone for full, solid low end\n- Room Correction: Compensate for speaker/listener position in studios and live venues\n- Live Sound: Align main PA speakers with delay fill speakers for uniform coverage\n- Post Production: Sync audio from multiple camera sources for video production\n- Mastering: Verify and optimize stereo image phase coherence\n\n"
                           "WORKFLOW TIPS:\n- Always check correlation meter when combining multiple signals\n- Use Auto-Align as starting point, then fine-tune by ear\n- Flip polarity first, then adjust timing delay\n- Solo signals individually, then together to hear phase interaction\n- Check phase at different frequencies using crossover bands\n- Zoom in on transients to align precisely by visual waveform\n\n"
                           "CONTROL DESCRIPTIONS:\n- Delay: Time offset in samples or milliseconds for precise timing alignment\n- Polarity: Instant signal inversion (normal/inverted) for phase correction\n- Phase Rotate: Continuous phase adjustment (0-360 degrees) for fine tuning\n- Crossover: Frequency split point for independent band processing and alignment";
                } else if (windowTitle.contains("TRANSIENT")) {
                    return "OVERVIEW:\nIntelligent transient detection and dynamic enhancement system for precise control over attack and sustain characteristics. This advanced algorithm separates transient and tonal components, allowing independent processing of each element.\n\n"
                           "TRANSIENT DETECTION:\n- Adaptive algorithm analyzes signal for transient events\n- Multi-band detection across frequency spectrum\n- Adjustable sensitivity for different source materials\n- Real-time analysis with minimal latency\n- Preserves original timing relationships\n\n"
                           "ATTACK PROCESSING:\n- Independent control over transient attack characteristics\n- Boost: Enhances punch and definition (0 to +12dB)\n- Cut: Softens aggressive transients (0 to -12dB)\n- Frequency-selective processing maintains tonal balance\n- Preserves natural decay characteristics\n\n"
                           "RELEASE PROCESSING:\n- Controls the decay and sustain portions of sounds\n- Extends or shortens natural sustain characteristics\n- Maintains harmonic content during processing\n- Independent from attack processing\n- Adjustable time constants for different materials\n\n"
                           "BOOST/SOFTEN CONTROL:\n- Overall enhancement or reduction of transient energy\n- Boost: Increases perceived punch and clarity\n- Soften: Reduces harsh or aggressive characteristics\n- Maintains musical balance between elements\n- Frequency-dependent processing for natural results\n\n"
                           "TECHNICAL SPECIFICATIONS:\n- Sensitivity: 0-100% detection threshold adjustment\n- Attack Time: 0.1ms to 10ms response window\n- Release Time: 10ms to 1000ms decay control\n- Boost/Soften: +/-12dB processing range\n- Frequency Response: 20Hz-20kHz full bandwidth\n- Processing Latency: <3ms for real-time use\n\n"
                           "PROFESSIONAL APPLICATIONS:\n- Drum Processing: Enhance kick punch, snare crack, cymbal definition\n- Vocal Enhancement: Control sibilance, improve clarity\n- Mix Bus Processing: Add cohesive punch to entire mix\n- Mastering: Final transient optimization\n- Live Sound: Real-time enhancement without artifacts\n- Post Production: Dialog and sound effect enhancement\n\n"
                           "CONTROL DESCRIPTIONS:\n- Sensitivity: Adjusts detection threshold for transient events\n- Attack: Controls transient attack enhancement or reduction\n- Release: Adjusts sustain and decay characteristics\n- Boost: Overall transient energy enhancement control";
                } else if (windowTitle.contains("DEESSER")) {
                    return "OVERVIEW:\nDe-essing - Advanced sibilance detection and transparent reduction system specifically designed for vocal processing. Uses frequency-selective dynamic processing to control harsh 'S' and 'T' sounds while preserving vocal clarity and natural character.\n\n"
                           "SIBILANCE DETECTION:\n- Frequency-selective band analysis (2-20kHz)\n- Adaptive threshold algorithm tracks vocal dynamics\n- Look-ahead detection prevents sibilance clipping\n- RMS and peak detection modes for different material\n- Real-time spectrum analysis for visual feedback\n\n"
                           "PROCESSING ALGORITHMS:\n- Transparent gain reduction maintains vocal character\n- Split-band processing isolates sibilant frequencies\n- Minimal phase distortion for natural sound\n- Adjustable attack and release times\n- Frequency-conscious dynamic control\n\n"
                           "FREQUENCY CONTROL (2-20kHz):\n- Targets specific sibilant frequency range\n- Variable center frequency for different voices\n- Male voices: typically 5-8kHz\n- Female voices: typically 6-10kHz\n- Adjustable to match individual vocal characteristics\n\n"
                           "Q FACTOR / BANDWIDTH (0.1-20):\n- Controls width of frequency band being processed\n- Narrow Q: Surgical removal of specific sibilance\n- Wide Q: Broader, more natural-sounding reduction\n- Adjustable for different sibilance characteristics\n- Prevents over-processing of adjacent frequencies\n\n"
                           "THRESHOLD CONTROL (-60 to 0dB):\n- Sets level at which de-essing begins\n- Adaptive threshold tracks signal dynamics\n- Lower threshold: more aggressive de-essing\n- Higher threshold: subtle, transparent control\n- Visual metering shows threshold activity\n\n"
                           "AMOUNT / REDUCTION (0-20dB):\n- Controls maximum gain reduction applied\n- Adjustable attenuation depth\n- Prevents over-processing and lisping\n- Maintains vocal presence while controlling sibilance\n- Real-time gain reduction metering\n\n"
                           "PROCESSING MODES:\n- Split-band: Processes only sibilant frequencies\n- Wide-band: Reduces entire signal when sibilance detected\n- Listen Mode: Solo sibilant frequency band for tuning\n- Stereo-linked: Maintains stereo image coherence\n- Mid/Side: Independent processing of center and sides\n\n"
                           "TECHNICAL SPECIFICATIONS:\n- Frequency Range: 2kHz to 20kHz center frequency\n- Q Factor: 0.1 to 20.0 (octave bandwidth control)\n- Threshold: -60dB to 0dB with 0.1dB resolution\n- Maximum Reduction: 0dB to 20dB attenuation\n- Attack Time: 0.1ms to 10ms\n- Release Time: 10ms to 1000ms\n- Processing Latency: <2ms for real-time use\n\n"
                           "PROFESSIONAL APPLICATIONS:\n- Vocal Production: Control sibilance in lead and backing vocals\n- Broadcast: Ensure compliance with broadcast standards\n- Podcasts: Improve dialogue clarity and listenability\n- Voice-Over: Professional narration and ADR work\n- Live Sound: Real-time sibilance control for PA systems\n- Mastering: Final polish for vocal-heavy mixes\n\n"
                           "CONTROL DESCRIPTIONS:\n- Frequency: Center frequency of sibilance detection band\n- Q Factor: Bandwidth of processing (narrow to wide)\n- Threshold: Level at which de-essing engages\n- Amount: Maximum gain reduction applied to sibilants";
                } else if (windowTitle.contains("MLAR")) {
                    return "OVERVIEW:\n'More Like A Record' - Advanced psychoacoustic processing that transforms digital audio to recreate the perceived warmth, depth, and cohesiveness of analog recordings. This process analyzes and enhances the subtle sonic characteristics that make recordings sound more musical and engaging.\n\n"
                           "CORE PROCESSING ALGORITHMS:\n- Harmonic Enhancement: Adds musically-related harmonics that create perceived warmth and richness\n- Stereo Field Optimization: Enhances spatial imaging and stereo width for better dimensionality\n- Dynamic Coherence: Improves the relationship between transients and sustained sounds\n- Spectral Balancing: Subtle frequency response adjustments for musical cohesiveness\n- Micro-Dynamic Processing: Enhances small-scale dynamics that contribute to analog 'life'\n\n"
                           "AMOUNT CONTROL (0-100%):\nControls the overall intensity of the MLAR processing. Higher values create more pronounced analog character, while lower values provide subtle enhancement. Optimal range is typically 25-75% depending on source material.\n\n"
                           "PROFILE CONTROL:\n- Warm: Emphasizes low-mid harmonics and gentle high-frequency rolloff\n- Bright: Enhances presence and clarity while maintaining warmth\n- Smooth: Focuses on coherence and glue without dramatic tonal changes\n- Punchy: Emphasizes transient definition and rhythmic clarity\n- Custom: User-adjustable profile for specific sonic goals\n\n"
                           "FOCUS CONTROL (20Hz-20kHz):\nFrequency-dependent processing that allows different amounts of MLAR enhancement across the spectrum. Low frequencies can have different processing intensity than high frequencies, enabling precise tonal shaping.\n\n"
                           "BLEND CONTROL (0-100%):\nParallel processing mix between the original dry signal and the MLAR-processed signal. This allows for precise control over the effect intensity while maintaining the original signal's integrity.\n\n"
                           "TECHNICAL SPECIFICATIONS:\n- Processing: 64-bit floating point with 4x internal oversampling\n- Frequency Response: 20Hz-20kHz (+/-0.1dB when bypassed)\n- THD+N: <0.001% at nominal levels\n- Dynamic Range: >120dB\n- Latency: <2ms for real-time monitoring\n- CPU Usage: Optimized for real-time operation\n\n"
                           "PROFESSIONAL APPLICATIONS:\n- Digital Mixing: Add analog cohesiveness to in-the-box productions\n- Mastering: Final polish for digital masters\n- Restoration: Enhance older recordings while preserving authenticity\n- Live Sound: Real-time enhancement for digital mixing consoles\n- Broadcast: Improve perceived quality of digital audio streams\n\n"
                           "USAGE GUIDELINES:\n- Start with Amount at 30-40% and adjust to taste\n- Use Warm profile for intimate, acoustic material\n- Use Bright profile for modern, energetic productions\n- Focus control helps balance processing across frequency ranges\n- Blend control allows for parallel processing techniques";
                } else if (windowTitle.contains("TRANSFORMER")) {
                    return "OVERVIEW:\nTransformer Emulation - Authentic vintage transformer modeling that recreates the sonic characteristics of classic hardware transformers. Each model accurately reproduces the harmonic distortion, frequency response, and saturation behavior of legendary audio equipment.\n\n"
                           "TRANSFORMER MODELS:\n\n"
                           "1. NEVE (Marinair 10468/31267):\n- Used in: Neve 1073, 1081, 1084 preamps and EQs\n- Character: Warm, thick low-mids with smooth high-frequency rolloff\n- Harmonics: Rich even-order harmonics, particularly H2 and H4\n- Frequency Response: Enhanced 60-200Hz, gentle rolloff above 15kHz\n- Best For: Vocals, bass, drums, mix bus warmth\n\n"
                           "2. API (Jensen JE-115K-E):\n- Used in: API 2520/550 series EQs and preamps\n- Character: Punchy, forward sound with excellent transient response\n- Harmonics: Balanced even and odd harmonics with extended highs\n- Frequency Response: Tight low-end, presence peak around 3-5kHz\n- Best For: Drums, guitars, aggressive vocals, rock/pop material\n\n"
                           "3. AMPEX (Peerless 4722):\n- Used in: Ampex 351/354 tape machines\n- Character: Vintage tape machine color with smooth compression\n- Harmonics: Warm even-order saturation, musical compression\n- Frequency Response: Rounded lows, sweet mid-range, soft highs\n- Best For: Mix bus processing, vintage character, mastering\n\n"
                           "4. TELEFUNKEN (BV08 / Haufe):\n- Used in: Telefunken V72/V76 tube preamps\n- Character: Open, transparent with silky high-frequency extension\n- Harmonics: Clean with subtle H2/H3 harmonic structure\n- Frequency Response: Extended highs, neutral mids, tight lows\n- Best For: Classical music, acoustic instruments, mastering\n\n"
                           "5. TRIAD (A-11J / Classic American):\n- Used in: Vintage American consoles and broadcast equipment\n- Character: Clean, neutral with subtle warmth and glue\n- Harmonics: Minimal distortion, gentle saturation at high levels\n- Frequency Response: Flat response with slight mid-range presence\n- Best For: Broadcast, clean tracking, transparent processing\n\n"
                           "DRIVE CONTROL (0-100%):\n- Controls signal level hitting the transformer model\n- Low drive (0-30%): Minimal coloration, subtle warmth\n- Medium drive (30-70%): Noticeable character, harmonic richness\n- High drive (70-100%): Saturated tone, compressed dynamics\n- Adjustable input gain determines saturation intensity\n\n"
                           "HARMONIC BALANCE:\n- Even Harmonics: 2nd, 4th, 6th order (warmth, thickness)\n- Odd Harmonics: 3rd, 5th, 7th order (bite, character)\n- Balance Control: Adjust even/odd harmonic ratio\n- Core Saturation: Models magnetic core non-linearity\n- Bias Control: Simulates DC offset and asymmetric saturation\n\n"
                           "FREQUENCY SHAPING:\n- Low-Frequency Response: Models transformer impedance curves\n- High-Frequency Extension: Simulates winding capacitance\n- Core Mode: Simulates different magnetic core materials\n- HF Control: Adjusts high-frequency character (bright to dark)\n- LF Control: Adjusts low-frequency response (tight to loose)\n\n"
                           "TECHNICAL SPECIFICATIONS:\n- Drive: 0-100% saturation intensity with +/-24dB input gain\n- Models: 5 authentic transformer emulations\n- Harmonic Content: Up to 10th order harmonics modeled\n- THD Range: 0.1% to 15% depending on drive and model\n- Frequency Response: 10Hz to 50kHz modeling bandwidth\n- Dynamic Range: >110dB when bypassed\n- Processing: 64-bit floating point with 4x oversampling\n\n"
                           "PROFESSIONAL APPLICATIONS:\n- Tracking: Add vintage character to individual instruments\n- Mixing: Create cohesiveness with subtle transformer coloration\n- Mastering: Final polish with classic console warmth\n- Re-amping: Add analog character to digital sources\n- Sound Design: Create vintage tones for film and games\n- Broadcast: Achieve classic radio/TV sound signatures\n\n"
                           "CONTROL DESCRIPTIONS:\n- Drive: Input level and saturation intensity\n- Model: Select transformer type (Neve/API/Ampex/Telefunken/Triad)\n- Even/Odd: Balance between harmonic types\n- Mix: Parallel processing blend (dry/wet)";
                }
                return "Professional audio processing information.";
            }
            
            void drawProcessGraphics(juce::Graphics& g, int x, int y, int width, int height)
            {
                g.setColour(juce::Colour(0xFF87ceeb));
                g.setFont(16.0f);
                g.drawText("VISUAL GRAPHICS", x, y, width, 30, juce::Justification::centred);
                
                if (windowTitle.contains("JITTER")) {
                    drawJitterGraphics(g, x, y + 40, width, height - 40);
                } else if (windowTitle.contains("SRC")) {
                    drawSRCGraphics(g, x, y + 40, width, height - 40);
                } else if (windowTitle.contains("ALIGN")) {
                    drawAlignGraphics(g, x, y + 40, width, height - 40);
                } else if (windowTitle.contains("TRANSIENT")) {
                    drawTransientGraphics(g, x, y + 40, width, height - 40);
                } else if (windowTitle.contains("DEESSER")) {
                    drawDeesserGraphics(g, x, y + 40, width, height - 40);
                } else if (windowTitle.contains("MLAR")) {
                    drawMLARGraphics(g, x, y + 40, width, height - 40);
                } else if (windowTitle.contains("TRANSFORMER")) {
                    drawTransformerGraphics(g, x, y + 40, width, height - 40);
                }
            }
            
            void drawJitterGraphics(juce::Graphics& g, int x, int y, int width, int height)
            {
                // Enhanced Jitter Analysis & Digital Audio Optimization
                g.setColour(juce::Colour(0xFF00d4aa));
                g.setFont(14.0f);
                g.drawText("JITTER ANALYSIS & DIGITAL OPTIMIZATION", x, y, width, 25, juce::Justification::centred);
                
                // Section 1: Clock Jitter Comparison (Top)
                g.setColour(juce::Colour(0xFF4a90e2));
                g.setFont(11.0f);
                g.drawText("CLOCK JITTER COMPARISON", x, y + 35, width, 15, juce::Justification::centred);
                
                // Perfect clock
                g.setColour(juce::Colour(0xFF00d4aa));
                g.setFont(9.0f);
                g.drawText("PERFECT CLOCK:", x + 20, y + 55, 120, 12, juce::Justification::left);
                for (int i = 0; i < 12; i++) {
                    g.drawLine(x + 30 + i * 20, y + 70, x + 30 + i * 20, y + 85);
                    g.drawLine(x + 30 + i * 20, y + 85, x + 35 + i * 20, y + 85);
                    g.drawLine(x + 35 + i * 20, y + 85, x + 35 + i * 20, y + 70);
                }
                
                // Jittered clock
                g.setColour(juce::Colour(0xFFff6b35));
                g.drawText("JITTERED CLOCK:", x + 20, y + 95, 120, 12, juce::Justification::left);
                for (int i = 0; i < 12; i++) {
                    float jitter = std::sin(i * 0.6f) * 2.5f; // Realistic jitter
                    g.drawLine(x + 30 + i * 20 + jitter, y + 110, x + 30 + i * 20 + jitter, y + 125);
                    g.drawLine(x + 30 + i * 20 + jitter, y + 125, x + 35 + i * 20 + jitter, y + 125);
                    g.drawLine(x + 35 + i * 20 + jitter, y + 125, x + 35 + i * 20 + jitter, y + 110);
                }
                
                // RMS measurement
                g.setColour(juce::Colours::white);
                g.setFont(8.0f);
                g.drawText("RMS: 125.3 ps | Peak: 287.1 ps", x + 20, y + 140, 200, 10, juce::Justification::left);
                
                // Section 2: Spectral Analysis (Middle Left)
                g.setColour(juce::Colour(0xFFff6b35));
                g.setFont(11.0f);
                g.drawText("JITTER SPECTRUM", x, y + 160, width/2, 15, juce::Justification::centred);
                
                // Frequency spectrum bars
                juce::String freqLabels[] = {"20Hz", "100Hz", "1kHz", "10kHz"};
                for (int i = 0; i < 4; i++) {
                    float amplitude = 35.0f * std::exp(-i * 0.4f) + 8.0f;
                    juce::Colour barColor = juce::Colour::fromHSV(i * 0.2f, 0.7f, 0.9f, 1.0f);
                    g.setColour(barColor);
                    g.fillRect(x + 30 + i * 30, y + 210 - (int)amplitude, 20, (int)amplitude);
                    
                    g.setColour(juce::Colours::white);
                    g.setFont(7.0f);
                    g.drawText(freqLabels[i], x + 30 + i * 30, y + 215, 20, 8, juce::Justification::centred);
                }
                
                // Section 3: Bit Depth Analysis (Middle Right)
                g.setColour(juce::Colour(0xFF87ceeb));
                g.setFont(11.0f);
                g.drawText("BIT DEPTH ANALYSIS", x + width/2, y + 160, width/2, 15, juce::Justification::centred);
                
                // Bit depth comparison
                juce::String bitDepths[] = {"16-bit", "24-bit", "32-bit"};
                juce::String snrValues[] = {"96.3dB", "144.5dB", "192.6dB"};
                juce::Colour bitColors[] = {juce::Colour(0xFF4a90e2), juce::Colour(0xFF00d4aa), juce::Colour(0xFFf7931e)};
                
                for (int i = 0; i < 3; i++) {
                    g.setColour(bitColors[i]);
                    g.fillRect(x + width/2 + 20, y + 180 + i * 18, 40, 14);
                    g.setColour(juce::Colours::white);
                    g.setFont(8.0f);
                    g.drawText(bitDepths[i], x + width/2 + 22, y + 180 + i * 18, 36, 14, juce::Justification::centred);
                    g.drawText(snrValues[i], x + width/2 + 70, y + 180 + i * 18, 50, 14, juce::Justification::left);
                }
                
                // Section 4: Digital Interface Status (Bottom)
                g.setColour(juce::Colour(0xFFf7931e));
                g.setFont(11.0f);
                g.drawText("DIGITAL INTERFACE STATUS", x, y + 240, width, 15, juce::Justification::centred);
                
                // Interface indicators
                juce::String interfaces[] = {"S/PDIF: 89ps", "AES/EBU: 45ps", "USB: 156ps", "Network: 23ps"};
                juce::Colour statusColors[] = {juce::Colour(0xFFffaa00), juce::Colour(0xFF00ff00), juce::Colour(0xFFff6600), juce::Colour(0xFF00ff00)};
                
                for (int i = 0; i < 4; i++) {
                    g.setColour(statusColors[i]);
                    g.fillEllipse(x + 30 + i * 80, y + 265, 8, 8);
                    g.setColour(juce::Colours::white);
                    g.setFont(8.0f);
                    g.drawText(interfaces[i], x + 45 + i * 80, y + 265, 70, 8, juce::Justification::left);
                }
                
                // Overall status
                g.setColour(juce::Colour(0xFF00ff00));
                g.setFont(9.0f);
                g.drawText("SYSTEM LOCKED - OPTIMAL PERFORMANCE", x, y + 285, width, 12, juce::Justification::centred);
            }
            
            void drawSRCGraphics(juce::Graphics& g, int x, int y, int width, int height)
            {
                // Sample Rate Conversion visualization
                g.setColour(juce::Colour(0xFF00d4aa));
                g.setFont(14.0f);
                g.drawText("SAMPLE RATE CONVERSION SYSTEM", x, y, width, 25, juce::Justification::centred);
                
                // Sample rate families
                g.setColour(juce::Colour(0xFF4a90e2));
                g.setFont(11.0f);
                g.drawText("SAMPLE RATE FAMILIES", x, y + 30, width, 15, juce::Justification::centred);
                
                // 44.1kHz family
                g.setColour(juce::Colour(0xFFff6b35));
                g.fillRoundedRectangle(x + 20, y + 50, 120, 45, 3.0f);
                g.setColour(juce::Colours::white);
                g.setFont(10.0f);
                g.drawText("44.1kHz Family", x + 20, y + 52, 120, 12, juce::Justification::centred);
                g.setFont(8.0f);
                g.drawText("CD / Music", x + 20, y + 65, 120, 10, juce::Justification::centred);
                g.drawText("44.1 | 88.2 | 176.4", x + 20, y + 78, 120, 12, juce::Justification::centred);
                
                // 48kHz family
                g.setColour(juce::Colour(0xFF00d4aa));
                g.fillRoundedRectangle(x + 160, y + 50, 140, 45, 3.0f);
                g.setColour(juce::Colours::white);
                g.setFont(10.0f);
                g.drawText("48kHz Family", x + 160, y + 52, 140, 12, juce::Justification::centred);
                g.setFont(8.0f);
                g.drawText("Video / Broadcast", x + 160, y + 65, 140, 10, juce::Justification::centred);
                g.drawText("48 | 96 | 192 | 384", x + 160, y + 78, 140, 12, juce::Justification::centred);
                
                // Conversion examples
                g.setColour(juce::Colour(0xFF87ceeb));
                g.setFont(8.0f);
                g.drawText("Easy: 44.1 -> 88.2 (2x)", x + 20, y + 98, 120, 10, juce::Justification::centred);
                g.drawText("Hard: 44.1 -> 48 (cross)", x + 160, y + 98, 140, 10, juce::Justification::centred);
                
                // Conversion process diagram
                g.setColour(juce::Colour(0xFFf7931e));
                g.setFont(11.0f);
                g.drawText("CONVERSION PROCESS FLOW", x, y + 120, width, 15, juce::Justification::centred);
                
                // Input signal
                g.setColour(juce::Colour(0xFF4a90e2));
                g.fillRoundedRectangle(x + 15, y + 145, 75, 30, 3.0f);
                g.setColour(juce::Colours::white);
                g.setFont(9.0f);
                g.drawText("INPUT", x + 15, y + 148, 75, 12, juce::Justification::centred);
                g.setFont(7.0f);
                g.drawText("44.1kHz", x + 15, y + 161, 75, 10, juce::Justification::centred);
                
                // Arrow 1
                g.setColour(juce::Colour(0xFF87ceeb));
                g.drawLine(x + 95, y + 160, x + 115, y + 160, 2.0f);
                juce::Path arrow1;
                arrow1.startNewSubPath(x + 115, y + 160);
                arrow1.lineTo(x + 108, y + 156);
                arrow1.lineTo(x + 108, y + 164);
                arrow1.closeSubPath();
                g.fillPath(arrow1);
                
                // Interpolate
                g.setColour(juce::Colour(0xFFff6b35));
                g.fillRoundedRectangle(x + 120, y + 145, 70, 30, 3.0f);
                g.setColour(juce::Colours::white);
                g.setFont(8.0f);
                g.drawText("UPSAMPLE", x + 120, y + 148, 70, 12, juce::Justification::centred);
                g.setFont(7.0f);
                g.drawText("Interpolate", x + 120, y + 161, 70, 10, juce::Justification::centred);
                
                // Arrow 2
                g.setColour(juce::Colour(0xFF87ceeb));
                g.drawLine(x + 195, y + 160, x + 215, y + 160, 2.0f);
                juce::Path arrow2;
                arrow2.startNewSubPath(x + 215, y + 160);
                arrow2.lineTo(x + 208, y + 156);
                arrow2.lineTo(x + 208, y + 164);
                arrow2.closeSubPath();
                g.fillPath(arrow2);
                
                // Filter
                g.setColour(juce::Colour(0xFF00d4aa));
                g.fillRoundedRectangle(x + 220, y + 145, 70, 30, 3.0f);
                g.setColour(juce::Colours::white);
                g.setFont(8.0f);
                g.drawText("FILTER", x + 220, y + 148, 70, 12, juce::Justification::centred);
                g.setFont(7.0f);
                g.drawText("Anti-alias", x + 220, y + 161, 70, 10, juce::Justification::centred);
                
                // Arrow 3
                g.setColour(juce::Colour(0xFF87ceeb));
                g.drawLine(x + 295, y + 160, x + 315, y + 160, 2.0f);
                juce::Path arrow3;
                arrow3.startNewSubPath(x + 315, y + 160);
                arrow3.lineTo(x + 308, y + 156);
                arrow3.lineTo(x + 308, y + 164);
                arrow3.closeSubPath();
                g.fillPath(arrow3);
                
                // Output signal
                g.setColour(juce::Colour(0xFF4a90e2));
                g.fillRoundedRectangle(x + 320, y + 145, 75, 30, 3.0f);
                g.setColour(juce::Colours::white);
                g.setFont(9.0f);
                g.drawText("OUTPUT", x + 320, y + 148, 75, 12, juce::Justification::centred);
                g.setFont(7.0f);
                g.drawText("48kHz", x + 320, y + 161, 75, 10, juce::Justification::centred);
                
                // Quality settings
                g.setColour(juce::Colour(0xFFf7931e));
                g.setFont(11.0f);
                g.drawText("QUALITY MODES & PERFORMANCE", x, y + 190, width, 15, juce::Justification::centred);
                
                // Quality bars
                juce::String qualityLevels[] = {"Draft", "Standard", "High", "Ultra"};
                int qualityWidths[] = {60, 100, 140, 180};
                juce::Colour qualityColors[] = {
                    juce::Colour(0xFF666666),
                    juce::Colour(0xFF4a90e2),
                    juce::Colour(0xFF00d4aa),
                    juce::Colour(0xFFff6b35)
                };
                
                for (int i = 0; i < 4; i++) {
                    g.setColour(qualityColors[i]);
                    g.fillRect(x + 30, y + 225 + (i * 25), qualityWidths[i], 18);
                    g.setColour(juce::Colours::white);
                    g.drawText(qualityLevels[i], x + 35, y + 227 + (i * 25), 120, 14, juce::Justification::left);
                }
                
                // Technical info
                g.setColour(juce::Colour(0xFF87ceeb));
                g.setFont(11.0f);
                g.drawText("Stopband: 80dB -> 160dB | Passband: 0.001-0.1dB ripple", x, y + 345, width, 15, juce::Justification::centred);
            }
            
            void drawAlignGraphics(juce::Graphics& g, int x, int y, int width, int height)
            {
                // Phase alignment visualization
                g.setColour(juce::Colour(0xFF00d4aa));
                g.setFont(14.0f);
                g.drawText("PHASE & TIME ALIGNMENT SYSTEM", x, y, width, 20, juce::Justification::centred);
                
                // Section 1: Phase Relationship Comparison
                g.setColour(juce::Colour(0xFF4a90e2));
                g.setFont(10.0f);
                g.drawText("OUT-OF-PHASE (Poor Alignment)", x, y + 25, width/2 - 5, 15, juce::Justification::centred);
                
                // Before alignment waveforms
                for (int i = 0; i < width/2 - 30; i += 2) {
                    float wave1 = std::sin(i * 0.06f) * 18;
                    float wave2 = std::sin((i + 35) * 0.06f) * 14; // Phase offset
                    g.setColour(juce::Colour(0xFF4a90e2));
                    g.drawLine(x + 15 + i, y + 55, x + 15 + i, y + 55 + wave1, 1.5f);
                    g.setColour(juce::Colour(0xFFff6b35));
                    g.drawLine(x + 15 + i, y + 88, x + 15 + i, y + 88 + wave2, 1.5f);
                }
                g.setColour(juce::Colour(0xFFff6b35));
                g.setFont(7.0f);
                g.drawText("Mic 1", x + 15, y + 42, 30, 8, juce::Justification::left);
                g.drawText("Mic 2 (delayed)", x + 15, y + 75, 60, 8, juce::Justification::left);
                g.drawText("Correlation: -0.42", x + 15, y + 110, 80, 10, juce::Justification::left);
                
                // After alignment
                g.setColour(juce::Colour(0xFF00d4aa));
                g.setFont(10.0f);
                g.drawText("IN-PHASE (Aligned)", x + width/2 + 5, y + 25, width/2 - 5, 15, juce::Justification::centred);
                for (int i = 0; i < width/2 - 30; i += 2) {
                    float wave1 = std::sin(i * 0.06f) * 18;
                    float wave2 = std::sin(i * 0.06f) * 14; // Aligned
                    g.setColour(juce::Colour(0xFF4a90e2));
                    g.drawLine(x + width/2 + 15 + i, y + 55, x + width/2 + 15 + i, y + 55 + wave1, 1.5f);
                    g.setColour(juce::Colour(0xFFff6b35));
                    g.drawLine(x + width/2 + 15 + i, y + 88, x + width/2 + 15 + i, y + 88 + wave2, 1.5f);
                }
                g.setColour(juce::Colour(0xFF00ff00));
                g.setFont(7.0f);
                g.drawText("Mic 1", x + width/2 + 15, y + 42, 30, 8, juce::Justification::left);
                g.drawText("Mic 2 (aligned)", x + width/2 + 15, y + 75, 60, 8, juce::Justification::left);
                g.drawText("Correlation: +0.96", x + width/2 + 15, y + 110, 80, 10, juce::Justification::left);
                
                // Section 2: Correlation Meter
                g.setColour(juce::Colour(0xFFf7931e));
                g.setFont(11.0f);
                g.drawText("PHASE CORRELATION METER", x, y + 130, width, 15, juce::Justification::centred);
                
                // Correlation meter scale
                g.setColour(juce::Colour(0xFF666666));
                g.fillRoundedRectangle(x + 40, y + 155, width - 80, 20, 3.0f);
                
                // Gradient from red (-1) through yellow (0) to green (+1)
                for (int i = 0; i < width - 80; i += 2) {
                    float position = (float)i / (width - 80); // 0 to 1
                    float correlation = -1.0f + position * 2.0f; // -1 to +1
                    juce::Colour meterColor;
                    if (correlation < 0) {
                        meterColor = juce::Colour(0xFFff0000).interpolatedWith(juce::Colour(0xFFffff00), (correlation + 1.0f));
                    } else {
                        meterColor = juce::Colour(0xFFffff00).interpolatedWith(juce::Colour(0xFF00ff00), correlation);
                    }
                    g.setColour(meterColor);
                    g.fillRect(x + 40 + i, y + 155, 2, 20);
                }
                
                // Correlation labels
                g.setColour(juce::Colours::white);
                g.setFont(8.0f);
                g.drawText("-1.0", x + 35, y + 178, 30, 10, juce::Justification::right);
                g.drawText("0.0", x + width/2 - 15, y + 178, 30, 10, juce::Justification::centred);
                g.drawText("+1.0", x + width - 65, y + 178, 30, 10, juce::Justification::left);
                g.setFont(7.0f);
                g.drawText("Out-of-Phase", x + 35, y + 190, 60, 8, juce::Justification::left);
                g.drawText("Mono", x + width/2 - 20, y + 190, 40, 8, juce::Justification::centred);
                
                // Section 3: Polarity Correction
                g.setColour(juce::Colour(0xFF87ceeb));
                g.setFont(11.0f);
                g.drawText("POLARITY INVERSION", x, y + 210, width, 15, juce::Justification::centred);
                
                // Normal polarity
                g.setColour(juce::Colour(0xFF4a90e2));
                g.fillRoundedRectangle(x + 40, y + 235, 100, 40, 3.0f);
                g.setColour(juce::Colours::white);
                g.setFont(9.0f);
                g.drawText("NORMAL", x + 40, y + 238, 100, 15, juce::Justification::centred);
                g.setFont(16.0f);
                g.drawText("+", x + 40, y + 252, 100, 20, juce::Justification::centred);
                
                // Polarity arrow
                g.setColour(juce::Colour(0xFFf7931e));
                g.drawLine(x + 150, y + 255, x + 180, y + 255, 2.0f);
                juce::Path arrowPol;
                arrowPol.startNewSubPath(x + 180, y + 255);
                arrowPol.lineTo(x + 173, y + 251);
                arrowPol.lineTo(x + 173, y + 259);
                arrowPol.closeSubPath();
                g.fillPath(arrowPol);
                
                // Inverted polarity
                g.setColour(juce::Colour(0xFFff6b35));
                g.fillRoundedRectangle(x + 190, y + 235, 100, 40, 3.0f);
                g.setColour(juce::Colours::white);
                g.setFont(9.0f);
                g.drawText("INVERTED", x + 190, y + 238, 100, 15, juce::Justification::centred);
                g.setFont(16.0f);
                g.drawText("-", x + 190, y + 252, 100, 20, juce::Justification::centred);
                
                // Section 4: Common Applications
                g.setColour(juce::Colour(0xFF00d4aa));
                g.setFont(10.0f);
                g.drawText("COMMON APPLICATIONS", x, y + 290, width, 15, juce::Justification::centred);
                
                juce::String applications[] = {
                    "Drum Overheads + Close Mics",
                    "Bass DI + Microphone",
                    "Stereo Pair Alignment",
                    "Multi-Camera Audio Sync"
                };
                
                for (int i = 0; i < 4; i++) {
                    g.setColour(juce::Colour(0xFF4a90e2));
                    g.fillEllipse(x + 30 + (i % 2) * 180, y + 310 + (i / 2) * 20, 6, 6);
                    g.setColour(juce::Colours::white);
                    g.setFont(8.0f);
                    g.drawText(applications[i], x + 42 + (i % 2) * 180, y + 308 + (i / 2) * 20, 130, 10, juce::Justification::left);
                }
            }
            
            void drawTransientGraphics(juce::Graphics& g, int x, int y, int width, int height)
            {
                // Transient detection and processing visualization
                g.setColour(juce::Colour(0xFF00d4aa));
                g.drawText("Transient Detection & Enhancement", x, y, width, 25, juce::Justification::centred);
                
                // Original waveform
                g.setColour(juce::Colour(0xFF4a90e2));
                g.drawText("ORIGINAL", x, y + 30, width/2, 15, juce::Justification::centred);
                for (int i = 0; i < width/2 - 20; i += 2) {
                    float wave = std::sin(i * 0.03f) * 15;
                    if (i > 60 && i < 80) wave *= 2.5; // Drum hit transient
                    if (i > 120 && i < 140) wave *= 1.8; // Snare transient
                    g.drawLine(x + 10 + i, y + 70, x + 10 + i, y + 70 + wave);
                }
                
                // Processed waveform
                g.setColour(juce::Colour(0xFFff6b35));
                g.drawText("ENHANCED", x + width/2, y + 30, width/2, 15, juce::Justification::centred);
                for (int i = 0; i < width/2 - 20; i += 2) {
                    float wave = std::sin(i * 0.03f) * 15;
                    if (i > 60 && i < 80) wave *= 4.0; // Enhanced attack
                    if (i > 120 && i < 140) wave *= 3.2; // Enhanced punch
                    g.drawLine(x + width/2 + 10 + i, y + 70, x + width/2 + 10 + i, y + 70 + wave);
                }
                
                // Attack/Sustain separation
                g.setColour(juce::Colour(0xFFf7931e));
                g.drawText("Attack/Sustain Analysis", x, y + 120, width, 20, juce::Justification::centred);
                
                // Attack phase
                g.setColour(juce::Colour(0xFF00d4aa));
                g.fillRect(x + 50, y + 150, 80, 30);
                g.setColour(juce::Colours::white);
                g.drawText("ATTACK", x + 50, y + 155, 80, 20, juce::Justification::centred);
                
                // Sustain phase
                g.setColour(juce::Colour(0xFF4a90e2));
                g.fillRect(x + 150, y + 150, 120, 30);
                g.setColour(juce::Colours::white);
                g.drawText("SUSTAIN", x + 150, y + 155, 120, 20, juce::Justification::centred);
                
                // Control symbols
                g.setColour(juce::Colour(0xFF87ceeb));
                g.drawText("Controls: 🎛️ Sensitivity | 🎛️ Attack | 🎛️ Release | 🎛️ Boost", x, y + 200, width, 20, juce::Justification::centred);
            }
            
            void drawDeesserGraphics(juce::Graphics& g, int x, int y, int width, int height)
            {
                // De-essing frequency analysis and processing
                g.setColour(juce::Colour(0xFF00d4aa));
                g.drawText("Sibilance Detection & Reduction", x, y, width, 25, juce::Justification::centred);
                
                // Frequency spectrum - before processing
                g.setColour(juce::Colour(0xFF4a90e2));
                g.drawText("BEFORE DE-ESSING", x, y + 30, width/2, 15, juce::Justification::centred);
                juce::Path beforeCurve;
                beforeCurve.startNewSubPath(x + 10, y + 80);
                for (int i = 0; i < width/2 - 20; i += 3) {
                    float freq = (float)i / (width/2 - 20);
                    float response = 20 * std::sin(freq * 10) + 30;
                    if (freq > 0.6f && freq < 0.8f) response += 40; // Sibilance peak
                    beforeCurve.lineTo(x + 10 + i, y + 120 - response * 0.5f);
                }
                g.strokePath(beforeCurve, juce::PathStrokeType(2.0f));
                
                // Frequency spectrum - after processing
                g.setColour(juce::Colour(0xFFff6b35));
                g.drawText("AFTER DE-ESSING", x + width/2, y + 30, width/2, 15, juce::Justification::centred);
                juce::Path afterCurve;
                afterCurve.startNewSubPath(x + width/2 + 10, y + 80);
                for (int i = 0; i < width/2 - 20; i += 3) {
                    float freq = (float)i / (width/2 - 20);
                    float response = 20 * std::sin(freq * 10) + 30;
                    if (freq > 0.6f && freq < 0.8f) response -= 15; // Reduced sibilance
                    afterCurve.lineTo(x + width/2 + 10 + i, y + 120 - response * 0.5f);
                }
                g.strokePath(afterCurve, juce::PathStrokeType(2.0f));
                
                // Sibilance detection zone
                g.setColour(juce::Colour(0xFFf7931e));
                g.drawText("Sibilance Detection Zone", x, y + 140, width, 20, juce::Justification::centred);
                g.fillRect(x + width/3, y + 170, width/3, 25);
                g.setColour(juce::Colours::white);
                g.drawText("2kHz - 12kHz", x + width/3, y + 175, width/3, 15, juce::Justification::centred);
                
                // Dynamic threshold
                g.setColour(juce::Colour(0xFF87ceeb));
                g.drawText("Dynamic Threshold: 🎚️ Freq | 🎚️ Q | 🎚️ Thresh | 🎚️ Reduce", x, y + 210, width, 20, juce::Justification::centred);
                
                // Waveform comparison
                g.setColour(juce::Colour(0xFF4a90e2));
                g.drawText("S-Sound Reduction", x, y + 240, width, 15, juce::Justification::centred);
                for (int i = 0; i < width - 40; i += 4) {
                    float wave = std::sin(i * 0.1f) * 10;
                    if (i > width/3 && i < 2*width/3) wave *= 0.3f; // Reduced sibilance
                    g.drawLine(x + 20 + i, y + 270, x + 20 + i, y + 270 + wave);
                }
            }
            
            void drawMLARGraphics(juce::Graphics& g, int x, int y, int width, int height)
            {
                // More Like A Record - Professional Vintage Processing
                g.setColour(juce::Colour(0xFF00d4aa));
                g.setFont(16.0f);
                g.drawText("MORE LIKE A RECORD - Professional Vintage Character Processing", x, y, width, 25, juce::Justification::centred);
                
                // Section 1: Analog Console Modeling (Top Left)
                g.setColour(juce::Colour(0xFF4a90e2));
                g.setFont(12.0f);
                g.drawText("ANALOG CONSOLE MODELING", x, y + 35, width/2 - 10, 15, juce::Justification::centred);
                
                // Console types with specific characteristics
                g.setColour(juce::Colour(0xFF1a2a3a));
                g.fillRoundedRectangle(x + 10, y + 55, width/2 - 20, 120, 5.0f);
                
                // Neve 1073 characteristics
                g.setColour(juce::Colour(0xFF4a90e2));
                g.fillRect(x + 20, y + 65, 80, 12);
                g.setColour(juce::Colours::white);
                g.setFont(9.0f);
                g.drawText("NEVE 1073", x + 20, y + 65, 80, 12, juce::Justification::centred);
                g.setFont(8.0f);
                g.drawText("- Warm midrange saturation", x + 110, y + 65, 120, 12, juce::Justification::left);
                
                // SSL 4000E characteristics  
                g.setColour(juce::Colour(0xFFff6b35));
                g.fillRect(x + 20, y + 85, 80, 12);
                g.setColour(juce::Colours::white);
                g.setFont(9.0f);
                g.drawText("SSL 4000E", x + 20, y + 85, 80, 12, juce::Justification::centred);
                g.setFont(8.0f);
                g.drawText("- Punchy transients & clarity", x + 110, y + 85, 120, 12, juce::Justification::left);
                
                // API 1608 characteristics
                g.setColour(juce::Colour(0xFF00d4aa));
                g.fillRect(x + 20, y + 105, 80, 12);
                g.setColour(juce::Colours::white);
                g.setFont(9.0f);
                g.drawText("API 1608", x + 20, y + 105, 80, 12, juce::Justification::centred);
                g.setFont(8.0f);
                g.drawText("- Musical harmonic distortion", x + 110, y + 105, 120, 12, juce::Justification::left);
                
                // Vintage Tube characteristics
                g.setColour(juce::Colour(0xFFf7931e));
                g.fillRect(x + 20, y + 125, 80, 12);
                g.setColour(juce::Colours::white);
                g.setFont(9.0f);
                g.drawText("TUBE PREAMP", x + 20, y + 125, 80, 12, juce::Justification::centred);
                g.setFont(8.0f);
                g.drawText("- Even harmonic enhancement", x + 110, y + 125, 120, 12, juce::Justification::left);
                
                // Custom Profile
                g.setColour(juce::Colour(0xFF87ceeb));
                g.fillRect(x + 20, y + 145, 80, 12);
                g.setColour(juce::Colours::white);
                g.setFont(9.0f);
                g.drawText("CUSTOM", x + 20, y + 145, 80, 12, juce::Justification::centred);
                g.setFont(8.0f);
                g.drawText("- User-defined character", x + 110, y + 145, 120, 12, juce::Justification::left);
                
                // Section 2: Tape Machine Modeling (Top Right)
                g.setColour(juce::Colour(0xFFff6b35));
                g.setFont(12.0f);
                g.drawText("TAPE MACHINE MODELING", x + width/2 + 10, y + 35, width/2 - 10, 15, juce::Justification::centred);
                
                g.setColour(juce::Colour(0xFF1a2a3a));
                g.fillRoundedRectangle(x + width/2 + 10, y + 55, width/2 - 20, 120, 5.0f);
                
                // Tape characteristics
                g.setColour(juce::Colours::white);
                g.setFont(9.0f);
                g.drawText("STUDER A800", x + width/2 + 20, y + 65, 100, 12, juce::Justification::left);
                g.setFont(8.0f);
                g.drawText("- Smooth high-frequency rolloff", x + width/2 + 20, y + 77, 140, 10, juce::Justification::left);
                
                g.setFont(9.0f);
                g.drawText("AMPEX ATR-102", x + width/2 + 20, y + 95, 100, 12, juce::Justification::left);
                g.setFont(8.0f);
                g.drawText("- Warm low-end compression", x + width/2 + 20, y + 107, 140, 10, juce::Justification::left);
                
                g.setFont(9.0f);
                g.drawText("MCI JH-24", x + width/2 + 20, y + 125, 100, 12, juce::Justification::left);
                g.setFont(8.0f);
                g.drawText("- Punchy midrange presence", x + width/2 + 20, y + 137, 140, 10, juce::Justification::left);
                
                // Frequency response visualization
                juce::Path tapeCurve;
                tapeCurve.startNewSubPath(x + width/2 + 20, y + 155);
                for (int i = 0; i < 120; i += 3) {
                    float freq = (float)i / 120.0f;
                    float response = -freq * freq * 15 + freq * 3; // Tape rolloff curve
                    tapeCurve.lineTo(x + width/2 + 20 + i, y + 155 + response);
                }
                g.setColour(juce::Colour(0xFFff6b35));
                g.strokePath(tapeCurve, juce::PathStrokeType(1.5f));
                
                // Section 3: Stereo Processing (Bottom Left)
                g.setColour(juce::Colour(0xFF87ceeb));
                g.setFont(12.0f);
                g.drawText("STEREO FIELD PROCESSING", x, y + 185, width/2 - 10, 15, juce::Justification::centred);
                
                g.setColour(juce::Colour(0xFF1a2a3a));
                g.fillRoundedRectangle(x + 10, y + 205, width/2 - 20, 100, 5.0f);
                
                // Mid-Side visualization
                g.setColour(juce::Colour(0xFF4a90e2));
                g.drawEllipse(x + 30, y + 220, 60, 30, 2.0f); // Narrow stereo
                g.setColour(juce::Colours::white);
                g.setFont(8.0f);
                g.drawText("BEFORE", x + 30, y + 255, 60, 10, juce::Justification::centred);
                
                // Arrow
                g.setColour(juce::Colour(0xFF00d4aa));
                g.drawArrow(juce::Line<float>(x + 100, y + 235, x + 130, y + 235), 2.0f, 8.0f, 6.0f);
                
                g.setColour(juce::Colour(0xFF00d4aa));
                g.drawEllipse(x + 140, y + 215, 80, 40, 2.0f); // Wide stereo
                g.setColour(juce::Colours::white);
                g.setFont(8.0f);
                g.drawText("AFTER", x + 140, y + 260, 80, 10, juce::Justification::centred);
                
                // Focus control visualization
                g.setColour(juce::Colours::white);
                g.setFont(9.0f);
                g.drawText("FOCUS CONTROL:", x + 20, y + 275, 100, 12, juce::Justification::left);
                g.setFont(8.0f);
                g.drawText("- Frequency-dependent width", x + 20, y + 287, 140, 10, juce::Justification::left);
                
                // Section 4: Harmonic Enhancement (Bottom Right)
                g.setColour(juce::Colour(0xFFf7931e));
                g.setFont(12.0f);
                g.drawText("HARMONIC ENHANCEMENT", x + width/2 + 10, y + 185, width/2 - 10, 15, juce::Justification::centred);
                
                g.setColour(juce::Colour(0xFF1a2a3a));
                g.fillRoundedRectangle(x + width/2 + 10, y + 205, width/2 - 20, 100, 5.0f);
                
                // Harmonic spectrum bars
                g.setColour(juce::Colours::white);
                g.setFont(8.0f);
                g.drawText("HARMONIC SPECTRUM:", x + width/2 + 20, y + 215, 120, 10, juce::Justification::left);
                
                // Draw harmonic bars with specific values
                for (int i = 1; i <= 6; i++) {
                    float amplitude = 40.0f / (i * 0.8f); // More realistic harmonic distribution
                    juce::Colour harmColor = (i % 2 == 0) ? juce::Colour(0xFF4a90e2) : juce::Colour(0xFFff6b35);
                    g.setColour(harmColor.withAlpha(0.8f));
                    g.fillRect(x + width/2 + 20 + i * 18, y + 265 - (int)amplitude, 12, (int)amplitude);
                    
                    g.setColour(juce::Colours::white);
                    g.setFont(7.0f);
                    g.drawText(juce::String(i), x + width/2 + 22 + i * 18, y + 270, 8, 8, juce::Justification::centred);
                }
                
                // Blend control info
                g.setColour(juce::Colours::white);
                g.setFont(8.0f);
                g.drawText("BLEND: Dry/Wet mix (0-100%)", x + width/2 + 20, y + 285, 140, 10, juce::Justification::left);
                
                // Control descriptions at bottom
                g.setColour(juce::Colour(0xFF87ceeb));
                g.setFont(10.0f);
                g.drawText("Controls: Amount (Intensity) | Profile (Character) | Focus (Frequency Balance) | Blend (Dry/Wet Mix)", x, y + 320, width, 15, juce::Justification::centred);
            }
            
            void drawTransformerGraphics(juce::Graphics& g, int x, int y, int width, int height)
            {
                // Classic Transformer Models visualization
                g.setColour(juce::Colour(0xFF00d4aa));
                g.drawText("Classic Transformer Models & Characteristics", x, y, width, 25, juce::Justification::centred);
                
                // Model boxes with details
                g.setColour(juce::Colour(0xFF4a90e2));
                g.drawText("TRANSFORMER MODELS", x, y + 35, width, 18, juce::Justification::centred);
                
                // NEVE Model
                g.setColour(juce::Colour(0xFF4a90e2));
                g.fillRoundedRectangle(x + 15, y + 60, 75, 55, 3.0f);
                g.setColour(juce::Colours::white);
                g.setFont(12.0f);
                g.drawText("NEVE", x + 15, y + 63, 75, 15, juce::Justification::centred);
                g.setFont(9.0f);
                g.drawText("Marinair 10468", x + 15, y + 78, 75, 12, juce::Justification::centred);
                g.drawText("Warm Low-Mids", x + 15, y + 90, 75, 10, juce::Justification::centred);
                g.drawText("Rich H2/H4", x + 15, y + 101, 75, 10, juce::Justification::centred);
                
                // API Model
                g.setColour(juce::Colour(0xFFff6b35));
                g.fillRoundedRectangle(x + 100, y + 60, 75, 55, 3.0f);
                g.setColour(juce::Colours::white);
                g.setFont(12.0f);
                g.drawText("API", x + 100, y + 63, 75, 15, juce::Justification::centred);
                g.setFont(9.0f);
                g.drawText("Jensen JE-115K", x + 100, y + 78, 75, 12, juce::Justification::centred);
                g.drawText("Punchy Forward", x + 100, y + 90, 75, 10, juce::Justification::centred);
                g.drawText("Presence Peak", x + 100, y + 101, 75, 10, juce::Justification::centred);
                
                // AMPEX Model
                g.setColour(juce::Colour(0xFFf7931e));
                g.fillRoundedRectangle(x + 185, y + 60, 75, 55, 3.0f);
                g.setColour(juce::Colours::white);
                g.setFont(12.0f);
                g.drawText("AMPEX", x + 185, y + 63, 75, 15, juce::Justification::centred);
                g.setFont(9.0f);
                g.drawText("Peerless 4722", x + 185, y + 78, 75, 12, juce::Justification::centred);
                g.drawText("Tape Warmth", x + 185, y + 90, 75, 10, juce::Justification::centred);
                g.drawText("Sweet Mids", x + 185, y + 101, 75, 10, juce::Justification::centred);
                
                // TELEFUNKEN Model
                g.setColour(juce::Colour(0xFF00d4aa));
                g.fillRoundedRectangle(x + 270, y + 60, 75, 55, 3.0f);
                g.setColour(juce::Colours::white);
                g.setFont(12.0f);
                g.drawText("TELEFUNKEN", x + 270, y + 63, 75, 15, juce::Justification::centred);
                g.setFont(9.0f);
                g.drawText("BV08 / Haufe", x + 270, y + 78, 75, 12, juce::Justification::centred);
                g.drawText("Open & Clean", x + 270, y + 90, 75, 10, juce::Justification::centred);
                g.drawText("Silky Highs", x + 270, y + 101, 75, 10, juce::Justification::centred);
                
                // TRIAD Model
                g.setColour(juce::Colour(0xFF87ceeb));
                g.fillRoundedRectangle(x + 355, y + 60, 75, 55, 3.0f);
                g.setColour(juce::Colours::white);
                g.setFont(12.0f);
                g.drawText("TRIAD", x + 355, y + 63, 75, 15, juce::Justification::centred);
                g.setFont(9.0f);
                g.drawText("A-11J American", x + 355, y + 78, 75, 12, juce::Justification::centred);
                g.drawText("Clean Neutral", x + 355, y + 90, 75, 10, juce::Justification::centred);
                g.drawText("Subtle Glue", x + 355, y + 101, 75, 10, juce::Justification::centred);
                
                // Harmonic content visualization
                g.setColour(juce::Colour(0xFFf7931e));
                g.setFont(14.0f);
                g.drawText("HARMONIC CONTENT", x, y + 130, width, 18, juce::Justification::centred);
                
                // Original signal
                g.setColour(juce::Colour(0xFF666666));
                g.setFont(11.0f);
                g.drawText("CLEAN", x, y + 155, width/2, 15, juce::Justification::centred);
                g.setColour(juce::Colour(0xFF4a90e2));
                g.fillRect(x + 50, y + 175, 20, 50); // Fundamental only
                g.setColour(juce::Colours::white);
                g.setFont(8.0f);
                g.drawText("F", x + 52, y + 230, 16, 10, juce::Justification::centred);
                
                // Processed signal with harmonics
                g.setColour(juce::Colour(0xFFff6b35));
                g.setFont(11.0f);
                g.drawText("WITH TRANSFORMER", x + width/2, y + 155, width/2, 15, juce::Justification::centred);
                juce::String harmonicLabels[] = {"F", "H2", "H3", "H4", "H5", "H6"};
                for (int i = 0; i < 6; i++) {
                    float amplitude = 50.0f * std::pow(0.5f, i); // Exponential decay
                    g.setColour(juce::Colour(0xFFff6b35).withAlpha(1.0f - i * 0.12f));
                    g.fillRect(x + width/2 + 25 + i * 22, (int)(y + 225 - amplitude), 16, (int)amplitude);
                    g.setColour(juce::Colours::white);
                    g.setFont(8.0f);
                    g.drawText(harmonicLabels[i], x + width/2 + 25 + i * 22, y + 230, 16, 10, juce::Justification::centred);
                }
                
                // Frequency response curves
                g.setColour(juce::Colour(0xFF87ceeb));
                g.setFont(14.0f);
                g.drawText("FREQUENCY RESPONSE", x, y + 255, width, 18, juce::Justification::centred);
                
                // Draw different transformer response curves
                juce::Path neveCurve, apiCurve, telefunkenCurve;
                float baseY = y + 305;
                neveCurve.startNewSubPath(x + 30, baseY);
                apiCurve.startNewSubPath(x + 30, baseY);
                telefunkenCurve.startNewSubPath(x + 30, baseY);
                
                for (int i = 0; i < width - 60; i += 4) {
                    float freq = (float)i / (width - 60);
                    // Neve: Enhanced 60-200Hz, rolloff above 15kHz
                    float neveResponse = (freq < 0.15f ? 3.0f * std::sin(freq * 10.0f) : 0.0f) 
                                       + (freq > 0.85f ? -6.0f * (freq - 0.85f) : 0.0f);
                    neveCurve.lineTo(x + 30 + i, baseY - neveResponse);
                    
                    // API: Tight low-end, presence peak 3-5kHz
                    float apiResponse = (freq > 0.3f && freq < 0.5f ? 5.0f * std::sin((freq - 0.3f) * 15.7f) : 0.0f);
                    apiCurve.lineTo(x + 30 + i, baseY - apiResponse);
                    
                    // Telefunken: Extended highs, neutral mids
                    float telefunkenResponse = freq > 0.7f ? 4.0f * (freq - 0.7f) : 0.0f;
                    telefunkenCurve.lineTo(x + 30 + i, baseY - telefunkenResponse);
                }
                
                g.setColour(juce::Colour(0xFF4a90e2));
                g.strokePath(neveCurve, juce::PathStrokeType(2.0f));
                g.setColour(juce::Colour(0xFFff6b35));
                g.strokePath(apiCurve, juce::PathStrokeType(2.0f));
                g.setColour(juce::Colour(0xFF00d4aa));
                g.strokePath(telefunkenCurve, juce::PathStrokeType(2.0f));
                
                // Labels with characteristics
                g.setFont(9.0f);
                g.setColour(juce::Colour(0xFF4a90e2));
                g.drawText("NEVE: Warm Lows + Soft Highs", x + 30, y + 280, 140, 12, juce::Justification::left);
                g.setColour(juce::Colour(0xFFff6b35));
                g.drawText("API: Presence Bump", x + 30, y + 293, 120, 12, juce::Justification::left);
                g.setColour(juce::Colour(0xFF00d4aa));
                g.drawText("TELEFUNKEN: Extended Air", x + 30, y + 306, 140, 12, juce::Justification::left);
                
                // Frequency markers
                g.setColour(juce::Colour(0xFF666666));
                g.setFont(8.0f);
                g.drawText("20Hz", x + 30, y + 320, 40, 10, juce::Justification::left);
                g.drawText("1kHz", x + width/2 - 15, y + 320, 40, 10, juce::Justification::centred);
                g.drawText("20kHz", x + width - 70, y + 320, 40, 10, juce::Justification::right);
                
                // Control info
                g.setColour(juce::Colour(0xFF87ceeb));
                g.setFont(11.0f);
                g.drawText("Controls: Drive (Saturation) | Model (Type) | Even/Odd (Harmonics) | Mix (Blend)", x, y + 345, width, 15, juce::Justification::centred);
            }
        };
    private:
        juce::String processName;
        std::unique_ptr<juce::ComboBox> presetCombo;
        std::unique_ptr<CleanKnob> knobs[4];
        std::unique_ptr<juce::TextButton> infoButton;
        float meterValues[4] = {0.5f, 0.3f, 0.7f, 0.4f}; // Default meter values
    };
    
    using DeesserControlWindow = TransientControlWindow;
    using MLARControlWindow = TransientControlWindow;
    using TransformerControlWindow = TransientControlWindow;
    
    switch (processIndex)
    {
        case 0: // ISP
        {
            auto* controlWindow = new ISPControlWindow("ISP - INTERSAMPLE PROCESSING", 
                "Reduces aliasing, pre-ringing, and intersample distortion");
            
            juce::DialogWindow::LaunchOptions opts;
            opts.content.setOwned(controlWindow);
            opts.dialogTitle = processName + " - Professional Controls";
            opts.componentToCentreAround = this;
            opts.escapeKeyTriggersCloseButton = true;
            opts.useNativeTitleBar = true;
            opts.resizable = true;
            opts.launchAsync();
            break;
        }
        case 1: // SRC
        {
            auto* controlWindow = new SRCControlWindow("SRC - SAMPLE RATE CONVERSION", 
                "High-quality sample rate conversion with advanced filtering");
            
            juce::DialogWindow::LaunchOptions opts;
            opts.content.setOwned(controlWindow);
            opts.dialogTitle = processName + " - Professional Controls";
            opts.componentToCentreAround = this;
            opts.escapeKeyTriggersCloseButton = true;
            opts.useNativeTitleBar = true;
            opts.resizable = true;
            opts.launchAsync();
            break;
        }
        case 2: // JITTER
        {
            auto* controlWindow = new JitterControlWindow("JITTER - JITTER & ACCUMULATION", 
                "Advanced jitter analysis and bit-depth management");
            
            juce::DialogWindow::LaunchOptions opts;
            opts.content.setOwned(controlWindow);
            opts.dialogTitle = processName + " - Professional Controls";
            opts.componentToCentreAround = this;
            opts.escapeKeyTriggersCloseButton = true;
            opts.useNativeTitleBar = true;
            opts.resizable = true;
            opts.launchAsync();
            break;
        }
        case 3: // ALIGN
        {
            auto* controlWindow = new AlignControlWindow("ALIGN - PHASE/TIME ALIGNMENT", 
                "Precision phase and time alignment for stereo imaging");
            
            juce::DialogWindow::LaunchOptions opts;
            opts.content.setOwned(controlWindow);
            opts.dialogTitle = processName + " - Professional Controls";
            opts.componentToCentreAround = this;
            opts.escapeKeyTriggersCloseButton = true;
            opts.useNativeTitleBar = true;
            opts.resizable = true;
            opts.launchAsync();
            break;
        }
        case 4: // TRANSIENT
        {
            auto* controlWindow = new TransientControlWindow("TRANSIENT - TRANSIENT PROCESSING", 
                "Advanced transient detection and enhancement");
            
            juce::DialogWindow::LaunchOptions opts;
            opts.content.setOwned(controlWindow);
            opts.dialogTitle = processName + " - Professional Controls";
            opts.componentToCentreAround = this;
            opts.escapeKeyTriggersCloseButton = true;
            opts.useNativeTitleBar = true;
            opts.resizable = true;
            opts.launchAsync();
            break;
        }
        case 5: // DEESSER
        {
            auto* controlWindow = new DeesserControlWindow("DEESSER - DE-ESSING", 
                "Intelligent sibilance detection and reduction");
            
            juce::DialogWindow::LaunchOptions opts;
            opts.content.setOwned(controlWindow);
            opts.dialogTitle = processName + " - Professional Controls";
            opts.componentToCentreAround = this;
            opts.escapeKeyTriggersCloseButton = true;
            opts.useNativeTitleBar = true;
            opts.resizable = true;
            opts.launchAsync();
            break;
        }
        case 6: // MLAR
        {
            auto* controlWindow = new MLARControlWindow("MLAR - MID-LATERAL RECONSTRUCTION", 
                "Advanced stereo field processing and imaging");
            
            juce::DialogWindow::LaunchOptions opts;
            opts.content.setOwned(controlWindow);
            opts.dialogTitle = processName + " - Professional Controls";
            opts.componentToCentreAround = this;
            opts.escapeKeyTriggersCloseButton = true;
            opts.useNativeTitleBar = true;
            opts.resizable = true;
            opts.launchAsync();
            break;
        }
        case 7: // TRANSFORMER
        {
            auto* controlWindow = new TransformerControlWindow("TRANSFORMER - TRANSFORMER MODELING", 
                "Vintage transformer saturation and harmonic enhancement");
            
            juce::DialogWindow::LaunchOptions opts;
            opts.content.setOwned(controlWindow);
            opts.dialogTitle = processName + " - Professional Controls";
            opts.componentToCentreAround = this;
            opts.escapeKeyTriggersCloseButton = true;
            opts.useNativeTitleBar = true;
            opts.resizable = true;
            opts.launchAsync();
            break;
        }
        default:
            juce::AlertWindow::showMessageBoxAsync(juce::AlertWindow::InfoIcon, 
                "Unknown Process", 
                "Process index out of range.", 
                "OK");
            return;
    }
    
    proguiLog("[UI] Opened " + processName + " professional subwindow");
}
