#include "bsp/native_resource_manager_trees.hpp"

#include "bsp/native_vfs_date_leaf_providers.hpp"
#include "bsp/singleton_lifetime.hpp"

#include <cstdint>

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
void* allocate_head_storage() {
    auto* const allocation = singleton_lifetime_allocate({
        SingletonAllocationKind::object, 0x1c, 0x1c});
    // E000 and E050 contain the same schedule. Preserve the three separate
    // computed-address tests; do not reinterpret them as a null-return guard.
    if (allocation) pointer(allocation) = nullptr;
    auto* slot = at(allocation, 4);
    if (slot) pointer(slot) = nullptr;
    slot = at(allocation, 8);
    if (slot) pointer(slot) = nullptr;
    *static_cast<volatile unsigned char*>(at(allocation, 0x18)) = 1;
    *static_cast<volatile unsigned char*>(at(allocation, 0x19)) = 0;
    return allocation;
}
} // namespace

void* allocate_native_resource_parser_head_storage_00b7e000() {
    return allocate_head_storage();
}

void* allocate_native_resource_cache_head_storage_00b7e050() {
    return allocate_head_storage();
}

void* lower_bound_native_resource_parser_name_00b7df40(void* tree,
    const void* query) {
    auto* candidate = pointer(tree, 4);
    auto* node = pointer(candidate, 4);
    while (!nil(node)) {
        // DF55..DF7A inlines the same gates/current field order as 443D00.
        // The native CALL is DF70->BF7FBF; no DF40->443D00 edge is claimed.
        if (less_native_string_headers_00443d00(at(node, 0x0c), query)) {
            node = pointer(node, 8);
        } else {
            candidate = node;
            node = pointer(node);
        }
    }
    return candidate;
}

void* find_native_resource_parser_name_00b7e740(void* tree, void* output,
    const void* query, const SingletonLifetimeCallbacks& callbacks) {
    auto* const candidate = lower_bound_native_resource_parser_name_00b7df40(tree, query);
    if (!tree) callbacks.invalid_parameter(callbacks.context);
    void* selected_node;
    if (candidate != pointer(tree, 4) &&
        !less_native_string_headers_00443d00(query, at(candidate, 0x0c))) {
        selected_node = candidate;
    } else {
        selected_node = pointer(tree, 4);
    }
    // Native EDX and ECX capture the selected local pair before either store.
    auto* const selected_owner = tree;
    auto* const captured_node = selected_node;
    pointer(output) = selected_owner;
    pointer(output, 4) = captured_node;
    return output;
}

void decrement_native_resource_parser_iterator_00b7cef0(void* iterator,
    const SingletonLifetimeCallbacks& callbacks) {
    if (!pointer(iterator)) callbacks.invalid_parameter(callbacks.context);
    auto* const node = pointer(iterator, 4);
    if (nil(node)) {
        auto* const maximum = pointer(node, 8);
        pointer(iterator, 4) = maximum;
        if (nil(maximum)) callbacks.invalid_parameter(callbacks.context);
        return; // CF13 native tail: preserve a returning handler's changes.
    }
    auto* child = pointer(node);
    if (!nil(child)) {
        auto* next = pointer(child, 8);
        while (!nil(next)) {
            child = next;
            next = pointer(child, 8);
        }
        pointer(iterator, 4) = child;
        return;
    }
    auto* ancestor = pointer(node, 4);
    while (!nil(ancestor) && pointer(iterator, 4) == pointer(ancestor)) {
        pointer(iterator, 4) = ancestor;
        ancestor = pointer(ancestor, 4);
    }
    if (nil(pointer(iterator, 4))) {
        callbacks.invalid_parameter(callbacks.context);
        return; // CF6F native tail: no final ancestor store after return.
    }
    pointer(iterator, 4) = ancestor;
}
} // namespace bsp
