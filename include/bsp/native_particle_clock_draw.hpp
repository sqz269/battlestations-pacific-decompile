#pragma once

namespace bsp {

// Complete B19A10, actual 1Ch clock owner. Original ECX owner, one float32
// stack word, RET4. Unused EDX reserves that original stack slot in this
// source fastcall declaration. +18 receives the original raw input bits.
// Each record's actual virtual sink receives a fresh x87 load/spill of the
// current public argument, preserving x87 status/NaN behavior and callbacks
// that modify the argument. No typed clock, callback projection or mode clamp.
void __fastcall set_native_particle_clock_time_00b19a10(
    void* actual_clock, void* unused_edx, float frame_time);

// Array start+8 and count+Ch define 2Ch records with sink pointer+28h.
// Initial cursor is captured before the +18 store. After each virtual call,
// current count and base form the wrapped end BEFORE advancing the cursor.
// Existing native storage/callable vtable and valid traversal are required.
} // namespace bsp


#include <cstddef>

namespace bsp {
// Borrow the actual current profile views, not substitute callable vtables.
// All backing/context identities survive the call. No ownership or publication
// binding is installed here. Original numeric/naked entry above is preserved.
struct NativeParticleClockDrawContext {
    const void* actual_profile_00d64478;
    const void* actual_profile_00d644b4;
    const void* actual_profile_00d79b54;
    const void* actual_milliseconds_scale_00ce47a0;
};
static_assert(sizeof(NativeParticleClockDrawContext) == 16);
static_assert(offsetof(NativeParticleClockDrawContext, actual_profile_00d64478) == 0);
static_assert(offsetof(NativeParticleClockDrawContext, actual_profile_00d644b4) == 4);
static_assert(offsetof(NativeParticleClockDrawContext, actual_profile_00d79b54) == 8);
static_assert(offsetof(NativeParticleClockDrawContext, actual_milliseconds_scale_00ce47a0) == 12);

// Explicit source-dispatch overload. Same actual owner/records/sinks; no added
// retain, count change, null-sink skip, synthetic profile, patched pointer or
// callback substitution. Current sink/profile/slot is captured between fresh
// FLD and FSTP, then qualified C302A0 invokes the genuine source leaf. Unknown
// profile/missing view/unsupported slot throws AFTER the spill at this regular
// C++ boundary, with no rollback; never through the original naked entry.
// Caller retains the actual owner/raw-view lifetime obligations separately.
void set_native_particle_clock_time_00b19a10(void* actual_clock,
    float milliseconds, NativeParticleClockDrawContext&);

} // namespace bsp
