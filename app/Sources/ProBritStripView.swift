import SwiftUI

struct ProBritStripView: View {
    @ObservedObject var audioEngine: AudioEngine = AudioEngine.shared
    
    // UI State
    @State private var faceplateLayout: FaceplateLayout?
    @State private var knobValues: [String: Double] = [
        "input": 0.5,
        "drive": 0.55,
        "character": 0.5,
        "saturation": 0.45,
        "presence": 0.5,
        "mix": 1.0,
        "output": 0.5
    ]
    
    @State private var switchValues: [String: Bool] = [
        "hp": true,
        "tp": false
    ]
    
    @State private var interpMode = 0
    
    var body: some View {
        ZStack {
            // Background
            PMXPalette.darkBackground.ignoresSafeArea()
            
            VStack {
                // Meters and main display
                HStack(spacing: 20) {
                    // Left VU meter
                    vuMeter(level: CGFloat(audioEngine.inputLevel))
                        .frame(width: 120, height: 80)
                    
                    // Spectrum display
                    spectrumDisplay(values: audioEngine.getSpectrumData())
                        .frame(height: 80)
                    
                    // Right VU meter
                    vuMeter(level: CGFloat(audioEngine.outputLevel))
                        .frame(width: 120, height: 80)
                }
                .padding(.horizontal, 20)
                .padding(.top, 10)
                
                // Faceplate with controls
                ZStack {
                    // Load faceplate layout from JSON if available
                    if faceplateLayout != nil {
                        BritStripFaceplate(layout: faceplateLayout!) {
                            // Control content goes here
                        }
                        .aspectRatio(2000/750, contentMode: .fit)
                    } else {
                        // Placeholder if layout not available
                        Rectangle()
                            .fill(Color.black.opacity(0.8))
                            .overlay(Text("BritStrip Pro").foregroundColor(.white))
                            .aspectRatio(2000/750, contentMode: .fit)
                    }
                    
                    // Overlay actual interactive controls
                    VStack {
                        // Top row - main knobs
                        HStack(spacing: 80) {
                            knob(id: "input", label: "INPUT")
                            knob(id: "drive", label: "DRIVE")
                            knob(id: "character", label: "CHARACTER")
                            knob(id: "saturation", label: "SATURATION")
                            knob(id: "presence", label: "PRESENCE")
                            knob(id: "mix", label: "MIX")
                            knob(id: "output", label: "OUTPUT")
                        }
                        .padding(.top, 100)
                        
                        Spacer()
                        
                        // Bottom row - switches and mode selector
                        HStack(spacing: 100) {
                            // Switches
                            VStack {
                                Toggle("HP", isOn: $switchValues["hp"]!)
                                    .toggleStyle(SwitchToggleStyle(tint: PMXPalette.mojoBlue))
                                    .onChange(of: switchValues["hp"]!) { _ in updateParams() }
                                
                                Toggle("TP", isOn: $switchValues["tp"]!)
                                    .toggleStyle(SwitchToggleStyle(tint: PMXPalette.mojoRed))
                                    .onChange(of: switchValues["tp"]!) { _ in updateParams() }
                            }
                            
                            // Interp mode selector
                            Picker("INTERP", selection: $interpMode) {
                                Text("Live HB 4×").tag(0)
                                Text("HQ Sinc 8×").tag(1)
                                Text("T-Spline 4×").tag(2)
                                Text("Adaptive").tag(3)
                                Text("Live+AI").tag(4)
                            }
                            .pickerStyle(SegmentedPickerStyle())
                            .frame(width: 400)
                            .onChange(of: interpMode) { _ in updateParams() }
                            
                            // LED status
                            HStack {
                                VStack(alignment: .leading, spacing: 5) {
                                    HStack {
                                        Circle()
                                            .fill(PMXPalette.mojoGreen)
                                            .frame(width: 10, height: 10)
                                        Text("ACTIVE")
                                            .font(.caption2)
                                            .foregroundColor(.white)
                                    }
                                    HStack {
                                        Circle()
                                            .fill(PMXPalette.mojoYellow)
                                            .frame(width: 10, height: 10)
                                        Text("OVERLOAD")
                                            .font(.caption2)
                                            .foregroundColor(.white)
                                    }
                                }
                            }
                        }
                        .padding(.bottom, 50)
                    }
                    .padding(.horizontal, 60)
                }
            }
            .padding(.vertical)
        }
        .onAppear {
            // Load faceplate layout from JSON
            faceplateLayout = FaceplateLayoutLoader.load(named: "faceplate_layout")
            if faceplateLayout == nil {
                // Create fallback layout
                faceplateLayout = FaceplateLayout(
                    faceplateAssetName: "heritage-audio-britStrip-GUI-01",
                    baseWidth: 2000,
                    baseHeight: 750,
                    rects: [:]
                )
            }
            
            // Initialize with current engine parameters
            loadCurrentParams()
        }
    }
    
    // Create a VU meter component
    private func vuMeter(level: CGFloat) -> some View {
        ZStack(alignment: .bottom) {
            // Background
            Rectangle()
                .fill(Color.black.opacity(0.8))
                .overlay(
                    Rectangle()
                        .stroke(Color.gray.opacity(0.5), lineWidth: 1)
                )
            
            // Level indicator
            Rectangle()
                .fill(
                    LinearGradient(
                        gradient: Gradient(colors: [.green, .yellow, .red]),
                        startPoint: .bottom,
                        endPoint: .top
                    )
                )
                .frame(height: 80 * level)
            
            // Scale markings
            VStack(spacing: 16) {
                ForEach([0.8, 0.6, 0.4, 0.2], id: \.self) { mark in
                    Divider()
                        .background(Color.white.opacity(0.5))
                        .frame(width: 50)
                        .offset(y: 80 * (mark - 0.5))
                }
                Spacer()
            }
        }
        .cornerRadius(4)
    }
    
    // Create a spectrum analyzer component
    private func spectrumDisplay(values: [Float]) -> some View {
        HStack(spacing: 2) {
            ForEach(0..<min(values.count, 40), id: \.self) { i in
                Rectangle()
                    .fill(barColor(for: values[i]))
                    .frame(width: 6, height: 80 * CGFloat(values[i]))
            }
        }
        .frame(maxWidth: .infinity, maxHeight: 80, alignment: .bottom)
        .background(Color.black.opacity(0.8))
        .cornerRadius(4)
    }
    
    // Determine bar color based on level
    private func barColor(for value: Float) -> Color {
        if value > 0.8 {
            return PMXPalette.mojoRed
        } else if value > 0.5 {
            return PMXPalette.mojoYellow
        } else {
            return PMXPalette.mojoGreen
        }
    }
    
    // Create a knob component
    private func knob(id: String, label: String) -> some View {
        let binding = Binding(
            get: { knobValues[id] ?? 0.5 },
            set: { knobValues[id] = $0; updateParams() }
        )
        
        return RoundKnob(
            value: binding,
            label: label,
            color: knobColor(for: id)
        )
        .frame(width: 70, height: 100)
    }
    
    // Determine knob color based on function
    private func knobColor(for id: String) -> Color {
        switch id {
        case "input":
            return PMXPalette.mojoRed
        case "drive", "saturation":
            return PMXPalette.mojoYellow
        case "character", "presence":
            return PMXPalette.mojoGreen
        case "output":
            return PMXPalette.mojoBlue
        default:
            return PMXPalette.mojoPurple
        }
    }
    
    // Update audio engine parameters based on UI state
    private func updateParams() {
        var params = [String: Any]()
        
        // Add all knob values
        for (key, value) in knobValues {
            params[key] = Float(value)
        }
        
        // Add switch values
        for (key, value) in switchValues {
            params[key] = value
        }
        
        // Add interp mode
        let interpModes = ["liveHB4x", "hqSinc8x", "transientSpline4x", "adaptive", "aiAnalogHook"]
        if interpMode < interpModes.count {
            params["interpMode"] = interpModes[interpMode]
        }
        
        // Update audio engine
        audioEngine.setParams(params)
    }
    
    // Load current parameters from audio engine
    private func loadCurrentParams() {
        // In a real implementation, we would retrieve the current parameters
        // from the audio engine and set the UI state accordingly
        updateParams()
    }
}
