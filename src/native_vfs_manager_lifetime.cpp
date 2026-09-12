#include "bsp/native_vfs_manager_lifetime.hpp"

#include "bsp/native_path_canonicalizer.hpp"
#include "bsp/native_physical_index_tree.hpp"
#include "bsp/native_string.hpp"
#include "bsp/native_string_pool_storage.hpp"
#include "bsp/native_string_vector.hpp"
#include "bsp/native_vfs_container_allocation.hpp"
#include "bsp/native_vfs_date_route.hpp"
#include "bsp/native_vfs_manager_base.hpp"
#include "bsp/native_vfs_mount_tree.hpp"
#include "bsp/native_vfs_request_list_lifetime.hpp"
#include "bsp/native_vfs_sequence_lifetime.hpp"
#include "bsp/native_vfs_string_tree.hpp"

namespace bsp {
namespace {
void* at(const void* base, std::uint32_t offset) noexcept {
    return reinterpret_cast<void*>(reinterpret_cast<std::uintptr_t>(base) + offset);
}
std::uint32_t word(const void* base, std::uint32_t offset = 0) noexcept {
    return *static_cast<const volatile std::uint32_t*>(at(base, offset));
}
void put(void* base, std::uint32_t offset, std::uint32_t value) noexcept {
    *static_cast<volatile std::uint32_t*>(at(base, offset)) = value;
}
void byte(void* base, std::uint32_t offset, std::uint8_t value) noexcept {
    *static_cast<volatile std::uint8_t*>(at(base, offset)) = value;
}
void* pointer(std::uint32_t value) noexcept { return reinterpret_cast<void*>(value); }
std::uint32_t address(const void* value) noexcept {
    return reinterpret_cast<std::uint32_t>(value);
}
void* link(const void* owner, std::uint32_t offset = 0) noexcept {
    return pointer(word(owner, offset));
}
NativeStringVectorStorage& string_vector(void* owner) noexcept {
    return *static_cast<NativeStringVectorStorage*>(owner);
}
void invalid(const SingletonLifetimeCallbacks& callbacks) {
    callbacks.invalid_parameter(callbacks.context);
}
void initialize_tree(void* tree, void* allocated, std::uint32_t nil_offset) noexcept {
    put(tree, 4, address(allocated));
    byte(allocated, nil_offset, 1);
    auto* current = link(tree, 4);
    put(current, 4, address(current));
    current = link(tree, 4);
    put(current, 0, address(current));
    current = link(tree, 4);
    put(current, 8, address(current));
    put(tree, 8, 0);
}

struct ManagerUnwind {
    void* owner;
    NativeVfsManagerLifetimeContext& context;
    int state = -1;
    ~ManagerUnwind() noexcept(false) {
        auto& strings = context.strings;
        const auto& crt = context.invalid_parameters;
        // E010A0/E01114: ten states, each chained to its predecessor. These
        // member actions reread current headers; no new allocation rollback.
        if (state >= 9) destroy_native_vfs_pair_vector_00be0350(at(owner, 0x94), strings);
        if (state >= 8) destroy_native_vfs_plain_list_thunk_007f8770(at(owner, 0x7c), nullptr);
        if (state >= 7) destroy_native_vfs_string_tree_004d74a0(at(owner, 0x6c), strings, crt);
        if (state >= 6) destroy_native_vfs_request_list_00be1d60(at(owner, 0x60), strings);
        if (state >= 5) destroy_native_physical_index_00be1700(at(owner, 0x54), strings, crt);
        if (state >= 4) destroy_native_string_vector_004d0fa0(string_vector(at(owner, 0x48)), strings);
        if (state >= 3) destroy_native_vfs_mount_tree_00be16c0(at(owner, 0x3c), strings, crt);
        if (state >= 2) destroy_native_vfs_plain_list_thunk_00bdb3c0(at(owner, 0x30), nullptr);
        if (state >= 1) destroy_native_string_header_0041dd20(at(owner, 0x0c), strings);
        if (state >= 0) destroy_native_vfs_manager_base_00bda790(owner,
            context.manager_0109ceec, context.lifetime);
    }
};
} // namespace

void* construct_native_vfs_manager_00be1dc0(void* owner,
    NativeVfsManagerLifetimeContext& context) {
    ManagerUnwind unwind{owner, context};
    construct_native_vfs_manager_base_00bda6f0(owner, context.manager_0109ceec,
        context.lifetime);
    put(owner, 0, 0x00d685b4u);
    put(owner, 4, 0);
    put(owner, 8, 0);
    unwind.state = 0;
    put(owner, 0x0c, 0);
    put(owner, 0x10, 0);
    put(owner, 0x14, 0);
    put(owner, 0x18, 0xffffffffu);
    put(owner, 0x1c, 0);
    byte(owner, 0x20, 0);
    auto* member = at(owner, 0x30);
    unwind.state = 1;
    put(owner, 0x24, 0);
    put(owner, 0x28, 0);
    put(owner, 0x2c, 0);
    auto* allocated = allocate_native_list_head_00bda960();
    put(member, 4, address(allocated));
    put(member, 8, 0);
    member = at(owner, 0x3c);
    unwind.state = 2;
    allocated = allocate_native_tree_node_00bdabf0();
    initialize_tree(member, allocated, 0x21);
    put(owner, 0x48, 0);
    put(owner, 0x4c, 0);
    put(owner, 0x50, 0);
    member = at(owner, 0x54);
    unwind.state = 4;
    allocated = allocate_native_tree_node_00bdaba0();
    initialize_tree(member, allocated, 0x1d);
    member = at(owner, 0x60);
    unwind.state = 5;
    allocated = allocate_native_list_head_00bda980();
    put(member, 4, address(allocated));
    put(member, 8, 0);
    member = at(owner, 0x6c);
    unwind.state = 6;
    allocated = allocate_native_tree_node_004c26b0();
    initialize_tree(member, allocated, 0x15);
    member = at(owner, 0x7c);
    unwind.state = 7;
    byte(owner, 0x78, 0);
    byte(owner, 0x79, 1);
    allocated = allocate_native_list_head_007f82f0();
    put(member, 4, address(allocated));
    put(member, 8, 0);
    put(owner, 0x88, 0);
    put(owner, 0x8c, 0);
    put(owner, 0x90, 0);
    put(owner, 0x94, 0);
    put(owner, 0x98, 0);
    put(owner, 0x9c, 0);
    unwind.state = 9;
    for (auto* path : context.path_examples)
        canonicalize_discard_native_path_00bdb970(path, context.canonicalizer);
    unwind.state = -1;
    return owner;
}

void destroy_native_vfs_manager_00be1f60(void* owner,
    NativeVfsManagerLifetimeContext& context) {
    ManagerUnwind unwind{owner, context};
    auto& strings = context.strings;
    const auto& crt = context.invalid_parameters;
    put(owner, 0, 0x00d685b4u);
    void* const file_log = context.file_access_log_0109cee8;
    unwind.state = 9;
    if (file_log != nullptr) {
        auto* const table = link(file_log);
        const auto target = word(table);
        context.virtual_calls.invoke_log_virtual0_00be1fa1(target, file_log, 1);
    }
    auto* const initial_head = link(owner, 0x40);
    auto* const first = link(initial_head);
    auto* const mounts = at(owner, 0x3c);
    std::uint32_t iterator[2] = {address(mounts), address(first)};
    auto* iterator_owner = mounts;
    for (;;) {
        auto* const last = link(mounts, 4);
        if (iterator_owner == nullptr || iterator_owner != mounts) invalid(crt);
        if (link(iterator, 4) == last) break;
        if (iterator_owner == nullptr) invalid(crt);
        if (link(iterator, 4) == link(iterator_owner, 4)) invalid(crt);
        auto* const provider = link(link(iterator, 4), 0x18);
        if (provider != nullptr) {
            auto* const table = link(provider);
            const auto target = word(table, 4);
            context.virtual_calls.invoke_provider_virtual4_00be1ffd(target, provider, 1);
        }
        advance_native_vfs_mount_iterator_00bd97e0(iterator, crt);
        iterator_owner = link(iterator);
    }
    auto* member = at(owner, 0x94);
    unwind.state = 8;
    resize_native_vfs_pair_vector_00bdec70(member, 0, strings);
    singleton_lifetime_free(link(member));
    // The native inline list algorithm matches this complete existing body,
    // including comparison-before-count-zero and current-head reloads.
    destroy_native_vfs_plain_list_007f8310(at(owner, 0x7c), nullptr);
    member = at(owner, 0x6c);
    auto* last = link(member, 4);
    auto* begin = link(last);
    std::uint32_t output[2];
    unwind.state = 6;
    erase_native_vfs_string_range_004d1a50(member, output, member, begin,
        member, last, strings, crt);
    singleton_lifetime_free(link(member, 4));
    put(member, 4, 0);
    put(member, 8, 0);

    auto* head = link(owner, 0x64);
    auto* current = link(head);
    put(head, 0, address(head));
    head = link(owner, 0x64);
    put(head, 4, address(head));
    const bool empty = current == link(owner, 0x64);
    unwind.state = 5;
    put(owner, 0x68, 0);
    if (!empty) {
        do {
            auto* const next = link(current);
            destroy_native_vfs_request_payload_00be1220(at(current, 8), strings);
            singleton_lifetime_free(current);
            current = next;
        } while (current != link(owner, 0x64));
    }
    singleton_lifetime_free(link(owner, 0x64));
    put(owner, 0x64, 0);
    member = at(owner, 0x54);
    last = link(member, 4);
    begin = link(last);
    unwind.state = 4;
    erase_native_physical_index_range_00be0c30(member, output, member, begin,
        member, last, strings, crt);
    singleton_lifetime_free(link(member, 4));
    put(member, 4, 0);
    put(member, 8, 0);
    member = at(owner, 0x48);
    unwind.state = 3;
    resize_native_string_vector_00427110(string_vector(member), 0, strings);
    singleton_lifetime_free(link(member));
    last = link(mounts, 4);
    begin = link(last);
    unwind.state = 2;
    erase_native_vfs_mount_range_00be0d00(mounts, output, mounts, begin,
        mounts, last, strings, crt);
    singleton_lifetime_free(link(mounts, 4));
    put(mounts, 4, 0);
    put(mounts, 8, 0);
    destroy_native_vfs_plain_list_00bdaed0(at(owner, 0x30), nullptr);
    const auto name_data = word(owner, 0x10);
    unwind.state = 0;
    if (name_data != 0) {
        const auto bytes = word(owner, 0x0c) + 1u;
        strings.release(reinterpret_cast<char*>(name_data), bytes);
    }
    unwind.state = -1;
    destroy_native_vfs_manager_base_00bda790(owner, context.manager_0109ceec,
        context.lifetime);
}

void* delete_native_vfs_manager_00be25c0(void* owner, std::uint32_t flags,
    NativeVfsManagerLifetimeContext& context) {
    destroy_native_vfs_manager_00be1f60(owner, context);
    if ((flags & 1u) != 0) singleton_lifetime_free(owner);
    return owner;
}
} // namespace bsp
