# MoreMojo Audio Processing Suite

A professional audio enhancement suite featuring a photorealistic GUI, high-quality DSP, and plugin integration.

## Features

- Two-level GUI interface: Simple View and detailed Mojo Maker
- High-quality audio processing with multiple interpolation strategies
- Photorealistic controls including knobs, chicken-head selectors, and VU meters
- Plugin integration (AU/VST3) using JUCE
- Automated build and CI/CD pipeline

## Build Instructions

### Prerequisites

- macOS 11.0 or later
- Xcode 13.0 or later
- Command line tools

### Building the App and Plugins

```bash
# Clone the repository
git clone https://github.com/yourusername/more-mojo.git
cd more-mojo

# Install required tools
make bootstrap

# Build the macOS app
make app

# Build the audio plugins (AU/VST3)
make plugin

# Package everything into the dist directory
make package
```

## Project Structure

- `app/` - macOS application
  - `Sources/` - Swift source files
  - `Assets.xcassets/` - Image resources
- `plugin/` - Audio plugin (JUCE)
  - `Source/` - C++ source files
- `scripts/` - Build and utility scripts
- `.github/workflows/` - CI/CD configuration

## DSP Implementation

The audio processing features:

- Multiple interpolation strategies (HB 4×, Sinc 8×, Spline 4×, Adaptive)
- Analog-style shaping with ADAA-like processing
- Variable drive, character, saturation, and presence
- Mode tilt (Warm/Vintage/Tape/Tube)
- Mix & output controls with TP safety
- Comprehensive metering with spectrum analysis

## Development Workflow

1. Make changes to the source code
2. Run `make app` or `make plugin` to build locally
3. Push to GitHub to trigger automatic builds
4. Download artifacts from GitHub Actions

## License

All rights reserved. Copyright © 2025.
