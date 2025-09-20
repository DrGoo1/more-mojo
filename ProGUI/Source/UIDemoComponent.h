#pragma once
#include "../../JUCE/modules/juce_core/juce_core.h"
#include "../../JUCE/modules/juce_events/juce_events.h"
#include "../../JUCE/modules/juce_graphics/juce_graphics.h"
#include "../../JUCE/modules/juce_gui_basics/juce_gui_basics.h"
#include "../../JUCE/modules/juce_gui_extra/juce_gui_extra.h"
#include "../shared/ui_core/Theme.h"
#include "../shared/ui_core/Knob.h"
#include "../shared/ui_core/Slider.h"
#include "../shared/ui_core/Primitives.h"
#include "../shared/ui_core/PillToggle.h"

class UIDemoComponent : public juce::Component, private juce::Timer
{
public:
    UIDemoComponent()
    {
        // Knob
        addAndMakeVisible(demoKnob);
        demoKnob.setRange(0.0, 100.0, 0.1);
        demoKnob.onValueChange = [this]{ level = (float) (demoKnob.getValue() / 100.0); repaint(); };

        // Slider
        addAndMakeVisible(demoSlider);
        demoSlider.setRange(0.0, 100.0, 0.1);
        demoSlider.onValueChange = [this]{ level2 = (float) (demoSlider.getValue() / 100.0); repaint(); };

        // Button
        demoButton.setButtonText("Button");
        addAndMakeVisible(demoButton);

        // Switch
        demoSwitch.setButtonText("Switch");
        demoSwitch.setClickingTogglesState(true);
        addAndMakeVisible(demoSwitch);

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
                repaint();
            });
        };
        addAndMakeVisible(loadSkinButton);

        startTimerHz(30);
        // Ensure initial layout even if no resize event yet
        resized();
    }

    void paint(juce::Graphics& g) override
    {
        g.fillAll(ui::theme().bg);
        // Bold diagnostic stripe background so painting is undeniable
        {
            auto b = getLocalBounds().toFloat();
            juce::Path p;
            const float stripeW = 20.f;
            for (float x = -b.getHeight(); x < b.getWidth() + b.getHeight(); x += stripeW * 2.f)
            {
                juce::Path stripe;
                stripe.startNewSubPath(x, b.getY());
                stripe.lineTo(x + b.getHeight(), b.getBottom());
                stripe.lineTo(x + b.getHeight() + stripeW, b.getBottom());
                stripe.lineTo(x + stripeW, b.getY());
                stripe.closeSubPath();
                p.addPath(stripe);
            }
            g.setColour(juce::Colours::red.withAlpha(0.12f));
            g.fillPath(p);
        }

        // Large visible header to guarantee visibility during diagnostics
        auto topBanner = getLocalBounds().removeFromTop(40);
        g.setColour(juce::Colours::white.withAlpha(0.9f));
        g.setFont(juce::Font(20.0f, juce::Font::bold));
        g.drawFittedText("[Demo] UIDemoComponent Content Area", topBanner, juce::Justification::centred, 1);

        // Visible guides for control area
        auto full = getLocalBounds();
        auto controlArea = full.reduced(16).removeFromLeft( getWidth() - 176 );
        g.setColour(juce::Colours::green.withAlpha(0.4f));
        g.drawRect(controlArea, 2);

        // Draw minimal modern-looking controls directly
        juce::Rectangle<int> knobR (32, 64, 160, 160);
        juce::Rectangle<int> sldR  (32, 240, juce::jmax(220, getWidth() - 200), 26);
        juce::Rectangle<int> btnR  (32, 280, 120, 28);
        juce::Rectangle<int> togR  (168, 280, 120, 28);

        // Knob (filled circle with arc)
        {
            auto rf = knobR.toFloat();
            auto c = rf.getCentre();
            float rad = juce::jmin(rf.getWidth(), rf.getHeight()) * 0.5f;
            g.setColour(ui::theme().panel);
            g.fillEllipse(rf);
            g.setColour(juce::Colours::black.withAlpha(0.5f));
            g.drawEllipse(rf, 1.5f);
            float start = juce::MathConstants<float>::pi * 1.2f;
            float end   = juce::MathConstants<float>::pi * (1.2f + 1.2f);
            float ang   = start + (end - start) * juce::jlimit(0.0f, 1.0f, level);
            juce::Path arc; arc.addCentredArc(c.x, c.y, rad-12.f, rad-12.f, 0, start, ang, true);
            g.setColour(ui::theme().accent);
            g.strokePath(arc, juce::PathStrokeType(3.0f));
        }

        // Slider (track + fill)
        {
            auto rs = sldR.toFloat();
            auto track = juce::Rectangle<float>(rs.getX(), rs.getCentreY() - 4.f, rs.getWidth(), 8.f);
            g.setColour(ui::theme().panel.darker(0.15f)); g.fillRoundedRectangle(track, 4.f);
            auto fill = track.withWidth(track.getWidth() * juce::jlimit(0.f, 1.f, level2));
            g.setColour(ui::theme().accent); g.fillRoundedRectangle(fill, 4.f);
        }

        // Button
        {
            auto bf = btnR.toFloat(); g.setColour(ui::theme().card);
            g.fillRoundedRectangle(bf, 6.f);
            g.setColour(juce::Colours::white.withAlpha(0.85f));
            g.drawFittedText("Button", btnR, juce::Justification::centred, 1);
        }

        // Toggle (pill)
        {
            auto tf = togR.toFloat();
            g.setColour(ui::theme().card); g.fillRoundedRectangle(tf, tf.getHeight()/2.f);
            bool on = level2 > 0.5f; g.setColour(on ? ui::theme().accent : ui::theme().panel);
            auto knob = juce::Rectangle<float>(tf.getX()+ (on ? tf.getWidth()-tf.getHeight() : 0.f), tf.getY(), tf.getHeight(), tf.getHeight());
            g.fillEllipse(knob);
            g.setColour(juce::Colours::white.withAlpha(0.85f));
            g.drawFittedText(on ? "On" : "Off", togR, juce::Justification::centred, 1);
        }
        juce::Rectangle<int> skinR (304, 280, 140, 28);
        g.drawText("[Load Skin]", skinR, juce::Justification::centred);

        // Meter frame
        auto meterArea = getLocalBounds().reduced(16).removeFromRight(140);
        drawMeter(g, meterArea, level, ui::theme().accent);
        auto meterArea2 = meterArea.withTrimmedTop( meterArea.getHeight()/2 + 8 );
        drawMeter(g, meterArea2, level2, ui::theme().accent2);

        // Header
        g.setColour(ui::theme().text);
        g.setFont(ui::theme().title);
        g.drawFittedText("UI Demo — Knob, Slider, Button, Switch, Meter", getLocalBounds().reduced(16).removeFromTop(24), juce::Justification::left, 1);

        // Diagnostics banner to verify rendering
        auto diag = getLocalBounds().reduced(12).removeFromBottom(36);
        g.setColour(juce::Colours::black.withAlpha(0.35f));
        g.fillRoundedRectangle(diag.toFloat(), 6.0f);
        g.setColour(juce::Colours::yellow);
        g.setFont(juce::Font(14.0f, juce::Font::bold));
        g.drawFittedText("[Diagnostics] UIDemoComponent is rendering", diag, juce::Justification::centred, 1);
        g.setColour(juce::Colours::white.withAlpha(0.25f));
        g.drawRoundedRectangle(diag.toFloat(), 6.0f, 1.0f);
    }

    void resized() override
    {
        auto bounds = getLocalBounds();
        // Absolute positions to guarantee visibility
        demoKnob.setBounds(32, 64, 160, 160);
        demoSlider.setBounds(32, 240, juce::jmax(200, bounds.getWidth() - 200), 26);
        demoButton.setBounds(32, 280, 120, 28);
        demoSwitch.setBounds(168, 280, 120, 28);
        loadSkinButton.setBounds(304, 280, 140, 28);
        // meters painted on right side in paint()

        // skin preview area next to meters (paint)
    }

private:
    ui::Knob demoKnob;
    ui::Slider demoSlider;
    juce::TextButton demoButton;
    ui::PillToggle demoSwitch;
    juce::TextButton loadSkinButton;
    juce::Image skinImage;

    float level {0.25f};
    float level2 {0.75f};

    void timerCallback() override { repaint(); }

    static void drawMeter(juce::Graphics& g, juce::Rectangle<int> area, float value, juce::Colour col)
    {
        auto r = area.toFloat();
        // background card
        ui::Card card; card.setBounds(area); // not added; just reuse painting style
        // emulate card paint
        juce::DropShadow (juce::Colours::black.withAlpha(0.5f), 8, {0,4}).drawForRectangle(g, area);
        auto top = ui::theme().card.brighter(0.07f);
        auto bot = ui::theme().card.darker(0.10f);
        g.setGradientFill(juce::ColourGradient(top, r.getCentreX(), r.getY(), bot, r.getCentreX(), r.getBottom(), false));
        g.fillRoundedRectangle(r, ui::theme().r_md);
        g.setColour(juce::Colours::white.withAlpha(0.06f));
        g.drawRoundedRectangle(r, ui::theme().r_md, 1.5f);

        // bar
        auto inner = r.reduced(12);
        auto bar = inner.withHeight(20);
        g.setColour(ui::theme().panel.darker(0.2f));
        g.fillRoundedRectangle(bar, 10.0f);
        auto fill = bar.withWidth(bar.getWidth() * juce::jlimit(0.0f, 1.0f, value));
        g.setColour(col);
        g.fillRoundedRectangle(fill, 10.0f);

        // skin preview (if any)
        if (area.getBottom() + 8 <= g.getClipBounds().getBottom()) {
            // draw skin on the right side above
        }
    }
};
