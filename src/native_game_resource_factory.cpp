#include "bsp/native_game_resource_factory.hpp"

#include "bsp/native_diagnostic_sink_lifetime.hpp"
#include "bsp/native_singleton_publication.hpp"
#include "bsp/native_singleton_vector_registration_wrappers.hpp"
#include "bsp/singleton_lifetime.hpp"

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>

namespace bsp {
namespace {
void* at(void* base, std::uint32_t offset) noexcept {
    return reinterpret_cast<void*>(reinterpret_cast<std::uintptr_t>(base) + offset);
}
volatile std::uint32_t& word(void* base, std::uint32_t offset = 0) noexcept {
    return *static_cast<volatile std::uint32_t*>(at(base, offset));
}
static_assert(sizeof(void*) == 4);
static_assert(sizeof(CRITICAL_SECTION) == 0x18);
} // namespace

void* get_native_game_resource_factory_007175d0(NativeGameResourceFactoryContext& context) {
    void* const initial = context.actual_factory_publication_00e19b90;
    if (initial) return initial;

    void* const first_manager = get_native_singleton_manager_00415350(
        context.actual_manager_publication_01090aa0);
    auto* const section = reinterpret_cast<CRITICAL_SECTION*>(word(first_manager, 0x10));
    alignas(4) std::uint32_t guard[2]{0x00ce37fc,
        reinterpret_cast<std::uint32_t>(section)};
    if (section) {
        EnterCriticalSection(section);
        word(section, 0x18) = word(section, 0x18) + 1u;
    }
    // State0 is armed after Enter and the physical section+18 increment.
    try {
        if (!context.actual_factory_publication_00e19b90) {
            void* const allocation = singleton_lifetime_allocate({
                SingletonAllocationKind::object, 8, 8});
            if (allocation) {
                // 717638..71764C: construction is inlined, with no extra EH state.
                word(allocation, 4) = 0x00cfd7f8;
                word(allocation) = 0x00cfd850;
                word(allocation, 4) = 0x00cfd84c;
            }
            context.actual_factory_publication_00e19b90 = allocation;
            void* const current = context.actual_factory_publication_00e19b90;
            void* const captured_secondary = current ? at(current, 4) : nullptr;
            void* const second_manager = get_native_singleton_manager_00415350(
                context.actual_manager_publication_01090aa0);
            register_native_singleton_object_00bd0c30(
                second_manager, nullptr, captured_secondary);
        }
        if (section) {
            word(section, 0x18) = word(section, 0x18) - 1u;
            LeaveCriticalSection(section);
        }
    } catch (...) {
        destroy_native_singleton_guard_00411ee0(guard);
        throw;
    }
    return context.actual_factory_publication_00e19b90;
}

void destroy_native_game_resource_factory_00716530(
    void* primary, NativeGameResourceFactoryContext& context) noexcept {
    void* const secondary = primary ? at(primary, 4) : nullptr;
    context.actual_factory_publication_00e19b90 = nullptr;
    word(secondary) = 0x00ce3818;
    word(primary) = 0x00cfd7dc;
}

void* delete_native_game_resource_factory_00716560(void* primary,
    std::uint32_t flags, NativeGameResourceFactoryContext& context) noexcept {
    destroy_native_game_resource_factory_00716530(primary, context);
    if ((flags & 1u) != 0) singleton_lifetime_free(primary);
    return primary;
}

void* delete_native_game_resource_factory_secondary_00716520(void* secondary,
    std::uint32_t flags, NativeGameResourceFactoryContext& context) noexcept {
    return delete_native_game_resource_factory_00716560(at(secondary, 0xfffffffcu),
        flags, context);
}

void* delete_native_game_resource_factory_base_007150b0(void* base,
    std::uint32_t flags, NativeGameResourceFactoryContext& context) noexcept {
    context.actual_factory_publication_00e19b90 = nullptr;
    word(base) = 0x00ce3818;
    if ((flags & 1u) != 0) singleton_lifetime_free(base);
    return base;
}
} // namespace bsp
