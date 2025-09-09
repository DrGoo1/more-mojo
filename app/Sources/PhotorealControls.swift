import SwiftUI
import Foundation

/// Extensions to help with value clamping
private extension Double { 
    var clamped01: Double { 
        Swift.min(1, Swift.max(0, self)) 
    } 
}

private extension CGFloat { 
    var clamped01: CGFloat { 
        Swift.min(1, Swift.max(0, self)) 
    } 
}

/// Sprite strip view for animated controls
struct SpriteStrip: View {
    let imageName: String
    let frames: Int
    let index: Int
    var body: some View {
        GeometryReader { geo in
            let frameW = geo.size.width
            Image(imageName).resizable().interpolation(.high).aspectRatio(contentMode: .fill)
                .frame(width: frameW * CGFloat(frames), height: geo.size.height, alignment: .leading)
                .offset(x: -CGFloat(index) * frameW, y: 0)
                .clipped()
        }
    }
}

/// Photorealistic round knob control
struct PRRoundKnob: View {
    @Binding var value: Double
    var label: String
    
    // Knob state
    @State private var isDragging = false
    @State private var startY: CGFloat = 0
    @State private var startValue: Double = 0
    
    // Computed frame index based on value
    private var frameIndex: Int {
        let maxFrame = MoreMojoAssets.knobFrameCount - 1
        return min(maxFrame, max(0, Int(value.clamped01 * Double(maxFrame))))
    }
    
    var body: some View {
        VStack(spacing: 4) {
            // Knob sprite
            SpriteStrip(
                imageName: MoreMojoAssets.roundKnobImage, 
                frames: MoreMojoAssets.knobFrameCount, 
                index: frameIndex
            )
            .aspectRatio(1, contentMode: .fit)
            .frame(width: 64, height: 64)
            .gesture(
                DragGesture(minimumDistance: 0)
                    .onChanged { gesture in
                        if !isDragging {
                            isDragging = true
                            startY = gesture.location.y
                            startValue = value
                        }
                        
                        // Calculate vertical drag (reversed)
                        let deltaY = startY - gesture.location.y
                        // Scale for sensitivity (adjust as needed)
                        let deltaValue = Double(deltaY) / 150.0
                        
                        // Update value with clamping
                        value = (startValue + deltaValue).clamped01
                    }
                    .onEnded { _ in
                        isDragging = false
                    }
            )
            
            // Label
            Text(label)
                .font(.system(size: 10, weight: .medium))
                .foregroundColor(.white.opacity(0.8))
            
            // Value indicator (percentage)
            Text("\(Int(value * 100))%")
                .font(.system(size: 9, weight: .bold, design: .monospaced))
                .foregroundColor(.white.opacity(0.7))
        }
    }
}

/// Photorealistic chicken head selector
struct PRChickenHeadSelector: View {
    @Binding var index: Int
    var label: String
    var count: Int = 3 // Default to 3 positions
    
    // Knob state
    @State private var isDragging = false
    @State private var startX: CGFloat = 0
    @State private var startIndex: Int = 0
    
    // Computed properties
    private var normalizedValue: Double {
        Double(index) / Double(count - 1)
    }
    
    private var frameIndex: Int {
        let maxFrame = MoreMojoAssets.selectorFrameCount - 1
        return min(maxFrame, max(0, Int(normalizedValue * Double(maxFrame))))
    }
    
    var body: some View {
        VStack(spacing: 4) {
            // Selector sprite
            SpriteStrip(
                imageName: MoreMojoAssets.selectorImage, 
                frames: MoreMojoAssets.selectorFrameCount, 
                index: frameIndex
            )
            .aspectRatio(1, contentMode: .fit)
            .frame(width: 64, height: 64)
            .gesture(
                DragGesture(minimumDistance: 0)
                    .onChanged { gesture in
                        if !isDragging {
                            isDragging = true
                            startX = gesture.location.x
                            startIndex = index
                        }
                        
                        // Calculate horizontal drag
                        let deltaX = gesture.location.x - startX
                        // Scale for sensitivity (adjust based on knob size)
                        let deltaIndex = Int(deltaX / 20)
                        
                        // Update index with clamping
                        index = min(count - 1, max(0, startIndex + deltaIndex))
                    }
                    .onEnded { _ in
                        isDragging = false
                    }
            )
            
            // Label
            Text(label)
                .font(.system(size: 10, weight: .medium))
                .foregroundColor(.white.opacity(0.8))
        }
    }
}

/// Photorealistic toggle switch (Neve-style)
struct PRToggleNeve: View {
    @Binding var isOn: Bool
    var label: String
    
    var body: some View {
        VStack(spacing: 4) {
            // Toggle switch image
            Image(isOn ? MoreMojoAssets.toggleOnImage : MoreMojoAssets.toggleOffImage)
                .resizable()
                .aspectRatio(contentMode: .fit)
                .frame(width: 40, height: 40)
                .onTapGesture {
                    withAnimation(.spring()) {
                        isOn.toggle()
                    }
                }
            
            // Label
            Text(label)
                .font(.system(size: 10, weight: .medium))
                .foregroundColor(.white.opacity(0.8))
        }
    }
}

/// Photorealistic LED indicator
struct PRLED: View {
    var color: Color
    var on: Bool
    
    var body: some View {
        ZStack {
            // LED base (off state)
            Circle()
                .fill(Color.black.opacity(0.5))
                .overlay(
                    Circle()
                        .strokeBorder(Color.gray.opacity(0.5), lineWidth: 1)
                )
            
            // LED glow when on
            if on {
                Circle()
                    .fill(color)
                    .blur(radius: 2)
                    .opacity(0.7)
                
                Circle()
                    .fill(color)
                    .scaleEffect(0.7)
            }
        }
        .frame(width: 10, height: 10)
    }
}

/// Photorealistic VU meter
struct PRVUMeter: View {
    var value: CGFloat // 0.0-1.0
    
    // Needle animation state
    @State private var animatedValue: CGFloat = 0
    
    var body: some View {
        ZStack {
            // VU meter background image
            Image(MoreMojoAssets.vuMeterImage)
                .resizable()
                .aspectRatio(contentMode: .fit)
            
            // Needle
            Rectangle()
                .fill(Color.black)
                .frame(width: 2, height: 40)
                .offset(y: -20)
                .rotationEffect(
                    .degrees(Double(-50 + animatedValue.clamped01 * 100)),
                    anchor: .bottom
                )
                .shadow(color: .black.opacity(0.5), radius: 1, x: 0, y: 0)
        }
        .onAppear {
            animatedValue = value
        }
        .onChange(of: value) { newValue in
            withAnimation(.spring(response: 0.3, dampingFraction: 0.6)) {
                animatedValue = newValue
            }
        }
    }
}
