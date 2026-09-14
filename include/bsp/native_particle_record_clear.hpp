#pragma once

#include "bsp/native_particle_record_resize.hpp"

namespace bsp {

// 004DCAA0..004DCAA7: ECX actual vector, PUSH0/CALL004DC410/RET.
// Original stack arguments: none. EDX now borrows the concrete resize context;
// the existing raw resize provider owns all current-storage and cleanup rules.
// No native caller is established. This entry lies directly after4DCA80's RET4,
// which caused the linear candidate sweep to attribute its CALL to4DCA80.
void __fastcall clear_native_particle_record_array_004dcaa0(
    NativeResourceRecordVectorStorage& actual_vector,
    NativeParticleRecordResizeContext& context);

} // namespace bsp
