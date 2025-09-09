
import SwiftUI

struct ContentView: View {
    @State private var params = ProcessorParams()
    var body: some View {
        MoreMojoContainer(onSubmitParams: { p in
            // engine.setParams(p) // wire your engine here
            params = p
        })
    }
}
