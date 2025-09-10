import SwiftUI
import Foundation

// Quality mode enum (unique to this file)
public enum QualityMode: String, Codable { case standard, high, extreme }

// Enhanced extensions for InterpMode (using the core type from SharedTypes.swift)
extension InterpMode {
    public var longDisplayName: String {
        switch self {
        case .liveHB4x: return "Live HB 4×"
        case .hqSinc8x: return "HQ Sinc 8×"
        case .transientSpline4x: return "Transient Spline 4×"
        case .adaptive: return "Adaptive"
        case .aiAnalogHook: return "Live+AI"
        }
    }
}

// Enums and extensions for ProcessorParams
extension ProcessorParams {
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
    
    // Additional properties extension
    public var warmth: Double {
        get { Double(presence) * 1.2 }
        set { presence = Float(newValue / 1.2) }
    }
    
    public var tone: Double {
        get { Double(character) }
        set { character = Float(newValue) }
    }
    
    public var quality: QualityMode { 
        get { interpMode == .hqSinc8x ? .high : .standard }
        set {
            switch newValue {
            case .high, .extreme: interpMode = .hqSinc8x
            default: interpMode = .liveHB4x
            }
        }
    }
    
    // Switch parameters as computed properties
    public var aiEnhance: Bool {
        get { interpMode == .aiAnalogHook }
        set { if newValue { interpMode = .aiAnalogHook } }
    }
    
    public var hqMode: Bool {
        get { interpMode == .hqSinc8x }
        set { if newValue { interpMode = .hqSinc8x } }
    }
    
    public var analogMode: Bool {
        get { mode == 1 }
        set { mode = newValue ? 1 : 0 }
    }
    
    // Helper to get preset for specific mojo level and audio type
    public static func presetFor(level: MojoLevel, type: AudioType) -> ProcessorParams {
        var params = ProcessorParams()
        
        // Adjust parameters based on level and type
        switch (level, type) {
        case (.mojo, .drums):
            params.drive = 0.6
            params.character = 0.7
            params.presence = 0.4 / 1.2 // warmth = 0.4
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
            params.presence = 0.7 / 1.2 // warmth = 0.7
        case (.moreMojo, .vocals):
            params.drive = 0.5
            params.presence = 0.7
            params.saturation = 0.3
        case (.mostMojo, .vocals):
            params.drive = 0.5
            params.presence = 0.8
            params.character = 0.7
            params.interpMode = .hqSinc8x // hqMode = true
            
        case (.mojo, .mix):
            params.drive = 0.4
            params.mix = 0.8
            params.presence = 0.7 / 1.2 // warmth = 0.7
        case (.mostMojo, .mix):
            params.drive = 0.9
            params.output = 0.6
            params.interpMode = .hqSinc8x // hqMode = true
            
        default:
            // Use default values for other combinations
            break
        }
        
        return params
    }
}
