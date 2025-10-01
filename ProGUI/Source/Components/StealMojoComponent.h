#pragma once
#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_gui_extra/juce_gui_extra.h>
#include "PsychedelicTheme.h"
#include "../Audio/StemExtractor.h"
#include "../Audio/AudioAnalyzer.h"

// Psychedelic "Steal The Mojo" consumer interface
// Visual style: Peter Max, Yellow Submarine, Austin Powers
// Phase 3: Real stem extraction + Phase 2: Real Mojo analysis
class StealMojoComponent : public juce::Component,
                           public juce::Timer {
public:
    StealMojoComponent()
    {
        // Initialize stem extractor and audio analyzer
        stemExtractor = std::make_unique<StemExtractor>();
        audioAnalyzer = std::make_unique<AudioAnalyzer>();
        
        // Check what extraction methods are available
        if (stemExtractor->isAIExtractionAvailable()) {
            DBG("✅ AI extraction available (Demucs)");
        } else {
            DBG("⚠️ Using DSP fallback only");
        }
        
        // Start animation timer
        startTimer(50); // 20fps for smooth animations
        
        // DON'T load video frames - use simple animation only for instant response
        // loadAnimationFrames();
        
        // Create upload button
        uploadButton = std::make_unique<juce::TextButton>("UPLOAD AUDIO FILE");
        uploadButton->onClick = [this]() { handleUpload(); };
        addAndMakeVisible(*uploadButton);
        
        // Create instrument dropdown
        instrumentCombo = std::make_unique<juce::ComboBox>();
        instrumentCombo->addItem("Vocal", 1);
        instrumentCombo->addItem("Drums", 2);
        instrumentCombo->addItem("Bass", 3);
        instrumentCombo->addItem("Guitar", 4);
        instrumentCombo->addItem("Piano/Keys", 5);
        instrumentCombo->addItem("Strings", 6);
        instrumentCombo->addItem("Brass/Winds", 7);
        instrumentCombo->addItem("Synth", 8);
        instrumentCombo->addItem("Full Mix", 9);
        instrumentCombo->setSelectedId(1);
        instrumentCombo->onChange = [this]() { 
            selectedInstrument = instrumentCombo->getText(); 
        };
        addAndMakeVisible(*instrumentCombo);
        
        // Create analyze button
        analyzeButton = std::make_unique<juce::TextButton>("EXTRACT AND ANALYZE MOJO");
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
        playOriginalButton = std::make_unique<juce::TextButton>("Original");
        playWithMojoButton = std::make_unique<juce::TextButton>("With Mojo");
        playMojoOnlyButton = std::make_unique<juce::TextButton>("Character");
        
        addAndMakeVisible(*playOriginalButton);
        addAndMakeVisible(*playWithMojoButton);
        addAndMakeVisible(*playMojoOnlyButton);
        
        // Create apply button
        applyButton = std::make_unique<juce::TextButton>("APPLY MOJO TO MY TRACK");
        applyButton->onClick = [this]() { handleApply(); };
        addAndMakeVisible(*applyButton);
        
        setSize(700, 1000);
    }
    
    void paint(juce::Graphics& g) override {
        auto bounds = getLocalBounds().toFloat();
        
        // Background with cosmic flow gradient
        auto bgGrad = PsychedelicTheme::createCosmicFlow(bounds);
        g.setGradientFill(bgGrad);
        g.fillAll();
        
        // Add animated stars/sparkles
        drawAnimatedStars(g);
        
        // Step areas (title removed)
        float yPos = 20.0f;
        
        // STEP 1: Upload
        auto step1 = bounds.withY(yPos).withHeight(120.0f).reduced(20.0f, 10.0f);
        PsychedelicTheme::drawSection(g, step1, "STEP 1: UPLOAD REFERENCE AUDIO");
        
        // Show selected file info
        if (selectedFile.existsAsFile()) {
            g.setColour(PsychedelicTheme::Colors::groovyGreen);
            g.setFont(juce::Font(14.0f));
            auto fileInfo = step1.reduced(10.0f).withTop(step1.getY() + 80);
            g.drawText(selectedFile.getFileName(), 
                      fileInfo, juce::Justification::centredLeft);
        }
        
        yPos += 130.0f;
        
        // STEP 2: Choose
        auto step2 = bounds.withY(yPos).withHeight(100.0f).reduced(20.0f, 10.0f);
        PsychedelicTheme::drawSection(g, step2, "STEP 2: CHOOSE THE VIBE");
        
        g.setColour(juce::Colours::white.withAlpha(0.8f));
        g.setFont(juce::Font(13.0f));
        auto chooseText = step2.reduced(10.0f).withTop(step2.getY() + 35);
        g.drawText("Choose the instrument to steal the vibe", 
                  chooseText, juce::Justification::centredLeft);
        
        yPos += 110.0f;
        
        // STEP 3: Extract (BIGGER)
        auto step3 = bounds.withY(yPos).withHeight(340.0f).reduced(20.0f, 10.0f);
        PsychedelicTheme::drawSection(g, step3, "STEP 3: EXTRACT AND ANALYZE THE MOJO");
        
        // Show progress animation or results
        if (isAnalyzing) {
            drawPsychedelicProgress(g, step3.reduced(10.0f).withTop(step3.getY() + 60));
        } else if (isAnalyzed) {
            // Draw spider diagram with mojo characteristics
            auto resultsArea = step3.reduced(10.0f).withTop(step3.getY() + 60);
            
            // Success message at top
            g.setColour(PsychedelicTheme::Colors::groovyGreen);
            g.setFont(juce::Font(28.0f, juce::Font::bold));
            auto msgArea = resultsArea.removeFromTop(40);
            g.drawText("MOJO EXTRACTED!", msgArea, juce::Justification::centred);
            
            // Spider diagram below
            drawMojoRadarWithLabels(g, resultsArea.reduced(0, 10));
        }
        
        yPos += 350.0f;
        
        // STEP 4: Apply (SMALLER)
        auto step4 = bounds.withY(yPos).withHeight(180.0f).reduced(20.0f, 10.0f);
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
        
        // Slider label - BIGGER and more psychedelic
        g.setColour(PsychedelicTheme::Colors::sunshineYellow);
        g.setFont(juce::Font(22.0f, juce::Font::bold));
        auto sliderLabel = step4.reduced(10.0f).withTop(step4.getY() + 110).withHeight(30.0f);
        g.drawText("HOW MUCH MOJO?", sliderLabel, juce::Justification::centred);
        
        // Draw psychedelic slider track
        auto sliderArea = step4.reduced(10.0f).withTop(step4.getY() + 145).withHeight(20.0f);
        drawPsychedelicSlider(g, sliderArea, mojoAmount);
    }
    
    void resized() override {
        auto bounds = getLocalBounds().reduced(20);
        
        // STEP 1: Upload button (adjusted for removed title)
        uploadButton->setBounds(bounds.getX() + 10, 60, 200, 40);
        
        // STEP 2: Instrument combo (adjusted for removed title)
        instrumentCombo->setBounds(bounds.getX() + 10, 205, bounds.getWidth() - 20, 30);
        
        float yPos = 260.0f;
        
        // STEP 3: Analyze button
        yPos += 40.0f;
        analyzeButton->setBounds(bounds.getCentreX() - 130, (int)yPos, 260, 50);
        
        yPos += 350.0f;
        
        // STEP 4: Amount slider
        yPos += 110.0f;
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
    juce::File extractedStemFile;
    juce::String selectedInstrument { "Vocal" };
    bool isAnalyzed { false };
    bool isAnalyzing { false };
    float analysisProgress { 0.0f };
    float mojoAmount { 0.75f };
    float animationPhase { 0.0f };
    
    // Stem extraction and audio analysis system
    std::unique_ptr<StemExtractor> stemExtractor;
    std::unique_ptr<AudioAnalyzer> audioAnalyzer;
    
    // Mojo characteristics (will be populated by analysis)
    float warmth { 0.8f };
    float vintage { 0.6f };
    float punch { 0.7f };
    float space { 0.5f };
    float shimmer { 0.75f };
    
    // Animation frames for progress
    std::vector<juce::Image> animationFrames;
    int totalFrames { 0 };
    
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
        
        DBG("🔮 Starting Mojo extraction for: " + selectedFile.getFileName());
        
        // Start analyzing animation
        isAnalyzing = true;
        isAnalyzed = false;
        analysisProgress = 0.0f;
        repaint();
        
        // Step 1: Extract stem using hybrid system
        auto instrumentType = getInstrumentType(selectedInstrument);
        
        stemExtractor->extractStem(
            selectedFile,
            instrumentType,
            StemExtractor::ExtractionMethod::Auto,
            [this](StemExtractor::ExtractionResult result) {
                juce::MessageManager::callAsync([this, result]() {
                    if (result.success) {
                        extractedStemFile = result.stemFile;
                        DBG("✅ Stem extracted successfully");
                        
                        // Step 2: Analyze the extracted stem for real Mojo values
                        performRealMojoAnalysis();
                    } else {
                        isAnalyzing = false;
                        repaint();
                        DBG("❌ Stem extraction failed: " + result.errorMessage);
                        juce::AlertWindow::showMessageBoxAsync(
                            juce::AlertWindow::WarningIcon,
                            "Extraction Failed",
                            "Could not extract stem: " + result.errorMessage,
                            "OK"
                        );
                    }
                });
            },
            [this](float progress) {
                analysisProgress = progress;
                DBG("Progress: " + juce::String(progress * 100, 1) + "%");
            }
        );
    }
    
    void performRealMojoAnalysis() {
        if (!extractedStemFile.existsAsFile()) {
            DBG("❌ No extracted stem file to analyze");
            return;
        }
        
        DBG("🎵 Analyzing Mojo characteristics...");
        
        // Perform real audio analysis
        audioAnalyzer->analyzeFile(
            extractedStemFile,
            [this](bool success, const AudioAnalyzer::MojoProfile& profile, const AudioAnalyzer::CharacterMetrics& metrics) {
                juce::MessageManager::callAsync([this, success, profile, metrics]() {
                    if (success) {
                        // Update with REAL analysis values
                        warmth = metrics.warmth;
                        vintage = metrics.vintage;
                        punch = metrics.punch;
                        space = metrics.space;
                        shimmer = metrics.shimmer;
                        
                        // Keep animation playing for at least 3 seconds to show the video
                        juce::Timer::callAfterDelay(3000, [this]() {
                            isAnalyzing = false;
                            isAnalyzed = true;
                            repaint();
                        });
                        
                        DBG("✅ Real Mojo Analysis Complete:");
                        DBG("   Warmth: " + juce::String(profile.warmth, 2));
                        DBG("   Punch: " + juce::String(profile.punch, 2));
                        DBG("   Space: " + juce::String(profile.space, 2));
                        DBG("   Shimmer: " + juce::String(profile.shimmer, 2));
                        DBG("   Vintage: " + juce::String(profile.vintage, 2));
                        
                        // No popup - user can see results in spider diagram and go back to main GUI
                    } else {
                        DBG("❌ Mojo analysis failed");
                        juce::AlertWindow::showMessageBoxAsync(
                            juce::AlertWindow::WarningIcon,
                            "Analysis Failed",
                            "Could not analyze audio characteristics",
                            "OK"
                        );
                    }
                });
            }
        );
    }
    
    StemExtractor::InstrumentType getInstrumentType(const juce::String& name) {
        if (name.contains("Vocal")) return StemExtractor::InstrumentType::Vocal;
        if (name.contains("Drums")) return StemExtractor::InstrumentType::Drums;
        if (name.contains("Bass")) return StemExtractor::InstrumentType::Bass;
        if (name.contains("Guitar")) return StemExtractor::InstrumentType::Guitar;
        if (name.contains("Piano")) return StemExtractor::InstrumentType::Piano;
        if (name.contains("Strings")) return StemExtractor::InstrumentType::Strings;
        if (name.contains("Brass")) return StemExtractor::InstrumentType::Brass;
        if (name.contains("Synth")) return StemExtractor::InstrumentType::Synth;
        return StemExtractor::InstrumentType::FullMix;
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
    
    void loadAnimationFrames() {
        // Try multiple locations for the frames
        juce::File resourcesDir;
        
        // 1. Try app bundle Resources folder
        auto appFile = juce::File::getSpecialLocation(juce::File::currentApplicationFile);
        resourcesDir = appFile.getParentDirectory().getParentDirectory()
            .getChildFile("Resources").getChildFile("psychedelic_frames");
        
        DBG("Trying app bundle: " + resourcesDir.getFullPathName());
        
        // 2. If not found, try current working directory
        if (!resourcesDir.exists()) {
            resourcesDir = juce::File::getCurrentWorkingDirectory()
                .getChildFile("Resources")
                .getChildFile("psychedelic_frames");
            DBG("Trying CWD: " + resourcesDir.getFullPathName());
        }
        
        // 3. Try project root
        if (!resourcesDir.exists()) {
            resourcesDir = juce::File("/Volumes/MP 1/MoreMojo/ChatGPT/ProGUI/Resources/psychedelic_frames");
            DBG("Trying absolute path: " + resourcesDir.getFullPathName());
        }
        
        if (!resourcesDir.exists()) {
            DBG("⚠️ Animation frames directory not found at any location");
            return;
        }
        
        DBG("✅ Found frames directory: " + resourcesDir.getFullPathName());
        
        // Load all PNG frames
        for (int i = 1; i <= 63; ++i) {
            auto frameName = juce::String::formatted("frame_%03d.png", i);
            auto frameFile = resourcesDir.getChildFile(frameName);
            
            if (frameFile.existsAsFile()) {
                auto image = juce::ImageFileFormat::loadFrom(frameFile);
                if (image.isValid()) {
                    animationFrames.push_back(image);
                    if (i <= 3) {
                        DBG("Loaded frame " + juce::String(i));
                    }
                }
            }
        }
        
        totalFrames = (int)animationFrames.size();
        DBG("✅ Loaded " + juce::String(totalFrames) + " animation frames total");
    }
    
    void drawVideoFrameProgress(juce::Graphics& g, juce::Rectangle<float> bounds) {
        // Clip to bounds to ensure nothing draws outside
        g.saveState();
        g.reduceClipRegion(bounds.toNearestInt());
        
        // If no frames loaded, this shouldn't be called
        if (animationFrames.empty() || totalFrames == 0) {
            g.restoreState();
            return;
        }
        
        // Continuous looping animation - always loop through frames
        int frameIndex = ((int)(animationPhase * 2.0f)) % totalFrames;
        frameIndex = juce::jlimit(0, totalFrames - 1, frameIndex);
        
        if (frameIndex >= 0 && frameIndex < animationFrames.size()) {
            auto& frame = animationFrames[frameIndex];
            
            // Use ONLY the bounds provided - don't extend beyond
            auto frameArea = bounds;
            
            // Draw the frame within bounds only
            g.drawImage(frame, frameArea, 
                       juce::RectanglePlacement::fillDestination | juce::RectanglePlacement::centred);
            
            // Progress text overlay at bottom
            auto textBounds = bounds;
            g.setColour(juce::Colours::black.withAlpha(0.7f));
            auto textBg = textBounds.removeFromBottom(40);
            g.fillRect(textBg);
            
            g.setColour(juce::Colours::white);
            g.setFont(juce::Font(20.0f, juce::Font::bold));
            g.drawText("Extracting the Vibes... " + 
                      juce::String((int)(analysisProgress * 100)) + "%", 
                      textBg, juce::Justification::centred);
        }
        
        g.restoreState();
    }
    
    void drawPsychedelicProgress(juce::Graphics& g, juce::Rectangle<float> bounds) {
        // Save graphics state and clip to exact bounds
        g.saveState();
        g.reduceClipRegion(bounds.toNearestInt());
        
        // Always use video frames if loaded, otherwise show simple animation immediately
        if (totalFrames > 0 && !animationFrames.empty()) {
            drawVideoFrameProgress(g, bounds);
            g.restoreState();
            return;
        }
        
        // Simple immediate pulsing animation (loads instantly) - CLIPPED to bounds
        auto center = bounds.getCentre();
        float maxRadius = juce::jmin(bounds.getWidth(), bounds.getHeight()) * 0.4f;
        
        // Simple pulsing circle with rainbow colors
        float pulse = 0.5f + 0.5f * std::sin(animationPhase * 3.0f);
        float radius = maxRadius * 0.5f + pulse * maxRadius * 0.3f;
        
        // Rainbow gradient
        juce::ColourGradient grad(
            PsychedelicTheme::Colors::electricPink, center.x, center.y - radius,
            PsychedelicTheme::Colors::psychPurple, center.x, center.y + radius,
            false
        );
        g.setGradientFill(grad);
        g.fillEllipse(center.x - radius, center.y - radius, radius * 2, radius * 2);
        
        // Rotating rings - scaled to fit bounds
        for (int i = 0; i < 3; ++i) {
            float ringPulse = std::sin(animationPhase * 2.0f + i * 0.8f) * 0.5f + 0.5f;
            float ringRadius = maxRadius * (0.6f + i * 0.15f);
            auto color = (i % 2 == 0) ? PsychedelicTheme::Colors::groovyGreen : PsychedelicTheme::Colors::sunshineYellow;
            g.setColour(color.withAlpha(0.4f * ringPulse));
            g.drawEllipse(center.x - ringRadius, center.y - ringRadius, 
                         ringRadius * 2, ringRadius * 2, 3.0f);
        }
        
        // Progress text at bottom (within bounds)
        g.setColour(juce::Colours::white);
        g.setFont(juce::Font(20.0f, juce::Font::bold));
        auto textArea = bounds.removeFromBottom(30);
        g.drawText("Extracting the Vibes...", textArea, juce::Justification::centred);
        
        g.restoreState();
    }
    
    void draw3DPsychedelicMessage(juce::Graphics& g, juce::Rectangle<float> bounds) {
        auto center = bounds.getCentre();
        
        // 3D layered text effect
        juce::String message = "MOJO EXTRACTED!";
        juce::Font font(60.0f, juce::Font::bold);
        
        // Draw multiple shadow layers for 3D depth
        for (int layer = 8; layer > 0; --layer) {
            float offset = layer * 2.0f;
            float alpha = 0.3f * (1.0f - layer / 8.0f);
            
            // Rainbow color based on layer
            juce::Colour layerColor;
            if (layer > 6) layerColor = PsychedelicTheme::Colors::psychPurple;
            else if (layer > 4) layerColor = PsychedelicTheme::Colors::electricPink;
            else if (layer > 2) layerColor = PsychedelicTheme::Colors::cosmicOrange;
            else layerColor = PsychedelicTheme::Colors::sunshineYellow;
            
            g.setColour(layerColor.withAlpha(alpha));
            g.setFont(font);
            g.drawText(message, 
                      bounds.translated(offset, offset), 
                      juce::Justification::centred);
        }
        
        // Top layer with glow
        g.setColour(PsychedelicTheme::Colors::groovyGreen);
        g.setFont(font);
        g.drawText(message, bounds, juce::Justification::centred);
        
        // Pulsing glow effect
        float pulse = 0.5f + 0.5f * std::sin(animationPhase * 3.0f);
        for (int i = 0; i < 3; ++i) {
            float glowRadius = 5.0f + i * 10.0f + pulse * 15.0f;
            g.setColour(PsychedelicTheme::Colors::sunshineYellow.withAlpha(0.2f * pulse));
            auto textBounds = bounds.withSizeKeepingCentre(bounds.getWidth() + glowRadius * 2, 
                                                           bounds.getHeight() + glowRadius * 2);
        }
        
        // Sub-text
        g.setColour(PsychedelicTheme::Colors::skyBlue);
        g.setFont(juce::Font(24.0f, juce::Font::italic));
        auto subText = bounds.withTop(bounds.getCentreY() + 40);
        g.drawText("The vibes are ready to apply!", subText, juce::Justification::centred);
    }
    
    void drawPsychedelicSlider(juce::Graphics& g, juce::Rectangle<float> bounds, float value) {
        // Rainbow gradient track
        juce::ColourGradient trackGrad(
            PsychedelicTheme::Colors::electricPink, bounds.getX(), bounds.getCentreY(),
            PsychedelicTheme::Colors::psychPurple, bounds.getRight(), bounds.getCentreY(),
            false
        );
        trackGrad.addColour(0.33, PsychedelicTheme::Colors::sunshineYellow);
        trackGrad.addColour(0.66, PsychedelicTheme::Colors::groovyGreen);
        
        g.setGradientFill(trackGrad);
        g.fillRoundedRectangle(bounds.reduced(0, 5), 5.0f);
        
        // Pulsing thumb
        float thumbX = bounds.getX() + (bounds.getWidth() * value);
        float pulse = 0.5f + 0.5f * std::sin(animationPhase * 4.0f);
        float thumbSize = 15.0f + pulse * 5.0f;
        
        // Glow behind thumb
        for (int i = 0; i < 3; ++i) {
            float glowSize = thumbSize + i * 8.0f;
            g.setColour(PsychedelicTheme::Colors::cosmicOrange.withAlpha(0.3f * (1.0f - i / 3.0f)));
            g.fillEllipse(thumbX - glowSize / 2, bounds.getCentreY() - glowSize / 2, 
                         glowSize, glowSize);
        }
        
        // Thumb
        g.setColour(juce::Colours::white);
        g.fillEllipse(thumbX - thumbSize / 2, bounds.getCentreY() - thumbSize / 2, 
                     thumbSize, thumbSize);
    }
    
    void drawMojoRadarWithLabels(juce::Graphics& g, juce::Rectangle<float> bounds) {
        auto center = bounds.getCentre();
        float radius = juce::jmin(bounds.getWidth(), bounds.getHeight()) * 0.35f;
        
        // Label info
        struct AxisLabel {
            juce::String name;
            float value;
        };
        
        AxisLabel labels[] = {
            {"Warmth", warmth},
            {"Shimmer", shimmer},
            {"Space", space},
            {"Punch", punch},
            {"Vintage", vintage}
        };
        
        // Draw web concentric circles
        g.setColour(PsychedelicTheme::Colors::psychPurple.withAlpha(0.3f));
        for (int i = 1; i <= 4; i++) {
            g.drawEllipse(center.x - radius * i / 4.0f, 
                         center.y - radius * i / 4.0f,
                         radius * 2.0f * i / 4.0f, 
                         radius * 2.0f * i / 4.0f, 
                         1.0f);
        }
        
        // Draw axes and labels
        const int numAxes = 5;
        for (int i = 0; i < numAxes; i++) {
            float angle = (float)i / (float)numAxes * juce::MathConstants<float>::twoPi - juce::MathConstants<float>::halfPi;
            juce::Point<float> axisEnd(center.x + std::cos(angle) * radius,
                                      center.y + std::sin(angle) * radius);
            
            // Draw axis line
            g.setColour(PsychedelicTheme::Colors::psychPurple.withAlpha(0.5f));
            g.drawLine(center.x, center.y, axisEnd.x, axisEnd.y, 1.0f);
            
            // Draw label
            juce::Point<float> labelPos(center.x + std::cos(angle) * (radius + 30),
                                       center.y + std::sin(angle) * (radius + 30));
            g.setColour(PsychedelicTheme::Colors::sunshineYellow);
            g.setFont(juce::Font(12.0f, juce::Font::bold));
            g.drawText(labels[i].name, 
                      juce::Rectangle<float>(labelPos.x - 40, labelPos.y - 10, 80, 20),
                      juce::Justification::centred);
        }
        
        // Draw Mojo profile polygon
        juce::Path mojoPath;
        for (int i = 0; i < numAxes; i++) {
            float angle = (float)i / (float)numAxes * juce::MathConstants<float>::twoPi - juce::MathConstants<float>::halfPi;
            float r = labels[i].value * radius;
            juce::Point<float> point(center.x + std::cos(angle) * r,
                                    center.y + std::sin(angle) * r);
            
            if (i == 0)
                mojoPath.startNewSubPath(point);
            else
                mojoPath.lineTo(point);
        }
        mojoPath.closeSubPath();
        
        // Fill with animated glow
        float glowPulse = 0.5f + 0.3f * std::sin(animationPhase * 2.0f);
        g.setColour(PsychedelicTheme::Colors::groovyGreen.withAlpha(0.3f + glowPulse * 0.2f));
        g.fillPath(mojoPath);
        
        // Stroke with bright line
        g.setColour(PsychedelicTheme::Colors::groovyGreen);
        g.strokePath(mojoPath, juce::PathStrokeType(3.0f));
        
        // Draw center marker with pulse
        float centerSize = 8.0f + glowPulse * 4.0f;
        g.setColour(PsychedelicTheme::Colors::electricPink);
        g.fillEllipse(center.x - centerSize/2, center.y - centerSize/2, centerSize, centerSize);
    }
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(StealMojoComponent)
};
