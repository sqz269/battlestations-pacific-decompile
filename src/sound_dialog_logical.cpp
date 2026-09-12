#include "bsp/sound_dialog_logical.hpp"
#include "bsp/native_pooled_string_substring.hpp"
#include "bsp/singleton_lifetime.hpp"
#include "bsp/sound_alternate_owner.hpp"
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>
#include <cstddef>
#include <cstring>
#include <stdexcept>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Logical sound reconstruction requires MSVC Win32 and x87.
#endif

namespace bsp {
namespace {
template<class T> T read(const void* p, std::size_t n) noexcept {
    T value; std::memcpy(&value, static_cast<const std::byte*>(p) + n, sizeof value);
    return value;
}
template<class T> void write(void* p, std::size_t n, T value) noexcept {
    std::memcpy(static_cast<std::byte*>(p) + n, &value, sizeof value);
}
void* at(void* p, std::size_t n) noexcept { return static_cast<std::byte*>(p) + n; }
const void* at(const void* p, std::size_t n) noexcept {
    return static_cast<const std::byte*>(p) + n;
}
void retain(void* p) noexcept {
    if (p) InterlockedIncrement(reinterpret_cast<volatile LONG*>(at(p, 4)));
}
void release(void* p, GameplayEffectComponentLifetime& refs) {
    if (p && InterlockedDecrement(reinterpret_cast<volatile LONG*>(at(p, 4))) == 0)
        refs.zero_references_slot_00(p);
}
void copy_reference(void* destination, const void* source,
    GameplayEffectComponentLifetime& refs) {
    void* const incoming = read<void*>(source, 0);
    void* const previous = read<void*>(destination, 0);
    if (previous != incoming) {
        write(destination, 0, incoming); // Published before either count change.
        retain(incoming);
        release(previous, refs); // No write after the zero-reference callback.
    }
}
float current_logical_gain(void* p, SoundDialogLogicalBindings& b) {
    switch (read<std::uint32_t>(p, 0)) {
    case 0x00d58e60: return sound_dialog_primary_gain_00a77820(b.global_00f8bbcc);
    case 0x00d58e64: return sound_dialog_secondary_gain_00a77830(b.global_00f8bbcc);
    default: throw std::logic_error("Unsupported logical dialog gain vtable");
    }
}
} // namespace

void clear_sound_dialog_configuration_references_00a77bb0(void* p,
    GameplayEffectComponentLifetime& refs) {
    if (void* const previous = read<void*>(p, 0x10)) {
        release(previous, refs);
        write<void*>(p, 0x10, nullptr);
    }
    write<void*>(p, 0x10, nullptr);
    release(read<void*>(p, 0x18), refs);
    write<void*>(p, 0x18, nullptr);
}
void* copy_sound_dialog_record_00a78230(void* p, const void* source,
    NativeStringStorage& strings, GameplayEffectComponentLifetime& refs) {
    copy_native_string_header_00be0a30_fragment(p, strings, source);
    copy_native_string_header_00be0a30_fragment(at(p, 8), strings, at(source, 8));
    copy_reference(at(p, 0x10), at(source, 0x10), refs);
    // Native FLD/FSTP, including quieting a signaling NaN under the current CW.
    const void* const input = at(source, 0x14);
    void* const output = at(p, 0x14);
    __asm {
        mov eax, input
        fld dword ptr [eax]
        mov eax, output
        fstp dword ptr [eax]
    }
    return p;
}
void* copy_sound_dialog_configuration_00a785d0(void* p, const void* source,
    NativeStringStorage& strings, GameplayEffectComponentLifetime& refs) {
    copy_sound_dialog_record_00a78230(p, source, strings, refs);
    copy_reference(at(p, 0x18), at(source, 0x18), refs);
    return p;
}
std::int32_t find_sound_dialog_record_00a865f0(void* table,
    const void* name) noexcept {
    auto row = reinterpret_cast<std::uintptr_t>(read<void*>(table, 8));
    const auto end = row + read<std::uint32_t>(table, 0xc) * 0x14u;
    std::uint32_t index = 0;
    while (row != end) {
        const void* const entry = reinterpret_cast<const void*>(row);
        const auto length = read<std::uint32_t>(entry, 0);
        if (length == read<std::uint32_t>(name, 0) &&
            (length == 0 || _stricmp(read<const char*>(entry, 4),
                read<const char*>(name, 4)) == 0))
            return static_cast<std::int32_t>(index);
        row += 0x14u; ++index;
    }
    return 0;
}
float sound_dialog_primary_gain_00a77820(void* volatile& global) noexcept {
    return read<float>(global, 0x218);
}
float sound_dialog_secondary_gain_00a77830(void* volatile& global) noexcept {
    return read<float>(global, 0x21c);
}

void start_sound_dialog_logical_00a783f0(void* p, SoundDialogLogicalBindings& b) {
    if (read<std::uint8_t>(p, 0x4c) != 0) {
        // 004254B0 is RET: the diagnostic call has no state effect.
        write<std::uint8_t>(p, 0x4c, 0);
        if (void* const previous = read<void*>(p, 4)) {
            release(previous, b.references);
            write<void*>(p, 4, nullptr);
        }
        void* const storage = singleton_lifetime_allocate(
            {SingletonAllocationKind::object, 0x54, 0x54});
        void* constructed = nullptr;
        if (storage) {
            void* const table_argument = read<void*>(p, 0x28);
            retain(table_argument);
            NativeString name_argument;
            try {
                copy_construct_native_string_header_00426060(
                    &name_argument, at(p, 0x10), b.strings);
            } catch (...) {
                // DEAC80 state1 -> CB4DBB/52E020, then state0 -> CB4DB0.
                // Failed string construction has no string cleanup state.
                release(table_argument, b.references);
                singleton_lifetime_free(storage);
                throw;
            }
            try {
                // Callee owns both argument destructors from entry, even on
                // failure; caller state0 owns only the receiver allocation.
                constructed = b.streams.construct_stream_00a877d0(
                    storage, name_argument, table_argument);
            } catch (...) {
                singleton_lifetime_free(storage);
                throw;
            }
        }
        write(p, 4, constructed);
        b.streams.start_stream_00a867b0(constructed, at(p, 0x18));
        copy_native_string_header_00be0a30_fragment(at(p, 8), b.strings, at(p, 0x18));
        std::int32_t row = 0;
        if (read<std::uint32_t>(p, 0x18) != 0)
            row = find_sound_dialog_record_00a865f0(
                read<void*>(read<void*>(p, 4), 0x3c), at(p, 0x18));
        const void* const gain_source = at(p, 0x24);
        float gain;
        __asm {
            mov eax, gain_source
            fld dword ptr [eax]
            fstp gain
        }
        b.streams.set_stream_row_gain_00a86670(read<void*>(p, 4), row, gain);
        clear_sound_dialog_configuration_references_00a77bb0(at(p, 0x10), b.references);
        return;
    }
    void* const stream = read<void*>(p, 4);
    if (stream && (read<std::int32_t>(stream, 0x20) == 0 ||
        read<std::int32_t>(stream, 0x20) == 3)) {
        NativeString diagnostic_name;
        copy_sound_stream_name_00a77ff0(stream, diagnostic_name, b.strings);
        // The native diagnostic constructs and releases this even though
        // 004254B0 itself is one RET. No destructor if the copy throws.
        destroy_native_string_header_0041dd20(&diagnostic_name, b.strings);
        b.streams.start_stream_00a867b0(read<void*>(p, 4), at(p, 8));
    }
}

void update_sound_dialog_logical_00a78820(void* p, float dt, float gain,
    SoundDialogLogicalBindings& b) {
    if (read<std::uint8_t>(p, 0x4d) != 0) {
        void* const timer = at(p, 0x2c);
        float rounded;
        unsigned char expired;
        __asm {
            mov eax, timer
            fld dword ptr [eax]
            fsub dt
            fstp rounded
            fld rounded
            fst dword ptr [eax]
            fldz
            fcomip st, st(1)
            fstp st(0)
            setae expired
        }
        if (expired) {
            write<std::uint8_t>(p, 0x4d, 0);
            write<std::uint8_t>(p, 0x4c, 1);
            copy_sound_dialog_configuration_00a785d0(
                at(p, 0x10), at(p, 0x30), b.strings, b.references);
            clear_sound_dialog_configuration_references_00a77bb0(at(p, 0x30), b.references);
        }
    }
    if (read<void*>(p, 4)) {
        const float base_gain = current_logical_gain(p, b);
        const void* const local_gain = at(p, 0x54);
        float combined;
        __asm {
            fld base_gain
            mov eax, local_gain
            fmul dword ptr [eax]
            fmul gain
            fstp combined
        }
        b.streams.set_stream_gain_00a864f0(read<void*>(p, 4), combined);
        float forwarded_dt;
        __asm {
            fld dt
            fstp forwarded_dt
        }
        b.streams.update_stream_00a874d0(read<void*>(p, 4), forwarded_dt);
    }
}
} // namespace bsp
