#include "bsp/native_spatial_index_publication.hpp"

#include "bsp/native_diagnostic_sink_lifetime.hpp"
#include "bsp/native_singleton_publication.hpp"
#include "bsp/native_singleton_vector_registration_wrappers.hpp"
#include "bsp/singleton_lifetime.hpp"

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>

#include <cstddef>
#include <cstdint>
#include <cstring>

namespace bsp {
namespace {

struct SpatialPublicationGuard {
    std::uint32_t profile_00;
    CRITICAL_SECTION* section_04;
};
static_assert(sizeof(void*) == 4);
static_assert(sizeof(SpatialPublicationGuard) == 8);
static_assert(offsetof(SpatialPublicationGuard, section_04) == 4);
static_assert(sizeof(CRITICAL_SECTION) == 0x18);

volatile std::uint32_t& word(void* storage, std::size_t offset) noexcept {
    return *reinterpret_cast<volatile std::uint32_t*>(
        static_cast<std::byte*>(storage) + offset);
}

} // namespace

__declspec(noinline) void* __fastcall construct_native_spatial_index_0042d450(
    void* actual_storage, void*) noexcept {
    word(actual_storage, 0) = 0x00ce3cecu;
    word(actual_storage, 4) = 0;
    std::memset(static_cast<std::byte*>(actual_storage) + 0x84, 0, 0x15f90);
    word(actual_storage, 0x80) = 0;
    word(actual_storage, 0x16014) = 0;
    return actual_storage;
}

__declspec(noinline) void* get_native_spatial_index_0042e630(
    void* volatile& actual_manager_publication_01090aa0,
    void* volatile& actual_index_publication_00f8a0d8) {
    void* const initial = actual_index_publication_00f8a0d8;
    if (initial) {
        return initial;
    }

    void* const first_manager = get_native_singleton_manager_00415350(
        actual_manager_publication_01090aa0);
    auto* const captured_section = *reinterpret_cast<CRITICAL_SECTION* volatile*>(
        static_cast<std::byte*>(first_manager) + 0x10);
    SpatialPublicationGuard guard{0x00ce37fcu, captured_section};
    if (captured_section) {
        EnterCriticalSection(captured_section);
        auto& depth = word(captured_section, 0x18);
        depth = depth + 1u;
    }

    // D851D8 / D851D0 has exactly one unwind state: C5EB20 tails 411EE0
    // with guard EBP-14. Neither allocation nor publication is rolled back.
    try {
        if (!actual_index_publication_00f8a0d8) {
            void* const allocation = singleton_lifetime_allocate({
                SingletonAllocationKind::object, 0x16018, 0x16018});
            actual_index_publication_00f8a0d8 = allocation
                ? construct_native_spatial_index_0042d450(allocation, nullptr)
                : nullptr;
            void* const current_manager = get_native_singleton_manager_00415350(
                actual_manager_publication_01090aa0);
            void* const current_index = actual_index_publication_00f8a0d8;
            register_native_singleton_object_00bd0c30(
                current_manager, nullptr, current_index);
        }

        if (captured_section) {
            auto& depth = word(captured_section, 0x18);
            depth = depth - 1u;
            LeaveCriticalSection(captured_section);
        }
    } catch (...) {
        destroy_native_singleton_guard_00411ee0(&guard);
        throw;
    }
    return actual_index_publication_00f8a0d8;
}

} // namespace bsp
