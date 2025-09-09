import SwiftUI

@main struct MoreMojoStudioApp: App {
    @StateObject var appState = AppState()
    
    init() {
        // Set up necessary environment variables
        let parentDir = URL(fileURLWithPath: FileManager.default.currentDirectoryPath)
            .deletingLastPathComponent().path
        setenv("PYTHONPATH", parentDir, 1)
        setenv("MOREMOJO_USE_MPS", "1", 1)
        
        print("More Mojo Studio v1.2 initializing...")
        print("PYTHONPATH: \(ProcessInfo.processInfo.environment["PYTHONPATH"] ?? "Not set")")
        print("MPS Acceleration: Enabled for M1/M2 Mac")
        print("Metal HQ Engine: Enabled")
        print("AI Enhancement: Checking for model...")
        
        // Initialize core components
        AudioEngine.shared.setupAudioSystem()
    }
    
    var body: some Scene {
        WindowGroup {
            ContentView()
                .environmentObject(appState)
                .frame(minWidth: 1024, minHeight: 768)
                .onAppear {
                    appState.syncWithHardware()
                }
        }
        .windowStyle(HiddenTitleBarWindowStyle())
    }
}
