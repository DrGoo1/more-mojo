import Foundation
import Cocoa
import OSLog

/// Enum defining the component types
@objc public enum ProGUIComponentType: Int32 {
    case eq = 0
    case compressor = 1
    case demo = 2
}

/// Swift class for interfacing with the PluginGuiMagic components
@objc public class ProGUIBridge: NSObject {
    // MARK: - Properties
    
    // Private reference to the C++ component wrapper
    private var componentPtr: UnsafeMutableRawPointer?
    
    // Store component type
    private var componentType: ProGUIComponentType
    
    // NSView that will be used to display the component
    public var view: NSView
    
    // MARK: - Lifecycle
    
    /// Initialize with component type and frame
    public init(type: ProGUIComponentType, frame: CGRect) {
        self.componentType = type
        self.view = NSView(frame: frame)
        self.view.wantsLayer = true
        
        let logger = Logger(subsystem: "com.moremojo.progui.testapp", category: "Bridge")

        super.init()
        
        // Create the component
        componentPtr = ProGUI_CreateComponent(Int32(type.rawValue),
                                             Float(frame.width),
                                             Float(frame.height))

        // Force attach-only parenting for reliability
        if let component = componentPtr {
            let parentPtr = Unmanaged.passUnretained(self.view).toOpaque()
            ProGUI_AttachToParent(component, parentPtr)
            NSLog("[Bridge] Called ProGUI_AttachToParent (forced attach-only)")
            logger.info("[ProGUIBridge] Called ProGUI_AttachToParent (forced attach-only)")
        } else {
            NSLog("[Bridge] ERROR: componentPtr is nil in init")
            logger.error("[ProGUIBridge] ERROR: componentPtr is nil")
        }

        // Deferred layout to ensure correct sizing and visibility
        DispatchQueue.main.asyncAfter(deadline: .now() + 0.05) { [weak self] in
            guard let self = self else { return }
            let b = self.view.bounds
            if let component = self.componentPtr {
                ProGUI_ResizeComponent(component, Float(b.width), Float(b.height))
            }
            self.view.subviews.forEach { sub in
                sub.frame = b
            }
            self.view.needsLayout = true
            self.view.layoutSubtreeIfNeeded()
            NSLog("[Bridge] Deferred layout applied; subviews=\(self.view.subviews.count) bounds=\(NSStringFromRect(self.view.bounds))")
            logger.info("[ProGUIBridge] Deferred layout applied; subviews: \(self.view.subviews.count)")
        }

        logger.info("[ProGUIBridge] Embedded JUCE view. Container subviews: \(self.view.subviews.count)")

    }

    deinit {
        // Clean up C++ resources
        if let component = componentPtr {
            ProGUI_DestroyComponent(component)
            componentPtr = nil
        }
    }
    
    // MARK: - Layout
    
    /// Update the size of the component
    public func resize(width: CGFloat, height: CGFloat) {
        if let component = componentPtr {
            ProGUI_ResizeComponent(component, Float(width), Float(height))
            view.frame = CGRect(x: view.frame.minX, y: view.frame.minY, width: width, height: height)
        }
    }
    
    // MARK: - Parameter Controls
    
    /// Set a parameter value
    public func setParameter(_ parameterId: String, value: Float) {
        if let component = componentPtr {
            parameterId.withCString { cString in
                ProGUI_SetParameterValue(component, cString, value)
            }
        }
    }
    
    /// Get a parameter value
    public func getParameter(_ parameterId: String) -> Float {
        if let component = componentPtr {
            return parameterId.withCString { cString in
                return ProGUI_GetParameterValue(component, cString)
            }
        }
        return 0.0
    }
    
    // MARK: - Audio Processing
    
    /// Process audio through the component
    public func processAudio(leftChannel: [Float], rightChannel: [Float]) {
        guard let component = componentPtr else { return }
        
        let numSamples = min(leftChannel.count, rightChannel.count)
        let numChannels = 2
        
        // Create a temporary array to hold pointers to the channel data
        let dataPointers = UnsafeMutablePointer<UnsafeMutablePointer<Float>?>.allocate(capacity: numChannels)
        defer { dataPointers.deallocate() }
        
        // Create buffers for the channel data
        let leftBuffer = UnsafeMutablePointer<Float>.allocate(capacity: numSamples)
        let rightBuffer = UnsafeMutablePointer<Float>.allocate(capacity: numSamples)
        defer {
            leftBuffer.deallocate()
            rightBuffer.deallocate()
        }
        
        // Copy the channel data to the buffers
        for i in 0..<numSamples {
            leftBuffer[i] = leftChannel[i]
            rightBuffer[i] = rightChannel[i]
        }
        
        // Set up the data pointers
        dataPointers[0] = leftBuffer
        dataPointers[1] = rightBuffer
        
        // Process the audio
        ProGUI_ProcessAudio(component, dataPointers, Int32(numChannels), Int32(numSamples))
    }

    // MARK: - Overlay and Test Signal
    public func setOverlayVisible(_ visible: Bool) {
        setParameter("ui.overlayVisible", value: visible ? 1.0 : 0.0)
    }

    public func setTestSignalEnabled(_ enabled: Bool) {
        setParameter("ui.testSignal", value: enabled ? 1.0 : 0.0)
    }
}

// MARK: - EQ Component Convenience Methods

public extension ProGUIBridge {
    /// Set the gain for an EQ band
    func setEQBandGain(band: Int, gainDB: Float) {
        let paramId = "eq.band\(band).gain"
        setParameter(paramId, value: gainDB)
    }
    
    /// Set the frequency for an EQ band
    func setEQBandFrequency(band: Int, frequency: Float) {
        let paramId = "eq.band\(band).freq"
        setParameter(paramId, value: frequency)
    }
    
    /// Set the Q factor for an EQ band
    func setEQBandQ(band: Int, q: Float) {
        let paramId = "eq.band\(band).q"
        setParameter(paramId, value: q)
    }
    
    /// Get the gain for an EQ band
    func getEQBandGain(band: Int) -> Float {
        let paramId = "eq.band\(band).gain"
        return getParameter(paramId)
    }
    
    /// Get the frequency for an EQ band
    func getEQBandFrequency(band: Int) -> Float {
        let paramId = "eq.band\(band).freq"
        return getParameter(paramId)
    }
    
    /// Get the Q factor for an EQ band
    func getEQBandQ(band: Int) -> Float {
        let paramId = "eq.band\(band).q"
        return getParameter(paramId)
    }
}

// MARK: - Compressor Component Convenience Methods

public extension ProGUIBridge {
    /// Set the threshold for the compressor
    func setCompressorThreshold(_ thresholdDB: Float) {
        setParameter("comp.threshold", value: thresholdDB)
    }
    
    /// Set the ratio for the compressor
    func setCompressorRatio(_ ratio: Float) {
        setParameter("comp.ratio", value: ratio)
    }
    
    /// Set the attack time for the compressor
    func setCompressorAttack(_ attackMs: Float) {
        setParameter("comp.attack", value: attackMs)
    }
    
    /// Set the release time for the compressor
    func setCompressorRelease(_ releaseMs: Float) {
        setParameter("comp.release", value: releaseMs)
    }
    
    /// Set the knee width for the compressor
    func setCompressorKnee(_ kneeDB: Float) {
        setParameter("comp.knee", value: kneeDB)
    }
    
    /// Set the makeup gain for the compressor
    func setCompressorMakeupGain(_ makeupDB: Float) {
        setParameter("comp.makeup", value: makeupDB)
    }
    
    /// Get the threshold for the compressor
    func getCompressorThreshold() -> Float {
        return getParameter("comp.threshold")
    }
    
    /// Get the ratio for the compressor
    func getCompressorRatio() -> Float {
        return getParameter("comp.ratio")
    }
    
    /// Get the attack time for the compressor
    func getCompressorAttack() -> Float {
        return getParameter("comp.attack")
    }
    
    /// Get the release time for the compressor
    func getCompressorRelease() -> Float {
        return getParameter("comp.release")
    }
    
    /// Get the knee width for the compressor
    func getCompressorKnee() -> Float {
        return getParameter("comp.knee")
    }
    
    /// Get the makeup gain for the compressor
    func getCompressorMakeupGain() -> Float {
        return getParameter("comp.makeup")
    }
}
