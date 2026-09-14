#include "bsp/native_vfs_lookup_routes.hpp"

#include "bsp/native_mpak_enumeration.hpp"
#include "bsp/native_mpak_open.hpp"
#include "bsp/native_physical_file_date.hpp"
#include "bsp/native_pooled_resource_path.hpp"
#include "bsp/native_pooled_string_substring.hpp"
#include "bsp/native_string_pool_storage.hpp"
#include "bsp/native_vfs_date_route.hpp"
#include "bsp/native_vfs_device_route.hpp"
#include "bsp/native_vfs_lookup_leaves.hpp"
#include "bsp/native_vfs_pending_routes.hpp"
#include "bsp/singleton_lifetime.hpp"

#include <cstring>
#include <stdexcept>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native VFS lookup routes require MSVC Win32.
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
std::uint8_t byte(const void* p, std::uint32_t offset) noexcept {
    return *static_cast<const volatile std::uint8_t*>(at(p, offset));
}
void put_byte(void* p, std::uint32_t offset, std::uint8_t value) noexcept {
    *static_cast<volatile std::uint8_t*>(at(p, offset)) = value;
}
void invalid(const SingletonLifetimeCallbacks& crt) {
    crt.invalid_parameter(crt.context);
}
std::uint32_t lookup_visitor_slot(const void* visitor, std::uint32_t offset,
    const NativeVfsLookupRouteContext& context) {
    const void* profile;
    switch (word(visitor)) {
    case 0x00d68398: profile = context.actual_exists_profile_00d68398; break;
    case 0x00d683e8: profile = context.actual_name_probe_profile_00d683e8; break;
    case 0x00d683f4:
        if (!context.device) throw std::invalid_argument("Native VFS device route is not bound");
        profile = context.device->actual_device_profile_00d683f4; break;
    case 0x00d68478:
        if (!context.pending) throw std::invalid_argument("Native VFS pending route is not bound");
        profile = context.pending->actual_pending_profile_00d68478; break;
    default: throw std::invalid_argument("Unimplemented native VFS lookup visitor identity");
    }
    return word(profile, offset);
}
void dispatch_lookup_visit(void* visitor, const void* payload, const void* name,
    NativeVfsLookupRouteContext& context) {
    switch (lookup_visitor_slot(visitor, 4, context)) {
    case 0x00bd90d0: read_native_vfs_exists_provider_00bd90d0(visitor, payload, name, context); return;
    case 0x00bdbc00: read_native_vfs_name_probe_provider_00bdbc00(visitor, payload, name, context); return;
    case 0x00bdbc70:
        if (!context.device) throw std::invalid_argument("Native VFS device route is not bound");
        read_native_vfs_device_provider_00bdbc70(visitor, payload, name, *context.device); return;
    case 0x00bdc1e0:
        if (!context.pending) throw std::invalid_argument("Native VFS pending route is not bound");
        read_native_vfs_pending_provider_00bdc1e0(visitor, payload, name, *context.pending); return;
    default: throw std::invalid_argument("Unimplemented current VFS lookup visitor call slot");
    }
}
std::uint8_t dispatch_lookup_stop(const void* visitor, NativeVfsLookupRouteContext& context) {
    switch (lookup_visitor_slot(visitor, 8, context)) {
    case 0x00bd90f0: return read_native_vfs_exists_result_00bd90f0(visitor);
    case 0x00bdb5d0: return read_native_vfs_name_probe_result_00bdb5d0(visitor);
    case 0x00bdb670: return read_native_vfs_device_result_00bdb670(visitor);
    case 0x00bdc1d0: return read_native_vfs_pending_result_00bdc1d0(visitor);
    default: throw std::invalid_argument("Unimplemented current VFS lookup visitor stop slot");
    }
}
} // namespace

std::uint8_t probe_native_mpkg_name_00bb8640(void*, void*) noexcept { return 0; }
std::uint8_t probe_native_msar_name_00bba080(void*, void*) noexcept { return 0; }

std::uint8_t read_native_vfs_exists_result_00bd90f0(const void* visitor) noexcept {
    return byte(visitor, 4);
}
std::uint8_t read_native_vfs_name_probe_result_00bdb5d0(const void* visitor) noexcept {
    return byte(visitor, 4);
}

std::uint8_t invoke_native_vfs_provider_contains(std::uintptr_t selected,
    void* provider, const void* name, NativeVfsLookupRouteContext& context) {
    std::uint8_t result;
    switch (selected) {
    case 0x00be5c00:
        result = contains_native_file_store_file_00be5c00(provider, name, context.physical.invalid_parameters); break;
    case 0x00bb8e80:
        result = contains_native_mpkg_file_00bb8e80(provider, name); break;
    case 0x00bba710:
        result = contains_native_msar_file_00bba710(provider, name, context.physical.invalid_parameters); break;
    case 0x00bf3f70:
        result = exists_native_physical_path_00bf3f70(provider, name, context.physical); break;
    case 0x00bb4b20:
        result = contains_native_mpak_file_00bb4b20(provider, name, context.physical.invalid_parameters); break;
    default: throw std::invalid_argument("Unimplemented current VFS provider contains slot");
    }
    return result;
}

void read_native_vfs_exists_provider_00bd90d0(void* visitor, const void* payload,
    const void* name, NativeVfsLookupRouteContext& context) {
    auto* provider = pointer(payload, 8);
    const auto selected = word(pointer(provider), 0x10);
    const auto result = invoke_native_vfs_provider_contains(selected, provider, name, context);
    put_byte(visitor, 4, result);
}

void read_native_vfs_name_probe_provider_00bdbc00(void* visitor, const void* payload,
    const void* name, NativeVfsLookupRouteContext& context) {
    auto* const owned_name = at(visitor, 8);
    assign_native_string_header_00425f40(owned_name, name, context.physical.strings);
    // Assignment may alter current payload/provider storage; capture it afterwards.
    auto* provider = pointer(payload, 8);
    const auto selected = word(pointer(provider), 0x18);
    std::uint8_t result;
    switch (selected) {
    case 0x00be5c40:
        result = probe_native_file_store_name_00be5c40(provider, owned_name, context.physical.invalid_parameters); break;
    case 0x00bb8640: result = probe_native_mpkg_name_00bb8640(provider, owned_name); break;
    case 0x00bba080: result = probe_native_msar_name_00bba080(provider, owned_name); break;
    case 0x00bb40c0: result = reject_native_mpak_probe_00bb40c0(owned_name); break;
    case 0x00bf39c0:
        result = replace_native_physical_path_00bf39c0(provider, owned_name, context.physical); break;
    default: throw std::invalid_argument("Unimplemented current VFS provider name probe slot");
    }
    put_byte(visitor, 4, result);
}

void reset_native_vfs_name_probe_base_00bd8fe0(void* visitor) noexcept {
    put(visitor, 0, 0x00d68380);
}

void destroy_native_vfs_name_probe_visitor_00bdb5e0(void* visitor,
    ActualNativeStringPoolStorage& strings) {
    try {
        auto* data = pointer(visitor, 0x0c);
        if (data) strings.release(static_cast<char*>(data), word(visitor, 8) + 1u);
    } catch (...) {
        // E0035C state0 -> CC6010 -> BD8FE0. Inherited noexcept release
        // limits throwing getter behavior; no header zeroing is introduced.
        reset_native_vfs_name_probe_base_00bd8fe0(visitor);
        throw;
    }
    put(visitor, 0, 0x00d68380);
}

// The dedicated BDD0A0 body follows below. It preserves the existing date
// traversal's native string/iterator ownership while selecting these visitors.

void visit_native_vfs_lookup_mounts_00bdd0a0(void* manager, const void* name,
    void* visitor, NativeVfsLookupRouteContext& context) {
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
                dispatch_lookup_visit(visitor, payload, callback_name, context);
                // Provider work can change the visitor/table; do not cache +8.
                const bool stop = dispatch_lookup_stop(visitor, context) != 0;
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

std::uint8_t exists_native_vfs_file_00bdd440(void* manager, const void* name,
    NativeVfsLookupRouteContext& context) {
    auto& strings = context.physical.strings;
    std::uint32_t copied_name[2];
    // No ownership state is armed during this initial copy.
    copy_construct_native_string_header_00426060(copied_name, name, strings);
    bool name_armed = true;
    std::uint32_t visitor[2];
    bool visitor_armed = false;
    try {
        normalize_native_resource_path_header_00bee690(copied_name, strings);
        put(visitor, 0, 0x00d68398);
        put_byte(visitor, 4, 0);
        visitor_armed = true;
        visit_native_vfs_lookup_mounts_00bdd0a0(manager, copied_name, visitor, context);
        auto* captured_data = pointer(copied_name, 4);
        const auto result = byte(visitor, 4);
        reset_native_vfs_date_visitor_00bd90b0(visitor);
        visitor_armed = false;
        name_armed = false;
        if (captured_data)
            strings.release(static_cast<char*>(captured_data), word(copied_name) + 1u);
        return result;
    } catch (...) {
        if (visitor_armed) reset_native_vfs_date_visitor_00bd90b0(visitor);
        if (name_armed) destroy_native_string_header_0041dd20(copied_name, strings);
        throw;
    }
}

bool replace_native_vfs_file_name_00bdd600(void* manager, void* name,
    NativeVfsLookupRouteContext& context) {
    auto& strings = context.physical.strings;
    normalize_native_resource_path_header_00bee690(name, strings);
    std::uint32_t visitor[4];
    put(visitor, 0, 0x00d683e8);
    put_byte(visitor, 4, 0);
    put(visitor, 8, 0);
    put(visitor, 0x0c, 0);
    bool visitor_armed = true;
    try {
        visit_native_vfs_lookup_mounts_00bdd0a0(manager, name, visitor, context);
        const bool result = byte(visitor, 4) != 0;
        if (result && name != at(visitor, 8)) {
            resize_native_string_header_0041dd40(name, strings, word(visitor, 8), true);
            if (word(visitor, 8) != 0) {
                const auto copied = word(name);
                auto* output = pointer(name, 4);
                auto* input = pointer(visitor, 0x0c);
                // Existing raw-string layer's zero-byte memcpy host boundary.
                if (copied != 0) std::memcpy(output, input, copied);
            }
        }
        visitor_armed = false;
        destroy_native_vfs_name_probe_visitor_00bdb5e0(visitor, strings);
        return result;
    } catch (...) {
        if (visitor_armed) destroy_native_vfs_name_probe_visitor_00bdb5e0(visitor, strings);
        throw;
    }
}
} // namespace bsp
