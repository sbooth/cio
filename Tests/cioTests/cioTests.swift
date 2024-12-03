import Testing
@testable import cio

@Test func stream_test() async throws {
	_ = cio.cstream()
}
