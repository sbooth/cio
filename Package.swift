// swift-tools-version: 5.9

import PackageDescription

let package = Package(
    name: "cio",
    products: [
        .library(
            name: "cio",
            targets: [
                "cio",
            ]),
    ],
    targets: [
        .target(
            name: "cio"),
        .testTarget(
            name: "cioTests",
            dependencies: [
                "cio",
            ],
            swiftSettings: [
                .interoperabilityMode(.Cxx),
            ]),
    ],
    cxxLanguageStandard: .cxx17
)
