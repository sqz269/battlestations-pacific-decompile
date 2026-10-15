#include "bsp/native_property_library_readers.hpp"

#include "bsp/native_property_library_lookup.hpp"
#include "bsp/native_string.hpp"

#include <cstddef>

namespace bsp {

static_assert(sizeof(void*) == 4 && sizeof(std::uint32_t) == 4,
    "The original property library readers use Win32 storage.");

bool contains_native_property_library_name_008f2db0(const void* actual_owner,
    const char* key, NativePropertyLibraryReaderContext& context) {
    alignas(4) std::byte raw_key[8];
    std::uint32_t bucket;
    auto& strings = context.strings();
    construct_native_string_header_0041e870(raw_key, strings.raw_context(), key);
    void* const node = find_native_property_library_node_00468ed0(
        static_cast<const std::byte*>(actual_owner) + 4, raw_key, bucket,
        strings.library_empty_00e18560(), strings.query_empty_00e17654());
    const bool found = node != nullptr;
    destroy_native_string_header_0041dd20(raw_key, strings.raw_context());
    return found;
}

std::uint32_t read_native_property_library_word_00469b60(
    const void* actual_owner, const char* key,
    NativePropertyLibraryReaderContext& context) {
    alignas(4) std::byte raw_key[8];
    std::uint32_t bucket;
    auto& strings = context.strings();
    construct_native_string_header_0041e870(raw_key, strings.raw_context(), key);
    void* const node = find_native_property_library_node_00468ed0(
        static_cast<const std::byte*>(actual_owner) + 4, raw_key, bucket,
        strings.library_empty_00e18560(), strings.query_empty_00e17654());
    // Original 00469BA3 performs this load before any temporary-key cleanup.
    // Keep the physical read explicit; a missing node is outside the domain.
    std::uint32_t mapped_word;
    __asm {
        mov eax, node
        mov eax, dword ptr [eax + 8]
        mov mapped_word, eax
    }
    destroy_native_string_header_0041dd20(raw_key, strings.raw_context());
    return mapped_word;
}

} // namespace bsp
