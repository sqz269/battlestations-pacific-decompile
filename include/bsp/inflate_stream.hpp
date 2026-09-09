#pragma once
#include <cstdint>
#include <memory>

namespace bsp {
// A retained source with one active cursor. Its owner must not move that cursor
// outside this adapter while decoding. These are host methods, not native slots.
class InflateSource {
public:
    virtual ~InflateSource() = default;
    virtual bool seek_absolute(std::uint32_t offset) noexcept = 0;
    // Always report actual <= requested, including on failure. Positive short
    // reads are accepted. The adapter initializes actual to zero before calling.
    virtual bool read(void* destination, std::uint32_t requested,
        std::uint32_t& actual) noexcept = 0;
};

struct InflateStreamDescriptor {
    std::uint32_t source_offset{};
    std::uint32_t compressed_size{};
    std::uint32_t decoded_size{};
};

enum class InflateStreamStatus {
    ok,
    end_of_stream,
    not_initialized,
    already_initialized,
    invalid_argument,
    allocation_failed,
    source_seek_failed,
    source_read_failed,
    invalid_source_count,
    decoder_error,
    decoded_size_mismatch,
    no_progress,
    unsupported_backward_seek
};

// Buffered projection of 00bbc1d0/00bbbf00/00bbc060/00bbc140. Uses stock raw
// zlib 1.2.1 and fixed 16KiB input / 64KiB output; not the native object or ABI.
// Full evidence and native failure differences: docs/INFLATE_STREAM_READ_SEEK.md
// and docs/INFLATE_STREAM_IMPLEMENTATION.md. No arbitrary restart/MPKG parser.
class InflateStream {
public:
    InflateStream() noexcept;
    ~InflateStream();
    InflateStream(InflateStream&&) noexcept;
    InflateStream& operator=(InflateStream&&) noexcept;
    InflateStream(const InflateStream&) = delete;
    InflateStream& operator=(const InflateStream&) = delete;

    // Retains source and seeks it to source_offset after initializing raw
    // inflate. A failed open keeps this adapter uninitialized. Source seek may
    // itself have side effects on failure. An open adapter cannot be reopened.
    InflateStreamStatus open_00bbc1d0_fragment(
        std::shared_ptr<InflateSource> source,
        InflateStreamDescriptor descriptor) noexcept;

    // Always writes actual when supplied, including on error. A short read at
    // declared EOF returns end_of_stream; an exact read returns ok. A decoder
    // failure can still publish a buffered prefix: it remains drainable, but
    // every nonzero read reports the latched failure. Zero requests are no-ops.
    InflateStreamStatus read_00bbc140(void* destination,
        std::uint32_t requested, std::uint32_t* actual = nullptr) noexcept;

    // Native low-DWORD wrapping, origins 0/1/other = start/current/end.
    // Forward seeks stop at EOF. Backward seeks may reuse the current output
    // block; farther rewinds return unsupported_backward_seek without mutation.
    InflateStreamStatus seek_00bbc060(std::int64_t offset,
        std::uint32_t origin) noexcept;

    std::uint64_t position_00bbbe50() const noexcept;
    std::uint64_t size_00bbbdd0() const noexcept;
    // Last stock-zlib return code, for diagnosing decoder_error; no zlib header
    // or register calling convention leaks into this public interface.
    int decoder_status() const noexcept;

private:
    struct Impl;
    std::unique_ptr<Impl> impl_;
    int initialization_decoder_status_{};
};
}
