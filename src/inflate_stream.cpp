#include "bsp/inflate_stream.hpp"
#include <zlib.h>
#include <algorithm>
#include <array>
#include <cstring>
#include <new>
#include <utility>

namespace bsp {
namespace {
constexpr std::uint32_t input_capacity = 0x4000;
constexpr std::uint32_t output_capacity = 0x10000;
static_assert(sizeof(z_stream) == 0x38, "The reconstructed target is MSVC Win32.");
static_assert(sizeof(InflateStreamDescriptor) == 12, "Native descriptor has three DWORDs.");
}

struct InflateStream::Impl {
    Impl(std::shared_ptr<InflateSource> retained_source,
        InflateStreamDescriptor desc) noexcept
        : source(std::move(retained_source)), descriptor(desc),
          compressed_remaining(desc.compressed_size),
          decoded_remaining(desc.decoded_size) {}

    ~Impl() {
        source.reset(); // Native 00bbc320 releases source before inflateEnd.
        if (decoder_initialized) inflateEnd(&decoder);
    }

    std::uint32_t available() const noexcept {
        return output_end - output_cursor;
    }

    // 00bbbf00: callers consume the current output block before refilling.
    // Unlike native, latch failures so an empty result cannot cause an endless
    // outer read/seek loop. A produced prefix remains readable on failure.
    void refill() noexcept {
        output_cursor = 0;
        output_end = 0;
        decoder.next_out = output.data();
        decoder.avail_out = output_capacity;
        while (decoder.avail_out != 0) {
            if (decoder.avail_in == 0 && compressed_remaining != 0) {
                const std::uint32_t requested =
                    (std::min)(input_capacity, compressed_remaining);
                std::uint32_t actual = 0;
                const bool read_ok = source->read(input.data(), requested, actual);
                if (actual > requested) {
                    failure = InflateStreamStatus::invalid_source_count;
                    break;
                }
                if (!read_ok) {
                    failure = InflateStreamStatus::source_read_failed;
                    break;
                }
                if (actual == 0) {
                    failure = InflateStreamStatus::no_progress;
                    break;
                }
                compressed_remaining -= actual;
                decoder.next_in = input.data();
                decoder.avail_in = actual;
            }

            // 00bbbf95..00bbbfb6 uses remaining-to-read, not avail_in, here.
            const int flush = compressed_remaining == 0 &&
                decoded_remaining <= decoder.avail_out ? Z_FINISH : Z_SYNC_FLUSH;
            const uInt previous_input = decoder.avail_in;
            const std::uint32_t previous_end = output_end;
            z_result = inflate(&decoder, flush);
            output_end = output_capacity - decoder.avail_out;
            // Equal to native total_out delta on successful calls. The buffer
            // extent also accounts for a prefix when stock 1.2.1 fails to
            // allocate its window before updating total_out.
            const std::uint32_t produced = output_end - previous_end;
            if (produced > decoded_remaining) {
                // Native subtracts and wraps. Keep only the declared prefix.
                output_end = previous_end + decoded_remaining;
                decoded_remaining = 0;
                failure = InflateStreamStatus::decoded_size_mismatch;
                break;
            }
            decoded_remaining -= produced;
            if (z_result == Z_STREAM_END) {
                if (decoded_remaining != 0)
                    failure = InflateStreamStatus::decoded_size_mismatch;
                break;
            }
            if (z_result != Z_OK) {
                if (z_result == Z_BUF_ERROR) {
                    // Stock 1.2.1 reports this for unfinished Z_FINISH even
                    // after producing bytes. Publish that block and continue
                    // only after it drains, as native does.
                    if (output_end == 0)
                        failure = InflateStreamStatus::no_progress;
                } else {
                    failure = InflateStreamStatus::decoder_error;
                }
                break;
            }
            if (produced == 0 && decoder.avail_in == previous_input) {
                failure = InflateStreamStatus::no_progress;
                break;
            }
        }
    }

    // Returns ok only when bytes are available. Decoder/source errors take
    // precedence over declared EOF after any published prefix is drained.
    InflateStreamStatus ensure_output() noexcept {
        if (available() != 0) return InflateStreamStatus::ok;
        if (failure != InflateStreamStatus::ok) return failure;
        if (decoded_remaining == 0) return InflateStreamStatus::end_of_stream;
        refill();
        if (available() != 0) return InflateStreamStatus::ok;
        if (failure == InflateStreamStatus::ok)
            failure = InflateStreamStatus::no_progress;
        return failure;
    }

    std::shared_ptr<InflateSource> source;
    InflateStreamDescriptor descriptor;
    std::array<Bytef, input_capacity> input;
    std::array<Bytef, output_capacity> output;
    z_stream decoder{};
    bool decoder_initialized{};
    int z_result{Z_OK};
    InflateStreamStatus failure{InflateStreamStatus::ok};
    std::uint32_t compressed_remaining;
    std::uint32_t decoded_remaining;
    std::uint32_t output_cursor{};
    std::uint32_t output_end{};
    std::uint32_t position{};
};

InflateStream::InflateStream() noexcept = default;
InflateStream::~InflateStream() = default;
InflateStream::InflateStream(InflateStream&&) noexcept = default;
InflateStream& InflateStream::operator=(InflateStream&&) noexcept = default;

InflateStreamStatus InflateStream::open_00bbc1d0_fragment(
    std::shared_ptr<InflateSource> source,
    InflateStreamDescriptor descriptor) noexcept {
    if (impl_) return InflateStreamStatus::already_initialized;
    initialization_decoder_status_ = Z_OK;
    if (!source) return InflateStreamStatus::invalid_argument;
    std::unique_ptr<Impl> candidate(new (std::nothrow) Impl(std::move(source), descriptor));
    if (!candidate) return InflateStreamStatus::allocation_failed;
    candidate->z_result = inflateInit2(&candidate->decoder, -15);
    initialization_decoder_status_ = candidate->z_result;
    if (candidate->z_result != Z_OK)
        return candidate->z_result == Z_MEM_ERROR ? InflateStreamStatus::allocation_failed
            : InflateStreamStatus::decoder_error;
    candidate->decoder_initialized = true;
    if (!candidate->source->seek_absolute(descriptor.source_offset))
        return InflateStreamStatus::source_seek_failed;
    impl_ = std::move(candidate);
    return InflateStreamStatus::ok;
}

InflateStreamStatus InflateStream::read_00bbc140(void* destination,
    std::uint32_t requested, std::uint32_t* actual) noexcept {
    if (actual) *actual = 0;
    if (!impl_) return InflateStreamStatus::not_initialized;
    if (requested == 0) return InflateStreamStatus::ok;
    if (!destination) return InflateStreamStatus::invalid_argument;
    auto* next = static_cast<unsigned char*>(destination);
    std::uint32_t copied = 0;
    while (copied != requested) {
        const InflateStreamStatus available = impl_->ensure_output();
        if (available != InflateStreamStatus::ok) {
            if (actual) *actual = copied;
            return available;
        }
        const std::uint32_t count =
            (std::min)(requested - copied, impl_->available());
        std::memcpy(next + copied, impl_->output.data() + impl_->output_cursor, count);
        impl_->output_cursor += count;
        impl_->position += count;
        copied += count;
    }
    if (actual) *actual = copied;
    return impl_->failure;
}

InflateStreamStatus InflateStream::seek_00bbc060(std::int64_t offset,
    std::uint32_t origin) noexcept {
    if (!impl_) return InflateStreamStatus::not_initialized;
    const std::uint32_t base = origin == 0 ? 0 :
        (origin == 1 ? impl_->position : impl_->descriptor.decoded_size);
    const std::uint32_t target = base + static_cast<std::uint32_t>(offset);
    if (target == impl_->position) return impl_->failure;
    if (target < impl_->position) {
        const std::uint32_t distance = impl_->position - target;
        if (distance > impl_->output_cursor)
            return InflateStreamStatus::unsupported_backward_seek;
        impl_->output_cursor -= distance;
        impl_->position = target;
        return impl_->failure;
    }
    while (impl_->position < target) {
        const InflateStreamStatus available = impl_->ensure_output();
        if (available != InflateStreamStatus::ok) return available;
        const std::uint32_t count =
            (std::min)(target - impl_->position, impl_->available());
        impl_->output_cursor += count;
        impl_->position += count;
    }
    return impl_->failure;
}

std::uint64_t InflateStream::position_00bbbe50() const noexcept {
    return impl_ ? impl_->position : 0;
}

std::uint64_t InflateStream::size_00bbbdd0() const noexcept {
    return impl_ ? impl_->descriptor.decoded_size : 0;
}

int InflateStream::decoder_status() const noexcept {
    return impl_ ? impl_->z_result : initialization_decoder_status_;
}
}
