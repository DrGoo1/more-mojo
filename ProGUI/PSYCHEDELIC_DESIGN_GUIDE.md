# Psychedelic STM Design Guide

**Created**: September 30, 2025  
**Status**: Phase 1 Complete - Visual System Implemented

---

## 🎨 Design Philosophy

**Inspiration**: Peter Max, Yellow Submarine, Austin Powers  
**Era**: 1960s-70s Psychedelic Revival  
**Mood**: Groovy, Fun, Colorful, Energetic  
**Audience**: Consumer/Creative users (vs. Professional/Technical)

---

## 🌈 Color System (Implemented)

### Primary Palette
```
Electric Pink    #FF1493  ████  Vibrant, eye-catching
Psychedelic Purple #9B30FF  ████  Deep, mystical
Cosmic Orange    #FF8C00  ████  Warm, energetic
Groovy Green     #00FF7F  ████  Fresh, lively
Sky Blue         #00BFFF  ████  Cool, spacious
Sunshine Yellow  #FFD700  ████  Bright, happy
```

### Usage Guidelines
- **Backgrounds**: Gradients (never solid)
- **Text**: White with colorful outlines or shadows
- **Buttons**: Gradient fills with 3D depth
- **Meters**: Color transitions based on value
- **Accents**: Rainbow gradients for emphasis

### Gradient Styles
1. **Sunset Swirl**: Pink → Orange → Purple (vertical)
2. **Cosmic Flow**: Blue → Purple → Pink (vertical)
3. **Rainbow Trail**: All colors (horizontal)
4. **Metallic**: Base color with highlights/shadows

---

## 🎭 Visual Effects (Implemented)

### 3D Effects
- **Drop Shadow**: 6-8px depth, black @ 50% opacity
- **Specular Highlight**: White @ 60% on top third
- **Glow Halo**: Expanding circles with fade
- **Metallic Gradient**: Light top, dark bottom + highlights

### Animation
- **Refresh Rate**: 20fps (50ms timer)
- **Twinkling Stars**: 50 particles, sine wave alpha
- **Rainbow Halo**: Pulsing intensity on amount knob
- **Smooth Transitions**: All state changes animated

### Typography
- **Main Title**: 40pt bold, rainbow gradient fill, white outline (3px)
- **Section Headers**: 16pt bold, yellow color
- **Body Text**: 13-14pt regular, white @ 70-80%
- **Percentage Values**: 48pt bold, pink color
- **Labels**: 14pt bold with drop shadow

---

## 🎛️ Component Designs

### Buttons

**Groovy 3D Button**:
```
States:
  Normal:  Gradient fill, drop shadow, specular highlight
  Hover:   Added glow halo (0.8 intensity)
  Pressed: Inverted gradient, translated down 2px

Styling:
  - Border radius: 10px
  - Text: 16pt bold white
  - Height: 40-60px
  - Gradient: Top lighter, bottom darker
  - Outline: White @ 30% opacity
```

**Color Mapping**:
- Upload Button: Orange → Pink
- Analyze Button: Blue → Purple → Pink (with 🔮 icon)
- Apply Button: Gold → Orange → Pink (with 🎨 icon)
- Preview Buttons: Blue (Original), Pink (Mojo), Orange (Only)

### Meters

**Psychedelic Horizontal Meter**:
```
Layout:
  [Label]  [████████░░] 80% 🔥
   80px      400px       30px

Colors (value-based):
  0-33%:  Blue → Cyan (cool)
  34-66%: Green → Yellow (warm)
  67-100%: Orange → Pink (hot)

Effects:
  - Rounded ends (5px radius)
  - Gradient fill in filled portion
  - Dark background track
  - Light border (white @ 20%)
  - Emoji indicator on right
```

**Character Meters** (5 total):
- 🔥 Warmth
- 📻 Vintage
- 💥 Punch
- 🌌 Space
- ✨ Shimmer

### Radar Chart

**Mojo Radar (8-dimensional)**:
```
Structure:
  - Center point (pink dot)
  - 3 concentric circles (purple @ 30%)
  - 8 radial axes (purple @ 50%)
  - Filled polygon (green fill @ 30%, stroke @ 100%)

Dimensions:
  1. Warmth (top)
  2. Vintage (top-right)
  3. Punch (right)
  4. Smooth (bottom-right)
  5. Space (bottom)
  6. Depth (bottom-left)
  7. Width (left)
  8. Modern (top-left)

Size: 300×300px
Animation: Pulsing when analyzing
```

### Sections

**Groovy Section Container**:
```
Background:
  - Vertical gradient (lighter top)
  - Base: Deep Space (#0A0A1A)
  - Border: Purple glow @ 50%
  - Radius: 10px

Header:
  - 30px tall
  - Yellow text (16pt bold)
  - Left-aligned in 10px padding

Content:
  - 10px padding all sides
  - White text @ 70-80% opacity
```

---

## 📐 Layout Structure

### Window Dimensions
```
Size: 700×900px
Min:  600×800px
Resizable: Yes
```

### Vertical Layout
```
┌─────────────────────────────────┐
│ Title Area         100px        │
├─────────────────────────────────┤
│ Step 1: Upload     120px        │
├─────────────────────────────────┤
│ Step 2: Choose     100px        │
├─────────────────────────────────┤
│ Step 3: Extract    340px        │
│   - Analyze button              │
│   - Radar chart                 │
│   - Character meters            │
├─────────────────────────────────┤
│ Step 4: Apply      240px        │
│   - Amount display              │
│   - Slider                      │
│   - Preview buttons             │
│   - Apply button                │
└─────────────────────────────────┘

Total: ~900px + padding
```

### Horizontal Margins
```
Side margins: 20px
Inner padding: 10px per section
Button spacing: 10px between
Control width: Responsive to container
```

---

## ✨ Interactive Behaviors

### Upload Button
```
Click → File Chooser
Formats: WAV, AIFF, MP3, FLAC
On Success:
  - Update selected file display
  - Show filename with 🎵 icon
  - Show duration and sample rate
  - Reset analysis state
```

### Instrument Dropdown
```
9 Options with Emojis:
  🎤 Vocal
  🥁 Drums
  🎸 Bass
  🎸 Guitar
  🎹 Piano/Keys
  🎻 Strings
  🎺 Brass/Winds
  🎛️ Synth
  🎵 Full Mix

On Change:
  - Store selected instrument
  - Update internal profile
```

### Analyze Button
```
Pre-flight Check:
  - File uploaded? → Show warning if not
  - Valid format? → Show error if not

On Click:
  1. Disable button
  2. Show progress (future)
  3. Run Mojo analysis
  4. Update radar chart
  5. Update character meters
  6. Enable preview buttons
  7. Show success message

Visual Feedback:
  - Rotating 🔮 icon during analysis
  - Pulsing rainbow border
  - Sparkle burst on completion
```

### Amount Slider
```
Range: 0-150%
Default: 75%
Step: 1%

Visual Feedback:
  - Rainbow track gradient
  - Large percentage display (48pt)
  - Pulsing halo around display
  - Real-time meter updates

Creative Use:
  - 0-100%: Normal range
  - 100-150%: Exaggerated "over-Mojo"
```

### Preview Buttons
```
Three modes:
  ▶ Original   - Play source audio
  ▶ With Mojo  - Play with effect applied
  ▶ Mojo Only  - Play difference signal

States:
  Disabled: Gray, not clickable (no analysis yet)
  Enabled:  Colorful, clickable
  Playing:  Animated equalizer bars
  
Visual:
  - Pill shape (120×30px)
  - Gradient fills
  - Play icon (▶)
```

### Apply Button
```
Size: 300×60px (prominent!)
Color: Animated gold→orange→pink flow
Icon: 🎨 spinning palette

Pre-flight Check:
  - Analysis complete? → Warning if not
  - Valid amount? → Validation

On Click:
  1. Show progress bar
  2. Apply Mojo processing
  3. Update target track
  4. Flash success effect
  5. Show completion message

Message Style:
  - Fun language ("Far Out!", "Groovy!")
  - Confirmation with details
```

---

## 🎭 Animation Timing

### Timer System
```cpp
startTimer(50);  // 20fps
```

### Animation Loop
```
Every 50ms:
  1. Increment phase (animationPhase += 0.1)
  2. Update star twinkles (sin wave)
  3. Update halo intensity (sin wave)
  4. Repaint entire component
```

### Smooth Transitions
```
Use juce::ComponentAnimator for:
  - Window open/close
  - Section expand/collapse
  - Control state changes
  - Value changes (0.2s duration)
```

---

## 🔧 Technical Implementation

### File Structure
```
PsychedelicTheme.h
  - Color constants
  - Gradient creators
  - Drawing utilities
  - Effect functions

PsychedelicStealMojoComponent.h
  - Main component class
  - UI layout
  - Event handlers
  - Animation logic
  - State management
```

### Key Classes
```cpp
namespace PsychedelicTheme {
  namespace Colors { ... }
  
  // Gradient creators
  ColourGradient createSunsetSwirl(Rectangle)
  ColourGradient createCosmicFlow(Rectangle)
  ColourGradient createRainbowTrail(Rectangle)
  
  // Effect functions
  void drawGlowHalo(Graphics&, Rectangle, Colour, intensity)
  void drawDropShadow(Graphics&, Rectangle, depth)
  void drawSpecularHighlight(Graphics&, Rectangle, angle)
  
  // Component drawers
  void drawGroovyButton(Graphics&, Rectangle, text, color, ...)
  void drawPsychedelicMeter(Graphics&, Rectangle, value, label, emoji)
  void drawPsychedelicTitle(Graphics&, Rectangle, text)
  void drawSection(Graphics&, Rectangle, title)
}

class PsychedelicStealMojoComponent {
  // UI components
  TextButton uploadButton, analyzeButton, applyButton
  ComboBox instrumentCombo
  Slider amountSlider
  TextButton preview buttons [3]
  
  // State
  File selectedFile
  String selectedInstrument
  bool isAnalyzed
  float mojoAmount
  float animationPhase
  float mojo values [5]  // warmth, vintage, punch, space, shimmer
  
  // Methods
  void handleUpload()
  void handleAnalyze()
  void handleApply()
  void drawAnimatedStars()
  void drawMojoRadar()
  void drawCharacterMeters()
}
```

---

## 🎯 Design Goals Achieved

✅ **Psychedelic Aesthetic**: Rainbow gradients, flowing colors, groovy fonts  
✅ **3D Depth**: Shadows, highlights, metallic gradients  
✅ **Engaging Animations**: Twinkling stars, pulsing halos, smooth transitions  
✅ **Clear Workflow**: 4 distinct steps with visual separation  
✅ **Fun Language**: Emojis, playful messages, groovy terminology  
✅ **Professional Quality**: Smooth rendering, proper effects, polished UI  

---

## 📝 Design Principles

### Do's ✅
- Use gradients everywhere (never flat colors)
- Add depth with shadows and highlights
- Animate important elements
- Use emojis for personality
- Keep text legible (white with outlines/shadows)
- Make buttons obviously clickable (3D effects)
- Provide visual feedback for all interactions

### Don'ts ❌
- Don't use flat solid colors
- Don't make text unreadable (contrast!)
- Don't over-animate (20fps max)
- Don't mix modern/minimal with psychedelic
- Don't forget accessibility (colorblind-friendly)
- Don't make buttons too small (<30px height)

---

## 🚀 Next Phase: Functionality

**Phase 2: DSP Integration** (Coming Next)
- Spectral analyzer
- Transient detector
- Spatial analyzer
- Mojo profile extraction
- Real Mojo values (not random)

**Phase 3: Source Separation**
- AI model integration (Spleeter/Demucs)
- Instrument extraction
- Preview extracted audio

**Phase 4: Mojo Application**
- Spectral matching
- Transient shaping
- Spatial processing
- Character effects
- Real-time preview

---

## 📚 Reference Materials

### Visual Inspiration
- Peter Max art prints (1960s-70s)
- Yellow Submarine (1968 film)
- Austin Powers: The Spy Who Shagged Me (1999)
- Vintage psychedelic concert posters
- 1960s album covers (The Beatles, Pink Floyd, etc.)

### Color Theory
- Triadic color schemes
- Complementary contrasts
- High saturation values
- Warm/cool color transitions

### Typography
- Bubble fonts (rounded, thick strokes)
- Hand-drawn aesthetic
- Bold outlines for readability
- Curved baselines for groovy feel

---

**Status**: Phase 1 Visual Design COMPLETE! 🎉✨

The psychedelic STM interface is now fully functional with:
- Complete visual system
- All 4 steps laid out
- Interactive controls
- Smooth animations
- Ready for DSP integration

**Next**: Click "Steal The Mojo" button and see the groovy interface! 🌈🎵
