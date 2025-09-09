
import Accelerate

public enum RFMode: Int, Codable { case warm=0, vintage=1, tape=2, tube=3 }

// ADAA-like shaper family
public final class RealFeelProcessorPro {
    public init() {}
    public func processStereo(_ L: UnsafeMutablePointer<Float>, _ R: UnsafeMutablePointer<Float>,
                              frames n: Int, mode: RFMode, intensity: Float, character: Float, presence: Float) {
        // Pre-emphasis (presence) at high rate
        var pre: Float = 1.0 + 0.02 * presence
        vDSP_vsmul(L, 1, &pre, L, 1, vDSP_Length(n))
        vDSP_vsmul(R, 1, &pre, R, 1, vDSP_Length(n))

        // Simple ADAA-ish soft shape
        func shape(_ x: Float, k: Float) -> Float {
            // cubic soft clip with character skew
            let s = x * (1 + k) - 0.35*character*powf(x,3) + 0.07*character*powf(x,5)
            return tanhf(s)
        }
        let k = 0.8 * intensity
        for i in 0..<n { L[i] = shape(L[i], k); R[i] = shape(R[i], k) }

        // Mode post-EQ tilt
        var tilt: Float = 1.0
        switch mode {
        case .warm: tilt = 1.00
        case .vintage: tilt = 1.02
        case .tape: tilt = 0.98
        case .tube: tilt = 1.03
        }
        vDSP_vsmul(L, 1, &tilt, L, 1, vDSP_Length(n))
        vDSP_vsmul(R, 1, &tilt, R, 1, vDSP_Length(n))

        // Trim
        var trim: Float = 0.98
        vDSP_vsmul(L, 1, &trim, L, 1, vDSP_Length(n))
        vDSP_vsmul(R, 1, &trim, R, 1, vDSP_Length(n))
    }
}
