import Cocoa
import SwiftUI

// Main entry point using NSApplicationMain pattern
@main
class AppDelegate: NSObject, NSApplicationDelegate {
    var window: NSWindow!
    
    func applicationDidFinishLaunching(_ notification: Notification) {
        // Create the SwiftUI view that provides the window contents.
        let contentView = ContentView()

        // Create the window and set the content view.
        window = NSWindow(
            contentRect: NSRect(x: 0, y: 0, width: 800, height: 600),
            styleMask: [.titled, .closable, .miniaturizable, .resizable],
            backing: .buffered,
            defer: false)
        window.title = "Pro GUI Integration Test"
        window.center()
        window.contentView = NSHostingView(rootView: contentView)
        window.makeKeyAndOrderFront(nil)
        
        // Ensure the app is activated
        NSApp.activate(ignoringOtherApps: true)
    }
}

struct ContentView: View {
    @State private var selectedTab = 0
    
    var body: some View {
        TabView(selection: $selectedTab) {
            EQView()
                .tabItem {
                    Text("EQ")
                }
                .tag(0)
            
            CompressorView()
                .tabItem {
                    Text("Compressor")
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
                    Text("EQ Component would appear here")
                        .foregroundColor(.secondary)
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
                    Text("Compressor Component would appear here")
                        .foregroundColor(.secondary)
                )
            
            VStack(spacing: 20) {
                HStack {
                    Text("Threshold: \(Int(threshold)) dB")
                        .frame(width: 100, alignment: .leading)
                    Slider(value: $threshold, in: -60...0, step: 1)
                }
                
                HStack {
                    Text("Ratio: \(ratio, specifier: "%.1f"):1")
                        .frame(width: 100, alignment: .leading)
                    Slider(value: $ratio, in: 1...20)
                }
                
                HStack {
                    Text("Attack: \(Int(attack)) ms")
                        .frame(width: 100, alignment: .leading)
                    Slider(value: $attack, in: 0.1...100)
                }
                
                HStack {
                    Text("Release: \(Int(release)) ms")
                        .frame(width: 100, alignment: .leading)
                    Slider(value: $release, in: 10...1000)
                }
            }
            .padding()
        }
    }
}
