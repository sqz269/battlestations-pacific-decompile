#include "bsp/native_filestore_factory.hpp"

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

void* __fastcall construct_native_filestore_factory_00be5320(
    void* primary, void*) noexcept {
    word(primary, 4) = 0x00d688ac;
    word(primary) = 0x00d688b4;
    word(primary, 4) = 0x00d688b0;
    word(primary, 8) = 0;
    return primary;
}

void* get_native_filestore_factory_004fc150(NativeFileStoreFactoryContext& context) {
    void* const initial = context.actual_factory_publication_0109db68;
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
    // Native state0 is armed after the successful enter and +18 increment.
    try {
        if (!context.actual_factory_publication_0109db68) {
            void* const allocation = singleton_lifetime_allocate({
                SingletonAllocationKind::object, 0x0c, 0x0c});
            // Native state1 owns the saved allocation only during construction.
            // The leaf constructor has no C++ throwing call. Its hardware-fault
            // unwind through C686A8 is outside the source /EHsc contract.
            void* const result = allocation
                ? construct_native_filestore_factory_00be5320(allocation, nullptr)
                : nullptr;
            context.actual_factory_publication_0109db68 = result;
            void* const current = context.actual_factory_publication_0109db68;
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
    return context.actual_factory_publication_0109db68;
}

void destroy_native_filestore_factory_00be5350(
    void* primary, NativeFileStoreFactoryContext& context) noexcept {
    void* const secondary = primary ? at(primary, 4) : nullptr;
    context.actual_factory_publication_0109db68 = nullptr;
    word(secondary) = 0x00ce3818;
    word(primary) = 0x00cfe9f4;
}

void* delete_native_filestore_factory_00be5790(void* primary, std::uint32_t flags,
    NativeFileStoreFactoryContext& context) noexcept {
    destroy_native_filestore_factory_00be5350(primary, context);
    if ((flags & 1u) != 0) singleton_lifetime_free(primary);
    return primary;
}

void* delete_native_filestore_factory_secondary_00be5340(void* secondary,
    std::uint32_t flags, NativeFileStoreFactoryContext& context) noexcept {
    return delete_native_filestore_factory_00be5790(at(secondary, 0xfffffffcu),
        flags, context);
}

void* delete_native_filestore_factory_base_00be5380(void* base, std::uint32_t flags,
    NativeFileStoreFactoryContext& context) noexcept {
    context.actual_factory_publication_0109db68 = nullptr;
    word(base) = 0x00ce3818;
    if ((flags & 1u) != 0) singleton_lifetime_free(base);
    return base;
}

} // namespace bsp
