import SwiftUI
import Combine

struct MoreMojoContainerView: View {
    // Engine reference
    @ObservedObject var engine: AudioEngine = AudioEngine.shared
    
    // UI state
    @State private var showTopLayer = true  // Toggle between Top/Bottom UI
    @State private var showSteal = false    // Steal Mojo sheet
    @State private var params = [String: Any]()   // Current parameters
    
    // UI animation state
    @State private var showingUIAnimation = false
    @State private var transitionProgress: CGFloat = 0
    
    var body: some View {
        ZStack {
            // Background
            PMXPalette.darkBackground.ignoresSafeArea()
            
            VStack(spacing: 0) {
                // Header with layer toggle
                HStack {
                    Text("MoreMojo")
                        .font(.headline)
                        .foregroundColor(PMXPalette.highlight)
                    
                    Spacer()
                    
                    Picker("Layer", selection: $showTopLayer) {
                        Text("Top").tag(true)
                        Text("Bottom").tag(false)
                    }
                    .pickerStyle(SegmentedPickerStyle())
                    .frame(width: 200)
                }
                .padding()
                .background(PMXPalette.mediumBackground)
                
                // Main content - layers
                ZStack {
                    // Bottom layer (Pro UI)
                    ProBritStripView()
                        .opacity(showTopLayer ? 0 : 1)
                    
                    // Top layer (Consumer UI)
                    TopConsumerView()
                        .opacity(showTopLayer ? 1 : 0)
                }
                .animation(.easeInOut(duration: 0.3), value: showTopLayer)
            }
        }
        .onAppear {
            // Initialize with default parameters
            initializeParams()
        }
        .onReceive(NotificationCenter.default.publisher(for: .showStealMojoSheet)) { _ in
            showSteal = true
        }
        .sheet(isPresented: $showSteal) {
            StealMojoPanel_SwiftOnly(onApply: { rec in
                // Handle the parameters received from Steal Mojo panel
                // Convert ProcessorParams to dictionary
                let paramsDict: [String: Any] = [
                    "drive": rec.drive,
                    "character": rec.character,
                    "saturation": rec.saturation,
                    "presence": rec.presence,
                    "warmth": Double(rec.warmth),
                    "output": rec.output,
                    "mix": rec.mix,
                    "mode": rec.mode,
                    "interpMode": String(describing: rec.interpMode)
                ]
                params = paramsDict
                engine.setParams(paramsDict)
                showSteal = false
            })
        }
    }
    
    // Initialize parameters with default values
    private func initializeParams() {
        params = [
            "drive": 0.5,
            "character": 0.5,
            "saturation": 0.45,
            "presence": 0.5,
            "warmth": 0.6,
            "mix": 1.0,
            "input": 0.5,
            "output": 0.0,
            "mode": 1
        ]
        
        // Apply initial parameters to engine
        engine.setParams(params)
    }
}
