#pragma once

#include "bsp/native_particle_clock_sink_release.hpp"
#include "bsp/native_particle_record_resize.hpp"

namespace bsp {

struct NativeParticleClockShutdownContext {
    NativeParticleRecordResizeContext& records;
    NativeParticleClockAtomic const volatile& actual_decrement_00ce2220;
    void* volatile& actual_publication_00f8d420;
    const void* actual_cache_profile_00ce7d08;
    const void* actual_cache_profile_00ce7d24;
    // Optional SAME canonical actual-resource domain. Borrow through drain;
    // null retains the existing raw callable-profile release contract.
    NativeRenderActualOwners* actual_resource_owners{};
};

// Actual secondary owner is complete clock+4. The two profile views above
// borrow the current native CE7D08/CE7D24 tables, whose +10 slots identify
// 004DDB40. Capture the current last sink before reading the current profile;
// use the current IAT/raw sink slot0 path or explicit canonical zero composition,
// then fresh count/data after it. Canonical dispatch performs no second decrement.
void __fastcall clear_native_particle_clock_records_004dda40(
    void* actual_secondary, NativeParticleClockShutdownContext& context);

// Resize0, then free current data; retain stale data/capacity as native does.
void __fastcall destroy_native_particle_record_array_004ddaa0(
    NativeResourceRecordVectorStorage& actual_vector,
    NativeParticleClockShutdownContext& context);

// Stamp CE7D08, arm array cleanup around release-all, disarm before the
// second resize0 and current-array free. The unwind target is 004DDAA0.
void __fastcall destroy_native_particle_clock_records_004de290(
    void* actual_secondary, NativeParticleClockShutdownContext& context);

// Stamp CE7D38/CE7D24; secondary destruction; clear current publication and
// stamp CE3818. Completed base cleanup also runs during source unwinding.
void __fastcall destroy_native_particle_clock_00b1b680(
    void* actual_complete_owner, NativeParticleClockShutdownContext& context);

// Original owner ECX and original flags stack word/RET4 remain. New EDX is
// the context. Read the original public flags slot AFTER owner destruction;
// free on bit0, and return the retained complete address even after free.
void* __fastcall delete_native_particle_clock_004de340(
    void* actual_complete_owner, NativeParticleClockShutdownContext& context,
    std::uint32_t flags);

// Subtract four from actual secondary ECX and tail-forward the same flags.
void* __fastcall delete_native_particle_clock_secondary_004de360(
    void* actual_secondary, NativeParticleClockShutdownContext& context,
    std::uint32_t flags);

// Actual record/pool/CRT and callable sink-profile domains are required.
// Additional contexts are new source ABIs; native private FH3 and a complete
// mixed-owner manager shutdown are separate integration boundaries.

// Full00736930[164] explicit current-publication route. Original is cdecl
// with no native inputs, RET0. Added source bindings borrow SAME manager cell,
// shutdown domain and current CE7D38/CE3818 slot-zero data views through return.
// Initial-null touches no manager/profile. Capture first manager lock; enter
// and increment; recheck publication; resolve second manager BEFORE capturing
// current publication for BCFCA0; reload current publication/profile/slotzero;
// flags1 delete; clear only after normal return; release captured first lock.
// Admit completeCE7D38->4DE340 and baseCE3818->412440 at the native dispatch
// boundary. Unsupported profiles/targets throw after completed unregister,
// with existing411EE0 guard cleanup. No clock replay/reset/rollback or manager
// drain is added. NativeFH3/SEH and original no-input ABI remain separate.
void __cdecl destroy_published_native_particle_clock_00736930(
    void* volatile& actual_manager_publication_01090aa0,
    NativeParticleClockShutdownContext& shutdown,
    const volatile std::uint32_t* actual_complete_profile_00ce7d38,
    const volatile std::uint32_t* actual_base_profile_00ce3818);

} // namespace bsp
