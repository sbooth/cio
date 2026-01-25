//
// SPDX-FileCopyrightText: 2024 Stephen F. Booth <contact@sbooth.dev>
// SPDX-License-Identifier: MIT
//
// Part of https://github.com/sbooth/cio
//

import Testing
@testable import cio

@Test func stream_test() async throws {
    let f = cio.cstream()
    let valid = f.__convertToBool()
    #expect(!valid)
}
