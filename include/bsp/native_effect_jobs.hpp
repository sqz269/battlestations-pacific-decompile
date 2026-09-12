#pragma once

#include "bsp/native_frame_job_lifetime.hpp"
#include "bsp/native_live_effect_manager.hpp"
#include "bsp/point_effect_children.hpp"

namespace bsp {

// Actual eight bytes at F87658. The second word is the lifetime interface;
// neither word is an intrusive count. Registration uses primary+4.
struct NativeEffectJobOwnerStorage {
    volatile std::uint32_t primary_table_00;
    volatile std::uint32_t secondary_table_04;
};
static_assert(sizeof(NativeEffectJobOwnerStorage) == 8);
static_assert(offsetof(NativeEffectJobOwnerStorage, secondary_table_04) == 4);

class NativeEffectJobEvents {
public:
    virtual ~NativeEffectJobEvents() = default;
    // REQUIRED pure lookup of the exact raw event's EXISTING canonical owner.
    // No callback, bind, allocation, count change or invented fallback event.
    virtual RenderCommandReference& existing_reference(void* actual_event) noexcept = 0;
};

// Complete8663B0, ECX job ignored/stack raw event/RET4. Read delta/reference
// globals at execution time, with x87 FLD/FSTP for delta, then call the exact
// event's CURRENT virtual28. Nonnull valid raw event is a native precondition.
void execute_native_effect_job_008663b0(void* actual_event,
    volatile float& actual_delta_00f87608, void* volatile& actual_reference_00f8760c,
    NativeEffectJobEvents&, PointEffectChildEvents&);

// Decorates the application's existing job executor. Reads the raw owner's
// current table/word0 on EVERY execution; only D0D3E8:8663B0 is established
// here. Remaining jobs require their real execution binding. No owned jobs.
class NativeEffectJobDispatch final : public NativeFrameJobDispatch {
public:
    NativeEffectJobDispatch(volatile float& actual_delta_00f87608,
        void* volatile& actual_reference_00f8760c,
        const volatile std::uint32_t* actual_table_00d0d3e8,
        NativeEffectJobEvents&, PointEffectChildEvents&, NativeFrameJobDispatch& remaining);
    void execute_current_00(void* actual_job_owner, std::uint32_t argument) override;
private:
    volatile float& delta_;
    void* volatile& reference_;
    const volatile std::uint32_t* table_;
    NativeEffectJobEvents& events_;
    PointEffectChildEvents& children_;
    NativeFrameJobDispatch& remaining_;
};

// Complete8667F0, cdecl/RET. Actual domain lock/recheck; allocate8, set both
// identities, publish. Capture SECONDARY registration pointer BEFORE the
// second415350, register it, unlock captured section, reload current global.
// Registration failure preserves publication; allocation failure only unlocks.
NativeEffectJobOwnerStorage* native_effect_job_singleton_008667f0(
    NativeEffectJobOwnerStorage* volatile& actual_global_00f87658,
    SingletonLifetimeDomain& actual_domain_01090aa0);

// Complete866400, ECX primary/stack flags/RET4/EAX original primary. Clear
// current global unconditionally, restore secondary CE3818, free iff flags&1.
// No unregister; the shared domain has popped its exact registration already.
NativeEffectJobOwnerStorage* delete_native_effect_job_00866400(
    NativeEffectJobOwnerStorage*, std::uint32_t flags,
    NativeEffectJobOwnerStorage* volatile& actual_global_00f87658) noexcept;
// Complete8-byte8663D0: adjust captured secondary ECX by-4, tail-call866400.
// The application must dispatch the registered pointer through its CURRENT
// D0D3E4 word0=8663D0. Return is the primary allocation, not the secondary.
NativeEffectJobOwnerStorage* delete_native_effect_job_secondary_008663d0(
    volatile std::uint32_t* actual_secondary, std::uint32_t flags,
    NativeEffectJobOwnerStorage* volatile& actual_global_00f87658) noexcept;

// Complete866C60, ECX manager/stack(delta,reference)/RET8. Capture the raw
// references_1c span once. Per cell: actual frame getter, THEN read cell;
// capture secondary pool, actual job getter, CURRENT enqueue04, actual enqueue.
// Always get the current frame again and call CURRENT dispatch08(use_workers1).
// Captured spans/owners survive callbacks. No retain, filtering, clear, queue
// substitute, dispatch exception rollback or independent singleton domain.
void dispatch_native_effect_entries_00866c60(NativeLiveEffectManagerStorage&,
    float delta, void* actual_reference,
    volatile float& actual_delta_00f87608, void* volatile& actual_reference_00f8760c,
    NativeEffectJobOwnerStorage* volatile& actual_global_00f87658,
    NativeFrameJobOwnerStorage* volatile& actual_frame_0109cf08,
    SingletonLifetimeDomain& actual_domain_01090aa0, NativeFrameJobLifetimeBindings&);

// Actual owner layout and established C++ implementations; new C++ ABI.
// Native integer tables/EH and original gameplay execution are not implied.
} // namespace bsp
