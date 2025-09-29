#include "CustomLookAndFeel.h"

CustomLookAndFeel::CustomLookAndFeel()
{
    // Set up dark color scheme for professional audio GUI
    setColour(juce::ResizableWindow::backgroundColourId, juce::Colour(0xff161616));
    setColour(juce::TabbedComponent::backgroundColourId, juce::Colour(0xff222222));
    setColour(juce::TabbedButtonBar::tabOutlineColourId, juce::Colour(0xff333333));
    setColour(juce::TabbedButtonBar::frontOutlineColourId, juce::Colour(0xff3373d7));
    setColour(juce::TabbedButtonBar::tabTextColourId, juce::Colours::lightgrey);
    setColour(juce::TabbedButtonBar::frontTextColourId, juce::Colours::white);
    
    // Custom slider colors
    setColour(juce::Slider::backgroundColourId, juce::Colour(0xff222222));
    setColour(juce::Slider::trackColourId, juce::Colour(0xff333333));
    setColour(juce::Slider::thumbColourId, juce::Colour(0xff3373d7));
    setColour(juce::Slider::rotarySliderFillColourId, juce::Colour(0xff3373d7));
    setColour(juce::Slider::rotarySliderOutlineColourId, juce::Colour(0xff444444));
    setColour(juce::Slider::textBoxTextColourId, juce::Colours::white);
    setColour(juce::Slider::textBoxBackgroundColourId, juce::Colour(0xff333333));
    setColour(juce::Slider::textBoxOutlineColourId, juce::Colour(0xff444444));
}

void CustomLookAndFeel::drawRotarySlider(juce::Graphics& g, int x, int y, int width, int height, 
                                          float sliderPos, float rotaryStartAngle, float rotaryEndAngle, 
                                          juce::Slider& slider)
{
    // Draw high-resolution 3D metallic knob
    bool isEnabled = slider.isEnabled();
    juce::Colour knobColor = slider.findColour(juce::Slider::rotarySliderFillColourId);
    
    // Adjust for a square shape
    int diameter = juce::jmin(width, height);
    int knobX = x + (width - diameter) / 2;
    int knobY = y + (height - diameter) / 2;
    
    drawMetalKnob(g, knobX, knobY, diameter, sliderPos, knobColor, isEnabled);
    
    // Draw value label if slider has textbox
    if (slider.getTextBoxPosition() == juce::Slider::NoTextBox) {
        auto fontHeight = juce::jmin(14.0f, (float)diameter * 0.15f);
        g.setFont(fontHeight);
        g.setColour(isEnabled ? juce::Colours::white : juce::Colours::grey);
        
        // Format value with precision
        juce::String valueText;
        if (slider.getInterval() < 0.1)
            valueText = juce::String(slider.getValue(), 2);
        else
            valueText = juce::String(slider.getValue(), 1);
        
        // Draw centered text below knob
        g.drawText(valueText, knobX, knobY + diameter, diameter, fontHeight + 4.0f, 
                  juce::Justification::centred, false);
    }
}

void CustomLookAndFeel::drawLinearSlider(juce::Graphics& g, int x, int y, int width, int height,
                                         float sliderPos, float minSliderPos, float maxSliderPos,
                                         const juce::Slider::SliderStyle style, juce::Slider& slider)
{
    if (style == juce::Slider::LinearVertical || style == juce::Slider::LinearHorizontal) {
        bool isVertical = (style == juce::Slider::LinearVertical);
        bool isEnabled = slider.isEnabled();
        
        juce::Colour trackColor = slider.findColour(juce::Slider::trackColourId);
        juce::Colour thumbColor = slider.findColour(juce::Slider::thumbColourId);
        
        if (!isEnabled) {
            trackColor = trackColor.withAlpha(0.4f);
            thumbColor = thumbColor.withAlpha(0.4f);
        }
        
        // Draw track
        juce::Rectangle<float> trackArea;
        if (isVertical) {
            int trackWidth = juce::jmin(8, width / 3);
            trackArea = juce::Rectangle<float>((float)x + (width - trackWidth) * 0.5f, (float)y, (float)trackWidth, (float)height);
        } else {
            int trackHeight = juce::jmin(8, height / 3);
            trackArea = juce::Rectangle<float>((float)x, (float)y + (height - trackHeight) * 0.5f, (float)width, (float)trackHeight);
        }
        
        // Draw background track with inner shadow
        g.setColour(slider.findColour(juce::Slider::backgroundColourId));
        g.fillRoundedRectangle(trackArea, trackArea.getWidth() * 0.5f);
        drawInnerShadow(g, trackArea, 2.0f);
        
        // Draw filled portion
        juce::Rectangle<float> filledArea;
        if (isVertical) {
            float fillHeight = sliderPos - y;
            filledArea = juce::Rectangle<float>(trackArea.getX(), sliderPos - fillHeight, 
                                                trackArea.getWidth(), fillHeight);
        } else {
            float fillWidth = sliderPos - x;
            filledArea = juce::Rectangle<float>(x, trackArea.getY(), 
                                                fillWidth, trackArea.getHeight());
        }
        
        g.setColour(thumbColor);
        g.fillRoundedRectangle(filledArea, filledArea.getWidth() * 0.5f);
        
        // Draw 3D thumb
        float thumbWidth = isVertical ? trackArea.getWidth() * 1.5f : juce::jmin(16.0f, (float)height * 0.6f);
        float thumbHeight = isVertical ? juce::jmin(16.0f, (float)width * 0.6f) : trackArea.getHeight() * 1.5f;
        
        juce::Rectangle<float> thumbArea;
        if (isVertical) {
            thumbArea = juce::Rectangle<float>(trackArea.getX() + (trackArea.getWidth() - thumbWidth) * 0.5f,
                                               sliderPos - thumbHeight * 0.5f,
                                               thumbWidth, thumbHeight);
        } else {
            thumbArea = juce::Rectangle<float>(sliderPos - thumbWidth * 0.5f,
                                               trackArea.getY() + (trackArea.getHeight() - thumbHeight) * 0.5f,
                                               thumbWidth, thumbHeight);
        }
        
        // Draw 3D thumb with metallic effect
        g.setColour(thumbColor);
        g.fillRoundedRectangle(thumbArea, thumbArea.getHeight() * 0.5f);
        
        // Add highlight to thumb
        g.setColour(juce::Colours::white.withAlpha(0.4f));
        g.drawRoundedRectangle(thumbArea.reduced(1.0f), thumbArea.getHeight() * 0.5f, 1.0f);
        
        // Add glossy highlight on top of the thumb
        juce::Rectangle<float> glossArea = thumbArea.reduced(1.0f).withHeight(thumbArea.getHeight() * 0.5f);
        drawGloss(g, glossArea, 0.2f);
    } else {
        // For other slider styles, use the default drawing
        LookAndFeel_V4::drawLinearSlider(g, x, y, width, height, sliderPos, 
                                        minSliderPos, maxSliderPos, style, slider);
    }
}

void CustomLookAndFeel::drawButtonBackground(juce::Graphics& g, juce::Button& button, 
                                            const juce::Colour& backgroundColour,
                                            bool shouldDrawButtonAsHighlighted, 
                                            bool shouldDrawButtonAsDown)
{
    // Draw 3D button
    auto bounds = button.getLocalBounds().toFloat();
    draw3DButton(g, bounds.getX(), bounds.getY(), bounds.getWidth(), bounds.getHeight(), 
                backgroundColour, shouldDrawButtonAsHighlighted, shouldDrawButtonAsDown);
}

void CustomLookAndFeel::drawTabButton(juce::TabBarButton& button, juce::Graphics& g, 
                                     bool isMouseOver, bool isMouseDown)
{
    auto activeArea = button.getActiveArea();
    auto isActive = button.getToggleState();
    
    // Tab background
    juce::Colour tabColor = isActive ? juce::Colour(0xff282828) : juce::Colour(0xff1a1a1a);
    if (isMouseOver && !isActive)
        tabColor = tabColor.brighter(0.1f);
    
    g.setColour(tabColor);
    g.fillRoundedRectangle(activeArea.withTrimmedBottom(1).toFloat(), 4.0f);
    
    // Tab outline
    g.setColour(isActive ? button.findColour(juce::TabbedButtonBar::frontOutlineColourId) 
                         : juce::Colours::transparentBlack);
    g.drawRoundedRectangle(activeArea.withTrimmedBottom(1).toFloat(), 4.0f, 1.0f);
    
    // Tab text
    g.setColour(isActive ? button.findColour(juce::TabbedButtonBar::frontTextColourId)
                         : button.findColour(juce::TabbedButtonBar::tabTextColourId));
    g.setFont(juce::Font(14.0f, juce::Font::bold));
    g.drawText(button.getButtonText(), activeArea.reduced(4, 2), juce::Justification::centred, false);
    
    // Add subtle highlight on active tab
    if (isActive) {
        g.setColour(juce::Colour(0xff3373d7).withAlpha(0.5f));
        g.fillRect(activeArea.getX(), activeArea.getBottom() - 2, activeArea.getWidth(), 2);
    }
}

void CustomLookAndFeel::drawTabAreaBehindFrontButton(juce::TabbedButtonBar& bar, juce::Graphics& g, 
                                                   int w, int h)
{
    // Draw tab area background with subtle gradient
    juce::Rectangle<int> tabArea (0, 0, w, h);
    
    g.setGradientFill(juce::ColourGradient(
        juce::Colour(0xff202020), 0, 0,
        juce::Colour(0xff1a1a1a), 0, h,
        false));
    g.fillRect(tabArea);
    
    // Draw bottom line
    g.setColour(juce::Colour(0xff333333));
    g.fillRect(0, h - 1, w, 1);
}

void CustomLookAndFeel::drawMetalKnob(juce::Graphics& g, int x, int y, int diameter, float sliderPos, 
                                     const juce::Colour& knobColor, bool isEnabled)
{
    // Create gradient for metallic look
    juce::Colour baseColor = isEnabled ? knobColor : knobColor.withBrightness(0.6f);
    juce::Colour highlightColor = baseColor.brighter(0.2f);
    juce::Colour shadowColor = baseColor.darker(0.4f);
    
    juce::Rectangle<float> knobArea(x, y, diameter, diameter);
    juce::Rectangle<float> centeredKnob = knobArea.reduced(diameter * 0.1f);
    
    // 1. Draw main knob body with gradient
    g.setGradientFill(juce::ColourGradient(
        highlightColor, centeredKnob.getX(), centeredKnob.getY(),
        shadowColor, centeredKnob.getRight(), centeredKnob.getBottom(),
        true));
    g.fillEllipse(centeredKnob);
    
    // 2. Draw outer ring (bevel)
    g.setGradientFill(juce::ColourGradient(
        shadowColor, knobArea.getX(), knobArea.getY(),
        highlightColor, knobArea.getRight(), knobArea.getBottom(),
        true));
    g.drawEllipse(centeredKnob.expanded(1.0f), 2.0f);
    
    // 3. Draw indicator line
    float angle = juce::jmap(sliderPos, 0.0f, 1.0f, 2.35f, 7.07f);
    float indicatorLength = diameter * 0.35f;
    float centerX = centeredKnob.getCentreX();
    float centerY = centeredKnob.getCentreY();
    
    juce::Path indicator;
    indicator.startNewSubPath(centerX, centerY);
    indicator.lineTo(centerX + indicatorLength * std::cos(angle),
                    centerY + indicatorLength * std::sin(angle));
    
    g.setColour(isEnabled ? juce::Colours::white : juce::Colours::grey);
    g.strokePath(indicator, juce::PathStrokeType(2.0f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
    
    // 4. Draw central highlight
    float glossDiameter = centeredKnob.getWidth() * 0.4f;
    juce::Rectangle<float> glossArea(centerX - glossDiameter / 2, 
                                  centerY - glossDiameter / 2, 
                                  glossDiameter, glossDiameter);
    
    g.setGradientFill(juce::ColourGradient(
        juce::Colours::white.withAlpha(0.3f), glossArea.getX(), glossArea.getY(),
        juce::Colours::white.withAlpha(0.0f), glossArea.getRight(), glossArea.getBottom(),
        true));
    g.fillEllipse(glossArea.translated(-glossArea.getWidth() * 0.2f, -glossArea.getHeight() * 0.2f));
}

void CustomLookAndFeel::draw3DButton(juce::Graphics& g, int x, int y, int width, int height, 
                                    const juce::Colour& buttonColor, bool isHighlighted, bool isDown)
{
    juce::Rectangle<float> buttonArea((float)x, (float)y, (float)width, (float)height);
    
    juce::Colour baseColor = isHighlighted ? buttonColor.brighter(0.1f) : buttonColor;
    baseColor = isDown ? baseColor.darker(0.2f) : baseColor;
    
    // Button body
    if (isDown) {
        g.setGradientFill(juce::ColourGradient(
            baseColor.darker(0.2f), buttonArea.getX(), buttonArea.getY(),
            baseColor, buttonArea.getRight(), buttonArea.getBottom(),
            false));
    } else {
        g.setGradientFill(juce::ColourGradient(
            baseColor.brighter(0.1f), buttonArea.getX(), buttonArea.getY(),
            baseColor.darker(0.1f), buttonArea.getRight(), buttonArea.getBottom(),
            false));
    }
    
    g.fillRoundedRectangle(buttonArea, 4.0f);
    
    // Button edge
    g.setColour(isDown ? baseColor.darker(0.4f) : baseColor.darker(0.2f));
    g.drawRoundedRectangle(buttonArea, 4.0f, 1.0f);
    
    // Inner highlight (when not pressed)
    if (!isDown) {
        g.setColour(juce::Colours::white.withAlpha(0.07f));
        g.drawRoundedRectangle(buttonArea.reduced(1.0f), 3.0f, 1.0f);
        
        // Top highlight
        juce::Rectangle<float> highlightArea = buttonArea.reduced(2.0f).withHeight(buttonArea.getHeight() * 0.3f);
        drawGloss(g, highlightArea, 0.1f);
    } else {
        // Inner shadow when pressed
        drawInnerShadow(g, buttonArea, 2.0f);
    }
}

void CustomLookAndFeel::drawGloss(juce::Graphics& g, const juce::Rectangle<float>& area, float opacity)
{
    g.setGradientFill(juce::ColourGradient(
        juce::Colours::white.withAlpha(opacity), area.getX(), area.getY(),
        juce::Colours::white.withAlpha(0.0f), area.getX(), area.getBottom(),
        false));
    g.fillRoundedRectangle(area, area.getHeight() * 0.5f);
}

void CustomLookAndFeel::drawInnerShadow(juce::Graphics& g, const juce::Rectangle<float>& area, float depth)
{
    juce::Rectangle<float> innerShadow = area.reduced(1.0f);
    
    // Top shadow
    g.setGradientFill(juce::ColourGradient(
        juce::Colours::black.withAlpha(0.2f), innerShadow.getX(), innerShadow.getY(),
        juce::Colours::black.withAlpha(0.0f), innerShadow.getX(), innerShadow.getY() + depth,
        false));
    g.fillRect(innerShadow.withHeight(depth));
    
    // Left shadow
    g.setGradientFill(juce::ColourGradient(
        juce::Colours::black.withAlpha(0.1f), innerShadow.getX(), innerShadow.getY(),
        juce::Colours::black.withAlpha(0.0f), innerShadow.getX() + depth, innerShadow.getY(),
        false));
    g.fillRect(innerShadow.withWidth(depth));
}
