# More Like The Record Plugin (v3.0)

A professional audio plugin that applies precision timing and phase corrections to make your mixes sound "more like the record" - delivering that elusive commercial polish through advanced DSP processing.

## Features

### DSP Processing Modules
- **TimingSafeResampler**: Windowed-sinc polyphase kernel with intersample peak protection
- **TransientDetect**: Multi-band phase deviation analysis for microsecond precision
- **FractionalDelayAlign**: Per-band (6-band) Linkwitz-Riley crossover with GCC-PHAT ITD estimation
- **AnalogContinuity**: Mid/side elliptical shaping for subtle analog warmth
- **LimiterLevelMatch**: ΔLUFS level-matching (±0.1 LU) with ISP-safe true-peak limiting

### Advanced Visualizations
- **GDS Heatmap**: Group delay spread visualization (frequency vs. group delay)
- **IACC Timeline**: Interaural coherence timeline tracking stereo stability
- **TEI Overlay**: Transient Edge Integrity visualization with waveform overlay
- **Residual Spectrum**: Frequency analysis of processed vs. original signal

### Dual User Interface
- **Consumer Mode**: Simple "More Like The Record" macro slider that controls all parameters
- **Pro Mode**: Detailed control of all processing modules with advanced visualization

### Export & Analysis
- CSV/PNG export of all metrics data
- A/B/X comparison with level-matched auditioning

## Building the Plugin

### Prerequisites
- CMake 3.15+
- C++17 compatible compiler
- JUCE 7.0+

### Building with CMake

1. Clone the JUCE library (if not already installed):
   ```
   git clone https://github.com/juce-framework/JUCE.git
   ```

2. Clone this repository:
   ```
   git clone https://github.com/moremojo/MoreLikeTheRecord.git
   cd MoreLikeTheRecord
   ```

3. Create a build directory:
   ```
   mkdir build
   cd build
   ```

4. Configure and build:
   ```
   cmake .. -DJUCE_DIR=/path/to/JUCE
   cmake --build . --config Release
   ```

### Adding SVG Resources

Place your SVG icon files in the `Resources/icons/` directory before building. These will be automatically embedded into the binary.

## Plugin Formats

The plugin builds in the following formats:
- Audio Unit (macOS)
- VST3 (macOS/Windows)

## Usage

### Consumer Interface
Adjust the "More Like The Record" slider to taste. Higher values apply more processing.

### Pro Interface
Toggle "Pro Mode" to access detailed controls:

1. **Resampler**: Controls sample rate conversion quality and ISP protection
2. **Transient**: Adjusts transient detection sensitivity and processing
3. **Align**: Controls fractional delay and phase alignment
4. **Analog**: Adds subtle analog-like frequency shaping
5. **Limiter**: Manages ceiling and level matching
6. **Metrics Hub**: Displays detailed analytics about your signal

## License

Copyright © 2025 MoreMojo Audio
All rights reserved.

## Credits

Developed by the MoreMojo Audio Team
