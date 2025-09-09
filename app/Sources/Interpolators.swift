import Foundation
import Accelerate

// Protocol defining the interface for audio interpolation algorithms
protocol InterpolatorProtocol {
    var oversamplingFactor: Int { get }
    func configure(sampleRate: Double)
    func up(_ input: [Float], _ output: inout [Float])
    func down(_ input: [Float], _ output: inout [Float])
    func upStereo(_ inputL: [Float], _ inputR: [Float], _ outputL: inout [Float], _ outputR: inout [Float])
    func downStereo(_ inputL: [Float], _ inputR: [Float], _ outputL: inout [Float], _ outputR: inout [Float])
}

// Half-band interpolator (4x oversampling) - efficient for real-time use
class HalftBandInterpolator: InterpolatorProtocol {
    let oversamplingFactor = 4
    private var sr: Double = 44100
    private var coeffs: [Float] = []
    
    func configure(sampleRate: Double) {
        self.sr = sampleRate
        // Initialize filter coefficients for half-band filtering
        // (simplified for this example)
        coeffs = [0.02, 0.0, -0.12, 0.0, 0.6, 1.0, 0.6, 0.0, -0.12, 0.0, 0.02]
    }
    
    func up(_ input: [Float], _ output: inout [Float]) {
        // Zero-stuffing and filtering - simplified version
        let n = input.count
        output = [Float](repeating: 0, count: n * oversamplingFactor)
        
        for i in 0..<n {
            output[i * oversamplingFactor] = input[i] * oversamplingFactor
        }
        
        // In real implementation, apply multi-stage filtering
    }
    
    func down(_ input: [Float], _ output: inout [Float]) {
        // Decimation - simplified version
        let outN = input.count / oversamplingFactor
        output = [Float](repeating: 0, count: outN)
        
        for i in 0..<outN {
            output[i] = input[i * oversamplingFactor]
        }
        
        // In real implementation, apply anti-aliasing filter first
    }
    
    func upStereo(_ inputL: [Float], _ inputR: [Float], _ outputL: inout [Float], _ outputR: inout [Float]) {
        up(inputL, &outputL)
        up(inputR, &outputR)
    }
    
    func downStereo(_ inputL: [Float], _ inputR: [Float], _ outputL: inout [Float], _ outputR: inout [Float]) {
        down(inputL, &outputL)
        down(inputR, &outputR)
    }
}

// Sinc interpolator (8x oversampling) - high quality for offline rendering
class SincInterpolator: InterpolatorProtocol {
    let oversamplingFactor = 8
    private var sr: Double = 44100
    
    func configure(sampleRate: Double) {
        self.sr = sampleRate
    }
    
    func up(_ input: [Float], _ output: inout [Float]) {
        // High-quality sinc interpolation - simplified
        let n = input.count
        output = [Float](repeating: 0, count: n * oversamplingFactor)
        
        for i in 0..<n {
            output[i * oversamplingFactor] = input[i] * Float(oversamplingFactor)
        }
        
        // In real implementation, apply windowed sinc filtering
    }
    
    func down(_ input: [Float], _ output: inout [Float]) {
        // High-quality decimation - simplified
        let outN = input.count / oversamplingFactor
        output = [Float](repeating: 0, count: outN)
        
        for i in 0..<outN {
            output[i] = input[i * oversamplingFactor]
        }
        
        // In real implementation, apply high-quality anti-aliasing filter
    }
    
    func upStereo(_ inputL: [Float], _ inputR: [Float], _ outputL: inout [Float], _ outputR: inout [Float]) {
        up(inputL, &outputL)
        up(inputR, &outputR)
    }
    
    func downStereo(_ inputL: [Float], _ inputR: [Float], _ outputL: inout [Float], _ outputR: inout [Float]) {
        down(inputL, &outputL)
        down(inputR, &outputR)
    }
}

// Spline interpolator optimized for transients (4x oversampling)
class TransientSplineInterpolator: InterpolatorProtocol {
    let oversamplingFactor = 4
    private var sr: Double = 44100
    
    func configure(sampleRate: Double) {
        self.sr = sampleRate
    }
    
    func up(_ input: [Float], _ output: inout [Float]) {
        // Transient-preserving spline interpolation - simplified
        let n = input.count
        output = [Float](repeating: 0, count: n * oversamplingFactor)
        
        for i in 0..<n {
            output[i * oversamplingFactor] = input[i] * Float(oversamplingFactor)
        }
    }
    
    func down(_ input: [Float], _ output: inout [Float]) {
        // Spline-based decimation - simplified
        let outN = input.count / oversamplingFactor
        output = [Float](repeating: 0, count: outN)
        
        for i in 0..<outN {
            output[i] = input[i * oversamplingFactor]
        }
    }
    
    func upStereo(_ inputL: [Float], _ inputR: [Float], _ outputL: inout [Float], _ outputR: inout [Float]) {
        up(inputL, &outputL)
        up(inputR, &outputR)
    }
    
    func downStereo(_ inputL: [Float], _ inputR: [Float], _ outputL: inout [Float], _ outputR: inout [Float]) {
        down(inputL, &outputL)
        down(inputR, &outputR)
    }
}

// Adaptive interpolator that switches based on audio content
class AdaptiveInterpolator: InterpolatorProtocol {
    let oversamplingFactor = 4
    private var sr: Double = 44100
    private let hbInterpolator = HalftBandInterpolator()
    private let splineInterpolator = TransientSplineInterpolator()
    
    func configure(sampleRate: Double) {
        self.sr = sampleRate
        hbInterpolator.configure(sampleRate: sampleRate)
        splineInterpolator.configure(sampleRate: sampleRate)
    }
    
    func up(_ input: [Float], _ output: inout [Float]) {
        // Adaptive selection based on audio content - simplified
        // For transient sections, use spline; for sustained, use half-band
        hbInterpolator.up(input, &output)
    }
    
    func down(_ input: [Float], _ output: inout [Float]) {
        hbInterpolator.down(input, &output)
    }
    
    func upStereo(_ inputL: [Float], _ inputR: [Float], _ outputL: inout [Float], _ outputR: inout [Float]) {
        up(inputL, &outputL)
        up(inputR, &outputR)
    }
    
    func downStereo(_ inputL: [Float], _ inputR: [Float], _ outputL: inout [Float], _ outputR: inout [Float]) {
        down(inputL, &outputL)
        down(inputR, &outputR)
    }
}

// Analog shaping algorithm
class AnalogInterpolator {
    private var drive: Float = 0.5
    private var character: Float = 0.5
    private var saturation: Float = 0.5
    private var presence: Float = 0.5
    
    func configure(drive: Float, character: Float, saturation: Float, presence: Float) {
        self.drive = drive
        self.character = character
        self.saturation = saturation
        self.presence = presence
    }
    
    func processSample(_ sample: Float) -> Float {
        // Apply drive gain
        var processed = sample * (1.0 + drive * 3.0)
        
        // Apply character (asymmetric distortion)
        processed += character * 0.2 * sin(processed)
        
        // Apply saturation (tanh waveshaper)
        processed = tanh(processed * (0.5 + saturation))
        
        // Apply presence (high frequency enhancement)
        // This is simplified; real implementation would use a filter
        
        return processed
    }
    
    func processStereo(_ leftSample: Float, _ rightSample: Float) -> (left: Float, right: Float) {
        return (processSample(leftSample), processSample(rightSample))
    }
}
