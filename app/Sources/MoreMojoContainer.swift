import SwiftUI

/// Container view that manages switching between simple and advanced views
struct MoreMojoContainer: View {
    @EnvironmentObject var appState: AppState
    @State private var showingMakerView = false
    
    // Audio parameters
    @State private var drive: Double = 0.55
    @State private var character: Double = 0.50
    @State private var saturation: Double = 0.45
    @State private var presence: Double = 0.50
    @State private var mix: Double = 1.0
    @State private var outputGain: Double = 0.0
    @State private var mojoSelectorIndex: Int = 1
    @State private var typeSelectorIndex: Int = 4
    
    // Toggle states
    @State private var toggle1: Bool = true
    
    // Audio engine reference
    private let audioEngine = AudioEngine.shared
    
    // Computed properties for spectrum visualization
    private var spectrum: [Float] {
        return audioEngine.getSpectrumData()
    }
    
    private var vuMeterL: CGFloat {
        return CGFloat(audioEngine.getCurrentInputLevel())
    }
    
    private var vuMeterR: CGFloat {
        return CGFloat(audioEngine.getCurrentOutputLevel())
    }
    
    var body: some View {
        VStack {
            // Title bar
            HStack {
                Text("More Mojo")
                    .font(.largeTitle)
                    .fontWeight(.bold)
                    .padding(.leading)
                
                Spacer()
                
                Button(action: {
                    // Process audio with current settings
                    let params = getProcessorParams()
                    _ = audioEngine.processAudio(with: params)
                }) {
                    Text("Process")
                        .padding(.horizontal, 20)
                        .padding(.vertical, 8)
                        .background(Color.blue)
                        .foregroundColor(.white)
                        .cornerRadius(8)
                }
                .padding(.trailing)
            }
            .padding(.top)
            
            // Container with conditional view
            ZStack {
                // Simple view (default)
                if !showingMakerView {
                    MoreMojoSimpleView(
                        mojoSelectorIndex: $mojoSelectorIndex,
                        typeSelectorIndex: $typeSelectorIndex,
                        onAdvanced: {
                            withAnimation(.spring()) {
                                showingMakerView = true
                            }
                        },
                        onProcess: {
                            let params = getProcessorParams()
                            _ = audioEngine.processAudio(with: params)
                        }
                    )
                }
                
                // Advanced view (when showingMakerView is true)
                if showingMakerView {
                    MojoMakerView(
                        drive: $drive,
                        mix: $mix,
                        outputNorm: $outputGain,
                        selectorIndex: $mojoSelectorIndex,
                        toggle1: $toggle1,
                        spectrum: spectrum,
                        vuL: vuMeterL,
                        vuR: vuMeterR,
                        onChange: {
                            let params = getProcessorParams()
                            _ = audioEngine.processAudio(with: params)
                        },
                        onBack: {
                            withAnimation(.spring()) {
                                showingMakerView = false
                            }
                        }
                    )
                    .transition(.move(edge: .bottom))
                }
            }
            .padding()
            .background(Color.black.opacity(0.05))
            .cornerRadius(12)
            
            Spacer()
        }
        .padding()
        .onAppear {
            // Set initial parameters based on presets
            updateParametersFromPresets()
        }
    }
    
    // Create processor parameters from current UI state
    private func getProcessorParams() -> ProcessorParams {
        var params = ProcessorParams()
        
        // Set mojo level based on selector index
        params.mojoLevel = getMojoLevelFromIndex(mojoSelectorIndex)
        
        // Set audio type based on selector index
        params.audioType = getAudioTypeFromIndex(typeSelectorIndex)
        
        // Set other parameters
        params.drive = Float(drive)
        params.character = Float(character)
        params.saturation = Float(saturation)
        params.presence = Float(presence)
        params.mix = Float(mix)
        params.output = Float(outputGain)
        params.aiEnhance = appState.aiEnabled
        
        return params
    }
    
    // Update parameters from presets based on current selection
    private func updateParametersFromPresets() {
        let mojoLevel = getMojoLevelFromIndex(mojoSelectorIndex)
        let audioType = getAudioTypeFromIndex(typeSelectorIndex)
        
        let preset = ProcessorParams.presetFor(level: mojoLevel, type: audioType)
        
        // Update state with preset values
        drive = Double(preset.drive)
        character = Double(preset.character)
        saturation = Double(preset.saturation)
        presence = Double(preset.presence)
        mix = Double(preset.mix)
        outputGain = Double(preset.output)
        
        // Apply to audio engine
        _ = audioEngine.processAudio(with: preset)
    }
    
    // Convert selector index to MojoLevel
    private func getMojoLevelFromIndex(_ index: Int) -> ProcessorParams.MojoLevel {
        switch index {
        case 0: return .mojo
        case 2: return .mostMojo
        default: return .moreMojo // Default to "More Mojo" (middle option)
        }
    }
    
    // Convert selector index to AudioType
    private func getAudioTypeFromIndex(_ index: Int) -> ProcessorParams.AudioType {
        switch index {
        case 0: return .drums
        case 1: return .bass
        case 2: return .guitar
        case 3: return .vocals
        default: return .mix // Default to "Full Mix"
        }
    }
}
