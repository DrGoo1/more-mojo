#pragma once
#include <juce_graphics/juce_graphics.h>
#include <juce_gui_basics/juce_gui_basics.h>

// Psychedelic color system and drawing utilities
// Inspired by Peter Max, Yellow Submarine, Austin Powers
namespace PsychedelicTheme {

// ============================================================================
// COLOR PALETTE
// ============================================================================

namespace Colors {
    // Primary psychedelic colors
    const juce::Colour electricPink   (0xFFFF1493);  // Hot Pink
    const juce::Colour psychPurple    (0xFF9B30FF);  // Purple
    const juce::Colour cosmicOrange   (0xFFFF8C00);  // Dark Orange
    const juce::Colour groovyGreen    (0xFF00FF7F);  // Spring Green
    const juce::Colour skyBlue        (0xFF00BFFF);  // Deep Sky Blue
    const juce::Colour sunshineYellow (0xFFFFD700);  // Gold
    
    // Secondary colors
    const juce::Colour neonCyan       (0xFF00FFFF);  // Cyan
    const juce::Colour hotMagenta     (0xFFFF00FF);  // Magenta
    const juce::Colour limeGreen      (0xFF32CD32);  // Lime
    const juce::Colour electricViolet (0xFF8B00FF);  // Violet
    
    // Metallics for 3D effects
    const juce::Colour chromeSilver   (0xFFC0C0C0);  // Silver
    const juce::Colour goldShine      (0xFFFFD700);  // Gold
    const juce::Colour copperGlow     (0xFFB87333);  // Copper
    
    // Background colors
    const juce::Colour deepSpace      (0xFF0A0A1A);  // Very dark blue
    const juce::Colour darkPurple     (0xFF1A0A2E);  // Dark purple
    const juce::Colour midnightBlue   (0xFF0F0F2D);  // Midnight
}

// ============================================================================
// GRADIENT CREATORS
// ============================================================================

// Sunset swirl gradient (Pink → Orange → Purple)
inline juce::ColourGradient createSunsetSwirl(juce::Rectangle<float> bounds) {
    juce::ColourGradient gradient;
    gradient = juce::ColourGradient::vertical(
        Colors::electricPink,
        bounds.getY(),
        Colors::psychPurple,
        bounds.getBottom()
    );
    gradient.addColour(0.5, Colors::cosmicOrange);
    return gradient;
}

// Cosmic flow gradient (Blue → Purple → Pink)
inline juce::ColourGradient createCosmicFlow(juce::Rectangle<float> bounds) {
    juce::ColourGradient gradient;
    gradient = juce::ColourGradient::vertical(
        Colors::skyBlue,
        bounds.getY(),
        Colors::electricPink,
        bounds.getBottom()
    );
    gradient.addColour(0.5, Colors::psychPurple);
    return gradient;
}

// Rainbow gradient (horizontal)
inline juce::ColourGradient createRainbowTrail(juce::Rectangle<float> bounds) {
    juce::ColourGradient gradient;
    gradient = juce::ColourGradient::horizontal(
        Colors::electricPink,
        bounds.getX(),
        Colors::psychPurple,
        bounds.getRight()
    );
    gradient.addColour(0.2, Colors::cosmicOrange);
    gradient.addColour(0.4, Colors::sunshineYellow);
    gradient.addColour(0.6, Colors::groovyGreen);
    gradient.addColour(0.8, Colors::skyBlue);
    return gradient;
}

// Radial rainbow (for halos)
inline juce::ColourGradient createRainbowHalo(juce::Point<float> center, float radius) {
    juce::ColourGradient gradient;
    gradient = juce::ColourGradient::horizontal(
        Colors::electricPink,
        center.x - radius,
        Colors::psychPurple,
        center.x + radius
    );
    gradient.addColour(0.33, Colors::sunshineYellow);
    gradient.addColour(0.67, Colors::groovyGreen);
    gradient.isRadial = true;
    gradient.point1 = center;
    gradient.point2 = juce::Point<float>(center.x + radius, center.y);
    return gradient;
}

// Metallic gradient for 3D effects
inline juce::ColourGradient createMetallicGradient(juce::Rectangle<float> bounds, 
                                                    juce::Colour baseColor) {
    auto lighter = baseColor.brighter(0.5f);
    auto darker = baseColor.darker(0.3f);
    
    juce::ColourGradient gradient;
    gradient = juce::ColourGradient::vertical(lighter, bounds.getY(), darker, bounds.getBottom());
    gradient.addColour(0.1, lighter.brighter(0.2f)); // Specular highlight
    gradient.addColour(0.9, darker.darker(0.2f));    // Shadow
    return gradient;
}

// ============================================================================
// 3D EFFECT HELPERS
// ============================================================================

// Draw a glowing halo around a component
inline void drawGlowHalo(juce::Graphics& g, juce::Rectangle<float> bounds, 
                         juce::Colour color, float intensity) {
    auto expanded = bounds.expanded(20.0f * intensity);
    
    for (int i = 0; i < 5; i++) {
        float alpha = (1.0f - i * 0.2f) * intensity;
        g.setColour(color.withAlpha(alpha * 0.3f));
        g.fillEllipse(expanded.reduced(i * 4.0f));
    }
}

// Draw a drop shadow
inline void drawDropShadow(juce::Graphics& g, juce::Rectangle<float> bounds, 
                          float depth = 8.0f) {
    juce::Path shadowPath;
    shadowPath.addRoundedRectangle(bounds.translated(depth * 0.5f, depth), 8.0f);
    
    juce::DropShadow shadow(juce::Colours::black.withAlpha(0.5f), 
                           (int)depth, 
                           juce::Point<int>(0, 0));
    shadow.drawForPath(g, shadowPath);
}

// Draw specular highlight (for 3D shine)
inline void drawSpecularHighlight(juce::Graphics& g, juce::Rectangle<float> bounds, 
                                  float angle = 45.0f) {
    auto highlight = bounds.reduced(bounds.getWidth() * 0.3f, bounds.getHeight() * 0.3f);
    highlight = highlight.withY(bounds.getY() + bounds.getHeight() * 0.1f);
    
    juce::ColourGradient grad = juce::ColourGradient::vertical(
        juce::Colours::white.withAlpha(0.6f),
        highlight.getY(),
        juce::Colours::white.withAlpha(0.0f),
        highlight.getBottom()
    );
    
    g.setGradientFill(grad);
    g.fillEllipse(highlight);
}

// ============================================================================
// BUTTON DRAWING
// ============================================================================

// Draw a groovy 3D button
inline void drawGroovyButton(juce::Graphics& g, juce::Rectangle<float> bounds,
                            const juce::String& text, juce::Colour baseColor,
                            bool isPressed = false, bool isHovered = false) {
    auto b = bounds;
    
    // Drop shadow
    if (!isPressed) {
        drawDropShadow(g, b, 6.0f);
    }
    
    // Button body with gradient
    if (isPressed) {
        b = b.translated(2.0f, 2.0f);
        auto grad = createMetallicGradient(b, baseColor.darker(0.2f));
        g.setGradientFill(grad);
    } else {
        auto grad = createMetallicGradient(b, baseColor);
        g.setGradientFill(grad);
    }
    
    g.fillRoundedRectangle(b, 10.0f);
    
    // Glow effect on hover
    if (isHovered && !isPressed) {
        drawGlowHalo(g, b, baseColor, 0.8f);
    }
    
    // Outline
    g.setColour(juce::Colours::white.withAlpha(0.3f));
    g.drawRoundedRectangle(b, 10.0f, 2.0f);
    
    // Specular highlight
    if (!isPressed) {
        drawSpecularHighlight(g, b);
    }
    
    // Text
    g.setColour(juce::Colours::white);
    g.setFont(juce::Font(16.0f, juce::Font::bold));
    g.drawText(text, b, juce::Justification::centred);
}

// ============================================================================
// METER DRAWING
// ============================================================================

// Draw a psychedelic horizontal meter with flowing animation
inline void drawPsychedelicMeter(juce::Graphics& g, juce::Rectangle<float> bounds,
                                float value, const juce::String& label,
                                const juce::String& emoji) {
    // Background track
    g.setColour(Colors::deepSpace);
    g.fillRoundedRectangle(bounds, 5.0f);
    
    // Filled portion with rainbow gradient
    if (value > 0.0f) {
        auto filled = bounds.withWidth(bounds.getWidth() * value);
        
        // Color based on value
        juce::Colour meterColor;
        if (value < 0.33f)
            meterColor = Colors::skyBlue.interpolatedWith(Colors::groovyGreen, value * 3.0f);
        else if (value < 0.67f)
            meterColor = Colors::groovyGreen.interpolatedWith(Colors::sunshineYellow, (value - 0.33f) * 3.0f);
        else
            meterColor = Colors::sunshineYellow.interpolatedWith(Colors::electricPink, (value - 0.67f) * 3.0f);
        
        auto grad = juce::ColourGradient::horizontal(
            meterColor.darker(0.2f), filled.getX(),
            meterColor, filled.getRight()
        );
        g.setGradientFill(grad);
        g.fillRoundedRectangle(filled, 5.0f);
        
        // Glow on top
        g.setColour(meterColor.withAlpha(0.3f));
        g.fillRoundedRectangle(filled.reduced(0, 2.0f), 5.0f);
    }
    
    // Border
    g.setColour(juce::Colours::white.withAlpha(0.2f));
    g.drawRoundedRectangle(bounds, 5.0f, 1.0f);
    
    // Label on left
    g.setColour(juce::Colours::white);
    g.setFont(juce::Font(14.0f, juce::Font::bold));
    auto labelArea = bounds.withWidth(80.0f).translated(-85.0f, 0);
    g.drawText(label + ":", labelArea, juce::Justification::centredRight);
    
    // Percentage
    g.drawText(juce::String((int)(value * 100)) + "%", 
               bounds.reduced(5, 0), 
               juce::Justification::centredRight);
    
    // Emoji on right
    g.setFont(juce::Font(20.0f));
    auto emojiArea = bounds.withX(bounds.getRight() + 5).withWidth(30.0f);
    g.drawText(emoji, emojiArea, juce::Justification::centredLeft);
}

// ============================================================================
// TITLE DRAWING
// ============================================================================

// Draw psychedelic title with rainbow outline
inline void drawPsychedelicTitle(juce::Graphics& g, juce::Rectangle<float> bounds,
                                const juce::String& text) {
    // Drop shadow
    g.setColour(juce::Colours::black.withAlpha(0.5f));
    g.setFont(juce::Font(40.0f, juce::Font::bold));
    g.drawText(text, bounds.translated(3, 3), juce::Justification::centred);
    
    // Rainbow gradient fill
    auto grad = createRainbowTrail(bounds);
    g.setGradientFill(grad);
    g.setFont(juce::Font(40.0f, juce::Font::bold));
    g.drawText(text, bounds, juce::Justification::centred);
    
    // White outline
    g.setColour(juce::Colours::white);
    juce::Path textPath;
    juce::GlyphArrangement glyphs;
    glyphs.addLineOfText(juce::Font(40.0f, juce::Font::bold), text, 
                        bounds.getCentreX(), bounds.getCentreY());
    glyphs.createPath(textPath);
    
    juce::PathStrokeType stroke(3.0f);
    g.strokePath(textPath, stroke);
}

// ============================================================================
// SECTION DRAWING
// ============================================================================

// Draw a section container with groovy styling
inline void drawSection(juce::Graphics& g, juce::Rectangle<float> bounds,
                       const juce::String& title) {
    // Background with gradient
    auto grad = juce::ColourGradient::vertical(
        Colors::deepSpace.brighter(0.2f), bounds.getY(),
        Colors::deepSpace, bounds.getBottom()
    );
    g.setGradientFill(grad);
    g.fillRoundedRectangle(bounds, 10.0f);
    
    // Border with glow
    g.setColour(Colors::psychPurple.withAlpha(0.5f));
    g.drawRoundedRectangle(bounds, 10.0f, 2.0f);
    
    // Title
    g.setColour(Colors::sunshineYellow);
    g.setFont(juce::Font(16.0f, juce::Font::bold));
    auto titleArea = bounds.removeFromTop(30.0f).reduced(10.0f, 5.0f);
    g.drawText(title, titleArea, juce::Justification::centredLeft);
}

} // namespace PsychedelicTheme
