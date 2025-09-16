import Foundation
import AVFoundation
import Accelerate

/// Mock implementation of the SwiftMojoAnalyzer for CI builds
enum SwiftMojoAnalyzer {
    
    static func separateHPSS(url: URL) throws -> ([Float], [Float], Double) {
        // Mock implementation that returns empty arrays
        return ([], [], 44100.0)
    }
    
    static func features(from y: [Float], sr: Double) -> [String: Double] {
        let n = y.count
        
        // FIX: peak magnitude using Accelerate C API (Swift 6 friendly)
        var peak: Float = 0
        vDSP_maxmgv(y, 1, &peak, vDSP_Length(n))
        
        // For mock implementation, just return sample values
        return [
            "spectralCentroid": 1200.0,
            "spectralSpread": 800.0,
            "rmsEnergy": 0.3,
            "transients": 0.2,
            "lowEndDensity": 0.4
        ]
    }
    
    static func recommend(from features: [String: Double], part: String) -> MojoRecommendation {
        // Create recommendation using the SharedTypes version
        var interpMode = "liveHB4x"
        var drive: Float = 0.5
        var character: Float = 0.5
        var saturation: Float = 0.5
        var presence: Float = 0.5
        
        // Different presets based on instrument type
        switch part.lowercased() {
        case "bass":
            drive = 0.6
            character = 0.7
            saturation = 0.4
            presence = 0.3
            interpMode = "hqSinc8x"
        case "drums":
            drive = 0.5
            character = 0.4
            saturation = 0.6
            presence = 0.7
            interpMode = "transientSpline4x"
        case "vocal":
            drive = 0.4
            character = 0.6
            saturation = 0.5
            presence = 0.6
            interpMode = "adaptive"
        default:
            drive = 0.5
            character = 0.5
            saturation = 0.5
            presence = 0.5
            interpMode = "liveHB4x"
        }
        
        return MojoRecommendation(
            interpMode: interpMode,
            drive: drive,
            saturation: saturation,
            character: character,
            presence: presence,
            mix: 1.0,
            output: 0.0
        )
    }
    
    // FIX: eqMatchBands should return SharedTypes MojoEQMatch and build bands accordingly
    static func eqMatchBands(srcURL: URL, refURL: URL, bands: Int = 8) -> MojoEQMatch? {
        // Mock EQ match with correct MojoEQBand structure from SharedTypes
        return MojoEQMatch(bands: [
            MojoEQBand(lo: 50, hi: 200, gain_dB: 2.0),
            MojoEQBand(lo: 200, hi: 800, gain_dB: -1.5),
            MojoEQBand(lo: 800, hi: 3000, gain_dB: 3.0),
            MojoEQBand(lo: 3000, hi: 15000, gain_dB: -2.0)
        ])
    }
    
    // Helper for geometric mean calculation
    static func geometricMean(_ a: [Float]) -> Float {
        var sum: Float = 0
        var count: Float = 0
        for v in a where v > 0 {
            sum += logf(v)
            count += 1
        }
        return count > 0 ? expf(sum / count) : 0
    }
}
