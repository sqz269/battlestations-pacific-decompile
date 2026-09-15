#include "bsp/native_vehicle_pointer_array.hpp"
#include "bsp/singleton_lifetime.hpp"

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native vehicle pointer arrays require MSVC Win32.
#endif

namespace bsp {
namespace {
using Word = std::uint32_t;
static_assert(sizeof(void*) == 4 && sizeof(LONG) == 4);

Word address(const void* p) noexcept { return reinterpret_cast<Word>(p); }
void* pointer(Word p) noexcept { return reinterpret_cast<void*>(p); }
Word read(Word p, Word byte_offset = 0) noexcept {
    Word result;
    __asm {
        mov eax, p
        mov edx, byte_offset
        mov eax, dword ptr [eax + edx]
        mov result, eax
    }
    return result;
}
void write(Word p, Word byte_offset, Word value) noexcept {
    __asm {
        mov eax, p
        mov edx, byte_offset
        mov ecx, value
        mov dword ptr [eax + edx], ecx
    }
}
std::int32_t count(Word header) noexcept {
    return static_cast<std::int32_t>(read(header, 4));
}
void release_slot(Word slot, NativeVehiclePointerArrayCalls& calls) {
    const Word owner = read(slot);
    if (owner) {
        if (InterlockedDecrement(reinterpret_cast<volatile LONG*>(owner + 4u)) == 0) {
            const Word table = read(owner);
            const Word target = read(table);
            calls.zero_reference(target, pointer(owner), table);
        }
        write(slot, 0, 0);
    }
}
} // namespace

void reserve_native_vehicle_pointer_array_00546630(
    void* actual_header, std::int32_t requested, NativeVehiclePointerArrayCalls& calls) {
    const Word header = address(actual_header);
    const std::int32_t capacity = requested < 1 ? 1 : requested;
    if (static_cast<std::int32_t>(read(header, 8)) >= capacity) return;
    const Word bytes = static_cast<Word>(capacity) * 4u;
    const Word replacement = address(singleton_lifetime_allocate(
        {SingletonAllocationKind::object, bytes, bytes}));
    Word destination = replacement;
    for (Word i = 0; static_cast<std::int32_t>(i) < count(header); ++i, destination += 4u) {
        if (destination) {
            const Word source = read(header) + i * 4u;
            write(destination, 0, 0);
            const Word owner = read(source);
            if (owner) {
                write(destination, 0, owner);
                InterlockedIncrement(reinterpret_cast<volatile LONG*>(owner + 4u));
            }
        }
    }
    for (Word i = 0; static_cast<std::int32_t>(i) < count(header); ++i)
        release_slot(read(header) + i * 4u, calls);
    singleton_lifetime_free(pointer(read(header)));
    write(header, 0, replacement);
    write(header, 8, static_cast<Word>(capacity));
}

void resize_native_vehicle_pointer_array_005471b0(
    void* actual_header, std::int32_t requested, NativeVehiclePointerArrayCalls& calls) {
    const Word header = address(actual_header);
    if (static_cast<std::int32_t>(read(header, 8)) < requested)
        reserve_native_vehicle_pointer_array_00546630(actual_header, requested, calls);
    for (Word i = read(header, 4); static_cast<std::int32_t>(i) < requested; ++i) {
        const Word destination = read(header) + i * 4u;
        if (destination) write(destination, 0, 0);
    }
    while (requested < count(header)) {
        write(header, 4, read(header, 4) - 1u);
        const Word index = read(header, 4);
        const Word data = read(header);
        release_slot(data + index * 4u, calls);
    }
    write(header, 4, static_cast<Word>(requested));
}
} // namespace bsp
