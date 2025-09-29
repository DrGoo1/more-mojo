#include "ControlInfoWindow.h"

ControlInfoWindow::ControlInfoWindow(const ControlInfo& info) : controlInfo(info)
{
    setSize(WINDOW_WIDTH, WINDOW_HEIGHT);
    
    // Create close button
    closeButton = std::make_unique<juce::TextButton>("Close");
    closeButton->setColour(juce::TextButton::buttonColourId, juce::Colour(0xFF4a4a6a));
    closeButton->setColour(juce::TextButton::textColourOffId, juce::Colours::white);
    closeButton->onClick = [this]()
    {
        // Find the dialog window that contains this component
        juce::Component* current = this;
        while (current != nullptr)
        {
            if (auto* dialogWindow = dynamic_cast<juce::DialogWindow*>(current))
            {
                dialogWindow->exitModalState(0);
                return;
            }
            current = current->getParentComponent();
        }
        
        // If we can't find a dialog window, try to close any modal component
        if (auto* modalComponent = juce::Component::getCurrentlyModalComponent())
        {
            modalComponent->exitModalState(0);
        }
    };
    addAndMakeVisible(*closeButton);
}

void ControlInfoWindow::paint(juce::Graphics& g)
{
    auto bounds = getLocalBounds();
    
    // Professional background with gradient
    g.fillAll(juce::Colour(0xFF1a1a2e));
    
    juce::ColourGradient gradient(juce::Colour(0xFF16213e), 0, 0, 
                                 juce::Colour(0xFF0f3460), 0, bounds.getHeight(), false);
    g.setGradientFill(gradient);
    g.fillAll();
    
    // Subtle texture
    g.setColour(juce::Colour(0xFF333355).withAlpha(0.2f));
    for (int y = 0; y < bounds.getHeight(); y += 2)
    {
        g.drawHorizontalLine(y, 0, bounds.getWidth());
    }
    
    // Content area - split into text and visual areas
    auto contentArea = bounds.reduced(MARGIN);
    auto textArea = contentArea.removeFromLeft(500); // Text on left
    auto visualArea = contentArea.removeFromRight(320); // Graphics on right
    
    int currentY = textArea.getY();
    
    // Title with accent color
    g.setColour(controlInfo.accentColor);
    g.setFont(juce::Font(26.0f, juce::Font::bold));
    auto titleHeight = 35;
    g.drawText(controlInfo.title, textArea.getX(), currentY, textArea.getWidth(), titleHeight, 
               juce::Justification::left);
    currentY += titleHeight + SECTION_SPACING;
    
    // Overview section
    g.setColour(juce::Colours::white);
    g.setFont(juce::Font(16.0f, juce::Font::bold));
    g.drawText("OVERVIEW", textArea.getX(), currentY, textArea.getWidth(), 22, 
               juce::Justification::left);
    currentY += 25;
    
    g.setColour(juce::Colour(0xFFe0e0e0));
    g.setFont(juce::Font(12.0f, juce::Font::plain));
    auto overviewHeight = 80;
    g.drawFittedText(controlInfo.overview, textArea.getX(), currentY, textArea.getWidth(), overviewHeight, 
                     juce::Justification::topLeft, 5);
    currentY += overviewHeight + SECTION_SPACING;
    
    // Technical Details section
    g.setColour(controlInfo.accentColor.withAlpha(0.9f));
    g.setFont(juce::Font(16.0f, juce::Font::bold));
    g.drawText("TECHNICAL DETAILS", textArea.getX(), currentY, textArea.getWidth(), 22, 
               juce::Justification::left);
    currentY += 25;
    
    g.setColour(juce::Colour(0xFF88aaff));
    g.setFont(juce::Font(11.0f, juce::Font::plain));
    auto techHeight = 80;
    g.drawFittedText(controlInfo.technicalDetails, textArea.getX(), currentY, textArea.getWidth(), techHeight, 
                     juce::Justification::topLeft, 5);
    currentY += techHeight + SECTION_SPACING;
    
    // Practical Usage section
    g.setColour(juce::Colours::white);
    g.setFont(juce::Font(16.0f, juce::Font::bold));
    g.drawText("PRACTICAL USAGE", textArea.getX(), currentY, textArea.getWidth(), 22, 
               juce::Justification::left);
    currentY += 25;
    
    g.setColour(juce::Colour(0xFF88ff88));
    g.setFont(juce::Font(11.0f, juce::Font::plain));
    auto usageHeight = 80;
    g.drawFittedText(controlInfo.practicalUsage, textArea.getX(), currentY, textArea.getWidth(), usageHeight, 
                     juce::Justification::topLeft, 5);
    currentY += usageHeight + SECTION_SPACING;
    
    // Common Settings section
    g.setColour(juce::Colours::white);
    g.setFont(juce::Font(16.0f, juce::Font::bold));
    g.drawText("COMMON SETTINGS", textArea.getX(), currentY, textArea.getWidth(), 22, 
               juce::Justification::left);
    currentY += 25;
    
    g.setColour(juce::Colour(0xFFffaa88));
    g.setFont(juce::Font(11.0f, juce::Font::plain));
    auto settingsHeight = 70;
    g.drawFittedText(controlInfo.commonSettings, textArea.getX(), currentY, textArea.getWidth(), settingsHeight, 
                     juce::Justification::topLeft, 4);
    currentY += settingsHeight + SECTION_SPACING;
    
    // Range information
    g.setColour(juce::Colour(0xFFffaa44));
    g.setFont(juce::Font(14.0f, juce::Font::bold));
    g.drawText("PARAMETER RANGE: " + controlInfo.range, textArea.getX(), currentY, textArea.getWidth(), 22, 
               juce::Justification::left);
    
    // Draw visual representation on the right side
    drawVisualRepresentation(g, visualArea);
}

void ControlInfoWindow::resized()
{
    auto bounds = getLocalBounds();
    
    // Position close button at bottom right (only if it exists)
    if (closeButton)
    {
        closeButton->setBounds(bounds.getWidth() - 100 - MARGIN, bounds.getHeight() - 40 - MARGIN, 100, 35);
    }
}

void ControlInfoWindow::drawVisualRepresentation(juce::Graphics& g, juce::Rectangle<int> area)
{
    // Draw border around visual area
    g.setColour(juce::Colour(0xFF444466));
    g.drawRect(area, 2);
    
    // Draw title for visual section
    g.setColour(juce::Colours::white);
    g.setFont(juce::Font(16.0f, juce::Font::bold));
    g.drawText("VISUAL REPRESENTATION", area.getX() + 10, area.getY() + 10, area.getWidth() - 20, 25, 
               juce::Justification::centred);
    
    auto graphicArea = area.reduced(15).removeFromBottom(area.getHeight() - 50);
    
    // Determine which graphic to draw based on the title
    if (controlInfo.title.contains("PASSBAND"))
        drawPassbandRolloffGraphic(g, graphicArea);
    else if (controlInfo.title.contains("STOPBAND"))
        drawStopbandAttenuationGraphic(g, graphicArea);
    else if (controlInfo.title.contains("TRUE-PEAK"))
        drawTruePeakCeilingGraphic(g, graphicArea);
    else if (controlInfo.title.contains("LOOKAHEAD"))
        drawLookaheadTimeGraphic(g, graphicArea);
    else if (controlInfo.title.contains("OVERSAMPLING"))
        drawOversamplingFactorGraphic(g, graphicArea);
    else if (controlInfo.title.contains("ANTI-ALIASING"))
        drawAntiAliasingFilterGraphic(g, graphicArea);
}

void ControlInfoWindow::drawPassbandRolloffGraphic(juce::Graphics& g, juce::Rectangle<int> area)
{
    // Draw frequency response curves showing different rolloff steepness
    g.setColour(juce::Colour(0xFF333355));
    g.fillRect(area);
    
    // Draw grid
    g.setColour(juce::Colour(0xFF555577));
    for (int i = 1; i < 4; i++)
    {
        int y = area.getY() + (area.getHeight() * i / 4);
        g.drawHorizontalLine(y, area.getX(), area.getRight());
    }
    for (int i = 1; i < 4; i++)
    {
        int x = area.getX() + (area.getWidth() * i / 4);
        g.drawVerticalLine(x, area.getY(), area.getBottom());
    }
    
    // Draw frequency response curves for different rolloff values
    auto centerX = area.getCentreX();
    auto centerY = area.getCentreY();
    
    // Gentle rolloff (0.5)
    g.setColour(juce::Colour(0xFF88ff88));
    juce::Path gentlePath;
    gentlePath.startNewSubPath(area.getX(), centerY - 20);
    for (int x = area.getX(); x < area.getRight(); x += 2)
    {
        float freq = (float)(x - area.getX()) / area.getWidth();
        float response = freq < 0.6f ? 0 : -20 * std::pow((freq - 0.6f) / 0.4f, 1.0f); // Gentle slope
        gentlePath.lineTo(x, centerY + response);
    }
    g.strokePath(gentlePath, juce::PathStrokeType(2.0f));
    
    // Medium rolloff (1.5)
    g.setColour(juce::Colour(0xFF4488ff));
    juce::Path mediumPath;
    mediumPath.startNewSubPath(area.getX(), centerY - 20);
    for (int x = area.getX(); x < area.getRight(); x += 2)
    {
        float freq = (float)(x - area.getX()) / area.getWidth();
        float response = freq < 0.6f ? 0 : -40 * std::pow((freq - 0.6f) / 0.4f, 2.0f); // Steeper slope
        mediumPath.lineTo(x, centerY + response);
    }
    g.strokePath(mediumPath, juce::PathStrokeType(2.0f));
    
    // Sharp rolloff (3.0)
    g.setColour(juce::Colour(0xFFff4444));
    juce::Path sharpPath;
    sharpPath.startNewSubPath(area.getX(), centerY - 20);
    for (int x = area.getX(); x < area.getRight(); x += 2)
    {
        float freq = (float)(x - area.getX()) / area.getWidth();
        float response = freq < 0.6f ? 0 : -60 * std::pow((freq - 0.6f) / 0.4f, 4.0f); // Very steep slope
        sharpPath.lineTo(x, centerY + response);
    }
    g.strokePath(sharpPath, juce::PathStrokeType(2.0f));
    
    // Labels
    g.setColour(juce::Colours::white);
    g.setFont(juce::Font(10.0f));
    g.drawText("Frequency →", area.getX() + 5, area.getBottom() - 20, 80, 15, juce::Justification::left);
    g.drawText("Gentle (0.5)", area.getX() + 5, area.getY() + 5, 80, 15, juce::Justification::left);
    g.setColour(juce::Colour(0xFF88ff88));
    g.fillRect(area.getX() + 85, area.getY() + 9, 10, 3);
    
    g.setColour(juce::Colours::white);
    g.drawText("Medium (1.5)", area.getX() + 5, area.getY() + 25, 80, 15, juce::Justification::left);
    g.setColour(juce::Colour(0xFF4488ff));
    g.fillRect(area.getX() + 85, area.getY() + 29, 10, 3);
    
    g.setColour(juce::Colours::white);
    g.drawText("Sharp (3.0)", area.getX() + 5, area.getY() + 45, 80, 15, juce::Justification::left);
    g.setColour(juce::Colour(0xFFff4444));
    g.fillRect(area.getX() + 85, area.getY() + 49, 10, 3);
}

void ControlInfoWindow::drawStopbandAttenuationGraphic(juce::Graphics& g, juce::Rectangle<int> area)
{
    // Draw spectrum showing aliasing suppression
    g.setColour(juce::Colour(0xFF333355));
    g.fillRect(area);
    
    // Draw frequency spectrum with aliasing components
    auto centerY = area.getCentreY();
    
    // Original signal spectrum
    g.setColour(juce::Colour(0xFF88ff88));
    for (int x = area.getX(); x < area.getCentreX(); x += 3)
    {
        float height = 30 * (1.0f - (float)(x - area.getX()) / (area.getWidth() * 0.5f));
        g.fillRect((float)x, centerY - height/2, 2.0f, height);
    }
    
    // Nyquist frequency line
    g.setColour(juce::Colour(0xFFffaa44));
    g.drawVerticalLine(area.getCentreX(), area.getY(), area.getBottom());
    g.setFont(juce::Font(10.0f));
    g.drawText("Nyquist", area.getCentreX() + 5, area.getY() + 10, 50, 15, juce::Justification::left);
    
    // Aliased components with different attenuation levels
    auto rightArea = area.removeFromRight(area.getWidth() / 2);
    
    // 60dB attenuation
    g.setColour(juce::Colour(0xFFff8888).withAlpha(0.3f));
    for (int x = rightArea.getX(); x < rightArea.getRight(); x += 3)
    {
        float height = 5 * (1.0f - (float)(x - rightArea.getX()) / rightArea.getWidth());
        g.fillRect((float)x, centerY - height/2, 2.0f, height);
    }
    
    // 100dB attenuation
    g.setColour(juce::Colour(0xFFff4444).withAlpha(0.1f));
    for (int x = rightArea.getX(); x < rightArea.getRight(); x += 3)
    {
        float height = 1 * (1.0f - (float)(x - rightArea.getX()) / rightArea.getWidth());
        g.fillRect((float)x, centerY - height/2, 2.0f, height);
    }
    
    // Labels
    g.setColour(juce::Colours::white);
    g.drawText("Original Signal", area.getX() + 5, area.getBottom() - 40, 100, 15, juce::Justification::left);
    g.drawText("Aliased Components", rightArea.getX() + 5, area.getBottom() - 40, 120, 15, juce::Justification::left);
    g.drawText("(Suppressed)", rightArea.getX() + 5, area.getBottom() - 25, 120, 15, juce::Justification::left);
}

void ControlInfoWindow::drawTruePeakCeilingGraphic(juce::Graphics& g, juce::Rectangle<int> area)
{
    // Draw waveform showing intersample peaks
    g.setColour(juce::Colour(0xFF333355));
    g.fillRect(area);
    
    // Draw grid
    g.setColour(juce::Colour(0xFF555577));
    auto centerY = area.getCentreY();
    g.drawHorizontalLine(centerY, area.getX(), area.getRight());
    
    // Draw sample points and reconstructed waveform
    juce::Path waveform;
    juce::Path reconstruction;
    
    waveform.startNewSubPath(area.getX(), centerY);
    reconstruction.startNewSubPath(area.getX(), centerY);
    
    for (int x = area.getX(); x < area.getRight(); x += 2)
    {
        float t = (float)(x - area.getX()) / area.getWidth() * 8.0f; // 8 cycles
        float sample = 0.8f * std::sin(t * 2.0f * juce::MathConstants<float>::pi);
        float reconstructed = 0.9f * std::sin(t * 2.0f * juce::MathConstants<float>::pi + 0.3f); // Slight phase shift creates overshoot
        
        waveform.lineTo(x, centerY - sample * 60);
        reconstruction.lineTo(x, centerY - reconstructed * 60);
    }
    
    // Draw sample waveform
    g.setColour(juce::Colour(0xFF88ff88));
    g.strokePath(waveform, juce::PathStrokeType(2.0f));
    
    // Draw reconstructed waveform (with intersample peaks)
    g.setColour(juce::Colour(0xFFff4444));
    g.strokePath(reconstruction, juce::PathStrokeType(1.5f));
    
    // Draw ceiling lines
    g.setColour(juce::Colour(0xFFffaa44));
    g.drawHorizontalLine(centerY - 48, area.getX(), area.getRight()); // -1dBFS ceiling
    g.drawHorizontalLine(centerY + 48, area.getX(), area.getRight());
    
    // Draw sample points
    g.setColour(juce::Colour(0xFF88ff88));
    for (int x = area.getX(); x < area.getRight(); x += 20)
    {
        float t = (float)(x - area.getX()) / area.getWidth() * 8.0f;
        float sample = 0.8f * std::sin(t * 2.0f * juce::MathConstants<float>::pi);
        g.fillEllipse(x - 2, centerY - sample * 60 - 2, 4, 4);
    }
    
    // Labels
    g.setColour(juce::Colours::white);
    g.setFont(juce::Font(10.0f));
    g.drawText("Sample Points", area.getX() + 5, area.getY() + 5, 100, 15, juce::Justification::left);
    g.setColour(juce::Colour(0xFF88ff88));
    g.fillRect(area.getX() + 105, area.getY() + 9, 10, 3);
    
    g.setColour(juce::Colours::white);
    g.drawText("Reconstructed", area.getX() + 5, area.getY() + 25, 100, 15, juce::Justification::left);
    g.setColour(juce::Colour(0xFFff4444));
    g.fillRect(area.getX() + 105, area.getY() + 29, 10, 3);
    
    g.setColour(juce::Colours::white);
    g.drawText("True-Peak Ceiling", area.getX() + 5, area.getY() + 45, 120, 15, juce::Justification::left);
    g.setColour(juce::Colour(0xFFffaa44));
    g.fillRect(area.getX() + 125, area.getY() + 49, 10, 3);
}

void ControlInfoWindow::drawLookaheadTimeGraphic(juce::Graphics& g, juce::Rectangle<int> area)
{
    // Draw timeline showing lookahead buffer and processing
    g.setColour(juce::Colour(0xFF333355));
    g.fillRect(area);
    
    auto centerY = area.getCentreY();
    auto timelineY = centerY - 40;
    auto bufferY = centerY + 20;
    
    // Draw timeline
    g.setColour(juce::Colours::white);
    g.drawHorizontalLine(timelineY, area.getX(), area.getRight());
    
    // Draw time markers
    for (int i = 0; i <= 4; i++)
    {
        int x = area.getX() + (area.getWidth() * i / 4);
        g.drawVerticalLine(x, timelineY - 5, timelineY + 5);
        g.setFont(juce::Font(9.0f));
        g.drawText(juce::String(i * 2) + "ms", x - 10, timelineY + 10, 20, 15, juce::Justification::centred);
    }
    
    // Draw input signal
    g.setColour(juce::Colour(0xFF88ff88));
    juce::Path inputSignal;
    inputSignal.startNewSubPath(area.getX(), timelineY - 20);
    for (int x = area.getX(); x < area.getRight(); x += 2)
    {
        float t = (float)(x - area.getX()) / area.getWidth() * 6.0f;
        float amplitude = 15 * std::sin(t * 2.0f * juce::MathConstants<float>::pi);
        if (x > area.getX() + area.getWidth() * 0.6f) // Peak at 60% through
            amplitude *= 1.5f; // Sudden peak
        inputSignal.lineTo(x, timelineY - amplitude);
    }
    g.strokePath(inputSignal, juce::PathStrokeType(1.5f));
    
    // Draw lookahead buffer
    auto bufferWidth = area.getWidth() * 0.3f; // 30% of timeline = lookahead
    g.setColour(juce::Colour(0xFFffaa44).withAlpha(0.3f));
    g.fillRect((float)area.getX(), (float)(bufferY - 15), bufferWidth, 30.0f);
    g.setColour(juce::Colour(0xFFffaa44));
    g.drawRect((float)area.getX(), (float)(bufferY - 15), bufferWidth, 30.0f);
    
    // Draw processing point
    auto processX = area.getX() + bufferWidth;
    g.setColour(juce::Colour(0xFFff4444));
    g.drawVerticalLine(processX, area.getY(), area.getBottom());
    
    // Draw output signal (with limiting applied)
    g.setColour(juce::Colour(0xFF4488ff));
    juce::Path outputSignal;
    outputSignal.startNewSubPath(processX, timelineY - 20);
    for (int x = processX; x < area.getRight(); x += 2)
    {
        float t = (float)(x - area.getX()) / area.getWidth() * 6.0f;
        float amplitude = 15 * std::sin(t * 2.0f * juce::MathConstants<float>::pi);
        if (x > area.getX() + area.getWidth() * 0.6f)
            amplitude = juce::jmin(amplitude * 1.5f, 12.0f); // Limited peak
        outputSignal.lineTo(x, timelineY - amplitude);
    }
    g.strokePath(outputSignal, juce::PathStrokeType(1.5f));
    
    // Labels
    g.setColour(juce::Colours::white);
    g.setFont(juce::Font(10.0f));
    g.drawText("Input Signal", area.getX() + 5, area.getY() + 5, 80, 15, juce::Justification::left);
    g.setColour(juce::Colour(0xFF88ff88));
    g.fillRect(area.getX() + 85, area.getY() + 9, 10, 3);
    
    g.setColour(juce::Colours::white);
    g.drawText("Lookahead Buffer", area.getX() + 5, bufferY - 35, 120, 15, juce::Justification::left);
    
    g.drawText("Output (Limited)", processX + 5, area.getY() + 5, 100, 15, juce::Justification::left);
    g.setColour(juce::Colour(0xFF4488ff));
    g.fillRect(processX + 105.0f, (float)(area.getY() + 9), 10.0f, 3.0f);
    
    g.setColour(juce::Colours::white);
    g.drawText("Processing Point", processX - 40, area.getBottom() - 20, 80, 15, juce::Justification::centred);
}

void ControlInfoWindow::drawOversamplingFactorGraphic(juce::Graphics& g, juce::Rectangle<int> area)
{
    g.setColour(juce::Colour(0xFF333355));
    g.fillRect(area);
    
    // Draw different oversampling rates with increasing resolution
    auto centerY = area.getCentreY();
    
    // 1x (original)
    g.setColour(juce::Colour(0xFF88ff88));
    for (int x = area.getX(); x < area.getRight(); x += 8)
    {
        float t = (float)(x - area.getX()) / area.getWidth() * 4.0f;
        float sample = 15 * std::sin(t * 2.0f * juce::MathConstants<float>::pi);
        g.fillEllipse(x - 2, centerY - 40 - sample - 2, 4, 4);
    }
    
    // 4x oversampled
    g.setColour(juce::Colour(0xFFff4444));
    for (int x = area.getX(); x < area.getRight(); x += 2)
    {
        float t = (float)(x - area.getX()) / area.getWidth() * 4.0f;
        float sample = 10 * std::sin(t * 2.0f * juce::MathConstants<float>::pi);
        g.fillEllipse(x - 1, centerY + 20 - sample - 1, 2, 2);
    }
    
    g.setColour(juce::Colours::white);
    g.setFont(juce::Font(10.0f));
    g.drawText("1x Original", area.getX() + 5, area.getY() + 5, 80, 15, juce::Justification::left);
    g.drawText("4x Oversampled", area.getX() + 5, area.getY() + 25, 100, 15, juce::Justification::left);
}

void ControlInfoWindow::drawAntiAliasingFilterGraphic(juce::Graphics& g, juce::Rectangle<int> area)
{
    g.setColour(juce::Colour(0xFF333355));
    g.fillRect(area);
    
    auto centerY = area.getCentreY();
    
    // Linear filter (sharp)
    g.setColour(juce::Colour(0xFF88ff88));
    juce::Path linearPath;
    linearPath.startNewSubPath(area.getX(), centerY - 20);
    for (int x = area.getX(); x < area.getRight(); x += 2)
    {
        float freq = (float)(x - area.getX()) / area.getWidth();
        float response = freq < 0.5f ? 0 : -60 * std::pow((freq - 0.5f) / 0.5f, 4.0f);
        linearPath.lineTo(x, centerY - 20 + response);
    }
    g.strokePath(linearPath, juce::PathStrokeType(2.0f));
    
    // Minimum filter (gentle)
    g.setColour(juce::Colour(0xFF4488ff));
    juce::Path minimumPath;
    minimumPath.startNewSubPath(area.getX(), centerY + 20);
    for (int x = area.getX(); x < area.getRight(); x += 2)
    {
        float freq = (float)(x - area.getX()) / area.getWidth();
        float response = freq < 0.4f ? 0 : -30 * std::pow((freq - 0.4f) / 0.6f, 2.0f);
        minimumPath.lineTo(x, centerY + 20 + response);
    }
    g.strokePath(minimumPath, juce::PathStrokeType(2.0f));
    
    g.setColour(juce::Colours::white);
    g.setFont(juce::Font(10.0f));
    g.drawText("Linear (Sharp)", area.getX() + 5, area.getY() + 5, 100, 15, juce::Justification::left);
    g.drawText("Minimum (Fast)", area.getX() + 5, area.getY() + 25, 100, 15, juce::Justification::left);
}
