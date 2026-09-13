#pragma once
#include <cstdint>

namespace bsp {
class NativeAdoptedSubstreamDispatch;

// Actual D64400 owner (34h), constructed by BBC1D0. Buffer headers are 10h:
// begin/end/capacity/current at +0/+4/+8/+C. Decoder is the original 38h
// Win32 z_stream layout, supplied by the existing stock zlib 1.2.1 library.
std::uint8_t open_native_raw_inflate_stream_00bbbdc0(const void*) noexcept;
std::uint64_t length_native_raw_inflate_stream_00bbbdd0(const void*) noexcept;
std::uint64_t position_native_raw_inflate_stream_00bbbe50(const void*) noexcept;

// BBBE10: reset decoder, seek CURRENT source to descriptor offset, reset three
// counters; retain stale input/output buffer cursors. EAX is forwarded from seek.
std::uint32_t reset_native_raw_inflate_stream_00bbbe10(void*, NativeAdoptedSubstreamDispatch&);
// BBBF00: reset decoder output, refill input on exhaustion, consume raw inflate
// status/counters and publish output. No added malformed-input/progress guard.
void refill_native_raw_inflate_stream_00bbbf00(void*, NativeAdoptedSubstreamDispatch&);
// BBC060: low DWORD wrapping target; high ignored; origins 0/1/other select
// zero/current/length. Preserve native stale-buffer rewind and incidental EAX:
// initial base, accepted backward cursor, reset's source EAX, or buffer pointer.
std::uint32_t seek_native_raw_inflate_stream_00bbc060(void*, std::uint32_t low,
    std::uint32_t ignored_high, std::uint32_t origin, NativeAdoptedSubstreamDispatch&);
// BBC140: ECX owner; destination/request/optional count; RET0C; EAX count pointer.
std::uint32_t* read_native_raw_inflate_stream_00bbc140(void*, void*,
    std::uint32_t requested, std::uint32_t* actual, NativeAdoptedSubstreamDispatch&);
// BBC1C0 is exactly RET0C: it does not touch even an optional count output.
void write_native_raw_inflate_stream_00bbc1c0(void*, const void*,
    std::uint32_t, std::uint32_t*) noexcept;
// BBC320: release captured source then clear current source field, end/free
// current decoder and captured buffer owners, then restore stream/base profiles.
// Native EH owns only base reset. No remaining-buffer cleanup on failure.
void destroy_native_raw_inflate_stream_00bbc320(void*, NativeAdoptedSubstreamDispatch&);
void* delete_native_raw_inflate_stream_00bbc3e0(void*, std::uint32_t flags,
    NativeAdoptedSubstreamDispatch&);

// New C++ interfaces, not original ABI/FH3 replacements. Original stack spill
// aliasing, simultaneous cleanup exceptions and gameplay require separate proof.
}
