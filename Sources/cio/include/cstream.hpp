//
// Copyright © 2024 Stephen F. Booth
// Part of https://github.com/sbooth/cio
// MIT license
//

#import <cassert>
#import <cstdio>
#import <cstdint>
#import <type_traits>
#import <vector>

namespace cio {

/// A class managing a C stream (a `std::FILE *` object).
///
/// This class wraps the C-style I/O functions in the `<cstdio>` header.
///
/// For more information see [cppreference](https://en.cppreference.com/w/cpp/io/c).
class cstream {

public:

	/// Initializes a `cio::cstream` object and sets the managed stream to `nullptr`.
	explicit constexpr cstream() noexcept = default;

	// This class is non-copyable
	cstream(const cstream& rhs) = delete;

	// This class is non-assignable
	cstream& operator=(const cstream& rhs) = delete;

	/// Calls `std::fclose` on the managed stream.
	~cstream()
	{
		reset();
	}

	/// Move constructor
	cstream(cstream&& rhs) noexcept
	: cstream{rhs.release()}
	{}

	/// Move assignment operator
	cstream& operator=(cstream&& rhs) noexcept
	{
		if(this != &rhs)
			reset(rhs.release());
		return *this;
	}

	/// Initializes a `cio::cstream` object and sets the managed stream to the result of `std::fopen(filename, mode)`.
	cstream(const char *filename, const char *mode) noexcept
	: stream_{std::fopen(filename, mode)}
	{}

	/// Initializes a `cio::cstream` object and sets the managed stream to `stream`.
	explicit cstream(std::FILE *stream) noexcept
	: stream_{stream}
	{}

	/// Compares two `cio::cstream` objects for equality.
	/// - parameter rhs: The object to compare.
	/// - returns: `true` if the objects are equal, `false` otherwise.
	bool operator==(const cstream& rhs) const noexcept
	{
		return stream_ == rhs.stream_;
	}

	/// Compares two `cio::cstream` objects for inequality.
	/// - parameter rhs: The object to compare.
	/// - returns: `true` if the objects are not equal, `false` otherwise.
	bool operator!=(const cstream& rhs) const noexcept
	{
		return !operator==(rhs);
	}

	/// Returns `true` if the managed stream is not `std::nullptr`.
	[[nodiscard]]
	explicit operator bool() const noexcept
	{
		return stream_ != nullptr;
	}

	/// Returns the managed C stream object.
	[[nodiscard]]
	operator std::FILE *() const noexcept
	{
		return get();
	}

	/// Returns the managed C stream object.
	[[nodiscard]]
	std::FILE * get() const noexcept
	{
		return stream_;
	}

	/// Closes the managed stream and replaces it with `stream`.
	void reset(std::FILE *stream = nullptr) noexcept
	{
		if(auto old = std::exchange(stream_, stream); old)
			std::fclose(old);
	}

	/// Swaps the managed streams of `*this` and `other`.
	void swap(cstream& other) noexcept
	{
		std::swap(stream_, other.stream_);
	}

	/// Releases ownership of the managed stream and returns it without closing.
	std::FILE * release() noexcept
	{
		return std::exchange(stream_, nullptr);
	}

	// MARK: File Access

	/// Calls `std::fopen`
	/// - seealso: [std::fopen](https://en.cppreference.com/w/cpp/io/c/fopen)
	cstream& fopen(const char *filename, const char *mode) noexcept
	{
#if false
		assert(mStream == nullptr);
		mStream = std::fopen(filename, mode);
#else
		reset(std::fopen(filename, mode));
#endif
		return *this;
	}

	/// Calls `std::freopen`
	/// - seealso: [std::freopen](https://en.cppreference.com/w/cpp/io/c/freopen)
	cstream& freopen(const char *filename, const char *mode) noexcept
	{
		stream_ = std::freopen(filename, mode, stream_);
		return *this;
	}

	/// Calls `std::fclose`
	/// - seealso: [std::fclose](https://en.cppreference.com/w/cpp/io/c/fclose)
	int fclose() noexcept
	{
		auto result = std::fclose(stream_);
		stream_ = nullptr;
		return result;
	}

	/// Calls `std::fflush`
	/// - seealso: [std::fflush](https://en.cppreference.com/w/cpp/io/c/fflush)
	int fflush() noexcept
	{
		return std::fflush(stream_);
	}

	/// Calls `std::setbuf`
	/// - seealso: [std::setbuf](https://en.cppreference.com/w/cpp/io/c/setbuf)
	void setbuf(char *buffer) noexcept
	{
		std::setbuf(stream_, buffer);
	}

	/// Calls `std::setvbuf`
	/// - seealso: [std::setvbuf](https://en.cppreference.com/w/cpp/io/c/setvbuf)
	int setvbuf(char *buffer, int mode, std::size_t size) noexcept
	{
		return std::setvbuf(stream_, buffer, mode, size);
	}

	/// Calls `std::setvbuf`
	/// - seealso: [std::setvbuf](https://en.cppreference.com/w/cpp/io/c/setvbuf)
	int setvbuf(std::nullptr_t) noexcept
	{
		return setvbuf(nullptr, _IONBF, 0);
	}

	// MARK: Direct Input/Output

	/// Calls `std::fread`
	/// - seealso: [std::fread](https://en.cppreference.com/w/cpp/io/c/fread)
	std::size_t fread(void *buffer, std::size_t size, std::size_t count) noexcept
	{
		return std::fread(buffer, size, count, stream_);
	}

	/// Calls `std::fread`
	/// - seealso: [std::fread](https://en.cppreference.com/w/cpp/io/c/fread)
	template <typename T, typename = std::enable_if_t<std::is_trivially_copyable_v<T>>>
	std::size_t fread(T *buffer, std::size_t count) noexcept
	{
		return fread(buffer, sizeof(T), count);
	}

	/// Calls `std::fread`
	/// - seealso: [std::fread](https://en.cppreference.com/w/cpp/io/c/fread)
	template <typename T, std::size_t S>
	std::size_t fread(T (&buffer)[S]) noexcept
	{
		return fread(buffer, sizeof(T), S);
	}

	/// Calls `std::fwrite`
	/// - seealso: [std::fwrite](https://en.cppreference.com/w/cpp/io/c/fwrite)
	std::size_t fwrite(const void *buffer, std::size_t size, std::size_t count) noexcept
	{
		return std::fwrite(buffer, size, count, stream_);
	}

	/// Calls `std::fwrite`
	/// - seealso: [std::fwrite](https://en.cppreference.com/w/cpp/io/c/fwrite)
	template <typename T, typename = std::enable_if_t<std::is_trivially_copyable_v<T>>>
	std::size_t fwrite(const T *buffer, std::size_t count) noexcept
	{
		return fwrite(buffer, sizeof(T), count);
	}

	/// Calls `std::fwrite`
	/// - seealso: [std::fwrite](https://en.cppreference.com/w/cpp/io/c/fwrite)
	template <typename T, std::size_t S>
	std::size_t fwrite(const T (&buffer)[S]) noexcept
	{
		return fwrite(buffer, sizeof(T), S);
	}

	// MARK: Unformatted Input/Output

	/// Calls `std::fgetc`
	/// - seealso: [std::fgetc](https://en.cppreference.com/w/cpp/io/c/fgetc)
	int fgetc() noexcept
	{
		return std::fgetc(stream_);
	}

	/// Calls `std::fgets`
	/// - seealso: [std::fgets](https://en.cppreference.com/w/cpp/io/c/fgets)
	char * fgets(char *str, int count) noexcept
	{
		return std::fgets(str, count, stream_);
	}

	/// Calls `std::fgets`
	/// - seealso: [std::fgets](https://en.cppreference.com/w/cpp/io/c/fgets)
	template <std::size_t S>
	char * fgets(char (&str)[S]) noexcept
	{
		return fgets(str, S);
	}

	/// Calls `std::fputc`
	/// - seealso: [std::fputc](https://en.cppreference.com/w/cpp/io/c/fputc)
	int fputc(int ch) noexcept
	{
		return std::fputc(ch, stream_);
	}

	/// Calls `std::fputs`
	/// - seealso: [std::fputs](https://en.cppreference.com/w/cpp/io/c/fputs)
	int fputs(const char *str) noexcept
	{
		return std::fputs(str, stream_);
	}

	/// Calls `std::ungetc`
	/// - seealso: [std::ungetc](https://en.cppreference.com/w/cpp/io/c/ungetc)
	int ungetc(int ch) noexcept
	{
		return std::ungetc(ch, stream_);
	}

	// MARK: Formatted Input/Output

	/// Calls `std::vfscanf`
	/// - seealso: [std::vfscanf](https://en.cppreference.com/w/cpp/io/c/vfscanf)
	template <typename... Args>
	int fscanf(const char *format, Args&&... args) noexcept
	{
		return std::fscanf(stream_, format, std::forward<Args>(args)...);
	}

	/// Calls `std::vfscanf`
	/// - seealso: [std::vfscanf](https://en.cppreference.com/w/cpp/io/c/vfscanf)
	int vfscanf(const char *format, va_list list) noexcept
	{
		return std::vfscanf(stream_, format, list);
	}

	/// Calls `std::fprintf`
	/// - seealso: [std::fprintf](https://en.cppreference.com/w/cpp/io/c/fprintf)
	template <typename... Args>
	int fprintf(const char *format, Args&&... args) noexcept
	{
		return std::fprintf(stream_, format, std::forward<Args>(args)...);
	}

	/// Calls `std::vfprintf`
	/// - seealso: [std::vfprintf](https://en.cppreference.com/w/cpp/io/c/vfprintf)
	int vfprintf(const char *format, va_list list) noexcept
	{
		return std::vfprintf(stream_, format, list);
	}

	// MARK: File Positioning

	/// Calls `std::ftell`
	/// - seealso: [std::ftell](https://en.cppreference.com/w/cpp/io/c/ftell)
	long ftell() const noexcept
	{
		return std::ftell(stream_);
	}

	/// Calls `std::fgetpos`
	/// - seealso: [std::fgetpos](https://en.cppreference.com/w/cpp/io/c/fgetpos)
	int fgetpos(std::fpos_t *pos) const noexcept
	{
		return std::fgetpos(stream_, pos);
	}

	/// Calls `std::fseek`
	/// - seealso: [std::fseek](https://en.cppreference.com/w/cpp/io/c/fseek)
	int fseek(long offset, int origin) noexcept
	{
		return std::fseek(stream_, offset, origin);
	}

	/// Calls `std::fsetpos`
	/// - seealso: [std::fsetpos](https://en.cppreference.com/w/cpp/io/c/fsetpos)
	int fsetpos(const std::fpos_t *pos) noexcept
	{
		return std::fsetpos(stream_, pos);
	}

	/// Calls `std::rewind`
	/// - seealso: [std::rewind](https://en.cppreference.com/w/cpp/io/c/rewind)
	void rewind() noexcept
	{
		std::rewind(stream_);
	}

	// MARK: Error Handling

	/// Calls `std::clearerr`
	/// - seealso: [std::clearerr](https://en.cppreference.com/w/cpp/io/c/clearerr)
	void clearerr() noexcept
	{
		std::clearerr(stream_);
	}

	/// Calls `std::feof`
	/// - seealso: [std::feof](https://en.cppreference.com/w/cpp/io/c/feof)
	int feof() const noexcept
	{
		return std::feof(stream_);
	}

	/// Calls `std::ferror`
	/// - seealso: [std::ferror](https://en.cppreference.com/w/cpp/io/c/ferror)
	int ferror() const noexcept
	{
		return std::ferror(stream_);
	}

	/// Calls `std::perror`
	/// - seealso: [std::perror](https://en.cppreference.com/w/cpp/io/c/perror)
	static void perror(const char *s) noexcept
	{
		std::perror(s);
	}

	// MARK: Operations on Files

	/// Calls `std::remove`
	/// - seealso: [std::remove](https://en.cppreference.com/w/cpp/io/c/remove)
	static int remove(const char *pathname) noexcept
	{
		return std::remove(pathname);
	}

	/// Calls `std::rename`
	/// - seealso: [std::rename](https://en.cppreference.com/w/cpp/io/c/rename)
	static int rename(const char *old_filename, const char *new_filename) noexcept
	{
		return std::rename(old_filename, new_filename);
	}

	/// Calls `std::tmpfile`
	/// - seealso: [std::tmpfile](https://en.cppreference.com/w/cpp/io/c/tmpfile)
	static cstream tmpfile() noexcept
	{
		return cstream{std::tmpfile()};
	}

	/// Calls `std::tmpnam`
	/// - seealso: [std::tmpnam](https://en.cppreference.com/w/cpp/io/c/tmpnam)
	[[deprecated("Use mkstemp(3) instead.")]]
	static char * tmpnam(char *filename) noexcept
	{
		return std::tmpnam(filename);
	}

	// MARK: - Extensions

	/// Reads a block of data.
	/// - parameter length: The maximum number of bytes to read.
	/// - returns: A `std::vector` containing the requested bytes.
	/// - throws: Any exception thrown by `Allocator::allocate()` (typically `std::bad_alloc`)
	/// - throws: `std::length_error`
	std::vector<uint8_t> read_block(std::vector<uint8_t>::size_type length)
	{
		if(length == 0)
			return {};
		std::vector<uint8_t> buf(length);
		buf.resize(fread(buf.data(), length));
		return buf;
	}

	/// Reads a value.
	/// - parameter value: A reference to receive the value.
	/// - returns: `true` on success, `false` otherwise.
	template <typename T, typename = std::enable_if_t<std::is_trivially_copyable_v<T>>>
	bool read(T& value) noexcept
	{
		return fread(&value, 1) == 1;
	}

	/// Reads a value.
	/// - returns: The value read or `std::nullopt` on failure.
	template <typename T, typename = std::enable_if_t<std::is_trivially_default_constructible_v<T>>>
	std::optional<T> read() noexcept(std::is_nothrow_default_constructible_v<T>)
	{
		T value{};
		if(!read(value))
			return std::nullopt;
		return value;
	}

	/// Possible byte orders.
	enum class byte_order
	{
		/// Little-endian byte order.
		little_endian,
		/// Big-endian byte order.
		big_endian,
		/// Host byte order.
		host,
		/// Swapped byte order.
		swapped,
	};

	/// Reads an unsigned integer value and optionally changes its byte order.
	/// - parameter value: A reference to receive the value.
	/// - parameter order: The desired byte order.
	/// - returns: `true` on success, `false` otherwise.
	template <typename T, typename = std::enable_if_t<std::is_integral_v<T> && std::is_unsigned_v<T> && (sizeof(T) == 2 || sizeof(T) == 4 || sizeof(T) == 8)>>
#if false
	template <typename T, typename = std::enable_if_t<std::is_same_v<T, uint16_t> || std::is_same_v<T, uint32_t> || std::is_same_v<T, uint64_t>>>
#endif
	bool read_uint(T& value, byte_order order) noexcept
	{
		if(!read(value))
			return false;

		switch(order) {
			case byte_order::little_endian:
				switch(sizeof(T)) {
					case 2:	value = static_cast<T>(OSSwapLittleToHostInt16(value)); break;
					case 4:	value = static_cast<T>(OSSwapLittleToHostInt32(value)); break;
					case 8:	value = static_cast<T>(OSSwapLittleToHostInt64(value)); break;
				}
				break;
			case byte_order::big_endian:
				switch(sizeof(T)) {
					case 2:	value = static_cast<T>(OSSwapBigToHostInt16(value)); break;
					case 4:	value = static_cast<T>(OSSwapBigToHostInt32(value)); break;
					case 8:	value = static_cast<T>(OSSwapBigToHostInt64(value)); break;
				}
				break;
			case byte_order::host:
				break;
			case byte_order::swapped:
				switch(sizeof(T)) {
					case 2: value = static_cast<T>(OSSwapInt16(value)); break;
					case 4: value = static_cast<T>(OSSwapInt32(value)); break;
					case 8: value = static_cast<T>(OSSwapInt64(value)); break;
				}
				break;
		}

		return true;
	}

	/// Reads a little-endian unsigned integer value and converts it to host byte order.
	/// - parameter value: A reference to receive the value.
	/// - returns: `true` on success, `false` otherwise.
	template <typename T>
	bool read_uint_little(T& value) noexcept
	{
		return read_uint(value, byte_order::little_endian);
	}

	/// Reads a big-endian unsigned integer value and converts it to host byte order.
	/// - parameter value: A reference to receive the value.
	/// - returns: `true` on success, `false` otherwise.
	template <typename T>
	bool read_uint_big(T& value) noexcept
	{
		return read_uint(value, byte_order::big_endian);
	}

	/// Reads an unsigned integer value in host byte order.
	/// - parameter value: A reference to receive the value.
	/// - returns: `true` on success, `false` otherwise.
	template <typename T>
	bool read_uint_host(T& value) noexcept
	{
		return read_uint(value, byte_order::host);
	}

	/// Reads an unsigned integer value and swaps it byte order.
	/// - parameter value: A reference to receive the value.
	/// - returns: `true` on success, `false` otherwise.
	template <typename T>
	bool read_uint_swapped(T& value) noexcept
	{
		return read_uint(value, byte_order::swapped);
	}

	/// Reads an unsigned integer value and optionally changes its byte order.
	/// - parameter order: The desired byte order.
	/// - returns: The value read or `std::nullopt` on failure.
	template <typename T, typename = std::enable_if_t<std::is_trivially_default_constructible_v<T>>>
	std::optional<T> read_uint(byte_order order) noexcept(std::is_nothrow_default_constructible_v<T>)
	{
		T value{};
		if(!read(value, order))
			return std::nullopt;
		return value;
	}

	/// Reads a little-endian unsigned integer value and converts it to host byte order.
	/// - returns: The value read or `std::nullopt` on failure.
	template <typename T>
	std::optional<T> read_uint_little() noexcept
	{
		return read_uint<T>(byte_order::little_endian);
	}

	/// Reads a big-endian unsigned integer value and converts it to host byte order.
	/// - returns: The value read or `std::nullopt` on failure.
	template <typename T>
	std::optional<T> read_uint_big() noexcept
	{
		return read_uint<T>(byte_order::big_endian);
	}

	/// Reads a little-endian unsigned integer value in host byte order.
	/// - returns: The value read or `std::nullopt` on failure.
	template <typename T>
	std::optional<T> read_uint_host() noexcept
	{
		return read_uint<T>(byte_order::host);
	}

	/// Reads a little-endian unsigned integer value and swaps its byte order.
	/// - returns: The value read or `std::nullopt` on failure.
	template <typename T>
	std::optional<T> read_uint_swapped() noexcept
	{
		return read_uint<T>(byte_order::swapped);
	}

private:

	/// The managed C stream.
	std::FILE *stream_ = nullptr;

};

} /* namespace cio */
