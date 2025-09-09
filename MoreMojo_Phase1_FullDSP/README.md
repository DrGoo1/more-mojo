
# More Mojo — Phase 1 (Full DSP App + Plugin + Automation)

## Local build
```bash
make bootstrap
make app
make plugin
make package
```
Artifacts in `./dist`.

## App (macOS)
- Two-level GUI (Simple presets, Mojo Maker with photo-real controls).
- Full DSP pipeline included:
  - Interp: HB 4× / Sinc 8× / Transient Spline 4× / Adaptive
  - Analog shaper (ADAA-like): Drive/Character/Saturation/Presence + mode tilt
  - Wet/Dry + Output (smoothed), TP-safe hook
  - Meters (RMS/Peak + Spectrum), Integrity snapshot hooks

## Plugin (JUCE AU/VST3)
- Oversampling scaffold (4× live / 8× offline). Add the analog shaper in `processBlock()` on the upsampled block.

## CI
- `.github/workflows/*` build app & plugin on macOS, upload artifacts to PRs.

## Assets
- Place your image sets in `app/Assets.xcassets/MoreMojo/` and map names in `MoreMojoAssets.swift`.
