
import SwiftUI

struct ContentView: View {
    @StateObject private var engine = AudioEngine()
    @State private var params = ProcessorParams()
    var body: some View {
        MoreMojoContainer(onSubmitParams: { p in
            engine.setParams(p)
            params = p
        })
    }
}
