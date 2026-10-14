#pragma once

#include "bsp/fixed_step_job_waves.hpp"
#include "bsp/native_frame_job_lifetime.hpp"

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native fixed-step job waves require MSVC Win32.
#endif

namespace bsp {

// Borrow the four actual lazy-singleton publication cells and the application's
// existing 01090AA0 domain. The native factories allocate 8/8/8/12 bytes, leave
// interpolation+08 uninitialized, and register the secondary owner at +04.
struct NativeFixedStepJobDescriptors {
    void* volatile& wave1_00f878d4;
    void* volatile& wave2_00f878dc;
    void* volatile& wave3_00f878d0;
    void* volatile& interpolation_00f878d8;
    SingletonLifetimeDomain& lifetime_01090aa0;
};
void* native_step_job_wave1_descriptor_008755a0(NativeFixedStepJobDescriptors&);
void* native_step_job_wave2_descriptor_00875750(NativeFixedStepJobDescriptors&);
void* native_step_job_wave3_descriptor_008754d0(NativeFixedStepJobDescriptors&);
void* native_step_job_interpolation_descriptor_00875670(NativeFixedStepJobDescriptors&);

// Extend the SAME domain's deleting-destructor dispatcher with this resolver.
// The pointer is the registered secondary (+04), not the primary job owner.
// Returns false for a different current table; true means the native thunk
// and scalar destructor ran, including publication clear and flags&1 free.
bool delete_native_fixed_step_descriptor_current_00(void* actual_secondary,
    std::uint32_t flags, NativeFixedStepJobDescriptors&) noexcept;

// Required existing bindings for the actual raw tick receiver. Each virtual
// method resolves that receiver's CURRENT table in its established Source
// execution domain; resolution must not execute a callback or mutate it.
// No replacement receiver, generic tick, retain/release or success fallback.
class NativeFixedStepTickBindings {
public:
    virtual ~NativeFixedStepTickBindings() = default;
    virtual void step_current_04(void* actual_element, float argument) = 0;
    virtual void pre_current_08(void* actual_element, float argument) = 0;
    virtual void post_current_0c(void* actual_element) = 0;
    virtual void refresh_world_00414db0(void* actual_payload) = 0;
    virtual void build_inverse_00b63d50(void* actual_output_110,
        const void* actual_source_cc) = 0;
    virtual void run_subnodes_008759b0(void* actual_element, float argument) = 0;
};

// A decorator for the application's existing job dispatcher. These are the
// actual four primary tables, not fabricated callable tables. Other owners
// continue through the existing dispatcher. Worker/caller bindings must share
// this same dispatcher and keep every dependency alive through pool shutdown.
struct NativeFixedStepJobTables {
    const volatile std::uint32_t* wave1_00d0deb4;
    const volatile std::uint32_t* wave2_00d0dec4;
    const volatile std::uint32_t* wave3_00d0deac;
    const volatile std::uint32_t* interpolation_00d0debc;
};
class NativeFixedStepJobDispatch final : public NativeFrameJobDispatch {
public:
    NativeFixedStepJobDispatch(const volatile float& actual_fixed_step_00d0de84,
        NativeFixedStepJobTables, NativeFixedStepTickBindings&,
        NativeFrameJobDispatch& remaining);
    void execute_current_00(void* actual_job_owner, std::uint32_t argument) override;
private:
    const volatile float& fixed_step_;
    NativeFixedStepJobTables tables_;
    NativeFixedStepTickBindings& ticks_;
    NativeFrameJobDispatch& remaining_;
};

struct NativeFixedStepJobWaveCounts {
    std::uint64_t elements_seen{};
    std::uint64_t jobs_queued{};
    std::uint64_t group_dispatches{};
};

// Actual five 68h groups at F876C0, with fixed +00/+34 sentinel backing,
// actual element/payload addresses, and the SAME frame owner/domain/bindings
// used by rendering/effect jobs. Owns no pool, thread, event, queue or receiver.
// Call only after genuine producer construction and 00874C90 publication;
// retain stable groups/elements/payloads through dispatch and worker waits.
// No capacity guard, rollback, admission rescan or second body walk is added.
// Construction binds references only; it does not create any game owner.
class NativeFixedStepJobWaves final {
public:
    NativeFixedStepJobWaves(void* actual_groups_00f876c0,
        void* actual_pending_head_00e0b6d0, void* actual_pending_tail_00e0b704,
        const volatile std::uint8_t& actual_run_pass_00e0b6ce,
        NativeFrameJobOwnerStorage* volatile& actual_frame_0109cf08,
        NativeFrameJobLifetimeBindings&, NativeFixedStepJobDescriptors&);
    void flush_pending_groups_00874c90();
    void run_fixed_waves(NativeFixedStepJobWaveCounts&);
    void run_interpolation(float actual_leftover, NativeFixedStepJobWaveCounts&);
private:
    void run_wave(JobWavePhase, NativeFixedStepJobWaveCounts&);
    void queue_element(JobWavePhase, void* actual_element);
    void dispatch_group();
    void* groups_;
    void* pending_head_;
    void* pending_tail_;
    const volatile std::uint8_t& run_pass_;
    NativeFrameJobOwnerStorage* volatile& frame_;
    NativeFrameJobLifetimeBindings& frame_lifetime_;
    NativeFixedStepJobDescriptors& descriptors_;
};

// Original bodies, raw layouts and new explicit C++ ABI. This service does
// not attach itself to a Source mission or establish native ABI/gameplay parity.
} // namespace bsp
