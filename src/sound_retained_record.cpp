#include "bsp/sound_retained_record.hpp"

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>
#include <cstring>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Retained source copying requires MSVC Win32 x87 operation ordering.
#endif

namespace bsp {
namespace {
template<class T> T read(const void* p, std::size_t offset) noexcept {
    T value;
    std::memcpy(&value, static_cast<const std::byte*>(p) + offset, sizeof value);
    return value;
}
template<class T> void write(void* p, std::size_t offset, T value) noexcept {
    std::memcpy(static_cast<std::byte*>(p) + offset, &value, sizeof value);
}
void retain(void* sample) noexcept {
    InterlockedIncrement(reinterpret_cast<volatile LONG*>(
        static_cast<std::byte*>(sample) + 4));
}
void release(void* sample, GameplayEffectComponentLifetime& lifetime) {
    if (InterlockedDecrement(reinterpret_cast<volatile LONG*>(
            static_cast<std::byte*>(sample) + 4)) == 0)
        lifetime.zero_references_slot_00(sample);
}
void destroy_sample_slot(void* slot, GameplayEffectComponentLifetime& lifetime) {
    void* const captured = read<void*>(slot, 0);
    if (captured) {
        release(captured, lifetime);
        write<void*>(slot, 0, nullptr);
    }
}
struct SampleUnwind {
    void* slot;
    GameplayEffectComponentLifetime& lifetime;
    bool armed{true};
    ~SampleUnwind() noexcept {
        if (armed) destroy_sample_slot(slot, lifetime);
    }
};
} // namespace

void* copy_sound_retained_source_record_00a7c5f0(void* destination,
    const void* source, NativeStringStorage& strings, GameplayEffectComponentLifetime& lifetime) {
    write<void*>(destination, 0, nullptr);
    void* const sample = read<void*>(source, 0);
    if (sample) {
        write(destination, 0, sample);
        retain(sample);
    }
    // Native state0: CB51A0 ->004C3810 releases only the actual sample slot.
    SampleUnwind unwind{destination, lifetime};
    auto* const name = static_cast<std::byte*>(destination) + 4;
    const auto* const source_name = static_cast<const std::byte*>(source) + 4;
    write<std::uint32_t>(name, 0, 0);
    write<void*>(name, 4, nullptr);
    if (name != source_name) {
        resize_native_string_header_0041dd40(name, strings,
            read<std::uint32_t>(source_name, 0), true);
        if (read<std::uint32_t>(source_name, 0) != 0) {
            const auto length = read<std::uint32_t>(name, 0);
            // Native BF7680 also handles overlapping copies. Reload these
            // fields after allocator callbacks, without snapshotting source.
            if (length) std::memmove(read<void*>(name, 4),
                read<const void*>(source_name, 4), length);
        }
    }
    // Preserve FLD/FSTP, including signaling-NaN handling, rather than a
    // bitwise/SSE copy. +10 is loaded only after the +C destination store.
    __asm {
        mov eax, source
        mov edx, destination
        fld dword ptr [eax + 0xc]
        fstp dword ptr [edx + 0xc]
        fld dword ptr [eax + 0x10]
        fstp dword ptr [edx + 0x10]
    }
    unwind.armed = false;
    return destination;
}

void destroy_sound_retained_source_record_004c7fa0(void* record,
    NativeStringStorage& strings, GameplayEffectComponentLifetime& lifetime) {
    destroy_native_string_header_0041dd20(static_cast<std::byte*>(record) + 4, strings);
    destroy_sample_slot(record, lifetime);
}

void* assign_sound_sample_reference_004e7bb0(void* destination,
    const void* source, GameplayEffectComponentLifetime& lifetime) {
    void* const incoming = read<void*>(source, 0);
    void* const old = read<void*>(destination, 0);
    if (old != incoming) {
        write(destination, 0, incoming);
        if (incoming) retain(incoming);
        if (old) release(old, lifetime);
    }
    return destination;
}
} // namespace bsp
