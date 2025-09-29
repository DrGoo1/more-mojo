#pragma once
#include <juce_gui_basics/juce_gui_basics.h>
#include "../../shared/ui_core/AmountKnobWithRing.h"
#include "../../shared/ui_core/SkinnedKnob.h"
#include <vector>

class ProMasterComponent : public juce::Component {
public:
    struct ProcessRow {
        juce::String name;
        std::unique_ptr<SkinnedKnob> amount;
        std::unique_ptr<juce::TextButton>  openBtn;
    };

    ProMasterComponent()
    {
        static const char* kProcessNames[] = {
            "ISP", "SRC", "Quant/Dither", "Align",
            "Transient", "De-esser", "MLAR", "Transformer"
        };
        for (auto* n : kProcessNames)
        {
            ProcessRow row;
            row.name   = n;
            row.amount = std::make_unique<SkinnedKnob>();
            row.openBtn= std::make_unique<juce::TextButton>("Open");
            addAndMakeVisible(*row.amount);
            addAndMakeVisible(*row.openBtn);
            rows.push_back(std::move(row));
        }
    }

    std::function<void(const juce::String&)> onOpenProcess;

    void paint(juce::Graphics& g) override
    {
        auto b = getLocalBounds().toFloat();
        g.setColour(juce::Colour(0xFF101419));
        g.fillRoundedRectangle(b, 8.0f);
        g.setColour(juce::Colours::white.withAlpha(0.08f));
        g.drawRoundedRectangle(b, 8.0f, 1.0f);

        // Titles
        g.setColour(juce::Colours::white.withAlpha(0.85f));
        g.setFont(16.0f);
        g.drawFittedText("Pro — Master Processes", getLocalBounds().removeFromTop(26), juce::Justification::centred, 1);
    }

    void resized() override
    {
        auto r = getLocalBounds().reduced(12);
        r.removeFromTop(28);
        const int rowH = 88;
        const int knobD = 72; // medium size knob
        for (int i = 0; i < (int)rows.size(); ++i)
        {
            auto rr = r.removeFromTop(rowH).reduced(6);
            // Label at left
            juce::Rectangle<int> nameR = rr.removeFromLeft(160);
            names.ensureStorageAllocated((int)rows.size());
            if (names.size() <= i) names.add({});
            names.getReference(i) = nameR;

            // Knob next
            auto knobR = rr.removeFromLeft(knobD);
            if (auto* a = rows[i].amount.get()) a->setBounds(knobR.withSizeKeepingCentre(knobD, knobD));

            rr.removeFromLeft(12);
            auto openW = 80;
            auto openR = rr.removeFromRight(openW);
            if (auto* b = rows[i].openBtn.get())
            {
                b->setBounds(openR.removeFromTop(28).withCentre(rr.getCentre()));
                b->onClick = [this, i]{ if (onOpenProcess) onOpenProcess(rows[i].name); };
            }

            r.removeFromTop(6);
        }
    }

    void paintOverChildren(juce::Graphics& g) override
    {
        g.setColour(juce::Colours::white.withAlpha(0.8f));
        g.setFont(14.0f);
        for (int i = 0; i < (int)rows.size(); ++i)
            g.drawFittedText(rows[i].name, names[i], juce::Justification::centredLeft, 1);
    }

private:
    std::vector<ProcessRow> rows;
    juce::Array<juce::Rectangle<int>> names;
};
