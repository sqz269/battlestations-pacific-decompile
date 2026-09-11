#include "bsp/native_resource_registry_tree_leaves.hpp"

#include "bsp/native_string_pool_storage.hpp"
#include "bsp/singleton_lifetime.hpp"

#include <cstdint>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native resource registry tree leaves require MSVC Win32.
#endif

namespace bsp {
namespace {
std::uint32_t word(const void* storage, std::uint32_t offset) noexcept {
    return *reinterpret_cast<const volatile std::uint32_t*>(
        reinterpret_cast<std::uintptr_t>(storage) + offset);
}
void* pointer(const void* storage, std::uint32_t offset) noexcept {
    return reinterpret_cast<void*>(word(storage, offset));
}
void put_pointer(void* storage, std::uint32_t offset, void* value) noexcept {
    *reinterpret_cast<volatile std::uint32_t*>(
        reinterpret_cast<std::uintptr_t>(storage) + offset) =
        reinterpret_cast<std::uint32_t>(value);
}
bool nil(const void* node) noexcept {
    return *reinterpret_cast<const volatile unsigned char*>(
        reinterpret_cast<std::uintptr_t>(node) + 0x19) != 0;
}
void invalid(const SingletonLifetimeCallbacks& service) {
    service.invalid_parameter(service.context);
}
} // namespace

void* maximum_native_resource_registry_node_00b196d0(void* node) noexcept {
    auto* child = pointer(node, 8);
    while (!nil(child)) {
        node = child;
        child = pointer(node, 8);
    }
    return node;
}

void* minimum_native_resource_registry_node_00b196f0(void* node) noexcept {
    auto* child = pointer(node, 0);
    while (!nil(child)) {
        node = child;
        child = pointer(node, 0);
    }
    return node;
}

void* rotate_native_resource_registry_right_00b19640(void* tree, void* node) noexcept {
    auto* const replacement = pointer(node, 0);
    put_pointer(node, 0, pointer(replacement, 8));
    auto* const child = pointer(replacement, 8);
    if (!nil(child)) put_pointer(child, 4, node);
    put_pointer(replacement, 4, pointer(node, 4));
    auto* const current_head = pointer(tree, 4);
    if (node == pointer(current_head, 4)) {
        put_pointer(current_head, 4, replacement);
    } else {
        auto* const current_parent = pointer(node, 4);
        if (node == pointer(current_parent, 8)) {
            put_pointer(current_parent, 8, replacement);
        } else {
            put_pointer(current_parent, 0, replacement);
        }
    }
    put_pointer(replacement, 8, node);
    put_pointer(node, 4, replacement);
    return replacement;
}

void* rotate_native_resource_registry_left_00b19830(void* tree, void* node) noexcept {
    auto* const replacement = pointer(node, 8);
    put_pointer(node, 8, pointer(replacement, 0));
    auto* const child = pointer(replacement, 0);
    if (!nil(child)) put_pointer(child, 4, node);
    put_pointer(replacement, 4, pointer(node, 4));
    auto* const current_head = pointer(tree, 4);
    if (node == pointer(current_head, 4)) {
        put_pointer(current_head, 4, replacement);
    } else {
        auto* const current_parent = pointer(node, 4);
        if (node == pointer(current_parent, 0)) {
            put_pointer(current_parent, 0, replacement);
        } else {
            put_pointer(current_parent, 8, replacement);
        }
    }
    put_pointer(replacement, 0, node);
    put_pointer(node, 4, replacement);
    return replacement;
}

void increment_native_resource_registry_iterator_00b19890(
    NativeResourceRegistryTreeIterator& iterator,
    const SingletonLifetimeCallbacks& invalid_parameters) {
    if (!pointer(&iterator, 0)) invalid(invalid_parameters);
    auto* const node = pointer(&iterator, 4);
    if (nil(node)) {
        invalid(invalid_parameters);
        return; // Returning native tail call, not a retry after handler mutation.
    }
    auto* selected = pointer(node, 8);
    if (!nil(selected)) {
        auto* child = pointer(selected, 0);
        while (!nil(child)) {
            selected = child;
            child = pointer(selected, 0);
        }
    } else {
        selected = pointer(node, 4);
        while (!nil(selected) && pointer(&iterator, 4) == pointer(selected, 8)) {
            put_pointer(&iterator, 4, selected);
            selected = pointer(selected, 4);
        }
    }
    put_pointer(&iterator, 4, selected);
}

void destroy_native_resource_registry_subtree_00b1a260(
    void* tree, void* node, ActualNativeStringPoolStorage& strings) {
    if (nil(node)) return;
    for (;;) {
        destroy_native_resource_registry_subtree_00b1a260(
            tree, pointer(node, 8), strings);
        auto* const data = pointer(node, 0x10);
        auto* const captured_left = pointer(node, 0);
        if (data) {
            const auto size = word(node, 0x0c) + 1u;
            strings.release(static_cast<char*>(data), size);
        }
        singleton_lifetime_free(node);
        const bool left_is_nil = nil(captured_left);
        node = captured_left;
        if (left_is_nil) return;
    }
}
} // namespace bsp
