import Foundation
public enum InterpMode: Int, Codable, CaseIterable, Identifiable {
    case liveHB4x = 0, hqSinc8x, transientSpline4x, adaptive, aiAnalogHook
    public var id: Int { rawValue }
}
public struct ProcessorParams: Codable, Equatable {
    public var input: Float = 0.0
    public var output: Float = 0.0
    public var drive: Float = 0.55
    public var character: Float = 0.50
    public var saturation: Float = 0.45
    public var presence: Float = 0.50
    public var mix: Float = 1.00
    public var interpMode: InterpMode = .liveHB4x
    public var mode: Int = 1
}
public struct MojoEQBand: Codable, Equatable { public var lo: Float; public var hi: Float; public var gain_dB: Float }
public struct MojoEQMatch: Codable, Equatable { public var bands: [MojoEQBand] }
