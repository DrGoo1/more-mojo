#pragma once
#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_gui_extra/juce_gui_extra.h>
#include "PsychedelicTheme.h"

// Psychedelic "Steal The Mojo" consumer interface
// Visual style: Peter Max, Yellow Submarine, Austin Powers
class PsychedelicStealMojoComponent : public juce::Component,
                                       public juce::Timer {
public:
    PsychedelicStealMojoComponent()
    {
        // Start animation timer
        startTimer(50); // 20fps for smooth animations
        
        // Create upload button
        uploadButton = std::make_unique<juce::TextButton>("📁 UPLOAD AUDIO FILE");
        uploadButton->onClick = [this]() { handleUpload(); };
        addAndMakeVisible(*uploadButton);
        
        // Create instrument dropdown
        instrumentCombo = std::make_unique<juce::ComboBox>();
        instrumentCombo->addItem("🎤 Vocal", 1);
        instrumentCombo->addItem("🥁 Drums", 2);
        instrumentCombo->addItem("🎸 Bass", 3);
        instrumentCombo->addItem("🎸 Guitar", 4);
        instrumentCombo->addItem("🎹 Piano/Keys", 5);
        instrumentCombo->addItem("🎻 Strings", 6);
        instrumentCombo->addItem("🎺 Brass/Winds", 7);
        instrumentCombo->addItem("🎛️ Synth", 8);
        instrumentCombo->addItem("🎵 Full Mix", 9);
        instrumentCombo->setSelectedId(1);
        instrumentCombo->onChange = [this]() { 
            selectedInstrument = instrumentCombo->getText(); 
        };
        addAndMakeVisible(*instrumentCombo);
        
        // Create analyze button
        analyzeButton = std::make_unique<juce::TextButton>("🔮 ANALYZE MOJO");
        analyzeButton->onClick = [this]() { handleAnalyze(); };
        addAndMakeVisible(*analyzeButton);
        
        // Create amount slider
        amountSlider = std::make_unique<juce::Slider>(juce::Slider::LinearHorizontal,
                                                      juce::Slider::NoTextBox);
        amountSlider->setRange(0.0, 150.0, 1.0);
        amountSlider->setValue(75.0);
        amountSlider->onValueChange = [this]() {
            mojoAmount = (float)amountSlider->getValue() / 100.0f;
            repaint();
        };
        addAndMakeVisible(*amountSlider);
        
        // Create preview buttons
        playOriginalButton = std::make_unique<juce::TextButton>("▶ Original");
        playWithMojoButton = std::make_unique<juce::TextButton>("▶ With Mojo");
        playMojoOnlyButton = std::make_unique<juce::TextButton>("▶ Mojo Only");
        
        addAndMakeVisible(*playOriginalButton);
        addAndMakeVisible(*playWithMojoButton);
        addAndMakeVisible(*playMojoOnlyButton);
        
        // Create apply button
        applyButton = std::make_unique<juce::TextButton>("🎨 APPLY MOJO TO MY TRACK");
        applyButton->onClick = [this]() { handleApply(); };
        addAndMakeVisible(*applyButton);
        
        setSize(700, 900);
    }
    
    void paint(juce::Graphics& g) override {
        auto bounds = getLocalBounds().toFloat();
        
        // Background with cosmic flow gradient
        auto bgGrad = PsychedelicTheme::createCosmicFlow(bounds);
        g.setGradientFill(bgGrad);
        g.fillAll();
        
        // Add animated stars/sparkles
        drawAnimatedStars(g);
        
        // Title area
        auto titleArea = bounds.removeFromTop(100.0f).reduced(20.0f);
        PsychedelicTheme::drawPsychedelicTitle(g, titleArea, "STEAL THE MOJO");
        
        // Subtitle
        g.setColour(PsychedelicTheme::Colors::sunshineYellow);
        g.setFont(juce::Font(18.0f, juce::Font::italic));
        auto subtitleArea = titleArea.withY(titleArea.getBottom() - 25);
        g.drawText("Grab the vibe from any track!", subtitleArea, juce::Justification::centred);
        
        // Step areas
        float yPos = 120.0f;
        
        // STEP 1: Upload
        auto step1 = bounds.withY(yPos).withHeight(120.0f).reduced(20.0f, 10.0f);
        PsychedelicTheme::drawSection(g, step1, "STEP 1: UPLOAD REFERENCE AUDIO");
        
        // Show selected file info
        if (selectedFile.existsAsFile()) {
            g.setColour(PsychedelicTheme::Colors::groovyGreen);
            g.setFont(juce::Font(14.0f));
            auto fileInfo = step1.reduced(10.0f).withTop(step1.getY() + 80);
            g.drawText("🎵 " + selectedFile.getFileName(), 
                      fileInfo, juce::Justification::centredLeft);
        }
        
        yPos += 130.0f;
        
        // STEP 2: Choose
        auto step2 = bounds.withY(yPos).withHeight(100.0f).reduced(20.0f, 10.0f);
        PsychedelicTheme::drawSection(g, step2, "STEP 2: CHOOSE THE VIBE");
        
        g.setColour(juce::Colours::white.withAlpha(0.8f));
        g.setFont(juce::Font(13.0f));
        auto chooseText = step2.reduced(10.0f).withTop(step2.getY() + 35);
        g.drawText("What kind of mojo are you stealing?", 
                  chooseText, juce::Justification::centredLeft);
        
        yPos += 110.0f;
        
        // STEP 3: Extract
        auto step3 = bounds.withY(yPos).withHeight(340.0f).reduced(20.0f, 10.0f);
        PsychedelicTheme::drawSection(g, step3, "STEP 3: EXTRACT THE MOJO");
        
        // Mojo analysis results (if analyzed)
        if (isAnalyzed) {
            drawMojoRadar(g, step3.reduced(10.0f).withTop(step3.getY() + 80).withHeight(150.0f));
            drawCharacterMeters(g, step3.reduced(10.0f).withTop(step3.getY() + 240));
        }
        
        yPos += 350.0f;
        
        // STEP 4: Apply
        auto step4 = bounds.withY(yPos).withHeight(240.0f).reduced(20.0f, 10.0f);
        PsychedelicTheme::drawSection(g, step4, "STEP 4: APPLY THE MOJO");
        
        // Amount display
        g.setColour(PsychedelicTheme::Colors::electricPink);
        g.setFont(juce::Font(48.0f, juce::Font::bold));
        auto amountArea = step4.reduced(10.0f).withTop(step4.getY() + 40).withHeight(60.0f);
        g.drawText(juce::String((int)(mojoAmount * 100.0f)) + "%", 
                  amountArea, juce::Justification::centred);
        
        // Draw rainbow halo around amount
        if (animationPhase > 0.0f) {
            auto haloCenter = amountArea.getCentre();
            float intensity = 0.5f + 0.5f * std::sin(animationPhase * 2.0f);
            PsychedelicTheme::drawGlowHalo(g, amountArea, 
                                          PsychedelicTheme::Colors::electricPink, 
                                          intensity);
        }
        
        // Slider label
        g.setColour(juce::Colours::white.withAlpha(0.7f));
        g.setFont(juce::Font(13.0f));
        auto sliderLabel = step4.reduced(10.0f).withTop(step4.getY() + 110).withHeight(20.0f);
        g.drawText("How much mojo do you want?", sliderLabel, juce::Justification::centred);
    }
    
    void resized() override {
        auto bounds = getLocalBounds().reduced(20);
        float yPos = 120.0f;
        
        // STEP 1: Upload button
        yPos += 40.0f;
        uploadButton->setBounds(bounds.getX() + 10, (int)yPos, 200, 40);
        
        yPos += 130.0f;
        
        // STEP 2: Instrument combo
        yPos += 60.0f;
        instrumentCombo->setBounds(bounds.getX() + 10, (int)yPos, 250, 30);
        
        yPos += 110.0f;
        
        // STEP 3: Analyze button
        yPos += 40.0f;
        analyzeButton->setBounds(bounds.getCentreX() - 100, (int)yPos, 200, 50);
        
        yPos += 350.0f;
        
        // STEP 4: Amount slider
        yPos += 130.0f;
        amountSlider->setBounds(bounds.getX() + 40, (int)yPos, bounds.getWidth() - 80, 30);
        
        yPos += 40.0f;
        
        // Preview buttons
        int buttonWidth = 120;
        int spacing = 10;
        int startX = bounds.getCentreX() - (3 * buttonWidth + 2 * spacing) / 2;
        
        playOriginalButton->setBounds(startX, (int)yPos, buttonWidth, 30);
        playWithMojoButton->setBounds(startX + buttonWidth + spacing, (int)yPos, buttonWidth, 30);
        playMojoOnlyButton->setBounds(startX + 2 * (buttonWidth + spacing), (int)yPos, buttonWidth, 30);
        
        yPos += 45.0f;
        
        // Apply button
        applyButton->setBounds(bounds.getCentreX() - 150, (int)yPos, 300, 60);
    }
    
    void timerCallback() override {
        // Update animation phase
        animationPhase += 0.1f;
        if (animationPhase > juce::MathConstants<float>::twoPi) {
            animationPhase -= juce::MathConstants<float>::twoPi;
        }
        
        repaint(); // Smooth animation
    }

private:
    // UI Components
    std::unique_ptr<juce::TextButton> uploadButton;
    std::unique_ptr<juce::ComboBox> instrumentCombo;
    std::unique_ptr<juce::TextButton> analyzeButton;
    std::unique_ptr<juce::Slider> amountSlider;
    std::unique_ptr<juce::TextButton> playOriginalButton;
    std::unique_ptr<juce::TextButton> playWithMojoButton;
    std::unique_ptr<juce::TextButton> playMojoOnlyButton;
    std::unique_ptr<juce::TextButton> applyButton;
    
    // State
    juce::File selectedFile;
    juce::String selectedInstrument { "🎤 Vocal" };
    bool isAnalyzed { false };
    float mojoAmount { 0.75f };
    float animationPhase { 0.0f };
    
    // Mojo characteristics (will be populated by analysis)
    float warmth { 0.8f };
    float vintage { 0.6f };
    float punch { 0.7f };
    float space { 0.5f };
    float shimmer { 0.75f };
    
    void handleUpload() {
        auto chooser = std::make_shared<juce::FileChooser>(
            "Select audio file",
            juce::File(),
            "*.wav;*.aif;*.aiff;*.mp3;*.flac"
        );
        
        auto flags = juce::FileBrowserComponent::openMode | 
                    juce::FileBrowserComponent::canSelectFiles;
        
        chooser->launchAsync(flags, [this, chooser](const juce::FileChooser& fc) {
            auto file = fc.getResult();
            if (file.existsAsFile()) {
                selectedFile = file;
                isAnalyzed = false;
                repaint();
            }
        });
    }
    
    void handleAnalyze() {
        if (!selectedFile.existsAsFile()) {
            juce::AlertWindow::showMessageBoxAsync(
                juce::AlertWindow::WarningIcon,
                "No File Selected",
                "Please upload an audio file first!",
                "OK"
            );
            return;
        }
        
        // TODO: Actual Mojo analysis
        // For now, simulate with random values
        warmth = juce::Random::getSystemRandom().nextFloat();
        vintage = juce::Random::getSystemRandom().nextFloat();
        punch = juce::Random::getSystemRandom().nextFloat();
        space = juce::Random::getSystemRandom().nextFloat();
        shimmer = juce::Random::getSystemRandom().nextFloat();
        
        isAnalyzed = true;
        repaint();
        
        juce::AlertWindow::showMessageBoxAsync(
            juce::AlertWindow::InfoIcon,
            "Mojo Extracted!",
            "The mojo has been analyzed from " + selectedFile.getFileName(),
            "Groovy!"
        );
    }
    
    void handleApply() {
        if (!isAnalyzed) {
            juce::AlertWindow::showMessageBoxAsync(
                juce::AlertWindow::WarningIcon,
                "No Mojo Extracted",
                "Please analyze the audio first!",
                "OK"
            );
            return;
        }
        
        // TODO: Actually apply Mojo to target track
        juce::AlertWindow::showMessageBoxAsync(
            juce::AlertWindow::InfoIcon,
            "Mojo Applied!",
            "The groovy vibes have been applied at " + 
            juce::String((int)(mojoAmount * 100)) + "% amount!",
            "Far Out!"
        );
    }
    
    void drawAnimatedStars(juce::Graphics& g) {
        // Draw twinkling stars/sparkles in background
        juce::Random random(12345);
        g.setColour(juce::Colours::white.withAlpha(0.3f));
        
        for (int i = 0; i < 50; i++) {
            float x = random.nextFloat() * getWidth();
            float y = random.nextFloat() * getHeight();
            float twinkle = std::sin(animationPhase + i * 0.5f) * 0.5f + 0.5f;
            float size = 1.0f + twinkle * 2.0f;
            
            g.setColour(juce::Colours::white.withAlpha(twinkle * 0.5f));
            g.fillEllipse(x, y, size, size);
        }
    }
    
    void drawMojoRadar(juce::Graphics& g, juce::Rectangle<float> bounds) {
        auto center = bounds.getCentre();
        float radius = juce::jmin(bounds.getWidth(), bounds.getHeight()) * 0.4f;
        
        // Draw concentric circles
        g.setColour(PsychedelicTheme::Colors::psychPurple.withAlpha(0.3f));
        for (int i = 1; i <= 3; i++) {
            g.drawEllipse(center.x - radius * i / 3.0f, 
                         center.y - radius * i / 3.0f,
                         radius * 2.0f * i / 3.0f, 
                         radius * 2.0f * i / 3.0f, 
                         1.0f);
        }
        
        // Draw axes
        g.setColour(PsychedelicTheme::Colors::psychPurple.withAlpha(0.5f));
        const int numAxes = 8;
        for (int i = 0; i < numAxes; i++) {
            float angle = (float)i / (float)numAxes * juce::MathConstants<float>::twoPi;
            juce::Point<float> end(center.x + std::cos(angle) * radius,
                                  center.y + std::sin(angle) * radius);
            g.drawLine(center.x, center.y, end.x, end.y, 1.0f);
        }
        
        // Draw Mojo profile polygon (simplified 5-point version)
        juce::Path mojoPath;
        float values[] = { warmth, vintage, punch, space, shimmer };
        
        for (int i = 0; i < 5; i++) {
            float angle = (float)i / 5.0f * juce::MathConstants<float>::twoPi - juce::MathConstants<float>::halfPi;
            float r = values[i] * radius;
            juce::Point<float> point(center.x + std::cos(angle) * r,
                                    center.y + std::sin(angle) * r);
            
            if (i == 0)
                mojoPath.startNewSubPath(point);
            else
                mojoPath.lineTo(point);
        }
        mojoPath.closeSubPath();
        
        // Fill with glow
        g.setColour(PsychedelicTheme::Colors::groovyGreen.withAlpha(0.3f));
        g.fillPath(mojoPath);
        
        // Stroke with bright line
        g.setColour(PsychedelicTheme::Colors::groovyGreen);
        g.strokePath(mojoPath, juce::PathStrokeType(2.0f));
        
        // Draw center marker
        g.setColour(PsychedelicTheme::Colors::electricPink);
        g.fillEllipse(center.x - 5, center.y - 5, 10, 10);
    }
    
    void drawCharacterMeters(juce::Graphics& g, juce::Rectangle<float> bounds) {
        float meterHeight = 25.0f;
        float spacing = 5.0f;
        float yPos = bounds.getY();
        
        auto meterBounds = bounds.withHeight(meterHeight);
        
        PsychedelicTheme::drawPsychedelicMeter(g, meterBounds.withY(yPos), 
                                               warmth, "Warmth", "🔥");
        yPos += meterHeight + spacing;
        
        PsychedelicTheme::drawPsychedelicMeter(g, meterBounds.withY(yPos), 
                                               vintage, "Vintage", "📻");
        yPos += meterHeight + spacing;
        
        PsychedelicTheme::drawPsychedelicMeter(g, meterBounds.withY(yPos), 
                                               punch, "Punch", "💥");
        yPos += meterHeight + spacing;
        
        PsychedelicTheme::drawPsychedelicMeter(g, meterBounds.withY(yPos), 
                                               space, "Space", "🌌");
        yPos += meterHeight + spacing;
        
        PsychedelicTheme::drawPsychedelicMeter(g, meterBounds.withY(yPos), 
                                               shimmer, "Shimmer", "✨");
    }
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PsychedelicStealMojoComponent)
};
