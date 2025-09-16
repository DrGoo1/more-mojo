// ProcessorParams.swift
// Defines core types for the app

import Foundation

public // DEPRECATED duplicate removed
// enum InterpMode: Int, Codable, CaseIterable, Identifiable {
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
    public var input: Float = 0.0
    public var output: Float = 0.0
    public var drive: Float = 0.55
    public var character: Float = 0.50
    public var saturation: Float = 0.45
    public var presence: Float = 0.50
    public var warmth: Float = 0.60
    public var mix: Float = 1.00
    public var interpMode: InterpMode = .liveHB4x
    public var mode: Int = 1
    
    public init() {}
}

public struct MojoEQBand: Codable, Equatable {
    public var lo: Float
    public var hi: Float
    public var gain_dB: Float
    
    public init(lo: Float, hi: Float, gain_dB: Float) {
        self.lo = lo
        self.hi = hi
        self.gain_dB = gain_dB
    }
}

public struct MojoEQMatch: Codable, Equatable {
    public var bands: [MojoEQBand]
    
    public init(bands: [MojoEQBand]) {
        self.bands = bands
    }
}

public struct MojoRecommendation: Codable, Equatable {
    public var interpMode: String
    public var drive: Float
    public var saturation: Float
    public var character: Float
    public var presence: Float
    public var mix: Float
    public var output: Float
    
    public init(interpMode: String, drive: Float, saturation: Float, character: Float, presence: Float, mix: Float, output: Float) {
        self.interpMode = interpMode
        self.drive = drive
        self.saturation = saturation
        self.character = character
        self.presence = presence
        self.mix = mix
        self.output = output
    }
}
