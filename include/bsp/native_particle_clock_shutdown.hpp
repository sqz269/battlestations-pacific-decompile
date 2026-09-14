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
};

// Actual secondary owner is complete clock+4. The two profile views above
// borrow the current native CE7D08/CE7D24 tables, whose +10 slots identify
// 004DDB40. Capture the current last sink before reading the current profile;
// use the current IAT/raw sink slot0 path, then fresh count/data after it.
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

} // namespace bsp
