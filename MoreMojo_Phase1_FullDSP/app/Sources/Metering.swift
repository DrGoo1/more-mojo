
import Accelerate
import AVFoundation

public struct MeterFrame { public var rms: Float; public var peak: Float; public var spectrum: [Float] }

public final class Metering {
    private let N: Int
    private let log2n: vDSP_Length
    private var fft: vDSP.FFT<DSPSplitComplex>!
    private var window: [Float]
    private var realp: [Float]
    private var imagp: [Float]
    private var magsq: [Float]
    private var current: MeterFrame

    public init(fftSize: Int) {
        N = fftSize; log2n = vDSP_Length(log2(Double(fftSize)))
        fft = vDSP.FFT(log2n: log2n, radix: .radix2, ofType: DSPSplitComplex.self)
        window = vDSP.window(ofType: Float.self, usingSequence: .hanningDenormalized, count: fftSize, isHalfWindow: false)
        realp = .init(repeating: 0, count: fftSize/2)
        imagp = .init(repeating: 0, count: fftSize/2)
        magsq = .init(repeating: 0, count: fftSize/2)
        current = .init(rms: 0, peak: 0, spectrum: Array(repeating: 0, count: fftSize/2))
    }

    public func analyze(_ buf: AVAudioPCMBuffer) {
        guard let ch = buf.floatChannelData?.pointee else { return }
        let n = Int(buf.frameLength); if n < N { return }
        let x = ch.advanced(by: n - N)
        var rms: Float = 0; vDSP_rmsqv(x, 1, &rms, vDSP_Length(N))
        var peak: Float = 0; vDSP_maxmgv(x, 1, &peak, vDSP_Length(N))

        var w = [Float](repeating: 0, count: N)
        vDSP_vmul(x, 1, window, 1, &w, 1, vDSP_Length(N))

        var split = DSPSplitComplex(realp: &realp, imagp: &imagp)
        w.withUnsafeMutableBufferPointer { wb in
            wb.baseAddress!.withMemoryRebound(to: DSPComplex.self, capacity: N) { ptr in
                vDSP.convert(interleavedComplexVector: ptr, toSplitComplexVector: &split, usingStrategy: .halfComplex)
            }
        }
        fft.forward(input: split, output: &split)
        vDSP.squareMagnitudes(split, result: &magsq)

        var db = [Float](repeating: 0, count: N/2); var one: Float = 1
        vDSP_vdbcon(&magsq, 1, &one, &db, 1, vDSP_Length(N/2), 0)
        let minDb: Float = -90, maxDb: Float = -5
        var norm = [Float](repeating: 0, count: N/2)
        vDSP_vsmsa(db, 1, [1/(maxDb - minDb)], [-minDb/(maxDb - minDb)], &norm, 1, vDSP_Length(N/2))
        vDSP_vclip(norm, 1, [0], [1], &norm, 1, vDSP_Length(N/2))

        let a: Float = 0.6
        var spec = current.spectrum
        vDSP_vsmsa(spec, 1, [a], [0], &spec, 1, vDSP_Length(N/2))
        vDSP_vsmsa(norm, 1, [1 - a], [0], &norm, 1, vDSP_Length(N/2))
        vDSP_vadd(spec, 1, norm, 1, &spec, 1, vDSP_Length(N/2))

        current = .init(rms: a*current.rms + (1-a)*min(1, rms*3),
                        peak: max(a*current.peak, peak),
                        spectrum: spec)
    }
    public func snapshot() -> MeterFrame { current }
}
