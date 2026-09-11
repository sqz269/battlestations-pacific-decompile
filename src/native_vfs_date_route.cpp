#include "bsp/native_vfs_date_route.hpp"

#include "bsp/native_physical_file_date.hpp"
#include "bsp/native_pooled_resource_path.hpp"
#include "bsp/native_pooled_string_substring.hpp"
#include "bsp/native_string_pool_storage.hpp"
#include "bsp/native_vfs_date_leaf_providers.hpp"
#include "bsp/singleton_lifetime.hpp"

#include <stdexcept>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native VFS date routes require MSVC Win32.
#endif

namespace bsp {
namespace {
const void* at(const void* p, std::uint32_t offset) noexcept {
    return reinterpret_cast<const void*>(reinterpret_cast<std::uintptr_t>(p) + offset);
}
void* at(void* p, std::uint32_t offset) noexcept {
    return reinterpret_cast<void*>(reinterpret_cast<std::uintptr_t>(p) + offset);
}
std::uint32_t word(const void* p, std::uint32_t offset = 0) noexcept {
    return *static_cast<const volatile std::uint32_t*>(at(p, offset));
}
void put(void* p, std::uint32_t offset, std::uint32_t value) noexcept {
    *static_cast<volatile std::uint32_t*>(at(p, offset)) = value;
}
void* pointer(const void* p, std::uint32_t offset = 0) noexcept {
    return reinterpret_cast<void*>(word(p, offset));
}
unsigned char byte(const void* p, std::uint32_t offset) noexcept {
    return *static_cast<const volatile unsigned char*>(at(p, offset));
}
void invalid(const SingletonLifetimeCallbacks& crt) {
    crt.invalid_parameter(crt.context);
}
void require_date_visitor_slot(const void* visitor, std::uint32_t offset,
    std::uint32_t expected, const NativeVfsDateRouteContext& context) {
    if (word(visitor) != 0x00d683b0)
        throw std::invalid_argument("Unimplemented native VFS visitor identity");
    if (word(context.actual_date_profile_00d683b0, offset) != expected)
        throw std::invalid_argument("Unimplemented current VFS date visitor slot");
}
} // namespace

void advance_native_vfs_mount_iterator_00bd97e0(void* iterator,
    const SingletonLifetimeCallbacks& crt) {
    if (!pointer(iterator)) invalid(crt);
    auto* node = pointer(iterator, 4);
    if (byte(node, 0x21) != 0) {
        invalid(crt); // Native tail call; returning handler returns to caller.
        return;
    }
    auto* child = pointer(node, 8);
    if (byte(child, 0x21) == 0) {
        auto* next = pointer(child);
        while (byte(next, 0x21) == 0) {
            child = next;
            next = pointer(child);
        }
        put(iterator, 4, reinterpret_cast<std::uint32_t>(child));
    } else {
        auto* parent = pointer(node, 4);
        while (byte(parent, 0x21) == 0) {
            if (pointer(iterator, 4) != pointer(parent, 8)) break;
            put(iterator, 4, reinterpret_cast<std::uint32_t>(parent));
            parent = pointer(parent, 4);
        }
        put(iterator, 4, reinterpret_cast<std::uint32_t>(parent));
    }
}

void reset_native_vfs_date_visitor_00bd90b0(void* visitor) noexcept {
    put(visitor, 0, 0x00d68380);
}

bool has_native_vfs_date_visitor_result_00bd9f00(const void* visitor) noexcept {
    return word(visitor, 4) != 0 || word(visitor, 8) != 0 ||
        word(visitor, 0x0c) != 0 || word(visitor, 0x10) != 0 ||
        word(visitor, 0x14) != 0;
}

void read_native_vfs_date_provider_00bd9e80(void* visitor,
    const void* payload, const void* name, NativeVfsDateRouteContext& context) {
    auto* provider = pointer(payload, 8);
    const auto selected = word(pointer(provider), 0x20);
    std::uint32_t hidden_output[5];
    void* returned;
    switch (selected) {
    case 0x00be5c80:
        returned = query_native_file_store_date_00be5c80(provider, hidden_output,
            name, context.physical.invalid_parameters);
        break;
    case 0x00bb9d50:
        returned = query_native_mpkg_file_date_00bb9d50(hidden_output, name);
        break;
    case 0x00bbb640:
        returned = query_native_msar_file_date_00bbb640(hidden_output, name);
        break;
    case 0x00bf3a80:
        returned = query_native_physical_file_date_00bf3a80(provider,
            hidden_output, name, context.physical);
        break;
    default:
        throw std::invalid_argument("Unimplemented current VFS provider date slot");
    }
    put(visitor, 4, word(returned));
    put(visitor, 8, word(returned, 4));
    put(visitor, 0x0c, word(returned, 8));
    put(visitor, 0x10, word(returned, 0x0c));
    put(visitor, 0x14, word(returned, 0x10));
}

void visit_native_vfs_date_mounts_00bdd0a0(void* manager, const void* name,
    void* visitor, NativeVfsDateRouteContext& context) {
    auto& strings = context.physical.strings;
    const auto& crt = context.physical.invalid_parameters;
    put(manager, 0x18, 0xffffffffu);
    std::uint32_t main_name[2] = {0, 0};
    assign_native_string_header_00425f40(main_name, name, strings);
    auto* head = pointer(manager, 0x40);
    auto* node = pointer(head);
    auto* const tree = at(manager, 0x3c);
    std::uint32_t iterator[2];
    put(iterator, 0, reinterpret_cast<std::uint32_t>(tree));
    put(iterator, 4, reinterpret_cast<std::uint32_t>(node));
    bool main_armed = true;
    std::uint32_t suffix_part[2];
    bool part_armed = false;
    std::uint32_t callback_name[2];
    bool callback_armed = false;
    try {
        auto* owner = tree;
        for (;;) {
            // Capture this iteration's head before the returning owner check.
            auto* const current_end = pointer(tree, 4);
            if (!owner || owner != tree) invalid(crt);
            if (node == current_end) break;
            if (!owner) invalid(crt);
            if (node == pointer(owner, 4)) invalid(crt);
            auto prefix_length = word(node, 0x10);
            auto* const payload = at(node, 0x10);
            bool matches = prefix_length == 0;
            if (!matches && prefix_length < word(main_name)) {
                std::uint32_t prefix_part[2];
                auto* returned_prefix = construct_native_string_substring_00469840(
                    main_name, prefix_part, 0, prefix_length, strings);
                // No prefix-temporary unwind state is present in E00724.
                matches = equal_native_string_headers_00435c40(returned_prefix, payload);
                if (matches) {
                    const auto current_prefix_length = word(payload);
                    auto* data = pointer(main_name, 4);
                    matches = byte(data, current_prefix_length) == '/';
                }
                destroy_native_string_header_0041dd20(prefix_part, strings);
            }
            if (matches) {
                prefix_length = word(payload);
                const void* selected_name = main_name;
                if (prefix_length != 0) {
                    selected_name = construct_native_string_substring_00469840(
                        main_name, suffix_part, prefix_length + 1u, 0x7fffffffu, strings);
                    part_armed = true; // State1 only after substring returns.
                }
                put(callback_name, 0, 0);
                put(callback_name, 4, 0);
                assign_native_string_header_00425f40(callback_name, selected_name, strings);
                callback_armed = true; // State3; failed initial copy is not owned.
                if (part_armed) {
                    part_armed = false;
                    destroy_native_string_header_0041dd20(suffix_part, strings);
                }
                auto* current_node = pointer(iterator, 4);
                auto* current_owner = pointer(iterator);
                if (current_node == pointer(current_owner, 4)) invalid(crt);
                require_date_visitor_slot(visitor, 4, 0x00bd9e80, context);
                read_native_vfs_date_provider_00bd9e80(visitor, payload, callback_name, context);
                // Provider work can change the visitor/table; do not cache +8.
                require_date_visitor_slot(visitor, 8, 0x00bd9f00, context);
                const bool stop = has_native_vfs_date_visitor_result_00bd9f00(visitor);
                callback_armed = false;
                destroy_native_string_header_0041dd20(callback_name, strings);
                if (stop) break;
            }
            advance_native_vfs_mount_iterator_00bd97e0(iterator, crt);
            node = pointer(iterator, 4);
            owner = pointer(iterator);
        }
        main_armed = false;
        destroy_native_string_header_0041dd20(main_name, strings);
    } catch (...) {
        if (callback_armed) destroy_native_string_header_0041dd20(callback_name, strings);
        if (part_armed) destroy_native_string_header_0041dd20(suffix_part, strings);
        if (main_armed) destroy_native_string_header_0041dd20(main_name, strings);
        throw;
    }
}

void* query_native_vfs_file_date_00bdd340(void* manager, void* output,
    const void* name, NativeVfsDateRouteContext& context) {
    std::uint32_t visitor[6];
    put(visitor, 0, 0x00d683b0);
    put(visitor, 0x14, 0); put(visitor, 0x10, 0); put(visitor, 0x0c, 0);
    put(visitor, 8, 0); put(visitor, 4, 0);
    auto& strings = context.physical.strings;
    std::uint32_t copied_name[2] = {0, 0};
    bool name_armed = false;
    try {
        assign_native_string_header_00425f40(copied_name, name, strings);
        name_armed = true;
        normalize_native_resource_path_header_00bee690(copied_name, strings);
        visit_native_vfs_date_mounts_00bdd0a0(manager, copied_name, visitor, context);
        const auto first = word(visitor, 4);
        const auto second = word(visitor, 8);
        const auto third = word(visitor, 0x0c);
        put(output, 0, first);
        const auto fourth = word(visitor, 0x10);
        put(output, 4, second);
        const auto fifth = word(visitor, 0x14);
        put(output, 8, third); put(output, 0x0c, fourth); put(output, 0x10, fifth);
        name_armed = false;
        destroy_native_string_header_0041dd20(copied_name, strings);
        // Normal native return does not call BD90B0; it is unwind-only here.
    } catch (...) {
        if (name_armed) destroy_native_string_header_0041dd20(copied_name, strings);
        reset_native_vfs_date_visitor_00bd90b0(visitor);
        throw;
    }
    return output;
}
} // namespace bsp
