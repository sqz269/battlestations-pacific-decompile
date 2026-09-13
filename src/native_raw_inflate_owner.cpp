#include "bsp/native_raw_inflate_owner.hpp"

#include "bsp/native_adopted_substream.hpp"
#include "bsp/singleton_lifetime.hpp"
#include <zlib.h>

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Actual raw inflate owner requires MSVC Win32.
#endif

namespace bsp {
namespace {
static_assert(sizeof(z_stream) == 0x38, "Native z_stream is 38h bytes.");
std::uint32_t word(const void* owner, std::uint32_t offset = 0) noexcept {
    return *reinterpret_cast<const volatile std::uint32_t*>(
        reinterpret_cast<std::uintptr_t>(owner) + offset);
}
void* pointer(const void* owner, std::uint32_t offset = 0) noexcept {
    return reinterpret_cast<void*>(word(owner, offset));
}
void put(void* owner, std::uint32_t offset, std::uint32_t value) noexcept {
    *reinterpret_cast<volatile std::uint32_t*>(
        reinterpret_cast<std::uintptr_t>(owner) + offset) = value;
}
void* allocate(std::uint32_t bytes) {
    return singleton_lifetime_allocate({SingletonAllocationKind::object, bytes, bytes});
}
void* construct_buffer(std::uint32_t bytes) {
    // Both inline constructions: state1 or state2 is armed only after the
    // scalar header allocation returns. BF55BE tail-calls the same allocator.
    void* const header = allocate(0x10);
    if (!header) return nullptr;
    try {
        void* const data = allocate(bytes);
        const auto begin = reinterpret_cast<std::uint32_t>(data);
        put(header, 0, begin);
        put(header, 4, begin);
        put(header, 8, begin + bytes);
        put(header, 0xc, begin);
    } catch (...) {
        singleton_lifetime_free(header);
        throw;
    }
    return header;
}
} // namespace

void* construct_native_raw_inflate_owner_00bbc1d0(void* owner,
    void* source, const void* descriptor, std::uint32_t unused_input_capacity,
    std::uint32_t unused_output_capacity, NativeAdoptedSubstreamDispatch& streams) {
    (void)unused_input_capacity;
    (void)unused_output_capacity;
    put(owner, 0, 0x00ceb130);
    put(owner, 4, 1);
    put(owner, 0xc, reinterpret_cast<std::uint32_t>(source));
    put(owner, 0, 0x00d64400);
    *reinterpret_cast<volatile std::uint8_t*>(
        reinterpret_cast<std::uintptr_t>(owner) + 9u) = 1;
    put(owner, 0x10, word(descriptor));
    put(owner, 0x14, word(descriptor, 4));
    const auto decoded = word(descriptor, 8);
    // Native state0 starts immediately before the final descriptor store.
    try {
        put(owner, 0x18, decoded);
        put(owner, 0x1c, reinterpret_cast<std::uint32_t>(construct_buffer(0x4000)));
        put(owner, 0x20, reinterpret_cast<std::uint32_t>(construct_buffer(0x10000)));
        void* const decoder = allocate(0x38);
        put(decoder, 0x20, 0);
        put(decoder, 0x24, 0);
        put(decoder, 0x28, 0);
        put(decoder, 0, 0);
        put(decoder, 4, 0);
        (void)inflateInit2_(static_cast<z_streamp>(decoder), -15, "1.2.1", 0x38);

        auto* const references = reinterpret_cast<volatile LONG*>(
            reinterpret_cast<std::uintptr_t>(pointer(owner, 0xc)) + 4u);
        put(owner, 0x28, reinterpret_cast<std::uint32_t>(decoder));
        (void)InterlockedIncrement(references);
        void* const current_source = pointer(owner, 0xc);
        const void* const current_table = pointer(current_source);
        const auto offset = word(owner, 0x10);
        const auto seek_entry = word(current_table, 0x1c);
        (void)streams.source_seek(seek_entry, current_source, offset, 0, 0);
        const auto remaining_compressed = word(owner, 0x14);
        const auto remaining_decoded = word(owner, 0x18);
        put(owner, 0x2c, remaining_compressed);
        put(owner, 0x24, 0);
        put(owner, 0x30, remaining_decoded);
    } catch (...) {
        // CC4AD0 -> BB86E0 -> BD30F0: only the stream/reference base.
        put(owner, 0, 0x00d5c104);
        put(owner, 0, 0x00ceb130);
        throw;
    }
    return owner;
}
} // namespace bsp
