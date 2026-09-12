#include "bsp/native_adopted_substream.hpp"
#include "bsp/singleton_lifetime.hpp"

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native adopted substreams require MSVC Win32.
#endif

namespace bsp {
namespace {
static_assert(sizeof(void*) == 4 && sizeof(LONG) == 4);
std::uint32_t word(const volatile void* base, std::uint32_t byte_offset = 0) noexcept {
    std::uint32_t value;
    __asm {
        mov eax, base
        mov edx, byte_offset
        mov eax, dword ptr [eax + edx]
        mov value, eax
    }
    return value;
}
void put(void* base, std::uint32_t byte_offset, std::uint32_t value) noexcept {
    __asm {
        mov eax, base
        mov edx, byte_offset
        mov ecx, value
        mov dword ptr [eax + edx], ecx
    }
}
void* pointer(std::uint32_t value) noexcept { return reinterpret_cast<void*>(value); }
std::uintptr_t current_method(void* source, std::uint32_t offset) noexcept {
    const auto table = word(source);
    return word(pointer(table), offset);
}
std::uint64_t wide(const void* owner, std::uint32_t offset) noexcept {
    const auto low = word(owner, offset);
    const auto high = word(owner, offset + 4);
    return (std::uint64_t{high} << 32) | low;
}
void destroy_stream_base(void* owner) noexcept {
    // BB86E0 -> BD30F0, also both actual FH3 unwind actions.
    put(owner, 0, 0x00d5c104);
    put(owner, 0, 0x00ceb130);
}
struct StreamBaseUnwind {
    void* owner;
    bool armed = true;
    ~StreamBaseUnwind() noexcept { if (armed) destroy_stream_base(owner); }
};
void advance(void* owner, std::uint32_t actual) noexcept {
    const auto old_low = word(owner, 0x20);
    const auto low = old_low + actual;
    put(owner, 0x20, low);
    put(owner, 0x24, word(owner, 0x24) + std::uint32_t{low < old_low});
}
class CallableDispatch final : public NativeAdoptedSubstreamDispatch {
public:
    std::uint8_t source_is_open(std::uintptr_t entry, void* source) override {
        using Fn = std::uint8_t (__fastcall*)(void*, void*);
        return reinterpret_cast<Fn>(entry)(source, pointer(static_cast<std::uint32_t>(entry)));
    }
    std::uint32_t source_seek(std::uintptr_t entry, void* source, std::uint32_t low,
        std::uint32_t high, std::uint32_t origin) override {
        using Fn = std::uint32_t (__fastcall*)(void*, void*, std::uint32_t,
            std::uint32_t, std::uint32_t);
        return reinterpret_cast<Fn>(entry)(source, pointer(high), low, high, origin);
    }
    void source_read(std::uintptr_t entry, void* source, void* destination,
        std::uint32_t requested, std::uint32_t* actual) override {
        using Fn = void (__fastcall*)(void*, void*, void*, std::uint32_t, std::uint32_t*);
        reinterpret_cast<Fn>(entry)(source, destination, destination, requested, actual);
    }
    void source_write(std::uintptr_t entry, void* source, const void* bytes,
        std::uint32_t requested, std::uint32_t* actual) override {
        using Fn = void (__fastcall*)(void*, const void*, const void*, std::uint32_t, std::uint32_t*);
        reinterpret_cast<Fn>(entry)(source, bytes, bytes, requested, actual);
    }
    void source_zero_reference(std::uintptr_t entry, void* source,
        std::uintptr_t captured_table) override {
        using Fn = void (__fastcall*)(void*, void*);
        reinterpret_cast<Fn>(entry)(source, pointer(static_cast<std::uint32_t>(captured_table)));
    }
};
} // namespace

NativeAdoptedSubstreamDispatch& callable_native_adopted_substream_dispatch() noexcept {
    static CallableDispatch dispatch;
    return dispatch;
}
bool query_native_file_stream_type_00bb8b80(std::uint32_t token,
    const volatile std::uint32_t* ids) noexcept {
    for (std::uint32_t index = 0; index != 2; ++index)
        if (ids[index] == token) return true;
    return false;
}
void* construct_native_adopted_substream_00bf1130(void* owner, void* source,
    std::uint32_t start_low, std::uint32_t start_high, std::uint32_t length_low,
    std::uint32_t length_high, NativeAdoptedSubstreamDispatch& dispatch) {
    put(owner, 0, 0x00ceb130);
    put(owner, 4, 1);
    const auto end_low = start_low + length_low;
    const auto end_high = start_high + length_high + std::uint32_t{end_low < start_low};
    put(owner, 0x10, start_low);
    put(owner, 0, 0x00d68db0);
    put(owner, 8, static_cast<std::uint32_t>(reinterpret_cast<std::uintptr_t>(source)));
    put(owner, 0x14, start_high);
    put(owner, 0x18, end_low);
    put(owner, 0x1c, end_high);
    const auto entry = current_method(source, 0x1c);
    StreamBaseUnwind cleanup{owner}; // State0 is armed only after slot lookup.
    (void)dispatch.source_seek(entry, source, start_low, start_high, 0);
    const auto current_low = word(owner, 0x10);
    const auto current_high = word(owner, 0x14);
    put(owner, 0x20, current_low);
    put(owner, 0x24, current_high);
    cleanup.armed = false;
    return owner;
}
std::uint32_t read_native_adopted_substream_00bf1000(void* owner, void* destination,
    std::uint32_t requested, std::uint32_t* optional_actual,
    NativeAdoptedSubstreamDispatch& dispatch) {
    auto actual = requested;
    auto* const source = pointer(word(owner, 8));
    const auto entry = current_method(source, 0x24);
    dispatch.source_read(entry, source, destination, requested, &actual);
    advance(owner, actual);
    if (optional_actual) put(optional_actual, 0, actual);
    return actual;
}
std::uint32_t write_native_adopted_substream_00bf1040(void* owner, const void* bytes,
    std::uint32_t requested, std::uint32_t* optional_actual,
    NativeAdoptedSubstreamDispatch& dispatch) {
    auto actual = requested;
    auto* const source = pointer(word(owner, 8));
    const auto entry = current_method(source, 0x28);
    dispatch.source_write(entry, source, bytes, requested, &actual);
    advance(owner, actual);
    if (optional_actual) put(optional_actual, 0, actual);
    return actual;
}
std::uint64_t position_native_adopted_substream_00bf1080(const void* owner) noexcept {
    const auto current_low = word(owner, 0x20);
    const auto start_low = word(owner, 0x10);
    const auto low = current_low - start_low;
    const auto current_high = word(owner, 0x24);
    const auto start_high = word(owner, 0x14);
    const auto high = current_high - start_high - std::uint32_t{current_low < start_low};
    return (std::uint64_t{high} << 32) | low;
}
std::uint64_t length_native_adopted_substream_00bf10a0(const void* owner) noexcept {
    const auto end_low = word(owner, 0x18);
    const auto start_low = word(owner, 0x10);
    const auto low = end_low - start_low;
    const auto end_high = word(owner, 0x1c);
    const auto start_high = word(owner, 0x14);
    const auto high = end_high - start_high - std::uint32_t{end_low < start_low};
    return (std::uint64_t{high} << 32) | low;
}
std::uint64_t origin_native_adopted_substream_00bf10b0(
    const void* owner, std::uint32_t origin) noexcept {
    return wide(owner, origin == 0 ? 0x10u : origin == 1 ? 0x20u : 0x18u);
}
std::uint8_t open_native_adopted_substream_00bf1090(
    void* owner, NativeAdoptedSubstreamDispatch& dispatch) {
    auto* const source = pointer(word(owner, 8));
    const auto entry = current_method(source, 0x18);
    return dispatch.source_is_open(entry, source);
}
std::uint32_t seek_native_adopted_substream_00bf10e0(void* owner,
    std::uint32_t low, std::uint32_t high, std::uint32_t origin,
    NativeAdoptedSubstreamDispatch& dispatch) {
    const auto base = origin_native_adopted_substream_00bf10b0(owner, origin);
    const auto position = base + ((std::uint64_t{high} << 32) | low);
    const auto absolute_low = static_cast<std::uint32_t>(position);
    const auto absolute_high = static_cast<std::uint32_t>(position >> 32);
    put(owner, 0x20, absolute_low);
    put(owner, 0x24, absolute_high);
    auto* const source = pointer(word(owner, 8));
    const auto entry = current_method(source, 0x1c);
    return dispatch.source_seek(entry, source, absolute_low, absolute_high, 0);
}
void destroy_native_adopted_substream_00bf11c0(void* owner,
    NativeAdoptedSubstreamDispatch& dispatch) {
    put(owner, 0, 0x00d68db0);
    auto* const source = pointer(word(owner, 8));
    StreamBaseUnwind cleanup{owner};
    if (source) {
        auto* const refs = reinterpret_cast<volatile LONG*>(
            reinterpret_cast<std::uintptr_t>(source) + 4u);
        if (InterlockedDecrement(refs) == 0) {
            const auto table = word(source);
            const auto entry = word(pointer(table));
            dispatch.source_zero_reference(entry, source, table);
        }
        put(owner, 8, 0);
    }
    cleanup.armed = false;
    destroy_stream_base(owner);
}
void* delete_native_adopted_substream_00bf1240(void* owner,
    std::uint32_t flags, NativeAdoptedSubstreamDispatch& dispatch) {
    destroy_native_adopted_substream_00bf11c0(owner, dispatch);
    if ((flags & 1u) != 0) singleton_lifetime_free(owner);
    return owner;
}
} // namespace bsp
