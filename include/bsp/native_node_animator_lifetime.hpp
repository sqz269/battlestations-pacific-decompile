#pragma once
#include "bsp/native_resource_instance_postprocess.hpp"
#include <cstdint>

namespace bsp {
// Object-write fragments inside B79BC0, not standalone recovered constructors.
// Caller allocates first and, for optimized animators, runs the current type
// predicate before these writes. No allocation, type lookup or stack-local
// writes are included here. Both start with CEB130/ref1; translation/angles
// are zero, borrowed track header20/24/28 and retained registry2C are null.
// Optimized38h adds flag30=(compatible_skin_node==null), borrowed item34=null,
// leaves31..33 untouched and publishes D62EB0 after its base fields.
// Ordinary30h publishes D62E60 before the remaining fields.
void initialize_native_track_animator_fragment_00b7a0ba(void* actual_animator);
void initialize_native_compact_animator_fragment_00b79d44(void* actual_animator,
    const void* compatible_skin_node);

// Full physical B780D0..B78161: ECXanimator/RET. Stamp D62E60, release current
// registry via actual slot0 and then clear2C; resize borrowed track array to0,
// free its current data, stamp D5C104 and invoke BD30F0. The source borrows
// the existing complete lifetime protocol; native FH3 is not reconstructed.
void destroy_native_node_animator_00b780d0(void*, NativeResourceAnimatorLifetime&);
// Original ECXself/stackflags/EAXself/RET4. Optimized wrapper first stamps
// D62EB0; both call the common destructor then free iff flags bit0 is set.
void* delete_native_track_animator_00b786e0(void*, std::uint32_t flags,
    NativeResourceAnimatorLifetime&);
void* delete_native_compact_animator_00b78720(void*, std::uint32_t flags,
    NativeResourceAnimatorLifetime&);

// B76790/B77530 are complete byte-equivalent specializations of the existing
// B1C500/B1C770 borrowed pointer reserve/resize. Reuse those canonical source
// entries on the SAME NativeRenderPointerArrayStorage at animator+20.

// B778C0: ECXregistry/stack signed index and raw float bits/RET8. Grow the
// registry sample array as needed; old sample passes through x87 before its
// comparison. Incoming wins on equal/unordered; raw incoming payload remains
// intact on the selected MOVSS path. Unused EDX preserves original stack ABI.
void __fastcall include_native_animation_sample_00b778c0(void*, void*,
    std::int32_t index, std::uint32_t incoming_bits);
// B77990: ECXanimator/RET. Current signed count/backing each iteration; for
// nonnull track, capture registry2C and track1C value before potential resize.
void __fastcall finalize_native_track_animator_00b77990(void*);
// B92610: ECXitem/stackregistry/RET4. Current itemC backing and10 count, stride
//18h, scalar at record0; x87-copy scalar to argument, include by ordinal index.
void __fastcall include_native_compact_track_samples_00b92610(void*, void*, void* registry);
// B75EE0: ECXanimator/RET. If current borrowed item34 is nonnull, delegate
// using current registry2C; flag30 is not read by this finalizer.
void __fastcall finalize_native_compact_animator_00b75ee0(void*);
} // namespace bsp
