#include "bsp/native_resource_tree_cleanup.hpp"
#include "bsp/native_string_pool_storage.hpp"
#include "bsp/native_string.hpp"
#include "bsp/singleton_lifetime.hpp"
#include <cstdint>

namespace bsp {
namespace {
void* at(const void* p, std::uint32_t n = 0) noexcept {
    return reinterpret_cast<void*>(reinterpret_cast<std::uintptr_t>(p) + n);
}
void* volatile& pointer(const void* p, std::uint32_t n = 0) noexcept {
    return *static_cast<void* volatile*>(at(p, n));
}
volatile std::uint32_t& word(const void* p, std::uint32_t n = 0) noexcept {
    return *static_cast<volatile std::uint32_t*>(at(p, n));
}
bool nil(const void* p) noexcept {
    return *static_cast<volatile unsigned char*>(at(p, 0x19)) != 0;
}
} // namespace

void* maximum_native_resource_parser_node_00b7ca20(void* node) noexcept {
    return maximum_native_resource_cache_node_00b7cb70(node);
}
void* minimum_native_resource_parser_node_00b7ca40(void* node) noexcept {
    return minimum_native_resource_cache_node_00b7cb90(node);
}
void increment_native_resource_parser_iterator_00b7cf80(void* iterator,
    const SingletonLifetimeCallbacks& callbacks) {
    increment_native_resource_cache_iterator_00b7ce80(iterator, callbacks);
}
void* erase_native_resource_parser_iterator_00b7f790(void* tree, void* output,
    NativeResourceCacheIteratorStorage first, NativeStringRawPoolContext& strings,
    const SingletonLifetimeCallbacks& callbacks) {
    return erase_native_resource_cache_iterator_00b7fa60(tree, output, first, strings, callbacks);
}

void destroy_native_resource_cache_subtree_00b7ff20(void* tree, void* node,
    NativeStringRawPoolContext& strings) {
    while (!nil(node)) {
        destroy_native_resource_cache_subtree_00b7ff20(tree, pointer(node, 8), strings);
        void* const key_data = pointer(node, 0x10);
        void* const next = pointer(node);
        if (key_data) {
            const auto bytes = word(node, 0xc) + 1u;
            auto* pool = native_string_pool_get_or_create_00419cc0(
                strings.actual_published_01090aa8, strings.actual_manager_publication_01090aa0);
            return_native_string_pool_00bd1510(pool, key_data, bytes,
                strings.actual_small_returns_disabled_01090aa4);
        }
        singleton_lifetime_free(node);
        node = next;
    }
}
void destroy_native_resource_parser_subtree_00b7f730(void* tree, void* node,
    NativeStringRawPoolContext& strings) {
    destroy_native_resource_cache_subtree_00b7ff20(tree, node, strings);
}

void* erase_native_resource_cache_range_00b805d0(void* tree, void* output,
    NativeResourceCacheIteratorStorage first, NativeResourceCacheIteratorStorage last,
    NativeStringRawPoolContext& strings, const SingletonLifetimeCallbacks& callbacks) {
    void* owner = pointer(&first);
    void* const captured_begin = pointer(pointer(tree, 4));
    if (!owner || owner != tree) callbacks.invalid_parameter(callbacks.context);
    void* node = pointer(&first, 4);
    if (node == captured_begin) {
        void* const last_owner = pointer(&last);
        void* const captured_head = pointer(tree, 4);
        if (!last_owner || last_owner != tree) callbacks.invalid_parameter(callbacks.context);
        if (pointer(&last, 4) == captured_head) {
            destroy_native_resource_cache_subtree_00b7ff20(tree, pointer(pointer(tree, 4), 4), strings);
            void* head = pointer(tree, 4);
            pointer(head, 4) = head;
            head = pointer(tree, 4); // captured before the count store
            word(tree, 8) = 0;
            pointer(head) = head;
            head = pointer(tree, 4);
            pointer(head, 8) = head;
            void* const result_node = pointer(pointer(tree, 4));
            pointer(output) = tree;
            pointer(output, 4) = result_node;
            return output;
        }
    }
    for (;;) {
        if (!owner || owner != pointer(&last)) callbacks.invalid_parameter(callbacks.context);
        if (node == pointer(&last, 4)) break;
        increment_native_resource_cache_iterator_00b7ce80(&first, callbacks);
        NativeResourceCacheIteratorStorage ignored;
        erase_native_resource_cache_iterator_00b7fa60(tree, &ignored,
            {owner, node}, strings, callbacks);
        node = pointer(&first, 4);
        owner = pointer(&first);
    }
    pointer(output) = owner;
    pointer(output, 4) = node;
    return output;
}
void* erase_native_resource_parser_range_00b80500(void* tree, void* output,
    NativeResourceCacheIteratorStorage first, NativeResourceCacheIteratorStorage last,
    NativeStringRawPoolContext& strings, const SingletonLifetimeCallbacks& callbacks) {
    return erase_native_resource_cache_range_00b805d0(tree, output, first, last, strings, callbacks);
}

void destroy_native_resource_cache_tree_00b80ed0(void* tree,
    NativeStringRawPoolContext& strings, const SingletonLifetimeCallbacks& callbacks) {
    void* const head = pointer(tree, 4);
    void* const begin = pointer(head);
    NativeResourceCacheIteratorStorage ignored;
    erase_native_resource_cache_range_00b805d0(tree, &ignored,
        {tree, begin}, {tree, head}, strings, callbacks);
    singleton_lifetime_free(pointer(tree, 4));
    pointer(tree, 4) = nullptr;
    word(tree, 8) = 0;
}
void destroy_native_resource_parser_tree_00b80e90(void* tree,
    NativeStringRawPoolContext& strings, const SingletonLifetimeCallbacks& callbacks) {
    destroy_native_resource_cache_tree_00b80ed0(tree, strings, callbacks);
}
} // namespace bsp
