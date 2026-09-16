#pragma once
#include <cstdint>

namespace bsp {
struct NativeFrameJobOwnerStorage;
class NativeFrameJobLifetimeBindings;
struct NativeRenderPreparationJobStorage;
struct NativeRenderPreparationJobSecondary;

// Borrow the actual manager and both actual job publication cells. The frame
// bindings supply the existing scheduler, random-thread registry, scope and
// real Win32 entry. They are required only for frame construction/destruction.
// Keep this same context alive through raw manager drain; no projected manager,
// extra singleton domain, implicit owner reset or scheduler is created here.
struct NativeRenderJobPublicationContext {
    void* volatile& actual_manager_01090aa0;
    NativeFrameJobOwnerStorage* volatile& actual_frame_0109cf08;
    NativeRenderPreparationJobStorage* volatile& actual_preparation_00f8d444;
    NativeFrameJobLifetimeBindings* frame_lifetime{};
};

// Full 4C1130[192], native cdecl/no arguments/EAX current primary/RET.
// Raw manager+10 captured guard; allocate138A8, genuine4BFA40, publish,
// fresh manager lookup then CURRENT publication registration. Constructor
// failure frees saved allocation after constructor cleanup. Registration
// failure retains publication. Normal Leave remains inside native state0.
NativeFrameJobOwnerStorage* get_native_frame_jobs_actual_004c1130(
    NativeRenderJobPublicationContext&);

// Full B0FFB0[206], native cdecl/no arguments/EAX current primary/RET.
// Same raw manager; allocate8, write secondaryD5E154/primaryD5E160/finalD5E15C.
// Capture published secondary+4 (or null) BEFORE the second manager getter,
// register that captured address, leave captured guard, reload publication.
NativeRenderPreparationJobStorage* get_native_preparation_job_actual_00b0ffb0(
    NativeRenderJobPublicationContext&);

// Full B0F1D0[26]: ECX primary, RET. Clear actual F8D444, then secondary
// CE3818. A null primary targets address zero, not +4; no null-success path.
void destroy_native_preparation_job_actual_00b0f1d0(
    NativeRenderPreparationJobStorage*, NativeRenderJobPublicationContext&) noexcept;
// Full B0F210[52]: ECX primary, flags stack/RET4, EAX original primary.
// Clear/reset, then ordinary-free original primary iff flags&1. No unregister.
NativeRenderPreparationJobStorage* delete_native_preparation_job_actual_00b0f210(
    NativeRenderPreparationJobStorage*, std::uint32_t flags,
    NativeRenderJobPublicationContext&) noexcept;
// Full B0F1C0[8]: subtract4 from ECX, tail dispatch to B0F210.
NativeRenderPreparationJobStorage* delete_native_preparation_job_secondary_actual_00b0f1c0(
    NativeRenderPreparationJobSecondary*, std::uint32_t flags,
    NativeRenderJobPublicationContext&) noexcept;
// Full B0D930[41]: supplied construction base itself is reset/freed, no -4.
NativeRenderPreparationJobSecondary* delete_native_preparation_job_base_actual_00b0d930(
    NativeRenderPreparationJobSecondary*, std::uint32_t flags,
    NativeRenderJobPublicationContext&) noexcept;

// Source interfaces add borrowed contexts. Original FH3/SEH/private-frame,
// register/fault ABI and active application/thread execution are separate
// validation boundaries. Native table words remain numeric identities.
} // namespace bsp
