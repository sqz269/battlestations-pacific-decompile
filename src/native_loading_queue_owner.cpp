#include "bsp/native_loading_queue_owner.hpp"

#include "bsp/native_diagnostic_sink_lifetime.hpp"
#include "bsp/native_event_owner.hpp"
#include "bsp/native_singleton_publication.hpp"
#include "bsp/native_singleton_vector_registration_wrappers.hpp"
#include "bsp/singleton_lifetime.hpp"

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>

#include <cstddef>

namespace bsp {
namespace {
struct NativeGuard {
    std::uint32_t table_00;
    CRITICAL_SECTION* section_04;
};
static_assert(sizeof(void*) == 4);
static_assert(sizeof(NativeLoadingQueueOwnerStorage) == 0x20);
static_assert(offsetof(NativeLoadingQueueOwnerStorage, table_00) == 0);
static_assert(offsetof(NativeLoadingQueueOwnerStorage, worker_stop_04) == 4);
static_assert(offsetof(NativeLoadingQueueOwnerStorage, worker_job_08) == 8);
static_assert(offsetof(NativeLoadingQueueOwnerStorage, event_0c) == 0x0c);
static_assert(offsetof(NativeLoadingQueueOwnerStorage, jobs_10) == 0x10);
static_assert(offsetof(NativeLoadingQueueOwnerStorage, count_14) == 0x14);
static_assert(offsetof(NativeLoadingQueueOwnerStorage, capacity_18) == 0x18);
static_assert(offsetof(NativeLoadingQueueOwnerStorage, worker_thread_1c) == 0x1c);
static_assert(sizeof(NativeGuard) == 8 && offsetof(NativeGuard, section_04) == 4);
static_assert(sizeof(CRITICAL_SECTION) == 0x18);

volatile std::uint32_t& tracked_depth(CRITICAL_SECTION* section) noexcept {
    return *reinterpret_cast<volatile std::uint32_t*>(
        reinterpret_cast<std::byte*>(section) + 0x18);
}
} // namespace

NativeLoadingQueueOwnerStorage* construct_native_loading_queue_owner_004fdba0(
    NativeLoadingQueueOwnerStorage* actual_owner,
    NativeLoadingQueueOwnerStorage* volatile& actual_publication_00e18d4c) {
    actual_owner->table_00 = native_loading_queue_table_00ceb198;
    // Native state0 precedes the following stores and the event factory call.
    try {
        actual_owner->worker_stop_04 = 0;
        actual_owner->worker_job_08 = nullptr;
        actual_owner->event_0c = create_native_event_owner_00bd1970(1);
        actual_owner->jobs_10 = nullptr;
        actual_owner->count_14 = 0;
        actual_owner->capacity_18 = 0;
        actual_owner->worker_thread_1c = nullptr;
    } catch (...) {
        // C68800 tail-calls4F93A0; it neither frees nor deletes the event.
        actual_publication_00e18d4c = nullptr;
        actual_owner->table_00 = 0x00ce3818;
        throw;
    }
    return actual_owner;
}

__declspec(noinline) NativeLoadingQueueOwnerStorage* get_native_loading_queue_owner_004fde20(
    void* volatile& actual_manager_publication_01090aa0,
    NativeLoadingQueueOwnerStorage* volatile& actual_publication_00e18d4c) {
    NativeLoadingQueueOwnerStorage* const initial = actual_publication_00e18d4c;
    if (initial) return initial;

    void* const first_manager = get_native_singleton_manager_00415350(
        actual_manager_publication_01090aa0);
    auto* const captured_section = *reinterpret_cast<CRITICAL_SECTION* volatile*>(
        static_cast<std::byte*>(first_manager) + 0x10);
    NativeGuard guard{0x00ce37fcu, captured_section};
    if (captured_section) {
        EnterCriticalSection(captured_section);
        auto& depth = tracked_depth(captured_section);
        depth = depth + 1u;
    }

    // C68833/D919B4 state0 starts only after acquisition and depth increment.
    try {
        if (!actual_publication_00e18d4c) {
            auto* const allocation = static_cast<NativeLoadingQueueOwnerStorage*>(
                singleton_lifetime_allocate({SingletonAllocationKind::object,
                    0x20, sizeof(NativeLoadingQueueOwnerStorage)}));
            NativeLoadingQueueOwnerStorage* result;
            // Native state1 owns the saved allocation only while constructing.
            try {
                result = allocation ? construct_native_loading_queue_owner_004fdba0(
                    allocation, actual_publication_00e18d4c) : nullptr;
            } catch (...) {
                singleton_lifetime_free(allocation); // C68828, returning free.
                throw;
            }
            actual_publication_00e18d4c = result;
            void* const current_manager = get_native_singleton_manager_00415350(
                actual_manager_publication_01090aa0);
            NativeLoadingQueueOwnerStorage* const current_owner = actual_publication_00e18d4c;
            register_native_singleton_object_00bd0c30(current_manager, nullptr, current_owner);
        }
        if (captured_section) {
            auto& depth = tracked_depth(captured_section);
            depth = depth - 1u;
            LeaveCriticalSection(captured_section);
        }
    } catch (...) {
        destroy_native_singleton_guard_00411ee0(&guard); // C68820, captured guard.
        throw;
    }
    return actual_publication_00e18d4c;
}

void native_loading_queue_on_filestore_ready_00504850(
    void* volatile& actual_manager_publication_01090aa0,
    NativeLoadingQueueOwnerStorage* volatile& actual_publication_00e18d4c,
    const void*, const void*) {
    auto* const loader = get_native_loading_queue_owner_004fde20(
        actual_manager_publication_01090aa0, actual_publication_00e18d4c);
    auto* const jobs = loader->jobs_10;
    void* const current_front = jobs[0];
    *static_cast<volatile std::uint32_t*>(current_front) = 2;
}

std::uint8_t poll_native_event_owner_00bd1920(const NativeEventOwnerStorage* owner) noexcept {
    const DWORD status = WaitForSingleObject(owner->handle_04, 0);
    if (status == WAIT_OBJECT_0) return 1;
    if (status == WAIT_TIMEOUT || status == WAIT_ABANDONED_0) return 0;
    if (status == WAIT_FAILED) {
        (void)GetLastError();
        return 0;
    }
    return static_cast<std::uint8_t>(status);
}
} // namespace bsp
