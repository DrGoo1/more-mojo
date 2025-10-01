#pragma once
#include <juce_gui_basics/juce_gui_basics.h>

// Psychedelic color palette inspired by Peter Max, Yellow Submarine, Austin Powers
namespace PsychedelicColors {
    inline const juce::Colour electricPink   (0xFFFF1493);
    inline const juce::Colour psychPurple    (0xFF9B30FF);
    inline const juce::Colour cosmicOrange   (0xFFFF8C00);
    inline const juce::Colour groovyGreen    (0xFF00FF7F);
    inline const juce::Colour skyBlue        (0xFF00BFFF);
    inline const juce::Colour sunshineYellow (0xFFFFD700);
    inline const juce::Colour darkBg         (0xFF1a1a2e);
    inline const juce::Colour mediumBg       (0xFF16213e);
    
    // Gradient creators
    inline juce::ColourGradient createSunsetSwirl(juce::Rectangle<float> area) {
        juce::ColourGradient gradient(electricPink, area.getX(), area.getY(),
                                       psychPurple, area.getRight(), area.getBottom(), false);
        gradient.addColour(0.5, cosmicOrange);
        return gradient;
    }
    
    inline juce::ColourGradient createCosmicFlow(juce::Rectangle<float> area) {
        juce::ColourGradient gradient(skyBlue, area.getX(), area.getY(),
                                       electricPink, area.getRight(), area.getBottom(), false);
        gradient.addColour(0.5, psychPurple);
        return gradient;
    }
    
    inline juce::ColourGradient createRainbowTrail(juce::Rectangle<float> area, bool horizontal = true) {
        juce::ColourGradient gradient;
        if (horizontal) {
            gradient = juce::ColourGradient(electricPink, area.getX(), area.getCentreY(),
                                             electricPink, area.getRight(), area.getCentreY(), false);
        } else {
            gradient = juce::ColourGradient(electricPink, area.getCentreX(), area.getY(),
                                             electricPink, area.getCentreX(), area.getBottom(), false);
        }
        gradient.addColour(0.16, psychPurple);
        gradient.addColour(0.33, skyBlue);
        gradient.addColour(0.50, groovyGreen);
        gradient.addColour(0.66, sunshineYellow);
        gradient.addColour(0.83, cosmicOrange);
        return gradient;
    }
    
    inline juce::ColourGradient createGlowGradient(juce::Rectangle<float> area, juce::Colour centerColor) {
        return juce::ColourGradient(centerColor, area.getCentreX(), area.getCentreY(),
                                     centerColor.withAlpha(0.0f), area.getRight(), area.getBottom(), true);
    }
}

// 3D effect helpers for psychedelic UI
namespace Effects3D {
    inline void drawGlowHalo(juce::Graphics& g, juce::Rectangle<float> area, juce::Colour color, float intensity = 1.0f) {
        auto expanded = area.expanded(20 * intensity);
        auto gradient = juce::ColourGradient(color.withAlpha(intensity * 0.6f), expanded.getCentreX(), expanded.getCentreY(),
                                              color.withAlpha(0.0f), expanded.getRight(), expanded.getBottom(), true);
        g.setGradientFill(gradient);
        g.fillEllipse(expanded);
    }
    
    inline void drawDropShadow(juce::Graphics& g, juce::Rectangle<float> area, float depth = 4.0f) {
        juce::Path shadow;
        shadow.addRoundedRectangle(area.translated(0, depth), 8.0f);
        g.setColour(juce::Colours::black.withAlpha(0.3f));
        g.fillPath(shadow);
    }
    
    inline void drawSpecularHighlight(juce::Graphics& g, juce::Rectangle<float> area, float angle = 45.0f) {
        auto highlightArea = area.reduced(area.getWidth() * 0.15f);
        highlightArea = highlightArea.withTop(area.getY() + area.getHeight() * 0.1f)
                                     .withHeight(area.getHeight() * 0.3f);
        
        auto gradient = juce::ColourGradient(juce::Colours::white.withAlpha(0.4f),
                                              highlightArea.getCentreX(), highlightArea.getY(),
                                              juce::Colours::white.withAlpha(0.0f),
                                              highlightArea.getCentreX(), highlightArea.getBottom(), false);
        g.setGradientFill(gradient);
        g.fillRoundedRectangle(highlightArea, 6.0f);
    }
    
    inline void drawMetallicGradient(juce::Graphics& g, juce::Rectangle<float> area, juce::Colour base) {
        // Create metallic appearance with multiple gradients
        auto light = base.brighter(0.8f);
        auto dark = base.darker(0.5f);
        
        auto gradient = juce::ColourGradient(light, area.getCentreX(), area.getY(),
                                              dark, area.getCentreX(), area.getBottom(), false);
        gradient.addColour(0.25, base);
        gradient.addColour(0.5, light.darker(0.2f));
        gradient.addColour(0.75, base);
        
        g.setGradientFill(gradient);
        g.fillEllipse(area);
        
        // Add specular highlight
        drawSpecularHighlight(g, area);
    }
    
    inline void draw3DButton(juce::Graphics& g, juce::Rectangle<float> area, juce::Colour color, bool pressed = false) {
        auto bounds = area;
        
        if (!pressed) {
            // Draw shadow first
            auto shadowBounds = bounds.translated(0, 3);
            g.setColour(juce::Colours::black.withAlpha(0.4f));
            g.fillRoundedRectangle(shadowBounds, 10.0f);
            
            // Main button with gradient
            auto gradient = juce::ColourGradient(color.brighter(0.3f), bounds.getCentreX(), bounds.getY(),
                                                  color.darker(0.3f), bounds.getCentreX(), bounds.getBottom(), false);
            g.setGradientFill(gradient);
            g.fillRoundedRectangle(bounds, 10.0f);
        } else {
            // Pressed state - inset with inverted gradient
            bounds = bounds.reduced(1).translated(0, 2);
            auto gradient = juce::ColourGradient(color.darker(0.4f), bounds.getCentreX(), bounds.getY(),
                                                  color.brighter(0.2f), bounds.getCentreX(), bounds.getBottom(), false);
            g.setGradientFill(gradient);
            g.fillRoundedRectangle(bounds, 10.0f);
        }
        
        // Border highlight
        g.setColour(juce::Colours::white.withAlpha(0.3f));
        g.drawRoundedRectangle(bounds, 10.0f, 1.5f);
    }
    
    inline void drawPsychedelicKnob(juce::Graphics& g, juce::Rectangle<float> area, float rotation, float amount) {
        auto center = area.getCentre();
        auto radius = area.getWidth() / 2.0f;
        
        // Outer rainbow halo (intensity based on amount)
        if (amount > 0.01f) {
            auto haloRadius = radius + 15 * amount;
            auto gradient = juce::ColourGradient(PsychedelicColors::electricPink.withAlpha(amount * 0.5f),
                                                  center.x, center.y,
                                                  juce::Colours::transparentBlack,
                                                  center.x + haloRadius, center.y, true);
            gradient.addColour(0.33, PsychedelicColors::sunshineYellow.withAlpha(amount * 0.4f));
            gradient.addColour(0.66, PsychedelicColors::groovyGreen.withAlpha(amount * 0.3f));
            g.setGradientFill(gradient);
            g.fillEllipse(area.expanded(15 * amount));
        }
        
        // Main knob body with metallic gradient
        auto knobArea = area.reduced(5);
        auto silver = juce::Colour(0xFFC0C0C0);
        drawMetallicGradient(g, knobArea, silver);
        
        // Colored ring based on amount
        auto ringColor = amount < 0.33f ? PsychedelicColors::skyBlue :
                         amount < 0.66f ? PsychedelicColors::groovyGreen :
                         PsychedelicColors::electricPink;
        
        g.setColour(ringColor);
        juce::Path ringPath;
        ringPath.addArc(knobArea.getX(), knobArea.getY(), knobArea.getWidth(), knobArea.getHeight(),
                        -2.356f, -2.356f + (4.712f * amount), true);
        g.strokePath(ringPath, juce::PathStrokeType(4.0f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
        
        // Indicator line
        auto indicatorLength = radius * 0.7f;
        auto indicatorAngle = rotation;
        auto indicatorEnd = center + juce::Point<float>(std::sin(indicatorAngle) * indicatorLength,
                                                         -std::cos(indicatorAngle) * indicatorLength);
        
        g.setColour(juce::Colours::white);
        juce::Line<float> indicator(center, indicatorEnd);
        g.drawLine(indicator, 3.0f);
        
        // Center dot
        g.setColour(ringColor.brighter(0.5f));
        g.fillEllipse(center.x - 4, center.y - 4, 8, 8);
    }
}
