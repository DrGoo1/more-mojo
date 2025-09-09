
import Accelerate

public final class HB4x {
    private let taps: [Float] = [
        -0.000321, 0.0, 0.001941, 0.0, -0.006975, 0.0, 0.018676, 0.0, -0.042026, 0.0,
         0.082318, 0.0,-0.167023, 0.0,  0.500000, 0.0, -0.167023, 0.0,  0.082318, 0.0,
        -0.042026, 0.0, 0.018676, 0.0, -0.006975, 0.0, 0.001941, 0.0, -0.000321
    ]
    public init() {}
    public func up(_ L: UnsafePointer<Float>, _ R: UnsafePointer<Float>, _ n: Int, _ outL: inout [Float], _ outR: inout [Float]) {
        let upN = n * 4
        outL = [Float](repeating: 0, count: upN + taps.count - 1)
        outR = [Float](repeating: 0, count: upN + taps.count - 1)
        var idx = 0
        for i in 0..<n { outL[idx] = L[i]; outR[idx] = R[i]; idx += 4 }
        var T = taps
        vDSP_conv(outL, 1, &T, 1, &outL, 1, vDSP_Length(upN), vDSP_Length(taps.count))
        vDSP_conv(outR, 1, &T, 1, &outR, 1, vDSP_Length(upN), vDSP_Length(taps.count))
        outL.removeLast(taps.count - 1)
        outR.removeLast(taps.count - 1)
    }
    public func down(_ L: UnsafePointer<Float>, _ R: UnsafePointer<Float>, _ upN: Int, _ outL: inout [Float], _ outR: inout [Float]) {
        var fl = [Float](repeating: 0, count: upN + taps.count - 1)
        var fr = [Float](repeating: 0, count: upN + taps.count - 1)
        var T = taps
        vDSP_conv(L, 1, &T, 1, &fl, 1, vDSP_Length(upN), vDSP_Length(taps.count))
        vDSP_conv(R, 1, &T, 1, &fr, 1, vDSP_Length(upN), vDSP_Length(taps.count))
        let n = upN / 4
        outL = [Float](repeating: 0, count: n)
        outR = [Float](repeating: 0, count: n)
        var j = 0; for i in stride(from: 0, to: upN, by: 4) { outL[j] = fl[i]; j += 1 }
        j = 0; for i in stride(from: 0, to: upN, by: 4) { outR[j] = fr[i]; j += 1 }
    }
}

fileprivate struct FIRDesigner {
    static func lowpass(length: Int, cutoff: Double, beta: Double = 8.6) -> [Float] {
        precondition(length % 2 == 1)
        let M = (length - 1) / 2
        var taps = [Double](repeating: 0, count: length)
        for n in -M...M {
            let i = n + M, x = Double(n)
            taps[i] = (n == 0) ? 2.0 * cutoff : sin(2.0 * .pi * cutoff * x) / (.pi * x)
            let r = Double(n)/Double(M)
            taps[i] *= besselI0(beta * sqrt(1 - r*r)) / besselI0(beta)
        }
        let s = taps.reduce(0,+)
        return taps.map { Float($0 / s) }
    }
    static func besselI0(_ x: Double) -> Double {
        let y = x/2; var sum = 1.0, t = 1.0
        for k in 1...20 { t *= (y*y)/Double(k*k); sum += t; if t < 1e-12 { break } }
        return sum
    }
}

public final class Sinc8x {
    private let taps: [Float]
    public init(length: Int = 257) {
        let cutoff = 0.5 / 8.0 * 0.95
        taps = FIRDesigner.lowpass(length: length, cutoff: cutoff)
    }
    public func up(_ L: UnsafePointer<Float>, _ R: UnsafePointer<Float>, _ n: Int, _ outL: inout [Float], _ outR: inout [Float]) {
        let upN = n * 8
        outL = [Float](repeating: 0, count: upN + taps.count - 1)
        outR = [Float](repeating: 0, count: upN + taps.count - 1)
        var idx = 0
        for i in 0..<n { outL[idx] = L[i]; outR[idx] = R[i]; idx += 8 }
        var T = taps
        vDSP_conv(outL, 1, &T, 1, &outL, 1, vDSP_Length(upN), vDSP_Length(taps.count))
        vDSP_conv(outR, 1, &T, 1, &outR, 1, vDSP_Length(upN), vDSP_Length(taps.count))
        outL.removeLast(taps.count - 1)
        outR.removeLast(taps.count - 1)
    }
    public func down(_ L: UnsafePointer<Float>, _ R: UnsafePointer<Float>, _ upN: Int, _ outL: inout [Float], _ outR: inout [Float]) {
        var fl = [Float](repeating: 0, count: upN + taps.count - 1)
        var fr = [Float](repeating: 0, count: upN + taps.count - 1)
        var T = taps
        vDSP_conv(L, 1, &T, 1, &fl, 1, vDSP_Length(upN), vDSP_Length(taps.count))
        vDSP_conv(R, 1, &T, 1, &fr, 1, vDSP_Length(upN), vDSP_Length(taps.count))
        let n = upN / 8
        outL = [Float](repeating: 0, count: n)
        outR = [Float](repeating: 0, count: n)
        var j = 0; for i in stride(from: 0, to: upN, by: 8) { outL[j] = fl[i]; j += 1 }
        j = 0; for i in stride(from: 0, to: upN, by: 8) { outR[j] = fr[i]; j += 1 }
    }
}

public final class TransientSpline4x {
    public init() {}
    func hermite(_ p0: Float, _ p1: Float, _ m0: Float, _ m1: Float, _ t: Float) -> Float {
        let t2 = t*t, t3 = t2*t
        return (2*t3 - 3*t2 + 1)*p0 + (t3 - 2*t2 + t)*m0 + (-2*t3 + 3*t2)*p1 + (t3 - t2)*m1
    }
    public func process(_ L: UnsafePointer<Float>, _ R: UnsafePointer<Float>, _ n: Int, _ outL: inout [Float], _ outR: inout [Float]) {
        let upN = n * 4
        outL = [Float](repeating: 0, count: upN); outR = outL
        var mL = [Float](repeating: 0, count: n), mR = mL
        mL[0]=L[1]-L[0]; mR[0]=R[1]-R[0]
        for i in 1..<n-1 { mL[i]=0.5*(L[i+1]-L[i-1]); mR[i]=0.5*(R[i+1]-R[i-1]) }
        mL[n-1]=L[n-1]-L[n-2]; mR[n-1]=R[n-1]-R[n-2]
        var k=0
        for i in 0..<n-1 {
            let p0L=L[i], p1L=L[i+1], m0L=mL[i], m1L=mL[i+1]
            let p0R=R[i], p1R=R[i+1], m0R=mR[i], m1R=mR[i+1]
            outL[k]=p0L; outR[k]=p0R; k+=1
            outL[k]=hermite(p0L,p1L,m0L,m1L,0.25); outR[k]=hermite(p0R,p1R,m0R,m1R,0.25); k+=1
            outL[k]=hermite(p0L,p1L,m0L,m1L,0.50); outR[k]=hermite(p0R,p1R,m0R,m1R,0.50); k+=1
            outL[k]=hermite(p0L,p1L,m0L,m1L,0.75); outR[k]=hermite(p0R,p1R,m0R,m1R,0.75); k+=1
        }
        outL[k]=L[n-1]; outR[k]=R[n-1]
        let t: [Float] = [0.05, 0.2, 0.5, 0.2, 0.05]; var T = t
        vDSP_conv(outL, 1, &T, 1, &outL, 1, vDSP_Length(upN), vDSP_Length(t.count))
        vDSP_conv(outR, 1, &T, 1, &outR, 1, vDSP_Length(upN), vDSP_Length(t.count))
    }
}

public final class AdaptiveInterp {
    private let hb = HB4x()
    private var prevE: Float = 0
    public init() {}
    public func process(_ L: UnsafePointer<Float>, _ R: UnsafePointer<Float>, _ n: Int,
                        _ outL: inout [Float], _ outR: inout [Float],
                        highRate: (_ upL: inout [Float], _ upR: inout [Float], _ upN: Int) -> Void) {
        var e: Float = 0
        for i in 0..<n { let s = fabsf(L[i])+fabsf(R[i]); e = max(e*0.98, s) }
        let needOS = (e > 0.2) || (prevE > 0.2)
        prevE = e
        if needOS {
            let upN = n * 4
            var upL = [Float](repeating: 0, count: upN), upR = [Float](repeating: 0, count: upN)
            hb.up(L,R,n,&upL,&upR)
            highRate(&upL,&upR,upN)
            hb.down(upL, upR, upN, &outL, &outR)
        } else {
            outL = [Float](unsafeUninitializedCapacity: n) { buf, _ in buf.baseAddress!.assign(from: L, count: n) }
            outR = [Float](unsafeUninitializedCapacity: n) { buf, _ in buf.baseAddress!.assign(from: R, count: n) }
        }
    }
}
