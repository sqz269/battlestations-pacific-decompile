#include "bsp/native_vfs_open_route.hpp"
#include "bsp/native_vfs_runtime_bindings.hpp"

#include "bsp/native_physical_file_date.hpp"
#include "bsp/native_pooled_resource_path.hpp"
#include "bsp/native_pooled_string_substring.hpp"
#include "bsp/native_string_pool_storage.hpp"
#include "bsp/native_vfs_date_route.hpp"
#include "bsp/singleton_lifetime.hpp"

#include <stdexcept>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native VFS open route requires MSVC Win32.
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
std::uint32_t open_visitor_slot(const void* visitor, std::uint32_t offset,
    const NativeVfsOpenRouteContext& context) {
    if (word(visitor) != 0x00d6838c)
        throw std::invalid_argument("Unimplemented native VFS open visitor identity");
    return word(context.actual_open_profile_00d6838c, offset);
}
void dispatch_open_visit(void* visitor, const void* payload, const void* name,
    NativeVfsOpenRouteContext& context) {
    if (open_visitor_slot(visitor, 4, context) != 0x00bda690)
        throw std::invalid_argument("Unimplemented current VFS open visitor call slot");
    open_native_vfs_mount_00bda690(visitor, payload, name, context);
}
std::uint32_t dispatch_open_stop(const void* visitor, NativeVfsOpenRouteContext& context) {
    if (open_visitor_slot(visitor, 8, context) != 0x00bd9040)
        throw std::invalid_argument("Unimplemented current VFS open visitor stop slot");
    return has_native_vfs_open_stream_00bd9040(visitor);
}
// The actual 004254B0 body is C3. Evaluate the native diagnostic name read
// without substituting a host logger or introducing observable formatting.
void evaluate_diagnostic_name(const void* name) noexcept {
    (void)word(name, 4);
}
using Open = void* (__fastcall*)(void*, void*, const void*, std::uint32_t);
using Failure = void (__fastcall*)(void*, void*);
using Size = std::uint32_t (__fastcall*)(void*, void*, std::uint32_t);
} // namespace

void apply_native_vfs_first_alias_00bdca80(void* manager, void* name,
    ActualNativeStringPoolStorage& strings) {
    if (static_cast<std::int32_t>(word(manager, 0x98)) <= 0) return;
    std::uint32_t index = 0;
    std::uint32_t offset = 0;
    do {
        const auto* entry = at(pointer(manager, 0x94), offset);
        if (equal_native_string_headers_00435c40(entry, name)) {
            // CRT comparison can be reentrant; reload the current vector base.
            const auto* replacement = at(pointer(manager, 0x94), index * 16u + 8u);
            assign_native_string_header_00425f40(name, replacement, strings);
            return;
        }
        ++index;
        offset += 16u;
    } while (static_cast<std::int32_t>(index) < static_cast<std::int32_t>(word(manager, 0x98)));
}

void open_native_vfs_mount_00bda690(void* visitor, const void* payload,
    const void* name, NativeVfsOpenRouteContext& context) {
    auto* current_manager = context.physical.manager_0109ceec;
    if (byte(current_manager, 0x20) && byte(payload, 0x0c) && !(byte(visitor, 8) & 1u))
        evaluate_diagnostic_name(name);
    auto* provider = pointer(payload, 8);
    const auto flags = word(visitor, 8);
    void* result;
    if(context.native_bindings)result=context.native_bindings->provider_open(provider,name,flags);
    else {
        auto* selected = pointer(pointer(provider), 8);
        result = reinterpret_cast<Open>(selected)(provider, selected, name, flags);
    }
    put(visitor, 4, reinterpret_cast<std::uint32_t>(result));
    if (result) put_byte(visitor, 0x0c, byte(payload, 0x0c));
}

std::uint32_t has_native_vfs_open_stream_00bd9040(const void* visitor) noexcept {
    return word(visitor, 4) != 0 ? 1u : 0u;
}

void visit_native_vfs_open_mounts_00bdd0a0(void* manager, const void* name,
    void* visitor, NativeVfsOpenRouteContext& context) {
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
                dispatch_open_visit(visitor, payload, callback_name, context);
                // Provider work can change the visitor/table; do not cache +8.
                const bool stop = dispatch_open_stop(visitor, context) != 0;
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


void* open_native_vfs_resource_00bdf310(void* manager, const void* name,
    std::uint32_t flags, NativeVfsOpenRouteContext& context) {
    auto& strings = context.physical.strings;
    std::uint32_t copied_name[2] = {0, 0};
    assign_native_string_header_00425f40(copied_name, name, strings);
    bool name_armed = true;
    std::uint32_t visitor[4]; // +0C is initialized only by a successful visit.
    bool visitor_armed = false;
    try {
        normalize_native_resource_path_header_00bee690(copied_name, strings);
        apply_native_vfs_first_alias_00bdca80(manager, copied_name, strings);
        put(visitor, 0, 0x00d6838c);
        put(visitor, 4, 0);
        put(visitor, 8, flags);
        visitor_armed = true;
        visit_native_vfs_open_mounts_00bdd0a0(manager, copied_name, visitor, context);
        auto* const result = pointer(visitor, 4);
        if (!result && !(flags & 1u)) {
            evaluate_diagnostic_name(name); // Original caller name, not alias.
            auto* current_manager = context.physical.manager_0109ceec;
            auto* selected = pointer(current_manager, 0x90); // field, not vtable
            reinterpret_cast<Failure>(selected)(current_manager, selected);
        } else if (result) {
            if (flags & 1u) {
                put(manager, 0x24, word(manager, 0x24) + 1u);
            } else {
                if (!context.native_bindings && !context.log_opened_resource_00bde9c0)
                    throw std::invalid_argument("Native VFS open requires actual BDE9C0 log service");
                const auto mount_byte = byte(visitor, 0x0c);
                auto* current_manager = context.physical.manager_0109ceec;
                if(context.native_bindings)context.native_bindings->log_open(current_manager,copied_name,result,mount_byte);
                else context.log_opened_resource_00bde9c0(current_manager, copied_name,
                    copied_name, result, mount_byte);
                put(manager, 0x28, word(manager, 0x28) + 1u);
                std::uint32_t size;
                if(context.native_bindings)size=context.native_bindings->stream_size_low(result,0);
                else {auto* selected = pointer(pointer(result), 0x2c);size=reinterpret_cast<Size>(selected)(result, selected, 0);}
                put(manager, 0x2c, word(manager, 0x2c) + size);
            }
        }
        // The native path captures data before resetting the visitor identity.
        auto* data = pointer(copied_name, 4);
        put(visitor, 0, 0x00d68380);
        visitor_armed = false;
        name_armed = false;
        if (data) strings.release(static_cast<char*>(data), word(copied_name) + 1u);
        return result;
    } catch (...) {
        if (visitor_armed) put(visitor, 0, 0x00d68380);
        if (name_armed) destroy_native_string_header_0041dd20(copied_name, strings);
        throw;
    }
}
} // namespace bsp
