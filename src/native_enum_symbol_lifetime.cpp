#include "bsp/native_enum_symbol_lifetime.hpp"
#include "bsp/native_enum_node_pool.hpp"
#include <cstdint>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native enum symbol lifetime requires MSVC Win32.
#endif
namespace bsp {
namespace {
static_assert(sizeof(void*) == 4 && sizeof(NativeString) == 8);
std::uint32_t word(const void* base, std::uint32_t byte_offset) noexcept {
    std::uint32_t result;
    __asm {
        mov eax, base
        mov edx, byte_offset
        mov eax, dword ptr [eax + edx]
        mov result, eax
    }
    return result;
}
void put(void* base, std::uint32_t byte_offset, std::uint32_t value) noexcept {
    __asm {
        mov eax, base
        mov edx, byte_offset
        mov ecx, value
        mov dword ptr [eax + edx], ecx
    }
}
void* at(void* base, std::uint32_t byte_offset) noexcept {
    return reinterpret_cast<void*>(reinterpret_cast<std::uintptr_t>(base) + byte_offset);
}
} // namespace

void clear_native_enum_symbol_map_008f4b60(void* receiver, void* pool,
    NativeStringRawPoolContext& strings) {
    void* cursor = at(receiver, 8);
    for (std::uint32_t remaining = 64; remaining != 0; --remaining) {
        void* node = reinterpret_cast<void*>(word(cursor, 0));
        while (node) {
            void* const next = reinterpret_cast<void*>(word(node, 0x0c));
            destroy_native_string_header_0041dd20(node, strings);
            // Complete provider matches the native inlined CS/depth/page/WORD
            // free-stack schedule. Neither operation clears the stale slot.
            return_native_enum_node_0043b0a0(pool, node);
            node = next;
        }
        put(cursor, 0, 0);
        cursor = at(cursor, 4);
    }
    put(receiver, 4, 0);
}

void destroy_native_scene_enum_owner_008f4e70(void* owner, void* pool,
    NativeStringRawPoolContext& strings) {
    put(owner, 0, 0x00d16508);
    void* const receiver = at(owner, 4);
    clear_native_enum_symbol_map_008f4b60(receiver, pool, strings);
    destroy_native_string_header_0041dd20(at(owner, 0x114), strings);
    put(receiver, 0, 0x00d162c0);
    clear_native_enum_symbol_map_008f4b60(receiver, pool, strings);
}
} // namespace bsp
