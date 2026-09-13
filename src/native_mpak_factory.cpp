#include "bsp/native_mpak_factory.hpp"

#include "bsp/native_diagnostic_sink_lifetime.hpp"
#include "bsp/native_physical_file_date.hpp"
#include "bsp/native_pooled_string_substring.hpp"
#include "bsp/native_singleton_publication.hpp"
#include "bsp/native_singleton_vector_registration_wrappers.hpp"
#include "bsp/native_string_compare.hpp"
#include "bsp/native_string_pool_storage.hpp"
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
void put(void* owner, std::uint32_t offset, std::uint32_t value) noexcept {
    *static_cast<volatile std::uint32_t*>(at(owner, offset)) = value;
}
}

void* get_native_mpak_factory_00736b60(NativeMpakFactoryContext& context) {
    void* const initial = context.actual_factory_publication_010904d4;
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
        if (!context.actual_factory_publication_010904d4) {
            void* const allocation = singleton_lifetime_allocate({
                SingletonAllocationKind::object, 8, 8});
            if (allocation) {
                put(allocation, 4, 0x00cfea00);
                put(allocation, 0, 0x00cfea20);
                put(allocation, 4, 0x00cfea1c);
            }
            context.actual_factory_publication_010904d4 = allocation;
            void* const current = context.actual_factory_publication_010904d4;
            void* const secondary = current ? at(current, 4) : nullptr;
            void* const second_manager = get_native_singleton_manager_00415350(
                context.actual_manager_publication_01090aa0);
            register_native_singleton_object_00bd0c30(second_manager, nullptr, secondary);
        }
        if (section) {
            put(section, 0x18, word(section, 0x18) - 1u);
            LeaveCriticalSection(section);
        }
    } catch (...) {
        destroy_native_singleton_guard_00411ee0(guard);
        throw;
    }
    return context.actual_factory_publication_010904d4;
}

void* delete_native_mpak_factory_007370d0(void* owner, std::uint32_t flags,
    NativeMpakFactoryContext& context) noexcept {
    void* const secondary = owner ? at(owner, 4) : nullptr;
    context.actual_factory_publication_010904d4 = nullptr;
    put(secondary, 0, 0x00ce3818);
    put(owner, 0, 0x00cfe9f4);
    if (flags & 1u) singleton_lifetime_free(owner);
    return owner;
}
void* delete_native_mpak_factory_secondary_00735d30(void* secondary,
    std::uint32_t flags, NativeMpakFactoryContext& context) noexcept {
    return delete_native_mpak_factory_007370d0(at(secondary, 0xfffffffcu), flags, context);
}
void* delete_native_mpak_factory_base_00735840(void* owner, std::uint32_t flags,
    NativeMpakFactoryContext& context) noexcept {
    context.actual_factory_publication_010904d4 = nullptr;
    put(owner, 0, 0x00ce3818);
    if (flags & 1u) singleton_lifetime_free(owner);
    return owner;
}

void* create_native_mpak_provider_00bb83a0(void*, const void* system_name,
    const void*, NativeMpakCreateContext& context) {
    const auto length = word(system_name);
    bool accepted = false;
    if (length > 4u) {
        alignas(4) std::uint32_t suffix[2];
        void* const result = construct_native_string_substring_00469840(system_name,
            suffix, length - 5u, 0x7fffffffu, context.strings);
        accepted = equal_native_string_header_00425850(result, ".mpak");
        // This suffix has no armed native EH owner. Reload its current header
        // only after comparison, and complete normal cleanup before lock access.
        auto* const data = reinterpret_cast<char*>(word(suffix, 4));
        if (data) context.strings.release(data, word(suffix) + 1u);
    }
    if (!accepted) return nullptr;

    auto* const entered = static_cast<CRITICAL_SECTION*>(
        context.actual_lock_publication_010904e0);
    EnterCriticalSection(entered);
    put(entered, 0x18, word(entered, 0x18) + 1u);
    auto* const leaving = static_cast<CRITICAL_SECTION*>(
        context.actual_lock_publication_010904e0);
    void* const cached = context.actual_cached_provider_010904dc;
    put(leaving, 0x18, word(leaving, 0x18) - 1u);
    LeaveCriticalSection(leaving);

    if (cached) {
        alignas(4) std::uint32_t empty[2];
        construct_native_string_cstring_0041e870(empty, "", context.strings);
        try {
            context.providers.replace_cached_00bb82f0(empty);
        } catch (...) {
            // DFE33C state0 -> CC4760 ->41DD20 reads the CURRENT header.
            auto* const data = reinterpret_cast<char*>(word(empty, 4));
            if (data) context.strings.release(data, word(empty) + 1u);
            throw;
        }
        auto* const data = reinterpret_cast<char*>(word(empty, 4));
        if (data) context.strings.release(data, word(empty) + 1u);
        return cached;
    }

    void* const allocation = singleton_lifetime_allocate({
        SingletonAllocationKind::object, 0x44, 0x44});
    try {
        return allocation ? context.providers.construct_00bb8240(allocation,
            system_name) : nullptr;
    } catch (...) {
        singleton_lifetime_free(allocation); // DFE33C state1 -> CC4768.
        throw;
    }
}
} // namespace bsp
