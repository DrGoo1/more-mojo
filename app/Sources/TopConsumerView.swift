import SwiftUI

// Notification for showing Steal Mojo panel
extension Notification.Name {
    static let showStealMojoSheet = Notification.Name("showStealMojoSheet")
}

struct TopConsumerView: View {
    @ObservedObject var audioEngine: AudioEngine = AudioEngine.shared
    @State private var rotation: Double = 45.0
    @State private var intensity: Double = 0.7
    @State private var houseMacro: Bool = true
    @State private var selectedPreset: Int = 0
    
    // Audio transport state
    @State private var isPlaying: Bool = false
    @State private var uploadSheetShowing = false
    
    var presets = ["Default", "Warm Tube", "Clear Vocal", "Punchy Drum", "Deep Bass"]
    
    var body: some View {
        VStack(spacing: 20) {
            // Top row with controls
            HStack(spacing: 30) {
                // Mojo wheel
                VStack {
                    Text("MOJO WHEEL")
                        .font(.caption)
                        .fontWeight(.bold)
                        .foregroundColor(PMXPalette.highlight)
                    
                    MojoWheel(
                        rotation: $rotation,
                        intensity: $intensity,
                        onValueChange: { val in
                            applyWheelSettings()
                        },
                        mode: houseMacro ? .app : .steal
                    )
                    .frame(width: 180, height: 180)
                    
                    // Macro mode toggle
                    Toggle("HOUSE MACRO", isOn: $houseMacro)
                        .toggleStyle(SwitchToggleStyle(tint: PMXPalette.mojoGreen))
                        .padding(.horizontal)
                        .onChange(of: houseMacro) { _ in
                            applyWheelSettings()
                        }
                }
                .padding(.top)
                
                // Preset selector + waveform
                VStack {
                    // Preset picker
                    HStack {
                        Text("PRESET")
                            .font(.caption)
                            .fontWeight(.bold)
                            .foregroundColor(PMXPalette.highlight)
                        
                        Picker("", selection: $selectedPreset) {
                            ForEach(0..<presets.count, id: \.self) { i in
                                Text(presets[i]).tag(i)
                            }
                        }
                        .pickerStyle(MenuPickerStyle())
                        .onChange(of: selectedPreset) { _ in
                            applyPreset()
                        }
                        
                        Spacer()
                        
                        // Upload button
                        Button(action: {
                            uploadSheetShowing = true
                        }) {
                            HStack {
                                Image(systemName: "arrow.up.doc")
                                Text("Upload")
                            }
                            .padding(6)
                            .background(PMXPalette.mojoBlue)
                            .cornerRadius(8)
                        }
                        .buttonStyle(PlainButtonStyle())
                    }
                    
                    // Waveform
                    WaveformSimple.demo()
                        .frame(height: 100)
                        .padding(.vertical, 8)
                    
                    // Transport controls
                    HStack {
                        Button(action: { isPlaying.toggle() }) {
                            Image(systemName: isPlaying ? "pause.fill" : "play.fill")
                                .font(.title2)
                                .foregroundColor(PMXPalette.highlight)
                                .frame(width: 40, height: 40)
                                .background(PMXPalette.mojoGreen)
                                .clipShape(Circle())
                        }
                        .buttonStyle(PlainButtonStyle())
                        
                        Button(action: { 
                            // Stop
                            isPlaying = false
                        }) {
                            Image(systemName: "stop.fill")
                                .font(.title2)
                                .foregroundColor(PMXPalette.highlight)
                                .frame(width: 40, height: 40)
                                .background(Color.gray.opacity(0.5))
                                .clipShape(Circle())
                        }
                        .buttonStyle(PlainButtonStyle())
                        
                        Spacer()
                        
                        Button(action: {
                            // Open Steal Mojo sheet
                            NotificationCenter.default.post(name: .showStealMojoSheet, object: nil)
                        }) {
                            HStack {
                                Image(systemName: "magnifyingglass")
                                Text("Steal Mojo")
                            }
                            .padding(6)
                            .background(PMXPalette.mojoPurple)
                            .cornerRadius(8)
                        }
                        .buttonStyle(PlainButtonStyle())
                    }
                }
                .frame(maxWidth: .infinity)
                
                // Input/Output sliders
                VStack(spacing: 20) {
                    makeSlider(
                        title: "INPUT",
                        value: Binding(
                            get: { Double(audioEngine.inputLevel) },
                            set: { level in
                                var params = createParamsDict()
                                params["input"] = Float(level)
                                audioEngine.setParams(params)
                            }
                        ),
                        color: PMXPalette.mojoRed
                    )
                    
                    makeSlider(
                        title: "OUTPUT",
                        value: Binding(
                            get: { Double(audioEngine.outputLevel) },
                            set: { level in
                                var params = createParamsDict()
                                params["output"] = Float(level)
                                audioEngine.setParams(params)
                            }
                        ),
                        color: PMXPalette.mojoBlue
                    )
                }
                .frame(width: 120)
                .padding(.trailing)
            }
            .padding(.horizontal)
        }
        .background(PMXPalette.darkBackground)
        .sheet(isPresented: $uploadSheetShowing) {
            // Simple upload sheet
            VStack(spacing: 20) {
                Text("Upload Audio File")
                    .font(.headline)
                
                Button("Select File") {
                    uploadSheetShowing = false
                }
                .padding()
                .background(PMXPalette.mojoBlue)
                .foregroundColor(.white)
                .cornerRadius(8)
                
                Button("Cancel") {
                    uploadSheetShowing = false
                }
                .padding()
            }
            .padding(40)
        }
    }
    
    // Create a vertical slider with label
    private func makeSlider(title: String, value: Binding<Double>, color: Color) -> some View {
        VStack {
            Text(title)
                .font(.caption)
                .fontWeight(.bold)
                .foregroundColor(PMXPalette.highlight)
            
            Slider(value: value, in: 0...1, step: 0.01)
                .rotationEffect(.degrees(-90))
                .frame(width: 100, height: 30)
                .accentColor(color)
            
            Text(String(format: "%.0", value.wrappedValue * 100) + "%")
                .font(.caption)
                .foregroundColor(color)
        }
    }
    
    // Apply settings from the mojo wheel
    private func applyWheelSettings() {
        var params = createParamsDict()
        
        // Calculate wheel parameters based on rotation and intensity
        let rotationNormalized = (rotation / 360.0).truncatingRemainder(dividingBy: 1.0)
        
        // Map rotation to different parameters based on mode
        if houseMacro {
            // House macro mode - rotation affects multiple parameters
            params["drive"] = Float(rotationNormalized * 0.8 + 0.2)
            params["presence"] = Float((1.0 - rotationNormalized) * 0.7 + 0.3)
            params["saturation"] = Float(rotationNormalized * 0.6 + 0.2)
        } else {
            // Steal mode - rotation selects character profile
            params["character"] = Float(rotationNormalized * 0.8 + 0.2)
        }
        
        // Intensity always affects mix
        params["mix"] = Float(intensity)
        
        // Apply parameters
        audioEngine.setParams(params)
    }
    
    // Apply the selected preset
    private func applyPreset() {
        var params = createParamsDict()
        
        switch selectedPreset {
        case 0: // Default
            params["drive"] = 0.5
            params["saturation"] = 0.45
            params["character"] = 0.5
            params["presence"] = 0.5
            params["warmth"] = 0.6
            params["mix"] = 1.0
        case 1: // Warm Tube
            params["drive"] = 0.65
            params["saturation"] = 0.7
            params["character"] = 0.6
            params["presence"] = 0.4
            params["warmth"] = 0.8
            params["mix"] = 0.9
        case 2: // Clear Vocal
            params["drive"] = 0.4
            params["saturation"] = 0.3
            params["character"] = 0.4
            params["presence"] = 0.7
            params["warmth"] = 0.5
            params["mix"] = 0.8
        case 3: // Punchy Drum
            params["drive"] = 0.7
            params["saturation"] = 0.6
            params["character"] = 0.5
            params["presence"] = 0.6
            params["warmth"] = 0.4
            params["mix"] = 1.0
        case 4: // Deep Bass
            params["drive"] = 0.5
            params["saturation"] = 0.5
            params["character"] = 0.7
            params["presence"] = 0.3
            params["warmth"] = 0.9
            params["mix"] = 1.0
        default:
            break
        }
        
        // Apply parameters
        audioEngine.setParams(params)
    }
    
    // Create a parameter dictionary
    private func createParamsDict() -> [String: Any] {
        // Start with current parameters or defaults
        return [
            "drive": 0.5,
            "character": 0.5,
            "saturation": 0.45,
            "presence": 0.5,
            "warmth": 0.6,
            "output": 0.0,
            "mix": 1.0,
            "mode": 1
        ]
    }
}
