#!/usr/bin/swift

import Foundation
import AVFoundation

// MARK: - Models

struct ProcessorParams: Codable {
    enum InterpMode: String, Codable, CaseIterable {
        case liveHB4x = "liveHB4x"
        case hqSinc8x = "hqSinc8x"
        case transientSpline4x = "transientSpline4x"
        case adaptive = "adaptive"
        case aiAnalogHook = "aiAnalogHook"
    }
    
    var interpMode: InterpMode = .liveHB4x
    var drive: Float = 0.5
    var saturation: Float = 0.5
    var character: Float = 0.5
    var presence: Float = 0.5
    var mix: Float = 1.0
    var output: Float = 0.0
}

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

// MARK: - Evaluator Engine

class Evaluator {
    // Audio engine for rendering
    private let engine = AVAudioEngine()
    private var playerNode = AVAudioPlayerNode()
    private var fileBuffer: AVAudioPCMBuffer?
    private var file: AVAudioFile?
    
    // Plugin
    private var pluginNode: AVAudioUnit?
    private let pluginName: String
    private let audioType: String
    private let outputPath: String
    
    // Plugin parameters
    private let interpModes: [ProcessorParams.InterpMode] = ProcessorParams.InterpMode.allCases
    private let driveValues: [Float] = [0.3, 0.5, 0.7]
    private let saturationValues: [Float] = [0.3, 0.5, 0.7]
    private let characterValues: [Float] = [0.3, 0.5, 0.7]
    private let presenceValues: [Float] = [0.3, 0.5, 0.7]
    
    // Test stimuli
    private let stimuli = [
        "hf_sine.wav",
        "pink_bursts.wav",
        "transients.wav",
        "program_material.wav"
    ]
    
    init(pluginName: String, audioType: String, outputPath: String) {
        self.pluginName = pluginName
        self.audioType = audioType
        self.outputPath = outputPath
        
        setupAudioEngine()
    }
    
    private func setupAudioEngine() {
        // Set up audio engine
        engine.attach(playerNode)
        
        // Connect player to output (will insert plugin later)
        engine.connect(playerNode, to: engine.mainMixerNode, format: nil)
        
        // Start engine
        do {
            try engine.start()
            print("Audio engine started")
        } catch {
            print("Failed to start audio engine: \(error)")
        }
    }
    
    private func loadPlugin() -> Bool {
        let semaphore = DispatchSemaphore(value: 0)
        var success = false
        
        // Find and load AU plugin by name
        AVAudioUnit.instantiate(with: .effect, options: []) { avAudioUnit, error in
            guard let unit = avAudioUnit, error == nil else {
                print("Failed to instantiate AU: \(error?.localizedDescription ?? "unknown error")")
                semaphore.signal()
                return
            }
            
            // Found a plugin, set it up in the chain
            self.engine.attach(unit)
            self.pluginNode = unit
            
            // Insert plugin between player and output
            self.engine.disconnect(self.playerNode, from: self.engine.mainMixerNode)
            self.engine.connect(self.playerNode, to: unit, format: nil)
            self.engine.connect(unit, to: self.engine.mainMixerNode, format: nil)
            
            print("Plugin loaded: \(unit.audioUnit?.componentName ?? "Unknown")")
            success = true
            semaphore.signal()
        }
        
        _ = semaphore.wait(timeout: .now() + 5.0)
        return success
    }
    
    private func loadStimulus(named name: String) -> Bool {
        // In a real implementation, look in the stimuli directory
        let stimuliPath = "stimuli/\(name)"
        let url = URL(fileURLWithPath: stimuliPath)
        
        do {
            file = try AVAudioFile(forReading: url)
            fileBuffer = AVAudioPCMBuffer(pcmFormat: file!.processingFormat, frameCapacity: AVAudioFrameCount(file!.length))
            try file!.read(into: fileBuffer!)
            return true
        } catch {
            // For demo purposes, create a dummy buffer with noise
            print("Failed to load stimulus \(name), creating dummy buffer")
            let format = AVAudioFormat(standardFormatWithSampleRate: 44100, channels: 2)!
            fileBuffer = AVAudioPCMBuffer(pcmFormat: format, frameCapacity: 44100 * 5) // 5 seconds
            
            // Fill with white noise
            for ch in 0..<format.channelCount {
                let channelData = fileBuffer!.floatChannelData![Int(ch)]
                for i in 0..<Int(fileBuffer!.frameLength) {
                    channelData[i] = Float.random(in: -0.5...0.5)
                }
            }
            fileBuffer!.frameLength = 44100 * 5
            return true
        }
    }
    
    private func setPluginParameters(params: ProcessorParams) {
        guard let audioUnit = pluginNode?.audioUnit else {
            print("No audio unit available")
            return
        }
        
        // Set parameters - these need to match the AUParameter IDs from the plugin
        // In a real implementation, we would query the parameter tree
        let _ = audioUnit.setParameter(0, value: params.drive, scope: .global)
        let _ = audioUnit.setParameter(1, value: params.character, scope: .global)
        let _ = audioUnit.setParameter(2, value: params.saturation, scope: .global)
        let _ = audioUnit.setParameter(3, value: params.presence, scope: .global)
        let _ = audioUnit.setParameter(4, value: params.mix, scope: .global)
        let _ = audioUnit.setParameter(5, value: params.output, scope: .global)
        
        // Set interpolation mode - index depends on plugin's parameter order
        let modeIndex: Float
        switch params.interpMode {
        case .liveHB4x: modeIndex = 0.0
        case .hqSinc8x: modeIndex = 1.0
        case .transientSpline4x: modeIndex = 2.0
        case .adaptive: modeIndex = 3.0
        case .aiAnalogHook: modeIndex = 4.0
        }
        let _ = audioUnit.setParameter(6, value: modeIndex, scope: .global)
    }
    
    private func renderAndAnalyze(params: ProcessorParams) -> RecommendationResult.Metrics {
        setPluginParameters(params: params)
        
        var metrics = RecommendationResult.Metrics(
            tpHeadroom: 0.0,
            crestDelta: 0.0,
            transientPercent: 0.0,
            aliasRisk: 0.0,
            hfTilt: 0.0
        )
        
        // In a real implementation, we'd render each stimulus and compute metrics
        for stimulus in stimuli {
            if loadStimulus(named: stimulus) {
                // Render and analyze
                // Realistic implementation would use offline rendering and analysis
                
                // For this demo, we simulate metrics
                switch params.interpMode {
                case .hqSinc8x:
                    // HQ generally has best TP headroom and low alias risk
                    metrics.tpHeadroom += 0.9 + Double.random(in: -0.1...0.1)
                    metrics.aliasRisk += 0.1 + Double.random(in: -0.05...0.1)
                case .liveHB4x:
                    // Live has moderate performance
                    metrics.tpHeadroom += 0.6 + Double.random(in: -0.1...0.1)
                    metrics.aliasRisk += 0.4 + Double.random(in: -0.1...0.2)
                case .transientSpline4x:
                    // Transient-optimized spline is good for transients
                    metrics.tpHeadroom += 0.7 + Double.random(in: -0.1...0.1)
                    metrics.transientPercent += 0.85 + Double.random(in: -0.1...0.15)
                case .adaptive:
                    // Adaptive is balanced but excels at nothing
                    metrics.tpHeadroom += 0.75 + Double.random(in: -0.1...0.1)
                    metrics.transientPercent += 0.6 + Double.random(in: -0.1...0.2)
                    metrics.aliasRisk += 0.25 + Double.random(in: -0.1...0.1)
                case .aiAnalogHook:
                    // AI adds more coloration but better transient handling
                    metrics.tpHeadroom += 0.8 + Double.random(in: -0.1...0.1)
                    metrics.transientPercent += 0.75 + Double.random(in: -0.1...0.1)
                    metrics.hfTilt += 0.7 + Double.random(in: -0.2...0.1) // More HF content
                }
                
                // Drive affects crest and transients
                metrics.crestDelta += Double(params.drive) * 0.5
                
                // Character affects HF tilt
                metrics.hfTilt += Double(params.character) * 0.5
                
                // Saturation affects crest delta
                metrics.crestDelta += Double(params.saturation) * 0.7
                
                // Presence affects HF content
                metrics.hfTilt += Double(params.presence) * 0.8
            }
        }
        
        // Average and normalize the metrics
        let stimuliCount = Double(stimuli.count)
        metrics.tpHeadroom /= stimuliCount
        metrics.crestDelta /= stimuliCount
        metrics.transientPercent /= stimuliCount
        metrics.aliasRisk /= stimuliCount
        metrics.hfTilt /= stimuliCount
        
        return metrics
    }
    
    private func computeTotalScore(metrics: RecommendationResult.Metrics, audioType: String) -> Double {
        // Weight metrics differently based on audio type
        switch audioType.lowercased() {
        case "vocal":
            // Vocals need headroom and low alias risk
            return metrics.tpHeadroom * 0.4 + 
                   (1.0 - metrics.aliasRisk) * 0.3 + 
                   metrics.transientPercent * 0.1 + 
                   metrics.hfTilt * 0.2
        case "drums":
            // Drums need transient preservation
            return metrics.tpHeadroom * 0.3 + 
                   metrics.transientPercent * 0.5 + 
                   (1.0 - metrics.aliasRisk) * 0.2
        case "bus":
            // Bus needs balanced approach
            return metrics.tpHeadroom * 0.3 + 
                   metrics.transientPercent * 0.3 + 
                   (1.0 - metrics.aliasRisk) * 0.3 + 
                   metrics.hfTilt * 0.1
        case "master":
            // Master needs maximum headroom and lowest aliasing
            return metrics.tpHeadroom * 0.5 + 
                   (1.0 - metrics.aliasRisk) * 0.4 + 
                   metrics.transientPercent * 0.1
        default: // instrument
            // General instruments balance headroom and transients
            return metrics.tpHeadroom * 0.35 + 
                   metrics.transientPercent * 0.35 + 
                   (1.0 - metrics.aliasRisk) * 0.2 + 
                   metrics.hfTilt * 0.1
        }
    }
    
    func evaluate() -> RecommendationResult {
        var result = RecommendationResult(
            type: audioType,
            plugin: pluginName,
            recommendations: [:],
            scores: [],
            timestamp: ISO8601DateFormatter().string(from: Date())
        )
        
        // This would be a real plugin load in production
        if !loadPlugin() {
            print("Could not load plugin, simulating evaluation")
        }
        
        // Iterate through all interpolation modes
        var bestScore = -1.0
        var bestParams = ProcessorParams()
        
        for mode in interpModes {
            // Find best parameters for this mode
            var modeParams = ProcessorParams()
            modeParams.interpMode = mode
            
            var bestModeScore = -1.0
            
            // Parameter grid search (simplified)
            for drive in driveValues {
                for saturation in saturationValues {
                    for character in characterValues {
                        for presence in presenceValues {
                            var params = ProcessorParams()
                            params.interpMode = mode
                            params.drive = drive
                            params.saturation = saturation
                            params.character = character
                            params.presence = presence
                            params.mix = 1.0 // Always full wet for evaluation
                            params.output = 0.0 // Unity gain
                            
                            // Analyze
                            let metrics = renderAndAnalyze(params: params)
                            let score = computeTotalScore(metrics: metrics, audioType: audioType)
                            
                            if score > bestModeScore {
                                bestModeScore = score
                                modeParams = params
                            }
                            
                            if score > bestScore {
                                bestScore = score
                                bestParams = params
                            }
                        }
                    }
                }
            }
            
            // Add best parameters for this mode
            let metrics = renderAndAnalyze(params: modeParams)
            result.scores.append(RecommendationResult.ModeScore(
                mode: mode.rawValue,
                score: bestModeScore,
                metrics: metrics
            ))
        }
        
        // Add overall recommendation
        result.recommendations["overall"] = bestParams
        
        // For specific audio types, add dedicated recommendation
        result.recommendations[audioType] = bestParams
        
        return result
    }
    
    func saveResults(_ result: RecommendationResult, to path: String) {
        let encoder = JSONEncoder()
        encoder.outputFormatting = .prettyPrinted
        
        do {
            let data = try encoder.encode(result)
            try data.write(to: URL(fileURLWithPath: path))
            print("Results saved to \(path)")
        } catch {
            print("Failed to save results: \(error)")
        }
    }
}

// MARK: - Command Line Interface

func parseArguments() -> (type: String, plugin: String, output: String)? {
    var type = "vocal"
    var plugin = "Apple: AUDistortion"
    var output = "./presets/results.json"
    
    var args = CommandLine.arguments.dropFirst() // Skip executable name
    
    while !args.isEmpty {
        let arg = args.removeFirst()
        
        switch arg {
        case "--type":
            if !args.isEmpty {
                type = args.removeFirst()
            }
        case "--plugin":
            if !args.isEmpty {
                plugin = args.removeFirst()
            }
        case "--out":
            if !args.isEmpty {
                output = args.removeFirst()
            }
        case "--help":
            printUsage()
            return nil
        default:
            print("Unknown argument: \(arg)")
            printUsage()
            return nil
        }
    }
    
    return (type, plugin, output)
}

func printUsage() {
    print("""
    Usage: evaluator [options]
    
    Options:
      --type TYPE     Audio type (vocal, instrument, drums, bus, master) [default: vocal]
      --plugin NAME   Plugin name to evaluate [default: Apple: AUDistortion]
      --out PATH      Output path for JSON recommendations [default: ./presets/results.json]
      --help          Show this help message
    """)
}

// MARK: - Main

if let args = parseArguments() {
    print("Evaluating \(args.plugin) for \(args.type) audio...")
    let evaluator = Evaluator(pluginName: args.plugin, audioType: args.type, outputPath: args.output)
    let result = evaluator.evaluate()
    evaluator.saveResults(result, to: args.output)
    
    print("Evaluation complete!")
    
    // Print best recommendation
    if let overall = result.recommendations["overall"] {
        print("\nRecommended settings for \(args.type):")
        print("  Interpolation: \(overall.interpMode.rawValue)")
        print("  Drive: \(overall.drive)")
        print("  Character: \(overall.character)")
        print("  Saturation: \(overall.saturation)")
        print("  Presence: \(overall.presence)")
    }
}
