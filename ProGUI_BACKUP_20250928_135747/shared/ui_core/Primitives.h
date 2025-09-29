#pragma once
#include "../../JUCE/modules/juce_graphics/juce_graphics.h"
#include "../../JUCE/modules/juce_gui_basics/juce_gui_basics.h"
#include "Theme.h"

namespace ui {

struct Card : public juce::Component {
    void paint(juce::Graphics& g) override {
        auto r = getLocalBounds().toFloat();
        juce::DropShadow (juce::Colours::black.withAlpha(0.6f), (int) theme().elev_md, {0, 4}).drawForRectangle (g, r.toNearestInt());
        auto top = theme().card.brighter(0.07f);
        auto bot = theme().card.darker(0.10f);
        g.setGradientFill (juce::ColourGradient (top, r.getCentreX(), r.getY(), bot, r.getCentreX(), r.getBottom(), false));
        g.fillRoundedRectangle (r, theme().r_md);
        g.setColour (juce::Colours::white.withAlpha(0.06f));
        g.drawRoundedRectangle (r, theme().r_md, 1.5f);
    }
};

struct Panel : public juce::Component {
    void paint(juce::Graphics& g) override {
        auto r = getLocalBounds().toFloat();
        auto top = theme().panel.brighter(0.05f);
        auto bot = theme().panel.darker(0.05f);
        g.setGradientFill (juce::ColourGradient (top, r.getX(), r.getY(), bot, r.getX(), r.getBottom(), false));
        g.fillRoundedRectangle (r, theme().r_sm);
        g.setColour (juce::Colours::white.withAlpha(0.05f));
        g.drawRoundedRectangle (r, theme().r_sm, 1.0f);
    }
};

struct GroupHeader : public juce::Component {
    juce::String text;
    explicit GroupHeader (juce::String t) : text (std::move(t)) {}
    void paint(juce::Graphics& g) override {
        g.setColour (theme().text);
        g.setFont (theme().title);
        g.drawFittedText (text, getLocalBounds().withTrimmedLeft(4), juce::Justification::left, 1);
    }
};

struct IconButton : public juce::Button {
    explicit IconButton (juce::String label) : juce::Button (label) { setTooltip (label); }
    void paintButton (juce::Graphics& g, bool over, bool down) override {
        auto r = getLocalBounds().toFloat().reduced (2);
        auto base = theme().panel;
        if (down) base = base.darker(0.15f); else if (over) base = base.brighter(0.08f);
        g.setColour (base);
        g.fillRoundedRectangle (r, theme().r_sm);
        g.setColour (theme().text);
        g.setFont (theme().body);
        g.drawFittedText (getButtonText(), getLocalBounds().reduced(6), juce::Justification::centred, 1);
    }
};

} // namespace ui
