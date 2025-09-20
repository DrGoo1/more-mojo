#include "LookAndFeel_Mojo.h"

namespace mojo {

LookAndFeel_Mojo::LookAndFeel_Mojo() {
    // Set default colors
    setColour(juce::ResizableWindow::backgroundColourId, palette.bg);
    setColour(juce::Slider::thumbColourId, palette.accent);
    setColour(juce::Slider::rotarySliderFillColourId, palette.accent);
    setColour(juce::Slider::trackColourId, palette.accent.withAlpha(0.3f));
    setColour(juce::TextButton::buttonColourId, palette.card);
    setColour(juce::TextButton::buttonOnColourId, palette.accent);
    setColour(juce::TextButton::textColourOffId, palette.textMain);
    setColour(juce::TextButton::textColourOnId, palette.textMain);
    setColour(juce::ComboBox::backgroundColourId, palette.card);
    setColour(juce::ComboBox::textColourId, palette.textMain);
}

void LookAndFeel_Mojo::drawCard(juce::Graphics& g, juce::Rectangle<float> bounds, 
                               juce::Colour fill, float cornerRadius, float elevation) {
    // Create path for rounded rectangle
    juce::Path path;
    path.addRoundedRectangle(bounds, cornerRadius);
    
    // Draw shadow
    juce::DropShadow(juce::Colours::black.withAlpha(0.6f), 
                    static_cast<int>(elevation), 
                    juce::Point<int>(0, static_cast<int>(elevation / 2)))
        .drawForPath(g, path);
    
    // Fill with gradient
    auto top = fill.brighter(0.05f);
    auto bottom = fill.darker(0.10f);
    g.setGradientFill(juce::ColourGradient(
        top, bounds.getCentreX(), bounds.getY(),
        bottom, bounds.getCentreX(), bounds.getBottom(),
        false));
    g.fillPath(path);
    
    // Draw a subtle highlight along the edge
    g.setColour(juce::Colours::white.withAlpha(0.05f));
    g.strokePath(path, juce::PathStrokeType(1.5f));
}

void LookAndFeel_Mojo::drawRotarySlider(juce::Graphics& g, int x, int y, int width, int height,
                                        float sliderPos, float startAngle, float endAngle, 
                                        juce::Slider& slider) {
    // Calculate bounds and radius
    auto bounds = juce::Rectangle<float>(static_cast<float>(x), static_cast<float>(y),
                                        static_cast<float>(width), static_cast<float>(height)).reduced(4.f);
    auto radius = juce::jmin(bounds.getWidth(), bounds.getHeight()) * 0.5f;
    auto center = bounds.getCentre();
    
    // Draw main knob body with gradient
    juce::Colour knobColor = slider.isEnabled() ? palette.panel : palette.panel.withMultipliedAlpha(0.5f);
    g.setGradientFill(juce::ColourGradient(
        knobColor.brighter(0.1f), center.x - radius * 0.4f, center.y - radius * 0.4f,
        knobColor.darker(0.15f), center.x + radius * 0.6f, center.y + radius * 0.6f,
        true));
    g.fillEllipse(bounds);
    
    // Draw shadow ring
    g.setColour(juce::Colours::black.withAlpha(0.35f));
    g.drawEllipse(bounds, 2.f);
    
    // Draw the colored arc showing position
    auto angle = startAngle + sliderPos * (endAngle - startAngle);
    juce::Path arc;
    arc.addCentredArc(center.x, center.y,
                      radius - 6.f, radius - 6.f,
                      0.f, startAngle, angle,
                      true);
    
    g.setColour(slider.findColour(juce::Slider::rotarySliderFillColourId));
    g.strokePath(arc, juce::PathStrokeType(3.f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
    
    // Draw indicator needle
    juce::Path needle;
    needle.addRectangle(-2.f, -radius + 10.f, 4.f, radius * 0.55f);
    g.setColour(juce::Colours::white.withAlpha(0.9f));
    g.fillPath(needle, juce::AffineTransform::rotation(angle).translated(center.x, center.y));
}

void LookAndFeel_Mojo::drawButtonBackground(juce::Graphics& g, juce::Button& button,
                                           const juce::Colour& backgroundColor,
                                           bool isHighlighted, bool isDown) {
    juce::ignoreUnused(backgroundColor);
    auto bounds = button.getLocalBounds().toFloat().reduced(1.f);
    auto baseColor = palette.card;
    
    if (isHighlighted) baseColor = baseColor.brighter(0.05f);
    if (isDown) baseColor = baseColor.darker(0.10f);
    if (button.getToggleState()) baseColor = palette.accent;
    
    drawCard(g, bounds, baseColor, 10.f, 8.f);
}

void LookAndFeel_Mojo::drawToggleButton(juce::Graphics& g, juce::ToggleButton& button,
                                       bool isHighlighted, bool isDown) {
    auto bounds = button.getLocalBounds();
    juce::ignoreUnused(isDown);
    
    auto backgroundColor = isHighlighted ? palette.card.brighter(0.05f) : palette.card;
    drawCard(g, bounds.toFloat(), backgroundColor, 8.f, 6.f);
    
    g.setColour(palette.textMain);
    g.setFont(juce::Font(14.f, juce::Font::bold));
    g.drawFittedText(button.getButtonText(), bounds.reduced(8), juce::Justification::centred, 1);
    
    // Draw the LED indicator
    auto led = juce::Rectangle<float>(bounds.getRight() - 18.f, bounds.getY() + 8.f, 10.f, 10.f);
    g.setColour(button.getToggleState() ? palette.ok : palette.textDim.withAlpha(0.4f));
    g.fillEllipse(led);
}

}  // namespace mojo
