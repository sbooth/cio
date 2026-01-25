// swift-tools-version: 5.9
//
// SPDX-FileCopyrightText: 2024 Stephen F. Booth <contact@sbooth.dev>
// SPDX-License-Identifier: MIT
//
// Part of https://github.com/sbooth/cio
//

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
