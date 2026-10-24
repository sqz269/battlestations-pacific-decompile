#pragma once
#include "bsp/native_string.hpp"
#include <cstddef>
#include <cstdint>
#include <exception>
#include <memory>

namespace bsp {
// Actual0Ch entries. These are raw string headers, not NativeString/std::string
// objects or owning host projections. Backing comes from the original-compatible
// CRT array domain; string buffers use the supplied actual process pool.
struct NativeGuiIconDelayedEntryStorage {
    std::uint32_t length_00;
    char* characters_04;
    const void* owner_08;
};
struct NativeGuiIconDelayedLoadsStorage {
    volatile std::uint32_t native_vtable_00;
    NativeGuiIconDelayedEntryStorage* volatile entries_04;
    volatile std::int32_t count_08;
    volatile std::int32_t capacity_0c;
};
static_assert(sizeof(void*) == 4, "Raw Icon delayed loads require Win32");
static_assert(sizeof(NativeGuiIconDelayedEntryStorage) == 0x0c);
static_assert(sizeof(NativeGuiIconDelayedLoadsStorage) == 0x10);
static_assert(offsetof(NativeGuiIconDelayedLoadsStorage, entries_04) == 4);

struct NativeGuiIconDelayedLoadsOperation {
    enum class Kind { getter, cancel, scalar_delete };
    enum class Phase { fresh, manager_guard, allocation, publication, registration,
        cancel, shrink, backing_free, publication_clear, base_profile, object_free,
        complete, failed };
    Kind kind{};
    Phase phase{Phase::fresh};
    std::uint32_t native_site{};
    NativeGuiIconDelayedLoadsStorage* actual{};
    void* allocated{};
    const void* cancel_owner{};
    std::uint32_t flags{};
    bool guard_entered{}, guard_cleanup_completed{}, registration_returned{};
    bool backing_free_started{}, object_free_started{};
    std::exception_ptr failure;
};

// One deferred producer/terminal service for the application's SAME external
// F8BD98 cell and actual string/01090AA0 manager domain. Constructor admits a
// fresh null publication and performs no native work. No private publication,
// semantic vector, native reference count or replacement manager is created.
//
// BEFORE get_00ab5700, the SAME process singleton deletion dispatcher must bind
// D5C4B8 to this service's delete_registered_00ab57c0(popped_owner, flags). Keep
// this service, publication and raw string/manager providers through full drain.
// This header does not install that shared dispatch arm or activate a caller.
//
// Current caller domain is ordinary/non-delayed Icon destruction. Queue insertion
// AB5F00 and file-delivery AB51B0/AB58D0 are not supplied; DelayedTextureLoad=true
// must remain rejected by the caller. Cancellation/drain use actual raw entries
// and also implement the observed valid nonempty cancellation/shrink schedules.
// Serialize all calls with queue/resource producers and process native drain.
// Native corrupt extents, destructive reentry and raw SEH/fault ABI are outside
// this new MSVC Win32 C++ API. Entered failures retain effects, prohibit replay
// and require retaining this entire service/provider graph through process exit.
class NativeGuiIconDelayedLoads final {
public:
    NativeGuiIconDelayedLoads(NativeStringRawPoolContext&,
        NativeGuiIconDelayedLoadsStorage* volatile& publication_00f8bd98);
    ~NativeGuiIconDelayedLoads() noexcept;
    NativeGuiIconDelayedLoads(const NativeGuiIconDelayedLoads&) = delete;
    NativeGuiIconDelayedLoads& operator=(const NativeGuiIconDelayedLoads&) = delete;

    // Native no-input RET/EAX borrowed singleton. Actual captured manager section,
    // raw10h allocation/profile, publication then second manager lookup/BD0C30.
    NativeGuiIconDelayedLoadsStorage* get_00ab5700();
    // Native ECX singleton, raw owner stack, RET4; owner remains opaque.
    void cancel_owner_00ab50e0(NativeGuiIconDelayedLoadsStorage&, const void* raw_owner);
    // Native ECX popped singleton, flags DWORD low byte, RET4/EAX same address.
    // Real AB5560 -> AB4850(0) -> backing free -> clear SAME current F8BD98 ->
    // CE3818 -> iff flags&1 physical object free. No unregister: the manager's
    // normal drain already popped this registration. For flags0 the caller owns
    // the completed CE3818 physical allocation; this service no longer borrows it.
    void* delete_registered_00ab57c0(void* popped_actual, std::uint32_t flags);

    bool requires_process_retention() const noexcept;
    const NativeGuiIconDelayedLoadsOperation* last_operation() const noexcept;
    const std::exception_ptr& failure() const noexcept;
private:
    struct Impl;
    std::unique_ptr<Impl> impl_;
};
} // namespace bsp
