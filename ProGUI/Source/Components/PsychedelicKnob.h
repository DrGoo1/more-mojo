#pragma once
#include <juce_gui_basics/juce_gui_basics.h>
#include "PsychedelicStyles.h"

class PsychedelicKnob : public juce::Component {
public:
    PsychedelicKnob() {
        setSize(120, 120);
    }
    
    void paint(juce::Graphics& g) override {
        auto area = getLocalBounds().toFloat();
        Effects3D::drawPsychedelicKnob(g, area, rotation, amount);
        
        // Draw percentage text in center
        g.setColour(juce::Colours::white);
        g.setFont(juce::Font(20.0f, juce::Font::bold));
        auto text = juce::String(static_cast<int>(amount * 100)) + "%";
        g.drawText(text, getLocalBounds(), juce::Justification::centred);
    }
    
    void mouseDown(const juce::MouseEvent& e) override {
        isDragging = true;
        lastY = e.y;
    }
    
    void mouseDrag(const juce::MouseEvent& e) override {
        if (!isDragging) return;
        
        auto delta = (lastY - e.y) * 0.005f;
        lastY = e.y;
        
        amount = juce::jlimit(0.0f, 1.0f, amount + delta);
        rotation = -2.356f + (4.712f * amount); // -135° to +135°
        
        if (onAmountChanged)
            onAmountChanged(amount);
        
        repaint();
    }
    
    void mouseUp(const juce::MouseEvent&) override {
        isDragging = false;
    }
    
    void setAmount(float newAmount) {
        amount = juce::jlimit(0.0f, 1.0f, newAmount);
        rotation = -2.356f + (4.712f * amount);
        repaint();
    }
    
    float getAmount() const { return amount; }
    
    std::function<void(float)> onAmountChanged;
    
private:
    float amount = 0.5f;
    float rotation = 0.0f;
    bool isDragging = false;
    int lastY = 0;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PsychedelicKnob)
};
