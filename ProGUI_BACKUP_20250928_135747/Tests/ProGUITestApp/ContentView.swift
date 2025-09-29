import SwiftUI

struct ContentView: View {
    @State private var selectedTab = 0
    
    var body: some View {
        TabView(selection: $selectedTab) {
            EQView()
                .tabItem {
                    Label("EQ", systemImage: "waveform")
                }
                .tag(0)
            
            CompressorView()
                .tabItem {
                    Label("Compressor", systemImage: "slider.horizontal.3")
                }
                .tag(1)
        }
        .padding()
        .frame(minWidth: 800, minHeight: 600)
    }
}

struct EQView: View {
    @State private var band1Gain: Float = 0
    @State private var band1Freq: Float = 1000
    @State private var band1Q: Float = 1
    
    var body: some View {
        VStack {
            Text("EQ Component Integration Test")
                .font(.headline)
                .padding()
            
            // This would be where the ProGUIBridge EQ component would be displayed
            Rectangle()
                .fill(Color.gray.opacity(0.2))
                .frame(height: 300)
                .overlay(
                    Text("EQ Component Visualization")
                        .font(.title)
                        .foregroundColor(.primary)
                )
            
            VStack(spacing: 20) {
                HStack {
                    Text("Gain: \(Int(band1Gain)) dB")
                        .frame(width: 100, alignment: .leading)
                    Slider(value: $band1Gain, in: -24...24, step: 1)
                }
                
                HStack {
                    Text("Freq: \(Int(band1Freq)) Hz")
                        .frame(width: 100, alignment: .leading)
                    Slider(value: $band1Freq, in: 20...20000)
                }
                
                HStack {
                    Text("Q: \(band1Q, specifier: "%.1f")")
                        .frame(width: 100, alignment: .leading)
                    Slider(value: $band1Q, in: 0.1...10)
                }
            }
            .padding()
        }
    }
}

struct CompressorView: View {
    @State private var threshold: Float = -20
    @State private var ratio: Float = 4
    @State private var attack: Float = 10
    @State private var release: Float = 100
    
    var body: some View {
        VStack {
            Text("Compressor Component Integration Test")
                .font(.headline)
                .padding()
            
            // This would be where the ProGUIBridge Compressor component would be displayed
            Rectangle()
                .fill(Color.gray.opacity(0.2))
                .frame(height: 300)
                .overlay(
                    Text("Compressor Visualization")
                        .font(.title)
                        .foregroundColor(.primary)
                )
            
            VStack(spacing: 20) {
                HStack {
                    Text("Threshold: \(Int(threshold)) dB")
                        .frame(width: 120, alignment: .leading)
                    Slider(value: $threshold, in: -60...0, step: 1)
                }
                
                HStack {
                    Text("Ratio: \(ratio, specifier: "%.1f"):1")
                        .frame(width: 120, alignment: .leading)
                    Slider(value: $ratio, in: 1...20)
                }
                
                HStack {
                    Text("Attack: \(Int(attack)) ms")
                        .frame(width: 120, alignment: .leading)
                    Slider(value: $attack, in: 0.1...100)
                }
                
                HStack {
                    Text("Release: \(Int(release)) ms")
                        .frame(width: 120, alignment: .leading)
                    Slider(value: $release, in: 10...1000)
                }
            }
            .padding()
        }
    }
}
