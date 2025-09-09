
import Accelerate
import AVFoundation

public struct IntegrityMetrics {
    public var tpHeadroom_dB: Float = 0
    public var crestIn_dB: Float = 0
    public var crestOut_dB: Float = 0
    public var crestDelta_dB: Float { crestOut_dB - crestIn_dB }
    public var transientPercent: Float = 1
    public var aliasRisk: Float = 0
}

public final class IntegrityCalculator {
    private var lastMag: [Float] = []
    private var stRMSIn: Float = 0, stPeakIn: Float = 0
    private var stRMSOut: Float = 0, stPeakOut: Float = 0
    private let alpha: Float = 0.2
    public init() {}

    public func update(output: AVAudioPCMBuffer, meter: MeterFrame) -> IntegrityMetrics {
        stRMSIn  = 0.9 * stRMSIn  + 0.1 * meter.rms
        stPeakIn = max(0.98 * stPeakIn, meter.peak * 0.95)

        let n = Int(output.frameLength)
        guard let outCh = output.floatChannelData?.pointee, n > 0 else { return IntegrityMetrics() }
        var rmsOut: Float = 0; vDSP_rmsqv(outCh, 1, &rmsOut, vDSP_Length(n))
        let peakOut = vDSP.maximumMagnitude(outCh, count: n)
        stRMSOut = alpha*rmsOut + (1-alpha)*stRMSOut
        stPeakOut = max(alpha*peakOut + (1-alpha)*stPeakOut, stPeakOut*0.98)

        let tp = truePeakApprox(outCh, n)
        let head = -20 * log10(max(tp, 1e-9))
        let cfIn  = crest(peak: stPeakIn,  rms: stRMSIn)
        let cfOut = crest(peak: stPeakOut, rms: stRMSOut)

        var trans: Float = 1
        if !lastMag.isEmpty && lastMag.count == meter.spectrum.count {
            var diff = [Float](repeating: 0, count: lastMag.count)
            vDSP_vsub(lastMag, 1, meter.spectrum, 1, &diff, 1, vDSP_Length(diff.count))
            vDSP_vthres(diff, 1, [0], &diff, 1, vDSP_Length(diff.count))
            let flux = vDSP.sum(diff)
            trans = max(0, min(1, 1 - flux*0.002))
        }
        lastMag = meter.spectrum

        return IntegrityMetrics(tpHeadroom_dB: head, crestIn_dB: cfIn, crestOut_dB: cfOut, transientPercent: trans, aliasRisk: 0)
    }

    private func crest(peak: Float, rms: Float) -> Float {
        guard rms > 1e-9 else { return 0 }
        return 20 * log10(max(peak, 1e-9) / rms)
    }
    private func truePeakApprox(_ x: UnsafePointer<Float>, _ n: Int) -> Float {
        if n < 4 { return vDSP.maximumMagnitude(x, count: n) }
        var tp: Float = 0
        for i in 1..<(n-2) {
            let xm1 = x[i-1], x0 = x[i], x1 = x[i+1], x2 = x[i+2]
            let c0 = x0
            let c1 = 0.5 * (x1 - xm1)
            let c2 = xm1 - 2.5*x0 + 2*x1 - 0.5*x2
            let c3 = 0.5*(x2 - xm1) + 1.5*(x0 - x1)
            for t in [0.25, 0.5, 0.75] as [Float] {
                let y = ((t*t*t)*c3 + (t*t)*c2 + t*c1 + c0)
                tp = max(tp, fabsf(y))
            }
        }
        return tp
    }
}
