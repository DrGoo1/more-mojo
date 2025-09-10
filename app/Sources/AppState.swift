import SwiftUI
import Combine
import Foundation
import AVFoundation
import ObjectiveC // For NSObject and Objective-C runtime

/// Main application state shared across all views
class AppState: ObservableObject {
    // Audio engine reference (accessed indirectly)
    private var audioEngineRef: Any? = nil
    
    // Helper to access audioEngine safely
    private func getAudioEngine() -> Any? {
        if audioEngineRef == nil {
            audioEngineRef = NSClassFromString("AudioEngine")?.value(forKey: "shared")
        }
        return audioEngineRef
    }
    
    // MARK: - Published Properties
    
    // Audio file handling
    @Published var currentAudioFile: String = ""
    @Published var recentAudioFiles: [String] = []
    
    // Audio device management
    @Published var availableInputDevices: [String] = ["Built-in Microphone", "Audio Interface"]
    @Published var availableOutputDevices: [String] = ["Built-in Speakers", "Audio Interface"]
    @Published var selectedInputDevice: String = "Built-in Microphone"
    @Published var selectedOutputDevice: String = "Built-in Speakers"
    
    // Processing state
    @Published var isProcessing: Bool = false
    @Published var processingProgress: Double = 0.0
    @Published var processingMessage: String = ""
    
    // UI state
    @Published var currentView: String = "main"
    
    // Audio enhancement settings
    @Published var aiEnabled: Bool = true
    @Published var presets: [String] = ["Default", "Vintage Warm", "Crystal Clear", "Dynamic Punch", "Subtle Enhancement"]
    @Published var activePreset: String = "Default"
    
    // Cancellables
    private var cancellables = Set<AnyCancellable>()
    
    // No need to redeclare audioEngineRef as it's already declared at the top
    
    // MARK: - Initialization
    
    init() {
        // Get audio engine reference through runtime to avoid import cycles
        self.audioEngineRef = NSClassFromString("AudioEngine")?.value(forKey: "shared")        // Initialize with default values
        loadRecentFiles()
    }
    
    // MARK: - Public Methods
    
    /// Sync with audio hardware
    func syncWithHardware() {
        // In a real app, this would detect available audio devices
//         print("Syncing with audio hardware...")
    }
    
    /// Open an audio file for processing
    func openAudioFile() {
        // In a real app, this would show an open file dialog
        let fileName = "Sample Audio \(Int.random(in: 1...100)).wav"
        currentAudioFile = fileName
        
        // Add to recent files if not already present
        if !recentAudioFiles.contains(fileName) {
            recentAudioFiles.insert(fileName, at: 0)
            // Keep only the 5 most recent files
            if recentAudioFiles.count > 5 {
                recentAudioFiles.removeLast()
            }
        }
//         
//         print("Opened audio file: \(fileName)")
    }
    
    /// Process the current audio file with the specified preset and AI settings
    func processAudio(preset: String, useAI: Bool) -> Bool {
        guard !currentAudioFile.isEmpty else {
            processingMessage = "No audio file selected"
            return false
        }
        
        // Start processing
        isProcessing = true
        processingProgress = 0.0
        processingMessage = "Processing \(currentAudioFile) with \(preset)..."
        
        // Create parameters as dictionary for flexibility
        var params: [String: Any] = [:]
        params["aiEnhance"] = useAI
        
        // Set mojo level based on preset
        if preset.contains("Warm") {
            params["mojoLevel"] = "mojo"
        } else if preset.contains("Crystal") || preset.contains("Subtle") {
            params["mojoLevel"] = "moreMojo"
        } else if preset.contains("Dynamic") || preset.contains("Punch") {
            params["mojoLevel"] = "mostMojo"
        }
        
        // Set audio type based on file analysis or default to mix
        params["audioType"] = "mix"
        
        // Use AudioEngine for actual processing
        let success = (getAudioEngine() as? NSObject)?.perform(NSSelectorFromString("processAudio:"), with: params) != nil
        
        if success {
            // Simulate processing progress with a timer
            let timer = Timer.publish(every: 0.1, on: .main, in: .common).autoconnect()
            timer.sink { [weak self] _ in
                guard let self = self else { return }
                
                if self.processingProgress < 1.0 {
                    self.processingProgress += 0.05
                    self.processingMessage = "Processing \(self.currentAudioFile)... \(Int(self.processingProgress * 100))%"
                } else {
                    self.processingMessage = "Processing complete"
                    self.isProcessing = false
                    // Cancel the timer
                    for cancellable in self.cancellables {
                        cancellable.cancel()
                    }
                }
            }
            .store(in: &cancellables)
        } else {
            isProcessing = false
            processingMessage = "Processing failed"
            return false
        }
        
        return true
    }
    
    // MARK: - Private Methods
    
    /// Load recent files from user defaults
    private func loadRecentFiles() {
        // In a real app, this would load from UserDefaults
        recentAudioFiles = [
            "Drum Loop 1.wav",
            "Guitar Track.wav",
            "Vocal Take 3.wav"
        ]
    }
}
typealias AppStateStub = AppState
