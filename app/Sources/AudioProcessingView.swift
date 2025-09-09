import SwiftUI

struct AudioProcessingView: View {
    @EnvironmentObject var appState: AppState
    @State private var isProcessing = false
    
    var body: some View {
        VStack(spacing: 20) {
            Text("Audio Processing")
                .font(.largeTitle)
                .padding(.top)
            
            // Waveform visualization (simplified)
            Rectangle()
                .fill(LinearGradient(
                    gradient: Gradient(colors: [.blue, .purple]),
                    startPoint: .leading,
                    endPoint: .trailing
                ))
                .frame(height: 100)
                .opacity(0.5)
                .cornerRadius(8)
                .padding()
            
            // Controls
            HStack(spacing: 30) {
                Picker("Preset", selection: $appState.activePreset) {
                    ForEach(appState.presets, id: \.self) { preset in
                        Text(preset).tag(preset)
                    }
                }
                .frame(width: 150)
                
                Toggle("AI Enhancement", isOn: $appState.aiEnabled)
            }
            .padding()
            
            // Process button
            Button(action: {
                withAnimation {
                    _ = appState.processAudio(preset: appState.activePreset, useAI: appState.aiEnabled)
                    isProcessing = true
                    // Simulate completion
                    DispatchQueue.main.asyncAfter(deadline: .now() + 3) {
                        isProcessing = false
                    }
                }
            }) {
                Text(isProcessing ? "Processing..." : "Process Audio")
                    .padding()
                    .frame(width: 200)
                    .background(Color.blue)
                    .foregroundColor(.white)
                    .cornerRadius(10)
            }
            .disabled(isProcessing)
            
            if isProcessing {
                ProgressView()
                    .padding()
            }
            
            Spacer()
            
            // File management section
            VStack(alignment: .leading) {
                Text("Recent Files")
                    .font(.headline)
                
                List {
                    ForEach(appState.recentAudioFiles, id: \.self) { fileName in
                        HStack {
                            Image(systemName: "music.note")
                            Text(fileName)
                            Spacer()
                            Button(action: {
                                appState.currentAudioFile = fileName
                            }) {
                                Text("Load")
                                    .font(.caption)
                                    .padding(.horizontal, 8)
                                    .padding(.vertical, 4)
                                    .background(Color.blue.opacity(0.7))
                                    .foregroundColor(.white)
                                    .cornerRadius(4)
                            }
                        }
                    }
                }
                .frame(height: 150)
                
                Button(action: {
                    appState.openAudioFile()
                }) {
                    HStack {
                        Image(systemName: "folder")
                        Text("Open Audio File")
                    }
                }
                .padding()
            }
            .padding()
        }
        .padding()
    }
}
