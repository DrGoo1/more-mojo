# MoreMojo Pro GUI Integration Plan

This document outlines the strategy for integrating the JUCE-based professional GUI components into the main MoreMojo Swift application.

## 1. Integration Architecture

### Option 1: Embedded JUCE Views in Swift App (Recommended)
- Create a native Swift wrapper around the JUCE components
- Use `NSViewComponent` from JUCE to embed JUCE components in Swift views
- Pass audio data and parameters between Swift and JUCE components

### Option 2: Separate Plugin Integration
- Build Pro GUI components as AU/VST3 plugins
- Host these plugins within the Swift app
- Use plugin parameters to control the GUI components

## 2. Required Components

### Swift Side
1. **JUCEBridgeView**: A Swift view that can host JUCE components
2. **AudioParameterBridge**: A class to pass audio parameters between Swift and JUCE
3. **AudioBufferBridge**: A class to pass audio data between Swift and JUCE

### JUCE Side
1. **MoreMojoProGUIComponent**: Main wrapper class for all Pro GUI components
2. **SpectrumAnalyzer**: FFT visualization component
3. **ProEQComponent**: EQ visualization and control component 
4. **ProCompressorComponent**: Compressor visualization and control component

## 3. Implementation Steps

### Step 1: Create Swift Wrapper for JUCE Components
```swift
import Cocoa
import JUCEKit // Custom framework to bridge JUCE components

class JUCEComponentView: NSView {
    private var juceComponent: JUCEComponentWrapper?
    
    func initWithComponent(_ componentType: JUCEComponentType) {
        juceComponent = JUCEComponentWrapper(componentType, frame: bounds)
        if let juceView = juceComponent?.nativeView {
            addSubview(juceView)
        }
    }
    
    func updateParameters(_ parameters: [String: Float]) {
        juceComponent?.updateParameters(parameters)
    }
    
    func processAudioBuffer(_ buffer: AudioBuffer) {
        juceComponent?.processAudioBuffer(buffer)
    }
}
```

### Step 2: Create JUCEKit Framework
1. Build a framework that bridges Swift and JUCE
2. Expose the JUCE components as Objective-C++ classes
3. Handle parameter and audio buffer conversion

### Step 3: Modify Build Script
```bash
#!/bin/bash

# Build JUCE components as static library
cd /Volumes/MP\ 1/MoreMojo/ChatGPT/ProGUI
mkdir -p build
cd build
cmake .. -DCMAKE_BUILD_TYPE=Release -DBUILD_SHARED_LIBS=OFF
cmake --build . --config Release

# Copy library to app frameworks
cp libMoreMojoProGUI.a /Volumes/MP\ 1/MoreMojo/ChatGPT/App/Frameworks/

# Build Swift app with JUCE components
cd /Volumes/MP\ 1/MoreMojo/ChatGPT/App
xcodebuild -project MoreMojo.xcodeproj -scheme MoreMojo -configuration Release
```

### Step 4: Add Integration Points in Main App

```swift
// In ProGUIView.swift
import SwiftUI
import JUCEKit

struct ProGUIView: NSViewRepresentable {
    var parameterValues: [String: Float]
    var audioProcessor: AudioProcessor
    
    func makeNSView(context: Context) -> JUCEComponentView {
        let view = JUCEComponentView()
        view.initWithComponent(.eqVisualizer) // or .compressor
        return view
    }
    
    func updateNSView(_ nsView: JUCEComponentView, context: Context) {
        nsView.updateParameters(parameterValues)
        
        if let buffer = audioProcessor.getCurrentBuffer() {
            nsView.processAudioBuffer(buffer)
        }
    }
}
```

## 4. Parameter Binding

### Swift Parameter Class
```swift
struct ProGUIParameter {
    let id: String
    let name: String
    let minValue: Float
    let maxValue: Float
    let defaultValue: Float
    var currentValue: Float
    
    func normalized() -> Float {
        return (currentValue - minValue) / (maxValue - minValue)
    }
}
```

### Parameter Mapping
| Swift Parameter ID | JUCE Parameter ID | Description |
|-------------------|-------------------|-------------|
| eq.band1.gain | band1Gain | EQ Band 1 Gain (-12 to +12 dB) |
| eq.band1.freq | band1Freq | EQ Band 1 Frequency (20-500 Hz) |
| eq.band1.q | band1Q | EQ Band 1 Q Factor (0.1-10) |
| comp.threshold | threshold | Compressor Threshold (-60 to 0 dB) |
| comp.ratio | ratio | Compressor Ratio (1-20) |
| comp.attack | attack | Compressor Attack (0.1-100 ms) |
| comp.release | release | Compressor Release (10-1000 ms) |

## 5. Audio Data Flow

### Audio Buffer Structure
```swift
struct AudioBuffer {
    var leftChannel: [Float]
    var rightChannel: [Float]
    var sampleRate: Double
    var bufferSize: Int
}
```

### Processing Flow
1. Audio is processed in Swift app
2. Buffer is passed to JUCE components for visualization
3. Parameter changes in JUCE components are sent back to Swift app
4. Swift app applies parameter changes to audio processing

## 6. Build Configuration

### Required Build Flags
- `-DJUCE_STANDALONE_APPLICATION=0`
- `-DJUCE_MODULE_AVAILABLE_juce_audio_utils=1`
- `-DJUCE_MODULE_AVAILABLE_juce_dsp=1`
- `-DJUCE_USE_CURL=0`
- `-DJUCE_WEB_BROWSER=0`

### Required Frameworks
- CoreAudio.framework
- CoreMIDI.framework
- Accelerate.framework
- Cocoa.framework
- IOKit.framework

## 7. Testing Strategy

1. Unit test JUCE components in isolation
2. Test Swift wrappers with mock JUCE components
3. Integration test with simple audio signals
4. End-to-end test with real audio processing chain

## 8. Fallback Strategy

If JUCE integration proves too complex, we can fallback to:

1. Using the HTML demos as a reference
2. Implementing simplified versions of the visualizations directly in SwiftUI
3. Focusing on core functionality rather than visual fidelity

## 9. Timeline

1. **Week 1**: Create Swift wrapper and JUCE bridge
2. **Week 2**: Implement parameter binding
3. **Week 3**: Implement audio data flow
4. **Week 4**: Testing and refinement

## 10. Resources

- [JUCE Framework Documentation](https://docs.juce.com)
- [JUCE NSViewComponent Class](https://docs.juce.com/master/classNSViewComponent.html)
- [Swift and C++ Interoperability](https://developer.apple.com/documentation/swift/swift_and_c_plus_plus_interoperability)
