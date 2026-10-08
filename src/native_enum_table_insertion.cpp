#include "bsp/native_enum_table_insertion.hpp"
#include "bsp/native_enum_dictionary_lookup.hpp"
#include "bsp/native_enum_node_pool.hpp"
#include "bsp/native_enum_scalar_delete.hpp"
#include <cstring>
#include <new>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native enum table insertion requires MSVC Win32.
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
std::uint32_t address(const void* value) noexcept {
    return reinterpret_cast<std::uintptr_t>(value);
}
void* at(const void* base, std::uint32_t byte_offset) noexcept {
    return reinterpret_cast<void*>(address(base) + byte_offset);
}
} // namespace

void* initialize_native_scene_enum_registry_storage_004d3069(void* owner) {
    put(owner, 0, 0x00ce78bc);
    put(owner, 4, 0x00ce7514);
    put(owner, 8, 0);
    for (std::uint32_t index = 0; index < 64; ++index) put(owner, 0x0c + index * 4u, 0);
    return owner;
}

std::uint32_t insert_native_cenum_table_008f17e0(void* receiver,
    const NativeString& key, void* value, void* table_pool, void* symbol_pool,
    NativeStringRawPoolContext& strings, const char* node_empty, const char* key_empty) {
    std::uint32_t bucket;
    auto* node = const_cast<void*>(find_native_enum_table_node_0048d4e0(
        receiver, key, bucket, node_empty, key_empty));
    if (node) {
        auto* const old_value = reinterpret_cast<void*>(word(node, 8));
        if (old_value) {
            // Producer/heap/lifetime precondition: genuine current-CRT CEnum,
            // D16508 slot0=008F59C0. Original profile/slot loads and generic
            // virtual dispatch are not replayed by this direct binding.
            delete_native_scene_enum_owner_008f59c0(old_value, 1, symbol_pool, strings);
            put(node, 8, 0); // native clear occurs only AFTER deletion returns
        }
        put(node, 8, address(value));
        return word(receiver, 4);
    }
    node = allocate_native_enum_node_004e7c00(table_pool);
    if (node) ::new (node) NativeString; // fresh header0/4 only; slot10 retained
    if (node != &key) {
        resize_native_string_header_0041dd40(node, strings, key.length(), true);
        if (key.length() != 0) {
            // Historical00BF7680 remains _memcpy. The admitted owning query
            // and fresh destination are disjoint; reload AFTER resize.
            auto* const destination = static_cast<NativeString*>(node);
            std::memcpy(destination->data(), key.data(), destination->length());
        }
    }
    put(node, 8, address(value));
    put(node, 0x0c, word(receiver, 8u + bucket * 4u));
    put(receiver, 8u + bucket * 4u, address(node));
    put(receiver, 4, word(receiver, 4) + 1u);
    return word(receiver, 4);
}
} // namespace bsp
