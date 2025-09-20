# MLAR v3.1.1 Integration Guide for Windsurf

This document provides step-by-step instructions for integrating the "More Like The Record" (MLAR) module into the existing MoreMojo app and plugin codebase.

## 1. Folder Structure Setup

First, create the necessary directory structure within your existing repository:

```bash
mkdir -p shared/mlar/dsp
mkdir -p shared/mlar/metrics
mkdir -p shared/mlar/ui/views
mkdir -p shared/mlar/ui/components
mkdir -p shared/mlar/Resources/icons
```

## 2. Copy Source Files

Copy the MLAR module source files into the newly created directory structure:

### Core Files
- `shared/mlar/MLAR.h` - Umbrella header for the entire module

### DSP Modules
- `shared/mlar/dsp/TimingSafeResampler.h`
- `shared/mlar/dsp/TransientDetect.h`
- `shared/mlar/dsp/FractionalDelayAlign.h`
- `shared/mlar/dsp/AnalogContinuity.h`
- `shared/mlar/dsp/LimiterLevelMatch.h`
- `shared/mlar/dsp/LinkwitzRiley.h`

### Metrics Analyzers
- `shared/mlar/metrics/GDSComputer.h`
- `shared/mlar/metrics/IACCAnalyzer.h`
- `shared/mlar/metrics/TEIAnalyzer.h`
- `shared/mlar/metrics/ResidualNull.h`

### UI Components
- `shared/mlar/ui/LookAndFeel_Mojo.h`
- `shared/mlar/ui/LookAndFeel_Mojo.cpp`
- `shared/mlar/ui/components/MojoSlider.h`
- `shared/mlar/ui/components/MojoMeters.h`
- `shared/mlar/ui/components/ProcessStrip.h`
- `shared/mlar/ui/views/ConsumerView.h`
- `shared/mlar/ui/views/ConsumerView.cpp`
- `shared/mlar/ui/views/ProOverviewView.h`
- `shared/mlar/ui/views/ProOverviewView.cpp`
- `shared/mlar/ui/views/DetailWindows.h`

### Resources
- Copy SVG icons to `shared/mlar/Resources/icons/` directory

## 3. CMake Integration

Add the MLAR module to your build system by creating a new `CMakeLists.txt` file in the `shared/mlar` directory:

```cmake
# shared/mlar/CMakeLists.txt
cmake_minimum_required(VERSION 3.15)
project(MoreLikeTheRecord VERSION 3.1.1)

# Define MLAR library target
add_library(mlar STATIC)

# Define source files
target_sources(mlar
    PRIVATE
        # DSP Sources
        dsp/TimingSafeResampler.h
        dsp/TransientDetect.h
        dsp/FractionalDelayAlign.h
        dsp/AnalogContinuity.h
        dsp/LimiterLevelMatch.h
        dsp/LinkwitzRiley.h
        
        # Metrics Sources
        metrics/GDSComputer.h
        metrics/IACCAnalyzer.h
        metrics/TEIAnalyzer.h
        metrics/ResidualNull.h
        
        # UI Sources
        ui/LookAndFeel_Mojo.cpp
        ui/LookAndFeel_Mojo.h
        ui/components/MojoSlider.h
        ui/components/MojoMeters.h
        ui/components/ProcessStrip.h
        ui/views/ConsumerView.cpp
        ui/views/ConsumerView.h
        ui/views/ProOverviewView.cpp
        ui/views/ProOverviewView.h
        ui/views/DetailWindows.h
        
        # Main include
        MLAR.h
)

# Set include directories
target_include_directories(mlar
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
)

# Link with JUCE modules
target_link_libraries(mlar
    PRIVATE
        juce::juce_audio_processors
        juce::juce_audio_utils
        juce::juce_dsp
)

# Add SVG resources
file(GLOB_RECURSE MLAR_SVG_RESOURCES "${CMAKE_CURRENT_SOURCE_DIR}/Resources/icons/*.svg")

# Create binary data target if SVGs exist and not already created
if(MLAR_SVG_RESOURCES AND NOT TARGET MojoBinaryData)
    juce_add_binary_data(MojoBinaryData SOURCES ${MLAR_SVG_RESOURCES})
    target_link_libraries(mlar PRIVATE MojoBinaryData)
endif()
```

Then update your root `CMakeLists.txt` to include the MLAR module:

```cmake
# Add MLAR module
add_subdirectory(shared/mlar)

# Link MLAR with your plugin and app targets
target_link_libraries(MoreMojoPlugin
    PRIVATE
        mlar
)

target_link_libraries(MoreMojoApp
    PRIVATE
        mlar
)
```

## 4. Plugin Processor Integration

Update your plugin processor header (`plugin/Source/PluginProcessor.h`) to include and use the MLAR module:

```cpp
// Add MLAR include
#include "shared/mlar/MLAR.h"

// Add MLAR members to your processor class
class MoreMojoAudioProcessor : public juce::AudioProcessor {
    // ... existing code ...

    // MLAR processor
    moremojo::mlar::Processor mlar;
    
    // Metrics ring buffer for thread-safe communication with UI
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

    // Metrics ring buffer
    MetricsRing<moremojo::mlar::MetricsFrame, 64> mlarMetrics;
    
    // Pre/post processing buffers for analysis
    juce::AudioBuffer<float> mlarPre;
    juce::AudioBuffer<float> mlarPost;
};
```

Update your plugin processor implementation (`plugin/Source/PluginProcessor.cpp`) to integrate MLAR processing:

```cpp
// In your createParameterLayout() or equivalent function, add MLAR parameters
auto mlarParams = moremojo::mlar::Processor::createParameterLayout();
params.insert(params.end(), 
             std::make_move_iterator(mlarParams.getParameters().begin()), 
             std::make_move_iterator(mlarParams.getParameters().end()));

// In prepareToPlay()
void MoreMojoAudioProcessor::prepareToPlay(double sampleRate, int maximumExpectedSamplesPerBlock) {
    // Existing code...
    
    // Initialize MLAR
    mlar.prepare(sampleRate, maximumExpectedSamplesPerBlock);
    mlarPre.setSize(getTotalNumInputChannels(), maximumExpectedSamplesPerBlock);
    mlarPost.setSize(getTotalNumInputChannels(), maximumExpectedSamplesPerBlock);
}

// In processBlock()
void MoreMojoAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages) {
    // Existing code...
    
    // Store pre-processing buffer
    mlarPre.makeCopyOf(buffer);

    // Process MLAR if enabled
    if (apvts.getRawParameterValue("mlar_macro")->load() > 0.1f) {
        mlar.process(buffer, apvts);
    }

    // Store post-processing buffer
    mlarPost.makeCopyOf(buffer);

    // Update latency if needed
    setLatencySamples(mlar.latencySamples());

    // Update metrics (every 8 blocks to avoid CPU overhead)
    static int blockCounter = 0;
    if (++blockCounter >= 8) {
        blockCounter = 0;
        moremojo::mlar::MetricsFrame frame;
        mlar.snapshotMetrics(frame);
        mlarMetrics.push(frame);
    }
    
    // Implement macro parameter mapping
    updateMacroMapping();
}

// Add macro parameter mapping
void MoreMojoAudioProcessor::updateMacroMapping() {
    float macro = apvts.getRawParameterValue("mlar_macro")->load() * 0.01f; // Convert to 0-1
    
    auto mapWithSoftKnee = [](float value, float start, float end) {
        float normalized = juce::jlimit(0.0f, 1.0f, (value - start) / (end - start));
        // Soft knee curve (smoothstep)
        return normalized * normalized * (3.0f - 2.0f * normalized);
    };
    
    // Map macro to individual parameters
    if (auto* param = apvts.getParameter("mlar_transient_sense")) {
        float mapped = mapWithSoftKnee(macro, 0.1f, 0.7f);
        param->setValueNotifyingHost(mapped);
    }
    
    if (auto* param = apvts.getParameter("mlar_align_amount")) {
        float mapped = mapWithSoftKnee(macro, 0.3f, 0.9f);
        param->setValueNotifyingHost(mapped);
    }
    
    if (auto* param = apvts.getParameter("mlar_analog_mix")) {
        float mapped = 0.3f * mapWithSoftKnee(macro, 0.6f, 1.0f); // Cap at 30%
        param->setValueNotifyingHost(mapped);
    }
}
```

## 5. Plugin Editor Integration

Update your plugin editor header (`plugin/Source/PluginEditor.h`) to include and use the MLAR UI components:

```cpp
// Add MLAR UI includes
#include "shared/mlar/ui/views/ConsumerView.h"
#include "shared/mlar/ui/views/ProOverviewView.h"
#include "shared/mlar/ui/views/DetailWindows.h"

// Add MLAR UI member variables
class MoreMojoAudioProcessorEditor : public juce::AudioProcessorEditor {
    // ... existing code ...

    // MLAR tab
    juce::ToggleButton mlarTab{"More Like The Record"};

    // MLAR Consumer and Pro views
    moremojo::mlar::mojoUI::ConsumerView mlarConsumer;
    moremojo::mlar::mojoUI::ProOverviewView mlarPro;

    // MLAR detail window
    std::unique_ptr<juce::CallOutBox> mlarDetailWindow;

    // MLAR parameter attachments
    std::unique_ptr<juce::SliderParameterAttachment> mlarMacroAttachment;

    // Method to open MLAR detail windows
    void openMlarDetail(int index);
};
```

Update your plugin editor implementation (`plugin/Source/PluginEditor.cpp`) to integrate the MLAR UI:

```cpp
// In constructor
MoreMojoAudioProcessorEditor::MoreMojoAudioProcessorEditor(MoreMojoAudioProcessor& p)
    : AudioProcessorEditor(&p), audioProcessor(p)
{
    // ... existing code ...

    // Setup MLAR tab
    addAndMakeVisible(mlarTab);
    mlarTab.setClickingTogglesState(true);
    mlarTab.setToggleState(false, juce::dontSendNotification);

    // Initialize MLAR Consumer view
    addAndMakeVisible(mlarConsumer);
    mlarConsumer.setVisible(false); // Hide initially

    // Initialize MLAR Pro view
    addAndMakeVisible(mlarPro);
    mlarPro.setVisible(false); // Hide initially

    // Create parameter attachment for macro slider
    mlarMacroAttachment.reset(new juce::SliderParameterAttachment(
        *audioProcessor.apvts.getParameter("mlar_macro"), mlarConsumer.getMacroSlider()));

    // Setup tab click handler
    mlarTab.onClick = [this]() {
        bool showMlar = mlarTab.getToggleState();
        
        // Hide existing tabs/views when MLAR is active
        // ... (adapt to your specific UI structure) ...
        
        // Show appropriate MLAR view based on pro mode state
        bool proMode = proModeButton.getToggleState(); // Adapt to your UI
        mlarConsumer.setVisible(showMlar && !proMode);
        mlarPro.setVisible(showMlar && proMode);
    };

    // Pro mode toggle should affect MLAR views too
    proModeButton.onClick = [this]() { // Adapt to your UI
        bool proMode = proModeButton.getToggleState();
        
        // Update existing UI (keep your existing code)
        
        // Update MLAR views if tab is active
        if (mlarTab.getToggleState()) {
            mlarConsumer.setVisible(!proMode);
            mlarPro.setVisible(proMode);
        }
    };

    // Connect MLAR Pro view to detail windows
    mlarPro.onOpenDetail = [this](int index) {
        openMlarDetail(index);
    };
}

// Add method to open MLAR detail windows
void MoreMojoAudioProcessorEditor::openMlarDetail(int index) {
    // Close any existing detail window
    if (mlarDetailWindow) {
        mlarDetailWindow.reset();
    }
    
    // Create the appropriate detail window based on index
    std::unique_ptr<juce::Component> detail;
    
    switch (index) {
        case 0: // Resampler detail
            detail = std::make_unique<moremojo::mlar::mojoUI::ResamplerDetail>(audioProcessor);
            break;
            
        case 1: // Transient detail
            detail = std::make_unique<moremojo::mlar::mojoUI::TransientDetail>(audioProcessor);
            break;
            
        case 2: // Align detail
            detail = std::make_unique<moremojo::mlar::mojoUI::AlignDetail>(audioProcessor);
            break;
            
        case 3: // Analog detail
            detail = std::make_unique<moremojo::mlar::mojoUI::AnalogDetail>(audioProcessor);
            break;
            
        case 4: // Limiter detail
            detail = std::make_unique<moremojo::mlar::mojoUI::LimiterDetail>(audioProcessor);
            break;
            
        case 5: // Jitter detail
            detail = std::make_unique<moremojo::mlar::mojoUI::JitterDetail>(audioProcessor);
            break;
            
        case 6: // Consumer Macro detail
            detail = std::make_unique<moremojo::mlar::mojoUI::ConsumerMacroDetail>(audioProcessor);
            break;
            
        case 7: // Metrics Hub detail
            detail = std::make_unique<moremojo::mlar::mojoUI::MetricsHubDetail>(audioProcessor);
            break;
            
        default:
            return;
    }
    
    // Create a call-out box with the detail window
    auto bounds = mlarPro.getBounds();
    auto position = bounds.getCentre();
    
    mlarDetailWindow = juce::CallOutBox::launchAsynchronously(
        std::move(detail),
        juce::Rectangle<int>(position.x - 25, position.y - 25, 50, 50),
        this);
}

// Update resized() to position MLAR UI components
void MoreMojoAudioProcessorEditor::resized() {
    // ... existing code ...
    
    // Position MLAR tab button
    int tabWidth = 150;
    int tabHeight = 24;
    int tabX = getWidth() - tabWidth - 20;
    int tabY = 20;
    mlarTab.setBounds(tabX, tabY, tabWidth, tabHeight);
    
    // Position MLAR Consumer and Pro views
    juce::Rectangle<int> contentArea(20, 60, getWidth() - 40, getHeight() - 80);
    mlarConsumer.setBounds(contentArea);
    mlarPro.setBounds(contentArea);
}

// Update timerCallback() to update MLAR metrics
void MoreMojoAudioProcessorEditor::timerCallback() {
    // ... existing code ...
    
    // Update MLAR metrics if visible
    if (mlarTab.getToggleState()) {
        moremojo::mlar::MetricsFrame frame;
        
        // Try to get the latest metrics
        if (audioProcessor.mlarMetrics.pop(frame)) {
            // Update Consumer view meters
            if (mlarConsumer.isVisible()) {
                mlarConsumer.updateMeters(frame);
            }
            
            // Update Pro view meters
            if (mlarPro.isVisible()) {
                mlarPro.updateMeters(frame);
            }
        }
    }
}
```

## 6. App Integration (Optional)

If you want to integrate MLAR into your standalone app as well, you'll need to:

1. Link the MLAR library to your app target in CMake
2. Create appropriate UI views in your app that utilize the MLAR components
3. Wire up the audio processing path similar to the plugin integration

## 7. SVG Icons

Make sure to include the following SVG icons in the `shared/mlar/Resources/icons` directory:

- `record.svg` - Main icon for the "More Like The Record" feature
- `filter.svg` - Icon for resampler/filter
- `transient.svg` - Icon for transient detector
- `clock.svg` - Icon for alignment/timing
- `vinyl.svg` - Icon for analog continuity
- `limiter.svg` - Icon for limiter
- `jitter.svg` - Icon for jitter control
- `metrics.svg` - Icon for metrics hub

## 8. QA Checklist

Before deploying the integrated MLAR module, verify the following:

- [  ] ΔLUFS level-matching works within ±0.1 LU precision (visible in Limiter detail)
- [  ] ITD variance decreases with Align enabled (visible in Metrics Hub)
- [  ] GDS heatmap shows tighter spread post-MLAR on transient-rich content
- [  ] TEI overlay shows rise time trending toward 0, no increase in pre-ring dB
- [  ] Residual monitor works with both On/Off toggle and gain range 0-12 dB
- [  ] All UI components render correctly at different window sizes
- [  ] Macro parameter mapping works smoothly without parameter jumps
- [  ] Export functionality correctly saves CSV data and PNG screenshots
