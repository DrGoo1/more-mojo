import Foundation

// Extension-only file. Canonical types live in SharedTypes.swift
extension ProcessorParams {
    /// Example convenience mapping for UI (-12..+12 dB -> 0..1)
    public var outputNormalized: Float { (output + 12) / 24 }
}
