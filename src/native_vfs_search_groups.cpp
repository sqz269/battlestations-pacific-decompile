#include "bsp/native_vfs_search_groups.hpp"

#include "bsp/native_alias_count_growth.hpp"
#include "bsp/native_render_resource_alias_nodes.hpp"
#include "bsp/native_render_resource_record_construction.hpp"
#include "bsp/native_string.hpp"
#include "bsp/native_string_pool_storage.hpp"
#include "bsp/native_vfs_request_list_lifetime.hpp"
#include "bsp/singleton_lifetime.hpp"

#include <cstddef>
#include <cstdint>
#include <cstring>
#include <stdexcept>

namespace bsp {
namespace {
void* at(const void* base, std::uint32_t offset) noexcept {
    return reinterpret_cast<void*>(reinterpret_cast<std::uintptr_t>(base) + offset);
}
template<class T> volatile T& field(const void* base, std::uint32_t offset) noexcept {
    return *reinterpret_cast<volatile T*>(at(base, offset));
}
void* link(const void* base, std::uint32_t offset) noexcept {
    return field<void*>(base, offset);
}
struct Header { std::uint32_t length; char* data; };
static_assert(sizeof(Header) == 8);

struct StringOwner {
    Header header{};
    ActualNativeStringPoolStorage& strings;
    explicit StringOwner(ActualNativeStringPoolStorage& pool) : strings(pool) {}
    ~StringOwner() noexcept {
        if (header.data) destroy_native_string_header_0041dd20(&header, strings);
    }
    void copy(const void* source) {
        copy_native_string_header_00be0a30_fragment(&header, strings, source);
    }
    void lower() noexcept { lowercase_native_string_header_004bcc00(&header); }
    void directory() {
        for (std::uint32_t i = 0; i < header.length; ++i)
            if (header.data[i] == '\\') header.data[i] = '/';
        // Native indexes data[length-1], including the unsafe empty case.
        const char* current = header.data ? header.data : "";
        if (current[header.length - 1] != '/') {
            const auto old = header.length;
            resize_native_string_header_0041dd40(&header, strings, old + 1u, true);
            header.data[old] = '/';
        }
    }
};

void invalid(const SingletonLifetimeCallbacks& callbacks) {
    callbacks.invalid_parameter(callbacks.context);
}

// BE10A0 creates a 20h payload from the already-lowercased group string.
// List owner+0 words are native opaque preimages: never initialize or copy.
struct Payload {
    alignas(4) std::byte bytes[0x20];
    ActualNativeStringPoolStorage& strings;
    int state{-1};
    explicit Payload(ActualNativeStringPoolStorage& storage) : strings(storage) {}
    void* data() noexcept { return bytes; }
    ~Payload() noexcept {
        if (state >= 2) destroy_native_vfs_string_list_004d2640(at(bytes, 0x14), strings);
        if (state >= 1) destroy_native_vfs_string_list_004d2640(at(bytes, 8), strings);
        if (state >= 0) destroy_native_string_header_0041dd20(bytes, strings);
    }
    void construct(const void* source) {
        field<std::uint32_t>(bytes, 0) = 0;
        field<char*>(bytes, 4) = nullptr;
        copy_native_string_header_00be0a30_fragment(bytes, strings, source);
        state = 0;
        field<void*>(bytes, 0x0c) = allocate_native_render_alias_sentinel_004c3020();
        field<std::uint32_t>(bytes, 0x10) = 0;
        state = 1;
        field<void*>(bytes, 0x18) = allocate_native_render_alias_sentinel_004c3020();
        field<std::uint32_t>(bytes, 0x1c) = 0;
        state = 2;
    }
};

// BE1A20/BE13E0 reached normal path: allocate 28h, write links, deep-copy
// the base string and both initially empty list sentinels. On copy failure
// only this detached node is unwound; a later count failure is the caller's.
void* allocate_group(void* next, void* previous, void* source,
    ActualNativeStringPoolStorage& strings) {
    void* node = singleton_lifetime_allocate({SingletonAllocationKind::object, 0x28, 0x28});
    field<void*>(node, 0) = next;
    field<void*>(node, 4) = previous;
    auto* payload = at(node, 8);
    field<std::uint32_t>(payload, 0) = 0;
    field<char*>(payload, 4) = nullptr;
    int state = -1;
    try {
        copy_native_string_header_00be0a30_fragment(payload, strings, source);
        state = 0;
        field<void*>(payload, 0x0c) = allocate_native_render_alias_sentinel_004c3020();
        field<std::uint32_t>(payload, 0x10) = 0;
        state = 1;
        field<void*>(payload, 0x18) = allocate_native_render_alias_sentinel_004c3020();
        field<std::uint32_t>(payload, 0x1c) = 0;
        return node;
    } catch (...) {
        if (state >= 1) destroy_native_vfs_string_list_004d2640(at(payload, 8), strings);
        if (state >= 0) destroy_native_string_header_0041dd20(payload, strings);
        singleton_lifetime_free(node);
        throw;
    }
}

void grow_group_count(void* list) {
    auto& count = field<std::uint32_t>(list, 8);
    const auto prior = count;
    if (0x07ffffffu - prior < 1u) throw std::length_error("list<T> too long");
    count = prior + 1u;
}

void* find_group(void* manager, const void* group,
    ActualNativeStringPoolStorage& strings, const SingletonLifetimeCallbacks& callbacks) {
    StringOwner lower(strings);
    lower.copy(group);
    lower.lower(); // BDC230 normalizes a separate copy before its scan.
    void* list = at(manager, 0x60);
    void* head = link(list, 4);
    for (void* node = link(head, 0); node != head; node = link(node, 0)) {
        if (node == head) invalid(callbacks); // checked iterator's returning boundary
        const auto length = field<std::uint32_t>(node, 8);
        if (length == lower.header.length &&
            (length == 0 || _stricmp(field<char*>(node, 0x0c), lower.header.data) == 0))
            return node;
    }
    return head;
}

void* select_values(void* manager, void* node, std::uint32_t selector,
    const SingletonLifetimeCallbacks& callbacks) {
    void* list = at(manager, 0x60);
    if (!list || node == link(list, 4)) invalid(callbacks);
    if (selector == 0) return at(node, 0x10);
    if (selector == 1) return at(node, 0x1c);
    return nullptr; // BDB2E0's unmatched selector result.
}
} // namespace

void register_native_vfs_search_group_value_00be2310(void* manager,
    const void* group, const void* value, std::uint32_t selector,
    ActualNativeStringPoolStorage& strings, const SingletonLifetimeCallbacks& callbacks) {
    void* list = at(manager, 0x60);
    void* node = find_group(manager, group, strings, callbacks);
    StringOwner lower_value(strings);
    lower_value.copy(value);
    lower_value.lower();
    if (selector == 1) lower_value.directory();

    void* head = link(list, 4);
    void* values;
    if (node == head) {
        StringOwner lower_group(strings);
        lower_group.copy(group);
        lower_group.lower();
        Payload temporary(strings);
        temporary.construct(&lower_group.header);
        void* const first = link(head, 0);
        void* const previous = link(first, 4);
        void* inserted = allocate_group(first, previous, temporary.data(), strings);
        grow_group_count(list); // Count first; a failure leaves the new node detached.
        field<void*>(first, 4) = inserted;
        field<void*>(link(inserted, 4), 0) = inserted;
        node = link(head, 0);
        values = select_values(manager, node, selector, callbacks);
        // Native destroys the temporary payload and lower-group copy here,
        // before it allocates the value node.
    } else {
        values = select_values(manager, node, selector, callbacks);
        void* const value_head = link(values, 4);
        for (void* cursor = link(value_head, 0); cursor != value_head;
             cursor = link(cursor, 0)) {
            const auto length = field<std::uint32_t>(cursor, 8);
            if (length == lower_value.header.length &&
                (length == 0 || _stricmp(field<char*>(cursor, 0x0c),
                    lower_value.header.data) == 0)) return;
        }
    }

    void* const value_head = link(values, 4);
    auto* const inserted = allocate_native_render_alias_node_004ce6f0(
        static_cast<NativeRenderResourceAliasNode*>(value_head),
        static_cast<NativeRenderResourceAliasNode*>(link(value_head, 4)),
        &lower_value.header, strings);
    grow_native_alias_list_count_004ce780(values, 1);
    field<void*>(value_head, 4) = inserted;
    field<void*>(link(inserted, 4), 0) = inserted;
}

void register_native_vfs_search_extension_00be25e0(void* manager,
    const void* group, const void* extension, ActualNativeStringPoolStorage& strings,
    const SingletonLifetimeCallbacks& callbacks) {
    register_native_vfs_search_group_value_00be2310(
        manager, group, extension, 0, strings, callbacks);
}

void register_native_vfs_search_directory_00be2600(void* manager,
    const void* group, const void* directory, ActualNativeStringPoolStorage& strings,
    const SingletonLifetimeCallbacks& callbacks) {
    StringOwner normalized(strings);
    normalized.copy(directory);
    normalized.lower();
    normalized.directory();
    register_native_vfs_search_group_value_00be2310(
        manager, group, &normalized.header, 1, strings, callbacks);
}
} // namespace bsp
