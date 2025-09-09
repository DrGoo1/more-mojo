import Foundation
import Accelerate

/// Analog-style audio shaper with ADAA (Anti-Derivative Anti-Aliasing) processing
class AnalogInterpolator {
    
    // MARK: - Properties
    
    // Processing parameters
    private var drive: Float = 0.5
    private var character: Float = 0.5
    private var saturation: Float = 0.5
    private var presence: Float = 0.5
    private var warmth: Float = 0.5
    private var outputGain: Float = 0.0
    private var mix: Float = 1.0
    private var mode: Int = 1
    
    // Internal state variables
    private var prevSample: Float = 0.0
    private var prevIntegral: Float = 0.0
    private var dcOffset: Float = 0.0
    private var sampleRate: Float = 48000.0
    
    // Shaper types
    private enum ShaperType: Int {
        case soft = 0    // Soft saturation (subtle)
        case warm = 1    // Warm tube-like
        case tape = 2    // Tape saturation
        case vintage = 3 // Vintage console
    }
    
    // MARK: - Public Methods
    
    /// Process audio samples with analog-style shaping
    /// - Parameter samples: Input audio samples
    /// - Returns: Processed audio samples
    func process(samples: [Float]) -> [Float] {
        let count = samples.count
        var result = [Float](repeating: 0.0, count: count)
        var drySignal = samples
        
        // Process each sample with the selected shaping algorithm
        for i in 0..<count {
            // Anti-aliased version of the shaping
            result[i] = processADAA(sample: samples[i])
        }
        
        // Apply presence enhancement if needed
        if presence > 0.01 {
            applyPresence(to: &result)
        }
        
        // Apply warmth (low-end enhancement)
        if warmth > 0.01 {
            applyWarmth(to: &result)
        }
        
        // Mix dry/wet signals
        if mix < 0.999 {
            vDSP_vsmul(result, 1, &mix, &result, 1, vDSP_Length(count))
            
            var dryMix: Float = 1.0 - mix
            vDSP_vsmul(drySignal, 1, &dryMix, &drySignal, 1, vDSP_Length(count))
            
            vDSP_vadd(result, 1, drySignal, 1, &result, 1, vDSP_Length(count))
        }
        
        // Apply output gain
        if outputGain != 0.0 {
            var gainFactor = powf(10.0, outputGain / 20.0) // Convert dB to linear
            vDSP_vsmul(result, 1, &gainFactor, &result, 1, vDSP_Length(count))
        }
        
        return result
    }
    
    /// Update the processing parameters
    /// - Parameter params: New parameters to use for processing
    func updateParameters(params: ProcessorParams) {
        drive = params.drive
        character = params.character
        saturation = params.saturation
        presence = params.presence
        warmth = Float(params.warmth)
        outputGain = params.output
        mix = params.mix
        mode = params.mode
    }
    
    /// Set the sample rate for processing
    /// - Parameter rate: The new sample rate in Hz
    func setSampleRate(_ rate: Float) {
        sampleRate = rate
    }
    
    // MARK: - Private Methods
    
    /// Process a single sample with Anti-Derivative Anti-Aliasing
    /// - Parameter sample: Input sample
    /// - Returns: Processed sample
    private func processADAA(sample: Float) -> Float {
        // Scale input based on drive parameter
        let driveAmount = 0.5 + drive * 4.0
        let input = sample * driveAmount
        
        // Select shaper type based on mode
        let shaperType = ShaperType(rawValue: mode) ?? .warm
        
        // Calculate shaper function and its anti-derivative
        let (shaperValue, integral) = computeShaperAndIntegral(input: input, type: shaperType)
        
        // Calculate ADAA value
        let adaaValue: Float
        if abs(input - prevSample) > 1e-6 {
            adaaValue = (integral - prevIntegral) / (input - prevSample)
        } else {
            adaaValue = shaperValue
        }
        
        // Update state
        prevSample = input
        prevIntegral = integral
        
        // DC blocker (high-pass filter to remove any DC offset)
        let alpha: Float = 0.995
        let output = adaaValue - dcOffset
        dcOffset = dcOffset * alpha + output * (1.0 - alpha)
        
        // Apply character as a tone control
        // (Simplified implementation - full implementation would include multi-band processing)
        let toneShaped = character > 0.5 ? 
            output * (1.0 + (character - 0.5) * 0.4) :  // Brighter
            output * (1.0 - (0.5 - character) * 0.3)    // Darker
        
        return toneShaped
    }
    
    /// Compute the shaper function and its anti-derivative for ADAA processing
    /// - Parameters:
    ///   - input: Input sample
    ///   - type: Type of shaping to apply
    /// - Returns: Tuple of (shaper output, integral)
    private func computeShaperAndIntegral(input: Float, type: ShaperType) -> (Float, Float) {
        switch type {
        case .soft:
            // Soft saturation: tanh(x)
            // Integral: log(cosh(x))
            return (tanh(input), log(cosh(input)))
            
        case .warm:
            // Warm tube-like: customized asymmetric saturation
            // Simplified version for demonstration
            let saturationAmount = 1.0 + saturation * 4.0
            
            if input > 0 {
                // Positive side (softer)
                return (
                    tanh(input * saturationAmount) / saturationAmount,
                    log(cosh(input * saturationAmount)) / (saturationAmount * saturationAmount)
                )
            } else {
                // Negative side (harder)
                let negInput = -input
                let factor: Float = 1.2 + saturation * 0.8
                return (
                    -tanh(negInput * saturationAmount * factor) / (saturationAmount * factor),
                    log(cosh(negInput * saturationAmount * factor)) / (saturationAmount * saturationAmount * factor * factor)
                )
            }
            
        case .tape:
            // Tape saturation: soft clipping with compression
            // x / (1 + abs(x))
            let x = input * (1.0 + saturation * 3.0)
            let absX = abs(x)
            let sign: Float = x > 0 ? 1.0 : -1.0
            
            // Function: x / (1 + |x|)
            let shaperValue = x / (1.0 + absX)
            
            // Integral: sign(x) * log(1 + |x|)
            let integral = sign * log(1.0 + absX)
            
            return (shaperValue, integral)
            
        case .vintage:
            // Vintage console: asymmetric with more harmonics
            // arctan function with asymmetry
            let x = input * (1.0 + saturation * 4.0)
            let asymmetry = 0.2 + saturation * 0.3
            
            // Asymmetric function: atan(x + asymmetry * x^2)
            let xSquaredTerm = asymmetry * x * x
            // Use x > 0 ? 1 : -1 instead of sign(x)
            let shaperValue = atan(x + (x > 0 ? 1 : -1) * xSquaredTerm) / (.pi/2)
            
            // Simplified integral (approximation)
            // True integral is complex, this is a working approximation
            let integral = log(1 + x*x) / 2 + asymmetry * (x*x*x / 3)
            
            return (shaperValue, integral)
        }
    }
    
    /// Apply presence enhancement (high mid-range enhancement)
    /// - Parameter samples: Samples to modify (in-place)
    private func applyPresence(to samples: inout [Float]) {
        // Simplified presence enhancement - in reality this would be a shelf or bell filter
        // Here we'll simulate it with a basic gain boost in the 2-5kHz range
        
        let count = samples.count
        guard count > 4 else { return }
        
        // Very simple presence effect - just a primitive high shelf
        // This is just for demonstration - real implementation would use a proper filter
        let presenceAmount = presence * 0.3
        
        var highFreq = [Float](repeating: 0, count: count)
        
        // Super simple highpass (just the difference)
        for i in 1..<count {
            highFreq[i] = samples[i] - samples[i-1]
        }
        
        // Add the high frequencies back in with boosted gain
        for i in 0..<count {
            samples[i] += highFreq[i] * presenceAmount
        }
    }
    
    /// Apply warmth enhancement (low end enhancement)
    /// - Parameter samples: Samples to modify (in-place)
    private func applyWarmth(to samples: inout [Float]) {
        // Simplified warmth - in reality this would be a proper low shelf filter
        // Here we simulate it with a simple lowpass filter and boost
        
        let count = samples.count
        guard count > 4 else { return }
        
        // Very simple lowpass filter
        var lowFreq = [Float](repeating: 0, count: count)
        lowFreq[0] = samples[0]
        
        // Simple one-pole lowpass
        let alpha: Float = 0.2
        for i in 1..<count {
            lowFreq[i] = alpha * samples[i] + (1 - alpha) * lowFreq[i-1]
        }
        
        // Add the low frequencies back in with boosted gain
        let warmthAmount = warmth * 0.4
        for i in 0..<count {
            samples[i] += lowFreq[i] * warmthAmount
        }
    }
}
