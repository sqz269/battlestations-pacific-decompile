// Packet cc13-ver16. See include/bsp/flak_ver_cc13.hpp and docs/CC13_VER_16.md.
// Precision: gameplay runs the x87 at 24-bit precision control, so each x87
// step below rounds to a float mantissa; float arithmetic reproduces it.
#include "bsp/flak_ver_cc13.hpp"

namespace bsp {

bool flak_bot_range_gate_0090332c(float lead_distance, float min_range,
                                  float max_range) noexcept {
    // 0090332C FLD [edx+58h]; FLD dist; FCOMI ST1; JBE 00903412 -> refuse.
    if (!(lead_distance > min_range)) return false;
    // 0090333D FLD [edx+60h]; FCOMPI ST1; JBE 009033DB -> refuse.
    return max_range > lead_distance;
}

float flak_bot_vertical_adjust_00903297(float vertical, bool target_is_plane) noexcept {
    // 00903297 FLD v; FLDZ; FCOMPI ST1; JBE 009032ED: 0 <= v (or NaN) keeps v.
    if (!(0.0f > vertical)) return vertical;
    // 009032A3-009032C4: target->vtable[5Ch](0Fh); a plane is aimed flat.
    if (target_is_plane) return 0.0f;
    // 009032C6 COMISS [00D7A320], v; JBE: only below -0.02f.
    if (!(kFlakBotLowVertical > vertical)) return vertical;
    // 009032D5-009032E9: FLD v; FLD qword 0.02; FADD ST1; FMUL qword 0.5;
    // FSUBP ST1; FSTP float. 0.02 is the widened float, exact as a float.
    const float bias = static_cast<float>(kFlakBotVerticalBias);
    const float half = (bias + vertical) * 0.5f;
    return vertical - half;
}

bool flak_fuse_armed_0070c3da(float flight_time, float fuse_time,
                              float scaled_step) noexcept {
    // 0070C3DA FLD [ecx+D8h]; FXCH; FCOMPI ST1; JBE 0070C808: flight <= fuse
    // (or NaN) skips the search.
    if (!(flight_time > fuse_time)) return false;
    // 0070C3EC-0070C400: UCOMISS step, [00D7A218]; JNP after TEST AH,44h
    // leaves when the step equals 0.0f.
    return scaled_step != 0.0f;
}

}  // namespace bsp
