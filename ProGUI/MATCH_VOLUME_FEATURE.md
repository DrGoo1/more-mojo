# Match Volume Feature - Consumer GUI Enhancement

## Overview
The Match Volume feature is a sophisticated gain compensation system integrated into the MoreMojo Consumer GUI. It allows users to compare the tonal effects of the "mojo" processing without being distracted by volume differences.

## Purpose
When audio processing adds harmonics, saturation, and other effects, it often increases the perceived loudness. This can make processed audio sound "better" simply because it's louder - a well-known psychoacoustic phenomenon. The Match Volume feature compensates for this by automatically adjusting the output gain to maintain consistent perceived loudness.

## Visual Design

### Toggle Switch
- **Component**: Photorealistic filmstrip switch (Button-08-B)
- **Size**: 60x60 pixels
- **Position**: Upper right area, above the "WITH MOJO" VU meter
- **States**: 
  - Frame 0 = OFF (switch up position)
  - Frame 1 = ON (switch down position)

### Labeling
- **Main Label**: "Match Volume" (above switch)
- **State Indicators**: "OFF" (left) and "ON" (right) below switch
- **Typography**: Bold psychedelic style with black outlines for readability
- **Dynamic Highlighting**: 
  - Active state shows bright yellow text
  - Inactive state shows dimmed orange text

## Technical Implementation

### Gain Compensation Algorithm
Located in `MojoAudioSource` class:

```cpp
void setGainCompensation(bool enabled) {
    gainCompensationEnabled = enabled;
    if (enabled) {
        // Calculate compensation based on current mojo amount
        // More mojo = more gain reduction needed
        float compensationFactor = 1.0f - (mojoAmount * 0.15f);
        outputGain = std::max(0.3f, compensationFactor);
    } else {
        outputGain = 1.0f;  // No compensation
    }
}
```

### Compensation Curve
- **No Mojo (0.0)**: 1.0x gain (no reduction)
- **Mojo (0.67)**: ~0.90x gain (10% reduction)
- **More Mojo (1.33)**: ~0.80x gain (20% reduction)  
- **Most Mojo (2.0)**: ~0.70x gain (30% reduction)
- **Minimum**: Never goes below 0.3x to prevent over-compensation

### Interactive Behavior
```cpp
void mouseDown(const juce::MouseEvent& event) override {
    if (matchVolumeSwitchBounds.contains(event.getPosition())) {
        matchVolumeEnabled = !matchVolumeEnabled;
        mojoSource.setGainCompensation(matchVolumeEnabled);
        repaint();
    }
}
```

## User Experience

### Without Match Volume (OFF)
- More mojo = louder output
- User may perceive loudness increase as "better sound"
- Difficult to objectively evaluate tonal changes

### With Match Volume (ON)
- Consistent perceived loudness across all mojo settings
- User can focus on harmonic content, warmth, and tonal character
- True A/B comparison of processing quality

## Design Consistency

### Black Outlines Enhancement
As part of this feature implementation, ALL labels in the Consumer GUI were enhanced with black outlines:

1. **VU Meter Labels**
   - "ORIGINAL" 
   - "WITH MOJO"
   - Thick 2-pixel black outline with illumination effects

2. **Mojo Bar Labels**
   - "MOJO"
   - "MORE MOJO"
   - "MOST MOJO"
   - 1-pixel black outline

3. **Match Volume Labels**
   - Main label: "Match Volume"
   - State indicators: "OFF" / "ON"
   - 1-pixel black outline

### Why Black Outlines?
- Superior readability against psychedelic gradient backgrounds
- Professional, polished appearance
- Maintains consistency across all text elements
- Allows fill colors to remain vibrant and dynamic
- Illumination and glow effects are more visible

## File Structure

### Modified Files
- `Source/Components/ConsumerGUI.h` - Main implementation

### Assets
- `/Volumes/MP 1/MoreMojo/Knobs/Button-08/Button-08-B-Filmstrip.png` - Toggle switch filmstrip

## Audio Processing Integration

### Signal Flow
```
Input Audio
    ↓
[Input Gain] (adjustable via left slider)
    ↓
[Mojo Processing] (harmonic saturation)
    ↓
[Gain Compensation] ← Match Volume toggle
    ↓
[Output Gain] (adjustable via right slider)
    ↓
Output Audio
```

### Real-Time Metering
- **Input Level**: Measured before processing (RMS with ballistics)
- **Output Level**: Measured after all processing and gain stages
- Both meters update at ~60 FPS with smooth attack/release

## Best Practices

### For Users
1. **Initial Setup**: Load audio and play without Match Volume
2. **Establish Baseline**: Note the character at different mojo levels
3. **Enable Match Volume**: Toggle ON to compare at matched loudness
4. **Make Decisions**: Choose mojo level based on tonal quality, not loudness

### For Developers
1. **Calibration**: Test compensation curve with reference tracks
2. **Metering**: Ensure RMS levels are accurate before applying compensation
3. **User Control**: Keep manual gain controls available for fine-tuning
4. **Visual Feedback**: Maintain clear state indication at all times

## Future Enhancements

### Potential Improvements
- [ ] True loudness metering (LUFS/EBU R128) instead of RMS
- [ ] Per-preset compensation profiles
- [ ] Psychoacoustic model for better perceived loudness matching
- [ ] Undo/redo support for toggle state
- [ ] Keyboard shortcut for quick toggle

### User-Requested Features
- [x] Photorealistic toggle switch visual
- [x] Clear ON/OFF state labels
- [x] Black outlines for readability
- [x] Optimal positioning

## Testing Notes

### Verified Functionality
- ✅ Toggle switches between ON/OFF states
- ✅ Gain compensation applies correctly
- ✅ Visual feedback updates immediately
- ✅ Labels illuminate based on state
- ✅ Black outlines improve readability
- ✅ Integration with existing audio pipeline
- ✅ Real-time processing without artifacts

## Version History

### Version 1.0 (October 2025)
- Initial implementation of Match Volume feature
- Photorealistic filmstrip toggle switch
- Dynamic state labels with illumination
- Black outline enhancement for all GUI text
- Real-time gain compensation algorithm
- Position optimization for workflow

---

**Created**: October 1, 2025  
**Status**: ✅ Production Ready  
**Component**: Consumer GUI  
**Category**: Audio Processing, User Experience
