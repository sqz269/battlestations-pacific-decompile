#include "bsp/native_raw_inflate_stream.hpp"
#include "bsp/native_adopted_substream.hpp"
#include "bsp/singleton_lifetime.hpp"
#include <cstring>
#include <zlib.h>
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native raw inflate streams require MSVC Win32.
#endif

namespace bsp {
namespace {
static_assert(sizeof(void*) == 4 && sizeof(z_stream) == 0x38);
std::uint32_t word(const void* p, std::uint32_t offset = 0) noexcept {
    return *reinterpret_cast<const volatile std::uint32_t*>(reinterpret_cast<std::uintptr_t>(p) + offset);
}
void put(void* p, std::uint32_t offset, std::uint32_t value) noexcept {
    *reinterpret_cast<volatile std::uint32_t*>(reinterpret_cast<std::uintptr_t>(p) + offset) = value;
}
void* pointer(const void* p, std::uint32_t offset = 0) noexcept {
    return reinterpret_cast<void*>(word(p, offset));
}
std::uint32_t bits(const void* p) noexcept { return reinterpret_cast<std::uint32_t>(p); }
void base(void* p) noexcept { put(p, 0, 0x00d5c104); put(p, 0, 0x00ceb130); }
}

std::uint8_t open_native_raw_inflate_stream_00bbbdc0(const void* owner) noexcept {
    return *reinterpret_cast<const volatile std::uint8_t*>(reinterpret_cast<std::uintptr_t>(owner) + 9u);
}
std::uint64_t length_native_raw_inflate_stream_00bbbdd0(const void* owner) noexcept { return word(owner, 0x18); }
std::uint64_t position_native_raw_inflate_stream_00bbbe50(const void* owner) noexcept { return word(owner, 0x24); }

std::uint32_t reset_native_raw_inflate_stream_00bbbe10(void* owner, NativeAdoptedSubstreamDispatch& streams) {
    (void)inflateReset(static_cast<z_streamp>(pointer(owner, 0x28)));
    auto* source = pointer(owner, 0x0c);
    auto* table = pointer(source);
    const auto offset = word(owner, 0x10);
    const auto target = word(table, 0x1c);
    const auto result = streams.source_seek(target, source, offset, 0, 0);
    const auto compressed = word(owner, 0x14);
    const auto decoded = word(owner, 0x18);
    put(owner, 0x24, 0);
    put(owner, 0x2c, compressed);
    put(owner, 0x30, decoded);
    return result;
}

void refill_native_raw_inflate_stream_00bbbf00(void* owner, NativeAdoptedSubstreamDispatch& streams) {
    auto* output = pointer(owner, 0x20);
    const auto beginning = word(output);
    put(pointer(owner, 0x28), 0x0c, beginning);
    output = pointer(owner, 0x20);
    const auto capacity = word(output, 8) - word(output);
    put(pointer(owner, 0x28), 0x10, capacity);
    // PUSH ECX at BBBF00 initializes the one stack DWORD reused for actual read.
    std::uint32_t actual = bits(owner);
    while (word(pointer(owner, 0x28), 0x10) != 0) {
        auto* input = pointer(owner, 0x1c);
        const auto available = word(input, 4) - word(input, 0x0c);
        if (available == 0 && word(owner, 0x2c) > available) {
            auto requested = word(input, 8) - word(input);
            const auto remaining = word(owner, 0x2c);
            if (remaining < requested) requested = remaining;
            if (!requested) return; // Native early exit does not publish output.
            auto* source = pointer(owner, 0x0c);
            auto* table = pointer(source);
            auto* destination = pointer(input);
            const auto target = word(table, 0x24);
            streams.source_read(target, source, destination, requested, &actual);
            input = pointer(owner, 0x1c);
            const auto begin = word(input);
            put(input, 0x0c, begin);
            put(input, 4, actual + begin);
            const auto consumed = actual;
            input = pointer(owner, 0x1c);
            put(owner, 0x2c, word(owner, 0x2c) - consumed);
            auto* decoder = pointer(owner, 0x28);
            put(decoder, 0, word(input));
            put(pointer(owner, 0x28), 4, actual);
        }
        int flush = 2;
        if (word(owner, 0x2c) == 0) {
            auto* decoder = pointer(owner, 0x28);
            if (word(owner, 0x30) <= word(decoder, 0x10)) flush = 4;
        }
        auto* decoder = pointer(owner, 0x28);
        const auto before = word(decoder, 0x14);
        const int status = inflate(static_cast<z_streamp>(decoder), flush);
        decoder = pointer(owner, 0x28);
        const auto produced = word(decoder, 0x14) - before;
        const auto current_input = word(decoder);
        put(pointer(owner, 0x1c), 0x0c, current_input);
        put(owner, 0x30, word(owner, 0x30) - produced);
        if (status != 0) break;
    }
    output = pointer(owner, 0x20);
    auto* decoder = pointer(owner, 0x28);
    const auto end = word(output, 8) - word(decoder, 0x10);
    const auto begin = word(output);
    put(output, 4, end);
    put(output, 0x0c, begin);
}

std::uint32_t seek_native_raw_inflate_stream_00bbc060(void* owner, std::uint32_t low,
    std::uint32_t ignored_high, std::uint32_t origin, NativeAdoptedSubstreamDispatch& streams) {
    (void)ignored_high;
    std::uint32_t result = origin == 0 ? 0 : word(owner, origin == 1 ? 0x24 : 0x18);
    const auto target = low + result;
    auto position = word(owner, 0x24);
    if (target == position) return result;
    if (target < position) {
        auto* output = pointer(owner, 0x20);
        result = target - position + word(output, 0x0c);
        if (word(output) <= result && result <= word(output, 4)) {
            put(output, 0x0c, result);
            put(owner, 0x24, target);
            return result;
        }
        result = reset_native_raw_inflate_stream_00bbbe10(owner, streams);
        position = word(owner, 0x24);
        if (target == position) return result;
    }
    if (position >= target) return result;
    do {
        auto* output = pointer(owner, 0x20);
        result = bits(output);
        const auto available_before = word(output, 4) - word(output, 0x0c);
        if (!available_before) {
            if (word(owner, 0x30) == available_before) return result;
            refill_native_raw_inflate_stream_00bbbf00(owner, streams);
        }
        output = pointer(owner, 0x20);
        result = bits(output);
        const auto end = word(output, 4);
        position = word(owner, 0x24);
        const auto available = end - word(output, 0x0c);
        const auto requested = target - position;
        if (requested > available) {
            const auto consumed = word(output, 4) - word(output, 0x0c);
            put(owner, 0x24, consumed + position);
            put(output, 0x0c, word(output, 4));
        } else {
            const auto cursor = word(output, 0x0c) + requested;
            if (word(output) <= cursor && cursor <= word(output, 4)) put(output, 0x0c, cursor);
            put(owner, 0x24, word(owner, 0x24) + requested);
        }
    } while (word(owner, 0x24) < target);
    return result;
}

std::uint32_t* read_native_raw_inflate_stream_00bbc140(void* owner, void* destination,
    std::uint32_t requested, std::uint32_t* actual, NativeAdoptedSubstreamDispatch& streams) {
    const auto original = bits(destination);
    auto next = original;
    while (requested) {
        auto* output = pointer(owner, 0x20);
        const auto available = word(output, 4) - word(output, 0x0c);
        if (!available) {
            if (word(owner, 0x30) == available) break;
            refill_native_raw_inflate_stream_00bbbf00(owner, streams);
        }
        output = pointer(owner, 0x20);
        const auto source = word(output, 0x0c);
        auto count = word(output, 4) - source;
        if (requested < count) count = requested;
        std::memmove(reinterpret_cast<void*>(next), reinterpret_cast<const void*>(source), count);
        put(output, 0x0c, word(output, 0x0c) + count);
        next += count;
        put(owner, 0x24, word(owner, 0x24) + count);
        requested -= count;
    }
    if (actual) *actual = next - original;
    return actual;
}

void write_native_raw_inflate_stream_00bbc1c0(void*, const void*, std::uint32_t, std::uint32_t*) noexcept {}

void destroy_native_raw_inflate_stream_00bbc320(void* owner, NativeAdoptedSubstreamDispatch& streams) {
    put(owner, 0, 0x00d64400);
    auto* const source = pointer(owner, 0x0c);
    try {
        if (source) {
            auto* references = reinterpret_cast<volatile LONG*>(reinterpret_cast<std::uintptr_t>(source) + 4u);
            if (InterlockedDecrement(references) == 0) {
                auto* table = pointer(source);
                const auto entry = word(table);
                streams.source_zero_reference(entry, source, bits(table));
            }
            put(owner, 0x0c, 0);
        }
        (void)inflateEnd(static_cast<z_streamp>(pointer(owner, 0x28)));
        singleton_lifetime_free(pointer(owner, 0x28));
        auto* input = pointer(owner, 0x1c);
        if (input) { singleton_lifetime_free(pointer(input)); singleton_lifetime_free(input); }
        auto* output = pointer(owner, 0x20);
        if (output) { singleton_lifetime_free(pointer(output)); singleton_lifetime_free(output); }
    } catch (...) { base(owner); throw; }
    base(owner);
}

void* delete_native_raw_inflate_stream_00bbc3e0(void* owner, std::uint32_t flags,
    NativeAdoptedSubstreamDispatch& streams) {
    destroy_native_raw_inflate_stream_00bbc320(owner, streams);
    if (flags & 1u) singleton_lifetime_free(owner);
    return owner;
}
}
