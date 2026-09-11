#include "bsp/native_logical_vertex_pool_return.hpp"

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <cstdint>
#include <cstring>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native logical vertex pool return requires MSVC Win32.
#endif

namespace bsp {
namespace {
using Word = std::uint32_t;
static_assert(sizeof(void*) == 4);
void* at(const void* base, Word byte_offset) noexcept {
    return reinterpret_cast<void*>(reinterpret_cast<Word>(base) + byte_offset);
}
__forceinline Word word(const void* base, Word byte_offset) noexcept {
    Word value;
    __asm {
        mov eax, base
        mov edx, byte_offset
        mov eax, dword ptr [eax + edx]
        mov value, eax
    }
    return value;
}
__forceinline std::uint16_t half(const void* base, Word byte_offset) noexcept {
    std::uint16_t value;
    __asm {
        mov eax, base
        mov edx, byte_offset
        mov ax, word ptr [eax + edx]
        mov value, ax
    }
    return value;
}
__forceinline void put(void* base, Word byte_offset, Word value) noexcept {
    __asm {
        mov eax, base
        mov edx, byte_offset
        mov ecx, value
        mov dword ptr [eax + edx], ecx
    }
}
__forceinline void put_half(void* base, Word byte_offset, std::uint16_t value) noexcept {
    __asm {
        mov eax, base
        mov edx, byte_offset
        mov cx, value
        mov word ptr [eax + edx], cx
    }
}
} // namespace

void __fastcall return_native_logical_vertex_pool_slot_00b49570(
    void* pool, const void* slot) {
    auto* const section = static_cast<CRITICAL_SECTION*>(at(pool, 0x0c));
    EnterCriticalSection(section);
    put(pool, 0x24, word(pool, 0x24) + 1u);
    const Word slab_index = word(slot, 0x74);
    const auto* const slab_table = reinterpret_cast<const void*>(word(pool, 0x28));
    auto* const slab = reinterpret_cast<void*>(word(slab_table, slab_index * 4u));
    const Word offset_bits = reinterpret_cast<Word>(slot) - reinterpret_cast<Word>(slab);
    std::int32_t signed_offset;
    std::memcpy(&signed_offset, &offset_bits, sizeof(signed_offset));
    const auto slot_index = static_cast<std::uint16_t>(signed_offset / 120);
    const Word free_count = half(slab, 0xf40);
    put_half(slab, 0xf00 + free_count * 2u, slot_index);
    put_half(slab, 0xf40, static_cast<std::uint16_t>(half(slab, 0xf40) + 1u));
    if (slab_index < word(pool, 0x34)) put(pool, 0x34, slab_index);
    put(pool, 0x24, word(pool, 0x24) - 1u);
    LeaveCriticalSection(section);
}

} // namespace bsp
