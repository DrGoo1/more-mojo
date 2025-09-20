import SwiftUI

@main
struct UIDesignLabApp: App {
    @State private var selectedTab = 0
    
    var body: some Scene {
        WindowGroup {
            TabView(selection: $selectedTab) {
                // Knob Designer
                KnobDesignSelector()
                    .frame(minWidth: 800, minHeight: 750)
                    .tabItem {
                        Label("Knob Designer", systemImage: "dial.min")
                    }
                    .tag(0)
                
                // Professional Controls Demo
                ControlsDemoView()
                    .frame(minWidth: 800, minHeight: 750)
                    .tabItem {
                        Label("Pro Controls", systemImage: "slider.horizontal.3")
                    }
                    .tag(1)
                    
                // Comparison View
                CompareKnobsView()
                    .frame(minWidth: 800, minHeight: 750)
                    .tabItem {
                        Label("Compare Knobs", systemImage: "arrow.left.and.right.square")
                    }
                    .tag(2)
            }
            .frame(minWidth: 850, minHeight: 800)
        }
    }
}
