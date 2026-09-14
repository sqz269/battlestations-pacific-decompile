#include "bsp/native_resource_cache_lookup.hpp"

#include "bsp/native_vfs_date_leaf_providers.hpp"
#include "bsp/singleton_lifetime.hpp"

#include <cstdint>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native resource-cache lookup requires MSVC Win32.
#endif

namespace bsp {
namespace {
void* at(const void* owner, std::uint32_t offset) noexcept {
    return reinterpret_cast<void*>(reinterpret_cast<std::uintptr_t>(owner) + offset);
}
void* volatile& pointer(const void* owner, std::uint32_t offset = 0) noexcept {
    return *static_cast<void* volatile*>(at(owner, offset));
}
bool nil(const void* node) noexcept {
    return *static_cast<const volatile unsigned char*>(at(node, 0x19)) != 0;
}
} // namespace

void* lower_bound_native_resource_cache_name_00b7dfa0(void* tree,
    const void* query) {
    auto* candidate = pointer(tree, 4);
    auto* node = pointer(candidate, 4);
    while (!nil(node)) {
        // B7DFB5..B7DFD8 inlines this exact empty-marker/current-C-string
        // comparison. Reuse its source body, not another tree's lookup body.
        if (less_native_string_headers_00443d00(at(node, 0x0c), query)) {
            node = pointer(node, 8);
        } else {
            candidate = node;
            node = pointer(node);
        }
    }
    return candidate;
}

void* find_native_resource_cache_name_00b7e7b0(void* tree, void* output,
    const void* query, const SingletonLifetimeCallbacks& callbacks) {
    auto* const candidate = lower_bound_native_resource_cache_name_00b7dfa0(tree, query);
    if (!tree) callbacks.invalid_parameter(callbacks.context);
    void* selected_node;
    if (candidate != pointer(tree, 4) &&
        !less_native_string_headers_00443d00(query, at(candidate, 0x0c))) {
        selected_node = candidate;
    } else {
        selected_node = pointer(tree, 4);
    }
    // EDX/ECX capture both selected local fields before either output store.
    // In particular output may overlap the tree's current head or a key.
    auto* const selected_owner = tree;
    auto* const captured_node = selected_node;
    pointer(output) = selected_owner;
    pointer(output, 4) = captured_node;
    return output;
}
} // namespace bsp
