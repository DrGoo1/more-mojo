#include <JuceHeader.h>
#include "../MLAR.h"
#include "../ui/views/ConsumerView.h"
#include "../ui/views/ProOverviewView.h"
#include "../ui/views/DetailWindows.h"

/**
 * Simple test application for the MLAR module
 * 
 * Features:
 * - Tests both Consumer and Pro UI views
 * - Simulates audio processing and metrics
 * - Allows interactive testing of all components
 */
class MLARTestComponent : public juce::Component,
                        private juce::Timer,
                        private juce::AudioIODeviceCallback {
public:
    MLARTestComponent() {
        // Set up title
        addAndMakeVisible(titleLabel);
        titleLabel.setText("MLAR Integration Test", juce::dontSendNotification);
        titleLabel.setFont(juce::Font(24.0f, juce::Font::bold));
        titleLabel.setJustificationType(juce::Justification::centred);
        
        // Set up view mode toggle
        addAndMakeVisible(viewModeToggle);
        viewModeToggle.setButtonText("Pro Mode");
        viewModeToggle.onClick = [this]() {
            consumerView.setVisible(!viewModeToggle.getToggleState());
            proView.setVisible(viewModeToggle.getToggleState());
        };
        
        // Create audio device manager
        deviceManager.initialise(2, 2, nullptr, true);
        deviceManager.addAudioCallback(this);
        
        // Set up format manager for audio file playback
        formatManager.registerBasicFormats();
        
        // Set up transport controls
        addAndMakeVisible(playButton);
        playButton.setButtonText("Play Test Audio");
        playButton.onClick = [this]() { startAudio(); };
        
        addAndMakeVisible(stopButton);
        stopButton.setButtonText("Stop");
        stopButton.onClick = [this]() { stopAudio(); };
        
        // Create audio processor
        processor = std::make_unique<TestAudioProcessor>();
        processor->prepareToPlay(44100.0, 512);
        
        // Set up parameter attachments
        setupParameterAttachments();
        
        // Set up views
        addAndMakeVisible(consumerView);
        addAndMakeVisible(proView);
        proView.setVisible(false);
        
        // Set up detail window callback
        proView.onOpenDetail([this](int index) { openDetailWindow(index); });
        
        // Start update timer
        startTimerHz(30);
        
        // Set initial size
        setSize(800, 600);
    }
    
    ~MLARTestComponent() override {
        stopAudio();
        deviceManager.removeAudioCallback(this);
        
        // Reset processor and attachments
        paramAttachments.clear();
        processor = nullptr;
    }
    
    void paint(juce::Graphics& g) override {
        g.fillAll(juce::Colour(15, 15, 20));
    }
    
    void resized() override {
        auto bounds = getLocalBounds().reduced(20);
        
        // Position title and mode toggle
        titleLabel.setBounds(bounds.removeFromTop(40));
        viewModeToggle.setBounds(bounds.removeFromTop(30).withSizeKeepingCentre(100, 30));
        
        // Position transport controls
        auto transportRow = bounds.removeFromBottom(40);
        playButton.setBounds(transportRow.removeFromLeft(transportRow.getWidth() / 2).reduced(5));
        stopButton.setBounds(transportRow.reduced(5));
        
        // Position views
        consumerView.setBounds(bounds);
        proView.setBounds(bounds);
    }
    
private:
    /**
     * Test audio processor for simulating the full MLAR processor
     */
    class TestAudioProcessor : public juce::AudioProcessor {
    public:
        TestAudioProcessor()
            : AudioProcessor(BusesProperties()
                .withInput("Input", juce::AudioChannelSet::stereo(), true)
                .withOutput("Output", juce::AudioChannelSet::stereo(), true)) {
            // Create parameter layout
            createParameterLayout();
        }
        
        ~TestAudioProcessor() override = default;
        
        void prepareToPlay(double sampleRate, int blockSize) override {
            // Initialize MLAR processor
            mlar.prepare(sampleRate, blockSize);
            
            // Initialize buffers
            mlarPre.setSize(2, blockSize);
            mlarPost.setSize(2, blockSize);
            
            // Reset state
            mlar.reset();
        }
        
        void releaseResources() override {}
        
        void processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer&) override {
            // Store pre-processing buffer
            mlarPre.makeCopyOf(buffer);
            
            // Process with MLAR if enabled
            if (apvts.getRawParameterValue("mlar_macro")->load() > 0.1f) {
                mlar.process(buffer, apvts);
            }
            
            // Store post-processing buffer
            mlarPost.makeCopyOf(buffer);
            
            // Update metrics
            moremojo::mlar::MetricsFrame frame;
            mlar.snapshotMetrics(frame);
            
            // Store in ring buffer
            metrics.push(frame);
        }
        
        juce::AudioProcessorEditor* createEditor() override { return nullptr; }
        bool hasEditor() const override { return false; }
        
        const juce::String getName() const override { return "MLAR Test Processor"; }
        bool acceptsMidi() const override { return false; }
        bool producesMidi() const override { return false; }
        double getTailLengthSeconds() const override { return 0.0; }
        
        int getNumPrograms() override { return 1; }
        int getCurrentProgram() override { return 0; }
        void setCurrentProgram(int) override {}
        const juce::String getProgramName(int) override { return {}; }
        void changeProgramName(int, const juce::String&) override {}
        
        void getStateInformation(juce::MemoryBlock&) override {}
        void setStateInformation(const void*, int) override {}
        
        // Create parameter layout
        void createParameterLayout() {
            // Get MLAR parameter layout and add to our APVTS
            auto paramLayout = moremojo::mlar::Processor::createParameterLayout();
            apvts = std::make_unique<juce::AudioProcessorValueTreeState>(
                *this, nullptr, "Parameters", std::move(paramLayout));
        }
        
        // MLAR components
        moremojo::mlar::Processor mlar;
        juce::AudioBuffer<float> mlarPre;
        juce::AudioBuffer<float> mlarPost;
        
        // Parameter state
        std::unique_ptr<juce::AudioProcessorValueTreeState> apvts;
        
        // Metrics ring buffer
        template<typename T, size_t N>
        class MetricsRing {
        public:
            bool push(const T& v) {
                size_t n = (w + 1) % N;
                if (n == r) return false;
                data[w] = v;
                w = n;
                return true;
            }
            
            bool pop(T& out) {
                if (r == w) return false;
                out = data[r];
                r = (r + 1) % N;
                return true;
            }
            
        private:
            T data[N];
            size_t r{0}, w{0};
        };
        
        MetricsRing<moremojo::mlar::MetricsFrame, 64> metrics;
    };
    
    void setupParameterAttachments() {
        if (!processor || !processor->apvts) return;
        
        // Clear existing attachments
        paramAttachments.clear();
        
        // Create attachments for Consumer view
        paramAttachments.push_back(std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
            *processor->apvts, "mlar_macro", consumerView.getMacroSlider()));
        
        // Create attachments for Pro view process strips
        for (int i = 0; i < proView.getNumProcessStrips(); ++i) {
            auto& strip = proView.getProcessStrip(i);
            
            // Attach parameters based on strip index
            switch (i) {
                case 0: // Resampler
                    paramAttachments.push_back(std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(
                        *processor->apvts, "mlar_resampler_on", strip.getToggleButton()));
                    paramAttachments.push_back(std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
                        *processor->apvts, "mlar_resampler_quality", strip.getSlider()));
                    break;
                    
                case 1: // Transient
                    paramAttachments.push_back(std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(
                        *processor->apvts, "mlar_transient_on", strip.getToggleButton()));
                    paramAttachments.push_back(std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
                        *processor->apvts, "mlar_transient_sense", strip.getSlider()));
                    break;
                    
                case 2: // Align
                    paramAttachments.push_back(std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(
                        *processor->apvts, "mlar_align_on", strip.getToggleButton()));
                    paramAttachments.push_back(std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
                        *processor->apvts, "mlar_align_amount", strip.getSlider()));
                    break;
                    
                case 3: // Analog
                    paramAttachments.push_back(std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(
                        *processor->apvts, "mlar_analog_on", strip.getToggleButton()));
                    paramAttachments.push_back(std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
                        *processor->apvts, "mlar_analog_mix", strip.getSlider()));
                    break;
                    
                case 4: // Limiter
                    paramAttachments.push_back(std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
                        *processor->apvts, "mlar_limit_ceiling", strip.getSlider()));
                    break;
                    
                case 5: // Macro control in Pro view
                    if (i == 5 && proView.getNumProcessStrips() > 5) {
                        paramAttachments.push_back(std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
                            *processor->apvts, "mlar_macro", strip.getSlider()));
                    }
                    break;
            }
        }
    }
    
    void openDetailWindow(int index) {
        // Close any existing detail window
        detailWindow.reset();
        
        // Create appropriate detail window based on index
        switch (index) {
            case 0: // Resampler
                detailWindow = std::make_unique<juce::DialogWindow>(
                    "Resampler Detail",
                    juce::Colour(15, 15, 20),
                    true, true);
                break;
                
            case 1: // Transient
                detailWindow = std::make_unique<juce::DialogWindow>(
                    "Transient Detail",
                    juce::Colour(15, 15, 20),
                    true, true);
                break;
                
            case 2: // Align
                detailWindow = std::make_unique<juce::DialogWindow>(
                    "Align Detail",
                    juce::Colour(15, 15, 20),
                    true, true);
                break;
                
            case 3: // Analog
                detailWindow = std::make_unique<juce::DialogWindow>(
                    "Analog Detail",
                    juce::Colour(15, 15, 20),
                    true, true);
                break;
                
            case 4: // Limiter
                detailWindow = std::make_unique<juce::DialogWindow>(
                    "Limiter Detail",
                    juce::Colour(15, 15, 20),
                    true, true);
                break;
                
            case 5: // Consumer Macro
                detailWindow = std::make_unique<juce::DialogWindow>(
                    "Macro Control",
                    juce::Colour(15, 15, 20),
                    true, true);
                break;
                
            case 6: // Metrics Hub
                detailWindow = std::make_unique<juce::DialogWindow>(
                    "Metrics Hub",
                    juce::Colour(15, 15, 20),
                    true, true);
                break;
        }
        
        // Show dialog if created
        if (detailWindow) {
            detailWindow->setSize(500, 400);
            detailWindow->setResizable(true, true);
            detailWindow->centreAroundComponent(this, detailWindow->getWidth(), detailWindow->getHeight());
            detailWindow->setVisible(true);
        }
    }
    
    // Start audio playback
    void startAudio() {
        stopAudio();  // Stop any existing playback
        
        // Open file chooser to select audio file
        juce::FileChooser chooser("Select Audio File", juce::File(), "*.wav;*.aiff;*.mp3");
        
        if (chooser.browseForFileToOpen()) {
            juce::File audioFile = chooser.getResult();
            
            // Create reader for the file
            auto* reader = formatManager.createReaderFor(audioFile);
            
            if (reader != nullptr) {
                // Create transport source
                transportSource = std::make_unique<juce::AudioTransportSource>();
                transportSource->setSource(new juce::AudioFormatReaderSource(reader, true));
                
                // Set up audio device
                deviceManager.setAudioSource(transportSource.get());
                
                // Start playback
                transportSource->setPosition(0);
                transportSource->start();
            }
        }
    }
    
    // Stop audio playback
    void stopAudio() {
        if (transportSource) {
            transportSource->stop();
            transportSource = nullptr;
        }
        
        deviceManager.setAudioSource(nullptr);
    }
    
    // AudioIODeviceCallback
    void audioDeviceIOCallback(const float** inputChannelData, int numInputChannels,
                             float** outputChannelData, int numOutputChannels,
                             int numSamples) override {
        // Clear output
        for (int ch = 0; ch < numOutputChannels; ++ch) {
            if (outputChannelData[ch]) {
                juce::FloatVectorOperations::clear(outputChannelData[ch], numSamples);
            }
        }
        
        // If transport source exists, get its output
        if (transportSource && numOutputChannels >= 2) {
            // Create buffer for transport
            juce::AudioBuffer<float> buffer(outputChannelData, numOutputChannels, numSamples);
            
            // Get output from transport
            juce::AudioSourceChannelInfo info(&buffer, 0, numSamples);
            transportSource->getNextAudioBlock(info);
            
            // Process with our test processor
            juce::MidiBuffer midi;
            processor->processBlock(buffer, midi);
        }
    }
    
    void audioDeviceAboutToStart(juce::AudioIODevice* device) override {
        if (transportSource) {
            transportSource->prepareToPlay(device->getCurrentBufferSizeSamples(),
                                         device->getCurrentSampleRate());
        }
        
        if (processor) {
            processor->prepareToPlay(device->getCurrentSampleRate(),
                                   device->getCurrentBufferSizeSamples());
        }
    }
    
    void audioDeviceStopped() override {
        if (transportSource) {
            transportSource->releaseResources();
        }
        
        if (processor) {
            processor->releaseResources();
        }
    }
    
    void timerCallback() override {
        if (processor) {
            // Get metrics from processor
            moremojo::mlar::MetricsFrame frame;
            if (processor->metrics.pop(frame)) {
                // Update UI with metrics
                if (!viewModeToggle.getToggleState()) {
                    consumerView.updateMeters(frame);
                } else {
                    proView.updateMeters(frame);
                }
            }
        }
    }
    
    // UI Components
    juce::Label titleLabel;
    juce::ToggleButton viewModeToggle;
    moremojo::mlar::mojoUI::ConsumerView consumerView;
    moremojo::mlar::mojoUI::ProOverviewView proView;
    
    // Transport Controls
    juce::TextButton playButton;
    juce::TextButton stopButton;
    
    // Audio Components
    juce::AudioDeviceManager deviceManager;
    juce::AudioFormatManager formatManager;
    std::unique_ptr<juce::AudioTransportSource> transportSource;
    
    // Test Processor
    std::unique_ptr<TestAudioProcessor> processor;
    
    // Parameter Attachments
    std::vector<std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment>> paramAttachments;
    std::vector<std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment>> buttonAttachments;
    
    // Detail Windows
    std::unique_ptr<juce::DialogWindow> detailWindow;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MLARTestComponent)
};

//==============================================================================
class MLARTestApplication : public juce::JUCEApplication {
public:
    MLARTestApplication() = default;
    
    const juce::String getApplicationName() override { return "MLAR Test"; }
    const juce::String getApplicationVersion() override { return "1.0.0"; }
    
    void initialise(const juce::String&) override {
        mainWindow = std::make_unique<MainWindow>("MLAR Test", new MLARTestComponent());
    }
    
    void shutdown() override {
        mainWindow = nullptr;
    }
    
    void systemRequestedQuit() override {
        quit();
    }
    
private:
    class MainWindow : public juce::DocumentWindow {
    public:
        MainWindow(const juce::String& name, juce::Component* content)
            : DocumentWindow(name, juce::Colours::darkgrey, DocumentWindow::allButtons) {
            setUsingNativeTitleBar(true);
            setContentOwned(content, true);
            setResizable(true, true);
            centreWithSize(800, 600);
            setVisible(true);
        }
        
        void closeButtonPressed() override {
            JUCEApplication::getInstance()->systemRequestedQuit();
        }
        
    private:
        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MainWindow)
    };
    
    std::unique_ptr<MainWindow> mainWindow;
};

//==============================================================================
START_JUCE_APPLICATION(MLARTestApplication)
