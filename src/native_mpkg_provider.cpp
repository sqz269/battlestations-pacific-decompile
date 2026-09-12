#include "bsp/native_mpkg_provider.hpp"

#include "bsp/native_diagnostic_sink_lifetime.hpp"
#include "bsp/native_file_provider_base.hpp"
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

void* get_native_mpkg_factory_00736a90(NativeMpkgFactoryContext& context) {
    void* const initial = context.actual_factory_publication_010904f4;
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
        if (!context.actual_factory_publication_010904f4) {
            void* const allocation = singleton_lifetime_allocate({
                SingletonAllocationKind::object, 8, 8});
            if (allocation) {
                put(allocation, 4, 0x00cfe9fc);
                put(allocation, 0, 0x00cfea14);
                put(allocation, 4, 0x00cfea10);
            }
            context.actual_factory_publication_010904f4 = allocation;
            void* const current = context.actual_factory_publication_010904f4;
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
    return context.actual_factory_publication_010904f4;
}

void* delete_native_mpkg_factory_00737090(void* owner, std::uint32_t flags,
    NativeMpkgFactoryContext& context) noexcept {
    void* const secondary = owner ? at(owner, 4) : nullptr;
    context.actual_factory_publication_010904f4 = nullptr;
    put(secondary, 0, 0x00ce3818);
    put(owner, 0, 0x00cfe9f4);
    if (flags & 1u) singleton_lifetime_free(owner);
    return owner;
}
void* delete_native_mpkg_factory_secondary_00735d00(void* secondary,
    std::uint32_t flags, NativeMpkgFactoryContext& context) noexcept {
    return delete_native_mpkg_factory_00737090(at(secondary, 0xfffffffcu), flags, context);
}
void* delete_native_mpkg_factory_base_00735800(void* owner, std::uint32_t flags,
    NativeMpkgFactoryContext& context) noexcept {
    context.actual_factory_publication_010904f4 = nullptr;
    put(owner, 0, 0x00ce3818);
    if (flags & 1u) singleton_lifetime_free(owner);
    return owner;
}

void* construct_native_mpkg_provider_00bb9cb0(void* owner,
    const void* system_name, NativeMpkgProviderContext& context) {
    construct_native_file_provider_base_00bb5590(owner, system_name, context.strings);
    try {
        put(owner, 0, 0x00d64390);
        void* const allocation = singleton_lifetime_allocate({
            SingletonAllocationKind::object, 0x34, 0x34});
        void* result = nullptr;
        try {
            if (allocation) result = context.archives.construct_00bb9920(allocation, system_name);
        } catch (...) {
            singleton_lifetime_free(allocation); // DFE534 state1 -> CC48D8.
            throw;
        }
        put(owner, 0x14, reinterpret_cast<std::uint32_t>(result));
    } catch (...) {
        destroy_native_file_provider_base_00bb5380(owner, context.strings); // state0 -> CC48D0.
        throw;
    }
    return owner;
}

void* create_native_mpkg_provider_00bb9d90(void*, const void* system_name,
    const void*, NativeMpkgProviderContext& context) {
    const auto length = word(system_name);
    bool accepted = false;
    if (length > 5u) {
        alignas(4) std::uint32_t suffix[2];
        void* const result = construct_native_string_substring_00469840(system_name,
            suffix, length - 5u, 0x7fffffffu, context.strings);
        accepted = equal_native_string_header_00425850(result, ".mpkg");
        // BB9D90 does not arm an EH suffix owner. The tested guard and these
        // current header reads are its normal cleanup, before provider allocation.
        auto* const data = reinterpret_cast<char*>(word(suffix, 4));
        if (data) context.strings.release(data, word(suffix) + 1u);
    }
    if (!accepted) return nullptr;
    void* const allocation = singleton_lifetime_allocate({
        SingletonAllocationKind::object, 0x18, 0x18});
    try {
        return allocation ? construct_native_mpkg_provider_00bb9cb0(allocation,
            system_name, context) : nullptr;
    } catch (...) {
        singleton_lifetime_free(allocation); // DFE568 state0 -> CC48F0.
        throw;
    }
}

void destroy_native_mpkg_provider_00bb9e70(void* owner, NativeMpkgProviderContext& context) {
    put(owner, 0, 0x00d64390);
    void* const archive = reinterpret_cast<void*>(word(owner, 0x14));
    try {
        if (archive) {
            context.archives.destroy_00bb9c10(archive);
            singleton_lifetime_free(archive);
        }
    } catch (...) {
        destroy_native_file_provider_base_00bb5380(owner, context.strings); // DFE594 state0.
        throw;
    }
    destroy_native_file_provider_base_00bb5380(owner, context.strings);
}
void* delete_native_mpkg_provider_00bb9ee0(void* owner, std::uint32_t flags,
    NativeMpkgProviderContext& context) {
    destroy_native_mpkg_provider_00bb9e70(owner, context);
    if (flags & 1u) singleton_lifetime_free(owner);
    return owner;
}
} // namespace bsp
