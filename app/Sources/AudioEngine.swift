import Foundation
import AVFoundation
import Accelerate

/// Audio processing engine for the More Mojo application
class AudioEngine {
    // Singleton instance
    static let shared = AudioEngine()
    
    // Audio engine for processing
    private var audioEngine: AVAudioEngine?
    
    // Processing nodes
    private var inputNode: AVAudioNode?
    private var outputNode: AVAudioNode?
    private var processingNode: AVAudioUnitEffect?
    
    // Audio file players
    private var audioPlayer: AVAudioPlayerNode?
    private var currentFile: AVAudioFile?
    
    // Current parameters
    private(set) var currentParams = ProcessorParams()
    
    // Interpolator instances
    private lazy var liveInterpolator = HalftBandInterpolator()
    private lazy var hqInterpolator = SincInterpolator()
    private lazy var splineInterpolator = TransientSplineInterpolator()
    private lazy var adaptiveInterpolator = AdaptiveInterpolator()
    
    // Analog shaping instance
    private lazy var analogShaper = AnalogInterpolator()
    
    // Audio meter levels
    private(set) var inputLevel: Float = 0.0
    private(set) var outputLevel: Float = 0.0
    
    // Spectrum analyzer data
    private(set) var spectrumData: [Float] = Array(repeating: 0, count: 128)
    
    // FFT setup
    private var fftSetup: FFTSetup?
    private let fftLength = 1024
    
    // Initialize with default configuration
    private init() {}
    
    // Set up audio system
    func setupAudioSystem() {
        // Set up audio engine
        setupAudioEngine()
        
        // Set up FFT for spectrum analysis
        setupFFT()
    }
    
    // Set up audio configuration for macOS
    private func setupAudioSession() {
        // No AVAudioSession on macOS - this would typically involve CoreAudio setup
//         print("Setting up audio system for macOS")
    }
    
    // Set up the audio engine
    private func setupAudioEngine() {
        audioEngine = AVAudioEngine()
        
        guard let engine = audioEngine else { return }
        
        // Set up the player node for audio file playback
        audioPlayer = AVAudioPlayerNode()
        
        if let player = audioPlayer {
            engine.attach(player)
        }
        
        // Main mixer
        let mainMixer = engine.mainMixerNode
        
        // Connect nodes
        if let player = audioPlayer {
            engine.connect(player, to: mainMixer, format: mainMixer.outputFormat(forBus: 0))
        }
        
        // Start the engine
        do {
            try engine.start()
//             print("Audio engine started successfully")
        } catch {
//             print("Failed to start audio engine: \(error.localizedDescription)")
        }
    }
    
    // Set up FFT for spectrum analysis
    private func setupFFT() {
        fftSetup = vDSP_create_fftsetup(vDSP_Length(log2(Float(fftLength))), FFTRadix(kFFTRadix2))
    }
    
    // Load an audio file
    func loadAudioFile(url: URL) -> Bool {
        do {
            let audioFile = try AVAudioFile(forReading: url)
            currentFile = audioFile
            
            guard let player = audioPlayer else { return false }
            player.stop()
            
            // Schedule file for playback
            player.scheduleFile(audioFile, at: nil)
            
            return true
        } catch {
//             print("Failed to load audio file: \(error.localizedDescription)")
            return false
        }
    }
    
    // Play loaded audio file
    func playAudioFile() -> Bool {
        guard let _ = audioEngine, let player = audioPlayer, let _ = currentFile else {
            return false
        }
        
        player.play()
        return true
    }
    
    // Stop playback
    func stopPlayback() {
        audioPlayer?.stop()
    }
    
    // Process audio with the specified parameters
    func processAudio(with params: ProcessorParams) -> Bool {
        // Store the parameters
        currentParams = params
        
        // Set up the appropriate interpolator based on the specified mode
        let interpolator: InterpolatorProtocol
        
        switch params.interpMode {
        case .liveHB4x:
            interpolator = liveInterpolator
        case .hqSinc8x:
            interpolator = hqInterpolator
        case .transientSpline4x:
            interpolator = splineInterpolator
        case .adaptive:
            interpolator = adaptiveInterpolator
        case .aiAnalogHook:
            // For AI, we still use the HB interpolator but with AI enhancement later
            interpolator = liveInterpolator
        }
        
        // Apply processing based on parameters
        applyProcessing(params: params, interpolator: interpolator)
        
        // Simulate audio processing for demonstration
        simulateProcessing()
        
        return true
    }
    
    // Apply audio processing based on parameters
    private func applyProcessing(params: ProcessorParams, interpolator: InterpolatorProtocol) {
        // For demo purposes, we'll just print the parameters
        // In a real app, this would apply DSP effects to the audio chain
        // print("Applying processing with parameters:")
        // print("  Mojo Level: \(params.mojoLevel.rawValue)")
        // print("  Audio Type: \(params.audioType.rawValue)")
        
        // Apply analog shaping
        analogShaper.updateParameters(
            drive: params.drive,
            character: params.character,
            saturation: params.saturation,
            presence: params.presence,
            warmth: params.warmth,
            output: params.output,
            mix: params.mix,
            mode: params.mode
        )
    }
    
    // Simulate processing for demonstration purposes
    private func simulateProcessing() {
        // Generate some fake metering data
        inputLevel = Float.random(in: 0.2...0.8)
        outputLevel = inputLevel * (currentParams.drive * 0.5 + 0.5)
        
        // Generate some fake spectrum data
        for i in 0..<spectrumData.count {
            let freq = Float(i) / Float(spectrumData.count)
            spectrumData[i] = Float.random(in: 0...0.3) + (0.7 * (1.0 - freq))
        }
    }
    
    // Set parameters directly
    func setParams(_ params: ProcessorParams) {
        self.currentParams = params
        // This would actually apply the parameters to audio processing in a real implementation
        processAudio(with: params)
    }
    
    // Get the current input level (0.0 to 1.0)
    func getCurrentInputLevel() -> Double {
        return Double(inputLevel)
    }
    
    // Get the current output level (0.0 to 1.0)
    func getCurrentOutputLevel() -> Double {
        return Double(outputLevel)
    }
    
    // Get the current spectrum data for visualization
    func getSpectrumData() -> [Float] {
        return spectrumData
    }
    
    // Clean up resources
    deinit {
        audioEngine?.stop()
        fftSetup.map { vDSP_destroy_fftsetup($0) }
    }
}
