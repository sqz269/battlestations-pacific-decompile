#pragma once

#include "bsp/sound_instance.hpp"

namespace bsp {

// Complete normal-return projections of D5AD58/D5ADA0/D5ADE8 vslot30.
// Native ECX=this; stack float dt, uint listener; RET8. These C++ interfaces
// are not native object layouts or binary replacements. Field names are offsets,
// not a recovered interpretation of the source record. References preserve the
// native read order; their storage must remain live for the call.
// Requires a live current owner and actual sample storage plus the valid domain
// of update_sound_channel_00a7af10. Uses current_owner.listener.transform_c4[13]
// (native owner+F8), independently of the forwarded listener argument.
// Preserves native x87 precision/rounding mode and float spills, SSE unordered
// branches, volume24/dirty14 publication and the unconditional base update call.
// Floating exceptions must be masked; exception delivery/status-register identity
// and game/audible validation are outside this behavioral projection.
void update_retained_sound_ramp_00a7b520(SoundChannelInstance&,
    const float& native_68, const float& native_6c, float dt,
    std::uint32_t listener, SoundInstanceContext&);
void update_retained_sound_scaled_ramp_00a7b5d0(SoundChannelInstance&,
    const float& native_68, const float& native_6c, const float& native_70,
    float dt, std::uint32_t listener, SoundInstanceContext&);
void update_retained_sound_nonpositive_00a7b690(SoundChannelInstance&,
    float dt, std::uint32_t listener, SoundInstanceContext&);

} // namespace bsp
