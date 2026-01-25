import Testing
@testable import cio

@Test func stream_test() async throws {
    let f = cio.cstream()
    let valid = f.__convertToBool()
    #expect(!valid)
}
