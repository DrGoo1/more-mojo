
import Foundation
import Accelerate
import AVFoundation

// MARK: - Swift-only HPSS (vDSP) + features + recommendation + EQ match

public enum SwiftMojoAnalyzer {
struct MojoFingerprint: Codable {
    struct Features: Codable {
        var crest_dB: Float
        var rms: Float
        var band_low: Float
        var band_mid: Float
        var band_high: Float
        var zcr_mean: Float
        var centroid_mean: Float
        var flat_mean: Float
        var flux_mean: Float
    }
    var source: String
    var part: String
    var features: Features
    var recommendation: MojoRecommendation
    var eq_match: MojoEQMatch?
}

enum SwiftMojoAnalyzer {

    // Simple STFT-based HPSS (median filter masks); mono only for analysis
    static func separateHPSS(url: URL, srOut: Double = 48000) throws -> (harm: [Float], perc: [Float], sr: Double) {
        let file = try AVAudioFile(forReading: url)
        let fmt = AVAudioFormat(standardFormatWithSampleRate: srOut, channels: 1)!
        let conv = AVAudioConverter(from: file.processingFormat, to: fmt)!
        let out = AVAudioPCMBuffer(pcmFormat: fmt, frameCapacity: AVAudioFrameCount(file.length))!
        var error: NSError? = nil
        conv.convert(to: out, error: &error) { inPackets, outStatus in
            do {
                let cap: AVAudioFrameCount = 4096
                let buf = AVAudioPCMBuffer(pcmFormat: file.processingFormat, frameCapacity: cap)!
                try file.read(into: buf, frameCount: cap)
                if buf.frameLength == 0 { outStatus.pointee = .endOfStream; return nil }
                outStatus.pointee = .haveData
                return buf
            } catch { outStatus.pointee = .endOfStream; return nil }
        }
        if let e = error { throw e }
        let n = Int(out.frameLength)
        let x = Array(UnsafeBufferPointer(start: out.floatChannelData![0], count: n))

        let nFFT = 2048, hop = 512
        let nFrames = max(1, (n - nFFT) / hop + 1)
        var win = [Float](repeating: 0, count: nFFT)
        vDSP_hann_window(&win, vDSP_Length(nFFT), Int32(vDSP_HANN_NORM))

        // Build magnitude spectrogram
        var mag = [[Float]](repeating: [Float](repeating: 0, count: nFrames), count: nFFT/2+1)
        var tempIn = [Float](repeating: 0, count: nFFT)
        var split = DSPSplitComplex(realp: .allocate(capacity: nFFT/2), imagp: .allocate(capacity: nFFT/2))
        split.realp.initialize(repeating: 0, count: nFFT/2)
        split.imagp.initialize(repeating: 0, count: nFFT/2)
        let setup = vDSP_DFT_zop_CreateSetup(nil, vDSP_Length(nFFT), .FORWARD)!
        defer {
            vDSP_DFT_DestroySetup(setup)
            split.realp.deallocate(); split.imagp.deallocate()
        }
        for t in 0..<nFrames {
            let start = t*hop
            tempIn[0..<nFFT] = Array(x[start..<min(start+nFFT, n)])
            vDSP_vmul(tempIn, 1, win, 1, &tempIn, 1, vDSP_Length(nFFT))
            tempIn.withUnsafeBufferPointer { ptr in
                ptr.baseAddress!.withMemoryRebound(to: DSPComplex.self, capacity: nFFT/2) { _ in }
            }
            // Pack real input to split complex
            tempIn.withUnsafeBufferPointer { ib in
                vDSP_ctoz(UnsafePointer<DSPComplex>(OpaquePointer(ib.baseAddress!)), 2, &split, 1, vDSP_Length(nFFT/2))
            }
            vDSP_DFT_Execute(setup, split.realp, split.imagp, split.realp, split.imagp)
            for k in 0...nFFT/2 {
                let re = split.realp[k], im = split.imagp[k]
                mag[k][t] = sqrtf(re*re + im*im) + 1e-9
            }
        }
        // Median filters along time (harmonic) and freq (percussive)
        func medfilt1(_ a: [Float], _ w: Int) -> [Float] {
            let n = a.count, w2 = w/2
            var out = [Float](repeating: 0, count: n)
            var window = [Float](repeating: 0, count: w)
            for i in 0..<n {
                let s = max(0, i-w2), e = min(n-1, i+w2)
                let len = e - s + 1
                window[0..<len] = a[s...e]
                out[i] = window[0..<len].sorted()[len/2]
            }
            return out
        }
        let wt = 17, wf = 17
        var Hmag = mag // harmonic likelihood
        var Pmag = mag // percussive likelihood
        for k in 0...nFFT/2 {
            Hmag[k] = medfilt1(mag[k], wt)
        }
        for t in 0..<nFrames {
            var col = [Float](repeating: 0, count: nFFT/2+1)
            for k in 0...nFFT/2 { col[k] = mag[k][t] }
            let m = medfilt1(col, wf)
            for k in 0...nFFT/2 { Pmag[k][t] = m[k] }
        }
        // Soft masks
        var Mh = mag, Mp = mag
        for k in 0...nFFT/2 {
            for t in 0..<nFrames {
                let h = Hmag[k][t], p = Pmag[k][t]
                let denom = h + p + 1e-9
                Mh[k][t] = h / denom
                Mp[k][t] = p / denom
            }
        }
        // Reconstruct with overlap-add
        func istft(masked: [[Float]]) -> [Float] {
            var y = [Float](repeating: 0, count: nFrames*hop + nFFT)
            let setupI = vDSP_DFT_zop_CreateSetup(nil, vDSP_Length(nFFT), .INVERSE)!
            var re = [Float](repeating: 0, count: nFFT/2), im = [Float](repeating: 0, count: nFFT/2)
            var outSplit = DSPSplitComplex(realp: &re, imagp: &im)
            var tempOut = [Float](repeating: 0, count: nFFT)
            for t in 0..<nFrames {
                for k in 0..<nFFT/2 {
                    let phase: Float = 0 // discard phase; acceptable for analysis
                    outSplit.realp[k] = masked[k][t] * cosf(phase)
                    outSplit.imagp[k] = masked[k][t] * sinf(phase)
                }
                vDSP_DFT_Execute(setupI, outSplit.realp, outSplit.imagp, outSplit.realp, outSplit.imagp)
                // back to interleaved
                var cplx = [DSPComplex](repeating: DSPComplex(), count: nFFT/2)
                vDSP_ztoc(&outSplit, 1, &cplx, 2, vDSP_Length(nFFT/2))
                // sum real parts (rough)
                for i in 0..<nFFT/2 {
                    tempOut[i] = outSplit.realp[i]
                }
                // overlap-add
                let start = t*hop
                for i in 0..<nFFT {
                    if start+i < y.count { y[start+i] += tempOut[i] }
                }
            }
            return y
        }
        let harm = istft(masked: Mh)
        let perc = istft(masked: Mp)
        return (harm, perc, srOut)
    }

    static func features(from y: [Float], sr: Double) -> MojoFingerprint.Features {
        let n = y.count
        let peak = y.withUnsafeBufferPointer { vDSP.maximumMagnitude($0.baseAddress!, count: n) }
        var rms: Float = 0; vDSP_rmsqv(y, 1, &rms, vDSP_Length(n))
        let crest = 20*log10f(max(1e-9, peak / max(1e-9, rms)))

        // zcr
        var zc: Float = 0
        for i in 1..<n { if (y[i-1] <= 0 && y[i] > 0) || (y[i-1] >= 0 && y[i] < 0) { zc += 1 } }
        let zcr = zc / Float(n)

        // crude spectral bands using vDSP FFT (single frame)
        let nFFT = 4096
        let M = nFFT/2
        var buf = [Float](repeating: 0, count: nFFT)
        let copyN = min(n, nFFT); for i in 0..<copyN { buf[i] = y[i] }
        var win = [Float](repeating: 0, count: nFFT)
        vDSP_hann_window(&win, vDSP_Length(nFFT), Int32(vDSP_HANN_NORM))
        vDSP_vmul(buf, 1, win, 1, &buf, 1, vDSP_Length(nFFT))
        var re = [Float](repeating: 0, count: M), im = [Float](repeating: 0, count: M)
        var split = DSPSplitComplex(realp: &re, imagp: &im)
        let setup = vDSP_DFT_zop_CreateSetup(nil, vDSP_Length(nFFT), .FORWARD)!
        defer { vDSP_DFT_DestroySetup(setup) }
        vDSP_ctoz(UnsafePointer<DSPComplex>(OpaquePointer(buf)), 2, &split, 1, vDSP_Length(M))
        vDSP_DFT_Execute(setup, split.realp, split.imagp, split.realp, split.imagp)
        var mag = [Float](repeating: 0, count: M)
        vDSP_zvabs(&split, 1, &mag, 1, vDSP_Length(M))
        // normalize
        var maxm: Float = 0; vDSP_maxv(mag, 1, &maxm, vDSP_Length(M)); if maxm > 0 { vDSP_vsdiv(mag, 1, &maxm, &mag, 1, vDSP_Length(M)) }
        func band(lo: Float, hi: Float) -> Float {
            let df = Float(sr)/Float(nFFT)
            let i0 = Int(lo/df), i1 = min(M-1, Int(hi/df))
            if i1 <= i0 { return 0 }
            var mean: Float = 0; vDSP_meamgv(Array(mag[i0...i1]), 1, &mean, vDSP_Length(i1-i0+1))
            return mean
        }
        let low = band(lo: 20, hi: 200)
        let mid = band(lo: 200, hi: 2000)
        let high = band(lo: 4000, hi: 12000)
        // centroid (rough)
        var idx = [Float](0..<Float(M))
        var num: Float = 0, den: Float = 1e-9
        vDSP_dotpr(idx, 1, mag, 1, &num, vDSP_Length(M))
        vDSP_sve(mag, 1, &den, vDSP_Length(M))
        let centroid = num / den * Float(sr)/Float(nFFT)

        // flatness (geometric mean / arithmetic mean)
        var amean: Float = 0; vDSP_meamgv(mag, 1, &amean, vDSP_Length(M))
        var gmean: Float = 0; vDSP_geoMean(mag, result: &gmean)
        let flat = gmean / max(1e-9, amean)

        // flux: mean positive diff
        var flux: Float = 0
        for i in 1..<M { let d = mag[i] - mag[i-1]; if d > 0 { flux += d } }
        flux /= Float(M)

        return .init(crest_dB: crest, rms: rms, band_low: low, band_mid: mid, band_high: high,
                     zcr_mean: zcr, centroid_mean: centroid, flat_mean: flat, flux_mean: flux)
    }

    static func recommend(from f: MojoFingerprint.Features, part: String) -> MojoRecommendation {
        func norm(_ x: Float, _ a: Float, _ b: Float) -> Float { max(0, min(1, (x - a) / (b - a + 1e-9))) }
        var drive = norm(f.flat_mean, 0.05, 0.25) * 0.8 + norm(f.crest_dB, 6, 18) * 0.2
        var character = norm(f.band_low/(f.band_high+1e-9), 0.5, 4.0)
        var saturation = norm(f.flux_mean, 0.0, 0.1) * 0.6 + 0.2
        var presence = norm(f.centroid_mean, 1000, 6000)

        let p = part.lowercased()
        if p.contains("bass") { character = max(character, 0.6); presence = min(presence, 0.4) }
        else if p.contains("vocal") { presence = max(presence, 0.6); saturation = min(saturation, 0.6) }
        else if p.contains("drum") { drive = max(drive, 0.5); saturation = max(saturation, 0.6) }

        return .init(interpMode: "adaptive", drive: drive, saturation: saturation,
                     character: character, presence: presence, mix: 1.0, output: 0.0)
    }

    static func eqMatchBands(srcURL: URL, refURL: URL, bands: Int = 8) -> MojoEQMatch? {
        // Very simple: compute average log-magnitude spectra and subtract in banded fashion
        do {
            func logMag(_ url: URL) throws -> (freq: [Float], mag: [Float], sr: Double) {
                let f = try AVAudioFile(forReading: url)
                let fmt = AVAudioFormat(standardFormatWithSampleRate: 48000, channels: 1)!
                let conv = AVAudioConverter(from: f.processingFormat, to: fmt)!
                let out = AVAudioPCMBuffer(pcmFormat: fmt, frameCapacity: AVAudioFrameCount(f.length))!
                var error: NSError? = nil
                conv.convert(to: out, error: &error) { inPackets, outStatus in
                    do {
                        let cap: AVAudioFrameCount = 4096
                        let buf = AVAudioPCMBuffer(pcmFormat: f.processingFormat, frameCapacity: cap)!
                        try f.read(into: buf, frameCount: cap)
                        if buf.frameLength == 0 { outStatus.pointee = .endOfStream; return nil }
                        outStatus.pointee = .haveData; return buf
                    } catch { outStatus.pointee = .endOfStream; return nil }
                }
                if let e = error { throw e }
                let sr = fmt.sampleRate
                let n = Int(out.frameLength); let nFFT = 4096
                var x = Array(UnsafeBufferPointer(start: out.floatChannelData![0], count: n))
                x += Array(repeating: 0, count: max(0, nFFT - n))
                var win = [Float](repeating: 0, count: nFFT); vDSP_hann_window(&win, vDSP_Length(nFFT), Int32(vDSP_HANN_NORM))
                vDSP_vmul(x, 1, win, 1, &x, 1, vDSP_Length(nFFT))
                var re = [Float](repeating: 0, count: nFFT/2), im = re
                var split = DSPSplitComplex(realp: &re, imagp: &im)
                let setup = vDSP_DFT_zop_CreateSetup(nil, vDSP_Length(nFFT), .FORWARD)!
                defer { vDSP_DFT_DestroySetup(setup) }
                vDSP_ctoz(UnsafePointer<DSPComplex>(OpaquePointer(x)), 2, &split, 1, vDSP_Length(nFFT/2))
                vDSP_DFT_Execute(setup, split.realp, split.imagp, split.realp, split.imagp)
                var mag = [Float](repeating: 0, count: nFFT/2)
                vDSP_zvabs(&split, 1, &mag, 1, vDSP_Length(nFFT/2))
                var one: Float = 1e-9; vDSP_vsadd(mag, 1, &one, &mag, 1, vDSP_Length(nFFT/2))
                var logmag = [Float](repeating: 0, count: nFFT/2)
                vDSP_vdbcon(mag, 1, &one, &logmag, 1, vDSP_Length(nFFT/2), 0)
                let df = Float(sr)/Float(nFFT)
                return ((0..<nFFT/2).map{ Float($0)*df }, logmag, sr)
            }
            let s = try logMag(srcURL), r = try logMag(refURL)
            let maxF = min(s.freq.last ?? 20000, r.freq.last ?? 20000)
            let fGrid = stride(from: Float(20), to: Float(maxF), by: Float(maxF)/512).map{ $0 }
            func interp(_ f: [Float], _ y: [Float], at grid: [Float]) -> [Float] {
                var out = [Float](); out.reserveCapacity(grid.count)
                for g in grid {
                    if g <= f.first! { out.append(y.first!) }
                    else if g >= f.last! { out.append(y.last!) }
                    else {
                        let i = f.firstIndex(where: { $0 >= g })!
                        let i0 = max(0, i-1); let i1 = i
                        let t = (g - f[i0]) / max(1e-6, f[i1] - f[i0])
                        out.append(y[i0]*(1-t) + y[i1]*t)
                    }
                }
                return out
            }
            let Ms = interp(s.freq, s.mag, at: fGrid)
            let Mr = interp(r.freq, r.mag, at: fGrid)
            let diff = zip(Mr, Ms).map { $0 - $1 } // dB to boost src to match ref
            // compress to bands (explicit element type + explicit initializer)
            var bands: [MojoEQBand] = []
            let N = 8
            let e = (0...N).map { i in Int(Float(i)/Float(N) * Float(fGrid.count-1)) }
            for i in 0..<N {
                let i0 = e[i], i1 = e[i+1]
                if i1 <= i0 { continue }
                let gains = diff[i0...i1]
                let midGain = gains.sorted()[gains.count/2]
                bands.append(MojoEQBand(lo: fGrid[i0], hi: fGrid[i1], gain_dB: midGain))
            }
            return MojoEQMatch(bands: bands)
        } catch { return nil }
    }
}

fileprivate extension Array {
    func clampedCount(_ n: Int) -> Int { Swift.max(1, Swift.min(self.count, n)) }
}
