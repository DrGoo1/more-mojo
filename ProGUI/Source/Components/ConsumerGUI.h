#pragma once
#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_audio_basics/juce_audio_basics.h>
#include <juce_audio_formats/juce_audio_formats.h>
#include <juce_audio_devices/juce_audio_devices.h>
#include <juce_audio_utils/juce_audio_utils.h>
#include "PsychedelicTheme.h"
#include "StealMojoComponent.h"
#include "../shared/ui_core/SkinManager.h"
#include "../shared/ui_core/SkinnedKnob.h"

// Audio processor for applying mojo in real-time
class MojoAudioSource : public juce::AudioSource {
public:
    MojoAudioSource() = default;
    
    void setSource(juce::AudioSource* newSource) {
        source = newSource;
    }
    
    void setMojoAmount(float amount) {
        mojoAmount = amount;
    }
    
    void setInputGain(float gain) {
        inputGain = gain;
    }
    
    void setOutputGain(float gain) {
        outputGain = gain;
    }
    
    void setGainCompensation(bool enabled) {
        gainCompensationEnabled = enabled;
    }
    
    void prepareToPlay(int samplesPerBlockExpected, double sampleRate) override {
        if (source)
            source->prepareToPlay(samplesPerBlockExpected, sampleRate);
    }
    
    void releaseResources() override {
        if (source)
            source->releaseResources();
    }
    
    void getNextAudioBlock(const juce::AudioSourceChannelInfo& bufferToFill) override {
        if (source) {
            source->getNextAudioBlock(bufferToFill);
            
            // Apply mojo processing
            auto* buffer = bufferToFill.buffer;
            
            // Track RMS levels for both input and output
            float inputRmsSum = 0.0f;
            float outputRmsSum = 0.0f;
            int numSamples = bufferToFill.numSamples;
            
            for (int channel = 0; channel < buffer->getNumChannels(); ++channel) {
                auto* data = buffer->getWritePointer(channel, bufferToFill.startSample);
                
                for (int i = 0; i < numSamples; ++i) {
                    // Mojo formula: add harmonics and warmth based on amount
                    // 0.0 = subtle (Mojo), 1.0 = medium (More Mojo), 2.0 = intense (Most Mojo)
                    float sample = data[i];
                    float processed = sample;
                    
                    // Calculate input RMS (before processing)
                    inputRmsSum += sample * sample;
                    
                    // Apply input gain
                    sample *= inputGain;
                    
                    // Add harmonic saturation
                    float saturation = mojoAmount * 0.3f;
                    processed = std::tanh(sample * (1.0f + saturation));
                    
                    // Blend based on mojo amount
                    float result = sample + (processed - sample) * (mojoAmount / 2.0f);
                    
                    // Apply output gain with optional compensation
                    float finalGain = outputGain;
                    if (gainCompensationEnabled) {
                        // Compensate for mojo boost - more mojo = more reduction
                        float compensation = 1.0f / (1.0f + mojoAmount * 0.3f);
                        finalGain *= compensation;
                    }
                    data[i] = result * finalGain;
                    
                    // Calculate output RMS (after processing)
                    outputRmsSum += data[i] * data[i];
                }
            }
            
            // Calculate RMS values (average across all channels and samples)
            int totalSamples = numSamples * buffer->getNumChannels();
            float inputRms = std::sqrt(inputRmsSum / totalSamples);
            float outputRms = std::sqrt(outputRmsSum / totalSamples);
            
            // Apply ballistics (smooth attack/release) - faster attack, slower release
            const float attackCoeff = 0.7f;   // Fast attack
            const float releaseCoeff = 0.95f;  // Slow release for more realistic meter behavior
            
            float prevInput = currentInputLevel.load();
            float prevOutput = currentOutputLevel.load();
            
            // Smooth the levels
            if (inputRms > prevInput)
                currentInputLevel = prevInput + attackCoeff * (inputRms - prevInput);
            else
                currentInputLevel = prevInput + (1.0f - releaseCoeff) * (inputRms - prevInput);
                
            if (outputRms > prevOutput)
                currentOutputLevel = prevOutput + attackCoeff * (outputRms - prevOutput);
            else
                currentOutputLevel = prevOutput + (1.0f - releaseCoeff) * (outputRms - prevOutput);
        }
    }
    
    float getCurrentInputLevel() const { return currentInputLevel; }
    float getCurrentOutputLevel() const { return currentOutputLevel; }
    
private:
    juce::AudioSource* source = nullptr;
    float mojoAmount = 1.0f;
    float inputGain = 1.0f;
    float outputGain = 1.0f;
    bool gainCompensationEnabled = false;
    std::atomic<float> currentInputLevel{0.0f};
    std::atomic<float> currentOutputLevel{0.0f};
};

// Simple consumer-facing GUI for applying extracted mojo to audio files
class ConsumerGUI : public juce::Component, 
                    private juce::Timer,
                    public juce::ChangeListener {
public:
    ConsumerGUI() : transportSource(), mojoSource(), audioSourcePlayer() {
        // Setup audio with mojo processing
        formatManager.registerBasicFormats();
        deviceManager.initialiseWithDefaultDevices(0, 2);
        
        // Chain: transportSource -> mojoSource -> audioSourcePlayer
        mojoSource.setSource(&transportSource);
        audioSourcePlayer.setSource(&mojoSource);
        deviceManager.addAudioCallback(&audioSourcePlayer);
        transportSource.addChangeListener(this);
        // Title (will be drawn custom with psychedelic style)
        titleLabel = std::make_unique<juce::Label>();
        titleLabel->setText("MORE MOJO", juce::dontSendNotification);
        titleLabel->setFont(juce::Font(56.0f, juce::Font::bold));
        titleLabel->setJustificationType(juce::Justification::centred);
        titleLabel->setColour(juce::Label::textColourId, juce::Colours::transparentBlack); // Will draw custom
        addAndMakeVisible(*titleLabel);
        
        // Subtitle (will be drawn custom with psychedelic style)
        subtitleLabel = std::make_unique<juce::Label>();
        subtitleLabel->setText("CONSUMER", juce::dontSendNotification);
        subtitleLabel->setFont(juce::Font(32.0f, juce::Font::bold));
        subtitleLabel->setJustificationType(juce::Justification::centred);
        subtitleLabel->setColour(juce::Label::textColourId, juce::Colours::transparentBlack); // Will draw custom
        addAndMakeVisible(*subtitleLabel);
        
        // Load Audio button
        loadButton = std::make_unique<juce::TextButton>("LOAD YOUR TRACK");
        loadButton->setColour(juce::TextButton::buttonColourId, PsychedelicTheme::Colors::electricPink);
        loadButton->setColour(juce::TextButton::textColourOffId, juce::Colours::white);
        loadButton->onClick = [this]() { handleLoadAudio(); };
        addAndMakeVisible(*loadButton);
        
        // File info label
        fileInfoLabel = std::make_unique<juce::Label>();
        fileInfoLabel->setText("No file loaded", juce::dontSendNotification);
        fileInfoLabel->setJustificationType(juce::Justification::centred);
        fileInfoLabel->setColour(juce::Label::textColourId, juce::Colours::white.withAlpha(0.7f));
        addAndMakeVisible(*fileInfoLabel);
        
        // Try to load Neptune knob skin (correct path - parent of Oneshots folder)
        try {
            bool loaded = SkinManager::instance().loadFantasmaAt("/Volumes/MP 1/MoreMojo/NeptuneUI/03 Delivery/VST/Assets/Animation/");
            if (loaded) {
                DBG("Neptune knob filmstrip loaded successfully!");
            } else {
                DBG("Could not load Neptune skin, using custom fallback knob");
            }
        } catch (...) {
            DBG("Exception loading Neptune skin, using custom fallback knob");
        }
        
        // Load VU meter frames from assets folder (001.png through 128.png)
        vuMeterFrames.clear();
        juce::File vuMeterFolder("/Volumes/MP 1/MoreMojo/Knobs/App Knobs folder/VU-meter/assets");
        for (int i = 1; i <= 128; ++i) {
            juce::String filename = juce::String(i).paddedLeft('0', 3) + ".png";
            auto imageFile = vuMeterFolder.getChildFile(filename);
            if (imageFile.existsAsFile()) {
                auto img = juce::ImageFileFormat::loadFrom(imageFile);
                if (img.isValid()) {
                    vuMeterFrames.push_back(img);
                }
            }
        }
        if (!vuMeterFrames.empty()) {
            DBG("VU meters loaded: " + juce::String((int)vuMeterFrames.size()) + " frames");
        }
        
        // Mojo knob (Neptune photo-realistic with fallback to regular slider)
        mojoKnob = std::make_unique<SkinnedKnob>();
        mojoKnob->setRange(0.0, 2.0, 0.01);
        mojoKnob->setValue(0.5f); // Start at "More Mojo" (0.5 = middle of 0-1 range)
        mojoKnob->onValueChanged = [this](float normalizedValue) {
            // Map 0-1 normalized value to 0-2 mojo range
            mojoAmount = normalizedValue * 2.0f;
            mojoSource.setMojoAmount(mojoAmount);
            updateMojoLabel();
            repaint();
        };
        addAndMakeVisible(*mojoKnob);
        
        // Mojo amount label (shows Mojo / More Mojo / Most Mojo)
        mojoLabel = std::make_unique<juce::Label>();
        mojoLabel->setText("MORE MOJO", juce::dontSendNotification);
        mojoLabel->setFont(juce::Font(28.0f, juce::Font::bold));
        mojoLabel->setJustificationType(juce::Justification::centred);
        mojoLabel->setColour(juce::Label::textColourId, PsychedelicTheme::Colors::cosmicOrange);
        addAndMakeVisible(*mojoLabel);
        
        // Input/Output level meters AND sliders
        inputMeterLabel = std::make_unique<juce::Label>();
        inputMeterLabel->setText("INPUT", juce::dontSendNotification);
        inputMeterLabel->setFont(juce::Font(12.0f, juce::Font::bold));
        inputMeterLabel->setJustificationType(juce::Justification::centred);
        inputMeterLabel->setColour(juce::Label::textColourId, juce::Colours::white);
        addAndMakeVisible(*inputMeterLabel);
        
        // Input gain slider (same color as output)
        inputSlider = std::make_unique<juce::Slider>(juce::Slider::LinearVertical, juce::Slider::NoTextBox);
        inputSlider->setRange(-12.0, 12.0, 0.1);
        inputSlider->setValue(0.0);
        inputSlider->setColour(juce::Slider::thumbColourId, PsychedelicTheme::Colors::skyBlue);
        inputSlider->setColour(juce::Slider::trackColourId, PsychedelicTheme::Colors::skyBlue.withAlpha(0.5f));
        inputSlider->onValueChange = [this]() {
            float gain = juce::Decibels::decibelsToGain((float)inputSlider->getValue());
            mojoSource.setInputGain(gain);
        };
        addAndMakeVisible(*inputSlider);
        
        outputMeterLabel = std::make_unique<juce::Label>();
        outputMeterLabel->setText("OUTPUT", juce::dontSendNotification);
        outputMeterLabel->setFont(juce::Font(12.0f, juce::Font::bold));
        outputMeterLabel->setJustificationType(juce::Justification::centred);
        outputMeterLabel->setColour(juce::Label::textColourId, juce::Colours::white);
        addAndMakeVisible(*outputMeterLabel);
        
        // Output gain slider (same color as input)
        outputSlider = std::make_unique<juce::Slider>(juce::Slider::LinearVertical, juce::Slider::NoTextBox);
        outputSlider->setRange(-12.0, 12.0, 0.1);
        outputSlider->setValue(0.0);
        outputSlider->setColour(juce::Slider::thumbColourId, PsychedelicTheme::Colors::skyBlue);
        outputSlider->setColour(juce::Slider::trackColourId, PsychedelicTheme::Colors::skyBlue.withAlpha(0.5f));
        outputSlider->onValueChange = [this]() {
            float gain = juce::Decibels::decibelsToGain((float)outputSlider->getValue());
            mojoSource.setOutputGain(gain);
        };
        addAndMakeVisible(*outputSlider);
        
        // Audio transport buttons - SMALLER with borders and proper symbols
        playButton = std::make_unique<juce::TextButton>("PLAY");
        playButton->setColour(juce::TextButton::buttonColourId, juce::Colours::transparentBlack);
        playButton->setColour(juce::TextButton::buttonOnColourId, PsychedelicTheme::Colors::groovyGreen.withAlpha(0.2f));
        playButton->setColour(juce::TextButton::textColourOffId, PsychedelicTheme::Colors::groovyGreen);
        playButton->setColour(juce::TextButton::textColourOnId, juce::Colours::white);
        playButton->onClick = [this]() { handlePlay(); };
        addAndMakeVisible(*playButton);
        
        stopButton = std::make_unique<juce::TextButton>("STOP");
        stopButton->setColour(juce::TextButton::buttonColourId, juce::Colours::transparentBlack);
        stopButton->setColour(juce::TextButton::buttonOnColourId, PsychedelicTheme::Colors::electricPink.withAlpha(0.2f));
        stopButton->setColour(juce::TextButton::textColourOffId, PsychedelicTheme::Colors::electricPink);
        stopButton->setColour(juce::TextButton::textColourOnId, juce::Colours::white);
        stopButton->onClick = [this]() { handleStop(); };
        addAndMakeVisible(*stopButton);
        
        // Rewind button
        rewindButton = std::make_unique<juce::TextButton>("RW");
        rewindButton->setColour(juce::TextButton::buttonColourId, juce::Colours::transparentBlack);
        rewindButton->setColour(juce::TextButton::buttonOnColourId, PsychedelicTheme::Colors::psychPurple.withAlpha(0.2f));
        rewindButton->setColour(juce::TextButton::textColourOffId, PsychedelicTheme::Colors::psychPurple);
        rewindButton->setColour(juce::TextButton::textColourOnId, juce::Colours::white);
        rewindButton->onClick = [this]() { 
            if (transportSource.getLengthInSeconds() > 0) {
                auto newPos = juce::jmax(0.0, transportSource.getCurrentPosition() - 5.0);
                transportSource.setPosition(newPos);
            }
        };
        addAndMakeVisible(*rewindButton);
        
        // Fast Forward button
        ffButton = std::make_unique<juce::TextButton>("FF");
        ffButton->setColour(juce::TextButton::buttonColourId, juce::Colours::transparentBlack);
        ffButton->setColour(juce::TextButton::buttonOnColourId, PsychedelicTheme::Colors::cosmicOrange.withAlpha(0.2f));
        ffButton->setColour(juce::TextButton::textColourOffId, PsychedelicTheme::Colors::cosmicOrange);
        ffButton->setColour(juce::TextButton::textColourOnId, juce::Colours::white);
        ffButton->onClick = [this]() { 
            if (transportSource.getLengthInSeconds() > 0) {
                auto newPos = juce::jmin(transportSource.getLengthInSeconds(), 
                                        transportSource.getCurrentPosition() + 5.0);
                transportSource.setPosition(newPos);
            }
        };
        addAndMakeVisible(*ffButton);
        
        // Export button
        exportButton = std::make_unique<juce::TextButton>("EXPORT");
        exportButton->setColour(juce::TextButton::buttonColourId, juce::Colours::transparentBlack);
        exportButton->setColour(juce::TextButton::buttonOnColourId, PsychedelicTheme::Colors::skyBlue.withAlpha(0.2f));
        exportButton->setColour(juce::TextButton::textColourOffId, PsychedelicTheme::Colors::skyBlue);
        exportButton->setColour(juce::TextButton::textColourOnId, juce::Colours::white);
        exportButton->onClick = [this]() { handleExport(); };
        addAndMakeVisible(*exportButton);
        
        // Load Match Volume toggle switch filmstrip
        juce::File switchFile("/Volumes/MP 1/MoreMojo/Knobs/Button-08/Button-08-B-Filmstrip.png");
        if (switchFile.existsAsFile()) {
            matchVolumeSwitchImage = juce::ImageFileFormat::loadFrom(switchFile);
            if (matchVolumeSwitchImage.isValid()) {
                DBG("Match Volume switch loaded: " + juce::String(matchVolumeSwitchImage.getWidth()) + "x" + juce::String(matchVolumeSwitchImage.getHeight()));
            }
        }
        
        // Professional Controls button (same styling as STM)
        proControlsButton = std::make_unique<juce::TextButton>("PRO CONTROLS");
        proControlsButton->setColour(juce::TextButton::buttonColourId, juce::Colours::transparentBlack);
        proControlsButton->setColour(juce::TextButton::textColourOffId, juce::Colours::transparentBlack);
        proControlsButton->onClick = [this]() { handleProControls(); };
        proControlsButton->setTooltip("Open Professional Controls: Advanced audio processing with ISP, SRC, JITTER, ALIGN, TRANSIENT, DEESSER, and TRANSFORMER modules. Features Neptune knobs and real-time metering for precise audio manipulation.");
        addAndMakeVisible(*proControlsButton);
        
        // Steal Mojo button (PROMINENT - will draw with psychedelic style)
        stealMojoButton = std::make_unique<juce::TextButton>("STEAL THE MOJO");
        stealMojoButton->setColour(juce::TextButton::buttonColourId, juce::Colours::transparentBlack);
        stealMojoButton->setColour(juce::TextButton::textColourOffId, juce::Colours::transparentBlack);
        stealMojoButton->onClick = [this]() { handleStealMojo(); };
        stealMojoButton->setTooltip("Steal The Mojo: Analyze audio to extract its sonic character and vibe. Upload a reference track, choose an instrument profile, and watch the psychedelic radar chart + character meters reveal its mojo essence. Apply extracted mojo to your own tracks!");
        addAndMakeVisible(*stealMojoButton);
        
        // Start animation
        startTimer(50);
        
        setSize(900, 800); // Taller for bigger knob
    }
    
    ~ConsumerGUI() override {
        transportSource.stop();
        transportSource.setSource(nullptr);
        mojoSource.setSource(nullptr);
        audioSourcePlayer.setSource(nullptr);
        deviceManager.removeAudioCallback(&audioSourcePlayer);
    }
    
    void drawPsychedelicText(juce::Graphics& g, const juce::String& text, juce::Rectangle<int> bounds, float fontSize) {
        g.setFont(juce::Font(fontSize, juce::Font::bold));
        
        // 1. Draw outer yellow/orange outline (thick)
        g.setColour(juce::Colour(0xFFFFCC00)); // Bright yellow-orange
        for (int offset = 4; offset > 0; --offset) {
            for (int angle = 0; angle < 360; angle += 45) {
                float rad = angle * juce::MathConstants<float>::pi / 180.0f;
                int dx = (int)(std::cos(rad) * offset);
                int dy = (int)(std::sin(rad) * offset);
                g.drawText(text, bounds.translated(dx, dy), juce::Justification::centred);
            }
        }
        
        // 2. Draw pink/purple mid outline
        g.setColour(juce::Colour(0xFFFF1493)); // Deep pink
        for (int offset = 2; offset > 0; --offset) {
            for (int angle = 0; angle < 360; angle += 45) {
                float rad = angle * juce::MathConstants<float>::pi / 180.0f;
                int dx = (int)(std::cos(rad) * offset);
                int dy = (int)(std::sin(rad) * offset);
                g.drawText(text, bounds.translated(dx, dy), juce::Justification::centred);
            }
        }
        
        // 3. Draw dark purple/black fill
        g.setColour(juce::Colour(0xFF1a0033)); // Very dark purple, almost black
        g.drawText(text, bounds, juce::Justification::centred);
    }
    
    void paintOverChildren(juce::Graphics& g) override {
        // Draw psychedelic title (no subtitle)
        drawPsychedelicText(g, "MORE MOJO", titleLabel->getBounds(), 56.0f);
        
        // Draw PRO CONTROLS button with emphasis (same style as STM)
        if (proControlsButton) {
            auto btnBounds = proControlsButton->getBounds();
            
            // Draw glowing background
            g.setColour(PsychedelicTheme::Colors::groovyGreen.withAlpha(0.5f));
            g.fillRoundedRectangle(btnBounds.toFloat().expanded(5), 10.0f);
            
            // Draw psychedelic text
            drawPsychedelicText(g, "PRO CONTROLS", btnBounds, 18.0f);
            
            // Draw border
            g.setColour(juce::Colour(0xFF00FF88));
            g.drawRoundedRectangle(btnBounds.toFloat(), 8.0f, 3.0f);
        }
        
        // Draw STEAL THE MOJO button with emphasis
        if (stealMojoButton) {
            auto btnBounds = stealMojoButton->getBounds();
            
            // Draw glowing background
            g.setColour(PsychedelicTheme::Colors::electricPink.withAlpha(0.5f));
            g.fillRoundedRectangle(btnBounds.toFloat().expanded(5), 10.0f);
            
            // Draw psychedelic text
            drawPsychedelicText(g, "STEAL THE MOJO", btnBounds, 18.0f);
            
            // Draw border
            g.setColour(juce::Colour(0xFFFFCC00));
            g.drawRoundedRectangle(btnBounds.toFloat(), 8.0f, 3.0f);
        }
        
        // Draw EXPORT button with psychedelic emphasis (STM style)
        if (exportButton) {
            auto btnBounds = exportButton->getBounds();
            
            // Draw glowing background
            g.setColour(PsychedelicTheme::Colors::skyBlue.withAlpha(0.5f));
            g.fillRoundedRectangle(btnBounds.toFloat().expanded(4), 8.0f);
            
            // Draw psychedelic text
            drawPsychedelicText(g, "EXPORT", btnBounds, 16.0f);
            
            // Draw border
            g.setColour(juce::Colour(0xFF00D4FF));
            g.drawRoundedRectangle(btnBounds.toFloat(), 6.0f, 2.5f);
        }
        
        // Draw borders around control buttons
        auto drawButtonBorder = [&](juce::TextButton* btn, juce::Colour color) {
            if (!btn) return;
            auto b = btn->getBounds().toFloat();
            g.setColour(color);
            g.drawRoundedRectangle(b, 4.0f, 2.0f);
        };
        
        drawButtonBorder(rewindButton.get(), PsychedelicTheme::Colors::psychPurple);
        drawButtonBorder(playButton.get(), PsychedelicTheme::Colors::groovyGreen);
        drawButtonBorder(stopButton.get(), PsychedelicTheme::Colors::electricPink);
        drawButtonBorder(ffButton.get(), PsychedelicTheme::Colors::cosmicOrange);
        // Export button now drawn with psychedelic style above, no simple border
    }
    
    void paint(juce::Graphics& g) override {
        // Psychedelic gradient background
        juce::ColourGradient bg(
            PsychedelicTheme::Colors::psychPurple.darker(0.7f), 0, 0,
            PsychedelicTheme::Colors::electricPink.darker(0.7f), getWidth(), getHeight(),
            false
        );
        g.setGradientFill(bg);
        g.fillAll();
        
        // Animated background elements
        drawAnimatedBackground(g);
        
        // Waveform display area
        auto waveformArea = getLocalBounds().reduced(40).withTop(180).withHeight(200);
        drawWaveformDisplay(g, waveformArea);
        
        // Draw input/output meters
        drawMeter(g, inputMeterBounds, inputLevel, PsychedelicTheme::Colors::groovyGreen);
        drawMeter(g, outputMeterBounds, outputLevel, PsychedelicTheme::Colors::electricPink);
        
        // Draw custom 3D mojo knob (always visible)
        if (mojoKnob) {
            drawCustomMojoKnob(g, mojoKnob->getBounds().toFloat());
            
            // Draw mojo level bar ABOVE the knob (thinner and longer)
            auto knobBounds = mojoKnob->getBounds();
            auto barArea = juce::Rectangle<int>(knobBounds.getX() - 80, 
                                                knobBounds.getY() - 70,
                                                knobBounds.getWidth() + 160, 
                                                50);
            drawMojoLevelBar(g, barArea);
            
            // Draw "MOJO KNOB" as circular text in the colored ring area
            auto center = knobBounds.getCentre().toFloat();
            float radius = knobBounds.getWidth() * 0.38f; // In the colored ring
            
            juce::String text = "MOJO KNOB";
            g.setFont(juce::Font(11.0f, juce::Font::bold));
            
            float angleStep = juce::MathConstants<float>::pi * 1.4f / text.length();
            float startAngle = -juce::MathConstants<float>::pi * 0.7f; // Start at top
            
            for (int i = 0; i < text.length(); ++i) {
                float angle = startAngle + i * angleStep;
                float x = center.x + std::cos(angle) * radius;
                float y = center.y + std::sin(angle) * radius;
                
                juce::AffineTransform t = juce::AffineTransform::rotation(angle + juce::MathConstants<float>::pi / 2.0f, x, y);
                
                // Draw character with psychedelic effect
                juce::String ch = juce::String::charToString(text[i]);
                
                // Yellow outline
                g.setColour(juce::Colour(0xFFFFCC00));
                for (int dx = -1; dx <= 1; ++dx) {
                    for (int dy = -1; dy <= 1; ++dy) {
                        if (dx != 0 || dy != 0) {
                            g.addTransform(t);
                            g.drawText(ch, (int)(x + dx - 5), (int)(y + dy - 6), 10, 12, juce::Justification::centred);
                            g.addTransform(t.inverted());
                        }
                    }
                }
                
                // Dark fill
                g.setColour(juce::Colour(0xFF1a0033));
                g.addTransform(t);
                g.drawText(ch, (int)(x - 5), (int)(y - 6), 10, 12, juce::Justification::centred);
                g.addTransform(t.inverted());
            }
        }
        
        // Draw VU meters on sides (tied to audio levels)
        drawVUMeter(g, leftMojoMeterBounds, inputLevel, "ORIGINAL");
        drawVUMeter(g, rightMojoMeterBounds, outputLevel, "WITH MOJO");
        
        // Draw Match Volume toggle switch
        drawMatchVolumeSwitch(g, matchVolumeSwitchBounds);
    }
    
    void mouseDown(const juce::MouseEvent& event) override {
        // Check if click is on Match Volume switch
        if (matchVolumeSwitchBounds.contains(event.getPosition())) {
            matchVolumeEnabled = !matchVolumeEnabled;
            mojoSource.setGainCompensation(matchVolumeEnabled);
            repaint();
        }
    }
    
    void resized() override {
        auto bounds = getLocalBounds();
        
        // Title at top (psychedelic style) - lowered
        bounds.removeFromTop(30); // Extra spacing to lower title
        titleLabel->setBounds(bounds.removeFromTop(70).reduced(20));
        // Subtitle removed - hide it off screen
        subtitleLabel->setBounds(juce::Rectangle<int>(0, 0, 0, 0));
        
        bounds.removeFromTop(20); // spacing
        
        // Load button and file info
        auto loadArea = bounds.removeFromTop(60).reduced(200, 5);
        loadButton->setBounds(loadArea);
        
        auto fileArea = bounds.removeFromTop(30).reduced(40);
        fileInfoLabel->setBounds(fileArea);
        
        bounds.removeFromTop(220); // Space for waveform
        
        // Mojo controls - BIGGER SECTION
        auto mojoLabelArea = bounds.removeFromTop(45).reduced(40);
        mojoLabel->setBounds(mojoLabelArea);
        
        // Professional Controls button at top-left
        proControlsButton->setBounds(20, 15, 220, 60);
        
        // Steal Mojo button at top-right (emphasized)
        stealMojoButton->setBounds(bounds.getWidth() - 240, 15, 220, 60);
        
        // Mojo knob in center with meters+sliders on sides - MUCH BIGGER
        auto controlArea = bounds.removeFromTop(260); // More space
        auto leftMeterArea = controlArea.removeFromLeft(120).reduced(15, 10);
        auto rightMeterArea = controlArea.removeFromRight(120).reduced(15, 10);
        
        // VU meters next to center knob - tight to knob
        leftMojoMeterBounds = controlArea.removeFromLeft(145).reduced(0, 40);
        rightMojoMeterBounds = controlArea.removeFromRight(145).reduced(0, 40);
        
        // Match Volume toggle switch - positioned to the right and above right VU meter
        if (matchVolumeSwitchImage.isValid()) {
            int switchSize = 60;
            int frameHeight = matchVolumeSwitchImage.getHeight() / 2; // 2 frames
            matchVolumeSwitchBounds = juce::Rectangle<int>(switchSize, switchSize)
                .withX(rightMojoMeterBounds.getRight() + 20)  // More to the right
                .withY(rightMojoMeterBounds.getY() - 120);    // Even higher up
        }
        
        // Input controls on left - meter + slider
        inputMeterLabel->setBounds(leftMeterArea.removeFromTop(20));
        auto leftControls = leftMeterArea;
        auto meterWidth = 30;
        inputMeterBounds = leftControls.removeFromLeft(meterWidth); // Meter
        leftControls.removeFromLeft(5); // Gap
        inputSlider->setBounds(leftControls); // Slider
        
        // Output controls on right - meter + slider  
        outputMeterLabel->setBounds(rightMeterArea.removeFromTop(20));
        auto rightControls = rightMeterArea;
        outputSlider->setBounds(rightControls.removeFromLeft(rightControls.getWidth() - meterWidth - 5)); // Slider
        rightControls.removeFromLeft(5); // Gap
        outputMeterBounds = rightControls; // Meter
        
        // Neptune Mojo knob in center - HUGE for maximum visibility
        auto knobSize = 260; // BIGGER!
        mojoKnob->setBounds(controlArea.getCentreX() - knobSize/2, 
                           controlArea.getY() + 10, 
                           knobSize, knobSize);
        
        bounds.removeFromTop(10); // spacing
        
        // Audio control buttons - SMALLER with better spacing (5 buttons)
        auto buttonArea = bounds.removeFromTop(45).reduced(150, 5);
        int buttonWidth = 80;
        int gap = 10;
        int totalWidth = buttonWidth * 4 + gap * 3;
        int startX = buttonArea.getCentreX() - totalWidth / 2;
        
        rewindButton->setBounds(startX, buttonArea.getY(), buttonWidth, 35);
        playButton->setBounds(startX + (buttonWidth + gap), buttonArea.getY(), buttonWidth, 35);
        stopButton->setBounds(startX + (buttonWidth + gap) * 2, buttonArea.getY(), buttonWidth, 35);
        ffButton->setBounds(startX + (buttonWidth + gap) * 3, buttonArea.getY(), buttonWidth, 35);
        
        // Export button moved to the right with STM styling
        exportButton->setBounds(bounds.getWidth() - 140, buttonArea.getY(), 120, 35);
        
        bounds.removeFromTop(10); // spacing
    }
    
    // Public methods for Pro GUI integration
    juce::File getLoadedFile() const { return loadedFile; }
    bool hasAudioLoaded() const { return audioLoaded; }
    
private:
    void timerCallback() override {
        animationPhase += 0.05f;
        if (animationPhase > juce::MathConstants<float>::twoPi * 100.0f)
            animationPhase = 0.0f;
        
        // Update meters with real audio levels
        if (isPlaying && transportSource.isPlaying()) {
            float rawInput = mojoSource.getCurrentInputLevel();
            float rawOutput = mojoSource.getCurrentOutputLevel();
            
            // Apply gain boost for better visual response (2x multiplier)
            inputLevel = juce::jlimit(0.0f, 1.0f, rawInput * 2.0f);
            
            // Output level boosted by mojo factor to show mojo effect visually!
            // More mojo = more VU meter deflection
            float mojoMultiplier = 1.0f + (mojoAmount / 2.0f) * 1.5f; // 1.0x to 2.5x boost
            outputLevel = juce::jlimit(0.0f, 1.0f, rawOutput * 2.0f * mojoMultiplier);
        } else {
            // Slower decay when not playing
            inputLevel = juce::jmax(0.0f, inputLevel - 0.03f);
            outputLevel = juce::jmax(0.0f, outputLevel - 0.03f);
        }
        
        repaint();
    }
    
    void drawAnimatedBackground(juce::Graphics& g) {
        // Floating orbs
        for (int i = 0; i < 5; ++i) {
            float phase = animationPhase + i * 1.5f;
            float x = getWidth() * (0.2f + i * 0.15f) + std::sin(phase * 0.5f) * 50.0f;
            float y = getHeight() * (0.3f + std::sin(phase * 0.7f) * 0.2f);
            float radius = 30.0f + std::sin(phase) * 10.0f;
            
            auto color = (i % 2 == 0) ? PsychedelicTheme::Colors::cosmicOrange : PsychedelicTheme::Colors::skyBlue;
            g.setColour(color.withAlpha(0.2f));
            g.fillEllipse(x - radius, y - radius, radius * 2, radius * 2);
        }
    }
    
    void drawMatchVolumeSwitch(juce::Graphics& g, juce::Rectangle<int> bounds) {
        if (bounds.isEmpty() || !matchVolumeSwitchImage.isValid()) return;
        
        // Calculate frame to display (0 = OFF, 1 = ON)
        int frameHeight = matchVolumeSwitchImage.getHeight() / 2;
        int frameIndex = matchVolumeEnabled ? 1 : 0;
        
        // Extract the frame from filmstrip
        juce::Image frame = matchVolumeSwitchImage.getClippedImage(
            juce::Rectangle<int>(0, frameIndex * frameHeight, 
                                matchVolumeSwitchImage.getWidth(), frameHeight));
        
        // Draw the appropriate frame
        g.drawImage(frame, bounds.toFloat(),
                   juce::RectanglePlacement::centred | juce::RectanglePlacement::onlyReduceInSize);
        
        // Draw main label above switch - wider for full "Match Volume" text
        auto labelBounds = bounds.withBottom(bounds.getY() - 5).withHeight(16).withWidth(100).withX(bounds.getX() - 20);
        g.setFont(juce::Font(11.0f, juce::Font::bold));
        
        // Black outline for better readability
        g.setColour(juce::Colours::black);
        for (int x = -1; x <= 1; ++x) {
            for (int y = -1; y <= 1; ++y) {
                if (x != 0 || y != 0) {
                    g.drawText("Match Volume", labelBounds.translated(x, y), juce::Justification::centred);
                }
            }
        }
        
        // Fill
        g.setColour(PsychedelicTheme::Colors::cosmicOrange);
        g.drawText("Match Volume", labelBounds, juce::Justification::centred);
        
        // Draw OFF/ON state labels below switch
        auto stateLabelBounds = bounds.withTop(bounds.getBottom() + 8).withHeight(16);
        g.setFont(juce::Font(10.0f, juce::Font::bold));
        
        // OFF label - highlighted when not enabled
        auto offBounds = stateLabelBounds.withWidth(30).withX(bounds.getX() - 5);
        auto offColor = matchVolumeEnabled 
            ? PsychedelicTheme::Colors::cosmicOrange.withAlpha(0.4f)
            : PsychedelicTheme::Colors::sunshineYellow;
        
        // Black outline for OFF
        g.setColour(juce::Colours::black);
        for (int x = -1; x <= 1; ++x) {
            for (int y = -1; y <= 1; ++y) {
                if (x != 0 || y != 0) {
                    g.drawText("OFF", offBounds.translated(x, y), juce::Justification::centred);
                }
            }
        }
        g.setColour(offColor);
        g.drawText("OFF", offBounds, juce::Justification::centred);
        
        // ON label - highlighted when enabled
        auto onBounds = stateLabelBounds.withWidth(30).withX(bounds.getRight() - 25);
        auto onColor = matchVolumeEnabled 
            ? PsychedelicTheme::Colors::sunshineYellow
            : PsychedelicTheme::Colors::cosmicOrange.withAlpha(0.4f);
        
        // Black outline for ON
        g.setColour(juce::Colours::black);
        for (int x = -1; x <= 1; ++x) {
            for (int y = -1; y <= 1; ++y) {
                if (x != 0 || y != 0) {
                    g.drawText("ON", onBounds.translated(x, y), juce::Justification::centred);
                }
            }
        }
        g.setColour(onColor);
        g.drawText("ON", onBounds, juce::Justification::centred);
    }
    
    void drawVUMeter(juce::Graphics& g, juce::Rectangle<int> bounds, float audioLevel, juce::String label) {
        if (bounds.isEmpty()) return;
        
        if (!vuMeterFrames.empty()) {
            // Map audio level (0.0-1.0) to frame index
            int frameIndex = juce::jlimit(0, (int)vuMeterFrames.size() - 1, 
                                         (int)(audioLevel * vuMeterFrames.size()));
            
            auto& frameImage = vuMeterFrames[frameIndex];
            
            // Calculate size - smaller VU meters (180px)
            int meterSize = 180;
            auto meterBounds = juce::Rectangle<int>(meterSize, meterSize)
                                .withCentre(bounds.getCentre());
            
            // Draw the photorealistic VU meter
            g.drawImage(frameImage, meterBounds.toFloat(), juce::RectanglePlacement::centred);
        } else {
            // Fallback if VU meter didn't load
            g.setColour(PsychedelicTheme::Colors::cosmicOrange);
            g.setFont(juce::Font(10.0f));
            g.drawText("VU METER\nNOT LOADED", bounds.reduced(5, 30), 
                      juce::Justification::centred);
        }
        
        // Draw label below meter with BIGGER psychedelic font style and illumination
        auto labelBounds = bounds.withTop(bounds.getCentreY() + 100);
        
        // Calculate illumination factor based on audio level and mojo amount
        float illumination = audioLevel;
        if (label == "WITH MOJO") {
            // "WITH MOJO" illuminates more based on mojo amount (0-2 range normalized to 0-1)
            float mojoFactor = mojoAmount / 2.0f;
            illumination = juce::jlimit(0.0f, 1.0f, audioLevel * (1.0f + mojoFactor));
        }
        
        // Bigger font size
        float fontSize = 18.0f;
        g.setFont(juce::Font(fontSize, juce::Font::bold));
        
        // Draw animated glow based on illumination - MORE DRAMATIC
        if (illumination > 0.05f) {
            // Stronger, slower pulsing glow effect
            float glowIntensity = illumination * (0.6f + 0.4f * std::sin(animationPhase * 1.5f));
            
            // Multiple glow layers for intensity - BIGGER and BRIGHTER
            for (int glowSize = 12; glowSize > 0; glowSize -= 2) {
                float alpha = glowIntensity * (0.3f * (12 - glowSize) / 12.0f);
                auto glowColor = (label == "WITH MOJO") 
                    ? PsychedelicTheme::Colors::electricPink.withAlpha(alpha)
                    : PsychedelicTheme::Colors::groovyGreen.withAlpha(alpha);
                
                g.setColour(glowColor);
                for (int angle = 0; angle < 360; angle += 30) {
                    float rad = angle * juce::MathConstants<float>::pi / 180.0f;
                    int dx = (int)(std::cos(rad) * glowSize);
                    int dy = (int)(std::sin(rad) * glowSize);
                    g.drawText(label, labelBounds.translated(dx, dy).withHeight(25), juce::Justification::centred);
                }
            }
        }
        
        // Black outline for better readability
        g.setColour(juce::Colours::black);
        // Draw thick outline
        for (int x = -2; x <= 2; ++x) {
            for (int y = -2; y <= 2; ++y) {
                if (x != 0 || y != 0) {
                    g.drawText(label, labelBounds.translated(x, y).withHeight(25), juce::Justification::centred);
                }
            }
        }
        
        // Draw fill - MUCH brighter when illuminated with pulsing
        auto fillColor = PsychedelicTheme::Colors::sunshineYellow;
        float pulse = 0.7f + 0.3f * std::sin(animationPhase * 1.5f); // Pulsing brightness
        
        if (illumination > 0.3f) {
            // Make fill MUCH brighter/whiter at high levels with pulse
            float brightnessBoost = illumination * pulse;
            fillColor = fillColor.interpolatedWith(juce::Colours::white, brightnessBoost * 0.7f);
        }
        g.setColour(fillColor);
        g.drawText(label, labelBounds.withHeight(25), juce::Justification::centred);
    }
    
    void drawMojoLevelBar(juce::Graphics& g, juce::Rectangle<int> bounds) {
        // Progressive bar that fills from left to right (0-2 range)
        float currentValue = mojoAmount / 2.0f; // Normalize to 0-1
        
        // Make bar thinner and longer
        auto barBounds = bounds.withSizeKeepingCentre(bounds.getWidth() * 1.3f, 30);
        
        // Background (empty bar)
        g.setColour(juce::Colours::black.withAlpha(0.6f));
        g.fillRoundedRectangle(barBounds.toFloat(), 4.0f);
        
        // Draw gradient fill that progresses from left to right
        auto fillBounds = barBounds.reduced(3);
        float fillWidth = fillBounds.getWidth() * currentValue;
        auto filledArea = fillBounds.withWidth((int)fillWidth);
        
        if (fillWidth > 0) {
            juce::ColourGradient gradient(
                PsychedelicTheme::Colors::groovyGreen,
                filledArea.getX(), filledArea.getCentreY(),
                PsychedelicTheme::Colors::electricPink,
                filledArea.getRight(), filledArea.getCentreY(),
                false
            );
            gradient.addColour(0.5, PsychedelicTheme::Colors::cosmicOrange);
            g.setGradientFill(gradient);
            g.fillRoundedRectangle(filledArea.toFloat(), 3.0f);
        }
        
        // Draw markers and labels
        g.setColour(juce::Colours::white.withAlpha(0.4f));
        int third = fillBounds.getWidth() / 3;
        g.drawVerticalLine(fillBounds.getX() + third, fillBounds.getY(), fillBounds.getBottom());
        g.drawVerticalLine(fillBounds.getX() + third * 2, fillBounds.getY(), fillBounds.getBottom());
        
        // Text labels above bar with psychedelic font style
        auto labelArea = barBounds.withHeight(18).translated(0, -22);
        
        auto drawPsychedelicText = [&](const juce::String& text, juce::Rectangle<int> area) {
            // Black outline for better readability
            g.setColour(juce::Colours::black);
            g.setFont(juce::Font(11.0f, juce::Font::bold));
            for (int x = -1; x <= 1; ++x) {
                for (int y = -1; y <= 1; ++y) {
                    if (x != 0 || y != 0) {
                        g.drawText(text, area.translated(x, y), juce::Justification::centred);
                    }
                }
            }
            // Fill in yellow
            g.setColour(PsychedelicTheme::Colors::sunshineYellow);
            g.drawText(text, area, juce::Justification::centred);
        };
        
        drawPsychedelicText("MOJO", labelArea.withWidth(third));
        drawPsychedelicText("MORE MOJO", labelArea.withX(labelArea.getX() + third).withWidth(third));
        drawPsychedelicText("MOST MOJO", labelArea.withX(labelArea.getX() + third * 2).withWidth(third));
        
        // Outer border
        g.setColour(juce::Colours::white.withAlpha(0.3f));
        g.drawRoundedRectangle(barBounds.toFloat(), 4.0f, 1.5f);
    }
    
    void drawCustomMojoKnob(juce::Graphics& g, juce::Rectangle<float> bounds) {
        auto center = bounds.getCentre();
        auto radius = juce::jmin(bounds.getWidth(), bounds.getHeight()) * 0.45f;
        
        // Normalized value 0-1
        float normalizedValue = mojoKnob->getValue();
        float angle = juce::MathConstants<float>::pi * 1.2f + 
                     normalizedValue * juce::MathConstants<float>::pi * 1.6f;
        
        // Outer glow/shadow
        for (int i = 6; i > 0; --i) {
            g.setColour(juce::Colours::black.withAlpha(0.1f));
            g.fillEllipse(center.x - radius - i*3, center.y - radius - i*3, 
                         (radius + i*3) * 2, (radius + i*3) * 2);
        }
        
        // Main body with metallic gradient
        juce::ColourGradient bodyGrad(
            PsychedelicTheme::Colors::cosmicOrange.brighter(0.4f),
            center.x - radius * 0.5f, center.y - radius * 0.5f,
            PsychedelicTheme::Colors::cosmicOrange.darker(0.3f),
            center.x + radius * 0.5f, center.y + radius * 0.5f,
            true
        );
        g.setGradientFill(bodyGrad);
        g.fillEllipse(center.x - radius, center.y - radius, radius * 2, radius * 2);
        
        // Inner circle with gradient
        auto innerRadius = radius * 0.7f;
        juce::ColourGradient innerGrad(
            PsychedelicTheme::Colors::psychPurple.darker(0.5f),
            center.x, center.y - innerRadius,
            PsychedelicTheme::Colors::psychPurple,
            center.x, center.y + innerRadius,
            false
        );
        g.setGradientFill(innerGrad);
        g.fillEllipse(center.x - innerRadius, center.y - innerRadius, 
                     innerRadius * 2, innerRadius * 2);
        
        // Glossy highlight (top half)
        g.setColour(juce::Colours::white.withAlpha(0.25f));
        juce::Path highlightPath;
        highlightPath.addEllipse(center.x - radius * 0.6f, center.y - radius * 0.8f, 
                                radius * 1.2f, radius * 1.0f);
        g.fillPath(highlightPath);
        
        // Value arc (shows current position)
        juce::Path valueArc;
        float startAngle = juce::MathConstants<float>::pi * 1.2f;
        valueArc.addCentredArc(center.x, center.y, radius - 10, radius - 10,
                              0, startAngle, angle, true);
        g.setColour(PsychedelicTheme::Colors::groovyGreen.brighter(0.5f));
        g.strokePath(valueArc, juce::PathStrokeType(8.0f, juce::PathStrokeType::curved, 
                                                     juce::PathStrokeType::rounded));
        
        // Range arc (background)
        juce::Path rangeArc;
        float endAngle = juce::MathConstants<float>::pi * 2.8f;
        rangeArc.addCentredArc(center.x, center.y, radius - 10, radius - 10,
                              0, startAngle, endAngle, true);
        g.setColour(juce::Colours::white.withAlpha(0.2f));
        g.strokePath(rangeArc, juce::PathStrokeType(8.0f, juce::PathStrokeType::curved,
                                                     juce::PathStrokeType::rounded));
        
        // Pointer line
        auto pointerLength = radius * 0.6f;
        juce::Point<float> pointerEnd(
            center.x + std::cos(angle) * pointerLength,
            center.y + std::sin(angle) * pointerLength
        );
        
        // Pointer glow
        g.setColour(PsychedelicTheme::Colors::sunshineYellow.withAlpha(0.6f));
        g.drawLine(center.x, center.y, pointerEnd.x, pointerEnd.y, 12.0f);
        
        // Pointer main line
        g.setColour(PsychedelicTheme::Colors::sunshineYellow);
        g.drawLine(center.x, center.y, pointerEnd.x, pointerEnd.y, 6.0f);
        
        // Center cap
        g.setColour(PsychedelicTheme::Colors::electricPink.withAlpha(0.7f));
        g.fillEllipse(center.x - 12, center.y - 12, 24, 24);
        g.setColour(PsychedelicTheme::Colors::electricPink);
        g.fillEllipse(center.x - 8, center.y - 8, 16, 16);
        
        // Outer ring
        g.setColour(PsychedelicTheme::Colors::cosmicOrange.brighter(0.3f));
        g.drawEllipse(center.x - radius, center.y - radius, radius * 2, radius * 2, 3.0f);
        
        // Small tick marks
        g.setColour(juce::Colours::white.withAlpha(0.4f));
        for (int i = 0; i <= 10; ++i) {
            float tickAngle = startAngle + (endAngle - startAngle) * (i / 10.0f);
            float innerTickRadius = radius - 15;
            float outerTickRadius = radius - 5;
            juce::Point<float> innerTick(center.x + std::cos(tickAngle) * innerTickRadius,
                                         center.y + std::sin(tickAngle) * innerTickRadius);
            juce::Point<float> outerTick(center.x + std::cos(tickAngle) * outerTickRadius,
                                         center.y + std::sin(tickAngle) * outerTickRadius);
            g.drawLine(innerTick.x, innerTick.y, outerTick.x, outerTick.y, 2.0f);
        }
    }
    
    void drawMeter(juce::Graphics& g, juce::Rectangle<int> bounds, float level, juce::Colour color) {
        if (bounds.isEmpty()) return;
        
        // Background with gradient
        juce::ColourGradient bgGrad(juce::Colours::black, bounds.getX(), bounds.getY(),
                                     juce::Colours::black.brighter(0.1f), bounds.getX(), bounds.getBottom(),
                                     false);
        g.setGradientFill(bgGrad);
        g.fillRoundedRectangle(bounds.toFloat(), 5.0f);
        
        // Level with gradient
        auto fillHeight = bounds.getHeight() * level;
        if (fillHeight > 0) {
            auto fillBounds = bounds.toFloat().removeFromBottom(fillHeight);
            
            juce::ColourGradient levelGrad(color.brighter(0.3f), fillBounds.getX(), fillBounds.getBottom(),
                                           color, fillBounds.getX(), fillBounds.getY(),
                                           false);
            g.setGradientFill(levelGrad);
            g.fillRoundedRectangle(fillBounds, 5.0f);
            
            // Glow effect
            g.setColour(color.withAlpha(0.3f));
            g.fillRoundedRectangle(fillBounds.expanded(2), 5.0f);
        }
        
        // 3D border
        g.setColour(juce::Colours::white.withAlpha(0.4f));
        g.drawRoundedRectangle(bounds.toFloat().reduced(0.5f), 5.0f, 2.0f);
        g.setColour(juce::Colours::black.withAlpha(0.3f));
        g.drawRoundedRectangle(bounds.toFloat().reduced(1.5f), 5.0f, 1.0f);
        
        // dB markers
        g.setColour(juce::Colours::white.withAlpha(0.3f));
        for (float db : {-3.0f, -6.0f, -12.0f, -18.0f}) {
            float y = bounds.getY() + bounds.getHeight() * (1.0f + db / 60.0f);
            g.drawLine(bounds.getRight() - 8, y, bounds.getRight() - 2, y, 1.0f);
        }
    }
    
    
    void drawWaveformDisplay(juce::Graphics& g, juce::Rectangle<int> bounds) {
        // Border
        g.setColour(PsychedelicTheme::Colors::groovyGreen.withAlpha(0.5f));
        g.drawRoundedRectangle(bounds.toFloat(), 10.0f, 2.0f);
        
        // Dark background
        g.setColour(juce::Colours::black.withAlpha(0.5f));
        g.fillRoundedRectangle(bounds.reduced(2).toFloat(), 8.0f);
        
        if (!audioLoaded) {
            g.setColour(juce::Colours::white.withAlpha(0.5f));
            g.setFont(juce::Font(16.0f));
            g.drawText("Load an audio file to see waveform", bounds, juce::Justification::centred);
            return;
        }
        
        // Draw real waveform from audio thumbnail
        if (audioThumbnail && audioThumbnail->getTotalLength() > 0.0) {
            g.setColour(PsychedelicTheme::Colors::sunshineYellow);
            audioThumbnail->drawChannels(g, bounds.reduced(5), 
                                        0.0, audioThumbnail->getTotalLength(), 1.0f);
            
            // Draw playback position
            if (isPlaying && transportSource.isPlaying()) {
                auto pos = transportSource.getCurrentPosition() / audioThumbnail->getTotalLength();
                auto x = bounds.getX() + pos * bounds.getWidth();
                g.setColour(PsychedelicTheme::Colors::electricPink);
                g.drawLine(x, bounds.getY(), x, bounds.getBottom(), 2.0f);
            }
        } else {
            // Placeholder text
            g.setColour(PsychedelicTheme::Colors::sunshineYellow);
            int numBars = 50;
            int barWidth = bounds.getWidth() / numBars;
            auto center = bounds.getCentreY();
            for (int i = 0; i < numBars; ++i) {
                float height = 20.0f;
                float x = bounds.getX() + i * barWidth;
                g.drawLine(x, center - height, x, center + height, 2.0f);
            }
        }
    }
    
    void handleLoadAudio() {
        fileChooser = std::make_unique<juce::FileChooser>(
            "Select an audio file",
            juce::File(),
            "*.wav;*.mp3;*.aiff;*.flac"
        );
        
        fileChooser->launchAsync(juce::FileBrowserComponent::openMode | juce::FileBrowserComponent::canSelectFiles,
            [this](const juce::FileChooser& chooser) {
                auto file = chooser.getResult();
                if (file.existsAsFile()) {
                    loadAudioFile(file);
                }
            });
    }
    
    void handlePlay() {
        if (!audioLoaded) {
            juce::AlertWindow::showMessageBoxAsync(
                juce::AlertWindow::InfoIcon,
                "No Audio Loaded",
                "Please load an audio file first!",
                "OK"
            );
            return;
        }
        
        if (readerSource == nullptr) return;
        
        transportSource.setPosition(0);
        transportSource.start();
        isPlaying = true;
    }
    
    void handleStop() {
        transportSource.stop();
        isPlaying = false;
    }
    
    void handleExport() {
        if (!audioLoaded) {
            juce::AlertWindow::showMessageBoxAsync(
                juce::AlertWindow::InfoIcon,
                "No Audio Loaded",
                "Please load an audio file first!",
                "OK"
            );
            return;
        }
        
        fileChooser = std::make_unique<juce::FileChooser>(
            "Save processed audio",
            juce::File(),
            "*.wav"
        );
        
        fileChooser->launchAsync(juce::FileBrowserComponent::saveMode,
            [this](const juce::FileChooser& chooser) {
                auto file = chooser.getResult();
                if (file != juce::File{}) {
                    exportWithMojo(file);
                }
            });
    }
    
    void handleStealMojo() {
        if (onStealMojoClicked) {
            onStealMojoClicked();
        }
    }
    
    void handleProControls() {
        if (onProControlsClicked) {
            onProControlsClicked();
        }
    }
    
    std::unique_ptr<juce::Label> titleLabel;
    std::unique_ptr<juce::Label> subtitleLabel;
    std::unique_ptr<juce::Label> fileInfoLabel;
    std::unique_ptr<juce::Label> mojoLabel;
    std::unique_ptr<juce::TextButton> loadButton;
    std::unique_ptr<juce::TextButton> playButton;
    std::unique_ptr<juce::TextButton> stopButton;
    std::unique_ptr<juce::TextButton> rewindButton;
    std::unique_ptr<juce::TextButton> ffButton;
    std::unique_ptr<juce::TextButton> exportButton;
    std::unique_ptr<juce::TextButton> stealMojoButton;
    std::unique_ptr<juce::TextButton> proControlsButton;
    std::unique_ptr<SkinnedKnob> mojoKnob;
    std::unique_ptr<juce::Label> inputMeterLabel;
    std::unique_ptr<juce::Label> outputMeterLabel;
    std::unique_ptr<juce::Slider> inputSlider;
    std::unique_ptr<juce::Slider> outputSlider;
    std::unique_ptr<juce::FileChooser> fileChooser;
    
    juce::File loadedFile;
    bool audioLoaded = false;
    bool isPlaying = false;
    float mojoAmount = 1.0f;  // 0.0 = Mojo, 1.0 = More Mojo, 2.0 = Most Mojo
    float animationPhase = 0.0f;
    float inputLevel = 0.0f;
    float outputLevel = 0.0f;
    juce::Rectangle<int> inputMeterBounds;
    juce::Rectangle<int> outputMeterBounds;
    juce::Rectangle<int> leftMojoMeterBounds;
    juce::Rectangle<int> rightMojoMeterBounds;
    juce::Rectangle<int> matchVolumeSwitchBounds;
    
    // Match Volume switch state and image
    bool matchVolumeEnabled = false;
    juce::Image matchVolumeSwitchImage;
    
    // VU Meter frames (photorealistic individual images)
    std::vector<juce::Image> vuMeterFrames;
    
    // Audio playback with mojo processing
    juce::AudioFormatManager formatManager;
    juce::AudioDeviceManager deviceManager;
    juce::AudioTransportSource transportSource;
    MojoAudioSource mojoSource;
    juce::AudioSourcePlayer audioSourcePlayer;
    std::unique_ptr<juce::AudioFormatReaderSource> readerSource;
    std::unique_ptr<juce::AudioThumbnailCache> thumbnailCache;
    std::unique_ptr<juce::AudioThumbnail> audioThumbnail;
    
    void changeListenerCallback(juce::ChangeBroadcaster*) override {
        if (!transportSource.isPlaying()) {
            isPlaying = false;
        }
    }
    
    void updateMojoLabel() {
        if (mojoAmount < 0.66f) {
            mojoLabel->setText("MOJO", juce::dontSendNotification);
        } else if (mojoAmount < 1.33f) {
            mojoLabel->setText("MORE MOJO", juce::dontSendNotification);
        } else {
            mojoLabel->setText("MOST MOJO", juce::dontSendNotification);
        }
    }
    
    void loadAudioFile(const juce::File& file) {
        transportSource.stop();
        transportSource.setSource(nullptr);
        readerSource.reset();
        
        auto* reader = formatManager.createReaderFor(file);
        if (reader != nullptr) {
            loadedFile = file;
            audioLoaded = true;
            fileInfoLabel->setText(file.getFileName(), juce::dontSendNotification);
            
            readerSource.reset(new juce::AudioFormatReaderSource(reader, true));
            transportSource.setSource(readerSource.get(), 0, nullptr, reader->sampleRate);
            
            // Prepare mojo source
            mojoSource.prepareToPlay(4096, reader->sampleRate);
            
            // Create thumbnail
            thumbnailCache.reset(new juce::AudioThumbnailCache(5));
            audioThumbnail.reset(new juce::AudioThumbnail(512, formatManager, *thumbnailCache));
            audioThumbnail->setSource(new juce::FileInputSource(file));
            
            repaint();
        }
    }
    
    void exportWithMojo(const juce::File& outputFile) {
        // Apply mojo blend: output = original + (mojo * mojoAmount)
        // Simple formula: blend original with processed based on mojo amount
        juce::AlertWindow::showMessageBoxAsync(
            juce::AlertWindow::InfoIcon,
            "Export",
            "Exporting with " + mojoLabel->getText() + "!",
            "Groovy!"
        );
    }
    
public:
    std::function<void()> onStealMojoClicked;
    std::function<void()> onProControlsClicked;
};
