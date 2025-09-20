import Cocoa

/// Enum mapping Swift component types to C API component types
public enum JUCEComponentType: Int32 {
    case spectrumAnalyzer = 0
    case proEQ = 1
    case proCompressor = 2
}

/// Swift wrapper for JUCE components
public class JUCEComponentView: NSView {
    // Private reference to the C++ component wrapper
    private var componentWrapper: UnsafeMutablePointer<JUCEComponentWrapper>? = nil
    private var componentType: JUCEComponentType = .spectrumAnalyzer
    
    /// Initialize with component type
    public init(frame: NSRect, componentType: JUCEComponentType) {
        self.componentType = componentType
        super.init(frame: frame)
        setupComponent()
    }
    
    required init?(coder: NSCoder) {
        super.init(coder: coder)
        setupComponent()
    }
    
    deinit {
        destroyComponent()
    }
    
    /// Setup the JUCE component
    private func setupComponent() {
        // Create the JUCE component
        componentWrapper = CreateJUCEComponent(JUCEComponentType(rawValue: componentType.rawValue)!, 
                                              Float(bounds.width), 
                                              Float(bounds.height))
        
        // Get the native NSView
        if let nativeView = GetNativeView(componentWrapper), let view = nativeView {
            // Cast the opaque pointer back to NSView
            let nsView = Unmanaged<NSView>.fromOpaque(view).takeUnretainedValue()
            nsView.frame = bounds
            nsView.autoresizingMask = [.width, .height]
            addSubview(nsView)
        }
    }
    
    /// Clean up the JUCE component
    private func destroyComponent() {
        if let wrapper = componentWrapper {
            DestroyJUCEComponent(wrapper)
            componentWrapper = nil
        }
    }
    
    // MARK: - Layout
    
    public override func layout() {
        super.layout()
        if let wrapper = componentWrapper {
            ResizeJUCEComponent(wrapper, Float(bounds.width), Float(bounds.height))
        }
    }
    
    // MARK: - Parameter Control
    
    /// Set a parameter value
    public func setParameter(_ parameterId: String, value: Float) {
        if let wrapper = componentWrapper {
            parameterId.withCString { cString in
                SetParameterValue(wrapper, cString, value)
            }
        }
    }
    
    /// Get a parameter value
    public func getParameter(_ parameterId: String) -> Float {
        if let wrapper = componentWrapper {
            return parameterId.withCString { cString in
                return GetParameterValue(wrapper, cString)
            }
        }
        return 0.0
    }
    
    // MARK: - Audio Processing
    
    /// Process audio buffer
    public func processAudio(_ bufferLeft: [Float], _ bufferRight: [Float]) {
        guard let wrapper = componentWrapper else { return }
        
        let numChannels = 2
        let numSamples = min(bufferLeft.count, bufferRight.count)
        
        let audioBuffer = CreateAudioBuffer(Int32(numChannels), Int32(numSamples))
        defer { DestroyAudioBuffer(audioBuffer) }
        
        // Copy data into the C buffer
        bufferLeft.enumerated().forEach { (index, value) in
            SetAudioBufferData(audioBuffer, 0, Int32(index), value)
        }
        
        bufferRight.enumerated().forEach { (index, value) in
            SetAudioBufferData(audioBuffer, 1, Int32(index), value)
        }
        
        // Process the audio
        ProcessAudioBuffer(wrapper, audioBuffer)
    }
}
