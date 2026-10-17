#pragma once
// 00902290, the turning-ship lead (packet s54_seg59_helpers, docs/SHIP_AI_OPEN_ITEMS.md
// 268). Name and field meanings are hypotheses, not recovered symbols. Not wired: the
// gunnery host's player-seat artillery aim records the call (009578C3,
// "PlayerGunSeat::artillery_hit_lead") instead.
//
// Original ABI: __fastcall(ECX = const float* shooter_pos, EDX = Entity* target)
// (float speed, const float* shooter_vel, float* out_aim, float* out_matrix 4x4,
// float* out_distance), RET 14h, body 00902290..0090291F. Sole caller 00957964 in
// 00957740 BSP_Aim_ResolveRayToWorldPoint, for a hit entity answering IsKindOf(6).
// Arithmetic follows the listing under the main thread's 24-bit x87 precision
// (docs/X87_CONTROL_WORD.md): every intermediate is a float.
#include "bsp/camera_projection.hpp"

#include <array>

namespace bsp {

// What 00902290 reads from the target, in read order.
struct ShipTurnLeadTarget {
    float field_9dc{0.0f};   // target+9DCh (00902299)
    float field_984{0.0f};   // target+984h (009022A0); their product gates the turn path
    // target+CCh..+108h after 00414DB0 (the refresh at 00902394 / 0090262B when
    // +C8h is clear): row 2 (+ECh, +F4h used) and row 3 (+FCh..+104h, the position).
    CameraMatrix pose{};
    std::array<float, 3> velocity{};   // target->vtable[34h](buf) (009023CC)
    float field_980{0.0f};   // target+980h, taken as |x| for 0082E890 (009025CE)
    float class_520{0.0f};   // [target+538h]+520h (0090260F)
    float yaw_rate{0.0f};    // target->vtable[38h]() (00902690), a float return
};

class ShipTurnLeadHost {
public:
    virtual ~ShipTurnLeadHost() = default;
    // 0082E890 BSP_ShipClass_ComputeRudderCurveDenominator(float), RET 4, ST0.
    virtual float rudder_curve_denominator_0082e890(float magnitude) = 0;
    // 00901C20 BSP_GunBot_InterceptSolution(ECX shooter, EDX target)(speed,
    // shooter_vel, out_aim, out_distance), RET 10h: the non-turning tail (00902379).
    virtual void intercept_solution_00901c20(const float* shooter, float speed,
        const float* shooter_vel, float* out_aim, float* out_distance) = 0;
};

struct ShipTurnLeadResult {
    bool via_intercept{false};   // the product was 0: 00901C20 answered (out_aim/out_distance its own)
    bool led{false};             // the turning lead ran (out_matrix written with the yaw rotation)
    float time{0.0f};            // the time used, 0.05 + t (0 when not led)
};

// out_matrix is written only on the 00901C20 path (identity) and on the led path
// (the yaw rotation 00B646E0); a turning target with no positive time leaves it.
ShipTurnLeadResult seg59_ship_turn_lead_00902290(const float shooter[3],
    const ShipTurnLeadTarget& target, float speed, const float shooter_vel[3],
    float out_aim[3], CameraMatrix& out_matrix, float* out_distance, ShipTurnLeadHost& host);

}  // namespace bsp
