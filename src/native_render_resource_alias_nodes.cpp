#include "bsp/native_render_resource_alias_nodes.hpp"
#include "bsp/native_string_pool_storage.hpp"
#include "bsp/singleton_lifetime.hpp"
#include <cstring>
#include <new>

namespace bsp {
namespace {
static_assert(sizeof(void*) == 4);
static_assert(sizeof(NativeRenderResourceAliasNode) == 0x10);
static_assert(offsetof(NativeRenderResourceAliasNode, next_00) == 0);
static_assert(offsetof(NativeRenderResourceAliasNode, previous_04) == 4);
static_assert(offsetof(NativeRenderResourceAliasNode, string_length_08) == 8);
static_assert(offsetof(NativeRenderResourceAliasNode, string_data_0c) == 12);

template<class T> T read(const void* header, std::size_t offset) noexcept {
    T value;
    std::memcpy(&value, static_cast<const std::byte*>(header) + offset, sizeof(value));
    return value;
}
template<class T> void write(void* header, std::size_t offset, T value) noexcept {
    std::memcpy(static_cast<std::byte*>(header) + offset, &value, sizeof(value));
}

void resize_fresh_header(void* destination, std::uint32_t requested,
    ActualNativeStringPoolStorage& strings) {
    resize_native_string_header_0041dd40(destination, strings, requested, true);
}

void resize_fresh_header(void* destination, std::uint32_t requested, SizedStoragePool& pool) {
    // The caller has just zeroed this actual header. The existing 0041DD40
    // operation therefore either takes its equal-zero early return or its
    // nonzero allocation branch. Use its complete live-field behavior here:
    // NativeString owns private fields and cannot borrow AliasNode's members.
    if (requested == 0) return;
    auto* replacement = static_cast<char*>(pool.allocate_00bd1120(requested + 1u));
    // A real allocation boundary can change these actual fields. Do not assume
    // they remain zero, and do not substitute a temporary string header.
    const auto current_length = read<std::uint32_t>(destination, 0);
    const auto copied = requested > current_length ? current_length : requested;
    if (copied != 0)
        std::memcpy(replacement, read<char*>(destination, 4), copied);
    auto* old_data = read<char*>(destination, 4);
    if (old_data)
        pool.release_00bd1510(old_data, read<std::uint32_t>(destination, 0) + 1u);
    write<char*>(destination, 4, replacement);
    write<std::uint32_t>(destination, 0, requested);
    replacement[requested] = '\0';
}
} // namespace

template<class Pool>
static void construct_alias_string_with_pool(void* destination,
    const void* source, Pool& pool) {
    if (!destination) return; // Native 0044BCE0 skips even the source read.
    const bool same_header = destination == source;
    write<std::uint32_t>(destination, 0, 0);
    write<char*>(destination, 4, nullptr);
    if (same_header) return; // The original comparison precedes both stores.
    resize_fresh_header(destination, read<std::uint32_t>(source, 0), pool);
    if (read<std::uint32_t>(source, 0) != 0) {
        const auto copied = read<std::uint32_t>(destination, 0);
        auto* current_source = read<char*>(source, 4);
        auto* current_destination = read<char*>(destination, 4);
        if (copied != 0) std::memcpy(current_destination, current_source, copied);
    }
    // 00C5FFB0 -> 00401130 is a RET-only placement-delete unwind leaf.
    // No string destruction or synthesized exception cleanup belongs here.
}

void construct_native_render_alias_string_0044bcb0(void* destination,
    const void* source, SizedStoragePool& pool) {
    construct_alias_string_with_pool(destination, source, pool);
}

void construct_native_render_alias_string_0044bcb0(void* destination,
    const void* source, ActualNativeStringPoolStorage& pool) {
    construct_alias_string_with_pool(destination, source, pool);
}

template<class Pool>
static NativeRenderResourceAliasNode* allocate_alias_node_with_pool(
    NativeRenderResourceAliasNode* next, NativeRenderResourceAliasNode* previous,
    const void* source, Pool& pool) {
    void* const raw = singleton_lifetime_allocate({SingletonAllocationKind::object,
        0x10, sizeof(NativeRenderResourceAliasNode)});
    try {
        auto* node = ::new (raw) NativeRenderResourceAliasNode;
        node->next_00 = next;
        node->previous_04 = previous;
        construct_native_render_alias_string_0044bcb0(
            static_cast<std::byte*>(raw) + 8, source, pool);
        return node;
    } catch (...) {
        // Exact responsibility of 004CE75C: free captured node and rethrow.
        // It does not release an embedded buffer or undo later list insertion.
        singleton_lifetime_free(raw);
        throw;
    }
}

NativeRenderResourceAliasNode* allocate_native_render_alias_node_004ce6f0(
    NativeRenderResourceAliasNode* next, NativeRenderResourceAliasNode* previous,
    const void* source, SizedStoragePool& pool) {
    return allocate_alias_node_with_pool(next, previous, source, pool);
}

NativeRenderResourceAliasNode* allocate_native_render_alias_node_004ce6f0(
    NativeRenderResourceAliasNode* next, NativeRenderResourceAliasNode* previous,
    const void* source, ActualNativeStringPoolStorage& pool) {
    return allocate_alias_node_with_pool(next, previous, source, pool);
}
} // namespace bsp
