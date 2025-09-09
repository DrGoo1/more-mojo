import Foundation
import Accelerate

// Common protocol for all interpolators
protocol InterpolatorProtocol {
    var oversampleFactor: Int { get }
    func process(samples: [Float]) -> [Float]
    func downsample(samples: [Float]) -> [Float]
}

// Half-band filter interpolator (4x)
class HalftBandInterpolator: InterpolatorProtocol {
    let oversampleFactor = 4
    
    // Half-band FIR filter coefficients
    private let filterCoefficients: [Float] = [
        0.0029, 0.0, -0.0161, 0.0, 0.0493, 0.0,
        -0.0956, 0.0, 0.3148, 0.5, 0.3148, 0.0,
        -0.0956, 0.0, 0.0493, 0.0, -0.0161, 0.0, 0.0029
    ]
    
    func process(samples: [Float]) -> [Float] {
        let originalLength = samples.count
        let oversampledLength = originalLength * oversampleFactor
        
        // Create array with zeros inserted between each sample
        var upsampled = [Float](repeating: 0.0, count: oversampledLength)
        for i in 0..<originalLength {
            upsampled[i * oversampleFactor] = samples[i] * Float(oversampleFactor)
        }
        
        // Apply half-band filter for each stage
        var result = upsampled
        for _ in 0..<2 { // Two stages for 4x oversampling
            result = applyFilter(to: result)
        }
        
        return result
    }
    
    func downsample(samples: [Float]) -> [Float] {
        let oversampledLength = samples.count
        let originalLength = oversampledLength / oversampleFactor
        
        // Decimation (take every 4th sample)
        var result = [Float](repeating: 0.0, count: originalLength)
        for i in 0..<originalLength {
            result[i] = samples[i * oversampleFactor]
        }
        
        return result
    }
    
    private func applyFilter(to samples: [Float]) -> [Float] {
        let length = samples.count
        var result = [Float](repeating: 0.0, count: length)
        
        // Direct convolution implementation
        let filterLength = filterCoefficients.count
        let halfFilterLength = filterLength / 2
        
        for i in 0..<length {
            var sum: Float = 0.0
            for j in 0..<filterLength {
                let sampleIndex = i + j - halfFilterLength
                
                // Handle edge cases
                if sampleIndex >= 0 && sampleIndex < length {
                    sum += samples[sampleIndex] * filterCoefficients[j]
                }
            }
            result[i] = sum
        }
        
        return result
    }
}

// Windowed-sinc interpolator (8x)
class SincInterpolator: InterpolatorProtocol {
    let oversampleFactor = 8
    
    // Kaiser-windowed sinc filter coefficients (simplified)
    private let filterCoefficients: [Float] = {
        // This would normally be a larger array of pre-computed coefficients
        // Simplified version with placeholder values
        return [Float](repeating: 0.0, count: 64).enumerated().map { i, _ in
            let x = Float(i) / 32.0 - 1.0
            if x == 0 {
                return 1.0
            } else {
                return sin(Float.pi * x) / (Float.pi * x) * (0.42 + 0.5 * cos(Float.pi * x) + 0.08 * cos(2 * Float.pi * x))
            }
        }
    }()
    
    func process(samples: [Float]) -> [Float] {
        let originalLength = samples.count
        let oversampledLength = originalLength * oversampleFactor
        
        // Create array with zeros inserted between each sample
        var upsampled = [Float](repeating: 0.0, count: oversampledLength)
        for i in 0..<originalLength {
            upsampled[i * oversampleFactor] = samples[i] * Float(oversampleFactor)
        }
        
        // Apply sinc filter
        let result = applyFilter(to: upsampled)
        
        return result
    }
    
    func downsample(samples: [Float]) -> [Float] {
        let oversampledLength = samples.count
        let originalLength = oversampledLength / oversampleFactor
        
        // Apply anti-aliasing filter before decimation
        let filtered = applyFilter(to: samples)
        
        // Decimate
        var result = [Float](repeating: 0.0, count: originalLength)
        for i in 0..<originalLength {
            result[i] = filtered[i * oversampleFactor]
        }
        
        return result
    }
    
    private func applyFilter(to samples: [Float]) -> [Float] {
        let length = samples.count
        var result = [Float](repeating: 0.0, count: length)
        
        // Simplified implementation - would use vDSP in production
        let filterLength = filterCoefficients.count
        let halfFilterLength = filterLength / 2
        
        for i in 0..<length {
            var sum: Float = 0.0
            for j in 0..<filterLength {
                let sampleIndex = i + j - halfFilterLength
                
                // Handle edge cases
                if sampleIndex >= 0 && sampleIndex < length {
                    sum += samples[sampleIndex] * filterCoefficients[j]
                }
            }
            result[i] = sum
        }
        
        return result
    }
}

// Spline interpolator (4x)
class TransientSplineInterpolator: InterpolatorProtocol {
    let oversampleFactor = 4
    
    func process(samples: [Float]) -> [Float] {
        let originalLength = samples.count
        let oversampledLength = originalLength * oversampleFactor
        
        var result = [Float](repeating: 0.0, count: oversampledLength)
        
        // Hermite spline interpolation
        for i in 0..<(originalLength - 1) {
            let x0 = i > 0 ? samples[i-1] : samples[i]
            let x1 = samples[i]
            let x2 = samples[i+1]
            let x3 = i < originalLength - 2 ? samples[i+2] : samples[i+1]
            
            for j in 0..<oversampleFactor {
                let t = Float(j) / Float(oversampleFactor)
                result[i * oversampleFactor + j] = hermiteInterpolate(x0, x1, x2, x3, t)
            }
        }
        
        // Handle the last segment
        let lastIndex = originalLength - 1
        let x0 = samples[lastIndex - 1]
        let x1 = samples[lastIndex]
        let x2 = samples[lastIndex]
        let x3 = samples[lastIndex]
        
        for j in 0..<oversampleFactor {
            let t = Float(j) / Float(oversampleFactor)
            result[lastIndex * oversampleFactor + j] = hermiteInterpolate(x0, x1, x2, x3, t)
        }
        
        return result
    }
    
    func downsample(samples: [Float]) -> [Float] {
        let oversampledLength = samples.count
        let originalLength = oversampledLength / oversampleFactor
        
        // Simple decimation
        var result = [Float](repeating: 0.0, count: originalLength)
        for i in 0..<originalLength {
            result[i] = samples[i * oversampleFactor]
        }
        
        return result
    }
    
    // Hermite cubic spline interpolation
    private func hermiteInterpolate(_ x0: Float, _ x1: Float, _ x2: Float, _ x3: Float, _ t: Float) -> Float {
        // Tension and bias parameters
        let tension: Float = 0.0
        let bias: Float = 0.0
        
        // Calculate hermite basis functions
        let t2 = t * t
        let t3 = t2 * t
        
        var m0 = (x2 - x0) * (1.0 + bias) * (1.0 - tension) / 2.0
        m0 += (x3 - x1) * (1.0 - bias) * (1.0 - tension) / 2.0
        
        var m1 = (x3 - x1) * (1.0 + bias) * (1.0 - tension) / 2.0
        m1 += (x2 - x0) * (1.0 - bias) * (1.0 - tension) / 2.0
        
        let a0 = 2.0 * t3 - 3.0 * t2 + 1.0
        let a1 = t3 - 2.0 * t2 + t
        let a2 = t3 - t2
        let a3 = -2.0 * t3 + 3.0 * t2
        
        return a0 * x1 + a1 * m0 + a2 * m1 + a3 * x2
    }
}

// Adaptive interpolator (1x-4x)
class AdaptiveInterpolator: InterpolatorProtocol {
    let oversampleFactor = 4 // Maximum oversampling factor
    
    private let hbInterpolator = HalftBandInterpolator()
    private let threshold: Float = 0.5 // Threshold for high frequency content
    
    func process(samples: [Float]) -> [Float] {
        // Check if signal needs oversampling
        if needsOversampling(samples: samples) {
            return hbInterpolator.process(samples: samples)
        } else {
            // No oversampling, just return the original samples
            return samples
        }
    }
    
    func downsample(samples: [Float]) -> [Float] {
        // Only downsample if the array is larger than the original
        if samples.count % oversampleFactor == 0 && samples.count > oversampleFactor {
            return hbInterpolator.downsample(samples: samples)
        } else {
            return samples
        }
    }
    
    // Detect if signal has significant high frequency content
    private func needsOversampling(samples: [Float]) -> Bool {
        // Simple high-frequency content detection
        // Calculate short-term energy in high frequencies
        var highFreqEnergy: Float = 0.0
        var totalEnergy: Float = 0.0
        
        for i in 1..<samples.count {
            let diff = samples[i] - samples[i-1]
            highFreqEnergy += diff * diff
            totalEnergy += samples[i] * samples[i]
        }
        
        if totalEnergy > 0.0001 { // Avoid division by very small numbers
            let ratio = highFreqEnergy / totalEnergy
            return ratio > threshold
        }
        
        return false
    }
}
