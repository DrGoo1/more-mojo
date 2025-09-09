import SwiftUI
import Foundation

public enum QualityMode: String, Codable { case standard, high, extreme }

public enum InterpMode: Int, Codable, CaseIterable, Identifiable {
    case liveHB4x = 0, hqSinc8x, transientSpline4x, adaptive, aiAnalogHook
    public var id: Int { rawValue }
    public var displayName: String {
        switch self {
        case .liveHB4x: return "Live HB 4×"
        case .hqSinc8x: return "HQ Sinc 8×"
        case .transientSpline4x: return "Transient Spline 4×"
        case .adaptive: return "Adaptive"
        case .aiAnalogHook: return "Live+AI"
        }
    }
}

public struct ProcessorParams: Codable, Equatable {
    // Audio enhancement levels
    public enum MojoLevel: String, CaseIterable, Codable {
        case mojo = "Mojo"
        case moreMojo = "More Mojo"
        case mostMojo = "Most Mojo"
    }
    
    // Audio type categories
    public enum AudioType: String, CaseIterable, Codable {
        case drums = "Drums"
        case bass = "Bass"
        case guitar = "Guitar"
        case vocals = "Vocals"
        case mix = "Full Mix"
    }
    
    // Main parameters
    public var mojoLevel: MojoLevel = .moreMojo
    public var audioType: AudioType = .mix
    
    // Processor parameters
    public var drive: Float = 0.55
    public var character: Float = 0.50
    public var saturation: Float = 0.45
    public var presence: Float = 0.50
    public var warmth: Double = 0.6
    public var tone: Double = 0.5
    public var mix: Float = 1.00
    public var output: Float = 0.00
    public var mode: Int = 1
    public var quality: QualityMode = .standard
    public var interpMode: InterpMode = .liveHB4x
    
    // Switch parameters
    public var aiEnhance: Bool = true
    public var hqMode: Bool = false
    public var analogMode: Bool = true
    
    // Initializer
    public init() {}
    
    // Helper to get preset for specific mojo level and audio type
    public static func presetFor(level: MojoLevel, type: AudioType) -> ProcessorParams {
        var params = ProcessorParams()
        params.mojoLevel = level
        params.audioType = type
        
        // Adjust parameters based on level and type
        switch (level, type) {
        case (.mojo, .drums):
            params.drive = 0.6
            params.character = 0.7
            params.warmth = 0.4
        case (.moreMojo, .drums):
            params.drive = 0.8
            params.character = 0.6
            params.saturation = 0.7
        case (.mostMojo, .drums):
            params.drive = 0.9
            params.saturation = 0.8
            params.output = 0.1
            
        case (.mojo, .vocals):
            params.drive = 0.3
            params.presence = 0.6
            params.warmth = 0.7
        case (.moreMojo, .vocals):
            params.drive = 0.5
            params.presence = 0.7
            params.saturation = 0.3
        case (.mostMojo, .vocals):
            params.drive = 0.5
            params.presence = 0.8
            params.character = 0.7
            params.hqMode = true
            
        case (.mojo, .mix):
            params.drive = 0.4
            params.mix = 0.8
            params.warmth = 0.7
        case (.mostMojo, .mix):
            params.drive = 0.9
            params.output = 0.6
            params.hqMode = true
            
        default:
            // Use default values for other combinations
            break
        }
        
        return params
    }
}
