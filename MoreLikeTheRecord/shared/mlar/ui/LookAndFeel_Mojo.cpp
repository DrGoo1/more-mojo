#include "LookAndFeel_Mojo.h"

namespace moremojo {
namespace mlar {
namespace mojoUI {

LNF::LNF() {
    // Set up default colors
    setColour(juce::ResizableWindow::backgroundColourId, backgroundColor);
    setColour(juce::TextButton::buttonColourId, backgroundColor.brighter(0.1f));
    setColour(juce::TextButton::buttonOnColourId, highlightColor);
    setColour(juce::TextButton::textColourOffId, foregroundColor);
    setColour(juce::TextButton::textColourOnId, foregroundColor);
    
    setColour(juce::Slider::backgroundColourId, backgroundColor.brighter(0.1f));
    setColour(juce::Slider::trackColourId, highlightColor.withAlpha(0.5f));
    setColour(juce::Slider::thumbColourId, highlightColor);
    setColour(juce::Slider::textBoxTextColourId, foregroundColor);
    setColour(juce::Slider::textBoxBackgroundColourId, backgroundColor.brighter(0.1f));
    
    setColour(juce::ComboBox::backgroundColourId, backgroundColor.brighter(0.1f));
    setColour(juce::ComboBox::textColourId, foregroundColor);
    setColour(juce::ComboBox::arrowColourId, foregroundColor);
    
    setColour(juce::PopupMenu::backgroundColourId, backgroundColor);
    setColour(juce::PopupMenu::textColourId, foregroundColor);
    setColour(juce::PopupMenu::highlightedBackgroundColourId, highlightColor.withAlpha(0.5f));
    setColour(juce::PopupMenu::highlightedTextColourId, foregroundColor);
    
    setColour(juce::Label::textColourId, foregroundColor);
    setColour(juce::Label::backgroundColourId, juce::Colours::transparentBlack);
    
    // Create knob image
    createKnobImage();
}

void LNF::createKnobImage() {
    // Create a pre-rendered knob image for better performance
    const int size = 128;
    knobImage = juce::Image(juce::Image::ARGB, size, size, true);
    
    juce::Graphics g(knobImage);
    
    // Draw a knob at position 0.5 (for caching)
    juce::Rectangle<float> bounds(0, 0, size, size);
    
    // Clear the background
    g.setColour(juce::Colours::transparentBlack);
    g.fillRect(bounds);
    
    // Draw the knob body
    juce::Rectangle<float> knobBounds = bounds.reduced(size * 0.2f);
    
    // Draw main knob body with gradient
    juce::ColourGradient gradient(backgroundColor.brighter(0.3f), knobBounds.getTopLeft(),
                                backgroundColor.darker(0.3f), knobBounds.getBottomRight(),
                                true);
    g.setGradientFill(gradient);
    g.fillEllipse(knobBounds);
    
    // Draw knob rim
    g.setColour(backgroundColor.brighter(0.5f));
    g.drawEllipse(knobBounds, 2.0f);
    
    // Draw indicator line
    juce::Rectangle<float> r = knobBounds;
    const float angle = juce::MathConstants<float>::pi * 1.5f; // 0.5 = 270 degrees
    const float radius = r.getWidth() * 0.4f;
    const float centerX = r.getCentreX();
    const float centerY = r.getCentreY();
    const float lineX = centerX + radius * std::cos(angle);
    const float lineY = centerY + radius * std::sin(angle);
    
    g.setColour(highlightColor);
    g.drawLine(centerX, centerY, lineX, lineY, 3.0f);
}

void LNF::drawRotarySlider(juce::Graphics& g, int x, int y, int width, int height, 
                          float sliderPos, float rotaryStartAngle, float rotaryEndAngle, 
                          juce::Slider& slider) {
    // Define bounds for the knob
    juce::Rectangle<float> bounds(x, y, width, height);
    juce::Rectangle<float> knobBounds = bounds.reduced(width * 0.15f);
    
    // Draw shadow under the knob
    drawShadow(g, knobBounds.expanded(width * 0.05f), 8.0f);
    
    // Calculate rotation angle
    const float angle = rotaryStartAngle + sliderPos * (rotaryEndAngle - rotaryStartAngle);
    
    // Draw the knob by transforming and drawing the cached image
    if (knobImage.isValid()) {
        // Calculate scaling and rotation
        const float scaling = knobBounds.getWidth() / static_cast<float>(knobImage.getWidth());
        
        g.saveState();
        g.addTransform(juce::AffineTransform::translation(-knobImage.getWidth() * 0.5f, -knobImage.getHeight() * 0.5f)
                      .rotated(angle - juce::MathConstants<float>::pi * 1.5f)
                      .scaled(scaling, scaling)
                      .translated(knobBounds.getCentreX(), knobBounds.getCentreY()));
        
        g.drawImageAt(knobImage, 0, 0);
        g.restoreState();
    } else {
        // Fallback drawing if cached image is not valid
        g.setColour(slider.findColour(juce::Slider::thumbColourId));
        g.fillEllipse(knobBounds);
        
        // Draw indicator line
        const float radius = knobBounds.getWidth() * 0.4f;
        const float centerX = knobBounds.getCentreX();
        const float centerY = knobBounds.getCentreY();
        const float lineX = centerX + radius * std::cos(angle);
        const float lineY = centerY + radius * std::sin(angle);
        
        g.setColour(foregroundColor);
        g.drawLine(centerX, centerY, lineX, lineY, 2.0f);
    }
    
    // Draw value text if the slider has text box hidden
    if (slider.getTextBoxPosition() == juce::Slider::NoTextBox) {
        const juce::String text = slider.getTextFromValue(slider.getValue());
        g.setColour(foregroundColor);
        g.setFont(juce::Font(height * 0.15f).boldened());
        g.drawText(text, bounds.removeFromBottom(height * 0.2f).toNearestInt(), juce::Justification::centred, false);
    }
}

void LNF::drawLinearSlider(juce::Graphics& g, int x, int y, int width, int height, 
                          float sliderPos, float minSliderPos, float maxSliderPos, 
                          const juce::Slider::SliderStyle style, juce::Slider& slider) {
    // Define bounds for the slider
    juce::Rectangle<float> bounds(x, y, width, height);
    
    // Different drawing for horizontal and vertical sliders
    if (style == juce::Slider::LinearHorizontal || style == juce::Slider::LinearBar) {
        // Draw track
        juce::Rectangle<float> trackBounds;
        
        if (style == juce::Slider::LinearBar) {
            trackBounds = bounds;
        } else {
            trackBounds = bounds.withSizeKeepingCentre(width, height * 0.25f);
        }
        
        // Draw shadow under the track
        drawShadow(g, trackBounds, 3.0f);
        
        // Draw background track
        g.setColour(slider.findColour(juce::Slider::backgroundColourId));
        g.fillRoundedRectangle(trackBounds, trackBounds.getHeight() * 0.5f);
        
        // Draw filled part of the track
        juce::Rectangle<float> filledTrack = trackBounds.withWidth((sliderPos - x) * slider.isMouseButtonDown() ? 1.05f : 1.0f);
        g.setColour(slider.findColour(juce::Slider::trackColourId));
        g.fillRoundedRectangle(filledTrack, trackBounds.getHeight() * 0.5f);
        
        // Draw thumb
        juce::Rectangle<float> thumbBounds(sliderPos - height * 0.3f, bounds.getCentreY() - height * 0.3f, 
                                         height * 0.6f, height * 0.6f);
        
        // Draw shadow under the thumb
        drawShadow(g, thumbBounds, 5.0f);
        
        // Draw thumb with gradient
        juce::ColourGradient thumbGradient(highlightColor.brighter(0.2f), thumbBounds.getTopLeft(),
                                        highlightColor.darker(0.2f), thumbBounds.getBottomRight(),
                                        true);
        g.setGradientFill(thumbGradient);
        g.fillEllipse(thumbBounds);
        
        // Draw thumb border
        g.setColour(highlightColor.brighter(0.5f));
        g.drawEllipse(thumbBounds, 1.0f);
    } else if (style == juce::Slider::LinearVertical) {
        // Draw track
        juce::Rectangle<float> trackBounds = bounds.withSizeKeepingCentre(width * 0.25f, height);
        
        // Draw shadow under the track
        drawShadow(g, trackBounds, 3.0f);
        
        // Draw background track
        g.setColour(slider.findColour(juce::Slider::backgroundColourId));
        g.fillRoundedRectangle(trackBounds, trackBounds.getWidth() * 0.5f);
        
        // Draw filled part of the track
        juce::Rectangle<float> filledTrack = trackBounds.withHeight(height - (sliderPos - y))
                                           .withY(sliderPos);
        g.setColour(slider.findColour(juce::Slider::trackColourId));
        g.fillRoundedRectangle(filledTrack, trackBounds.getWidth() * 0.5f);
        
        // Draw thumb
        juce::Rectangle<float> thumbBounds(bounds.getCentreX() - width * 0.3f, sliderPos - width * 0.3f,
                                         width * 0.6f, width * 0.6f);
        
        // Draw shadow under the thumb
        drawShadow(g, thumbBounds, 5.0f);
        
        // Draw thumb with gradient
        juce::ColourGradient thumbGradient(highlightColor.brighter(0.2f), thumbBounds.getTopLeft(),
                                        highlightColor.darker(0.2f), thumbBounds.getBottomRight(),
                                        true);
        g.setGradientFill(thumbGradient);
        g.fillEllipse(thumbBounds);
        
        // Draw thumb border
        g.setColour(highlightColor.brighter(0.5f));
        g.drawEllipse(thumbBounds, 1.0f);
    }
}

void LNF::drawToggleButton(juce::Graphics& g, juce::ToggleButton& button, 
                         bool shouldDrawButtonAsHighlighted, 
                         bool shouldDrawButtonAsDown) {
    // Define bounds for the button
    juce::Rectangle<float> bounds = button.getLocalBounds().toFloat().reduced(4.0f);
    
    // Determine button colors based on state
    juce::Colour buttonColor = button.getToggleState() ? 
                             highlightColor : 
                             backgroundColor.brighter(0.1f);
    
    if (shouldDrawButtonAsHighlighted) {
        buttonColor = buttonColor.brighter(0.2f);
    }
    
    if (shouldDrawButtonAsDown) {
        buttonColor = buttonColor.darker(0.2f);
    }
    
    // Draw shadow under button
    drawShadow(g, bounds, button.getToggleState() ? 3.0f : 5.0f);
    
    // Draw button background
    g.setColour(buttonColor);
    g.fillRoundedRectangle(bounds, bounds.getHeight() * 0.2f);
    
    // Draw button text
    g.setColour(foregroundColor);
    g.setFont(button.getFont());
    g.drawText(button.getButtonText(), bounds.toNearestInt(), juce::Justification::centred, true);
}

void LNF::drawComboBox(juce::Graphics& g, int width, int height, bool isButtonDown, 
                     int buttonX, int buttonY, int buttonW, int buttonH, 
                     juce::ComboBox& box) {
    // Define bounds for the combo box
    juce::Rectangle<float> bounds(0, 0, width, height);
    
    // Draw shadow under the combo box
    drawShadow(g, bounds, 5.0f);
    
    // Draw combo box background
    juce::Colour bgColor = box.findColour(juce::ComboBox::backgroundColourId);
    
    if (isButtonDown) {
        bgColor = bgColor.darker(0.1f);
    }
    
    g.setColour(bgColor);
    g.fillRoundedRectangle(bounds, height * 0.2f);
    
    // Draw arrow
    juce::Rectangle<float> arrowBounds(width - height * 0.8f, height * 0.2f, height * 0.6f, height * 0.6f);
    
    juce::Path arrow;
    arrow.startNewSubPath(arrowBounds.getX(), arrowBounds.getY() + arrowBounds.getHeight() * 0.3f);
    arrow.lineTo(arrowBounds.getCentreX(), arrowBounds.getY() + arrowBounds.getHeight() * 0.7f);
    arrow.lineTo(arrowBounds.getRight(), arrowBounds.getY() + arrowBounds.getHeight() * 0.3f);
    
    g.setColour(box.findColour(juce::ComboBox::arrowColourId));
    g.strokePath(arrow, juce::PathStrokeType(2.0f));
}

void LNF::drawPopupMenuBackground(juce::Graphics& g, int width, int height) {
    // Define bounds for the menu
    juce::Rectangle<float> bounds(0, 0, width, height);
    
    // Draw shadow under the menu
    drawShadow(g, bounds, 10.0f);
    
    // Draw menu background
    g.setColour(findColour(juce::PopupMenu::backgroundColourId));
    g.fillRoundedRectangle(bounds, 8.0f);
}

void LNF::drawPopupMenuItem(juce::Graphics& g, const juce::Rectangle<int>& area, 
                          bool isSeparator, bool isActive, bool isHighlighted, 
                          bool isTicked, bool hasSubMenu, const juce::String& text, 
                          const juce::String& shortcutKeyText, 
                          const juce::Drawable* icon, const juce::Colour* textColour) {
    if (isSeparator) {
        juce::Rectangle<int> r(area.reduced(4, 0));
        r = r.removeFromTop(1);
        g.setColour(findColour(juce::PopupMenu::textColourId).withAlpha(0.3f));
        g.fillRect(r);
        return;
    }
    
    juce::Colour textColor = findColour(juce::PopupMenu::textColourId);
    if (textColour != nullptr) {
        textColor = *textColour;
    }
    
    // Highlight if selected
    if (isHighlighted && isActive) {
        g.setColour(findColour(juce::PopupMenu::highlightedBackgroundColourId));
        g.fillRoundedRectangle(area.toFloat(), 4.0f);
        textColor = findColour(juce::PopupMenu::highlightedTextColourId);
    }
    
    // Set alpha for inactive items
    if (!isActive) {
        textColor = textColor.withMultipliedAlpha(0.5f);
    }
    
    // Draw tick if needed
    if (isTicked) {
        const int tickWidth = area.getHeight() * 0.7f;
        const juce::Rectangle<float> tickBounds((area.getHeight() - tickWidth) * 0.5f, 
                                              (area.getHeight() - tickWidth) * 0.5f, 
                                              tickWidth, tickWidth);
        
        juce::Path tick;
        tick.startNewSubPath(tickBounds.getX() + tickBounds.getWidth() * 0.2f, 
                           tickBounds.getCentreY());
        tick.lineTo(tickBounds.getCentreX(), tickBounds.getBottom() - tickBounds.getHeight() * 0.2f);
        tick.lineTo(tickBounds.getRight() - tickBounds.getWidth() * 0.1f, 
                  tickBounds.getY() + tickBounds.getHeight() * 0.2f);
        
        g.setColour(highlightColor);
        g.strokePath(tick, juce::PathStrokeType(2.0f));
    }
    
    // Draw text
    const int indent = (isTicked ? area.getHeight() : 0) + 8;
    
    g.setColour(textColor);
    g.setFont(area.getHeight() * 0.6f);
    g.drawFittedText(text, area.getX() + indent, area.getY(), 
                    area.getWidth() - indent, area.getHeight(), 
                    juce::Justification::centredLeft, 1);
    
    // Draw shortcut text
    if (shortcutKeyText.isNotEmpty()) {
        g.setFont(area.getHeight() * 0.5f);
        g.drawText(shortcutKeyText, area.getX(), area.getY(), 
                  area.getWidth() - 8, area.getHeight(), 
                  juce::Justification::centredRight, true);
    }
}

void LNF::drawTabButton(juce::Graphics& g, juce::TabBarButton& button, 
                      bool isMouseOver, bool isMouseDown) {
    // Define bounds for the tab
    juce::Rectangle<int> activeArea(button.getActiveArea());
    juce::Rectangle<float> bounds = activeArea.toFloat();
    
    // Determine tab colors based on state
    juce::Colour tabColor = button.getToggleState() ? 
                          backgroundColor.brighter(0.15f) : 
                          backgroundColor.brighter(0.05f);
    
    if (isMouseOver) {
        tabColor = tabColor.brighter(0.1f);
    }
    
    if (isMouseDown) {
        tabColor = tabColor.darker(0.1f);
    }
    
    // Draw tab background
    g.setColour(tabColor);
    
    // Adjust shape based on tab position
    if (button.getTabbedButtonBar().isVertical()) {
        if (button.getOrientation() == juce::TabbedButtonBar::TabsAtLeft) {
            g.fillRoundedRectangle(bounds.withTrimmedRight(2), 4.0f);
        } else {
            g.fillRoundedRectangle(bounds.withTrimmedLeft(2), 4.0f);
        }
    } else {
        if (button.getOrientation() == juce::TabbedButtonBar::TabsAtTop) {
            g.fillRoundedRectangle(bounds.withTrimmedBottom(2), 4.0f);
        } else {
            g.fillRoundedRectangle(bounds.withTrimmedTop(2), 4.0f);
        }
    }
    
    // Draw active indicator
    if (button.getToggleState()) {
        g.setColour(highlightColor);
        
        if (button.getTabbedButtonBar().isVertical()) {
            if (button.getOrientation() == juce::TabbedButtonBar::TabsAtLeft) {
                g.fillRect(bounds.getX(), bounds.getY() + 4, 3, bounds.getHeight() - 8);
            } else {
                g.fillRect(bounds.getRight() - 3, bounds.getY() + 4, 3, bounds.getHeight() - 8);
            }
        } else {
            if (button.getOrientation() == juce::TabbedButtonBar::TabsAtTop) {
                g.fillRect(bounds.getX() + 4, bounds.getY(), bounds.getWidth() - 8, 3);
            } else {
                g.fillRect(bounds.getX() + 4, bounds.getBottom() - 3, bounds.getWidth() - 8, 3);
            }
        }
    }
    
    // Draw tab text
    g.setColour(button.getToggleState() ? foregroundColor : foregroundColor.withAlpha(0.8f));
    g.setFont(bounds.getHeight() * 0.6f);
    g.drawFittedText(button.getButtonText(), bounds.toNearestInt(), juce::Justification::centred, 1);
}

void LNF::drawGroupComponentOutline(juce::Graphics& g, int width, int height, 
                                  const juce::String& text, const juce::Justification& position, 
                                  juce::GroupComponent& group) {
    // Define bounds for the group
    juce::Rectangle<float> bounds(0, 0, width, height);
    
    // Draw group background
    g.setColour(backgroundColor.darker(0.2f));
    g.fillRoundedRectangle(bounds, 8.0f);
    
    // Draw group border
    g.setColour(backgroundColor.brighter(0.1f));
    g.drawRoundedRectangle(bounds.reduced(0.5f), 8.0f, 1.0f);
    
    // Calculate text bounds
    const float textH = 15.0f;
    const float textEdgeGap = 4.0f;
    int textW = juce::jmin(text.length() * 8, width - 20);
    
    // Calculate the position for the text
    juce::Rectangle<int> textBounds;
    
    if (position.getOnlyHorizontalFlags() & juce::Justification::left) {
        textBounds = juce::Rectangle<int>(textEdgeGap, 0, textW, static_cast<int>(textH));
    } else if (position.getOnlyHorizontalFlags() & juce::Justification::right) {
        textBounds = juce::Rectangle<int>(width - textW - textEdgeGap, 0, textW, static_cast<int>(textH));
    } else {
        textBounds = juce::Rectangle<int>((width - textW) / 2, 0, textW, static_cast<int>(textH));
    }
    
    // Draw text
    g.setColour(foregroundColor);
    g.setFont(textH * 0.8f);
    g.drawText(text, textBounds, juce::Justification::centred, false);
}

void LNF::card(juce::Graphics& g, juce::Rectangle<float> bounds, juce::Colour color) {
    // Draw card shadow
    drawShadow(g, bounds, 8.0f);
    
    // Draw card background
    g.setColour(color);
    g.fillRoundedRectangle(bounds, 8.0f);
    
    // Draw subtle highlight at top edge
    juce::ColourGradient edgeGradient(color.brighter(0.1f), bounds.getTopLeft(),
                                     color, bounds.getTopLeft().translated(0, 20),
                                     false);
    g.setGradientFill(edgeGradient);
    g.fillRoundedRectangle(bounds.withHeight(20), 8.0f);
}

void LNF::drawShadow(juce::Graphics& g, juce::Rectangle<float> bounds, float depth) {
    // Create shadow path
    juce::Path shadowPath;
    shadowPath.addRoundedRectangle(bounds, 8.0f);
    
    // Draw shadow
    juce::DropShadow shadow(juce::Colours::black.withAlpha(0.5f), 
                          static_cast<int>(depth), 
                          juce::Point<int>(0, 2));
    shadow.drawForPath(g, shadowPath);
}

void LNF::drawInnerShadow(juce::Graphics& g, juce::Rectangle<float> bounds, float depth) {
    // Create inner shadow by drawing outer shadow with clipping
    juce::Path outer;
    outer.addRoundedRectangle(bounds.expanded(depth), 8.0f + depth);
    
    juce::Path inner;
    inner.addRoundedRectangle(bounds, 8.0f);
    
    // Create path for shadow area (outer - inner)
    juce::Path shadow = outer;
    shadow.addPath(inner);
    shadow.setUsingNonZeroWinding(false);
    
    g.setColour(juce::Colours::black.withAlpha(0.2f));
    g.fillPath(shadow);
}

}}} // namespace moremojo::mlar::mojoUI
