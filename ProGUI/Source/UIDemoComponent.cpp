#include "../shared/ui_core/ProcessSubwindow.h"
#include "Components/ISPWindow.h"
#include "Components/SRCWindow.h"
#include "Components/QuantDitherWindow.h"
#include "Components/AlignWindow.h"
#include "UIDemoComponent.h"
#include "Components/NeptuneVUMeter.h"
#include "Components/NeptuneKnob.h"
#include "Components/NeptuneMixKnob.h"
#include "Components/HorizontalBarMeter.h"
#include "Components/ProcessControl.h"
#include "Components/NeptuneMixKnob.h"
#include "Components/NeptuneKnob.h"
#include "Components/NeptuneISPSubwindow.h"
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
            juce::Component* sub = nullptr;
            if (name == "ISP")
            {
                sub = new ISPWindow();
                static_cast<ProcessSubwindow*>(sub)->setInfoText(processInfoText(name));
            }
            else if (name == "SRC")
            {
                sub = new SRCWindow();
                static_cast<ProcessSubwindow*>(sub)->setInfoText(processInfoText(name));
            }
            else if (name == "Quant/Dither")
            {
                sub = new QuantDitherWindow();
                static_cast<ProcessSubwindow*>(sub)->setInfoText(processInfoText(name));
            }
            else if (name == "Align")
            {
                sub = new AlignWindow();
                static_cast<ProcessSubwindow*>(sub)->setInfoText(processInfoText(name));
            }
            else if (name == "Transient")
            {
                // sub = new TransientWindow(); // Temporarily disabled
                auto* generic = new ProcessSubwindow(name);
                generic->setInfoText(processInfoText(name));
                sub = generic;
            }
            else if (name == "De-esser")
            {
                // sub = new DeEsserWindow(); // Temporarily disabled
                auto* generic = new ProcessSubwindow(name);
                generic->setInfoText(processInfoText(name));
                sub = generic;
            }
            else if (name == "MLAR")
            {
                // sub = new MLARWindow(); // Temporarily disabled
                auto* generic = new ProcessSubwindow(name);
                generic->setInfoText(processInfoText(name));
                sub = generic;
            }
            else if (name == "Transformer")
            {
                // sub = new TransformerWindow(); // Temporarily disabled
                auto* generic = new ProcessSubwindow(name);
                generic->setInfoText(processInfoText(name));
                sub = generic;
            }
            else
            {
                auto* generic = new ProcessSubwindow(name);
                generic->setInfoText(processInfoText(name));
                sub = generic;
            }

            juce::DialogWindow::LaunchOptions subOpts;
            subOpts.content.setOwned(sub);
            subOpts.dialogTitle = name + " — Details";
            subOpts.componentToCentreAround = this;
            subOpts.escapeKeyTriggersCloseButton = true;
            subOpts.useNativeTitleBar = true;
            subOpts.resizable = true;
            subOpts.launchAsync();
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
        auto* comp = new StealMojoComponent();
        juce::DialogWindow::LaunchOptions opts;
        opts.content.setOwned(comp);
        opts.dialogTitle = "Steal The Mojo";
        opts.componentToCentreAround = this;
        opts.escapeKeyTriggersCloseButton = true;
        opts.useNativeTitleBar = true;
        opts.resizable = false;
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
    
    // Hide all bottom buttons for clean interface
    btnProMaster.setBounds(0, 0, 0, 0); // hidden
    btnStealMojo.setBounds(0, 0, 0, 0); // hidden
    
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
    // Click detection for "OPEN CONTROLS" buttons
    auto clickPos = event.getPosition();
    proguiLog("[DEBUG] Mouse click at: " + juce::String(clickPos.x) + ", " + juce::String(clickPos.y));
    
    // Define button bounds for the 8 processes (2×4 grid)
    struct ProcessButton {
        juce::String name;
        juce::Rectangle<int> bounds;
        int index;
    };
    
    // Button bounds based on the Professional Mojo Controls layout (2×4 grid for all 8 processes)
    ProcessButton buttons[] = {
        {"ISP", juce::Rectangle<int>(318, 274, 104, 65), 0},
        {"SRC", juce::Rectangle<int>(669, 274, 104, 65), 1}, 
        {"JITTER", juce::Rectangle<int>(318, 339, 104, 65), 2},
        {"ALIGN", juce::Rectangle<int>(669, 339, 104, 65), 3},
        {"TRANSIENT", juce::Rectangle<int>(318, 404, 104, 65), 4},
        {"DEESSER", juce::Rectangle<int>(669, 404, 104, 65), 5},
        {"MLAR", juce::Rectangle<int>(318, 469, 104, 65), 6},
        {"TRANSFORMER", juce::Rectangle<int>(669, 469, 104, 65), 7}
    };
    
    // Check if click is within any button bounds
    for (auto& button : buttons)
    {
        proguiLog("[DEBUG] " + button.name + " button bounds: " + juce::String(button.bounds.getX()) + " " + 
                 juce::String(button.bounds.getY()) + " " + juce::String(button.bounds.getWidth()) + " " + 
                 juce::String(button.bounds.getHeight()));
                 
        if (button.bounds.contains(clickPos))
        {
            proguiLog("[UI] Opening " + button.name + " subwindow!");
            openProcessSubwindow(button.index);
            return;
        }
    }
}

void UIDemoComponent::openProcessSubwindow(int processIndex)
{
    juce::String processNames[] = {"ISP", "SRC", "JITTER", "ALIGN", "TRANSIENT", "DEESSER", "MLAR", "TRANSFORMER"};
    
    if (processIndex < 0 || processIndex >= 8)
        return;
        
    juce::String processName = processNames[processIndex];
    
    // Create Neptune subwindows
    if (processName == "ISP")
    {
        // Create simple Neptune ISP subwindow
        auto* ispWindow = new NeptuneISPSubwindow();
        
        juce::DialogWindow::LaunchOptions opts;
        opts.content.setOwned(ispWindow);
        opts.dialogTitle = "ISP - Intersample Processing";
        opts.componentToCentreAround = this;
        opts.escapeKeyTriggersCloseButton = true;
        opts.useNativeTitleBar = true;
        opts.resizable = false;
        opts.launchAsync();
        
        proguiLog("[ISP] Opened Neptune ISP subwindow");
        return;
    }
    
    // For other processes, show placeholder
    if (processIndex >= 0 && processIndex < 8)
    {
        // Create the complete working ISP subwindow based on the individual files
        class WorkingISPSubwindow : public juce::Component
        {
        public:
            WorkingISPSubwindow()
            {
                setSize(900, 700);
                createISPControls();
                proguiLog("[ISP] Created working ISP subwindow with complete structure");
            }
            
            void createISPControls()
            {
                // Sample Rate dropdown (renamed from OS Factor)
                juce::StringArray sampleRates = {"44.1 kHz", "48 kHz", "88.2 kHz", "96 kHz", "176.4 kHz", "192 kHz"};
                sampleRateCombo = std::make_unique<juce::ComboBox>("Sample Rate");
                sampleRateCombo->addItemList(sampleRates, 1);
                sampleRateCombo->setSelectedItemIndex(1); // Default to 48 kHz
                addAndMakeVisible(*sampleRateCombo);
                
                // Filter Type dropdown
                juce::StringArray filterTypes = {"Linear Phase", "Minimum Phase", "Polyphase"};
                filterTypeCombo = std::make_unique<juce::ComboBox>("Filter Type");
                filterTypeCombo->addItemList(filterTypes, 1);
                filterTypeCombo->setSelectedItemIndex(2); // Default to Polyphase
                addAndMakeVisible(*filterTypeCombo);
                
                // Control knobs (Neptune style without rings)
                passbandRolloffKnob = createKnobWithMeter("Passband Rolloff", 0.0f, 3.0f, 0.5f);
                stopbandAttenKnob = createKnobWithMeter("Stopband Atten", 60.0f, 120.0f, 90.0f);
                tpCeilingKnob = createKnobWithMeter("TP Ceiling", -6.0f, 0.0f, -1.0f);
                lookaheadKnob = createKnobWithMeter("Lookahead", 0.0f, 10.0f, 2.0f);
                
                // Create OPEN buttons for each control
                createOpenButtons();
            }
            
            std::unique_ptr<SkinnedKnob> createKnobWithMeter(const juce::String& name, float min, float max, float defaultVal)
            {
                auto knob = std::make_unique<SkinnedKnob>();
                // SkinnedKnob uses normalized 0.0-1.0 range, so normalize the default value
                float normalizedValue = (defaultVal - min) / (max - min);
                knob->setValue(juce::jlimit(0.0f, 1.0f, normalizedValue));
                knob->setName(name);
                addAndMakeVisible(*knob);
                return knob;
            }
            
            void createOpenButtons()
            {
                // Create OPEN buttons for each control with info windows
                sampleRateOpenButton = std::make_unique<juce::TextButton>("OPEN");
                sampleRateOpenButton->onClick = [this]() { openControlInfo("Sample Rate"); };
                addAndMakeVisible(*sampleRateOpenButton);
                
                filterTypeOpenButton = std::make_unique<juce::TextButton>("OPEN");
                filterTypeOpenButton->onClick = [this]() { openControlInfo("Filter Type"); };
                addAndMakeVisible(*filterTypeOpenButton);
                
                passbandOpenButton = std::make_unique<juce::TextButton>("OPEN");
                passbandOpenButton->onClick = [this]() { openControlInfo("Passband Rolloff"); };
                addAndMakeVisible(*passbandOpenButton);
                
                stopbandOpenButton = std::make_unique<juce::TextButton>("OPEN");
                stopbandOpenButton->onClick = [this]() { openControlInfo("Stopband Atten"); };
                addAndMakeVisible(*stopbandOpenButton);
                
                tpCeilingOpenButton = std::make_unique<juce::TextButton>("OPEN");
                tpCeilingOpenButton->onClick = [this]() { openControlInfo("TP Ceiling"); };
                addAndMakeVisible(*tpCeilingOpenButton);
                
                lookaheadOpenButton = std::make_unique<juce::TextButton>("OPEN");
                lookaheadOpenButton->onClick = [this]() { openControlInfo("Lookahead"); };
                addAndMakeVisible(*lookaheadOpenButton);
            }
            
            void openControlInfo(const juce::String& controlName)
            {
                // Create info window for each control
                juce::String infoText;
                if (controlName == "Sample Rate")
                    infoText = "Sample Rate determines the frequency at which audio samples are captured and processed. Higher sample rates provide better frequency response but require more processing power.";
                else if (controlName == "Filter Type")
                    infoText = "Filter Type determines the phase characteristics of the anti-aliasing filter. Linear Phase maintains phase relationships, Minimum Phase reduces latency, and Polyphase provides optimal balance.";
                else if (controlName == "Passband Rolloff")
                    infoText = "Passband Rolloff controls how gradually the filter transitions from the passband to the stopband. Lower values provide sharper cutoff but may introduce ringing artifacts.";
                else if (controlName == "Stopband Atten")
                    infoText = "Stopband Attenuation determines how much unwanted frequencies above the Nyquist limit are suppressed. Higher values provide better alias rejection.";
                else if (controlName == "TP Ceiling")
                    infoText = "True-Peak Ceiling sets the maximum allowed peak level to prevent intersample peaks that could cause digital clipping during D/A conversion.";
                else if (controlName == "Lookahead")
                    infoText = "Lookahead Time allows the limiter to anticipate upcoming peaks and apply smoother gain reduction, reducing distortion at the cost of latency.";
                
                // Simple info dialog
                juce::AlertWindow::showMessageBoxAsync(juce::AlertWindow::InfoIcon, 
                    controlName + " - Information", infoText, "OK");
            }
            
            void paint(juce::Graphics& g) override
            {
                // Professional background matching main ProGUI style
                g.fillAll(juce::Colour(0xFF1a1a2e));
                
                // Header
                g.setColour(juce::Colours::white);
                g.setFont(juce::Font(18.0f, juce::Font::bold));
                g.drawText("INTERSAMPLE PROCESSING", 20, 20, getWidth() - 40, 30, juce::Justification::centred);
                
                // Description
                g.setColour(juce::Colour(0xFF00ffff));
                g.setFont(juce::Font(12.0f, juce::Font::plain));
                g.drawText("Reduces aliasing, pre-ringing, and intersample distortion for cleaner reconstruction", 
                          20, 50, getWidth() - 40, 30, juce::Justification::centred, true);
                
                // Draw control sections with labels and meters (like I/O meters)
                drawControlSection(g, "Sample Rate", 30, 100, 0);
                drawControlSection(g, "Filter Type", 30, 160, 1);
                drawControlSection(g, "Passband Rolloff", 30, 220, 2);
                drawControlSection(g, "Stopband Atten", 30, 280, 3);
                drawControlSection(g, "TP Ceiling", 30, 340, 4);
                drawControlSection(g, "Lookahead", 30, 400, 5);
            }
            
            void drawControlSection(juce::Graphics& g, const juce::String& name, int x, int y, int index)
            {
                // Control name
                g.setColour(juce::Colours::white);
                g.setFont(juce::Font(14.0f, juce::Font::bold));
                g.drawText(name, x, y, 150, 20, juce::Justification::left);
                
                // Meter bar (like I/O meters in main window)
                auto meterArea = juce::Rectangle<int>(x + 160, y, 200, 20);
                g.setColour(juce::Colour(0xFF333333));
                g.fillRoundedRectangle(meterArea.toFloat(), 2.0f);
                
                // Animated meter level for each control
                float level = 0.2f + 0.6f * std::sin(juce::Time::getMillisecondCounter() * 0.002f + index * 0.5f);
                auto activeArea = meterArea.removeFromLeft((int)(meterArea.getWidth() * level));
                
                // Different colors for different controls
                juce::Array<juce::Colour> colors = {
                    juce::Colour(0xFF00ff00), // Sample Rate - green
                    juce::Colour(0xFF0080ff), // Filter Type - blue
                    juce::Colour(0xFFff8000), // Passband - orange
                    juce::Colour(0xFFff0080), // Stopband - pink
                    juce::Colour(0xFFff4040), // TP Ceiling - red
                    juce::Colour(0xFF8040ff)  // Lookahead - purple
                };
                
                g.setColour(colors[index % colors.size()]);
                g.fillRoundedRectangle(activeArea.toFloat(), 2.0f);
                
                // Value display
                g.setColour(juce::Colours::white.withAlpha(0.8f));
                g.setFont(juce::Font(10.0f, juce::Font::plain));
                g.drawText(juce::String((int)(level * 100)) + "%", meterArea.getX() + meterArea.getWidth() + 10, y, 40, 20, juce::Justification::left);
            }
            
            void resized() override
            {
                int controlY = 100;
                int controlSpacing = 60;
                int knobX = 420;
                int openButtonX = 520;
                
                // Layout Sample Rate control
                if (sampleRateCombo) sampleRateCombo->setBounds(knobX, controlY, 80, 25);
                if (sampleRateOpenButton) sampleRateOpenButton->setBounds(openButtonX, controlY, 50, 25);
                controlY += controlSpacing;
                
                // Layout Filter Type control
                if (filterTypeCombo) filterTypeCombo->setBounds(knobX, controlY, 80, 25);
                if (filterTypeOpenButton) filterTypeOpenButton->setBounds(openButtonX, controlY, 50, 25);
                controlY += controlSpacing;
                
                // Layout knob controls with OPEN buttons
                if (passbandRolloffKnob) passbandRolloffKnob->setBounds(knobX, controlY, 60, 60);
                if (passbandOpenButton) passbandOpenButton->setBounds(openButtonX, controlY + 15, 50, 25);
                controlY += controlSpacing;
                
                if (stopbandAttenKnob) stopbandAttenKnob->setBounds(knobX, controlY, 60, 60);
                if (stopbandOpenButton) stopbandOpenButton->setBounds(openButtonX, controlY + 15, 50, 25);
                controlY += controlSpacing;
                
                if (tpCeilingKnob) tpCeilingKnob->setBounds(knobX, controlY, 60, 60);
                if (tpCeilingOpenButton) tpCeilingOpenButton->setBounds(openButtonX, controlY + 15, 50, 25);
                controlY += controlSpacing;
                
                if (lookaheadKnob) lookaheadKnob->setBounds(knobX, controlY, 60, 60);
                if (lookaheadOpenButton) lookaheadOpenButton->setBounds(openButtonX, controlY + 15, 50, 25);
            }
            
        private:
            // Controls
            std::unique_ptr<juce::ComboBox> sampleRateCombo;
            std::unique_ptr<juce::ComboBox> filterTypeCombo;
            std::unique_ptr<SkinnedKnob> passbandRolloffKnob;
            std::unique_ptr<SkinnedKnob> stopbandAttenKnob;
            std::unique_ptr<SkinnedKnob> tpCeilingKnob;
            std::unique_ptr<SkinnedKnob> lookaheadKnob;
            
            // OPEN buttons for each control
            std::unique_ptr<juce::TextButton> sampleRateOpenButton;
            std::unique_ptr<juce::TextButton> filterTypeOpenButton;
            std::unique_ptr<juce::TextButton> passbandOpenButton;
            std::unique_ptr<juce::TextButton> stopbandOpenButton;
            std::unique_ptr<juce::TextButton> tpCeilingOpenButton;
            std::unique_ptr<juce::TextButton> lookaheadOpenButton;
        };
        
        auto* ispWindow = new WorkingISPSubwindow();
        
        juce::DialogWindow::LaunchOptions opts;
        opts.content.setOwned(ispWindow);
        opts.dialogTitle = "ISP — Professional Controls";
        opts.componentToCentreAround = this;
        opts.escapeKeyTriggersCloseButton = true;
        opts.useNativeTitleBar = true;
        opts.resizable = true;
        opts.launchAsync();
        
        proguiLog("[UI] Opened working ISP subwindow with complete professional structure");
    }
    else if (processName == "JITTER")
    {
        // Create JITTER & ACCUMULATION subwindow
        juce::AlertWindow::showMessageBoxAsync(juce::AlertWindow::InfoIcon, 
            "JITTER & ACCUMULATION", 
            "Jitter & Accumulation Controls:\n\n"
            "• Jitter RMS - Controls random timing variations\n"
            "• Jitter Spectrum - Frequency distribution of jitter\n"
            "• Bit Depth - Digital resolution (16-32 bits)\n"
            "• Quantization - Digital word length processing\n"
            "• Dither - Noise shaping for low-level signals\n\n"
            "This subwindow will be implemented with full Neptune controls.", 
            "OK");
    }
    else if (processName == "TRANSFORMER")
    {
        // Create TRANSFORMER subwindow
        juce::AlertWindow::showMessageBoxAsync(juce::AlertWindow::InfoIcon, 
            "TRANSFORMER", 
            "Transformer Controls:\n\n"
            "• Drive - Input saturation level\n"
            "• Even/Odd Balance - Harmonic content control\n"
            "• Bias - DC offset for asymmetric saturation\n"
            "• Core Mode - Transformer core characteristics\n"
            "• HF/LF Controls - Frequency-dependent processing\n"
            "• Mix - Blend between processed and dry signal\n\n"
            "This subwindow will be implemented with full Neptune controls.", 
            "OK");
    }
    else
    {
        proguiLog("[UI] " + processName + " subwindow not yet implemented");
    }
}

// Mouse drag and up methods removed - knobs positioned correctly
