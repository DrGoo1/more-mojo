import Foundation

extension AnalogInterpolator {
    // Add the missing configure method to satisfy the protocol requirements
    func configure(drive: Float, character: Float, saturation: Float, presence: Float) {
        updateParameters(
            drive: drive,
            character: character,
            saturation: saturation,
            presence: presence
        )
    }
}
