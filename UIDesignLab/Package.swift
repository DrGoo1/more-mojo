// swift-tools-version: 5.9
// The swift-tools-version declares the minimum version of Swift required to build this package.

import PackageDescription

let package = Package(
    name: "UIDesignLab",
    platforms: [
        .macOS(.v13)
    ],
    products: [
        .executable(
            name: "UIDesignLab",
            targets: ["UIDesignLab"]),
    ],
    dependencies: [],
    targets: [
        .executableTarget(
            name: "UIDesignLab",
            dependencies: [],
            path: "Sources"),
    ]
)
