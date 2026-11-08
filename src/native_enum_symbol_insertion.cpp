#include "bsp/native_enum_symbol_insertion.hpp"
#include "bsp/native_enum_dictionary_lookup.hpp"
#include "bsp/native_enum_node_pool.hpp"
#include "bsp/gui_widgets_cc13.hpp"
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

// AA9A20's only service is the same raw 0041DD40 resize used by 8F2850.
// Borrow the insertion's current string domain; do not create another pool.
class EnumEntryStrings final : public gui_widgets_cc13::GuiNativeStringHost {
public:
    explicit EnumEntryStrings(NativeStringRawPoolContext& strings) noexcept
        : strings_(strings) {}
    void resize_0041dd40(void* header, std::uint32_t length, bool preserve) override {
        resize_native_string_header_0041dd40(header, strings_, length, preserve);
    }
private:
    NativeStringRawPoolContext& strings_;
};
} // namespace

void* initialize_native_scene_enum_owner_008f4dd0(void* owner,
    NativeStringRawPoolContext& strings, const void* type_bytes) {
    return &initialize_native_scene_enum_owner_008f4dd0(
        *static_cast<NativeSceneEnumOwnerStorage*>(owner), strings, type_bytes);
}

NativeSceneEnumOwnerStorage& initialize_native_scene_enum_owner_008f4dd0(
    NativeSceneEnumOwnerStorage& storage,
    NativeStringRawPoolContext& strings, const void* type_bytes) {
    void* const owner = &storage;
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
    return storage;
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
    // bsp.py show 008F2850 and 00AA9A20: both zero +0/+4, resize with keep=1,
    // reload the source length and both buffers after resize, copy destination
    // length bytes, then store the supplied word at +8. The fresh 14h enum
    // node's owning 0Ch prefix is exactly that string/value pair. The live key
    // and fresh allocation are disjoint in this admitted successful domain;
    // AA9A20's memcpy therefore has the replaced memmove's result. Leave +Ch
    // linkage and +10h preserved storage to this insertion. Source reuse only,
    // with an explicit pool adapter; no Original register/stack ABI claim.
    EnumEntryStrings entry_strings(strings);
    gui_widgets_cc13::gui_named_entry_construct_00aa9a20(node, &key, value, entry_strings);
    put(node, 0x0c, word(receiver, 8u + bucket * 4u));
    put(receiver, 8u + bucket * 4u, address(node));
    put(receiver, 4, word(receiver, 4) + 1u);
    return word(receiver, 4);
}
} // namespace bsp
