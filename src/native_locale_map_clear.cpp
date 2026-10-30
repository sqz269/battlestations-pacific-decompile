#include "bsp/native_locale_map_clear.hpp"

#include "bsp/native_locale_map_node_lifetime.hpp"
#include "bsp/native_locale_map_node_recycling.hpp"

#include <cstdint>
#include <cstring>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Raw locale map clear requires MSVC Win32.
#endif

namespace bsp {
namespace {
using Word = std::uint32_t;
static_assert(sizeof(void*) == 4 && sizeof(std::uintptr_t) == 4 &&
    sizeof(Word) == 4, "requires the Win32 address model");

Word word(Word base, Word byte_offset) noexcept {
    Word result;
    __asm {
        mov eax, base
        mov edx, byte_offset
        mov eax, dword ptr [eax + edx]
        mov result, eax
    }
    return result;
}

void put_word(Word base, Word byte_offset, Word value) noexcept {
    __asm {
        mov eax, base
        mov edx, byte_offset
        mov ecx, value
        mov dword ptr [eax + edx], ecx
    }
}

void* pointer(Word value) noexcept {
    return reinterpret_cast<void*>(static_cast<std::uintptr_t>(value));
}
} // namespace

void clear_native_locale_map_0073c240(void* actual_map,
    NativeStringRawPoolContext& strings, void* actual_pool) {
    const auto map = static_cast<Word>(
        reinterpret_cast<std::uintptr_t>(actual_map));
    for (Word bucket = 0; bucket < 0x1000u; ++bucket) {
        Word node = word(map, bucket * 4u);
        while (node != 0u) {
            const Word next = word(node, 0x14u);
            destroy_native_locale_map_node_payload_00737ce0(
                pointer(node), strings);
            return_native_locale_node_slot_00736090(actual_pool, pointer(node));
            node = next;
        }
    }
    std::memset(pointer(map), 0, 0x4000u);
    put_word(map, 0x4000u, 0u);
}
} // namespace bsp
