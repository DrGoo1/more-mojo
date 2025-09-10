import Foundation

// Single source of truth for core types
public enum InterpMode: Int, Codable, CaseIterable, Identifiable {
    case liveHB4x = 0, hqSinc8x, transientSpline4x, adaptive, aiAnalogHook
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

public struct ProcessorParams: Codable, Equatable {
    public var input:  Float = 0.0   // dB
    public var output: Float = 0.0   // dB
    public var drive:      Float = 0.55
    public var character:  Float = 0.50
    public var saturation: Float = 0.45
    public var presence:   Float = 0.50
    public var mix:        Float = 1.00
    public var interpMode: InterpMode = .liveHB4x
    public var mode:       Int = 1
}

public struct MojoEQBand: Codable, Equatable {
    public var lo: Float
    public var hi: Float
    public var gain_dB: Float
}

public struct MojoEQMatch: Codable, Equatable {
    public var bands: [MojoEQBand]
}
