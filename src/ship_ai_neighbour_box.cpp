// 009EAE20 and 009EAFC0, the two neighbour-node box refreshes 009F0EA0 runs.
// Evidence, ABI and uncertainty: docs/SHIP_AI_NEIGHBOUR_BOX.md.
// Unordered-branch corrections and bounded original-byte evidence:
// docs/SHIP_AI_NEIGHBOUR_BOX_MATH.md (009EAFC0 and its009EB4D1 arc arm only).
//
// Semantic projections for MSVC Win32, not binary replacements. Written
// operation for operation from the listing: where the native leaves a value on
// the x87 stack across several operations and stores once, the projection uses a
// double and casts at the store; where it stores a float between two operations,
// the projection stores a float too.

#include "bsp/ship_ai_neighbour_box.hpp"

#include <cmath>

namespace bsp {
namespace {

// 00415550 BSP_Math_MaxFloatByRef: FCOMIP a,b then JBE returns b, so an
// unordered pair returns b. 009EB19B, 009EB27F, 009EB34B.
float max_by_ref_00415550(float a, float b) noexcept
{
    return (a > b) ? a : b;
}

// 00415510 BSP_Math_MinFloatByRef: FCOMIP b,a then JBE returns b.
// 009EB296, 009EB329.
float min_by_ref_00415510(float a, float b) noexcept
{
    return (b > a) ? a : b; // JBE also selects b for unordered operands
}

// 00415620 BSP_Math_ClampFloatByRef, RET 4: low wins only on ordered JA;
// the second JBE retains value on unordered comparison. 009EB444.
float clamp_by_ref_00415620(float value, float low, float high) noexcept
{
    if (low > value) {
        return low;
    }
    return (value > high) ? high : value;
}

// 009EB594..009EB5A0 / 009EB5D2..009EB5DE. The original retains radius
// in x87, multiplies by each projected axis and spills before the center add
// or subtract. x87's two-NaN selection differs from SSE MULSS here.
float projected_offset_product_009eb596(float radius, float axis) noexcept
{
    float result;
    __asm {
        fld radius
        fmul axis
        fstp result
    }
    return result;
}

// 009EAE60..009EAE7A, the same fold 006BC0C0 does at 006BC0C5..006BC0E3:
// pi/2 - heading, lifted by one turn when it lands below zero. The subtraction
// and the lift are both double-to-float stores.
float fold_heading_to_bearing(float heading) noexcept
{
    float bearing = static_cast<float>(kShipAiSectorBearingOrigin - static_cast<double>(heading));
    if (bearing < 0.0f) { // 009EAE6E FLDZ, 009EAE70 FCOMIP, 009EAE72 JBE
        bearing = static_cast<float>(static_cast<double>(bearing) + kShipAiSectorBearingTurn);
    }
    return bearing;
}

// 009EB2C9..009EB301, the tail every near arm falls into: the avoid box becomes
// the near box, field for field.
void copy_near_box_to_avoid_box(ShipAiObstacleNode& node, ShipAiNeighbourNodeMotion& motion) noexcept
{
    node.avoid_box_x = node.near_box_x;            // 009EB2C9, 009EB2CD
    node.avoid_box_z = node.near_box_z;            // 009EB2D1, 009EB2D4
    node.corner_beam_x = node.axis_beam_x;         // +4Ch <- +28h, 009EB2D7
    node.corner_beam_z = node.axis_beam_z;         // +50h <- +2Ch, 009EB2DD
    node.corner_forward_x = node.axis_forward_x;   // +54h <- +30h, 009EB2E3
    node.corner_forward_z = node.axis_forward_z;   // +58h <- +34h, 009EB2E9
    node.avoid_half_beam = node.near_half_beam;    // +5Ch <- +38h, 009EB2EF
    node.avoid_half_length = node.near_half_length;// +60h <- +3Ch, 009EB2F5
    motion.projected_heading_64 = motion.heading_40; // +64h <- +40h, 009EB2FB
}

} // namespace

// ---------------------------------------------------------------------------
// 009EAECA..009EAF07
// ---------------------------------------------------------------------------
float ship_ai_neighbour_lookahead_009eaeca(float settings_pos_speed_corrig_1a8,
                                           float body_axis_speed,
                                           float hull_length_09c8) noexcept
{
    // 009EAECA FLD [EAX+1A8h]; 009EAED0 FMUL [ESP+1Ch]; 009EAED7 FSTP float.
    const float advance = static_cast<float>(static_cast<double>(settings_pos_speed_corrig_1a8) *
                                             static_cast<double>(body_axis_speed));
    // 009EAEDB FLD [EDI+9C8h]; 009EAEE1 FMUL double 0.25; 009EAEE7 FSTP float.
    const float cap = static_cast<float>(static_cast<double>(hull_length_09c8) *
                                         kShipAiNeighbourBoxLookaheadLengthFraction);
    // 009EAEF3 FCOMIP advance,cap; 009EAEF7 JBE keeps advance. A negative
    // advance (sternway) always wins the min, which is what grows +38h astern.
    return (advance > cap) ? cap : advance;
}

// ---------------------------------------------------------------------------
// 009EAE20
// ---------------------------------------------------------------------------
bool ship_ai_neighbour_near_box_refresh_009eae20(ShipAiObstacleNode& node,
                                                 ShipAiNeighbourNodeMotion& motion,
                                                 ShipAiNeighbourNearBoxHost& host)
{
    // 009EAE26 MOV ECX,[ESI+14h]; 009EAE2B JZ epilogue;
    // 009EAE33 CMP [ECX+5Eh],AL; 009EAE3B JZ epilogue. Nothing is written.
    if (node.owner == nullptr || node.owner_gone_5e) {
        return false;
    }

    // 009EAE46 CALL EDX; 009EAE48 FSTP [ESI+40h].
    const float heading = host.observed_heading_vtable50();
    motion.heading_40 = heading;

    // 009EAE4E MOV ECX,[EAX+1018h]; 009EAE54 CALL 0092D730.
    const float body_axis_speed = host.observed_body_axis_speed_0092d730();

    // 009EAE5D..009EAE99: the bearing fold, then FCOS and FSIN of the same
    // float. This pair is the observed hull's forward axis in world XZ.
    const float bearing = fold_heading_to_bearing(heading);
    const float forward_x = static_cast<float>(std::cos(static_cast<double>(bearing))); // 009EAE87
    const float forward_z = static_cast<float>(std::sin(static_cast<double>(bearing))); // 009EAE97
    node.axis_beam_x = forward_x; // +28h, 009EAE9D -- the FORWARD axis
    node.axis_beam_z = forward_z; // +2Ch, 009EAEA8

    // 009EAEAD FLD [ESI+2Ch]; 009EAEB8 FSTP [ESI+30h];
    // 009EAEB0 MOVSS XMM0,[00D7A208] (-0.0f); 009EAEBB SUBSS; 009EAEC0 store.
    node.axis_forward_x = node.axis_beam_z;                               // +30h
    node.axis_forward_z = kShipAiSectorNegateZero - node.axis_beam_x;     // +34h

    // The native order is CALL 00424C40 (009EAEC5), then MOV EDI,[ESI+14h]
    // (009EAED4), then FLD [EDI+9C8h] (009EAEDB); C++ argument order is
    // unspecified, so the two host calls are sequenced here.
    const float pos_speed_corrig = host.settings_pos_speed_corrig_1a8();
    const float hull_length = host.observed_hull_length_09c8();
    const float advance =
        ship_ai_neighbour_lookahead_009eaeca(pos_speed_corrig, body_axis_speed, hull_length);

    // 009EAF07 CMP [EDI+C8h],0; 009EAF2C JNZ skips; 009EAF30 CALL 00414DB0.
    // The world translation below is read only after this.
    if (!host.observed_pose_valid_00c8()) {
        host.refresh_observed_pose_00414db0();
    }
    const std::array<float, 2> position = host.observed_world_position_xz_00fc();

    // 009EAF11..009EAF28: both products are stored as floats before the adds at
    // 009EAF5A and 009EAF73.
    const float along_x = forward_x * advance; // 009EAF21
    const float along_z = advance * forward_z; // 009EAF28
    node.near_box_x = position[0] + along_x;   // +20h, 009EAF5A, 009EAF7F
    node.near_box_z = position[1] + along_z;   // +24h, 009EAF73, 009EAF86

    // 009EAF5E..009EAF67 AND 7FFFFFFFh on the stored advance.
    const float advance_magnitude = std::fabs(advance);

    // 009EAF8C FLD [EAX+9C8h]; FMUL double 0.55; FADD |advance|; FSTP [ESI+38h].
    // One rounding, at the store.
    node.near_half_beam = static_cast<float>(
        static_cast<double>(host.observed_hull_length_09c8()) * kShipAiNeighbourBoxHalfLengthScale +
        static_cast<double>(advance_magnitude));
    // 009EAF9F FLD [EAX+9CCh]; FMUL double 0.6; FSTP [ESI+3Ch].
    node.near_half_length = static_cast<float>(
        static_cast<double>(host.observed_hull_beam_09cc()) * kShipAiNeighbourBoxHalfBeamScale);
    return true;
}

// ---------------------------------------------------------------------------
// 009EB14D..009EB182
// ---------------------------------------------------------------------------
float ship_ai_neighbour_closing_speed_009eb14d(const std::array<float, 2>& relative_velocity,
                                               const std::array<float, 2>& direction_to_self,
                                               float go_away_spd_add_1d0) noexcept
{
    // 009EB14D..009EB15F: the dot is stored as a float before the add.
    const float dot = static_cast<float>(
        static_cast<double>(relative_velocity[0]) * static_cast<double>(direction_to_self[0]) +
        static_cast<double>(relative_velocity[1]) * static_cast<double>(direction_to_self[1]));
    // 009EB167 FADD [EDI+50h]; 009EB16A FSTP float.
    float closing = static_cast<float>(static_cast<double>(dot) +
                                       static_cast<double>(go_away_spd_add_1d0));
    // 009EB172 FLD1; 009EB174 FCOMIP 1.0,closing; 009EB178 JBE. An unordered
    // compare skips the floor and preserves closing.
    if (kShipAiNeighbourBoxMinClosingSpeed > closing) {
        closing = kShipAiNeighbourBoxMinClosingSpeed; // 009EB17A
    }
    return closing;
}

// ---------------------------------------------------------------------------
// 009EB277..009EB2AB
// ---------------------------------------------------------------------------
float ship_ai_neighbour_extent_shrink_009eb277(float body_axis_speed,
                                               float reference_speed_0080fc30,
                                               float near_half_length_38,
                                               float projection_time,
                                               float est_pos_size_dec_mul_1c0,
                                               float est_pos_size_dec_min_1c4) noexcept
{
    // 009EB246 FMUL double 0.05; 009EB255 FSTP float.
    float reference;
    const double* reference_fraction = &kShipAiNeighbourBoxReferenceSpeedFraction;
    __asm {
        fld reference_speed_0080fc30
        mov eax, reference_fraction
        fmul qword ptr [eax]
        fstp reference
    }
    // 009EB269 AND 7FFFFFFFh on the second 0092D730 result.
    const float speed_magnitude = std::fabs(body_axis_speed);
    // 009EB27F max; 009EB284 FDIV [ESP+38h]; 009EB28F FMUL [EDI+40h];
    // 009EB292 FSTP float. Preserve both the sole binary32 spill and the
    // caller's x87 precision for intermediate divide/multiply operations.
    const float maximum = max_by_ref_00415550(speed_magnitude, reference);
    float rate;
    __asm {
        fld maximum
        fdiv near_half_length_38
        fmul est_pos_size_dec_mul_1c0
        fstp rate
    }
    // 009EB296 min; 009EB29B FMUL excess; 009EB2A7 FLD1; 009EB2A9 FSUBRP.
    const float capped = min_by_ref_00415510(est_pos_size_dec_min_1c4, rate);
    float shrink;
    __asm {
        fld capped
        fmul projection_time
        fld1
        fsubrp st(1), st(0)
        fstp shrink
    }
    return shrink;
}

// ---------------------------------------------------------------------------
// 009EB334..009EB399
// ---------------------------------------------------------------------------
float ship_ai_neighbour_travel_limit_009eb334(float slack,
                                              float hull_length_09c8,
                                              float self_speed,
                                              float closing_speed,
                                              const ShipAiNeighbourAvoidSettings& settings) noexcept
{
    // 009EB340 FLD [EDI+2Ch]; FMUL slack; 009EB347 FSTP float.
    const float distance_limit = static_cast<float>(
        static_cast<double>(settings.est_pos_dist_limit_mul_1ac) * static_cast<double>(slack));
    // 009EB34B max([EBX+9C8h], [EDI+30h]); FMUL [EDI+34h]; 009EB353 FSTP float.
    const float length_limit = static_cast<float>(
        static_cast<double>(max_by_ref_00415550(hull_length_09c8,
                                                settings.est_pos_min_ship_length_1b0)) *
        static_cast<double>(settings.est_pos_ship_length_limit_mul_1b4));
    // 009EB35F FCOMI distance_limit,length_limit; 009EB363 JBE keeps the former.
    float limit = (distance_limit > length_limit) ? length_limit : distance_limit;

    // 009EB387 FCOMI self_speed,closing_speed; 009EB389 JBE skips the scale.
    if (self_speed > closing_speed) {
        // 009EB38B FDIVRP; FADD double 1.0; FMUL double 0.5; FMULP;
        // 009EB39B FSTP float -- the whole chain stays on the x87 stack.
        limit = static_cast<float>(
            static_cast<double>(limit) *
            ((static_cast<double>(self_speed) / static_cast<double>(closing_speed) +
              kShipAiNeighbourBoxSpeedRatioBias) *
             kShipAiNeighbourBoxSpeedRatioScale));
    }
    return limit;
}

// ---------------------------------------------------------------------------
// 009EAFC0
// ---------------------------------------------------------------------------
ShipAiNeighbourAvoidArm ship_ai_neighbour_avoid_box_refresh_009eafc0(
    ShipAiObstacleNode& node,
    ShipAiNeighbourNodeMotion& motion,
    const ShipAiNeighbourAvoidBoxInputs& inputs,
    ShipAiNeighbourAvoidBoxHost& host)
{
    // 009EAFC6..009EAFD7, then 009EAFD9/009EAFDE: the only arm that sets both
    // bytes from the same AL. node+69h is not the filter's answer here.
    if (node.owner == nullptr || node.owner_gone_5e) {
        node.no_pose_68 = true;
        node.no_arc_69 = true;
        return ShipAiNeighbourAvoidArm::owner_gone;
    }

    // 009EAFED CALL EDX (slot +20h); 009EAFF1 TEST EAX,EAX.
    if (host.observed_model_vtable20() != nullptr) {
        // 009EB005 CALL EDX again, 009EB009 CALL 0098A8E0 on its result.
        const void* model = host.observed_model_vtable20();
        std::array<float, 3> minimum{};
        std::array<float, 3> maximum{};
        host.observed_world_bounds_0098a8e0(model, minimum, maximum);
        motion.bounds_max_y_80 = maximum[1]; // +80h, 009EB00E reads the second buffer
        motion.bounds_min_y_84 = minimum[1]; // +84h, 009EB01C reads the first
    }

    // 009EB02A CMP byte [ESP+48h],0; 009EB039 SETZ AL; 009EB040 store.
    node.no_arc_69 = !inputs.avoidance_accepted;

    // 009EB03C FCOMIP node+84h,arg6 JA bail; 009EB053 FCOMIP arg7,node+80h JA
    // bail. The observed hull and this one have to overlap on Y at all.
    if (motion.bounds_min_y_84 > inputs.self_bounds_max_y ||
        inputs.self_bounds_min_y > motion.bounds_max_y_80) {
        node.no_pose_68 = true;                                       // 009EB619
        node.avoid_box_x = node.near_box_x;                           // 009EB61D
        node.avoid_box_z = node.near_box_z;                           // 009EB623
        node.avoid_half_length = kShipAiNeighbourBoxCollapsedExtent;  // +60h, 009EB629
        node.avoid_half_beam = kShipAiNeighbourBoxCollapsedExtent;    // +5Ch, 009EB62E
        motion.bounds_min_y_84 = kShipAiNeighbourBoxNoBounds;         // 009EB63B
        motion.bounds_max_y_80 = kShipAiNeighbourBoxNoBounds;         // 009EB643
        return ShipAiNeighbourAvoidArm::no_vertical_overlap;
    }

    const ShipAiNeighbourAvoidSettings settings = host.settings_ship_avoidance_180(); // 009EB05F

    // 009EB06C and 009EB071: both stores are dead, every path out of here
    // rewrites the pair (the copy tail from +38h/+3Ch, the projection from the
    // shrink). They are kept because the native makes them.
    node.avoid_half_length = kShipAiNeighbourBoxCollapsedExtent;
    node.avoid_half_beam = kShipAiNeighbourBoxCollapsedExtent;
    node.no_pose_68 = false; // 009EB076

    // 009EB07A..009EB0A4: the range from this hull to the observed near box.
    const std::array<float, 2> to_node{node.near_box_x - inputs.self_x,
                                       node.near_box_z - inputs.self_z};
    const float range = static_cast<float>(static_cast<double>(length_2d_00414c60(to_node)) -
                                           static_cast<double>(node.near_half_beam));
    // 009EB0A8..009EB0B6: and then this hull's own half length.
    const float gap = range - inputs.self_half_length;
    // 009EB0BA FMUL [EDI+24h]; 009EB0C9 FCOMIP gap,threshold; 009EB0CD JBE.
    const float arrive_distance = static_cast<float>(
        static_cast<double>(inputs.self_half_length) *
        static_cast<double>(settings.arrive_dist_min_1a4));
    if (!(gap > arrive_distance)) {
        copy_near_box_to_avoid_box(node, motion);
        return ShipAiNeighbourAvoidArm::near_box_copy;
    }

    // 009EB0D3..009EB115: the unit vector from the observed near box to us.
    const std::array<float, 2> delta{node.near_box_x - inputs.self_x,
                                     node.near_box_z - inputs.self_z};
    const float inverse_length = host.reciprocal_length_00419260(delta);
    const std::array<float, 2> direction{inverse_length * delta[0],   // 009EB10D
                                         inverse_length * delta[1]};  // 009EB115

    // 009EB119 the velocity getter, 009EB12C..009EB140 the relative velocity.
    const std::array<float, 3> observed_velocity = host.observed_velocity_vtable34();
    const std::array<float, 2> relative_velocity{inputs.self_velocity_x - observed_velocity[0],
                                                 inputs.self_velocity_z - observed_velocity[2]};
    // 009EB144 CALL 00414C60 with ECX still pointing at the argument pair
    // (009EB11D LEA ECX,[ESP+38h], not reloaded), so this is our own speed, not
    // the relative one.
    const std::array<float, 2> self_velocity{inputs.self_velocity_x, inputs.self_velocity_z};
    const float self_speed = length_2d_00414c60(self_velocity);

    const float closing_speed = ship_ai_neighbour_closing_speed_009eb14d(
        relative_velocity, direction, settings.go_away_spd_add_1d0);

    // 009EB188..009EB1A0: the distance threshold turned into a time, floored.
    const float arrive_time = max_by_ref_00415550(
        settings.arrive_time_min_1a0,
        static_cast<float>(static_cast<double>(arrive_distance) /
                           static_cast<double>(closing_speed)));
    // 009EB1A4..009EB1CE.
    const float time_to_gap = static_cast<float>(static_cast<double>(gap) /
                                                 static_cast<double>(closing_speed));
    float projection_time = time_to_gap - arrive_time;
    const float slack = static_cast<float>(static_cast<double>(gap) -
                                           static_cast<double>(closing_speed) *
                                               static_cast<double>(arrive_time));
    // 009EB1D8 JBE and 009EB1E8 COMISS: both have to be strictly positive.
    if (!(projection_time > 0.0f) || !(slack > 0.0f)) {
        copy_near_box_to_avoid_box(node, motion);
        return ShipAiNeighbourAvoidArm::near_box_copy;
    }

    // 009EB1FE..009EB212: how far the observed ship runs in that time.
    const float observed_speed = host.observed_body_axis_speed_0092d730();
    float travel = static_cast<float>(static_cast<double>(observed_speed) *
                                      static_cast<double>(projection_time) *
                                      static_cast<double>(settings.est_pos_ship_spd_mul_1bc));
    float travel_magnitude = std::fabs(travel); // 009EB21E..009EB227
    // 009EB231 COMISS |travel|,1.0f; 009EB238 JBE.
    if (!(travel_magnitude > kShipAiNeighbourBoxMinTravel)) {
        copy_near_box_to_avoid_box(node, motion);
        return ShipAiNeighbourAvoidArm::near_box_copy;
    }

    // 009EB241 runs before 009EB259; C++ argument order is unspecified, so both
    // host calls are sequenced here in the native's order.
    const float reference_speed = host.observed_reference_speed_0080fc30(); // 009EB241
    const float observed_speed_again = host.observed_body_axis_speed_0092d730(); // 009EB259
    const float shrink = ship_ai_neighbour_extent_shrink_009eb277(
        observed_speed_again,
        reference_speed,
        node.near_half_beam, // +38h, 009EB266
        projection_time,
        settings.est_pos_size_dec_mul_1c0,
        settings.est_pos_size_dec_min_1c4);

    // 009EB2B5 FCOMIP 0.0,shrink; 009EB2B7 JC continues for positive OR
    // unordered shrink. Only ordered zero/negative takes the collapse/copy arm.
    if (shrink <= 0.0f) {
        // 009EB2BB and 009EB2C0 write 1.0f into both extents and the copy tail
        // immediately overwrites them; only the byte survives.
        node.avoid_half_length = kShipAiNeighbourBoxCollapsedExtent;
        node.avoid_half_beam = kShipAiNeighbourBoxCollapsedExtent;
        node.no_pose_68 = true; // 009EB2C5
        copy_near_box_to_avoid_box(node, motion);
        return ShipAiNeighbourAvoidArm::near_box_copy;
    }

    // 009EB30E..009EB33D: the projected box is narrower than the near box, and
    // the beam shrinks six times faster than the length until it saturates.
    node.avoid_half_beam = static_cast<float>(static_cast<double>(shrink) *
                                              static_cast<double>(node.near_half_beam)); // +5Ch
    node.avoid_half_length = static_cast<float>(
        static_cast<double>(min_by_ref_00415510(
            kShipAiNeighbourBoxCollapsedExtent,
            static_cast<float>(static_cast<double>(shrink) * kShipAiNeighbourBoxBeamShrinkGain))) *
        static_cast<double>(node.near_half_length)); // +60h

    const float observed_hull_length = host.observed_hull_length_09c8(); // 009EB337
    const float limit = ship_ai_neighbour_travel_limit_009eb334(
        slack, observed_hull_length, self_speed, closing_speed, settings);

    // 009EB3B3 FCOMI |travel|,limit; 009EB3B5 JBE leaves everything alone.
    if (travel_magnitude > limit) {
        // 009EB3B7 the ratio, 009EB3CC it shortens the time as well, 009EB3C6
        // and 009EB3D6/009EB3EA rewrite the magnitude and the signed value.
        projection_time = static_cast<float>(static_cast<double>(projection_time) *
                                             (static_cast<double>(limit) /
                                              static_cast<double>(travel_magnitude)));
        travel = (travel > 0.0f) ? limit : (kShipAiSectorNegateZero - limit);
        travel_magnitude = limit;
    }
    // 009EB400 FCOMIP |travel|,0.1 (double); 009EB404 JBE.
    if (!(static_cast<double>(travel_magnitude) > kShipAiNeighbourBoxMinProjectedTravel)) {
        copy_near_box_to_avoid_box(node, motion);
        return ShipAiNeighbourAvoidArm::near_box_copy;
    }

    // 009EB428 the commanded yaw rate, 009EB42D negated, 009EB42F scaled by the
    // projection time, 009EB444 clamped to a quarter turn either way.
    const float turn = clamp_by_ref_00415620(
        static_cast<float>(-static_cast<double>(host.observed_command_yaw_rate_00811940()) *
                           static_cast<double>(projection_time)),
        -kShipAiNeighbourBoxTurnLimit, // 00CE3CCC, -pi/2, 009EB418
        kShipAiNeighbourBoxTurnLimit); // 00CE3C64, +pi/2, 009EB40A

    // 009EB466 FCOMIP 3deg,|turn|; 009EB470 JBE takes the arc.
    if (static_cast<double>(std::fabs(turn)) < kShipAiNeighbourBoxArcThreshold) {
        // 009EB475..009EB4C7: a straight run along the near box's own forward
        // axis, with the axis pair copied across unchanged. node+64h is not
        // written on this arm.
        const float along_x = node.axis_beam_x * travel; // 009EB483
        const float along_z = travel * node.axis_beam_z; // 009EB48A
        node.avoid_box_x = along_x + node.near_box_x;    // 009EB492, 009EB4A8
        node.avoid_box_z = node.near_box_z + along_z;    // 009EB49C, 009EB4AF
        node.corner_beam_x = node.axis_beam_x;           // 009EB4B2
        node.corner_beam_z = node.axis_beam_z;           // 009EB4B8
        node.corner_forward_x = node.axis_forward_x;     // 009EB4BE
        node.corner_forward_z = node.axis_forward_z;     // 009EB4C4
        return ShipAiNeighbourAvoidArm::straight;
    }

    // 009EB4DE..009EB51B: the heading the observed ship reaches, and the axis
    // pair that goes with it, built the same way 009EAE20 builds +28h..+34h.
    const float projected_heading = wrapped_angle_add_00438aa0(motion.heading_40, turn);
    motion.projected_heading_64 = projected_heading; // 009EB4F0
    const std::array<float, 2> projected_axis = host.heading_to_direction_006bc0c0(projected_heading);
    node.corner_beam_x = projected_axis[0];                                   // +4Ch, 009EB505
    node.corner_beam_z = projected_axis[1];                                   // +50h, 009EB50B
    node.corner_forward_x = node.corner_beam_z;                               // +54h, 009EB51B
    node.corner_forward_z = kShipAiSectorNegateZero - node.corner_beam_x;     // +58h, 009EB516

    // 009EB540..009EB554: the radius of the circle the run describes.
    const float radius = std::fabs(static_cast<float>(static_cast<double>(travel) /
                                                      static_cast<double>(turn)));
    // 009EB55C FCOMIP travel*turn,0.0: the turn's hand decides which side the
    // centre sits on. The product is never stored, so it stays in a double.
    const bool centre_on_beam_side =
        (static_cast<double>(travel) * static_cast<double>(turn)) > 0.0;

    const float offset_x = node.axis_forward_x * radius; // 009EB56D
    const float offset_z = node.axis_forward_z * radius; // 009EB576
    float centre_x = node.near_box_x;                    // 009EB527
    float centre_z = node.near_box_z;                    // 009EB538
    if (centre_on_beam_side) {
        centre_x = offset_x + centre_x; // 009EB580
        centre_z = offset_z + centre_z; // 009EB58C
        const float projected_x = projected_offset_product_009eb596(radius, node.corner_forward_x);
        const float projected_z = projected_offset_product_009eb596(radius, node.corner_forward_z);
        node.avoid_box_x = centre_x - projected_x; // 009EB599, 009EB5A8
        node.avoid_box_z = centre_z - projected_z; // 009EB5A0, 009EB5B4
    } else {
        centre_x = centre_x - offset_x; // 009EB5BE
        centre_z = centre_z - offset_z; // 009EB5CA
        const float projected_x = projected_offset_product_009eb596(radius, node.corner_forward_x);
        const float projected_z = projected_offset_product_009eb596(radius, node.corner_forward_z);
        node.avoid_box_x = projected_x + centre_x; // 009EB5D7, 009EB5E6
        node.avoid_box_z = projected_z + centre_z; // 009EB5DE, 009EB5F2
    }
    return ShipAiNeighbourAvoidArm::arc;
}

} // namespace bsp
