// Packet cc8_ship_ai_approach_slot_tune. Evidence:
// docs/SHIP_AI_APPROACH_SLOT_TUNE.md.
//
// Transcribed from the listing of BSP_UnitVehicleBase_Construct between the
// allocator call at 0081F200 and the pointer store at 0081F28C. The stores are
// SSE MOVSS from .rdata addresses, not x87, so there is no rounding to model:
// each field is the exact float those four bytes spell.

#include "bsp/ship_ai_approach_tune.hpp"

namespace bsp {

ShipAiApproachTune ship_ai_approach_tune_defaults_0081f200() noexcept
{
    ShipAiApproachTune out{};
    out.slot_score_scale = 10.0f;      // 0081F214, 00CE38B8 = 41200000
    out.slot_weight = 4.0f;            // 0081F220, 00CE3D34 = 40800000
    out.avoid_strength = 3.0f;         // 0081F22D, 00CE3854 = 40400000
    out.avoid_span = 1000.0f;          // 0081F254, 00CE3804 = 447A0000
    out.evade_bearing = 1.0f;          // 0081F205/0081F269, 00D7A24C = 3F800000
    out.evade_weight = 0.25f;          // 0081F23A, 00CE3868 = 3E800000
    out.evade_span = 1.0471976f;       // 0081F247, 00D05AAC = 3F860A92, pi/3
    out.range_override = -1.0f;        // 0081F261/0081F26E, 00D7A260 = BF800000
    return out;
}

}  // namespace bsp
