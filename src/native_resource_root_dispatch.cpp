#include "bsp/native_resource_root_dispatch.hpp"
#include "bsp/native_resource_fallback_item.hpp"
#include "bsp/native_resource_manager_trees.hpp"
#include "bsp/native_resource_node_traversal.hpp"
#include "bsp/native_resource_pointer_array.hpp"
#include "bsp/native_resource_reader_references.hpp"
#include "bsp/native_resource_value_reads.hpp"
#include "bsp/native_vfs_date_leaf_providers.hpp"
#include "bsp/singleton_lifetime.hpp"
#include <cstring>
#include <exception>

namespace bsp {
namespace {
using U = std::uint32_t;
static_assert(sizeof(void*) == 4);
void* at(const void* p, U offset) noexcept {
    return reinterpret_cast<void*>(reinterpret_cast<std::uintptr_t>(p) + offset);
}
U word(const void* p, U offset = 0) noexcept {
    return *static_cast<const volatile U*>(at(p, offset));
}
void put(void* p, U offset, U value) noexcept {
    *static_cast<volatile U*>(at(p, offset)) = value;
}
void* ptr(U value) noexcept { return reinterpret_cast<void*>(value); }
U bits(const void* p) noexcept { return static_cast<U>(reinterpret_cast<std::uintptr_t>(p)); }
std::int32_t signed_bits(U value) noexcept {
    std::int32_t result; std::memcpy(&result, &value, 4); return result;
}
bool tag_is(void* child, const char* value) {
    const auto data = word(child, 0x14);
    return data != 0 && _stricmp(static_cast<const char*>(ptr(data)), value) == 0;
}
void cleanup_child(void* handle, NativeResourceRootDispatchContext& context) noexcept {
    try { release_native_structured_node_handle_00be9ed0(handle, context.hierarchy.reads.streams); }
    catch (...) { std::terminate(); }
}
void invalid(NativeResourceRootDispatchContext& context) {
    const auto& callback = context.invalid_parameters;
    callback.invalid_parameter(callback.context);
}
} // namespace

void append_native_resource_item_00b87aa0(void* resource, void* item) {
    auto capacity = word(resource, 0x18);
    const auto count = word(resource, 0x14);
    auto* const header = at(resource, 0x10);
    if (count == capacity) {
        capacity += 16u;
        if (signed_bits(capacity) <= 16) capacity = 16;
        reserve_native_resource_item_pointers_00b872f0(header, signed_bits(capacity));
    }
    const auto current_count = word(header, 4);
    const auto data = word(header);
    auto* const destination = ptr(data + current_count * 4u);
    if (destination) put(destination, 0, bits(item));
    put(header, 4, word(header, 4) + 1u);
}

void begin_native_resource_root_00b1fe40() noexcept {}
void end_native_resource_root_00b28570() noexcept {}

NativeDefaultResourceDispatchCalls::NativeDefaultResourceDispatchCalls(NativeResourceDispatchCalls& other)
    : other_(other) {}
void NativeDefaultResourceDispatchCalls::renderer_hook(std::uintptr_t target, void* renderer) {
    if (target == 0x00b1fe40) begin_native_resource_root_00b1fe40();
    else if (target == 0x00b28570) end_native_resource_root_00b28570();
    else other_.renderer_hook(target, renderer);
}
void* NativeDefaultResourceDispatchCalls::parse_item(std::uintptr_t target, void* parser, void* handle) {
    return other_.parse_item(target, parser, handle);
}
void NativeDefaultResourceDispatchCalls::append_item(std::uintptr_t target, void* resource, void* item) {
    if (target == 0x00b87aa0) append_native_resource_item_00b87aa0(resource, item);
    else other_.append_item(target, resource, item);
}

void dispatch_native_resource_items_00b7e970(void* manager, void* handle,
    NativeResourceRootDispatchContext& context) {
    auto& reads = context.hierarchy.reads;
    if (!native_resource_node_has_remaining_00715bf0(handle)) return;
    auto* const tree = at(manager, 8);
    void* child = nullptr;
    void* allocation = nullptr;
    int state = -1;
    try {
        do {
            create_native_resource_child_00bea680(handle, &child, reads);
            auto* const query = at(child, 0x10);
            state = 0;
            auto* const candidate = lower_bound_native_resource_parser_name_00b7df40(tree, query);
            if (!tree) invalid(context);
            void* selected;
            if (candidate == ptr(word(tree, 4)) ||
                less_native_string_headers_00443d00(query, at(candidate, 0x0c))) {
                selected = ptr(word(tree, 4));
            } else selected = candidate;
            // The two private iterators both carry the captured tree owner.
            auto* const owner = tree;
            auto* const current_end = ptr(word(tree, 4));
            if (!owner || owner != tree) invalid(context);
            void* item;
            if (selected != current_end) {
                if (!owner) invalid(context);
                if (selected == ptr(word(owner, 4))) invalid(context);
                auto* const parser = ptr(word(selected, 0x14));
                const auto table = word(parser);
                const auto target = word(ptr(table), 8);
                item = context.calls.parse_item(target, parser, &child);
            } else {
                allocation = singleton_lifetime_allocate({SingletonAllocationKind::object, 8, 8});
                state = 1;
                item = allocation ? construct_native_resource_fallback_item_00b86930(allocation) : nullptr;
                state = 0;
                skip_native_resource_node_00be9c40(&child, reads);
            }
            auto* const resource = ptr(word(manager, 0x24));
            const auto table = word(resource);
            const auto target = word(ptr(table), 0x0c);
            context.calls.append_item(target, resource, item);
            state = -1;
            release_native_structured_node_handle_00be9ed0(&child, reads.streams);
        } while (native_resource_node_has_remaining_00715bf0(handle));
    } catch (...) {
        if (state == 1) singleton_lifetime_free(allocation);
        if (state >= 0) cleanup_child(&child, context);
        throw;
    }
}

void dispatch_native_resource_root_00b7f430(void* manager, void* handle,
    NativeResourceRootDispatchContext& context) {
    auto& reads = context.hierarchy.reads;
    auto* renderer = context.renderer_00f8d394;
    auto table = word(renderer);
    auto target = word(ptr(table), 0x50);
    context.calls.renderer_hook(target, renderer);
    read_native_resource_node_control_00be9a40(handle, reads);
    void* child = nullptr;
    bool active = false;
    try {
        while (native_resource_node_has_remaining_00715bf0(handle)) {
            create_native_resource_child_00bea680(handle, &child, reads);
            active = true;
            if (tag_is(child, "Resource")) {
                dispatch_native_resource_items_00b7e970(manager, &child, context);
            } else if (tag_is(child, "Hierarchy")) {
                parse_native_resource_hierarchy_00b7f100(manager, &child, context.hierarchy);
            } else if (tag_is(child, "BoundingBox")) {
                U box[6];
                read_native_resource_bounds_00b93310(&child, box, reads);
                auto* const resource = ptr(word(manager, 0x24));
                for (U i = 0; i != 6; ++i) put(resource, 0x28 + i * 4, box[i]);
            } else skip_native_resource_node_00be9c40(&child, reads);
            active = false;
            release_native_structured_node_handle_00be9ed0(&child, reads.streams);
        }
    } catch (...) {
        if (active) cleanup_child(&child, context);
        throw;
    }
    renderer = context.renderer_00f8d394;
    table = word(renderer);
    target = word(ptr(table), 0x54);
    context.calls.renderer_hook(target, renderer);
}
} // namespace bsp
