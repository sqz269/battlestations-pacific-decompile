#include "bsp/native_vfs_enumeration.hpp"

#include "bsp/native_alias_count_growth.hpp"
#include "bsp/native_physical_file_date.hpp"
#include "bsp/native_pooled_string_substring.hpp"
#include "bsp/native_render_resource_alias_nodes.hpp"
#include "bsp/native_string.hpp"
#include "bsp/native_string_pool_storage.hpp"
#include "bsp/native_vfs_date_route.hpp"
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
std::uint32_t word(const void* base, std::uint32_t offset = 0) noexcept {
    return *static_cast<const volatile std::uint32_t*>(at(base, offset));
}
void put(void* base, std::uint32_t offset, std::uint32_t value) noexcept {
    *static_cast<volatile std::uint32_t*>(at(base, offset)) = value;
}
void* pointer(const void* base, std::uint32_t offset = 0) noexcept {
    return reinterpret_cast<void*>(word(base, offset));
}
std::uint8_t byte(const void* base, std::uint32_t offset) noexcept {
    return *static_cast<const volatile std::uint8_t*>(at(base, offset));
}
void put_byte(void* base, std::uint32_t offset, std::uint8_t value) noexcept {
    *static_cast<volatile std::uint8_t*>(at(base, offset)) = value;
}
void invalid(const SingletonLifetimeCallbacks& callbacks) {
    callbacks.invalid_parameter(callbacks.context);
}
void require_entry(const void* visitor, std::uint32_t slot, std::uint32_t expected,
    const NativeVfsEnumerationContext& context) {
    const auto profile = word(visitor);
    if (profile != 0x00d6846cu ||
        context.actual_visitor_profile_00d6846c !=
            reinterpret_cast<const void*>(static_cast<std::uintptr_t>(profile)) ||
        word(context.actual_visitor_profile_00d6846c, slot) != expected)
        throw std::invalid_argument("Unsupported current VFS enumeration visitor entry");
}

// BDBE20: current 14h visitor identity, copied extension, low flag and
// caller's actual list. Only those reached bytes are written.
void construct_00bdbe20(void* visitor, const void* extension, std::uint32_t flags,
    void* output, ActualNativeStringPoolStorage& strings) {
    put(visitor, 0, 0x00d6846cu);
    put(visitor, 4, 0);
    put(visitor, 8, 0);
    copy_native_string_header_00be0a30_fragment(at(visitor, 4), strings, extension);
    put_byte(visitor, 0x0c, static_cast<std::uint8_t>(flags));
    put(visitor, 0x10, reinterpret_cast<std::uint32_t>(output));
}

// BDB6E0: release the current extension data before replacing identity with
// D68380. This source retains the established noexcept pooled-return boundary.
void destroy_00bdb6e0(void* visitor, ActualNativeStringPoolStorage& strings) noexcept {
    auto* const data = static_cast<char*>(pointer(visitor, 8));
    if (data) strings.release(data, word(visitor, 4) + 1u);
    put(visitor, 0, 0x00d68380u);
}

// BE0FC0: linear actual-list lookup and first-spelling tail append. The
// by-value header is consumed, including its own pooled return at the end.
void append_unique_00be0fc0(void* actual_list, std::uint32_t by_value[2],
    NativeVfsEnumerationContext& context) {
    try {
        void* const head = pointer(actual_list, 4);
        void* cursor = pointer(head);
        for (; cursor != head; cursor = pointer(cursor)) {
            if (word(cursor, 8) == by_value[0] &&
                (by_value[0] == 0 ||
                 _stricmp(static_cast<const char*>(pointer(cursor, 0x0c)),
                     reinterpret_cast<const char*>(by_value[1])) == 0)) break;
        }
        if (cursor == head) {
            auto* current_head = static_cast<NativeRenderResourceAliasNode*>(pointer(actual_list, 4));
            auto* inserted = allocate_native_render_alias_node_004ce6f0(current_head,
                static_cast<NativeRenderResourceAliasNode*>(pointer(current_head, 4)),
                by_value, context.strings);
            grow_native_alias_list_count_004ce780(actual_list, 1);
            put(current_head, 4, reinterpret_cast<std::uint32_t>(inserted));
            put(pointer(inserted, 4), 0, reinterpret_cast<std::uint32_t>(inserted));
        } else {
            context.duplicates.rejected_duplicate(by_value[1]
                ? reinterpret_cast<const char*>(by_value[1]) : context.empty_name_0109cef0);
        }
    } catch (...) {
        auto* const data = reinterpret_cast<char*>(by_value[1]);
        if (data) context.strings.release(data, by_value[0] + 1u);
        throw;
    }
    auto* const current_data = reinterpret_cast<char*>(by_value[1]);
    if (current_data) context.strings.release(current_data, by_value[0] + 1u);
}

// BE1130: capture provider+14 after reading the current mount provider, then
// collect actual0Ch vector names and feed them one by one to BE0FC0.
void enumerate_mount_00be1130(void* visitor, const void* payload,
    const void* suffix, NativeVfsEnumerationContext& context) {
    NativeStringVectorStorage names{};
    auto* provider = pointer(payload, 8);
    const auto profile = word(provider);
    const auto selected = word(reinterpret_cast<const void*>(
        static_cast<std::uintptr_t>(profile)), 0x14);
    const std::uint32_t flags = byte(visitor, 0x0c);
    try {
        context.providers.enumerate_entry(selected, provider, suffix, at(visitor, 4),
            flags, names);
        for (std::int32_t i = 0; i < names.count_04; ++i) {
            const void* source = names.data_00 + i;
            std::uint32_t copied[2]{0, 0};
            copy_native_string_header_00be0a30_fragment(copied, context.strings, source);
            append_unique_00be0fc0(pointer(visitor, 0x10), copied, context);
        }
    } catch (...) {
        destroy_native_string_vector_004d0fa0(names, context.strings);
        throw;
    }
    destroy_native_string_vector_004d0fa0(names, context.strings);
}
} // namespace

void visit_native_vfs_enumeration_mounts_00bdd0a0(void* manager,
    const void* directory, void* visitor, NativeVfsEnumerationContext& context) {
    auto& strings = context.strings;
    const auto& crt = context.invalid_parameters;
    put(manager, 0x18, 0xffffffffu);
    std::uint32_t name[2]{0, 0};
    assign_native_string_header_00425f40(name, directory, strings);
    void* tree = at(manager, 0x3c);
    void* head = pointer(tree, 4);
    void* node = pointer(head);
    std::uint32_t iterator[2]{reinterpret_cast<std::uint32_t>(tree),
        reinterpret_cast<std::uint32_t>(node)};
    bool name_armed = true;
    std::uint32_t suffix[2];
    bool suffix_armed = false;
    std::uint32_t callback_name[2];
    bool callback_armed = false;
    try {
        void* owner = tree;
        for (;;) {
            void* const current_end = pointer(tree, 4);
            if (!owner || owner != tree) invalid(crt);
            if (node == current_end) break;
            if (!owner) invalid(crt);
            if (node == pointer(owner, 4)) invalid(crt);
            auto* payload = at(node, 0x10);
            auto prefix_length = word(payload);
            bool matches = prefix_length == 0;
            if (!matches && prefix_length < word(name)) {
                std::uint32_t prefix[2];
                auto* compared = construct_native_string_substring_00469840(
                    name, prefix, 0, prefix_length, strings);
                matches = equal_native_string_headers_00435c40(compared, payload);
                if (matches)
                    matches = byte(pointer(name, 4), word(payload)) == '/';
                destroy_native_string_header_0041dd20(prefix, strings);
            }
            if (matches) {
                prefix_length = word(payload);
                const void* selected = name;
                if (prefix_length != 0) {
                    selected = construct_native_string_substring_00469840(name,
                        suffix, prefix_length + 1u, 0x7fffffffu, strings);
                    suffix_armed = true;
                }
                put(callback_name, 0, 0);
                put(callback_name, 4, 0);
                assign_native_string_header_00425f40(callback_name, selected, strings);
                callback_armed = true;
                if (suffix_armed) {
                    suffix_armed = false;
                    destroy_native_string_header_0041dd20(suffix, strings);
                }
                if (pointer(iterator, 4) == pointer(pointer(iterator), 4)) invalid(crt);
                require_entry(visitor, 4, 0x00be1130u, context);
                enumerate_mount_00be1130(visitor, payload, callback_name, context);
                require_entry(visitor, 8, 0x00bdbeb0u, context);
                callback_armed = false;
                destroy_native_string_header_0041dd20(callback_name, strings);
                // BDBEB0 always returns false: continue through every mount.
            }
            advance_native_vfs_mount_iterator_00bd97e0(iterator, crt);
            node = pointer(iterator, 4);
            owner = pointer(iterator);
        }
        name_armed = false;
        destroy_native_string_header_0041dd20(name, strings);
    } catch (...) {
        if (callback_armed) destroy_native_string_header_0041dd20(callback_name, strings);
        if (suffix_armed) destroy_native_string_header_0041dd20(suffix, strings);
        if (name_armed) destroy_native_string_header_0041dd20(name, strings);
        throw;
    }
}

void enumerate_native_vfs_resources_00bdd990(void* manager,
    const void* directory, const void* extension, std::uint32_t flags,
    void* output, NativeVfsEnumerationContext& context) {
    alignas(4) std::byte visitor[0x14];
    construct_00bdbe20(visitor, extension, flags, output, context.strings);
    try {
        visit_native_vfs_enumeration_mounts_00bdd0a0(manager, directory, visitor, context);
    } catch (...) {
        destroy_00bdb6e0(visitor, context.strings);
        throw;
    }
    destroy_00bdb6e0(visitor, context.strings);
}
} // namespace bsp
