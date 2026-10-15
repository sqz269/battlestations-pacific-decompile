#include "bsp/native_scene_property_insertion.hpp"

#include "bsp/game_native_string_process.hpp"
#include "bsp/native_enum_dictionary_lookup.hpp"
#include "bsp/native_string.hpp"

#include <cstddef>

namespace bsp {
namespace {
static_assert(sizeof(void*) == 4 && sizeof(std::uint32_t) == 4,
    "The original property insertion storage is Win32.");

// Physical DWORD operations retain the intermediate mapped-word zero and
// reload/store ordering, as in the existing recursive lifetime providers.
__forceinline std::uint32_t load_word(const void* root, std::size_t byte_offset) noexcept {
    std::uint32_t value;
    __asm {
        mov eax, root
        mov edx, byte_offset
        mov eax, dword ptr [eax + edx]
        mov value, eax
    }
    return value;
}

__forceinline void store_word(void* root, std::size_t byte_offset, std::uint32_t value) noexcept {
    __asm {
        mov eax, root
        mov edx, byte_offset
        mov ecx, value
        mov dword ptr [eax + edx], ecx
    }
}
} // namespace

NativeScenePropertyInsertionContext::NativeScenePropertyInsertionContext(
    game::GameNativeStringProcess& actual_strings,
    game::GameNativePhysicalPoolProcess& actual_physical)
    : lifetimes_(actual_strings, actual_physical),
      property_empty_(actual_strings.property_empty_00e177e4()),
      query_empty_(actual_strings.query_empty_00e17654()) {}

// Whole Original [008F28F0,008F299C), SHA256
// 352616a3d3506a800c42a875c8ea4d86ccdeb788250d77a8dcec8abee5520878.
// Fixed Source calls bind the admitted actual producer profile; literal native
// phase DATA is never dereferenced as a callable Source table.
__declspec(noinline) std::uint32_t insert_or_replace_native_scene_property_map_008f28f0(
    void* actual_map, const void* actual_query_header,
    std::uint32_t incoming_record_word, NativeScenePropertyInsertionContext& context) {
    std::uint32_t bucket;
    void* const node = find_native_property_map_node_0043b8b0(actual_map,
        actual_query_header, bucket, context.property_empty(), context.query_empty());
    auto& lifetime = context.lifetimes();
    if (node) {
        const auto old_word = load_word(node, 8);
        if (old_word != 0) {
            void* const old_record = reinterpret_cast<void*>(
                static_cast<std::uintptr_t>(old_word));
            scalar_delete_native_scene_property_record_004e6730(old_record, 1, lifetime);
            store_word(node, 8, 0);
        }
        store_word(node, 8, incoming_record_word);
        return load_word(actual_map, 4);
    }

    return publish_native_property_map_new_node_008f2930_fragment(actual_map,
        actual_query_header, bucket, incoming_record_word,
        lifetime.property_pool(), lifetime.keys());
}

// Whole Original [008F33F0,008F348B), SHA256
// 546af6bc24638da2025274f378467e5e8ef4be515a2e80b0892b4ddc653cdffc.
// The normal inline key return is exactly the raw0041DD20 schedule. No local
// exception cleanup is added to this ordinary successful-domain reconstruction.
__declspec(noinline) void* insert_native_scene_property_bag_008f33f0(
    void* actual_bag, const char* actual_key, void* actual_incoming_record,
    NativeScenePropertyInsertionContext& context) {
    alignas(std::uint32_t) std::byte temporary_key[8];
    auto& keys = context.lifetimes().keys();
    construct_native_string_header_0041e870(temporary_key, keys, actual_key);
    insert_or_replace_native_scene_property_map_008f28f0(
        static_cast<std::byte*>(actual_bag) + 4, temporary_key,
        static_cast<std::uint32_t>(reinterpret_cast<std::uintptr_t>(actual_incoming_record)),
        context);
    destroy_native_string_header_0041dd20(temporary_key, keys);

    const bool ordinal_is_zero = load_word(actual_incoming_record, 0x34) == 0;
    store_word(actual_incoming_record, 0x30,
        static_cast<std::uint32_t>(reinterpret_cast<std::uintptr_t>(actual_bag)));
    if (ordinal_is_zero) {
        const auto old_ordinal = load_word(actual_bag, 0x10c);
        store_word(actual_bag, 0x10c, old_ordinal + 1u);
        store_word(actual_incoming_record, 0x34, old_ordinal);
    }
    return actual_bag;
}

} // namespace bsp
