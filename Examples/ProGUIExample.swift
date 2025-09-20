import SwiftUI
import Cocoa

// Example of how to use Pro GUI components in SwiftUI
struct ProGUIEQView: NSViewRepresentable {
    // EQ parameters
    @Binding var band1Gain: Float
    @Binding var band1Freq: Float
    @Binding var band1Q: Float
    
    // Audio buffer for processing
    var audioBuffer: (left: [Float], right: [Float])?
    
    func makeNSView(context: Context) -> NSView {
        // Create the ProGUIBridge
        let bridge = ProGUIBridge(type: .eq, frame: NSRect(x: 0, y: 0, width: 600, height: 400))
        
        // Set initial parameter values
        bridge.setEQBandGain(band: 1, gainDB: band1Gain)
        bridge.setEQBandFrequency(band: 1, frequency: band1Freq)
        bridge.setEQBandQ(band: 1, q: band1Q)
        
        return bridge.view
    }
    
    func updateNSView(_ nsView: NSView, context: Context) {
        // Get the ProGUIBridge from the view's first subview
        if let view = nsView.subviews.first?.superview as? NSView,
           let bridge = context.coordinator.bridge {
            
            // Update parameters if they've changed
            let currentGain = bridge.getEQBandGain(band: 1)
            if currentGain != band1Gain {
                bridge.setEQBandGain(band: 1, gainDB: band1Gain)
            }
            
            let currentFreq = bridge.getEQBandFrequency(band: 1)
            if currentFreq != band1Freq {
                bridge.setEQBandFrequency(band: 1, frequency: band1Freq)
            }
            
            let currentQ = bridge.getEQBandQ(band: 1)
            if currentQ != band1Q {
                bridge.setEQBandQ(band: 1, q: band1Q)
            }
            
            // Process audio if available
            if let buffer = audioBuffer {
                bridge.processAudio(leftChannel: buffer.left, rightChannel: buffer.right)
            }
        }
    }
    
    func makeCoordinator() -> Coordinator {
        Coordinator(self)
    }
    
    class Coordinator {
        var parent: ProGUIEQView
        var bridge: ProGUIBridge?
        
        init(_ parent: ProGUIEQView) {
            self.parent = parent
            self.bridge = ProGUIBridge(type: .eq, frame: NSRect(x: 0, y: 0, width: 600, height: 400))
        }
    }
}
