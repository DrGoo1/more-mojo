import Cocoa
import SwiftUI
import OSLog

@main
struct ProGUITestApp: App {
    var body: some Scene {
        WindowGroup {
            ContentView()
                .frame(minWidth: 800, minHeight: 600)
        }
    }
}

// MARK: - Standalone demo window (bypass SwiftUI embedding) and file diagnostics
private var proguiStandaloneWindow: NSWindow?
private var proguiStandaloneBridge: ProGUIBridge?

@discardableResult
private func writeSwiftDiag(_ message: String) -> Bool {
    let path = "/tmp/progui_demo_swift.log"
    let line = "[Swift] \(Date()) \(message)\n"
    let url = URL(fileURLWithPath: path)
    if FileManager.default.fileExists(atPath: path) {
        do {
            let h = try FileHandle(forWritingTo: url)
            defer { try? h.close() }
            h.seekToEndOfFile()
            if let data = line.data(using: .utf8) { try h.write(contentsOf: data) }
            return true
        } catch { return false }
    } else {
        return (try? line.write(to: url, atomically: true, encoding: .utf8)) != nil
    }
}

@MainActor
private func openStandaloneDemoWindow() {
    // If we already have a window, just bring it to front and return
    if let w = proguiStandaloneWindow {
        w.makeKeyAndOrderFront(nil)
        NSApp.activate(ignoringOtherApps: true)
        _ = writeSwiftDiag("[Standalone] Reusing existing window; bringing to front")
        return
    }
    let frame = NSRect(x: 120, y: 120, width: 720, height: 480)
    let w = NSWindow(contentRect: frame,
                     styleMask: [.titled, .closable, .miniaturizable, .resizable],
                     backing: .buffered,
                     defer: false)
    w.title = "ProGUI Standalone Demo"
    w.isReleasedWhenClosed = false // prevent dealloc to avoid dangling pointer
    NSLog("[Standalone] Creating window and container")
    _ = writeSwiftDiag("[Standalone] Creating window and container")
    // Assign early to ensure we keep a strong reference
    proguiStandaloneWindow = w
    // Clear globals when window closes
    NotificationCenter.default.addObserver(forName: NSWindow.willCloseNotification, object: w, queue: .main) { _ in
        _ = writeSwiftDiag("[Standalone] Window will close — clearing retained bridge/window")
        proguiStandaloneBridge = nil
        proguiStandaloneWindow = nil
    }
    let container = NSView(frame: NSRect(x: 0, y: 0, width: 720, height: 480))
    container.wantsLayer = true
    container.layer?.backgroundColor = NSColor.systemRed.withAlphaComponent(0.35).cgColor
    // Visible header label so we always see something in the window
    let header = NSTextField(labelWithString: "Standalone Container — expecting embedded JUCE demo")
    header.textColor = .systemYellow
    header.font = .boldSystemFont(ofSize: 13)
    header.alignment = .center
    header.frame = NSRect(x: 10, y: container.bounds.height - 30, width: container.bounds.width - 20, height: 20)
    header.autoresizingMask = [.width, .minYMargin]
    container.addSubview(header)
    // Add a visible border box
    // Layer-based border (replace deprecated NSBox borderType)
    container.wantsLayer = true
    if let layer = container.layer {
        layer.borderColor = NSColor.systemBlue.withAlphaComponent(0.6).cgColor
        layer.borderWidth = 2.0
        layer.cornerRadius = 6.0
        layer.masksToBounds = true
    }
    // Big center label for absolute visibility
    let big = NSTextField(labelWithString: "STANDALONE DEMO WINDOW")
    big.textColor = .white
    big.font = .boldSystemFont(ofSize: 24)
    big.alignment = .center
    big.frame = NSRect(x: 10, y: container.bounds.midY - 16, width: container.bounds.width - 20, height: 32)
    big.autoresizingMask = [.width, .minYMargin, .maxYMargin]
    container.addSubview(big)
    NSLog("[Standalone] Added visible header/border/big-label")
    _ = writeSwiftDiag("[Standalone] Added visible header/border/big-label")
    let bridge = ProGUIBridge(type: .demo, frame: container.bounds)
    proguiStandaloneBridge = bridge // retain so it doesn't deinit
    let juceView = bridge.view
    juceView.frame = container.bounds
    juceView.autoresizingMask = [.width, .height]
    // Make the JUCE container view obviously visible
    juceView.wantsLayer = true
    juceView.layer?.backgroundColor = NSColor.systemGreen.withAlphaComponent(0.35).cgColor
    // Layer-based border for JUCE container
    if let jl = juceView.layer {
        jl.borderColor = NSColor.systemGreen.withAlphaComponent(0.6).cgColor
        jl.borderWidth = 2.0
        jl.cornerRadius = 6.0
        jl.masksToBounds = true
    }
    let inside = NSTextField(labelWithString: "INSIDE JUCE CONTAINER (Swift layer) — expecting JUCE demo below")
    inside.textColor = .white
    inside.font = .boldSystemFont(ofSize: 18)
    inside.alignment = .center
    inside.frame = NSRect(x: 10, y: juceView.bounds.midY - 14, width: juceView.bounds.width - 20, height: 28)
    inside.autoresizingMask = [.width, .minYMargin, .maxYMargin]
    juceView.addSubview(inside)
    container.addSubview(juceView)
    w.contentView = container
    w.center()
    w.makeKeyAndOrderFront(nil)
    NSApp.activate(ignoringOtherApps: true)
    // Basic setup on retained bridge
    bridge.setOverlayVisible(true) // show overlay to confirm attachment/size
    bridge.setTestSignalEnabled(true)
    let b = container.bounds.size
    bridge.resize(width: b.width, height: b.height)
    NSLog("[Standalone] Opened window; container subviews=\(container.subviews.count); retained bridge")
    _ = writeSwiftDiag("[Standalone] Opened ProGUI Standalone Demo window; container subviews=\(container.subviews.count); retained bridge")
    // Alert removed for stability; rely on on-screen labels + logs instead
}

struct ContentView: View {
    @State private var selectedView = 2
    @State private var band1Gain: Float = 0.0
    @State private var band1Freq: Float = 1000.0
    @State private var band1Q: Float = 1.0
    @State private var band2Gain: Float = 0.0
    @State private var band2Freq: Float = 300.0
    @State private var band2Q: Float = 1.0
    @State private var band3Gain: Float = 0.0
    @State private var band3Freq: Float = 1000.0
    @State private var band3Q: Float = 1.0
    @State private var band4Gain: Float = 0.0
    @State private var band4Freq: Float = 3000.0
    @State private var band4Q: Float = 1.0
    @State private var band5Gain: Float = 0.0
    @State private var band5Freq: Float = 10000.0
    @State private var band5Q: Float = 0.7
    @State private var threshold: Float = -20.0
    @State private var ratio: Float = 4.0
    @State private var attack: Float = 10.0
    @State private var release: Float = 100.0
    @State private var knee: Float = 6.0
    @State private var makeupGain: Float = 0.0
    @State private var overlayVisible: Bool = true
    @State private var testSignalEnabled: Bool = true
    @State private var showSwiftControls: Bool = false
    @State private var testBuffer: (left: [Float], right: [Float])? = nil
    
    var body: some View {
        VStack {
            Picker("Component", selection: $selectedView) {
                Text("EQ").tag(0)
                Text("Compressor").tag(1)
                Text("Demo").tag(2)
            }
            .pickerStyle(SegmentedPickerStyle())
            .padding()
            
            if selectedView == 0 {
                VStack {
                    Text("EQ Controls").font(.headline)
                    ProGUIEQView(band1Gain: $band1Gain, 
                                band1Freq: $band1Freq, 
                                band1Q: $band1Q,
                                band2Gain: $band2Gain,
                                band2Freq: $band2Freq,
                                band2Q: $band2Q,
                                band3Gain: $band3Gain,
                                band3Freq: $band3Freq,
                                band3Q: $band3Q,
                                band4Gain: $band4Gain,
                                band4Freq: $band4Freq,
                                band4Q: $band4Q,
                                band5Gain: $band5Gain,
                                band5Freq: $band5Freq,
                                band5Q: $band5Q,
                                testBuffer: $testBuffer,
                                overlayVisible: $overlayVisible,
                                testSignalEnabled: $testSignalEnabled)
                    .frame(height: 400)
                    
                    DisclosureGroup("Swift Controls", isExpanded: $showSwiftControls) {
                    HStack {
                        VStack {
                            Text("Band 1 Gain: \(band1Gain, specifier: "%.1f") dB")
                            Slider(value: $band1Gain, in: -24...24)
                        }
                        VStack {
                            Text("Band 1 Freq: \(band1Freq, specifier: "%.0f") Hz")
                            Slider(value: $band1Freq, in: 20...20000)
                        }
                        VStack {
                            Text("Band 1 Q: \(band1Q, specifier: "%.2f")")
                            Slider(value: $band1Q, in: 0.1...10)
                        }
                        VStack {
                            Text(" ")
                            Button("Reset B1") {
                                band1Gain = 0.0
                                band1Freq = 1000.0
                                band1Q = 1.0
                            }
                        }
                    }
                    .padding()
                    
                    // Bands 2–5 compact controls
                    VStack(spacing: 8) {
                        HStack {
                            VStack { Text("B2 Gain: \(band2Gain, specifier: "%.1f") dB"); Slider(value: $band2Gain, in: -24...24) }
                            VStack { Text("B2 Freq: \(band2Freq, specifier: "%.0f") Hz"); Slider(value: $band2Freq, in: 20...20000) }
                            VStack { Text("B2 Q: \(band2Q, specifier: "%.2f")"); Slider(value: $band2Q, in: 0.1...10) }
                            VStack { Text(" "); Button("Reset B2") { band2Gain = 0.0; band2Freq = 300.0; band2Q = 1.0 } }
                        }
                        HStack {
                            VStack { Text("B3 Gain: \(band3Gain, specifier: "%.1f") dB"); Slider(value: $band3Gain, in: -24...24) }
                            VStack { Text("B3 Freq: \(band3Freq, specifier: "%.0f") Hz"); Slider(value: $band3Freq, in: 20...20000) }
                            VStack { Text("B3 Q: \(band3Q, specifier: "%.2f")"); Slider(value: $band3Q, in: 0.1...10) }
                            VStack { Text(" "); Button("Reset B3") { band3Gain = 0.0; band3Freq = 1000.0; band3Q = 1.0 } }
                        }
                        HStack {
                            VStack { Text("B4 Gain: \(band4Gain, specifier: "%.1f") dB"); Slider(value: $band4Gain, in: -24...24) }
                            VStack { Text("B4 Freq: \(band4Freq, specifier: "%.0f") Hz"); Slider(value: $band4Freq, in: 20...20000) }
                            VStack { Text("B4 Q: \(band4Q, specifier: "%.2f")"); Slider(value: $band4Q, in: 0.1...10) }
                            VStack { Text(" "); Button("Reset B4") { band4Gain = 0.0; band4Freq = 3000.0; band4Q = 1.0 } }
                        }
                        HStack {
                            VStack { Text("B5 Gain: \(band5Gain, specifier: "%.1f") dB"); Slider(value: $band5Gain, in: -24...24) }
                            VStack { Text("B5 Freq: \(band5Freq, specifier: "%.0f") Hz"); Slider(value: $band5Freq, in: 20...20000) }
                            VStack { Text("B5 Q: \(band5Q, specifier: "%.2f")"); Slider(value: $band5Q, in: 0.1...10) }
                            VStack { Text(" "); Button("Reset B5") { band5Gain = 0.0; band5Freq = 10000.0; band5Q = 0.7 } }
                        }
                        HStack {
                            Button("Reset All EQ") {
                                band1Gain = 0.0; band1Freq = 1000.0; band1Q = 1.0
                                band2Gain = 0.0; band2Freq = 300.0; band2Q = 1.0
                                band3Gain = 0.0; band3Freq = 1000.0; band3Q = 1.0
                                band4Gain = 0.0; band4Freq = 3000.0; band4Q = 1.0
                            }
                        }
                    }
                    } // end DisclosureGroup
                }
            } else if selectedView == 1 {
                VStack {
                    Text("Compressor Controls").font(.headline)
                    ProGUICompressorView(threshold: $threshold,
                                       ratio: $ratio,
                                       attack: $attack,
                                       release: $release,
                                       knee: $knee,
                                       makeupGain: $makeupGain,
                                       testBuffer: $testBuffer,
                                       overlayVisible: $overlayVisible,
                                       testSignalEnabled: $testSignalEnabled)
                    .frame(height: 400)
                    
                    DisclosureGroup("Swift Controls", isExpanded: $showSwiftControls) {
                        HStack {
                            VStack {
                                Text("Threshold: \(threshold, specifier: "%.1f") dB")
                                Slider(value: $threshold, in: -60...0)
                            }
                            VStack {
                                Text("Ratio: \(ratio, specifier: "%.1f"):1")
                                Slider(value: $ratio, in: 1...20)
                            }
                        }
                        .padding(.bottom, 8)
                        HStack {
                            VStack {
                                Text("Attack: \(attack, specifier: "%.1f") ms")
                                Slider(value: $attack, in: 0.1...100)
                            }
                            VStack {
                                Text("Release: \(release, specifier: "%.0f") ms")
                                Slider(value: $release, in: 10...1000)
                            }
                        }
                        .padding(.bottom, 8)
                        HStack {
                            VStack {
                                Text("Knee: \(knee, specifier: "%.1f") dB")
                                Slider(value: $knee, in: 0...24)
                            }
                            VStack {
                                Text("Makeup: \(makeupGain, specifier: "%.1f") dB")
                                Slider(value: $makeupGain, in: 0...24)
                            }
                        }
                    }
                    .padding()
                }
            } else {
                VStack(alignment: .leading, spacing: 8) {
                    Text("UI Demo Components").font(.headline)
                    // Always-visible SwiftUI banner for diagnostics
                    ZStack {
                        Rectangle().fill(Color(NSColor(calibratedWhite: 0.15, alpha: 1.0)))
                        Text("SwiftUI Banner — Demo Area Below")
                            .foregroundColor(.yellow)
                            .font(.system(size: 13, weight: .bold))
                    }
                    .frame(height: 30)
                    .cornerRadius(4)

                    // JUCE demo view in a bordered container
                    ProGUIDemoView(testBuffer: $testBuffer,
                                   overlayVisible: $overlayVisible,
                                   testSignalEnabled: $testSignalEnabled)
                        .frame(height: 400)
                        .border(Color.blue.opacity(0.6), width: 1)

                    HStack {
                        Button("Open Standalone Demo Window") {
                            openStandaloneDemoWindow()
                        }
                        .keyboardShortcut("o", modifiers: [.command, .shift])
                    }
                }
            }
            
            Button("Generate Test Audio") {
                generateTestAudio()
            }
            .padding()
            HStack {
                Toggle("Consumer Overlay", isOn: $overlayVisible)
                Toggle("Test Signal", isOn: $testSignalEnabled)
            }
            .padding(.horizontal)
        }
        .padding()
        .onAppear {
            _ = writeSwiftDiag("[ContentView] onAppear -> opening standalone demo window")
            openStandaloneDemoWindow()
        }
    }
    
    func generateTestAudio() {
        // Create a simple sine wave for testing
        let sampleRate = 44100
        let duration = 2 // 2 seconds
        var leftChannel = [Float]()
        var rightChannel = [Float]()
        
        for i in 0..<(sampleRate * duration) {
            let time = Float(i) / Float(sampleRate)
            let value = sin(2 * Float.pi * 440 * time) * 0.5 // 440Hz A note
            
            leftChannel.append(value)
            rightChannel.append(value)
        }
        
        // Hand the buffer to whichever view is active via binding
        testBuffer = (left: leftChannel, right: rightChannel)
    }
}

// EQ View implementation
struct ProGUIEQView: NSViewRepresentable {
    @Binding var band1Gain: Float
    @Binding var band1Freq: Float
    @Binding var band1Q: Float
    @Binding var band2Gain: Float
    @Binding var band2Freq: Float
    @Binding var band2Q: Float
    @Binding var band3Gain: Float
    @Binding var band3Freq: Float
    @Binding var band3Q: Float
    @Binding var band4Gain: Float
    @Binding var band4Freq: Float
    @Binding var band4Q: Float
    @Binding var band5Gain: Float
    @Binding var band5Freq: Float
    @Binding var band5Q: Float
    @Binding var testBuffer: (left: [Float], right: [Float])?
    @Binding var overlayVisible: Bool
    @Binding var testSignalEnabled: Bool
    
    var audioBuffer: (left: [Float], right: [Float])?
    
    func makeNSView(context: Context) -> NSView {
        // Create the ProGUIBridge
        let bridge = ProGUIBridge(type: .eq, frame: NSRect(x: 0, y: 0, width: 600, height: 400))
        
        // Store the bridge in coordinator
        context.coordinator.bridge = bridge
        
        // Set initial parameter values
        bridge.setEQBandGain(band: 1, gainDB: band1Gain)
        bridge.setEQBandFrequency(band: 1, frequency: band1Freq)
        bridge.setEQBandQ(band: 1, q: band1Q)
        bridge.setEQBandGain(band: 2, gainDB: band2Gain)
        bridge.setEQBandFrequency(band: 2, frequency: band2Freq)
        bridge.setEQBandQ(band: 2, q: band2Q)
        bridge.setEQBandGain(band: 3, gainDB: band3Gain)
        bridge.setEQBandFrequency(band: 3, frequency: band3Freq)
        bridge.setEQBandQ(band: 3, q: band3Q)
        bridge.setEQBandGain(band: 4, gainDB: band4Gain)
        bridge.setEQBandFrequency(band: 4, frequency: band4Freq)
        bridge.setEQBandQ(band: 4, q: band4Q)
        bridge.setEQBandGain(band: 5, gainDB: band5Gain)
        bridge.setEQBandFrequency(band: 5, frequency: band5Freq)
        bridge.setEQBandQ(band: 5, q: band5Q)
        bridge.setOverlayVisible(overlayVisible)
        bridge.setTestSignalEnabled(testSignalEnabled)
        
        return bridge.view
    }
    
    func updateNSView(_ nsView: NSView, context: Context) {
        guard let bridge = context.coordinator.bridge else { return }
        // Ensure the embedded JUCE view matches the Swift container size
        let size = nsView.bounds.size
        bridge.resize(width: size.width, height: size.height)
        
        // Update parameters if they've changed
        let currentGain = bridge.getEQBandGain(band: 1)
        if currentGain != band1Gain {
            bridge.setEQBandGain(band: 1, gainDB: band1Gain)
        }
        
        let currentFreq = bridge.getEQBandFrequency(band: 1)
        if currentFreq != band1Freq {
            bridge.setEQBandFrequency(band: 1, frequency: band1Freq)
        }
        
        let currentQ = bridge.getEQBandQ(band: 1)
        if currentQ != band1Q {
            bridge.setEQBandQ(band: 1, q: band1Q)
        }
        // Bands 2–5
        if bridge.getEQBandGain(band: 2) != band2Gain { bridge.setEQBandGain(band: 2, gainDB: band2Gain) }
        if bridge.getEQBandFrequency(band: 2) != band2Freq { bridge.setEQBandFrequency(band: 2, frequency: band2Freq) }
        if bridge.getEQBandQ(band: 2) != band2Q { bridge.setEQBandQ(band: 2, q: band2Q) }
        if bridge.getEQBandGain(band: 3) != band3Gain { bridge.setEQBandGain(band: 3, gainDB: band3Gain) }
        if bridge.getEQBandFrequency(band: 3) != band3Freq { bridge.setEQBandFrequency(band: 3, frequency: band3Freq) }
        if bridge.getEQBandQ(band: 3) != band3Q { bridge.setEQBandQ(band: 3, q: band3Q) }
        if bridge.getEQBandGain(band: 4) != band4Gain { bridge.setEQBandGain(band: 4, gainDB: band4Gain) }
        if bridge.getEQBandFrequency(band: 4) != band4Freq { bridge.setEQBandFrequency(band: 4, frequency: band4Freq) }
        if bridge.getEQBandQ(band: 4) != band4Q { bridge.setEQBandQ(band: 4, q: band4Q) }
        if bridge.getEQBandGain(band: 5) != band5Gain { bridge.setEQBandGain(band: 5, gainDB: band5Gain) }
        if bridge.getEQBandFrequency(band: 5) != band5Freq { bridge.setEQBandFrequency(band: 5, frequency: band5Freq) }
        if bridge.getEQBandQ(band: 5) != band5Q { bridge.setEQBandQ(band: 5, q: band5Q) }
        bridge.setOverlayVisible(overlayVisible)
        bridge.setTestSignalEnabled(testSignalEnabled)
        
        // Process audio if provided via binding
        if let buffer = testBuffer {
            bridge.processAudio(leftChannel: buffer.left, rightChannel: buffer.right)
            // clear after consumption
            testBuffer = nil
        }
    }
    
    func makeCoordinator() -> Coordinator {
        Coordinator(self)
    }
    
    class Coordinator {
        var parent: ProGUIEQView
        var bridge: ProGUIBridge?
        
        init(_ parent: ProGUIEQView) {
            self.parent = parent
        }
    }
}

// Compressor View implementation
struct ProGUICompressorView: NSViewRepresentable {
    @Binding var threshold: Float
    @Binding var ratio: Float
    @Binding var attack: Float
    @Binding var release: Float
    @Binding var knee: Float
    @Binding var makeupGain: Float
    @Binding var testBuffer: (left: [Float], right: [Float])?
    @Binding var overlayVisible: Bool
    @Binding var testSignalEnabled: Bool
    
    var audioBuffer: (left: [Float], right: [Float])?
    
    func makeNSView(context: Context) -> NSView {
        // Create the ProGUIBridge
        let bridge = ProGUIBridge(type: .compressor, frame: NSRect(x: 0, y: 0, width: 600, height: 400))
        
        // Store the bridge in coordinator
        context.coordinator.bridge = bridge
        
        // Set initial parameter values
        bridge.setCompressorThreshold(threshold)
        bridge.setCompressorRatio(ratio)
        bridge.setCompressorAttack(attack)
        bridge.setCompressorRelease(release)
        bridge.setCompressorKnee(knee)
        bridge.setCompressorMakeupGain(makeupGain)
        bridge.setOverlayVisible(overlayVisible)
        bridge.setTestSignalEnabled(testSignalEnabled)
        
        return bridge.view
    }
    
    func updateNSView(_ nsView: NSView, context: Context) {
        guard let bridge = context.coordinator.bridge else { return }
        // Ensure the embedded JUCE view matches the Swift container size
        let size = nsView.bounds.size
        bridge.resize(width: size.width, height: size.height)
        
        // Update parameters if they've changed
        let currentThreshold = bridge.getCompressorThreshold()
        if currentThreshold != threshold {
            bridge.setCompressorThreshold(threshold)
        }
        
        let currentRatio = bridge.getCompressorRatio()
        if currentRatio != ratio {
            bridge.setCompressorRatio(ratio)
        }
        
        let currentAttack = bridge.getCompressorAttack()
        if currentAttack != attack {
            bridge.setCompressorAttack(attack)
        }
        
        let currentRelease = bridge.getCompressorRelease()
        if currentRelease != release {
            bridge.setCompressorRelease(release)
        }
        
        let currentKnee = bridge.getCompressorKnee()
        if currentKnee != knee {
            bridge.setCompressorKnee(knee)
        }
        
        let currentMakeupGain = bridge.getCompressorMakeupGain()
        if currentMakeupGain != makeupGain {
            bridge.setCompressorMakeupGain(makeupGain)
        }
        bridge.setOverlayVisible(overlayVisible)
        bridge.setTestSignalEnabled(testSignalEnabled)
        
        // Process audio if provided via binding
        if let buffer = testBuffer {
            bridge.processAudio(leftChannel: buffer.left, rightChannel: buffer.right)
            testBuffer = nil
        }
    }
    
    func makeCoordinator() -> Coordinator {
        Coordinator(self)
    }
    
    class Coordinator {
        var parent: ProGUICompressorView
        var bridge: ProGUIBridge?
        
        init(_ parent: ProGUICompressorView) {
            self.parent = parent
        }
    }
}

// Demo View implementation showing 3D knob/slider/button/switch via JUCE component
struct ProGUIDemoView: NSViewRepresentable {
    @Binding var testBuffer: (left: [Float], right: [Float])?
    @Binding var overlayVisible: Bool
    @Binding var testSignalEnabled: Bool
    
    func makeNSView(context: Context) -> NSView {
        let logger = Logger(subsystem: "com.moremojo.progui.testapp", category: "Demo")
        logger.info("[ProGUIDemoView] makeNSView starting")
        // Visible container so user always sees something even if JUCE fails to attach
        let container = NSView(frame: NSRect(x: 0, y: 0, width: 600, height: 400))
        container.wantsLayer = true
        container.layer?.backgroundColor = NSColor(calibratedWhite: 0.12, alpha: 1.0).cgColor

        let label = NSTextField(labelWithString: "Demo Container — embedding JUCE view...")
        label.textColor = .systemYellow
        label.font = .boldSystemFont(ofSize: 13)
        label.alignment = .center
        label.frame = NSRect(x: 8, y: 8, width: 580, height: 20)
        container.addSubview(label)

        // Create the bridge and try to embed its view
        let bridge = ProGUIBridge(type: .demo, frame: container.bounds)
        context.coordinator.bridge = bridge
        bridge.setOverlayVisible(overlayVisible)
        bridge.setTestSignalEnabled(testSignalEnabled)

        let juceView = bridge.view
        juceView.frame = container.bounds
        juceView.autoresizingMask = [.width, .height]
        container.addSubview(juceView)

        logger.info("[ProGUIDemoView] makeNSView finished. Container subviews: \(container.subviews.count)")
        return container
    }
    
    func updateNSView(_ nsView: NSView, context: Context) {
        let logger = Logger(subsystem: "com.moremojo.progui.testapp", category: "Demo")
        logger.info("[ProGUIDemoView] updateNSView. Bounds=\(String(describing: nsView.bounds)) subviews=\(nsView.subviews.count)")
        guard let bridge = context.coordinator.bridge else { return }
        let size = nsView.bounds.size
        bridge.resize(width: size.width, height: size.height)
        bridge.setOverlayVisible(overlayVisible)
        bridge.setTestSignalEnabled(testSignalEnabled)
        if let buffer = testBuffer {
            bridge.processAudio(leftChannel: buffer.left, rightChannel: buffer.right)
            testBuffer = nil
        }
    }
    
    func makeCoordinator() -> Coordinator { Coordinator(self) }
    
    class Coordinator {
        var parent: ProGUIDemoView
        var bridge: ProGUIBridge?
        init(_ parent: ProGUIDemoView) { self.parent = parent }
    }
}
