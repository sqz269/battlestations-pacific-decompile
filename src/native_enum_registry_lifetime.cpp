#include "bsp/native_enum_registry_lifetime.hpp"
#include "bsp/native_enum_node_pool.hpp"
#include "bsp/native_enum_scalar_delete.hpp"
#include <cstdint>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native enum registry lifetime requires MSVC Win32.
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

void clear_native_cenum_table_map_004d0760(void* receiver, void* table_pool,
    void* symbol_pool, NativeStringRawPoolContext& strings) {
    void* cursor = at(receiver, 8);
    for (std::uint32_t remaining = 64; remaining != 0; --remaining) {
        void* node = reinterpret_cast<void*>(word(cursor, 0));
        while (node) {
            auto* const payload = reinterpret_cast<void*>(word(node, 8));
            void* const next = reinterpret_cast<void*>(word(node, 0x0c));
            if (payload) {
                // Genuine admitted CEnum only; no original virtual dispatch.
                delete_native_scene_enum_owner_008f59c0(payload, 1, symbol_pool, strings);
                put(node, 8, 0); // ONLY after the scalar operation returns
            }
            destroy_native_string_header_0041dd20(node, strings);
            // Exact complete14h provider matches actualE175E8 inlined OSCS,
            // depth/page/WORD/earliest-page schedule; stale fields survive.
            return_native_enum_node_0043b0a0(table_pool, node);
            node = next;
        }
        put(cursor, 0, 0); // AFTER the entire chain, including null payloads
        cursor = at(cursor, 4);
    }
    put(receiver, 4, 0); // AFTER all64 heads
}

void destroy_native_scene_enum_registry_008f4f00(void* owner, void* table_pool,
    void* symbol_pool, NativeStringRawPoolContext& strings) {
    put(owner, 0, 0x00ce78bc);
    void* const receiver = at(owner, 4);
    clear_native_cenum_table_map_004d0760(receiver, table_pool, symbol_pool, strings);
    put(receiver, 0, 0x00ce7514);
    clear_native_cenum_table_map_004d0760(receiver, table_pool, symbol_pool, strings);
}
} // namespace bsp
