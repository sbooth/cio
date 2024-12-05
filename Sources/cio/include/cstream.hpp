//
// Copyright © 2024 Stephen F. Booth
// Part of https://github.com/sbooth/cio
// MIT license
//

#import <cassert>
#import <cstdarg>
#import <cstdint>
#import <cstdio>
#import <optional>
#import <type_traits>
#import <vector>

#import <libkern/OSByteOrder.h>

namespace cio {

/// A class managing a C stream (`std::FILE *`) object.
///
/// This class wraps the C-style I/O functions in the `<cstdio>` header.
///
/// For more information see [cppreference](https://en.cppreference.com/w/cpp/io/c).
class cstream {

public:

	// MARK: Standard Six

	/// Initializes a `cio::cstream` object and sets the managed stream to `nullptr`.
	explicit constexpr cstream() noexcept = default;

	// This class is non-copyable.
	cstream(const cstream& rhs) = delete;

	// This class is non-assignable.
	cstream& operator=(const cstream& rhs) = delete;

	/// Closes the managed stream.
	~cstream()
	{
		reset();
	}

	/// Initializes a `cio::cstream` object with the managed stream from `rhs` and sets the managed stream of `rhs` to `nullptr`.
	cstream(cstream&& rhs) noexcept
	: cstream{rhs.release()}
	{}

	/// Closes the managed stream and replaces it with the managed stream from `rhs`, then sets the managed stream of `rhs` to `nullptr`.
	cstream& operator=(cstream&& rhs) noexcept
	{
		if(this != &rhs)
			reset(rhs.release());
		return *this;
	}

	// MARK: Initialization

	/// Initializes a `cio::cstream` object and sets the managed stream to the result of `std::fopen(filename, mode)`.
	cstream(const char *filename, const char *mode) noexcept
	: stream_{std::fopen(filename, mode)}
	{}

	/// Initializes a `cio::cstream` object and sets the managed stream to `stream`.
	explicit cstream(std::FILE *stream) noexcept
	: stream_{stream}
	{}

	// MARK: Comparison

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

	// MARK: Managed Stream Handling

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

	/// Swaps the managed streams of this object and `other`.
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

	/// Sets the managed stream to the result of `std::fopen(filename, mode)`.
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

	/// Sets the managed stream to the result of `std::freopen(filename, mode)` on the current managed stream.
	/// - seealso: [std::freopen](https://en.cppreference.com/w/cpp/io/c/freopen)
	cstream& freopen(const char *filename, const char *mode) noexcept
	{
		stream_ = std::freopen(filename, mode, stream_);
		return *this;
	}

	/// Returns the result of `std::fclose()` on the managed stream and sets the managed stream to `std::nullptr`.
	/// - seealso: [std::fclose](https://en.cppreference.com/w/cpp/io/c/fclose)
	int fclose() noexcept
	{
		auto result = std::fclose(stream_);
		stream_ = nullptr;
		return result;
	}

	/// Returns the result of `std::fflush()` on the managed stream.
	/// - seealso: [std::fflush](https://en.cppreference.com/w/cpp/io/c/fflush)
	int fflush() noexcept
	{
		return std::fflush(stream_);
	}

	/// Calls `std::setbuf(buffer)` on the managed stream.
	/// - seealso: [std::setbuf](https://en.cppreference.com/w/cpp/io/c/setbuf)
	void setbuf(char *buffer) noexcept
	{
		std::setbuf(stream_, buffer);
	}

	/// Returns the result of `std::setvbuf(buffer, mode, size)` on the managed stream.
	/// - seealso: [std::setvbuf](https://en.cppreference.com/w/cpp/io/c/setvbuf)
	int setvbuf(char *buffer, int mode, std::size_t size) noexcept
	{
		return std::setvbuf(stream_, buffer, mode, size);
	}

	/// Returns the result of `setvbuf(nullptr, _IONBF, 0)`.
	int setvbuf(std::nullptr_t) noexcept
	{
		return setvbuf(nullptr, _IONBF, 0);
	}

	// MARK: Direct Input/Output

	/// Returns the result of `std::fread(buffer, size, count)` on the managed stream.
	/// - seealso: [std::fread](https://en.cppreference.com/w/cpp/io/c/fread)
	std::size_t fread(void *buffer, std::size_t size, std::size_t count) noexcept
	{
		return std::fread(buffer, size, count, stream_);
	}

	/// Returns the result of `fread(buffer, sizeof(T), count)`.
	template <typename T, typename = std::enable_if_t<std::is_trivially_copyable_v<T>>>
	std::size_t fread(T *buffer, std::size_t count) noexcept
	{
		return fread(buffer, sizeof(T), count);
	}

	/// Returns the result of `fread(buffer, S)`.
	template <typename T, std::size_t S>
	std::size_t fread(T (&buffer)[S]) noexcept
	{
		return fread(buffer, S);
	}

	/// Returns the result of `fread(&value, 1) == 1`.
	template <typename T>
	bool fread(T& value) noexcept
	{
		return fread(&value, 1) == 1;
	}

	/// Returns the result of `std::fwrite(buffer, size, count)` on the managed stream.
	/// - seealso: [std::fwrite](https://en.cppreference.com/w/cpp/io/c/fwrite)
	std::size_t fwrite(const void *buffer, std::size_t size, std::size_t count) noexcept
	{
		return std::fwrite(buffer, size, count, stream_);
	}

	/// Returns the result of `fwrite(buffer, sizeof(T), count)`.
	template <typename T, typename = std::enable_if_t<std::is_trivially_copyable_v<T>>>
	std::size_t fwrite(const T *buffer, std::size_t count) noexcept
	{
		return fwrite(buffer, sizeof(T), count);
	}

	/// Returns the result of `fwrite(buffer, S)`.
	template <typename T, std::size_t S>
	std::size_t fwrite(const T (&buffer)[S]) noexcept
	{
		return fwrite(buffer, S);
	}

	/// Returns the result of `fwrite(&value, 1) == 1`.
	template <typename T>
	bool fwrite(const T& value) noexcept
	{
		return fwrite(&value, 1) == 1;
	}

	// MARK: Unformatted Input/Output

	/// Returns the result of `std::fgetc()` on the managed stream.
	/// - seealso: [std::fgetc](https://en.cppreference.com/w/cpp/io/c/fgetc)
	[[nodiscard]]
	int fgetc() noexcept
	{
		return std::fgetc(stream_);
	}

	/// Returns the result of `std::fgets(str)` on the managed stream.
	/// - seealso: [std::fgets](https://en.cppreference.com/w/cpp/io/c/fgets)
	char * fgets(char *str, int count) noexcept
	{
		return std::fgets(str, count, stream_);
	}

	/// Returns the result of `fgets(str, S)`.
	template <std::size_t S>
	char * fgets(char (&str)[S]) noexcept
	{
		return fgets(str, S);
	}

	/// Returns the result of `std::fputc(ch)` on the managed stream.
	/// - seealso: [std::fputc](https://en.cppreference.com/w/cpp/io/c/fputc)
	int fputc(int ch) noexcept
	{
		return std::fputc(ch, stream_);
	}

	/// Returns the result of `std::fputs(str)` on the managed stream.
	/// - seealso: [std::fputs](https://en.cppreference.com/w/cpp/io/c/fputs)
	int fputs(const char *str) noexcept
	{
		return std::fputs(str, stream_);
	}

	/// Returns the result of `std::ungetc(ch)` on the managed stream.
	/// - seealso: [std::ungetc](https://en.cppreference.com/w/cpp/io/c/ungetc)
	int ungetc(int ch) noexcept
	{
		return std::ungetc(ch, stream_);
	}

	// MARK: Formatted Input/Output

	/// Returns the result of `std::fscanf(format, std::forward<Args>(args)...` on the managed stream.
	/// - seealso: [std::fscanf](https://en.cppreference.com/w/cpp/io/c/fscanf)
	template <typename... Args>
	int fscanf(const char *format, Args&&... args) noexcept
	{
		return std::fscanf(stream_, format, std::forward<Args>(args)...);
	}

	/// Returns the result of `std::vfscanf(format, vlist)` on the managed stream.
	/// - seealso: [std::vfscanf](https://en.cppreference.com/w/cpp/io/c/vfscanf)
	int vfscanf(const char *format, std::va_list vlist) noexcept
	{
		return std::vfscanf(stream_, format, vlist);
	}

	/// Returns the result of `std::fprintf(format, std::forward<Args>(args)...)` on the managed stream.
	/// - seealso: [std::fprintf](https://en.cppreference.com/w/cpp/io/c/fprintf)
	template <typename... Args>
	int fprintf(const char *format, Args&&... args) noexcept
	{
		return std::fprintf(stream_, format, std::forward<Args>(args)...);
	}

	/// Returns the result of `std::vfprintf(format, vlist)` on the managed stream.
	/// - seealso: [std::vfprintf](https://en.cppreference.com/w/cpp/io/c/vfprintf)
	int vfprintf(const char *format, std::va_list vlist) noexcept
	{
		return std::vfprintf(stream_, format, vlist);
	}

	// MARK: File Positioning

	/// Returns the result of `std::ftell()` on the managed stream.
	/// - seealso: [std::ftell](https://en.cppreference.com/w/cpp/io/c/ftell)
	[[nodiscard]]
	long ftell() const noexcept
	{
		return std::ftell(stream_);
	}

	/// Returns the result of `std::fgetpos(pos)` on the managed stream.
	/// - seealso: [std::fgetpos](https://en.cppreference.com/w/cpp/io/c/fgetpos)
	int fgetpos(std::fpos_t *pos) const noexcept
	{
		return std::fgetpos(stream_, pos);
	}

	/// Returns the result of `std::fseek(offset, origin)` on the managed stream.
	/// - seealso: [std::fseek](https://en.cppreference.com/w/cpp/io/c/fseek)
	int fseek(long offset, int origin) noexcept
	{
		return std::fseek(stream_, offset, origin);
	}

	/// Returns the result of `std::fsetpos(pos)` on the managed stream.
	/// - seealso: [std::fsetpos](https://en.cppreference.com/w/cpp/io/c/fsetpos)
	int fsetpos(const std::fpos_t *pos) noexcept
	{
		return std::fsetpos(stream_, pos);
	}

	/// Calls `std::rewind()` on the managed stream.
	/// - seealso: [std::rewind](https://en.cppreference.com/w/cpp/io/c/rewind)
	void rewind() noexcept
	{
		std::rewind(stream_);
	}

	// MARK: Error Handling

	/// Calls `std::clearerr()` on the managed stream.
	/// - seealso: [std::clearerr](https://en.cppreference.com/w/cpp/io/c/clearerr)
	void clearerr() noexcept
	{
		std::clearerr(stream_);
	}

	/// Returns the result of `std::feof()` on the managed stream.
	/// - seealso: [std::feof](https://en.cppreference.com/w/cpp/io/c/feof)
	[[nodiscard]]
	int feof() const noexcept
	{
		return std::feof(stream_);
	}

	/// Returns the result of `std::ferror()` on the managed stream.
	/// - seealso: [std::ferror](https://en.cppreference.com/w/cpp/io/c/ferror)
	[[nodiscard]]
	int ferror() const noexcept
	{
		return std::ferror(stream_);
	}

	/// Calls `std::perror(s)`.
	/// - seealso: [std::perror](https://en.cppreference.com/w/cpp/io/c/perror)
	static void perror(const char *s) noexcept
	{
		std::perror(s);
	}

	// MARK: Operations on Files

	/// Returns the result of `std::remove(pathname)`.
	/// - seealso: [std::remove](https://en.cppreference.com/w/cpp/io/c/remove)
	static int remove(const char *pathname) noexcept
	{
		return std::remove(pathname);
	}

	/// Returns the result of `std::rename(old_filename, new_filename)`.
	/// - seealso: [std::rename](https://en.cppreference.com/w/cpp/io/c/rename)
	static int rename(const char *old_filename, const char *new_filename) noexcept
	{
		return std::rename(old_filename, new_filename);
	}

	/// Returns a `cio::cstream` object initialized with the result of `std::tmpfile()`.
	/// - seealso: [std::tmpfile](https://en.cppreference.com/w/cpp/io/c/tmpfile)
	[[nodiscard]]
	static cstream tmpfile() noexcept
	{
		return cstream{std::tmpfile()};
	}

	/// Returns the result of `std::tmpnam(filename)`.
	/// - seealso: [std::tmpnam](https://en.cppreference.com/w/cpp/io/c/tmpnam)
	[[deprecated("Use mkstemp(3) instead.")]]
	static char * tmpnam(char *filename) noexcept
	{
		return std::tmpnam(filename);
	}

	// MARK: - Extensions

	/// Reads a block of data.
	/// - parameter count: The maximum number of elements to read.
	/// - returns: A `std::vector` containing the requested elements.
	/// - throws: Any exception thrown by `Allocator::allocate()` (typically `std::bad_alloc`)
	/// - throws: `std::length_error`
	template <typename T>
	std::vector<T> read_block(typename std::vector<T>::size_type count)
	{
		if(count == 0)
			return {};
		std::vector<T> buf(count);
		buf.resize(fread(buf.data(), count));
		return buf;
	}

	/// Writes a block of data.
	/// - parameter v: The data to write.
	/// - returns: The number of elements written.
	template <typename T>
	typename std::vector<T>::size_type write_block(const std::vector<T>& v) noexcept
	{
		return static_cast<typename std::vector<T>::size_type>(fwrite(v.data(), v.size()));
	}


	/// Reads a value.
	/// - returns: The value read or `std::nullopt` on failure.
	template <typename T, typename = std::enable_if_t<std::is_trivially_default_constructible_v<T>>>
	std::optional<T> read() noexcept(std::is_nothrow_default_constructible_v<T>)
	{
		T value{};
		if(!fread(value))
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
	template <typename T, typename = std::enable_if_t<std::is_same_v<T, std::uint16_t> || std::is_same_v<T, std::uint32_t> || std::is_same_v<T, std::uint64_t>>>
	bool read_uint(T& value, byte_order order = byte_order::host) noexcept
	{
		if(!fread(value))
			return false;

		switch(order) {
			case byte_order::little_endian:
				switch(sizeof(T)) {
					case 2:	value = OSSwapLittleToHostInt16(value); break;
					case 4:	value = OSSwapLittleToHostInt32(value); break;
					case 8:	value = OSSwapLittleToHostInt64(value); break;
				}
				break;
			case byte_order::big_endian:
				switch(sizeof(T)) {
					case 2:	value = OSSwapBigToHostInt16(value); break;
					case 4:	value = OSSwapBigToHostInt32(value); break;
					case 8:	value = OSSwapBigToHostInt64(value); break;
				}
				break;
			case byte_order::host:
				break;
			case byte_order::swapped:
				switch(sizeof(T)) {
					case 2: value = OSSwapInt16(value); break;
					case 4: value = OSSwapInt32(value); break;
					case 8: value = OSSwapInt64(value); break;
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
	std::optional<T> read_uint(byte_order order = byte_order::host) noexcept(std::is_nothrow_default_constructible_v<T>)
	{
		T value{};
		if(!read_uint(value, order))
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
