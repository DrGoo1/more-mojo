# MLAR v3.1.1 Integration Guide

This guide documents the integration of the "More Like The Record" (MLAR) v3.1.1 module into the MoreMojo project.

## Components Integrated

### 1. Core MLAR Files
- **Source files:** Created in `/shared/mlar/` directory
  - `MLAR.h` - Core header with `Processor` class and `MetricsFrame` struct
  - `MLAR.cpp` - Implementation file with basic audio processing functionality
  - `CMakeLists.txt` - Build configuration for MLAR as a static library
  - `dsp/TimingSafeResampler.h` - Core DSP component for resampling

### 2. Plugin Integration
- **CMake Configuration:** Updated `plugin/CMakeLists.txt` to:
  - Include MLAR module directory
  - Link MLAR static library
  - Set proper paths for JUCE framework
- **Plugin Processor:** Modified `PluginProcessor.h/.cpp` to:
  - Include MLAR header
  - Add MLAR processor instance
  - Implement conditional MLAR processing in the audio pipeline
  - Add MLAR enable/disable toggle parameter

### 3. App Integration
- **Swift Bridge:** Created `app/Sources/MLARIntegration.swift` with:
  - Singleton instance for app-wide access
  - Enable/disable toggle
  - Audio buffer processing method
  - Macro parameter for MLAR intensity control

## Build Instructions

### Plugin Build
To build the plugin with MLAR integration:

```bash
cd plugin/build
cmake ..
make
```

This will generate:
- AU: ~/Library/Audio/Plug-Ins/Components/More Mojo by Umbo Gumbo.component
- VST3: ~/Library/Audio/Plug-Ins/VST3/More Mojo by Umbo Gumbo.vst3
- Standalone: plugin/build/MoreMojoPlugin_artefacts/Standalone/More Mojo by Umbo Gumbo.app

### App Integration
The Swift bridge (`MLARIntegration.swift`) provides integration for the app. To use:

1. Include `MLARIntegration.swift` in your app project
2. Access via the singleton: `MLARIntegration.shared`
3. Enable/disable with: `MLARIntegration.shared.isEnabled = true`
4. Adjust macro value: `MLARIntegration.shared.macroValue = 0.75`
5. Process audio buffers with: `MLARIntegration.shared.process(buffer)`

## Testing

### Plugin Testing
Test the plugin by:
1. Loading in a DAW that supports AU or VST3
2. Toggle the "MLAR" parameter to enable/disable
3. Process audio through the plugin to verify functionality

### Integration Test
A simple test file (`test_mlar_integration.swift`) is provided to verify the Swift integration works correctly. Run it with:

```bash
swift test_mlar_integration.swift
```

## Future Improvements

1. **App Project Integration**: Update app's Xcode project to properly include MLARIntegration.swift
2. **Improved Performance**: Optimize MLAR processing for real-time use
3. **More Parameters**: Add additional MLAR parameters for finer control
4. **UI Controls**: Add dedicated UI controls for MLAR parameters
5. **CI Integration**: Add MLAR integration to CI workflows

## Troubleshooting

### Common Issues
1. **Missing JUCE**: Ensure JUCE is properly located and linked
2. **Build Errors**: Check that MLAR module is correctly included
3. **Audio Glitches**: Verify buffer sizes and processing chain

### Maintaining MLAR
To update MLAR in the future:
1. Replace files in `/shared/mlar/` with newer versions
2. Update version number in documentation
3. Rebuild plugins and test functionality
