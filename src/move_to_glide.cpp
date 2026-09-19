#include "bsp/move_to_glide.hpp"

#include <cmath>

#include "bsp/unit_rudder.hpp"  // clamped_interpolate_00419010

// `009C18C0 BSP_BotStateMoveTo_Tick`, the two halves that command anything: step 1's planar
// separation and step 5's glide slope. Read from the listing, 009C18C0-009C1BB3.
// docs/DIVE_BOMB_APPROACH.md. Every name is a hypothesis, not a recovered symbol.

namespace bsp {

// 009C192C-009C1958 builds `dx*dx + dz*dz` on the x87 stack from `target - unit`, components 0
// and 2 only; 009C195C compares it against the double at 00CE3820 and takes `00BF7030 sqrt`
// only when the square exceeds it, else 009C1982 leaves the result at zero.
float move_to_planar_distance_009c1950(float dx, float dz) {
    const double planar = static_cast<double>(dx) * dx + static_cast<double>(dz) * dz;
    if (planar <= move_to_glide_constant::kDistanceEpsilonSq) {
        return 0.0f;
    }
    return static_cast<float>(std::sqrt(planar));
}

MoveToGlideCommand move_to_glide_009c18c0(const MoveToGlideInputs& in) {
    MoveToGlideCommand out;

    // 009C19F1-009C1A1F. `FLD [EDI+34h] / FADD [ESP+20h]` is the far range lifted by the
    // TARGET's live world Y, not by any aim-point field; `FCOMIP` then floors it with the
    // near range. Equivalent to bsp::move_to_target_altitude in bot_task_states.hpp, kept
    // here so this file states 009FBA50's whole argument list in one place.
    const float lifted = in.far_range_34 + in.target_world_y;
    out.base = lifted < in.near_range_30 ? in.near_range_30 : lifted;

    // 009C1A35-009C1A6B. 1400 less the aircraft's own altitude, floored at 50.
    float margin = move_to_glide_constant::kMarginCeiling - in.unit_world_y;
    if (margin < move_to_glide_constant::kMarginFloor) {
        margin = move_to_glide_constant::kMarginFloor;
    }

    // 009C1A71-009C1AB3. The distance still to close past 1000 m, clamped into [50, 2000].
    // The floor is implicit: 009C1A89's `JA` leaves XMM0 holding the 50.0f that 009C1A3B
    // loaded for the margin, so one constant serves both clamps.
    float denom = in.planar_distance - move_to_glide_constant::kDenomOffset;
    if (denom < move_to_glide_constant::kDenomFloor) {
        denom = move_to_glide_constant::kDenomFloor;
    }
    if (denom >= move_to_glide_constant::kDenomCap) {
        denom = move_to_glide_constant::kDenomCap;
    }

    // 009C1AB9-009C1AF3.
    out.scale = clamped_interpolate_00419010(
        move_to_glide_constant::kScaleRatioLow,
        move_to_glide_constant::kScaleAtLow,
        move_to_glide_constant::kScaleRatioHigh,
        move_to_glide_constant::kScaleAtHigh,
        margin / denom);

    // 009C1B09 and 009C1B01: the second and third arguments of the 009C1B17 call.
    out.range_low = in.speed_range_38;
    out.range_high = in.planar_distance;
    return out;
}

}  // namespace bsp
