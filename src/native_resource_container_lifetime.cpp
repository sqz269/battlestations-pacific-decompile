#include "bsp/native_resource_container_lifetime.hpp"
#include "bsp/native_ref_counted.hpp"
#include "bsp/native_resource_cache_erase.hpp"
#include "bsp/native_resource_fallback_item.hpp"
#include "bsp/native_resource_hierarchy_fields.hpp"
#include "bsp/native_resource_hierarchy_pool.hpp"
#include "bsp/native_resource_manager_lifetime.hpp"
#include "bsp/native_resource_pointer_array.hpp"
#include "bsp/native_singleton_vector_leaves.hpp"
#include "bsp/native_string.hpp"
#include "bsp/native_string_pool_owner.hpp"
#include "bsp/native_string_pool_storage.hpp"
#include "bsp/singleton_lifetime.hpp"
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>
#include <cstring>
#include <exception>
#include <stdexcept>

namespace bsp {
namespace {
using U = std::uint32_t;
static_assert(sizeof(void*) == 4);
U bits(const void* p) noexcept { return static_cast<U>(reinterpret_cast<std::uintptr_t>(p)); }
void* ptr(U value) noexcept { return reinterpret_cast<void*>(value); }
void* at(const void* p, U offset = 0) noexcept { return ptr(bits(p) + offset); }
U word(const void* p, U offset = 0) noexcept { return *static_cast<const volatile U*>(at(p, offset)); }
void put(void* p, U offset, U value) noexcept { *static_cast<volatile U*>(at(p, offset)) = value; }
std::int32_t signed_bits(U value) noexcept { std::int32_t result; std::memcpy(&result, &value, 4); return result; }

// The normal destructor inlines resize(0)/free. Preserve that exact machine
// schedule: a negative count grows by a wrapping BYTE offset with an ADD/JS
// loop, and data is captured BEFORE the final count=0 store. The separate EH
// actions still use their actual B87B20/B87B40 entry points below.
void clear_inline_pointer_array(void* header) {
    const auto capacity = signed_bits(word(header, 8));
    if (capacity < 0 && capacity < 16) {
        auto* const replacement = singleton_lifetime_allocate({SingletonAllocationKind::pointer_slots, 64, 64});
        U index = 0;
        auto destination = bits(replacement);
        while (signed_bits(index) < signed_bits(word(header, 4))) {
            if (destination != 0) {
                const auto data = word(header);
                const auto value = word(ptr(data + index * 4u));
                put(ptr(destination), 0, value);
            }
            index += 1u; destination += 4u;
        }
        auto* const data = ptr(word(header));
        singleton_lifetime_free(data);
        put(header, 0, bits(replacement)); put(header, 8, 16);
    }
    const auto count = word(header, 4);
    if (signed_bits(count) < 0) {
        U offset = count * 4u;
        do {
            const auto destination = word(header) + offset;
            if (destination != 0) put(ptr(destination), 0, 0);
            offset += 4u;
        } while (signed_bits(offset) < 0);
    }
    while (signed_bits(word(header, 4)) > 0) put(header, 4, word(header, 4) - 1u);
    auto* const data = ptr(word(header));
    put(header, 4, 0);
    singleton_lifetime_free(data);
}
void unwind(void* resource, int state, NativeResourceContainerLifetimeContext& context) noexcept {
    try {
        if (state >= 3) destroy_native_hierarchy_item_pointers_00b87b40(at(resource, 0x1c));
        if (state >= 2) destroy_native_resource_item_pointers_00b87b20(at(resource, 0x10));
        if (state >= 1) destroy_native_string_header_0041dd20(at(resource, 8), context.manager.strings);
        if (state >= 0) destroy_native_ref_counted_base_00bd30f0(resource);
    } catch (...) { std::terminate(); }
}
}

void destroy_native_resource_container_00b88430(void* resource, NativeResourceContainerLifetimeContext& context) {
    put(resource, 0, 0x00d63228);
    int state = 3;
    try {
        U index = 0;
        while (signed_bits(index) < signed_bits(word(resource, 0x14))) {
            auto* const data = ptr(word(resource, 0x10));
            auto* const item = ptr(word(data, index * 4u));
            // A native null entry is not accepted: there is no guard before +4.
            if (InterlockedDecrement(static_cast<volatile LONG*>(at(item, 4))) == 0) {
                const auto table = word(item);
                const auto entry = word(ptr(table));
                context.item_references.source_zero_reference(entry, item, table);
            }
            index += 1u;
        }
        auto* const name = at(resource, 8);
        auto* const manager = get_native_resource_manager_004c1400(context.manager);
        erase_native_resource_manager_name_00b801c0(manager, name,
            context.manager.strings, context.manager.invalid_parameters);
        index = 0;
        while (signed_bits(index) < signed_bits(word(resource, 0x20))) {
            auto* const data = ptr(word(resource, 0x1c));
            auto* const record = ptr(word(data, index * 4u));
            if (record) {
                destroy_native_hierarchy_fields_00b88180(record, context.manager.strings);
                return_native_hierarchy_pool_slot_00b17af0(context.hierarchy_pool_0109022c, record);
            }
            index += 1u;
        }
        state = 2; clear_inline_pointer_array(at(resource, 0x1c));
        state = 1; clear_inline_pointer_array(at(resource, 0x10));
        auto* const name_data = ptr(word(resource, 0x0c)); state = 0;
        if (name_data) {
            const auto bytes = word(resource, 8) + 1u;
            auto& strings = context.manager.strings;
            auto* const pool = native_string_pool_get_or_create_00419cc0(
                strings.actual_published_01090aa8, strings.actual_manager_publication_01090aa0);
            return_native_string_pool_00bd1510(pool, name_data, bytes, strings.actual_small_returns_disabled_01090aa4);
        }
        state = -1;
        destroy_native_ref_counted_base_00bd30f0(resource);
    } catch (...) { unwind(resource, state, context); throw; }
}
void destroy_native_game_resource_container_00718810(void* resource, NativeResourceContainerLifetimeContext& context) {
    put(resource, 0, 0x00cfd8cc);
    clear_native_singleton_storage_00bd0220(at(resource, 0x64), nullptr);
    clear_native_singleton_storage_00bd0220(at(resource, 0x54), nullptr);
    clear_native_singleton_storage_00bd0220(at(resource, 0x44), nullptr);
    destroy_native_resource_container_00b88430(resource, context);
}
void* delete_native_resource_container_00b88760(void* resource, U flags, NativeResourceContainerLifetimeContext& context) {
    destroy_native_resource_container_00b88430(resource, context);
    if ((flags & 1u) != 0) singleton_lifetime_free(resource);
    return resource;
}
void* delete_native_game_resource_container_00718c20(void* resource, U flags, NativeResourceContainerLifetimeContext& context) {
    destroy_native_game_resource_container_00718810(resource, context);
    if ((flags & 1u) != 0) singleton_lifetime_free(resource);
    return resource;
}
NativeResourceContainerReferences::NativeResourceContainerReferences(NativeAdoptedSubstreamDispatch& other,
    NativeResourceManagerContext& manager, NativeMaterialParameterPool& pool)
    : other_(other), context_{manager, pool, *this} {}
std::uint8_t NativeResourceContainerReferences::source_is_open(std::uintptr_t e, void* owner) { return other_.source_is_open(e, owner); }
U NativeResourceContainerReferences::source_seek(std::uintptr_t e, void* owner, U lo, U hi, U origin) { return other_.source_seek(e, owner, lo, hi, origin); }
void NativeResourceContainerReferences::source_read(std::uintptr_t e, void* owner, void* data, U count, U* actual) { other_.source_read(e, owner, data, count, actual); }
void NativeResourceContainerReferences::source_write(std::uintptr_t e, void* owner, const void* data, U count, U* actual) { other_.source_write(e, owner, data, count, actual); }
void NativeResourceContainerReferences::source_zero_reference(std::uintptr_t e, void* owner, std::uintptr_t table) {
    if (e == 0x00bd30e0 && (table == 0x00d63228 || table == 0x00cfd8cc || table == 0x00d631c0)) {
        if (!owner) return;
        const auto target = word(ptr(word(owner)), 4);
        if (target == 0x00b88760) delete_native_resource_container_00b88760(owner, 1, context_);
        else if (target == 0x00718c20) delete_native_game_resource_container_00718c20(owner, 1, context_);
        else if (target == 0x00b86990) scalar_delete_native_resource_fallback_item_00b86990(owner, 1);
        else throw std::runtime_error("Reached resource-container deleting target is not reconstructed");
        return;
    }
    other_.source_zero_reference(e, owner, table);
}
} // namespace bsp
