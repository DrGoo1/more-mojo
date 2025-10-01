# SRC, JITTER, and ALIGN Info Button Implementation

## Overview
This document details the implementation of comprehensive INFO buttons for the SRC (Sample Rate Conversion), JITTER (Jitter Analysis & Optimization), and ALIGN (Phase/Time Alignment) control windows in ProGUI. These info buttons now display detailed technical documentation with sophisticated graphics, matching the functionality previously implemented for the ISP control window.

## Changes Made

### 1. SRCControlWindow (Sample Rate Conversion)
**File**: `Source/UIDemoComponent.cpp`

#### Info Button Integration
- **Modified**: Info button `onClick` handler to call `showInfoWindow()` instead of basic alert
- **Added**: `showInfoWindow()` method that creates and displays `ISPInfoComponent("SRC")`

#### Features
- Displays comprehensive SRC technical documentation
- Shows frequency response graphics demonstrating filter characteristics
- Illustrates passband, transition band, and stopband regions
- Provides visual explanation of sample rate conversion quality settings

### 2. JitterControlWindow (Jitter Analysis & Optimization)
**File**: `Source/UIDemoComponent.cpp`

#### Info Button Integration
- **Modified**: Info button `onClick` handler to call `showInfoWindow()` instead of basic alert
- **Added**: `showInfoWindow()` method that creates and displays `ISPInfoComponent("JITTER")`

#### Features
- Displays detailed jitter analysis documentation
- Shows waveform graphics with peak detection
- Illustrates bit-depth and dithering concepts
- Provides visual representation of jitter effects on audio

### 3. AlignControlWindow (Phase/Time Alignment)
**File**: `Source/UIDemoComponent.cpp`

#### Info Button Integration
- **Modified**: Info button `onClick` handler to call `showInfoWindow()` instead of basic alert
- **Added**: `showInfoWindow()` method that creates and displays `ISPInfoComponent("ALIGN")`

#### Features
- Displays comprehensive phase alignment documentation
- Shows phase relationship graphics
- Illustrates stereo imaging and time alignment concepts
- Provides visual examples of phase correction techniques

## Technical Implementation Details

### Code Structure
Each control window now includes:

```cpp
void showInfoWindow()
{
    auto* infoComponent = new ISPInfoComponent("PROCESS_NAME");
    
    juce::DialogWindow::LaunchOptions opts;
    opts.content.setOwned(infoComponent);
    opts.dialogTitle = "PROCESS_NAME - Information";
    opts.componentToCentreAround = this;
    opts.escapeKeyTriggersCloseButton = true;
    opts.useNativeTitleBar = true;
    opts.resizable = true;
    opts.launchAsync();
}
```

### ISPInfoComponent Graphics Dispatching
The `ISPInfoComponent` class intelligently routes to appropriate graphics based on the window title:

- **"SRC"** → `drawSRCGraphics()` - Frequency response and filter visualization
- **"JITTER"** → `drawJitterGraphics()` - Waveform and bit-depth visualization  
- **"ALIGN"** → `drawAlignGraphics()` - Phase relationship and stereo imaging
- **"ISP"** → Multiple graphics showing oversampling, resolution, etc.

### Graphics Methods
Each graphics method provides:
- **Technical accuracy** - Correctly illustrates audio processing concepts
- **Visual clarity** - Clean, professional graphics with proper labeling
- **Educational value** - Helps users understand complex audio processing

## User Experience

### Before
- Info buttons showed simple text-only alert boxes
- No visual documentation
- Limited technical information

### After
- Info buttons open sophisticated modal dialogs
- Comprehensive technical documentation with graphics
- Professional, educational presentation
- Resizable windows for detailed viewing
- Consistent UI/UX across all control windows

## Status of Meters

All three control windows **already have meters implemented**:

### SRCControlWindow Meters
- **4 vertical meters** positioned above knobs
- Colors: Teal (Quality), Orange (Ripple), Blue (Stopband), Yellow (Transition)
- 20-segment LED-style display
- Labels: "Qual", "Rip", "Stop", "Tran"

### JitterControlWindow Meters
- **4 vertical meters** positioned above knobs
- Colors: Teal (RMS), Orange (Spectrum), Blue (Bits), Yellow (Dither)
- 20-segment LED-style display
- Labels: "RMS", "Spec", "Bits", "Dith"

### AlignControlWindow Meters
- **4 vertical meters** positioned above knobs
- Colors: Teal (Delay), Orange (Polarity), Blue (Phase), Yellow (Crossover)
- 20-segment LED-style display
- Labels: "Del", "Pol", "Phs", "Xov"

## Integration Points

### Related Classes
- **ISPInfoComponent**: Shared component for displaying process-specific information
- **ISPControlWindow**: Original implementation reference
- **CleanKnob**: Custom knob component used in all control windows
- **juce::DialogWindow**: Modal dialog framework

### Dependencies
- JUCE graphics framework
- JUCE GUI components
- C++ exception handling
- Memory management (smart pointers)

## Build Requirements

The changes are purely additive to existing functionality:
- No new dependencies required
- No breaking changes to existing interfaces
- Compatible with existing build system

## Testing Recommendations

1. **Info Button Functionality**
   - Click INFO button in each window (SRC, JITTER, ALIGN)
   - Verify modal dialog opens with correct title
   - Confirm documentation text is appropriate for each process
   - Check that graphics render correctly

2. **Graphics Display**
   - Verify SRC shows frequency response graphics
   - Verify JITTER shows waveform/bit-depth graphics
   - Verify ALIGN shows phase relationship graphics
   - Confirm all graphics are labeled and clear

3. **Dialog Behavior**
   - Test window resizing
   - Test ESC key to close
   - Verify dialog centers on parent window
   - Check native title bar functionality

4. **Meter Display**
   - Confirm all meters are visible and positioned correctly
   - Verify segment rendering and colors
   - Check meter labels are readable

## Future Enhancements

1. **Interactive Graphics**
   - Add mouse-over tooltips
   - Implement zoom functionality
   - Add animation for dynamic concepts

2. **Additional Content**
   - Link to external documentation
   - Add audio examples
   - Include preset explanations

3. **Customization**
   - User-adjustable text size
   - Theme support for graphics
   - Saved window positions/sizes

## Summary

The SRC, JITTER, and ALIGN control windows now have comprehensive INFO buttons with detailed technical documentation and sophisticated graphics, matching the quality and functionality of the ISP control window. All windows already include professional meter displays with 20-segment LED-style indicators. The implementation is clean, maintainable, and provides significant educational value to users.
