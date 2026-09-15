#include "bsp/native_vehicle_camera_array.hpp"
#include "bsp/native_string.hpp"
#include "bsp/singleton_lifetime.hpp"

#include <cstring>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native vehicle camera arrays require MSVC Win32.
#endif

namespace bsp {
namespace {
using Word = std::uint32_t;
static_assert(sizeof(void*) == 4);
Word address(const void* value) noexcept { return reinterpret_cast<Word>(value); }
void* pointer(Word value) noexcept { return reinterpret_cast<void*>(value); }
Word read(Word base, Word byte_offset = 0) noexcept {
    Word result;
    __asm {
        mov eax, base
        mov edx, byte_offset
        mov eax, dword ptr [eax + edx]
        mov result, eax
    }
    return result;
}
void write(Word base, Word byte_offset, Word value) noexcept {
    __asm {
        mov eax, base
        mov edx, byte_offset
        mov ecx, value
        mov dword ptr [eax + edx], ecx
    }
}
void copy_row_tail(Word destination, Word source) noexcept {
    __asm {
        mov edx, destination
        mov eax, source
        lea esi, [eax + 8]
        lea edi, [edx + 8]
        mov ecx, 16
        rep movsd
        mov ecx, dword ptr [eax + 48h]
        mov dword ptr [edx + 48h], ecx
        mov ecx, dword ptr [eax + 4ch]
        mov dword ptr [edx + 4ch], ecx
        mov ecx, dword ptr [eax + 50h]
        mov dword ptr [edx + 50h], ecx
        mov ecx, dword ptr [eax + 54h]
        mov dword ptr [edx + 54h], ecx
        mov cl, byte ptr [eax + 58h]
        mov byte ptr [edx + 58h], cl
    }
}
void copy_limit(Word destination, const volatile Word& source) noexcept {
    const volatile Word* cell = &source;
    __asm {
        mov eax, cell
        mov edx, destination
        movss xmm0, dword ptr [eax]
        movss dword ptr [edx], xmm0
    }
}
} // namespace

void* initialize_native_vehicle_camera_limits_0078e230(
    void* actual_four_words, const NativeVehicleCameraDefaults& defaults) noexcept {
    const Word destination = address(actual_four_words);
    copy_limit(destination, defaults.word_00ce684c);
    copy_limit(destination + 4u, defaults.word_00d7a264);
    copy_limit(destination + 8u, defaults.word_00ce3ccc);
    copy_limit(destination + 12u, defaults.word_00ce3c64);
    return actual_four_words;
}

void* copy_native_vehicle_camera_row_005cd070(
    void* actual_destination, const void* actual_source,
    NativeStringRawPoolContext& strings) {
    const Word destination = address(actual_destination);
    const Word source = address(actual_source);
    const bool same = destination == source;
    write(destination, 0, 0);
    write(destination, 4, 0);
    if (!same) {
        resize_native_string_header_0041dd40(
            actual_destination, strings, read(source), true);
        if (read(source)) {
            const Word bytes = read(destination);
            const Word source_data = read(source, 4);
            const Word destination_data = read(destination, 4);
            // Native BF7680 supports overlap. No buffer access for zero bytes.
            if (bytes) std::memmove(pointer(destination_data), pointer(source_data), bytes);
        }
    }
    copy_row_tail(destination, source);
    return actual_destination;
}

void reserve_native_vehicle_camera_array_005cd260(
    void* actual_header, std::int32_t requested, NativeStringRawPoolContext& strings) {
    const Word header = address(actual_header);
    const std::int32_t capacity = requested < 1 ? 1 : requested;
    if (static_cast<std::int32_t>(read(header, 8)) >= capacity) return;
    const Word bytes = static_cast<Word>(capacity) * 0x5cu;
    const Word replacement = address(singleton_lifetime_allocate(
        {SingletonAllocationKind::object, bytes, bytes}));
    // The native EH state invokes RET-only00401130, with no rollback.
    for (Word i = 0;
         static_cast<std::int32_t>(i) < static_cast<std::int32_t>(read(header, 4)); ++i) {
        const Word row_offset = i * 0x5cu;
        const Word destination = replacement + row_offset;
        if (destination) copy_native_vehicle_camera_row_005cd070(
            pointer(destination), pointer(read(header) + row_offset), strings);
    }
    Word row_offset = 0;
    for (Word i = 0;
         static_cast<std::int32_t>(i) < static_cast<std::int32_t>(read(header, 4));
         ++i, row_offset += 0x5cu) {
        destroy_native_string_header_0041dd20(pointer(read(header) + row_offset), strings);
    }
    singleton_lifetime_free(pointer(read(header)));
    write(header, 0, replacement);
    write(header, 8, static_cast<Word>(capacity));
}

void resize_native_vehicle_camera_array_005cd640(
    void* actual_header, std::int32_t requested, NativeStringRawPoolContext& strings,
    const NativeVehicleCameraDefaults& defaults) {
    const Word header = address(actual_header);
    if (requested > static_cast<std::int32_t>(read(header, 8)))
        reserve_native_vehicle_camera_array_005cd260(actual_header, requested, strings);
    for (Word i = read(header, 4); static_cast<std::int32_t>(i) < requested; ++i) {
        const Word row = read(header) + i * 0x5cu;
        if (row) {
            write(row, 0, 0);
            write(row, 4, 0);
            initialize_native_vehicle_camera_limits_0078e230(pointer(row + 0x48u), defaults);
        }
    }
    while (requested < static_cast<std::int32_t>(read(header, 4))) {
        write(header, 4, read(header, 4) - 1u);
        const Word row_offset = read(header, 4) * 0x5cu;
        const Word row = read(header) + row_offset;
        destroy_native_string_header_0041dd20(pointer(row), strings);
    }
    write(header, 4, static_cast<Word>(requested));
}
} // namespace bsp
