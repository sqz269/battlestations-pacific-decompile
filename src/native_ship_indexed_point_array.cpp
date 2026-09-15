#include "bsp/native_ship_indexed_point_array.hpp"
#include "bsp/singleton_lifetime.hpp"

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native ship indexed point arrays require MSVC Win32.
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
void copy_indexed_point(Word destination, Word source) noexcept {
    __asm {
        mov edx, destination
        mov eax, source
        fld dword ptr [eax]
        fstp dword ptr [edx]
        fld dword ptr [eax + 4]
        fstp dword ptr [edx + 4]
        fld dword ptr [eax + 8]
        fstp dword ptr [edx + 8]
        mov ecx, dword ptr [eax + 12]
        mov dword ptr [edx + 12], ecx
    }
}
} // namespace

void reserve_native_ship_indexed_point_array_00829180(
    void* actual_header, std::int32_t requested) {
    const Word header = address(actual_header);
    const std::int32_t capacity = requested < 1 ? 1 : requested;
    if (static_cast<std::int32_t>(read(header, 8)) >= capacity) return;

    const Word bytes = static_cast<Word>(capacity) * 16u;
    const Word replacement = address(singleton_lifetime_allocate(
        {SingletonAllocationKind::object, bytes, bytes}));
    Word destination = replacement;
    Word offset = 0;
    for (Word i = 0;
         static_cast<std::int32_t>(i) < static_cast<std::int32_t>(read(header, 4));
         ++i, offset += 16u, destination += 16u) {
        if (destination) copy_indexed_point(destination, read(header) + offset);
    }
    singleton_lifetime_free(pointer(read(header)));
    write(header, 0, replacement);
    write(header, 8, static_cast<Word>(capacity));
}
} // namespace bsp
