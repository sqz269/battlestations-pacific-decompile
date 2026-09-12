#include "bsp/native_physical_factory.hpp"

#include "bsp/native_diagnostic_sink_lifetime.hpp"
#include "bsp/native_singleton_publication.hpp"
#include "bsp/native_singleton_vector_registration_wrappers.hpp"
#include "bsp/singleton_lifetime.hpp"

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native physical factories require MSVC Win32.
#endif

namespace bsp {
namespace {
void* at(void* base, std::uint32_t offset) noexcept {
    return reinterpret_cast<void*>(reinterpret_cast<std::uintptr_t>(base) + offset);
}
volatile std::uint32_t& word(void* base, std::uint32_t offset = 0) noexcept {
    return *static_cast<volatile std::uint32_t*>(at(base, offset));
}
} // namespace

void* get_native_physical_factory_00bed990(NativePhysicalFactoryContext& context) {
    void* const initial = context.actual_factory_publication_0109dbe8;
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
    // Native state0 is armed only after Enter and the depth increment.
    try {
        if (!context.actual_factory_publication_0109dbe8) {
            void* const allocation = singleton_lifetime_allocate({
                SingletonAllocationKind::object, 8, 8});
            if (allocation) {
                word(allocation, 4) = 0x00d68cf4;
                word(allocation) = 0x00d68cfc;
                word(allocation, 4) = 0x00d68cf8;
            }
            context.actual_factory_publication_0109dbe8 = allocation;
            void* const current = context.actual_factory_publication_0109dbe8;
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
        // The published allocation is deliberately retained on registration
        // failure. Original EH has only the captured guard cleanup here.
        destroy_native_singleton_guard_00411ee0(guard);
        throw;
    }
    return context.actual_factory_publication_0109dbe8;
}

void* delete_native_physical_factory_00bed950(void* primary, std::uint32_t flags,
    NativePhysicalFactoryContext& context) noexcept {
    void* const secondary = primary ? at(primary, 4) : nullptr;
    context.actual_factory_publication_0109dbe8 = nullptr;
    word(secondary) = 0x00ce3818;
    word(primary) = 0x00cfe9f4;
    if ((flags & 1u) != 0) singleton_lifetime_free(primary);
    return primary;
}

void* delete_native_physical_factory_secondary_00bed910(void* secondary,
    std::uint32_t flags, NativePhysicalFactoryContext& context) noexcept {
    return delete_native_physical_factory_00bed950(at(secondary, 0xfffffffcu),
        flags, context);
}

} // namespace bsp
