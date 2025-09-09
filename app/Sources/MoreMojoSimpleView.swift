import SwiftUI

/// Simple view with basic controls for More Mojo
struct MoreMojoSimpleView: View {
    @Binding var mojoSelectorIndex: Int
    @Binding var typeSelectorIndex: Int
    
    var onAdvanced: () -> Void
    var onProcess: () -> Void
    
    // State for showing Steal That Mojo panel
    @State private var showStealMojo = false
    
    // Reference to the audio engine for applying params
    @EnvironmentObject var engine: AudioEngine
    
    // Audio type options
    private let audioTypes = ["Drums", "Bass", "Guitar", "Vocals", "Mix"]
    
    // Mojo level options
    private let mojoLevels = ["Mojo", "More Mojo", "Most Mojo"]
    
    var body: some View {
        VStack(spacing: 30) {
            // Title and advanced button
            HStack {
                Text("More Mojo")
                    .font(.title)
                    .fontWeight(.bold)
                
                Spacer()
                
                Button("Steal That Mojo...") {
                    showStealMojo = true
                }
                .padding(.horizontal, 15)
                .padding(.vertical, 8)
                .background(Color.blue.opacity(0.7))
                .foregroundColor(.white)
                .cornerRadius(8)
                
                Button("Advanced") {
                    onAdvanced()
                }
                .padding(.horizontal, 15)
                .padding(.vertical, 8)
                .background(Color.gray.opacity(0.2))
                .cornerRadius(8)
            }
            
            // Audio type selection
            VStack(alignment: .leading, spacing: 10) {
                Text("Audio Type")
                    .font(.headline)
                
                HStack(spacing: 10) {
                    ForEach(0..<audioTypes.count, id: \.self) { index in
                        Button(action: {
                            typeSelectorIndex = index
                        }) {
                            Text(audioTypes[index])
                                .padding(.vertical, 8)
                                .padding(.horizontal, 12)
                                .background(
                                    typeSelectorIndex == index ?
                                        Color.blue :
                                        Color.gray.opacity(0.2)
                                )
                                .foregroundColor(
                                    typeSelectorIndex == index ?
                                        Color.white :
                                        Color.primary
                                )
                                .cornerRadius(8)
                        }
                        .buttonStyle(PlainButtonStyle())
                    }
                }
            }
            
            // Mojo level selection
            VStack(alignment: .leading, spacing: 20) {
                Text("Mojo Level")
                    .font(.headline)
                
                HStack {
                    ZStack {
                        // Base
                        Capsule()
                            .fill(Color.gray.opacity(0.2))
                            .frame(height: 50)
                        
                        // Selected highlight
                        Capsule()
                            .fill(
                                LinearGradient(
                                    gradient: Gradient(colors: [
                                        Color.orange,
                                        Color.red
                                    ]),
                                    startPoint: .leading,
                                    endPoint: .trailing
                                )
                            )
                            .frame(height: 50)
                            .mask(
                                GeometryReader { geo in
                                    Capsule()
                                        .frame(
                                            width: geo.size.width / CGFloat(mojoLevels.count),
                                            height: 50
                                        )
                                        .offset(
                                            x: CGFloat(mojoSelectorIndex) * geo.size.width / CGFloat(mojoLevels.count)
                                        )
                                }
                            )
                        
                        // Text labels
                        HStack(spacing: 0) {
                            ForEach(0..<mojoLevels.count, id: \.self) { index in
                                Text(mojoLevels[index])
                                    .fontWeight(mojoSelectorIndex == index ? .bold : .regular)
                                    .foregroundColor(mojoSelectorIndex == index ? .white : .primary)
                                    .frame(maxWidth: .infinity)
                                    .onTapGesture {
                                        withAnimation(.spring()) {
                                            mojoSelectorIndex = index
                                        }
                                    }
                            }
                        }
                    }
                }
            }
            
            Spacer()
            
            // Process button
            Button(action: onProcess) {
                Text("Process Audio")
                    .fontWeight(.bold)
                    .padding()
                    .frame(maxWidth: .infinity)
                    .background(Color.blue)
                    .foregroundColor(.white)
                    .cornerRadius(10)
            }
            .padding(.top, 20)
        }
        .padding()
        .background(
            RoundedRectangle(cornerRadius: 12)
                .fill(Color.white.opacity(0.2))
        )
        .sheet(isPresented: $showStealMojo) {
            StealMojoPanel_SwiftOnly(onApply: { params in
                engine.setParams(params)
            })
        }
    }
}
