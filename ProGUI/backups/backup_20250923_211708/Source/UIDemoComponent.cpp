#include "UIDemoComponent.h"
#include "../shared/ui_core/ProcessSubwindow.h"
#include "Components/ISPWindow.h"
#include "Components/SRCWindow.h"
#include "Components/QuantDitherWindow.h"
#include "Components/AlignWindow.h"
#include "Components/TransientWindow.h"
#include "Components/DeEsserWindow.h"
#include "Components/MLARWindow.h"
#include "Components/TransformerWindow.h"
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
                sub = new TransientWindow();
                static_cast<ProcessSubwindow*>(sub)->setInfoText(processInfoText(name));
            }
            else if (name == "De-esser")
            {
                sub = new DeEsserWindow();
                static_cast<ProcessSubwindow*>(sub)->setInfoText(processInfoText(name));
            }
            else if (name == "MLAR")
            {
                sub = new MLARWindow();
                static_cast<ProcessSubwindow*>(sub)->setInfoText(processInfoText(name));
            }
            else if (name == "Transformer")
            {
                sub = new TransformerWindow();
                static_cast<ProcessSubwindow*>(sub)->setInfoText(processInfoText(name));
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
    
    // Set knobs to different positions to show Neptune frames clearly
    inputTrim.setValue(0.3f);   // 30% - frame ~77/257
    outputTrim.setValue(0.7f);  // 70% - frame ~180/257
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
    // Top-right status: readable pill (moved higher to avoid title overlap)
    {
        auto top = this->getLocalBounds().reduced(8).removeFromTop(20);
        auto right = top.removeFromRight(400).reduced(4, 0);
        const auto& sk = SkinManager::instance().skin();
        juce::String s = "🎛️ NEPTUNE UI KIT ACTIVE • " + juce::String((int)sk.knobFrames.size()) + " frames loaded";
        juce::Colour pill = juce::Colours::orange.withAlpha(0.8f);
        g.setColour(pill);
        g.fillRoundedRectangle(right.toFloat(), 4.0f);
        g.setColour(juce::Colours::black);
        g.setFont(juce::Font(12.0f, juce::Font::bold));
        g.drawFittedText(s, right.reduced(8, 2), juce::Justification::centredRight, 1);
    }

#if 0 // PROGUI_VECTOR_FALLBACK - DISABLED to prevent overlap with Neptune knobs
    // Draw minimal modern-looking controls directly (fallback visuals)
    juce::Rectangle<int> knobR (32, 64, 160, 160);
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
        }
    }

    // Toggle (use skin filmstrip frames when available)
    {
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
        
        auto workArea = centerPanel.reduced(0, 30);
        int wfH = juce::jlimit(120, 160, workArea.getHeight() - 40);
        workArea.removeFromTop(wfH); // skip waveform area
        workArea.removeFromTop(30);  // skip transport area
        
        // Apply same margin reduction to prevent knob overlap
        workArea = workArea.reduced(10, 0);
        
        auto remainingArea = workArea.reduced(8);
        if (remainingArea.getHeight() > 100)
        {
            // Draw Pro Master section with gradient background
            auto bg = juce::ColourGradient(juce::Colour(0xFF1a1a2e), remainingArea.getX(), remainingArea.getY(),
                                         juce::Colour(0xFF16213e), remainingArea.getX(), remainingArea.getBottom(), false);
            g.setGradientFill(bg);
            g.fillRoundedRectangle(remainingArea.toFloat(), 12.0f);
            
            // Stylish border
            g.setColour(juce::Colour(0xFF0f3460).withAlpha(0.8f));
            g.drawRoundedRectangle(remainingArea.toFloat(), 12.0f, 2.0f);
            
            // Header with 3D effect
            auto headerArea = remainingArea.removeFromTop(45);
            // 3D shadow
            g.setColour(juce::Colour(0xFF000000).withAlpha(0.8f));
            g.setFont(juce::Font("Arial Black", 18.0f, juce::Font::bold));
            g.drawText("PROFESSIONAL MOJO CONTROLS", headerArea.translated(2, 2), juce::Justification::centred);
            // Main text with gradient effect
            g.setColour(juce::Colour(0xFFffffff));
            g.drawText("PROFESSIONAL MOJO CONTROLS", headerArea, juce::Justification::centred);
            
            // Draw process rows with Neptune knobs and Open buttons
            const char* processes[] = {"IMAGE SIGNAL PROCESSING", "SAMPLE RATE CONVERSION", "PHASE ALIGNMENT", "TRANSIENT SHAPING", "DE-ESSING", "MORE LIKE A RECORD"};
            const char* descriptions[] = {"Restores Digital Warmth", "Eliminates Digital Harshness", "Creates Spatial Coherence", "Adds Punch & Presence", "Removes Harsh Frequencies", "Vintage Analog Character"};
            const char* details[] = {"Adds the missing harmonics and subtle distortion that makes digital audio feel alive and musical", "Removes the cold, sterile quality of digital conversion with smooth, musical resampling", "Aligns phase relationships to create the wide, immersive soundstage of great recordings", "Enhances attack and sustain to make instruments cut through the mix with natural dynamics", "Intelligently removes sibilance while preserving the natural brightness that gives vocals their sparkle", "Applies the subtle compression, saturation, and frequency response that made classic records so compelling"};
            int rowH = 48;
            
            // Create a 2x3 grid layout for processes
            int cols = 2, rows = 3;
            int cellW = (remainingArea.getWidth() - 20) / cols;
            int cellH = (remainingArea.getHeight() - 10) / rows;
            
            for (int i = 0; i < 6; ++i)
            {
                int col = i % cols;
                int row = i / cols;
                auto cellArea = juce::Rectangle<int>(
                    remainingArea.getX() + col * cellW + 10,
                    remainingArea.getY() + row * cellH + 5,
                    cellW - 10, cellH - 5
                );
                
                // Neptune knob (left side) - LARGER
                auto knobArea = cellArea.removeFromLeft(70).reduced(5);
                const auto& skin = SkinManager::instance().skin();
                if (!skin.knobFrames.empty())
                {
                    int frameIdx = (i * 43) % (int)skin.knobFrames.size();
                    auto img = skin.knobFrames[frameIdx];
                    float scale = juce::jmin(knobArea.getWidth() / (float)img.getWidth(), 
                                           knobArea.getHeight() / (float)img.getHeight()) * 0.85f;
                    auto dest = juce::Rectangle<float>(img.getWidth() * scale, img.getHeight() * scale)
                                  .withCentre(knobArea.getCentre().toFloat());
                    g.drawImage(img, dest);
                }
                
                // Wider Open Controls button (right side) to fit text
                auto btnArea = cellArea.removeFromRight(80).reduced(3, 8);
                // 3D button with metallic effect
                auto btnGrad = juce::ColourGradient(juce::Colour(0xFF4a90e2), btnArea.getX(), btnArea.getY(),
                                                  juce::Colour(0xFF2171b5), btnArea.getX(), btnArea.getBottom(), false);
                g.setGradientFill(btnGrad);
                g.fillRoundedRectangle(btnArea.toFloat(), 6.0f);
                // Button highlight
                g.setColour(juce::Colour(0xFFffffff).withAlpha(0.3f));
                g.fillRoundedRectangle(btnArea.removeFromTop(btnArea.getHeight()/2).toFloat(), 6.0f);
                // Button border
                g.setColour(juce::Colour(0xFF1a5490));
                g.drawRoundedRectangle(btnArea.toFloat(), 6.0f, 1.5f);
                // Button text - fit to wider button
                g.setColour(juce::Colours::white);
                g.setFont(juce::Font("Arial", 8.0f, juce::Font::bold));
                g.drawText("OPEN CONTROLS", btnArea, juce::Justification::centred);
                
                // Process labels (center area)
                auto labelArea = cellArea.reduced(5, 2);
                
                // Process name with clean, larger text
                auto nameRect = labelArea.removeFromTop(22);
                // Simple shadow for depth
                g.setColour(juce::Colour(0xFF000000).withAlpha(0.8f));
                g.setFont(juce::Font("Arial", 16.0f, juce::Font::bold));
                g.drawText(processes[i], nameRect.translated(1, 1), juce::Justification::centredLeft);
                // Main text - bright and clear
                g.setColour(juce::Colour(0xFFffffff));
                g.drawText(processes[i], nameRect, juce::Justification::centredLeft);
                
                // Description with glow effect
                g.setColour(juce::Colour(0xFF00d4aa).withAlpha(0.3f));
                g.setFont(juce::Font("Arial", 11.0f, juce::Font::bold));
                auto descRect = labelArea.removeFromTop(14);
                g.drawText(descriptions[i], descRect.translated(1, 1), juce::Justification::centredLeft);
                g.setColour(juce::Colour(0xFF00d4aa));
                g.drawText(descriptions[i], descRect, juce::Justification::centredLeft);
                
                // Horizontal level bar
                auto levelArea = labelArea.removeFromTop(6).reduced(0, 1);
                g.setColour(juce::Colour(0xFF333333));
                g.fillRoundedRectangle(levelArea.toFloat(), 2.0f);
                // Active level (different for each process)
                float level = 0.3f + (i * 0.1f);
                auto activeLevelArea = levelArea.removeFromLeft((int)(levelArea.getWidth() * level));
                g.setColour(juce::Colour(0xFF00d4aa));
                g.fillRoundedRectangle(activeLevelArea.toFloat(), 2.0f);
                
                // Technical details
                g.setColour(juce::Colour(0xFFb0b0b0));
                g.setFont(juce::Font("Arial", 8.0f, juce::Font::plain));
                g.drawText(details[i], labelArea, juce::Justification::centredLeft);
            }
        }
    }

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
    
    auto outer = bounds.reduced(16);
    const int colW = 160; // maximum wide columns for Neptune knobs
    const int gap = 8;
    proguiLog("[DEBUG] Using colW=" + juce::String(colW) + " gap=" + juce::String(gap));
    auto leftCol  = outer.removeFromLeft(colW);
    outer.removeFromLeft(gap);
    auto rightCol = outer.removeFromRight(colW);
    outer.removeFromRight(gap);
    auto centerPanel = outer;

    // Leave space at top for status banner, then place waveform and transport
    auto workArea = centerPanel.reduced(0, 30); // leave 30px at top for status banner
    int wfH = juce::jlimit(120, 160, workArea.getHeight() - 60);
    auto waveformArea  = workArea.removeFromTop(wfH);
    auto transportArea = workArea.removeFromTop(50); // transport BELOW waveform
    
    // Reduce remaining area to prevent overlap with side knobs
    workArea = workArea.reduced(10, 0); // smaller margin since columns are wider now
    
    if (waveform) waveform->setBounds(waveformArea);
    
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

    // Place LED meters at top and IO trims at bottom of side columns
    auto leftArea  = leftCol.reduced(3);
    auto rightArea = rightCol.reduced(3);
    const int knobD = 80; // much larger knobs for better Neptune visibility
    
    // Leave space at top for bypass button and status banner
    auto leftAreaWithMargin = leftArea.withTrimmedTop(40);
    auto rightAreaWithMargin = rightArea.withTrimmedTop(40);
    
    // Ensure areas have minimum size
    if (leftAreaWithMargin.getWidth() > 0 && leftAreaWithMargin.getHeight() > knobD + 24)
    {
        // Left column: meter top, Input knob bottom
        auto leftKnobArea = leftAreaWithMargin.removeFromBottom(knobD + 24);
        lblIn.setBounds(leftKnobArea.removeFromTop(20));
        inputTrim.setBounds(leftKnobArea.withSizeKeepingCentre(knobD, knobD));
        // Keep meter narrow (38px) even with wider column
        if (ledMeter1) {
            auto meterArea = leftAreaWithMargin.withWidth(38).withX(leftAreaWithMargin.getCentreX() - 19);
            ledMeter1->setBounds(meterArea);
        }
    }
    
    if (rightAreaWithMargin.getWidth() > 0 && rightAreaWithMargin.getHeight() > knobD + 24)
    {
        // Right column: meter top, Output knob bottom
        auto rightKnobArea = rightAreaWithMargin.removeFromBottom(knobD + 24);
        lblOut.setBounds(rightKnobArea.removeFromTop(20));
        outputTrim.setBounds(rightKnobArea.withSizeKeepingCentre(knobD, knobD));
        // Keep meter narrow (38px) even with wider column
        if (ledMeter2) {
            auto meterArea = rightAreaWithMargin.withWidth(38).withX(rightAreaWithMargin.getCentreX() - 19);
            ledMeter2->setBounds(meterArea);
        }
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
    
    // Force repaint to ensure visual changes appear
    this->repaint();
    if (ledMeter1) ledMeter1->repaint();
    if (ledMeter2) ledMeter2->repaint();
    if (waveform) waveform->repaint();
    if (transport) transport->repaint();
}

void UIDemoComponent::timerCallback()
{
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
