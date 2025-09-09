import SwiftUI

struct ContentView: View {
    @EnvironmentObject var appState: AppState
    
    var body: some View {
        TabView {
            MainDashboardView()
                .tabItem {
                    Label("Dashboard", systemImage: "house")
                }
            
            AudioProcessingView()
                .tabItem {
                    Label("Audio", systemImage: "waveform")
                }
            
            MoreMojoContainer()
                .tabItem {
                    Label("More Mojo", systemImage: "dial.max.fill")
                }
            
            SettingsView()
                .tabItem {
                    Label("Settings", systemImage: "gear")
                }
        }
        .frame(minWidth: 800, minHeight: 600)
    }
}

struct MainDashboardView: View {
    @EnvironmentObject var appState: AppState
    
    var body: some View {
        VStack(spacing: 20) {
            Text("More Mojo Studio v1.2")
                .font(.largeTitle)
                .fontWeight(.bold)
            
            Image(systemName: "waveform.path")
                .resizable()
                .scaledToFit()
                .frame(width: 100, height: 100)
                .foregroundColor(.blue)
            
            Text("Audio Enhancement Suite")
                .font(.title2)
                .foregroundColor(.secondary)
            
            Divider()
            
            HStack(spacing: 30) {
                FeatureButton(title: "Processing", iconName: "slider.horizontal.3", color: .blue) {
                    // Action
                }
                
                FeatureButton(title: "AI Enhancement", iconName: "waveform.path.ecg", color: .green) {
                    appState.aiEnabled.toggle()
                }
                
                FeatureButton(title: "Presets", iconName: "square.stack.3d.up", color: .purple) {
                    // Action
                }
            }
            
            Spacer()
            
            Button(action: {
                _ = appState.processAudio(preset: appState.activePreset, useAI: appState.aiEnabled)
            }) {
                Text("Start Processing")
                    .font(.headline)
                    .foregroundColor(.white)
                    .padding()
                    .frame(width: 200)
                    .background(Color.blue)
                    .cornerRadius(10)
            }
            .disabled(appState.isProcessing)
            
            if appState.isProcessing {
                ProgressView(value: appState.processingProgress)
                    .frame(maxWidth: 300)
                Text(appState.processingMessage)
                    .font(.caption)
                    .foregroundColor(.secondary)
            }
        }
        .padding()
    }
}

struct SettingsView: View {
    @EnvironmentObject var appState: AppState
    
    var body: some View {
        Form {
            Section(header: Text("Audio Settings")) {
                Picker("Input Device", selection: $appState.selectedInputDevice) {
                    ForEach(appState.availableInputDevices, id: \.self) { device in
                        Text(device).tag(device)
                    }
                }
                
                Picker("Output Device", selection: $appState.selectedOutputDevice) {
                    ForEach(appState.availableOutputDevices, id: \.self) { device in
                        Text(device).tag(device)
                    }
                }
                
                Toggle("Enable AI Processing", isOn: $appState.aiEnabled)
            }
        }
        .padding()
    }
}

struct FeatureButton: View {
    var title: String
    var iconName: String
    var color: Color
    var action: () -> Void
    
    var body: some View {
        Button(action: action) {
            VStack {
                Image(systemName: iconName)
                    .resizable()
                    .scaledToFit()
                    .frame(width: 40, height: 40)
                Text(title)
            }
            .padding()
            .background(color.opacity(0.1))
            .cornerRadius(10)
            .foregroundColor(.primary)
        }
        .buttonStyle(PlainButtonStyle())
    }
}
