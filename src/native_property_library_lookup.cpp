#include "bsp/native_property_library_lookup.hpp"

#include "bsp/entity_identity.hpp"
#include "bsp/band_0040_cc13.hpp"

#include <cstddef>

namespace bsp {
namespace {
static_assert(sizeof(void*) == 4 && sizeof(std::uint32_t) == 4,
    "The original property library storage is Win32.");

// Retain the Original physical load/store order over actual raw headers and
// map storage, as in the existing native property composition providers.
__forceinline std::uint32_t load_word(const void* root,
    std::size_t byte_offset) noexcept {
    std::uint32_t value;
    __asm {
        mov eax, root
        mov edx, byte_offset
        mov eax, dword ptr [eax + edx]
        mov value, eax
    }
    return value;
}

__forceinline void store_word(void* root, std::size_t byte_offset,
    std::uint32_t value) noexcept {
    __asm {
        mov eax, root
        mov edx, byte_offset
        mov ecx, value
        mov dword ptr [eax + edx], ecx
    }
}

__forceinline void* load_pointer(const void* root,
    std::size_t byte_offset) noexcept {
    return reinterpret_cast<void*>(load_word(root, byte_offset));
}
} // namespace

std::uint32_t native_property_library_key_bucket_004689d0(
    const void* actual_query_header, const char* actual_empty_00e18560) {
    const char* data = static_cast<const char*>(load_pointer(actual_query_header, 4));
    const std::uint32_t length = load_word(actual_query_header, 0);
    if (!data) data = actual_empty_00e18560;
    // 004689D0 uses the same signed-byte CRT hash as 0043B700, with its
    // library-specific E18560 fallback and final six-bit bucket mask.
    return cc13_band0040::hash_upper_case_0043b700(length, data) & 0x3fu;
}

void* find_native_property_library_node_00468ed0(const void* actual_map,
    const void* actual_query_header, std::uint32_t& bucket,
    const char* actual_empty_00e18560, const char* actual_query_empty_00e17654) {
    const std::uint32_t selected = native_property_library_key_bucket_004689d0(
        actual_query_header, actual_empty_00e18560);
    store_word(&bucket, 0, selected);
    void* node = load_pointer(actual_map, 8u + selected * 4u);
    if (!node) return nullptr;
    const std::uint32_t query_length = load_word(actual_query_header, 0);
    do {
        if (load_word(node, 0) == query_length) {
            const char* data = static_cast<const char*>(load_pointer(node, 4));
            if (!data) data = actual_empty_00e18560;
            const char* query = static_cast<const char*>(
                load_pointer(actual_query_header, 4));
            if (!query) query = actual_query_empty_00e17654;
            if (compare_insensitive_00438e10(query, data) == 0) return node;
        }
        node = load_pointer(node, 0x0c);
    } while (node);
    return nullptr;
}

} // namespace bsp
