import Foundation

// MARK: - Interp mode (use this consistently everywhere)
public enum InterpMode: Int, Codable, CaseIterable, Identifiable {
    case liveHB4x = 0      // half-band FIR 4x (low latency)
    case hqSinc8x          // linear-phase windowed-sinc 8x
    case transientSpline4x // Hermite 4x (transient-friendly)
    case adaptive          // 1x↔4x based on HF/transient
    case aiAnalogHook      // for future AI path (bypass if absent)
    public var id: Int { rawValue }
    public var displayName: String {
        switch self {
        case .liveHB4x: return "HB 4×"
        case .hqSinc8x: return "HQ Sinc 8×"
        case .transientSpline4x: return "Spline 4×"
        case .adaptive: return "Adaptive"
        case .aiAnalogHook: return "Live+AI"
        }
    }
}

// MARK: - Processor parameters (single source of truth)
public struct ProcessorParams: Codable, Equatable {
    // Pre-/post gain
    public var input:  Float = 0.0    // dB (-12...+12)
    public var output: Float = 0.0    // dB (-12...+12)

    // Mojo core
    public var drive:      Float = 0.55  // 0..1
    public var character:  Float = 0.50  // 0..1
    public var saturation: Float = 0.45  // 0..1
    public var presence:   Float = 0.50  // 0..1

    // Mix
    public var mix:        Float = 1.00  // 0..1

    // Mode & quality
    public var interpMode: InterpMode = .liveHB4x
    public var mode:       Int = 1       // curve family (Warm/Vintage/Tape/Tube), if you use it
}

// MARK: - EQ Match (shared)
public struct MojoEQBand: Codable, Equatable {
    public var lo: Float   // Hz
    public var hi: Float   // Hz
    public var gain_dB: Float
}
public struct MojoEQMatch: Codable, Equatable {
    public var bands: [MojoEQBand]
}
