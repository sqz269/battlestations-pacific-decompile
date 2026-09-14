#include "bsp/native_resource_cache_links.hpp"

#include "bsp/singleton_lifetime.hpp"

#include <cstdint>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native resource-cache links require MSVC Win32.
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
void invalid(const SingletonLifetimeCallbacks& callbacks) {
    callbacks.invalid_parameter(callbacks.context);
}
} // namespace

void* rotate_native_resource_cache_right_00b7cbd0(void* tree, void* node) noexcept {
    auto* const pivot = pointer(node);
    pointer(node) = pointer(pivot, 8);
    auto* const moved = pointer(pivot, 8); // Reload after the original link store.
    if (!nil(moved)) pointer(moved, 4) = node;
    pointer(pivot, 4) = pointer(node, 4);
    auto* const current_head = pointer(tree, 4);
    if (node == pointer(current_head, 4)) {
        pointer(current_head, 4) = pivot;
    } else {
        auto* const parent = pointer(node, 4);
        if (node == pointer(parent, 8)) pointer(parent, 8) = pivot;
        else pointer(parent) = pivot;
    }
    pointer(pivot, 8) = node;
    pointer(node, 4) = pivot;
    return pivot;
}

void* rotate_native_resource_cache_left_00b7d5f0(void* tree, void* node) noexcept {
    auto* const pivot = pointer(node, 8);
    pointer(node, 8) = pointer(pivot);
    auto* const moved = pointer(pivot); // Reload after the original link store.
    if (!nil(moved)) pointer(moved, 4) = node;
    pointer(pivot, 4) = pointer(node, 4);
    auto* const current_head = pointer(tree, 4);
    if (node == pointer(current_head, 4)) {
        pointer(current_head, 4) = pivot;
    } else {
        auto* const parent = pointer(node, 4);
        if (node == pointer(parent)) pointer(parent) = pivot;
        else pointer(parent, 8) = pivot;
    }
    pointer(pivot) = node;
    pointer(node, 4) = pivot;
    return pivot;
}

void decrement_native_resource_cache_iterator_00b7cdf0(void* iterator,
    const SingletonLifetimeCallbacks& callbacks) {
    if (!pointer(iterator)) invalid(callbacks);
    auto* const node = pointer(iterator, 4);
    if (nil(node)) {
        auto* const maximum = pointer(node, 8);
        pointer(iterator, 4) = maximum;
        if (nil(maximum)) invalid(callbacks); // B7CE13 tail; no later stores.
        return;
    }
    auto* child = pointer(node);
    if (!nil(child)) {
        auto* right = pointer(child, 8);
        while (!nil(right)) {
            child = right;
            right = pointer(child, 8);
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
        invalid(callbacks); // B7CE6F tail; preserve a returning handler's repair.
        return;
    }
    pointer(iterator, 4) = ancestor;
}
} // namespace bsp
