#include "bsp/native_enum_symbol_insertion.hpp"
#include "bsp/native_enum_dictionary_lookup.hpp"
#include "bsp/native_enum_node_pool.hpp"
#include <cstring>
#include <new>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native enum insertion requires MSVC Win32.
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
void* at(const void* base, std::uint32_t offset) noexcept {
    return reinterpret_cast<void*>(address(base) + offset);
}
} // namespace

void* initialize_native_scene_enum_owner_008f4dd0(void* owner,
    NativeStringRawPoolContext& strings, const void* type_bytes) {
    put(owner, 0, 0x00d16508);
    put(owner, 4, 0x00d162c0);
    put(owner, 8, 0);
    for (std::uint32_t index = 0; index < 64; ++index) put(owner, 0x0c + index * 4u, 0);
    put(owner, 0x10c, 9999);
    put(owner, 0x110, 0);
    auto* const type = ::new (at(owner, 0x114)) NativeString;
    resize_native_string_header_0041dd40(type, strings, 1, false);
    if (type->data()) {
        // Historical00BF7680 is named _memcpy; its admitted copy permits overlap.
        std::memmove(type->data(), type_bytes, type->length());
    }
    *static_cast<unsigned char*>(at(owner, 0x11c)) = 0;
    return owner;
}

std::uint32_t insert_native_enum_symbol_word_008f2850(void* receiver,
    const NativeString& key, std::uint32_t value, void* symbol_pool,
    NativeStringRawPoolContext& strings, const char* node_empty, const char* key_empty) {
    std::uint32_t bucket;
    auto* node = const_cast<void*>(find_native_enum_symbol_node_0048d480(
        receiver, key, bucket, node_empty, key_empty));
    if (node) {
        put(node, 8, value);
        return word(receiver, 4);
    }
    node = allocate_native_enum_node_004e7c00(symbol_pool);
    if (node) ::new (node) NativeString; // native fresh header stores0/4 only
    if (node != &key) {
        resize_native_string_header_0041dd40(node, strings, key.length(), true);
        if (key.length() != 0) {
            // Reload both headers AFTER resize; do not replace owning copy with
            // a pointer assignment, CString scan, or the unrelated copy fragment.
            auto* const destination = static_cast<NativeString*>(node);
            std::memmove(destination->data(), key.data(), destination->length());
        }
    }
    put(node, 8, value);
    put(node, 0x0c, word(receiver, 8u + bucket * 4u));
    put(receiver, 8u + bucket * 4u, address(node));
    put(receiver, 4, word(receiver, 4) + 1u);
    return word(receiver, 4);
}
} // namespace bsp
