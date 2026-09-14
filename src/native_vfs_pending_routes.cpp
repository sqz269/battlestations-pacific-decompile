#include "bsp/native_vfs_pending_routes.hpp"

#include "bsp/native_physical_file_date.hpp"
#include "bsp/native_string_pool_storage.hpp"
#include "bsp/native_vfs_date_route.hpp"
#include "bsp/native_vfs_lookup_routes.hpp"
#include "bsp/singleton_lifetime.hpp"

#include <cstring>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native VFS pending routes require MSVC Win32.
#endif

namespace bsp {
namespace {
using U = std::uint32_t;
const void* at(const void* p, U offset) noexcept {
    return reinterpret_cast<const void*>(reinterpret_cast<std::uintptr_t>(p) + offset);
}
void* at(void* p, U offset) noexcept {
    return reinterpret_cast<void*>(reinterpret_cast<std::uintptr_t>(p) + offset);
}
U word(const void* p, U offset = 0) noexcept {
    return *static_cast<const volatile U*>(at(p, offset));
}
void put(void* p, U offset, U value) noexcept {
    *static_cast<volatile U*>(at(p, offset)) = value;
}
void* pointer(const void* p, U offset = 0) noexcept {
    return reinterpret_cast<void*>(word(p, offset));
}
std::uint8_t byte(const void* p, U offset) noexcept {
    return *static_cast<const volatile std::uint8_t*>(at(p, offset));
}
void put_byte(void* p, U offset, std::uint8_t value) noexcept {
    *static_cast<volatile std::uint8_t*>(at(p, offset)) = value;
}
void copy_member(void* destination, const void* source,
    ActualNativeStringPoolStorage& strings) {
    put(destination, 0, 0);
    put(destination, 4, 0);
    if (destination == source) return;
    resize_native_string_header_0041dd40(destination, strings, word(source), true);
    if (word(source) != 0) {
        // BF7680 includes backward overlap. Reload the headers after resize.
        const auto length = word(destination);
        auto* const output = pointer(destination, 4);
        auto* const input = pointer(source, 4);
        if (length != 0) std::memmove(output, input, length);
    }
}
} // namespace

std::uint8_t decline_native_filestore_pending_00be7cb0(void*, const void*,
    const void*, std::uintptr_t, U) noexcept {
    return 0;
}
void tick_native_filestore_pending_00be7cc0(void*) noexcept {}

void* construct_native_vfs_pending_visitor_00bdc100(void* visitor,
    const void* first, const void* second, std::uintptr_t callback, U flags,
    ActualNativeStringPoolStorage& strings) {
    put(visitor, 0, 0x00d68478);
    bool first_complete = false;
    try {
        copy_member(at(visitor, 4), first, strings);
        first_complete = true; // Native state1 only after first copy returns.
        copy_member(at(visitor, 0x0c), second, strings);
        put(visitor, 0x18, flags);
        put_byte(visitor, 0x14, 0);
        put(visitor, 0x1c, callback);
    } catch (...) {
        // E0053C: state1 -> first string (CC6168), state0 -> base (CC6160).
        // A failed copy has no armed destructor for its incomplete header.
        if (first_complete) destroy_native_string_header_0041dd20(at(visitor, 4), strings);
        reset_native_vfs_name_probe_base_00bd8fe0(visitor);
        throw;
    }
    return visitor;
}

void destroy_native_vfs_pending_visitor_00bdb740(void* visitor,
    ActualNativeStringPoolStorage& strings) {
    destroy_native_string_header_0041dd20(at(visitor, 0x0c), strings);
    destroy_native_string_header_0041dd20(at(visitor, 4), strings);
    put(visitor, 0, 0x00d68380);
}

std::uint8_t read_native_vfs_pending_result_00bdc1d0(const void* visitor) noexcept {
    return byte(visitor, 0x14);
}

void read_native_vfs_pending_provider_00bdc1e0(void* visitor,
    const void* payload, const void*, NativeVfsPendingRouteContext& context) {
    auto* const provider = pointer(payload, 8);
    const auto flags = word(visitor, 0x18);
    const auto entry = word(pointer(provider), 0x0c);
    const auto callback = word(visitor, 0x1c);
    const auto result = context.providers.invoke_submit(entry, provider,
        at(visitor, 4), at(visitor, 0x0c), callback, flags);
    put_byte(visitor, 0x14, result);
}

std::uint8_t submit_native_vfs_pending_00bdda10(void* manager,
    const void* first, const void* second, std::uintptr_t callback, U flags,
    NativeVfsPendingRouteContext& context) {
    // 4254B0 is a single RET: the diagnostic argument has no native effect.
    auto& strings = context.lookup.physical.strings;
    U visitor[8];
    construct_native_vfs_pending_visitor_00bdc100(visitor, first, second,
        callback, flags, strings);
    try {
        visit_native_vfs_lookup_mounts_00bdd0a0(manager, second, visitor, context.lookup);
    } catch (...) {
        // State0's CC63E0 tail invokes the complete visitor destructor.
        destroy_native_vfs_pending_visitor_00bdb740(visitor, strings);
        throw;
    }
    const auto result = byte(visitor, 0x14);
    destroy_native_vfs_pending_visitor_00bdb740(visitor, strings);
    return result;
}

void pump_native_vfs_pending_00bdb0b0(void* manager,
    NativeVfsPendingRouteContext& context) {
    const auto& crt = context.lookup.physical.invalid_parameters;
    auto* const head = pointer(manager, 0x40);
    auto* node = pointer(head);
    auto* const tree = at(manager, 0x3c);
    auto* owner = tree;
    U iterator[2] = {reinterpret_cast<U>(owner), reinterpret_cast<U>(node)};
    for (;;) {
        auto* const current_end = pointer(tree, 4);
        if (!owner || owner != tree) crt.invalid_parameter(crt.context);
        if (node == current_end) return;
        if (!owner) crt.invalid_parameter(crt.context);
        if (node == pointer(owner, 4)) crt.invalid_parameter(crt.context);
        auto* const provider = pointer(node, 0x18);
        const auto entry = word(pointer(provider), 0x28);
        context.providers.invoke_tick(entry, provider);
        advance_native_vfs_mount_iterator_00bd97e0(iterator, crt);
        node = pointer(iterator, 4);
        owner = pointer(iterator);
    }
}
} // namespace bsp
