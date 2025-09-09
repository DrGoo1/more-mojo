
import SwiftUI

final class MojoVM: ObservableObject {
    @Published var level: MojoLevel = .mojo
    @Published var type: AudioType  = .vocal
    @Published var showMaker = false
}

struct MoreMojoContainer: View {
    @StateObject var vm = MojoVM()
    var onSubmitParams: ((ProcessorParams)->Void)? = nil

    @State private var drive = 0.55
    @State private var mix = 1.0
    @State private var outputNorm = 0.5
    @State private var selectorIndex = 4
    @State private var toggle1 = false
    @State private var spectrum = Array(repeating: Float(0.1), count: 128)
    @State private var vuL: CGFloat = 0.6
    @State private var vuR: CGFloat = 0.58

    var body: some View {
        ZStack {
            MoreMojoSimpleView(vm: vm) {
                let p = Presets.apply(vm.level, type: vm.type)
                onSubmitParams?(p)
            }
            .opacity(vm.showMaker ? 0 : 1)
            .blur(radius: vm.showMaker ? 4 : 0)
            .scaleEffect(vm.showMaker ? 0.98 : 1)

            if vm.showMaker {
                MojoMakerView(drive: $drive, mix: $mix, outputNorm: $outputNorm,
                              selectorIndex: $selectorIndex, toggle1: $toggle1,
                              spectrum: spectrum, vuL: vuL, vuR: vuR,
                              onChange: {
                                  var p = ProcessorParams()
                                  p.drive = Float(drive)
                                  p.mix   = Float(mix)
                                  p.output = Float(outputNorm * 12 - 6)
                                  onSubmitParams?(p)
                              },
                              onBack: { vm.showMaker = false })
                .transition(.move(edge: .bottom).combined(with: .opacity))
            }
        }
        .frame(minWidth: 1080, minHeight: 680)
        .background(Color.black)
    }
}
