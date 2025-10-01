# 🎨 Steal The Mojo - Quick Reference Card

## 🚀 Quick Start

### Build & Run
```bash
cd /Volumes/MP\ 1/MoreMojo/ChatGPT/ProGUI/Tools/JuceRunner/build
cmake --build . --config Debug
open ProGUIJuceRunner_artefacts/"ProGUI JUCE Runner.app"
```

### Open STM Window
1. Launch ProGUI JUCE Runner
2. Click "Steal The Mojo" button (top-right)
3. Window opens at 700×1000px

---

## 📁 File Structure

```
Source/Components/
├── PsychedelicStyles.h      ← Colors + 3D effects
├── PsychedelicKnob.h        ← Custom knob component
├── MojoRadarChart.h         ← 8D visualization
├── CharacterMeter.h         ← Animated meters
└── StealMojoComponent.h     ← Main window
```

---

## 🎨 Color Quick Reference

```cpp
PsychedelicColors::electricPink    // #FF1493
PsychedelicColors::psychPurple     // #9B30FF
PsychedelicColors::cosmicOrange    // #FF8C00
PsychedelicColors::groovyGreen     // #00FF7F
PsychedelicColors::skyBlue         // #00BFFF
PsychedelicColors::sunshineYellow  // #FFD700
```

---

## 🎛️ Component Sizes

| Component | Size | Location |
|-----------|------|----------|
| Main Window | 700×1000px | - |
| Title Area | 700×80px | Top |
| Upload Button | 200×45px | Step 1 |
| Instrument Dropdown | 250×35px | Step 2 |
| Analyze Button | 220×50px | Step 3 |
| Radar Chart | 200×200px | Step 3 |
| Character Meter | 400×25px | Step 3 |
| Psychedelic Knob | 120×120px | Step 4 |
| Apply Button | 300×55px | Bottom |

---

## 🔧 Key Functions

### PsychedelicStyles.h
```cpp
// Gradients
createSunsetSwirl(area)     // Pink→Orange→Purple
createCosmicFlow(area)      // Blue→Purple→Pink
createRainbowTrail(area)    // Full rainbow

// 3D Effects
drawGlowHalo(g, area, color, intensity)
drawDropShadow(g, area, depth)
drawMetallicGradient(g, area, color)
drawPsychedelicKnob(g, area, rotation, amount)
```

### PsychedelicKnob
```cpp
setAmount(0.75f)           // Set to 75%
getAmount()                // Returns 0.0-1.0
onAmountChanged = [](float) {}  // Callback
```

### MojoRadarChart
```cpp
setMojoValues(values)      // 8 dimensions
reset()                    // Clear chart
// Auto-animates at 30fps
```

### CharacterMeter
```cpp
setValue(0.80f)            // Set to 80%
getValue()                 // Returns 0.0-1.0
// Auto-animates at 30fps
```

---

## 🎯 Common Tasks

### Add New Color
```cpp
// In PsychedelicStyles.h
inline const juce::Colour myNewColor (0xFFRRGGBB);
```

### Create Custom Gradient
```cpp
auto gradient = juce::ColourGradient(
    startColor, startX, startY,
    endColor, endX, endY, 
    isRadial
);
gradient.addColour(0.5, middleColor);
g.setGradientFill(gradient);
```

### Add 3D Button
```cpp
Effects3D::draw3DButton(g, area, color, isPressed);
```

### Animate Component
```cpp
class MyComponent : public juce::Component,
                    private juce::Timer {
    MyComponent() { startTimerHz(30); }
    void timerCallback() override { repaint(); }
};
```

---

## 🐛 Troubleshooting

### Build Warnings (Font Deprecation)
```
⚠️ Warning: 'Font' is deprecated
✅ Solution: Non-critical, safe to ignore
Future: Update to FontOptions API
```

### Window Not Opening
```
❌ Check: Button click handler
✅ Fix: Verify btnStealMojo.onClick lambda
```

### Animations Stuttering
```
❌ Issue: Timer frequency too high
✅ Fix: Use startTimerHz(30) not (60)
```

### Colors Not Showing
```
❌ Check: Gradient bounds match component
✅ Fix: Use getLocalBounds().toFloat()
```

---

## 📊 Performance Tips

### Optimize Paint
```cpp
void paint(Graphics& g) override {
    // Cache expensive operations
    static auto gradient = createCosmicFlow(bounds);
    
    // Use reduced areas
    auto reduced = getLocalBounds().reduced(10);
    
    // Limit complex shapes
    g.fillRoundedRectangle(area, 8.0f); // Simple
}
```

### Memory Management
```cpp
// Use unique_ptr for components
std::unique_ptr<CharacterMeter> meter;

// Initialize in constructor
meter = std::make_unique<CharacterMeter>("Label", "🔥");

// Auto-deleted when parent destroyed
```

---

## 🎨 Design Patterns

### Psychedelic Section
```cpp
// 1. Draw gradient background
auto gradient = PsychedelicColors::createCosmicFlow(area);
g.setGradientFill(gradient);
g.fillRoundedRectangle(area, 8.0f);

// 2. Add glow halo
Effects3D::drawGlowHalo(g, area, color, 0.8f);

// 3. Draw main content
g.setColour(juce::Colours::white);
g.drawText(text, area, juce::Justification::centred);
```

### 3D Depth Effect
```cpp
// 1. Shadow first (behind)
Effects3D::drawDropShadow(g, area, 4.0f);

// 2. Main element
Effects3D::drawMetallicGradient(g, area, baseColor);

// 3. Specular highlight (on top)
Effects3D::drawSpecularHighlight(g, area);
```

---

## 📝 Code Snippets

### Create New Meter
```cpp
auto newMeter = std::make_unique<CharacterMeter>(
    "Body",    // Label
    "🎵"       // Emoji
);
newMeter->setValue(0.66f);  // 66%
addAndMakeVisible(newMeter.get());
```

### Rainbow Border
```cpp
auto bounds = getLocalBounds().toFloat();
auto gradient = PsychedelicColors::createRainbowTrail(bounds);
g.setGradientFill(gradient);
g.drawRoundedRectangle(bounds.reduced(2), 20.0f, 4.0f);
```

### Animated Knob
```cpp
PsychedelicKnob knob;
knob.setAmount(0.75f);
knob.onAmountChanged = [this](float amount) {
    // Handle value change
    mojoAmount = amount;
    repaint();
};
addAndMakeVisible(knob);
```

---

## 🎯 Implementation Checklist

### Phase 1: Visual Redesign ✅
- [x] Psychedelic color system
- [x] 3D effects library
- [x] Custom PsychedelicKnob
- [x] MojoRadarChart (8D)
- [x] CharacterMeter (5 types)
- [x] Complete UI layout
- [x] 30fps animations
- [x] File upload system
- [x] Placeholder analysis
- [x] Build & test

### Phase 2: Mojo Analysis (Pending)
- [ ] FFT spectral analyzer
- [ ] Transient detector
- [ ] Spatial analyzer
- [ ] Mojo profile structure
- [ ] Real analysis data

### Phase 3: Source Separation (Pending)
- [ ] AI model integration
- [ ] Band filtering
- [ ] Instrument profiles
- [ ] Preview system

### Phase 4: Application (Pending)
- [ ] Spectral matching
- [ ] Transient shaping
- [ ] Spatial processing
- [ ] Character effects
- [ ] A/B comparison

---

## 🎉 Quick Wins

### Test Visual Design
```bash
# Build and launch
make -C Tools/JuceRunner/build
open Tools/JuceRunner/build/ProGUIJuceRunner_artefacts/"ProGUI JUCE Runner.app"

# Click "Steal The Mojo"
# Verify: Rainbow border, gradient background, title
```

### Test Interactions
```
1. Upload: Click button → Select audio file
2. Choose: Select "Drums" from dropdown
3. Analyze: Click button → See radar + meters animate
4. Amount: Drag knob → See halo change
5. Apply: Click button → See success message
```

### Verify Animations
```
✓ Knob halo pulses
✓ Meters flow left-to-right
✓ Radar fills smoothly
✓ All at 30fps
```

---

## 📚 Related Documentation

- `STEAL_THE_MOJO_SPEC.md` - Full specification
- `STEAL_THE_MOJO_IMPLEMENTATION.md` - Implementation summary
- `STEAL_THE_MOJO_VISUAL_GUIDE.md` - Visual design guide
- `README_PRO_GUI.md` - Pro GUI overview

---

## 🆘 Support

### Common Issues
1. **Build Errors**: Check CMakeLists.txt includes all files
2. **Missing Components**: Verify #include paths
3. **Crashes**: Check Timer inheritance (private juce::Timer)
4. **No Visuals**: Verify addAndMakeVisible() called

### Debug Mode
```cpp
// Add to paint() for debugging
g.setColour(juce::Colours::red);
g.drawRect(getLocalBounds(), 2);  // Show bounds
```

---

*Quick reference for Steal The Mojo psychedelic interface* 🎨✨
