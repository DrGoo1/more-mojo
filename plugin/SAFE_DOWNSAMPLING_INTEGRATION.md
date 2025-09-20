# SafeDownsampler Integration with JUCE Oversampling

This document describes the implementation and integration of the `SafeDownsampler` class with JUCE's oversampling system to prevent crashes on Apple Silicon macOS.

## Background

JUCE's `dsp::Oversampling` class has been found to cause segmentation faults and memory corruption on Apple Silicon macOS, specifically during the downsampling phase. These crashes happen because the current implementation uses FFT-based algorithms that can lead to memory alignment issues and numerical instability on ARM processors.

## Implementation Strategy

Our solution integrates a custom `SafeDownsampler` class with JUCE's existing oversampling system:

1. **Keep JUCE Upsampling**: We maintain JUCE's upsampling functionality which works correctly
2. **Replace Downsampling**: We replace JUCE's downsampling with our `SafeDownsampler` implementation
3. **Compile-Time Switch**: A CMake option allows switching between implementations

## SafeDownsampler Overview

The `SafeDownsampler` class is a multichannel polyphase halfband FIR decimator with the following features:

- **Filter Types**: 
  - Standard31: 31-tap FIR (lower quality, less CPU)
  - HQ63: 63-tap FIR (medium quality, medium CPU)
  - HQ127: 127-tap FIR (highest quality, higher CPU)

- **Phase Response**:
  - Linear phase: Perfect phase response, higher latency
  - Minimum phase: Lower latency, slightly distorted phase response

- **Stability Features**:
  - Precomputed filter coefficients (no FFT operations during runtime)
  - Robust memory management with bounds checking
  - Safe buffer resizing
  - Exception handling

## Integration Points

The integration happens in the `Oversampler` class in `PluginProcessor.cpp`:

1. **CMake Configuration**:
   - Option `MOREMOJO_USE_SAFE_DOWNSAMPLER` enables/disables SafeDownsampler
   - Default is ON to prevent crashes

2. **Initialization**:
   - `SafeDownsampler` instances are initialized in the `prepare` method
   - Both X2 and X4 downsampler instances are created for 4x and 8x oversampling

3. **Audio Processing**:
   - JUCE's oversampling is used for upsampling
   - `SafeDownsampler` is used for downsampling when enabled
   - Fallback to JUCE downsampling is available if the CMake option is disabled

4. **Latency Handling**:
   - Both JUCE and SafeDownsampler latencies are accounted for
   - Latency is reported properly to the host

## Usage in Code

To use the SafeDownsampler in your plugin:

```cpp
// In your processor class:
std::unique_ptr<Oversampler> oversampler;

// In prepare to play:
oversampler->prepare(spec, oversamplingFactor); // 4 or 8

// In processBlock:
auto block = juce::dsp::AudioBlock<float>(buffer);
auto osBlock = oversampler->processSamplesUp(block);

// Process your audio with osBlock here
// ...

// Then downsample (uses SafeDownsampler when enabled):
oversampler->processSamplesDown(block);
```

## Build Instructions

1. Configure with SafeDownsampler (default):
   ```
   cmake -DMOREMOJO_USE_SAFE_DOWNSAMPLER=ON ..
   ```

2. Configure with JUCE downsampling (not recommended on Apple Silicon):
   ```
   cmake -DMOREMOJO_USE_SAFE_DOWNSAMPLER=OFF ..
   ```

3. Build the plugin:
   ```
   make
   ```

## Technical Details

The `SafeDownsampler` uses a polyphase implementation of a halfband FIR filter for efficient downsampling. The filter coefficients are precomputed to avoid runtime FFT operations that could cause instability.

The implementation handles buffer copying and filter application in a bounds-checked manner to prevent memory access issues.

## Troubleshooting

If you experience any issues:

1. Ensure `MOREMOJO_USE_SAFE_DOWNSAMPLER` is ON for Apple Silicon
2. Check that buffer sizes are correctly handled in your code
3. Verify that your DSP chain is properly initialized before processing

## Performance Considerations

The SafeDownsampler may use slightly more CPU than JUCE's FFT-based implementation in some cases, but it provides guaranteed stability on Apple Silicon. The quality settings can be adjusted to balance CPU usage vs. audio quality.
