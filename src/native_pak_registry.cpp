#include "bsp/native_pak_registry.hpp"

#include "bsp/native_diagnostic_sink_lifetime.hpp"
#include "bsp/native_ref_counted.hpp"
#include "bsp/native_singleton_publication.hpp"
#include "bsp/native_singleton_vector_registration_wrappers.hpp"
#include "bsp/singleton_lifetime.hpp"

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>

namespace bsp {
namespace {
static_assert(sizeof(void*) == 4);
static_assert(sizeof(CRITICAL_SECTION) == 0x18);
void* at(const void* owner, std::uint32_t offset) noexcept {
    return reinterpret_cast<void*>(reinterpret_cast<std::uintptr_t>(owner) + offset);
}
std::uint32_t word(const void* owner, std::uint32_t offset = 0) noexcept {
    return *static_cast<const volatile std::uint32_t*>(at(owner, offset));
}
std::int32_t signed_word(const void* owner, std::uint32_t offset) noexcept {
    return static_cast<std::int32_t>(word(owner, offset));
}
void put(void* owner, std::uint32_t offset, std::uint32_t value) noexcept {
    *static_cast<volatile std::uint32_t*>(at(owner, offset)) = value;
}
}

void* construct_native_pak_registry_00bb4fb0(void* owner) {
    put(owner, 0, 0x00ceb130);
    put(owner, 4, 1);
    put(owner, 8, 0x00d64188);
    put(owner, 0, 0x00d64190);
    put(owner, 8, 0x00d6418c);
    put(owner, 0x0c, 100);
    put(owner, 0x10, 0);
    put(owner, 0x14, 0);
    put(owner, 0x18, 0);
    return owner;
}

void* get_native_pak_registry_00736c30(NativePakRegistryContext& context) {
    void* const initial = context.actual_registry_publication_010904d8;
    if (initial) return initial;
    void* const first_manager = get_native_singleton_manager_00415350(
        context.actual_manager_publication_01090aa0);
    auto* const section = reinterpret_cast<CRITICAL_SECTION*>(word(first_manager, 0x10));
    alignas(4) std::uint32_t guard[2]{0x00ce37fc,
        reinterpret_cast<std::uint32_t>(section)};
    if (section) {
        EnterCriticalSection(section);
        put(section, 0x18, word(section, 0x18) + 1u);
    }
    try {
        if (!context.actual_registry_publication_010904d8) {
            void* const allocation = singleton_lifetime_allocate({
                SingletonAllocationKind::object, 0x1c, 0x1c});
            void* result = nullptr;
            try {
                if (allocation) result = construct_native_pak_registry_00bb4fb0(allocation);
            } catch (...) {
                // DB5570 state1 -> C86088; disarmed before publication.
                singleton_lifetime_free(allocation);
                throw;
            }
            context.actual_registry_publication_010904d8 = result;
            void* const current = context.actual_registry_publication_010904d8;
            void* const secondary = current ? at(current, 8) : nullptr;
            void* const second_manager = get_native_singleton_manager_00415350(
                context.actual_manager_publication_01090aa0);
            register_native_singleton_object_00bd0c30(second_manager, nullptr, secondary);
        }
        if (section) {
            put(section, 0x18, word(section, 0x18) - 1u);
            LeaveCriticalSection(section);
        }
    } catch (...) {
        destroy_native_singleton_guard_00411ee0(guard); // DB5570 state0.
        throw;
    }
    return context.actual_registry_publication_010904d8;
}

void reserve_native_pak_registry_slots_00bb46f0(void* vector, std::int32_t capacity) {
    if (capacity < 1) capacity = 1;
    if (signed_word(vector, 8) >= capacity) return;
    const auto bytes = static_cast<std::uint32_t>(capacity) * 4u;
    void* const allocation = singleton_lifetime_allocate({
        SingletonAllocationKind::pointer_slots, bytes, bytes});
    std::uint32_t index = 0;
    void* destination = allocation;
    while (static_cast<std::int32_t>(index) < signed_word(vector, 4)) {
        if (destination) {
            void* const current_base = reinterpret_cast<void*>(word(vector));
            put(destination, 0, word(current_base, index * 4u));
        }
        ++index;
        destination = at(destination, 4);
    }
    singleton_lifetime_free(reinterpret_cast<void*>(word(vector)));
    put(vector, 0, reinterpret_cast<std::uint32_t>(allocation));
    put(vector, 8, static_cast<std::uint32_t>(capacity));
}

void resize_native_pak_registry_slots_00bb47a0(void* vector, std::int32_t count) {
    if (count > signed_word(vector, 8)) reserve_native_pak_registry_slots_00bb46f0(vector, count);
    std::uint32_t index = word(vector, 4);
    while (static_cast<std::int32_t>(index) < count) {
        void* const slot = at(reinterpret_cast<void*>(word(vector)), index * 4u);
        if (slot) put(slot, 0, 0);
        ++index;
    }
    while (count < signed_word(vector, 4)) put(vector, 4, word(vector, 4) - 1u);
    put(vector, 4, static_cast<std::uint32_t>(count));
}

void destroy_native_pak_registry_lifetime_base_00bb4610(void* secondary,
    NativePakRegistryContext& context) noexcept {
    context.actual_registry_publication_010904d8 = nullptr;
    put(secondary, 0, 0x00ce3818);
}
void destroy_native_pak_registry_reference_base_00bb3f50(void* owner) noexcept {
    put(owner, 0, 0x00d64178);
    destroy_native_ref_counted_base_00bd30f0(owner);
}

void destroy_native_pak_registry_00bb5000(void* owner, NativePakRegistryContext& context) {
    try {
        void* const vector = at(owner, 0x10);
        resize_native_pak_registry_slots_00bb47a0(vector, 0);
        singleton_lifetime_free(reinterpret_cast<void*>(word(vector)));
    } catch (...) {
        // DFDC0C state1 -> CC4328, then state0 -> CC4320. No vector retry.
        destroy_native_pak_registry_lifetime_base_00bb4610(owner ? at(owner, 8) : nullptr, context);
        destroy_native_pak_registry_reference_base_00bb3f50(owner);
        throw;
    }
    destroy_native_pak_registry_lifetime_base_00bb4610(owner ? at(owner, 8) : nullptr, context);
    destroy_native_pak_registry_reference_base_00bb3f50(owner);
}

void* delete_native_pak_registry_00bb5410(void* owner, std::uint32_t flags,
    NativePakRegistryContext& context) {
    destroy_native_pak_registry_00bb5000(owner, context);
    if (flags & 1u) singleton_lifetime_free(owner);
    return owner;
}
void* delete_native_pak_registry_secondary_00bb4ff0(void* secondary, std::uint32_t flags,
    NativePakRegistryContext& context) {
    return delete_native_pak_registry_00bb5410(at(secondary, 0xfffffff8u), flags, context);
}
void* delete_native_pak_registry_base_00bb49c0(void* owner, std::uint32_t flags,
    NativePakRegistryContext& context) noexcept {
    destroy_native_pak_registry_lifetime_base_00bb4610(owner, context);
    if (flags & 1u) singleton_lifetime_free(owner);
    return owner;
}
} // namespace bsp
