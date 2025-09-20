#pragma once
#include <JuceHeader.h>
#include "../LookAndFeel_Mojo.h"
#include "../components/MojoMeters.h"

namespace mojo {

/**
 * Base class for all detail windows
 * Provides common styling and title handling
 */
class DetailWindow : public juce::Component {
public:
    DetailWindow(juce::String header) : title(std::move(header)) {
        addAndMakeVisible(titleLabel);
        titleLabel.setText(title, juce::dontSendNotification);
        titleLabel.setFont(juce::Font(18.f, juce::Font::bold));
    }
    
    void paint(juce::Graphics& g) override {
        g.fillAll(juce::Colour(15, 15, 20));
        LookAndFeel_Mojo::drawCard(g, getLocalBounds().toFloat().reduced(8), 
                                  juce::Colour(24, 24, 32), 14.f, 12.f);
    }
    
    void resized() override {
        auto bounds = getLocalBounds().reduced(16);
        titleLabel.setBounds(bounds.removeFromTop(28));
    }
    
protected:
    juce::Label titleLabel;
    juce::String title;
};

/**
 * Detail window for the Consumer "More Like The Record" macro
 * Shows how the macro maps to individual processes
 */
class ConsumerMacroDetail : public DetailWindow {
public:
    ConsumerMacroDetail() : DetailWindow("More Like The Record — Macro Mapping") {
        // Add main macro slider
        addAndMakeVisible(macro);
        macro.setSliderStyle(juce::Slider::LinearHorizontal);
        macro.setRange(0.0, 100.0, 0.1);
        macro.setTextValueSuffix(" %");
        
        // Add mapping curves label
        addAndMakeVisible(curveLabel);
        curveLabel.setText("Mapping Curves", juce::dontSendNotification);
        
        // Add per-process amount sliders
        addAndMakeVisible(resamplerAmt);
        resamplerAmt.setRange(0.0, 100.0, 0.1);
        resamplerAmt.setSliderStyle(juce::Slider::LinearBar);
        
        addAndMakeVisible(transientAmt);
        transientAmt.setRange(0.0, 100.0, 0.1);
        transientAmt.setSliderStyle(juce::Slider::LinearBar);
        
        addAndMakeVisible(alignAmt);
        alignAmt.setRange(0.0, 100.0, 0.1);
        alignAmt.setSliderStyle(juce::Slider::LinearBar);
        
        addAndMakeVisible(analogAmt);
        analogAmt.setRange(0.0, 100.0, 0.1);
        analogAmt.setSliderStyle(juce::Slider::LinearBar);
        
        // Add preview button
        addAndMakeVisible(previewBtn);
        previewBtn.setButtonText("Preview Morph (250ms)");
        
        // Add SCPI meter
        addAndMakeVisible(scpi);
    }
    
    void resized() override {
        DetailWindow::resized();
        
        auto bounds = getLocalBounds().reduced(24);
        bounds.removeFromTop(32); // Space for title
        
        // Main macro slider
        macro.setBounds(bounds.removeFromTop(32));
        bounds.removeFromTop(12);
        
        // Mapping curves
        curveLabel.setBounds(bounds.removeFromTop(22));
        auto row = bounds.removeFromTop(52);
        resamplerAmt.setBounds(row.removeFromLeft(bounds.getWidth() / 4).reduced(6));
        transientAmt.setBounds(row.removeFromLeft(bounds.getWidth() / 4).reduced(6));
        alignAmt.setBounds(row.removeFromLeft(bounds.getWidth() / 4).reduced(6));
        analogAmt.setBounds(row.reduced(6));
        
        // Preview button
        bounds.removeFromTop(8);
        previewBtn.setBounds(bounds.removeFromTop(28).removeFromLeft(200));
        
        // SCPI meter
        bounds.removeFromTop(8);
        scpi.setBounds(bounds.removeFromTop(120));
    }
    
private:
    juce::Slider macro, resamplerAmt, transientAmt, alignAmt, analogAmt;
    juce::Label curveLabel;
    juce::TextButton previewBtn;
    LUFSMeter scpi; // Using LUFS meter as a placeholder for SCPI meter
};

/**
 * Detail window for the TimingSafeResampler module
 */
class ResamplerDetail : public DetailWindow {
public:
    ResamplerDetail() : DetailWindow("TimingSafeResampler") {
        // Add oversampling slider
        addAndMakeVisible(overs);
        overs.setSliderStyle(juce::Slider::LinearHorizontal);
        overs.setRange(1, 8, 1);
        overs.setTextValueSuffix("x");
        
        // Add filter type dropdown
        addAndMakeVisible(filterType);
        filterType.addItemList({"Linear", "Minimum", "Mixed"}, 1);
        
        // Add ISP guard toggle
        addAndMakeVisible(ispGuard);
        ispGuard.setButtonText("ISP Guard");
        
        // Add interpolation toggle
        addAndMakeVisible(interpToggle);
        interpToggle.setButtonText("Intersample Interpolation");
        
        // Add pre-ring meter
        addAndMakeVisible(preRingMeter);
    }
    
    void resized() override {
        DetailWindow::resized();
        
        auto bounds = getLocalBounds().reduced(24);
        bounds.removeFromTop(32); // Space for title
        
        // Oversampling slider
        overs.setBounds(bounds.removeFromTop(32));
        bounds.removeFromTop(8);
        
        // Filter type dropdown
        filterType.setBounds(bounds.removeFromTop(28));
        bounds.removeFromTop(8);
        
        // ISP guard toggle
        ispGuard.setBounds(bounds.removeFromTop(28));
        bounds.removeFromTop(8);
        
        // Interpolation toggle
        interpToggle.setBounds(bounds.removeFromTop(28));
        bounds.removeFromTop(8);
        
        // Pre-ring meter
        preRingMeter.setBounds(bounds.removeFromTop(140));
    }
    
private:
    juce::Slider overs;
    juce::ComboBox filterType;
    juce::ToggleButton ispGuard, interpToggle;
    LUFSMeter preRingMeter; // Using LUFS meter as a placeholder
};

/**
 * Detail window for the TransientDetect module
 */
class TransientDetail : public DetailWindow {
public:
    TransientDetail() : DetailWindow("TransientDetect") {
        // Add bands slider
        addAndMakeVisible(bands);
        bands.setRange(4, 12, 1);
        bands.setSliderStyle(juce::Slider::LinearBar);
        
        // Add sensitivity slider
        addAndMakeVisible(sensitivity);
        sensitivity.setRange(0.0, 100.0, 0.1);
        sensitivity.setSliderStyle(juce::Slider::LinearBar);
        
        // Add hysteresis slider
        addAndMakeVisible(hysteresis);
        hysteresis.setRange(0.0, 100.0, 0.1);
        hysteresis.setSliderStyle(juce::Slider::LinearBar);
        
        // Add smoothing slider
        addAndMakeVisible(smoothing);
        smoothing.setRange(0.0, 50.0, 0.1);
        smoothing.setSliderStyle(juce::Slider::LinearBar);
        
        // Add onset meter
        addAndMakeVisible(onsetMeter);
    }
    
    void resized() override {
        DetailWindow::resized();
        
        auto bounds = getLocalBounds().reduced(24);
        bounds.removeFromTop(32); // Space for title
        
        // Row of sliders
        auto row = bounds.removeFromTop(32);
        bands.setBounds(row.removeFromLeft(row.getWidth() / 4).reduced(4));
        sensitivity.setBounds(row.removeFromLeft(row.getWidth() / 3).reduced(4));
        hysteresis.setBounds(row.removeFromLeft(row.getWidth() / 2).reduced(4));
        smoothing.setBounds(row.reduced(4));
        
        // Onset meter
        bounds.removeFromTop(8);
        onsetMeter.setBounds(bounds.removeFromTop(160));
    }
    
private:
    juce::Slider bands, sensitivity, hysteresis, smoothing;
    CorrelationMeter onsetMeter; // Using correlation meter as a placeholder
};

/**
 * Detail window for the FractionalDelayAlign module
 */
class AlignDetail : public DetailWindow {
public:
    AlignDetail() : DetailWindow("FractionalDelayAlign") {
        // Add max correction slider
        addAndMakeVisible(maxCorrection);
        maxCorrection.setRange(0.0, 0.6, 0.01);
        maxCorrection.setTextValueSuffix(" ms");
        maxCorrection.setSliderStyle(juce::Slider::LinearBar);
        
        // Add step size slider
        addAndMakeVisible(stepSize);
        stepSize.setRange(0.01, 0.10, 0.01);
        stepSize.setTextValueSuffix(" ms");
        stepSize.setSliderStyle(juce::Slider::LinearBar);
        
        // Add ITD target slider
        addAndMakeVisible(itdTarget);
        itdTarget.setRange(-0.5, 0.5, 0.01);
        itdTarget.setTextValueSuffix(" ms");
        itdTarget.setSliderStyle(juce::Slider::LinearBar);
        
        // Add width protect slider
        addAndMakeVisible(widthProtect);
        widthProtect.setRange(0.0, 100.0, 1.0);
        widthProtect.setSliderStyle(juce::Slider::LinearBar);
        
        // Add comb avoid slider
        addAndMakeVisible(combAvoid);
        combAvoid.setRange(0.0, 100.0, 1.0);
        combAvoid.setSliderStyle(juce::Slider::LinearBar);
        
        // Add meters
        addAndMakeVisible(gdsHeatmap);
        addAndMakeVisible(itdVar);
    }
    
    void resized() override {
        DetailWindow::resized();
        
        auto bounds = getLocalBounds().reduced(24);
        bounds.removeFromTop(32); // Space for title
        
        // First row of sliders
        auto row = bounds.removeFromTop(32);
        maxCorrection.setBounds(row.removeFromLeft(row.getWidth() / 3).reduced(4));
        stepSize.setBounds(row.removeFromLeft(row.getWidth() / 2).reduced(4));
        itdTarget.setBounds(row.reduced(4));
        
        // Second row of sliders
        bounds.removeFromTop(8);
        auto row2 = bounds.removeFromTop(28);
        widthProtect.setBounds(row2.removeFromLeft(row2.getWidth() / 2).reduced(4));
        combAvoid.setBounds(row2.reduced(4));
        
        // Meters
        bounds.removeFromTop(8);
        gdsHeatmap.setBounds(bounds.removeFromTop(160));
        bounds.removeFromTop(8);
        itdVar.setBounds(bounds.removeFromTop(120));
    }
    
private:
    juce::Slider maxCorrection, stepSize, itdTarget, widthProtect, combAvoid;
    LUFSMeter gdsHeatmap; // Using LUFS meter as a placeholder for heatmap
    CorrelationMeter itdVar; // Using correlation meter as a placeholder
};

/**
 * Detail window for the AnalogContinuity module
 */
class AnalogDetail : public DetailWindow {
public:
    AnalogDetail() : DetailWindow("AnalogContinuity") {
        // Add elliptical filter frequency slider
        addAndMakeVisible(elliptFreq);
        elliptFreq.setRange(60.0, 200.0, 1.0);
        elliptFreq.setSliderStyle(juce::Slider::LinearBar);
        
        // Add elliptical slope dropdown
        addAndMakeVisible(elliptSlope);
        elliptSlope.addItemList({"6 dB/oct", "12 dB/oct"}, 1);
        
        // Add crosstalk tilt slider
        addAndMakeVisible(xtalkTilt);
        xtalkTilt.setRange(-45.0, -25.0, 0.5);
        xtalkTilt.setSliderStyle(juce::Slider::LinearBar);
        
        // Add asymmetry slider
        addAndMakeVisible(asymmetry);
        asymmetry.setRange(0.0, 10.0, 0.1);
        asymmetry.setSliderStyle(juce::Slider::LinearBar);
        
        // Add mix slider
        addAndMakeVisible(mix);
        mix.setRange(0.0, 100.0, 0.1);
        mix.setSliderStyle(juce::Slider::LinearBar);
        
        // Add meters
        addAndMakeVisible(widthMap);
        addAndMakeVisible(goniometer);
    }
    
    void resized() override {
        DetailWindow::resized();
        
        auto bounds = getLocalBounds().reduced(24);
        bounds.removeFromTop(32); // Space for title
        
        // First row of controls
        auto row = bounds.removeFromTop(30);
        elliptFreq.setBounds(row.removeFromLeft(row.getWidth() / 4).reduced(4));
        elliptSlope.setBounds(row.removeFromLeft(row.getWidth() / 3).reduced(4));
        xtalkTilt.setBounds(row.removeFromLeft(row.getWidth() / 2).reduced(4));
        mix.setBounds(row.reduced(4));
        
        // Asymmetry slider
        bounds.removeFromTop(8);
        asymmetry.setBounds(bounds.removeFromTop(28));
        
        // Meters
        bounds.removeFromTop(8);
        widthMap.setBounds(bounds.removeFromTop(150));
        bounds.removeFromTop(8);
        goniometer.setBounds(bounds.removeFromTop(150));
    }
    
private:
    juce::Slider elliptFreq, xtalkTilt, asymmetry, mix;
    juce::ComboBox elliptSlope;
    LUFSMeter widthMap; // Using LUFS meter as a placeholder
    CorrelationMeter goniometer; // Using correlation meter as a placeholder
};

/**
 * Detail window for the Limiter + Level-Match module
 */
class LimiterDetail : public DetailWindow {
public:
    LimiterDetail() : DetailWindow("Limiter + Level-Match") {
        // Add lookahead slider
        addAndMakeVisible(lookahead);
        lookahead.setRange(0.5, 2.0, 0.1);
        lookahead.setTextValueSuffix(" ms");
        lookahead.setSliderStyle(juce::Slider::LinearBar);
        
        // Add ceiling slider
        addAndMakeVisible(ceiling);
        ceiling.setRange(-6.0, 0.0, 0.1);
        ceiling.setTextValueSuffix(" dB");
        ceiling.setSliderStyle(juce::Slider::LinearBar);
        
        // Add release slider
        addAndMakeVisible(release);
        release.setRange(5.0, 200.0, 1.0);
        release.setTextValueSuffix(" ms");
        release.setSliderStyle(juce::Slider::LinearBar);
        
        // Add ISP guard toggle
        addAndMakeVisible(ispGuard);
        ispGuard.setButtonText("ISP Guard");
        
        // Add match target dropdown
        addAndMakeVisible(matchTarget);
        matchTarget.addItemList({"Match A", "Match B"}, 1);
        
        // Add gain reduction meter
        addAndMakeVisible(grMeter);
    }
    
    void resized() override {
        DetailWindow::resized();
        
        auto bounds = getLocalBounds().reduced(24);
        bounds.removeFromTop(32); // Space for title
        
        // Row of sliders
        auto row = bounds.removeFromTop(28);
        lookahead.setBounds(row.removeFromLeft(row.getWidth() / 3).reduced(4));
        ceiling.setBounds(row.removeFromLeft(row.getWidth() / 2).reduced(4));
        release.setBounds(row.reduced(4));
        
        // Toggles and dropdown
        bounds.removeFromTop(8);
        ispGuard.setBounds(bounds.removeFromTop(24));
        matchTarget.setBounds(bounds.removeFromTop(24));
        
        // Gain reduction meter
        bounds.removeFromTop(8);
        grMeter.setBounds(bounds.removeFromTop(150));
    }
    
private:
    juce::Slider lookahead, ceiling, release;
    juce::ToggleButton ispGuard;
    juce::ComboBox matchTarget;
    LUFSMeter grMeter; // Using LUFS meter as a placeholder
};

/**
 * Detail window for the Metrics Hub
 */
class MetricsHubDetail : public DetailWindow {
public:
    MetricsHubDetail() : DetailWindow("Metrics Hub") {
        // Add meters and visualizations
        addAndMakeVisible(gdsHeatmap);
        addAndMakeVisible(iaccTimeline);
        addAndMakeVisible(itdVar);
        addAndMakeVisible(teiTable);
        addAndMakeVisible(nullSpectrum);
        
        // Add export button
        addAndMakeVisible(exportBtn);
        exportBtn.setButtonText("Export CSV/PNG");
    }
    
    void resized() override {
        DetailWindow::resized();
        
        auto bounds = getLocalBounds().reduced(20);
        bounds.removeFromTop(32); // Space for title
        
        // GDS heatmap
        gdsHeatmap.setBounds(bounds.removeFromTop(140));
        bounds.removeFromTop(8);
        
        // IACC timeline
        iaccTimeline.setBounds(bounds.removeFromTop(100));
        bounds.removeFromTop(8);
        
        // ITD variance and TEI table
        auto row = bounds.removeFromTop(120);
        itdVar.setBounds(row.removeFromLeft(row.getWidth() / 2).reduced(4));
        teiTable.setBounds(row.reduced(4));
        
        // Null spectrum
        bounds.removeFromTop(8);
        nullSpectrum.setBounds(bounds.removeFromTop(120));
        
        // Export button
        bounds.removeFromTop(8);
        exportBtn.setBounds(bounds.removeFromTop(28).removeFromRight(160));
    }
    
private:
    LUFSMeter gdsHeatmap, iaccTimeline, nullSpectrum; // Using LUFS meters as placeholders
    CorrelationMeter itdVar, teiTable; // Using correlation meters as placeholders
    juce::TextButton exportBtn;
};

}  // namespace mojo
