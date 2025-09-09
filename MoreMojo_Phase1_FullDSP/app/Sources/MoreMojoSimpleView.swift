
import SwiftUI

enum MojoLevel: String, CaseIterable, Identifiable { case mojo, more, most; var id: String { rawValue } }
enum AudioType: String, CaseIterable, Identifiable { case vocal, instrument, bus, master; var id: String { rawValue } }

struct MoreMojoSimpleView: View {
    @ObservedObject var vm: MojoVM
    var onApply: ()->Void
    var body: some View {
        VStack(spacing: 18) {
            Text("More Mojo").font(.system(size: 26, weight: .bold, design: .rounded))
            HStack {
                ForEach(MojoLevel.allCases) { l in
                    Button(l.rawValue.capitalized) { vm.level = l }
                        .buttonStyle(.borderedProminent).tint(vm.level == l ? .blue : .gray)
                }
            }
            Picker("Audio Type", selection: $vm.type) {
                ForEach(AudioType.allCases) { t in Text(t.rawValue.capitalized).tag(t) }
            }
            .pickerStyle(.segmented).frame(width: 420)

            HStack(spacing: 12) {
                Button("Apply") { onApply() }.buttonStyle(.borderedProminent)
                Button("Mojo Maker") { onApply(); withAnimation(.spring()) { vm.showMaker = true } }
            }
        }
        .padding(24).background(Color.black.opacity(0.85))
    }
}
