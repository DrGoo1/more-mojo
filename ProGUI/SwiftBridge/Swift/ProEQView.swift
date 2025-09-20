import Cocoa
import SwiftUI

/// SwiftUI wrapper for the ProEQ component
public struct ProEQView: NSViewRepresentable {
    // MARK: - Parameters
    @Binding var band1Gain: Float
    @Binding var band1Freq: Float
    @Binding var band1Q: Float
    
    @Binding var band2Gain: Float
    @Binding var band2Freq: Float
    @Binding var band2Q: Float
    
    @Binding var band3Gain: Float
    @Binding var band3Freq: Float
    @Binding var band3Q: Float
    
    @Binding var band4Gain: Float
    @Binding var band4Freq: Float
    @Binding var band4Q: Float
    
    @Binding var band5Gain: Float
    @Binding var band5Freq: Float
    @Binding var band5Q: Float
    
    // Audio buffer for processing
    var audioBuffer: AudioBuffer?
    
    // MARK: - NSViewRepresentable
    
    public func makeNSView(context: Context) -> JUCEComponentView {
        let view = JUCEComponentView(frame: NSRect(x: 0, y: 0, width: 600, height: 400), 
                                    componentType: .proEQ)
        
        // Initialize with current parameter values
        updateParameters(view)
        
        return view
    }
    
    public func updateNSView(_ nsView: JUCEComponentView, context: Context) {
        // Update parameters
        updateParameters(nsView)
        
        // Process audio if available
        if let buffer = audioBuffer {
            nsView.processAudio(buffer.leftChannel, buffer.rightChannel)
        }
    }
    
    // MARK: - Helper Methods
    
    private func updateParameters(_ view: JUCEComponentView) {
        // Band 1 parameters
        view.setParameter("eq.band1.gain", value: band1Gain)
        view.setParameter("eq.band1.freq", value: band1Freq)
        view.setParameter("eq.band1.q", value: band1Q)
        
        // Band 2 parameters
        view.setParameter("eq.band2.gain", value: band2Gain)
        view.setParameter("eq.band2.freq", value: band2Freq)
        view.setParameter("eq.band2.q", value: band2Q)
        
        // Band 3 parameters
        view.setParameter("eq.band3.gain", value: band3Gain)
        view.setParameter("eq.band3.freq", value: band3Freq)
        view.setParameter("eq.band3.q", value: band3Q)
        
        // Band 4 parameters
        view.setParameter("eq.band4.gain", value: band4Gain)
        view.setParameter("eq.band4.freq", value: band4Freq)
        view.setParameter("eq.band4.q", value: band4Q)
        
        // Band 5 parameters
        view.setParameter("eq.band5.gain", value: band5Gain)
        view.setParameter("eq.band5.freq", value: band5Freq)
        view.setParameter("eq.band5.q", value: band5Q)
    }
}

// AudioBuffer struct for passing audio data
public struct AudioBuffer {
    var leftChannel: [Float]
    var rightChannel: [Float]
    
    public init(leftChannel: [Float], rightChannel: [Float]) {
        self.leftChannel = leftChannel
        self.rightChannel = rightChannel
    }
}
