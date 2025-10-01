# Steal The Mojo (STM) - Complete Specification

**Version**: 2.0 - Psychedelic Consumer Edition  
**Date**: September 30, 2025  
**Status**: Design & Implementation Specification

---

## 🎨 Visual Design Concept

### Psychedelic Theme Inspiration
- **Peter Max**: Bold, vibrant colors with flowing curves and cosmic patterns
- **Yellow Submarine**: Whimsical, hand-drawn aesthetic with bright primaries
- **Austin Powers (The Spy Who Shagged Me)**: Groovy 60s/70s retro-futurism with metallic finishes

### Key Visual Elements

#### Color Palette
```
PRIMARY COLORS (Bold & Vibrant):
- Electric Pink:     #FF1493 (Hot Pink)
- Psychedelic Purple: #9B30FF (Purple)
- Cosmic Orange:     #FF8C00 (Dark Orange)
- Groovy Green:      #00FF7F (Spring Green)
- Sky Blue:          #00BFFF (Deep Sky Blue)
- Sunshine Yellow:   #FFD700 (Gold)

GRADIENT BACKGROUNDS:
- Sunset Swirl:      Pink → Orange → Purple
- Cosmic Flow:       Blue → Purple → Pink
- Groovy Rainbow:    All colors in smooth gradient

METALLIC ACCENTS (3D depth):
- Chrome Silver:     #C0C0C0 with specular highlights
- Gold Shine:        #FFD700 with gradient to #FFA500
- Copper Glow:       #B87333 with warm highlights
```

#### Typography
```
MAIN TITLE: 
- Font Style: Bubble/Rounded with thick outlines
- Effects: Rainbow gradient fill, white outline stroke (3px)
- Size: 32-40pt
- Example: "STEAL THE MOJO"

LABELS:
- Font Style: Bold, slightly curved baseline
- Effects: Shadow/3D depth, contrasting colors
- Size: 14-18pt

BUTTONS:
- Font Style: Bold with rounded edges
- Effects: Embossed/raised 3D look
- Size: 16-20pt
```

#### 3D Control Design

**Knobs**:
- Circular with gradient shading (light at top, dark at bottom)
- Metallic rim with specular highlights
- Center indicator: Glowing neon line or dot
- Rotation: Shows depth with shadow changes
- Rainbow halo effect when active

**Buttons**:
- Pill-shaped with rounded corners
- Gradient fill (lighter at top for 3D effect)
- Drop shadow for depth
- Pressed state: Inverted gradient + moved shadow
- Glow effect on hover

**Sliders**:
- Track: Gradient with rainbow sections
- Thumb: 3D sphere or pill shape
- Trail: Filled portion shows vibrant color

**Meters**:
- Flowing wave patterns instead of bars
- Pulsing glow effect
- Color shifts based on level (cool → warm)

---

## 🎵 Mojo Extraction & Analysis Pipeline

### Phase 1: Audio Upload & Track Extraction

#### 1.1 Upload Audio File
```
Input Formats:
- WAV (PCM, Float)
- AIFF
- MP3 (decoded to float)
- FLAC (decoded to float)

Constraints:
- Max file size: 500MB
- Max duration: 15 minutes
- Sample rates: 44.1k - 192kHz
- Bit depths: 16, 24, 32-bit
```

#### 1.2 Track Selection/Isolation
```
Methods:
a) Source Separation (AI-based):
   - Vocal extraction
   - Drum extraction  
   - Bass extraction
   - Harmonic/percussive separation
   
b) Frequency Band Isolation:
   - Low (20Hz - 250Hz): Bass/Kick
   - Low-Mid (250Hz - 2kHz): Body/Warmth
   - Mid-High (2kHz - 8kHz): Presence/Clarity
   - High (8kHz - 20kHz): Air/Sparkle

c) Manual EQ Shaping:
   - User-guided band selection
   - Real-time preview
```

#### 1.3 Instrument Profile Selection
```
Profiles (9 options):
1. Vocal        - Focus on 200Hz-8kHz, formants, breathiness
2. Drums        - Transients, punch, tonal decay
3. Bass         - Sub frequencies, sustain, harmonics
4. Guitar       - Pick attack, body resonance, harmonics
5. Piano/Keys   - Hammer attack, sustain, tonal balance
6. Strings      - Bow articulation, vibrato, warmth
7. Brass/Winds  - Breath noise, dynamic envelope, timbre
8. Synth        - Harmonic content, modulation, movement
9. Full Mix     - Overall cohesion, spatial qualities

Each profile optimizes analysis for specific characteristics.
```

---

### Phase 2: Mojo Quantification

#### 2.1 Multi-Dimensional Analysis

**A) Spectral Characteristics**
```
- Harmonic Richness: Even vs odd harmonics ratio
- Spectral Tilt: Energy distribution across frequency
- Formant Emphasis: Vowel-like resonances
- High Frequency Character: Air, sparkle, crispness
- Low Frequency Weight: Warmth, body, power
```

**B) Temporal Characteristics**
```
- Transient Profile: Attack sharpness, decay rate
- Envelope Shape: ADSR characteristics
- Micro-timing: Groove, swing, human feel
- Dynamic Range: Loud/quiet variation
- Sustain Quality: Tail behavior, decay
```

**C) Spatial Characteristics**
```
- Stereo Width: L-R correlation, spread
- Depth: Near/far perception cues
- Phase Relationships: Coherence, imaging
- Ambience: Room sound, reflections
```

**D) Tonal Characteristics**
```
- Pitch Stability: Vibrato, drift, tuning
- Harmonic Distortion: Tube-like, tape-like saturation
- Compression Character: Natural dynamics vs controlled
- EQ Curve: Overall tonal balance signature
```

**E) Character Qualities ("The Magic")**
```
- Analog Warmth: Even harmonics, saturation
- Digital Artifacts: Aliasing, quantization (to avoid or add)
- Vintage Quality: Tape wow/flutter, tube coloration
- Modern Polish: Clarity, precision, cleanliness
- Live Energy: Room ambience, performance dynamics
```

#### 2.2 Mojo Signature Extraction

**Create a Mojo Profile** (multi-parameter fingerprint):
```json
{
  "spectral": {
    "tilt": -3.5,           // dB/octave
    "harmonicRatio": 0.68,  // even/odd
    "airiness": 0.45,       // 10kHz+ energy
    "warmth": 0.72          // <500Hz presence
  },
  "temporal": {
    "transientSharpness": 0.85,
    "attackTime": 2.3,      // ms
    "decayRate": 0.62,
    "dynamicRange": 18.5    // dB
  },
  "spatial": {
    "stereoWidth": 0.55,
    "depth": 0.40,
    "correlation": 0.78
  },
  "character": {
    "analogWarmth": 0.65,
    "saturation": 0.38,
    "compression": 0.45,
    "vintage": 0.52
  }
}
```

#### 2.3 Visualization of Mojo
```
Display Types:
1. Radar Chart: 8-dimensional Mojo profile
2. Spectrum Graph: Frequency response curve
3. Waveform: Transient and dynamic envelope
4. Stereo Field: L-R imaging visualization
5. Character Bars: Warmth, Vintage, Polish meters
```

---

### Phase 3: Mojo Application

#### 3.1 Target Audio Selection
```
Options:
a) Main audio track (already loaded in Pro GUI)
b) New upload for consumer-only mode
c) Real-time input (future: live processing)
```

#### 3.2 Mojo Transfer Processing

**Processing Chain**:
```
1. Spectral Matching:
   - Multiband dynamic EQ to match tilt
   - Harmonic enhancement to match ratio
   - High-shelf for air character
   - Low-shelf for warmth

2. Temporal Matching:
   - Transient shaping to match attack
   - Envelope following for dynamics
   - Micro-timing adjustment (subtle)

3. Spatial Matching:
   - Stereo width processor
   - Mid-side adjustment
   - Phase alignment

4. Character Matching:
   - Harmonic saturation (tube/tape modeling)
   - Subtle compression for dynamics
   - Vintage effects (tape flutter, vinyl crackle - optional)

5. Amount Control:
   - 0%: No processing (bypass)
   - 50%: Balanced application
   - 100%: Full Mojo transfer
   - >100%: Exaggerated effect (creative)
```

#### 3.3 Real-Time Preview
```
Features:
- A/B comparison (Original / With Mojo)
- Solo extracted Mojo elements
- Difference signal (what's being added)
- Spectral comparison display
- Waveform before/after
```

---

## 🎛️ Updated STM Window Layout

### Window Specifications
```
Size: 700px × 900px (resizable, min 600×800)
Background: Psychedelic gradient (sunset swirl or cosmic flow)
Border: Rounded corners (20px) with rainbow gradient outline
Style: 3D depth with drop shadows throughout
```

### Control Layout

```
┌─────────────────────────────────────────────────────────────┐
│  ✨ STEAL THE MOJO ✨                              [X]      │ (Title bar)
├─────────────────────────────────────────────────────────────┤
│                                                               │
│  ╔═══════════════════════════════════════════════════════╗  │
│  ║         🎵  S T E A L   T H E   M O J O  🎵          ║  │ (Main title)
│  ║           Grab the vibe from any track!               ║  │
│  ╚═══════════════════════════════════════════════════════╝  │
│                                                               │
│  ┌─────────────────────────────────────────────────────┐    │
│  │ STEP 1: UPLOAD REFERENCE AUDIO                      │    │
│  │                                                       │    │
│  │   [  📁  UPLOAD AUDIO FILE  ]   ← Groovy button     │    │
│  │                                                       │    │
│  │   🎵 Selected: funky_bass_line.wav                   │    │
│  │   Duration: 3:42  •  Sample Rate: 48kHz              │    │
│  └─────────────────────────────────────────────────────┘    │
│                                                               │
│  ┌─────────────────────────────────────────────────────┐    │
│  │ STEP 2: CHOOSE THE VIBE                             │    │
│  │                                                       │    │
│  │   What kind of mojo are you stealing?                │    │
│  │                                                       │    │
│  │   [Instrument Type ▼]  ← Psychedelic dropdown        │    │
│  │    • Vocal        • Drums      • Bass                │    │
│  │    • Guitar       • Piano      • Strings             │    │
│  │    • Brass/Winds  • Synth      • Full Mix            │    │
│  └─────────────────────────────────────────────────────┘    │
│                                                               │
│  ┌─────────────────────────────────────────────────────┐    │
│  │ STEP 3: EXTRACT THE MOJO                            │    │
│  │                                                       │    │
│  │   [  🔮  ANALYZE MOJO  ]  ← 3D glowing button        │    │
│  │                                                       │    │
│  │   ┌──────────── MOJO RADAR ────────────┐            │    │
│  │   │         Warmth                      │            │    │
│  │   │            *                        │            │    │
│  │   │  Vintage  * * *  Modern             │            │    │
│  │   │         *  [X]  *                   │            │    │
│  │   │  Punch  *       *  Smooth           │            │    │
│  │   │         *       *                   │            │    │
│  │   │           Space                     │            │    │
│  │   └─────────────────────────────────────┘            │    │
│  │                                                       │    │
│  │   Character Meters:                                  │    │
│  │   Warmth:    ████████░░ 80%  🔥                      │    │
│  │   Vintage:   ██████░░░░ 60%  📻                      │    │
│  │   Punch:     ███████░░░ 70%  💥                      │    │
│  │   Space:     █████░░░░░ 50%  🌌                      │    │
│  │   Shimmer:   ████████░░ 75%  ✨                      │    │
│  └─────────────────────────────────────────────────────┘    │
│                                                               │
│  ┌─────────────────────────────────────────────────────┐    │
│  │ STEP 4: APPLY THE MOJO                              │    │
│  │                                                       │    │
│  │           ┌─────────────────┐                        │    │
│  │           │                 │                        │    │
│  │           │    [AMOUNT]     │  ← Giant 3D knob       │    │
│  │           │                 │     with rainbow halo  │    │
│  │           │       75%       │                        │    │
│  │           └─────────────────┘                        │    │
│  │                                                       │    │
│  │   How much mojo do you want?                         │    │
│  │   0% ═════●═════════ 100%                            │    │
│  │        └─ Slider with rainbow trail                  │    │
│  │                                                       │    │
│  │   Preview Options:                                   │    │
│  │   [▶ Original] [▶ With Mojo] [▶ Mojo Only]          │    │
│  │                                                       │    │
│  └─────────────────────────────────────────────────────┘    │
│                                                               │
│  ┌─────────────────────────────────────────────────────┐    │
│  │                                                       │    │
│  │   [  🎨  APPLY MOJO TO MY TRACK  ]  ← Big button    │    │
│  │                                                       │    │
│  │   Status: Ready to steal that groovy vibe! ✨        │    │
│  └─────────────────────────────────────────────────────┘    │
│                                                               │
└─────────────────────────────────────────────────────────────┘
```

---

## 🎨 Detailed Control Specifications

### 1. Title Area
```
Text: "STEAL THE MOJO"
Font: 40pt bubble font with rainbow gradient
Effect: White 4px outline, drop shadow
Subtitle: "Grab the vibe from any track!"
Font: 16pt, italic, yellow with pink shadow
```

### 2. Upload Button
```
Shape: Rounded rectangle (180px × 50px)
Colors: Gradient orange→pink
Icon: 📁 folder icon (3D)
Text: "UPLOAD AUDIO FILE" (18pt bold, white)
Hover: Pulsing glow effect
Press: Inset shadow, slight scale down
```

### 3. Instrument Dropdown
```
Shape: Rounded rectangle (250px × 40px)
Colors: Purple gradient with gold border
Text: 18pt bold, white
Icons: Each option has emoji + 3D icon
Open: Slides down with bounce animation
Hover: Item highlights with neon glow
```

### 4. Analyze Button
```
Shape: Rounded rectangle (200px × 60px)
Colors: Gradient blue→purple→pink
Icon: 🔮 crystal ball (animated spin)
Text: "ANALYZE MOJO" (20pt bold, white)
Hover: Rainbow halo pulsing
Press: Sparkle burst animation
Active: Progress spinner in 60s style
```

### 5. Mojo Radar Chart
```
Type: Spider/Radar chart (8 dimensions)
Size: 300px × 300px
Background: Concentric circles (purple→pink gradient)
Lines: Neon green glow
Fill: Translucent cyan with shimmer
Labels: Bubble font, rotating around perimeter
Animation: Pulsing when analyzing
```

### 6. Character Meters
```
Type: Horizontal bars with emoji indicators
Width: 400px × 30px each
Colors: Rainbow gradient based on value
  0-33%:  Blue → Cyan (cool)
  34-66%: Green → Yellow (warm)
  67-100%: Orange → Pink (hot)
Fill: Animated flow (left to right)
Labels: Bold 14pt with drop shadow
Emojis: 3D rendered, 24px
```

### 7. Amount Knob (Main Control)
```
Type: Large rotary knob
Size: 180px × 180px
Body: Metallic silver with specular highlights
Indicator: Neon line from center to edge
Range: 0-100% (extensible to 150% for creative)
Halo: Rainbow gradient glow (intensity = position)
Center Display: Large 48pt percentage value
Rotation: Smooth with momentum physics
```

### 8. Amount Slider (Alternative/Fine Tune)
```
Type: Horizontal slider
Size: 400px × 40px
Track: Rainbow gradient background
Fill: Bright flowing color (animated)
Thumb: 3D sphere with metallic shine (50px)
Labels: 0%, 50%, 100% in bubble font
Shadow: Deep drop shadow for 3D depth
```

### 9. Preview Buttons
```
Shape: Pill buttons (120px × 35px each)
Colors:
  - Original: Blue gradient
  - With Mojo: Pink gradient  
  - Mojo Only: Orange gradient
Icons: ▶ play symbol (3D)
Text: 14pt bold white
Active: Pulsing border glow
Playing: Animated equalizer bars
```

### 10. Apply Button (Primary Action)
```
Shape: Large rounded rectangle (300px × 70px)
Colors: Gradient gold→orange→pink (animated flow)
Icon: 🎨 palette icon (3D spinning)
Text: "APPLY MOJO TO MY TRACK" (22pt bold)
Hover: Expanding glow halo
Press: Scale down + bright flash
Processing: Rotating rainbow border
```

---

## 🔧 Technical Implementation

### UI Framework
```
Technology: JUCE with custom graphics
Components: All custom-drawn for psychedelic style
Animation: 60fps with OpenGL acceleration
Effects: Custom shaders for gradients and glows
```

### Graphics Assets Needed
```
Fonts:
- Bubble font (license-free alternative to Cooper Black)
- Groovy serif (similar to Benguiat)

Textures:
- Metallic gradient maps
- Specular highlight overlays
- Rainbow gradient strips
- Glow/halo alpha masks

Icons:
- 3D rendered emojis (or license-free alternatives)
- Custom instrument symbols
- Playback controls
- Status indicators
```

### Color System Implementation
```cpp
// Psychedelic color palette
namespace PsychedelicColors {
    const juce::Colour electricPink   (0xFFFF1493);
    const juce::Colour psychPurple    (0xFF9B30FF);
    const juce::Colour cosmicOrange   (0xFFFF8C00);
    const juce::Colour groovyGreen    (0xFF00FF7F);
    const juce::Colour skyBlue        (0xFF00BFFF);
    const juce::Colour sunshineYellow (0xFFFFD700);
    
    // Gradient creators
    juce::ColourGradient createSunsetSwirl();
    juce::ColourGradient createCosmicFlow();
    juce::ColourGradient createRainbowTrail();
}

// 3D effect helpers
namespace Effects3D {
    void drawGlowHalo(Graphics& g, Rectangle area, Colour color, float intensity);
    void drawDropShadow(Graphics& g, Path shape, float depth);
    void drawSpecularHighlight(Graphics& g, Rectangle area, float angle);
    void drawMetallicGradient(Graphics& g, Rectangle area, Colour base);
}
```

### Animation System
```cpp
class PsychedelicAnimator {
public:
    void startPulse(Component* comp, float frequency);
    void startGlow(Component* comp, Colour color);
    void startRainbowFlow(Component* comp, float speed);
    void startSparkle(Component* comp, int particleCount);
    void startRotate(Component* comp, float rpm);
};
```

---

## 📊 Mojo Analysis Engine

### DSP Pipeline Architecture
```
┌─────────────────┐
│ Audio Input     │
│ (Reference)     │
└────────┬────────┘
         │
         ▼
┌─────────────────┐
│ Source          │
│ Separation      │ ← AI model or band filtering
└────────┬────────┘
         │
         ▼
┌─────────────────┐
│ Multi-Band      │
│ Analysis        │ ← FFT, envelope, correlation
└────────┬────────┘
         │
         ├─→ Spectral Analysis  → Tilt, Harmonics, Air
         ├─→ Temporal Analysis  → Transients, Dynamics
         ├─→ Spatial Analysis   → Width, Depth, Phase
         └─→ Character Analysis → Warmth, Saturation
         
         ▼
┌─────────────────┐
│ Mojo Profile    │
│ Extraction      │ ← Create signature
└────────┬────────┘
         │
         ▼
┌─────────────────┐
│ Mojo Profile    │
│ Storage         │ ← JSON format
└─────────────────┘
```

### Processing Modules Required

**Module 1: Source Separator**
```
Options:
- Spleeter (Python/TensorFlow)
- Demucs (PyTorch)
- Open-Unmix (PyTorch)
- Band filtering fallback
```

**Module 2: Spectral Analyzer**
```
- FFT: 4096-8192 bins
- Windowing: Hann or Blackman-Harris
- Overlap: 75%
- Features:
  * Spectral centroid
  * Spectral rolloff
  * Harmonic ratio
  * Spectral flux
```

**Module 3: Transient Detector**
```
- Envelope follower
- Peak detection
- Attack/decay measurement
- RMS dynamics tracking
```

**Module 4: Spatial Analyzer**
```
- Mid-side decomposition
- Correlation coefficient
- Phase coherence
- Stereo width calculation
```

**Module 5: Character Extractor**
```
- Harmonic distortion analysis
- Saturation curve fitting
- Compression detection
- Vintage effect detection
```

### Mojo Application Engine
```
┌─────────────────┐
│ Target Audio    │
│ (User's Track)  │
└────────┬────────┘
         │
         ▼
┌─────────────────┐
│ Load Mojo       │
│ Profile         │
└────────┬────────┘
         │
         ▼
┌─────────────────┐
│ Apply           │
│ Processing      │
│ Chain           │
└────────┬────────┘
         │
         ├─→ Spectral Matching  → EQ, filtering
         ├─→ Temporal Matching  → Transient shaping
         ├─→ Spatial Matching   → Width, imaging
         └─→ Character Matching → Saturation, compression
         
         ▼
┌─────────────────┐
│ Amount Control  │
│ Mix             │ ← 0-100% wet/dry
└────────┬────────┘
         │
         ▼
┌─────────────────┐
│ Output Audio    │
│ (With Mojo)     │
└─────────────────┘
```

---

## 🚀 Implementation Phases

### Phase 1: Visual Redesign (Week 1-2)
- [ ] Create psychedelic color system
- [ ] Design 3D controls (knobs, buttons, sliders)
- [ ] Implement gradient backgrounds
- [ ] Add animation system
- [ ] Create custom fonts/text rendering
- [ ] Build Mojo radar chart visualization
- [ ] Design character meters

### Phase 2: Basic Mojo Analysis (Week 3-4)
- [ ] Implement spectral analyzer
- [ ] Add transient detector
- [ ] Create spatial analyzer
- [ ] Build basic Mojo profile structure
- [ ] Display analysis results

### Phase 3: Source Separation (Week 5-6)
- [ ] Integrate AI separation model (or)
- [ ] Implement band filtering fallback
- [ ] Add instrument profile presets
- [ ] Create extraction preview

### Phase 4: Mojo Application (Week 7-8)
- [ ] Build spectral matching EQ
- [ ] Implement transient shaper
- [ ] Add spatial processors
- [ ] Create character effects
- [ ] Wire amount control
- [ ] Add A/B comparison

### Phase 5: Polish & Integration (Week 9-10)
- [ ] Performance optimization
- [ ] Real-time preview
- [ ] Save/load Mojo profiles
- [ ] Integration with Pro GUI
- [ ] User testing
- [ ] Final visual polish

---

## 📝 User Workflow

### Typical Use Case
```
1. User opens "Steal The Mojo" window
2. Uploads reference audio (e.g., favorite vinyl recording)
3. Selects instrument type (e.g., "Drums")
4. Clicks "Analyze Mojo" → sees radar chart + meters
5. Reviews the extracted Mojo characteristics
6. Adjusts Amount knob to 75%
7. Previews result with "With Mojo" button
8. Fine-tunes amount for desired effect
9. Clicks "Apply Mojo to My Track"
10. Processed audio ready in main track
```

---

## 🎯 Success Criteria

### Visual
- [ ] Unmistakably psychedelic aesthetic
- [ ] Smooth 60fps animations
- [ ] Professional 3D depth effects
- [ ] Readable despite colorful design
- [ ] Fun and engaging interface

### Functional
- [ ] Accurate Mojo extraction (<5% error)
- [ ] Real-time preview (<100ms latency)
- [ ] Effective Mojo application (user satisfaction >80%)
- [ ] Stable performance (no crashes)
- [ ] Intuitive workflow (learnable in <5 min)

---

**Next Steps**: Review spec, approve design direction, begin Phase 1 implementation.

---

*"Groovy, baby! Let's steal some mojo!" 🎵✨*
