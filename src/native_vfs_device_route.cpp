#include "bsp/native_vfs_device_route.hpp"

#include "bsp/native_physical_file_date.hpp"
#include "bsp/native_pooled_resource_path.hpp"
#include "bsp/native_string_pool_storage.hpp"
#include "bsp/native_vfs_lookup_routes.hpp"
#include "bsp/singleton_lifetime.hpp"

#include <cstring>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native VFS device routes require MSVC Win32.
#endif

namespace bsp {
namespace {
using U = std::uint32_t;
using I = std::int32_t;
static_assert(sizeof(void*) == 4, "Native VFS headers contain Win32 pointers.");
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
} // namespace

void read_native_vfs_device_provider_00bdbc70(void* visitor, const void* payload,
    const void* name, NativeVfsDeviceRouteContext& context) {
    auto& strings = context.lookup.physical.strings;
    if (word(payload) == 0) {
        auto* const provider = pointer(payload, 8);
        const auto entry = word(pointer(provider), 0x24);
        const auto result = context.providers.invoke_resolve(entry, provider, name, at(visitor, 8));
        put_byte(visitor, 4, result);
    } else {
        U provider_name[2] = {0, 0};
        U slash[2];
        U prefixed[2];
        U complete[2];
        int state = 0;
        try {
            auto* const provider = pointer(payload, 8);
            const auto entry = word(pointer(provider), 0x24);
            const auto result = context.providers.invoke_resolve(entry, provider, name, provider_name);
            put_byte(visitor, 4, result);
            construct_native_string_cstring_0041e870(slash, "/", strings);
            state = 1;
            auto* const returned_prefix = concatenate_native_string_headers_004261a0(
                payload, prefixed, slash, strings);
            state = 2;
            auto* const returned_complete = concatenate_native_string_headers_004261a0(
                returned_prefix, complete, provider_name, strings);
            state = 3;
            auto* const destination = at(visitor, 8);
            if (destination != returned_complete) {
                resize_native_string_header_0041dd40(destination, strings, word(returned_complete), true);
                if (word(returned_complete) != 0) {
                    const auto count = word(destination);
                    auto* const output = pointer(destination, 4);
                    const auto* input = pointer(returned_complete, 4);
                    // BF7680 has an explicit backward-overlap branch.
                    if (count != 0) std::memmove(output, input, count);
                }
            }
            state = 2;
            destroy_native_string_header_0041dd20(complete, strings);
            state = 1;
            destroy_native_string_header_0041dd20(prefixed, strings);
            state = 0;
            destroy_native_string_header_0041dd20(slash, strings);
            state = -1;
            destroy_native_string_header_0041dd20(provider_name, strings);
        } catch (...) {
            if (state >= 3) destroy_native_string_header_0041dd20(complete, strings);
            if (state >= 2) destroy_native_string_header_0041dd20(prefixed, strings);
            if (state >= 1) destroy_native_string_header_0041dd20(slash, strings);
            if (state >= 0) destroy_native_string_header_0041dd20(provider_name, strings);
            throw;
        }
    }
    // Concatenation/allocation/cleanup can alter the current callback byte,
    // payload provider, and device. BDBDDA/E0/E3 reads them only now.
    if (byte(visitor, 4) != 0) put(visitor, 0x10, word(pointer(payload, 8), 0x10));
}

std::uint8_t read_native_vfs_device_result_00bdb670(const void* visitor) noexcept {
    return byte(visitor, 4);
}

void destroy_native_vfs_device_visitor_00bdb680(void* visitor,
    ActualNativeStringPoolStorage& strings) {
    try {
        auto* const data = pointer(visitor, 0x0c);
        if (data) strings.release(static_cast<char*>(data), word(visitor, 8) + 1u);
    } catch (...) {
        // CC6030 reloads saved this, then tails to the existing BD8FE0 reset.
        // The actual string-storage release is noexcept in this source domain.
        reset_native_vfs_name_probe_base_00bd8fe0(visitor);
        throw;
    }
    put(visitor, 0, 0x00d68380);
}

void* delete_native_vfs_device_visitor_00bdbe00(void* visitor, U flags,
    ActualNativeStringPoolStorage& strings) {
    destroy_native_vfs_device_visitor_00bdb680(visitor, strings);
    if ((flags & 1u) != 0) singleton_lifetime_free(visitor);
    return visitor;
}

I select_native_vfs_device_00bdd850(void* manager, const void* name,
    const void*, NativeVfsDeviceRouteContext& context) {
    auto& strings = context.lookup.physical.strings;
    alignas(4) unsigned char visitor[0x14]; // Original padding5..7 is untouched.
    put(visitor, 0, 0x00d683f4);
    put_byte(visitor, 4, 0);
    put(visitor, 8, 0);
    put(visitor, 0x0c, 0);
    put(visitor, 0x10, 0xffffffffu);
    bool visitor_armed = true; // Native state0 already owns visitor during copy.
    U copied_name[2] = {0, 0};
    bool name_armed = false;
    try {
        if (static_cast<const void*>(copied_name) != name) {
            resize_native_string_header_0041dd40(copied_name, strings, word(name), true);
            if (word(name) != 0) {
                const auto count = word(copied_name);
                const auto* input = pointer(name, 4);
                auto* const output = pointer(copied_name, 4);
                if (count != 0) std::memmove(output, input, count);
            }
        }
        name_armed = true; // State1 only after initial construction completes.
        normalize_native_resource_path_header_00bee690(copied_name, strings);
        visit_native_vfs_lookup_mounts_00bdd0a0(manager, copied_name, visitor, context.lookup);
        const bool found = byte(visitor, 4) != 0;
        auto* const captured_data = pointer(copied_name, 4);
        name_armed = false;
        // The unsuccessful original branch does not read visitor+10.
        const auto device = found ? static_cast<I>(word(visitor, 0x10)) : -1;
        if (captured_data)
            strings.release(static_cast<char*>(captured_data), word(copied_name) + 1u);
        visitor_armed = false;
        destroy_native_vfs_device_visitor_00bdb680(visitor, strings);
        return device;
    } catch (...) {
        if (name_armed) destroy_native_string_header_0041dd20(copied_name, strings);
        if (visitor_armed) destroy_native_vfs_device_visitor_00bdb680(visitor, strings);
        throw;
    }
}

bool resolve_native_provider_logical_name_00bf0fb0(void* provider,
    const void* suffix, void* output, NativeVfsLookupRouteContext& context) {
    const auto selected = word(pointer(provider), 0x10);
    if (invoke_native_vfs_provider_contains(selected, provider, suffix, context) == 0)
        return false;
    if (output != suffix) {
        auto& strings = context.physical.strings;
        resize_native_string_header_0041dd40(output, strings, word(suffix), true);
        if (word(suffix) != 0) {
            const auto count = word(output);
            const auto* input = pointer(suffix, 4);
            auto* const destination = pointer(output, 4);
            if (count != 0) std::memmove(destination, input, count);
        }
    }
    return true;
}
} // namespace bsp
