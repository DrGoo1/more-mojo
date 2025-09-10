import Foundation
import AVFoundation

/// Mock implementation of the SwiftMojoAnalyzer for CI builds
enum SwiftMojoAnalyzer {
    
    struct MojoRecommendation {
        var drive: Float = 0.5
        var character: Float = 0.5
        var saturation: Float = 0.5
        var presence: Float = 0.5
        var mix: Float = 1.0
        var output: Float = 0.0
        var interpMode: String = "liveHB4x"
    }
    
    static func separateHPSS(url: URL) throws -> ([Float], [Float], Double) {
        // Mock implementation that returns empty arrays
        return ([], [], 44100.0)
    }
    
    static func features(from harmonic: [Float], sr: Double) -> [String: Double] {
        return [
            "spectralCentroid": 1200.0,
            "spectralSpread": 800.0,
            "rmsEnergy": 0.3,
            "transients": 0.2,
            "lowEndDensity": 0.4
        ]
    }
    
    static func recommend(from features: [String: Double], part: String) -> MojoRecommendation {
        var rec = MojoRecommendation()
        
        // Different presets based on instrument type
        switch part.lowercased() {
        case "bass":
            rec.drive = 0.6
            rec.character = 0.7
            rec.saturation = 0.4
            rec.presence = 0.3
            rec.interpMode = "hqSinc8x"
        case "drums":
            rec.drive = 0.5
            rec.character = 0.4
            rec.saturation = 0.6
            rec.presence = 0.7
            rec.interpMode = "transientSpline4x"
        case "vocal":
            rec.drive = 0.4
            rec.character = 0.6
            rec.saturation = 0.5
            rec.presence = 0.6
            rec.interpMode = "adaptive"
        default:
            rec.drive = 0.5
            rec.character = 0.5
            rec.saturation = 0.5
            rec.presence = 0.5
            rec.interpMode = "liveHB4x"
        }
        
        return rec
    }
    
    static func eqMatchBands(srcURL: URL, refURL: URL) -> MojoEQMatch? {
        // Mock EQ match
        return MojoEQMatch(bands: [
            MojoEQBand(freq: 100, gain: 2.0, q: 1.0),
            MojoEQBand(freq: 500, gain: -1.5, q: 1.0),
            MojoEQBand(freq: 2000, gain: 3.0, q: 1.0),
            MojoEQBand(freq: 8000, gain: -2.0, q: 1.0)
        ])
    }
}

struct MojoEQBand {
    var freq: Float
    var gain: Float
    var q: Float
}

struct MojoEQMatch {
    var bands: [MojoEQBand]
}
