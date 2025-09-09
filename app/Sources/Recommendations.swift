import Foundation
import SwiftUI

// MARK: - Models

struct RecommendationResult: Codable {
    struct ModeScore: Codable {
        var mode: String
        var score: Double
        var metrics: Metrics
    }
    
    struct Metrics: Codable {
        var tpHeadroom: Double
        var crestDelta: Double
        var transientPercent: Double
        var aliasRisk: Double
        var hfTilt: Double
    }
    
    var type: String
    var plugin: String
    var recommendations: [String: ProcessorParams]
    var scores: [ModeScore]
    var timestamp: String
}

// MARK: - Recommendations Handler

class RecommendationsManager {
    static let shared = RecommendationsManager()
    
    private init() {}
    
    func loadRecommendation(for audioType: String) -> RecommendationResult? {
        let fileName = "\(audioType)_reco.json"
        
        // First try the app bundle
        if let bundlePath = Bundle.main.path(forResource: audioType + "_reco", ofType: "json") {
            return loadFromPath(bundlePath)
        }
        
        // Try user's Documents folder
        let fileManager = FileManager.default
        let documentsPath = fileManager.urls(for: .documentDirectory, in: .userDomainMask)[0]
        let filePath = documentsPath.appendingPathComponent(fileName).path
        
        if fileManager.fileExists(atPath: filePath) {
            return loadFromPath(filePath)
        }
        
        // Check the evaluator directory (for development)
        let evaluatorPath = (ProcessInfo.processInfo.environment["PROJECT_DIR"] ?? "") + "/tools/evaluator_cli/presets/\(fileName)"
        if fileManager.fileExists(atPath: evaluatorPath) {
            return loadFromPath(evaluatorPath)
        }
        
        return nil
    }
    
    private func loadFromPath(_ path: String) -> RecommendationResult? {
        do {
            let data = try Data(contentsOf: URL(fileURLWithPath: path))
            let decoder = JSONDecoder()
            return try decoder.decode(RecommendationResult.self, from: data)
        } catch {
            print("Error loading recommendation: \(error)")
            return nil
        }
    }
}

// MARK: - Recommendations UI

struct RecommendationsPanel: View {
    @State private var selectedType = "vocal"
    @State private var recommendation: RecommendationResult?
    @State private var status = ""
    
    var onApply: ((ProcessorParams) -> Void)?
    
    var body: some View {
        VStack(alignment: .leading, spacing: 16) {
            Text("Apply Recommendation").font(.headline)
            
            Picker("Audio Type", selection: $selectedType) {
                Text("Vocal").tag("vocal")
                Text("Instrument").tag("instrument")
                Text("Drums").tag("drums")
                Text("Bus").tag("bus")
                Text("Master").tag("master")
            }
            .pickerStyle(.segmented)
            .onChange(of: selectedType) { newValue in
                loadRecommendation()
            }
            
            Button("Load Recommendation") {
                loadRecommendation()
            }
            .buttonStyle(.borderedProminent)
            
            if let rec = recommendation {
                VStack(alignment: .leading, spacing: 8) {
                    Text("Recommendation for \(rec.type)").font(.subheadline)
                    Text("Generated: \(formatTimestamp(rec.timestamp))")
                        .font(.caption)
                        .foregroundColor(.secondary)
                    
                    Divider()
                    
                    if let overall = rec.recommendations["overall"] {
                        VStack(alignment: .leading, spacing: 4) {
                            Text("Recommended settings:").font(.subheadline)
                            Text("Interpolation: \(formatInterpolation(overall.interpMode))")
                            Text("Drive: \(String(format: "%.2f", overall.drive))")
                            Text("Character: \(String(format: "%.2f", overall.character))")
                            Text("Saturation: \(String(format: "%.2f", overall.saturation))")
                            Text("Presence: \(String(format: "%.2f", overall.presence))")
                        }
                        .padding(.vertical, 4)
                        
                        Button("Apply Recommendation") {
                            onApply?(overall)
                            status = "Applied recommendation for \(rec.type)"
                        }
                        .buttonStyle(.bordered)
                    }
                }
                .padding()
                .background(Color.secondary.opacity(0.1))
                .cornerRadius(8)
            } else if !status.isEmpty && !status.contains("Applied") {
                Text(status)
                    .foregroundColor(.red)
            }
            
            if !status.isEmpty && status.contains("Applied") {
                Text(status)
                    .foregroundColor(.green)
                    .padding(.top, 4)
            }
        }
        .padding()
        .frame(width: 400)
        .onAppear {
            loadRecommendation()
        }
    }
    
    private func loadRecommendation() {
        status = "Loading recommendation..."
        
        if let rec = RecommendationsManager.shared.loadRecommendation(for: selectedType) {
            recommendation = rec
            status = "Recommendation loaded"
        } else {
            recommendation = nil
            status = "No recommendation found for \(selectedType)"
        }
    }
    
    private func formatTimestamp(_ timestamp: String) -> String {
        // Convert ISO8601 to more readable format
        let dateFormatter = ISO8601DateFormatter()
        if let date = dateFormatter.date(from: timestamp) {
            let outputFormatter = DateFormatter()
            outputFormatter.dateStyle = .medium
            outputFormatter.timeStyle = .short
            return outputFormatter.string(from: date)
        }
        return timestamp
    }
    
    private func formatInterpolation(_ mode: ProcessorParams.InterpMode) -> String {
        switch mode {
        case .liveHB4x: return "Live (HB 4×)"
        case .hqSinc8x: return "HQ (Sinc 8×)"
        case .transientSpline4x: return "Transient (Spline 4×)"
        case .adaptive: return "Adaptive"
        case .aiAnalogHook: return "AI Analog"
        }
    }
}
