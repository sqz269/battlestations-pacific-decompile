// 009EB660, the obstacle sector scan, and the two producers that feed it.
// Evidence, ABI and uncertainty: docs/SHIP_AI_SECTOR_SCAN.md.
//
// Semantic projections for MSVC Win32, not binary replacements. Every routine
// here is written operation for operation from the listing; where the native
// keeps a value in an x87 register across a store the projection keeps it in a
// double, which is what the image's FLD/FMUL double pairs do.

#include "bsp/ship_ai_sector_scan.hpp"

#include <cmath>

namespace bsp {
namespace {

// 009EBFB8..009EBFD7, 009EC03E..009EC054, 009EC0E7..009EC0FD and 009D8755:
// one conditional add, no loop. A bearing below zero is lifted by one turn.
float fold_bearing(float value) noexcept
{
    if (value < 0.0f) {
        return static_cast<float>(static_cast<double>(value) + kShipAiSectorBearingTurn);
    }
    return value;
}

// 009EBE5E, 009D87D4, 009D81AE: AND with 0x7FFFFFFF, the sign clear.
float clear_sign(float value) noexcept
{
    return std::fabs(value);
}

struct Vec2 {
    float x{0.0f};
    float z{0.0f};
};

Vec2 scaled_add(const Vec2& base, float scale, float dx, float dz) noexcept
{
    Vec2 out;
    out.x = base.x + scale * dx;
    out.z = base.z + scale * dz;
    return out;
}

// 009EC1C5..009EC201: the squared length is rounded to float before the
// comparison with the double 1e-10, and only then does the CRT square root run.
float planar_length(float dx, float dz) noexcept
{
    const float squared = dx * dx + dz * dz;
    if (static_cast<double>(squared) > kShipAiSectorLengthEpsilon) {
        return static_cast<float>(std::sqrt(static_cast<double>(squared)));
    }
    return 0.0f;
}

} // namespace

// ---------------------------------------------------------------------------
// The bearing convention
// ---------------------------------------------------------------------------
float ship_ai_sector_bearing_009ebfa1(float dx, float dz) noexcept
{
    // 009EBF9D FLD dx over FLD dz, then the CRT atan2 helper at 00BF701A takes
    // ST(1) as y and ST(0) as x, so the call is atan2(dz, dx).
    const float raw = static_cast<float>(std::atan2(static_cast<double>(dz),
                                                    static_cast<double>(dx)));
    // 009EBFAE FSUBR against the double at 00CE3830, float-stored at 009EBFB4.
    const float turned = static_cast<float>(kShipAiSectorBearingOrigin
                                            - static_cast<double>(raw));
    return fold_bearing(turned);
}

std::array<float, 2> ship_ai_sector_direction_009ec056(float bearing) noexcept
{
    // 009EC030 FSUBR, 009EC03E..009EC054 the fold, 009EC05A FCOS, 009EC06A FSIN.
    const float turned = fold_bearing(static_cast<float>(kShipAiSectorBearingOrigin
                                                         - static_cast<double>(bearing)));
    const double angle = static_cast<double>(turned);
    return {static_cast<float>(std::cos(angle)), static_cast<float>(std::sin(angle))};
}

// ---------------------------------------------------------------------------
// 009E0270: the twelve sector shapes
// ---------------------------------------------------------------------------
void ship_ai_build_sector_shapes_009e0270(
    const ShipAiSectorHullMetrics& hull,
    std::array<ShipAiObstacleSector, 12>& sectors) noexcept
{
    // Compatibility projection: derive constructor3E4 once. The actual full
    // pre-step uses the overload below with the existing stored3E4 instead.
    const float hull_length = hull.half_length_9c8; // legacy field name: full length.
    float reach;
    __asm {
        fld hull_length
        fmul qword ptr kShipAiSectorReachHullScale
        fstp reach
    }
    ship_ai_build_sector_shapes_009e0270(
        hull.turn_radius_reference, reach, hull.half_width_9cc, sectors);
}

void ship_ai_build_sector_shapes_009e0270(float radius, float stored_reach,
    float full_beam, std::array<ShipAiObstacleSector, 12>& sectors) noexcept
{
    static_assert(sizeof(ShipAiObstacleSector) == 0x2c);
    static_assert(offsetof(ShipAiObstacleSector, kind) == 0);
    static_assert(offsetof(ShipAiObstacleSector, half_width) == 4);
    static_assert(offsetof(ShipAiObstacleSector, reach) == 0x0c);
    static_assert(offsetof(ShipAiObstacleSector, lateral) == 0x10);
    ShipAiObstacleSector* slots = sectors.data();
    float scratch[2];
    // Native009E02E0..009E0499 with plain stable field reads supplied as
    // explicit float inputs. Retains x87 constants, stack/store order, FCHS,
    // both direction groups and selective byte/word stores into each record.
    __asm {
        FLD dword ptr radius // 009e02e0
        MOVSS XMM0,dword ptr radius // 009e02e4
        FMUL qword ptr kShipAiSectorWideRadiusScale // 009e02ea
        MOVSS XMM1,dword ptr kShipAiSectorStraightRadius // 009e02f0
        XOR EDX,EDX // 009e02f8
        MOV EAX,slots
        ADD EAX,0x10 // 009e02fa
        FSTP dword ptr scratch[4] // 009e0300
        FLD qword ptr kShipAiSectorOuterLateralDivisor // 009e0304
        MOVSS XMM2,dword ptr scratch[4] // 009e030a
        FLD qword ptr kShipAiSectorNearReachDivisor // 009e0310
        FLD qword ptr kShipAiSectorInnerLateralScale // 009e0316
        FLD qword ptr kShipAiSectorMidReachDivisor // 009e031c
        FLD qword ptr kShipAiSectorFarReachScale // 009e0322
    L_sector_loop:
        FLD dword ptr full_beam // 009e032e
        TEST EDX,EDX // 009e0334
        FDIV st(0),st(5) // 009e0336
        SETZ CL // 009e0338
        FSTP dword ptr scratch[4] // 009e033b
        FLD dword ptr stored_reach // 009e033f
        FDIV st(0),st(4) // 009e0345
        MOV byte ptr [EAX + -0x10],CL // 009e0347
        MOVSS dword ptr [EAX + -0xc],XMM0 // 009e034a
        FSTP dword ptr scratch[0] // 009e034f
        FLD dword ptr scratch[4] // 009e0353
        FSTP dword ptr [EAX] // 009e0357
        FLD dword ptr scratch[0] // 009e0359
        FSTP dword ptr [EAX + -0x4] // 009e035d
        FLD dword ptr full_beam // 009e0366
        FMUL st(0),st(3) // 009e036c
        FSTP dword ptr scratch[4] // 009e036e
        FLD dword ptr stored_reach // 009e0372
        MOV byte ptr [EAX + 0x1c],CL // 009e0378
        FDIV st(0),st(2) // 009e037b
        MOVSS dword ptr [EAX + 0x20],XMM2 // 009e037d
        FSTP dword ptr scratch[0] // 009e0382
        FLD dword ptr scratch[4] // 009e0386
        FSTP dword ptr [EAX + 0x2c] // 009e038a
        FLD dword ptr scratch[0] // 009e038d
        FSTP dword ptr [EAX + 0x28] // 009e0391
        FLD dword ptr full_beam // 009e039a
        FDIV st(0),st(5) // 009e03a0
        FSTP dword ptr scratch[4] // 009e03a2
        FLD dword ptr stored_reach // 009e03a6
        FMUL st(0),st(1) // 009e03ac
        MOV byte ptr [EAX + 0x48],CL // 009e03ae
        MOVSS dword ptr [EAX + 0x4c],XMM1 // 009e03b1
        FSTP dword ptr scratch[0] // 009e03b6
        FLD dword ptr scratch[4] // 009e03ba
        FSTP dword ptr [EAX + 0x58] // 009e03be
        FLD dword ptr scratch[0] // 009e03c1
        FSTP dword ptr [EAX + 0x54] // 009e03c5
        FLD dword ptr full_beam // 009e03ce
        FCHS // 009e03d4
        FDIV st(0),st(5) // 009e03d6
        FSTP dword ptr scratch[4] // 009e03d8
        FLD dword ptr stored_reach // 009e03dc
        FMUL st(0),st(1) // 009e03e2
        MOV byte ptr [EAX + 0x74],CL // 009e03e4
        MOVSS dword ptr [EAX + 0x78],XMM1 // 009e03e7
        FSTP dword ptr scratch[0] // 009e03ec
        FLD dword ptr scratch[4] // 009e03f0
        FSTP dword ptr [EAX + 0x84] // 009e03f4
        FLD dword ptr scratch[0] // 009e03fa
        FSTP dword ptr [EAX + 0x80] // 009e03fe
        FLD dword ptr full_beam // 009e040a
        FCHS // 009e0410
        FMUL st(0),st(3) // 009e0412
        FSTP dword ptr scratch[4] // 009e0414
        FLD dword ptr stored_reach // 009e0418
        MOV byte ptr [EAX + 0xa0],CL // 009e041e
        FDIV st(0),st(2) // 009e0424
        MOVSS dword ptr [EAX + 0xa4],XMM2 // 009e0426
        FSTP dword ptr scratch[0] // 009e042e
        FLD dword ptr scratch[4] // 009e0432
        FSTP dword ptr [EAX + 0xb0] // 009e0436
        FLD dword ptr scratch[0] // 009e043c
        FSTP dword ptr [EAX + 0xac] // 009e0440
        FLD dword ptr full_beam // 009e044c
        FCHS // 009e0452
        FDIV st(0),st(5) // 009e0454
        FSTP dword ptr scratch[4] // 009e0456
        FLD dword ptr stored_reach // 009e045a
        FDIV st(0),st(4) // 009e0460
        MOV byte ptr [EAX + 0xcc],CL // 009e0462
        MOVSS dword ptr [EAX + 0xd0],XMM0 // 009e0468
        FSTP dword ptr scratch[0] // 009e0470
        FLD dword ptr scratch[4] // 009e0474
        FSTP dword ptr [EAX + 0xdc] // 009e0478
        FLD dword ptr scratch[0] // 009e047e
        ADD EDX,0x1 // 009e0482
        FSTP dword ptr [EAX + 0xd8] // 009e0485
        ADD EAX,0x108 // 009e048b
        CMP EDX,0x2 // 009e0490
        JL L_sector_loop // 009e0493
        FSTP st(4) // 009e0499
        FSTP st(2) // 009e049e
        FSTP st(0) // 009e04a7
        FSTP st(0) // 009e04b7
        FSTP st(0) // 009e04bd
    }
}

// ---------------------------------------------------------------------------
// 009EF230
// ---------------------------------------------------------------------------
ShipAiSectorRefreshSlots ship_ai_sector_refresh_slots_009ef230(int counter) noexcept
{
    ShipAiSectorRefreshSlots out;
    // 009EF2C9 CMP ECX,2 and 009EF2CE SETGE DL.
    const int group = (counter >= 2) ? 1 : 0;
    // 009EF2D1 AND EAX,0x80000001 with the 009EF2DC..009EF2E2 sign fixup is the
    // signed remainder, and 009EF2E3..009EF2E7 folds it to 0 or 1.
    const int parity = ((counter % 2) != 0) ? 1 : 0;
    // 009EF308 LEA ECX,[EDX+EDX*2]; 009EF30B LEA EDX,[EAX+ECX*2].
    const int start = parity + group * kShipAiObstacleGroupSize;
    // 009EF312..009EF323: ((5 - parity) >> 1) + 1 iterations, 009EF339 steps two
    // strides at a time.
    for (int i = 0; i < 3; ++i) {
        out.sector[static_cast<std::size_t>(i)] = start + 2 * i;
    }
    // 009EF2E9..009EF2FD.
    out.next_counter = (counter < 3) ? (counter + 1) : 0;
    return out;
}

float ship_ai_sector_braking_distance_009ef230(float body_axis_speed,
                                               float class_top_speed_500,
                                               float class_deceleration_508,
                                               float hull_half_length_9c8) noexcept
{
    // 009EF25D, the double at 00D7A3A0.
    const float floor_speed = static_cast<float>(static_cast<double>(class_top_speed_500) * 0.1);
    // 009EF26F FCOMIP with 009EF273 JBE: the larger of the two.
    const float speed = (body_axis_speed <= floor_speed) ? floor_speed : body_axis_speed;
    // 009EF28F, the double at 00D7A2B0.
    const float raised = static_cast<float>(static_cast<double>(speed) + 3.0);
    // 009EF29F FDIV, 009EF2A7 FMULP, 009EF2A9 FMUL against 00CEC8F0.
    const double stopping = (static_cast<double>(raised) / static_cast<double>(class_deceleration_508))
                            * static_cast<double>(raised) * 0.55;
    const float stopping_f = static_cast<float>(stopping);
    // 009EF2C3 FMUL against 00CEFF98, 009EF2D6 FADDP.
    return static_cast<float>(static_cast<double>(hull_half_length_9c8) * 0.6
                              + static_cast<double>(stopping_f));
}

// ---------------------------------------------------------------------------
// 009D84E0
// ---------------------------------------------------------------------------
ShipAiSectorPassChoice ship_ai_node_passing_corner_009d84e0(
    const ShipAiObstacleNode& node,
    float observer_x, float observer_z,
    float reference_bearing,
    int other_side) noexcept
{
    ShipAiSectorPassChoice out;
    if (node.no_pose_68) {
        // 009D84E9..009D84FB: no box, the centre is the answer and the flag is
        // left as the caller set it (the caller cleared it at 009EBEC1).
        out.corner_x = node.avoid_box_x;
        out.corner_z = node.avoid_box_z;
        return out;
    }

    // 009D8509..009D871B: the four corners of the avoid box, in the order
    // (+beam +forward), (+beam -forward), (-beam -forward), (-beam +forward).
    const float beam = node.avoid_half_beam;      // +5Ch
    const float length = node.avoid_half_length;  // +60h
    Vec2 corner[4];
    const float signs_beam[4] = {1.0f, 1.0f, -1.0f, -1.0f};
    const float signs_forward[4] = {1.0f, -1.0f, -1.0f, 1.0f};
    for (int i = 0; i < 4; ++i) {
        corner[i].x = node.avoid_box_x + signs_beam[i] * beam * node.corner_beam_x
                      + signs_forward[i] * length * node.corner_forward_x;
        corner[i].z = node.avoid_box_z + signs_beam[i] * beam * node.corner_beam_z
                      + signs_forward[i] * length * node.corner_forward_z;
    }

    // 009D861E and 009D863F: the seeds are -pi and +pi.
    float widest_left = -kShipAiObstacleHalfTurn;  // 00CE684C
    float widest_right = kShipAiObstacleHalfTurn;  // 00D7A264
    int left_index = 0;                            // EBX
    int right_index = 0;                           // EDI
    for (int i = 0; i < 4; ++i) {
        // 009D8720..009D8780: the corner's bearing from the observer, relative
        // to the reference.
        const float bearing = ship_ai_sector_bearing_009ebfa1(corner[i].x - observer_x,
                                                              corner[i].z - observer_z);
        const float relative = wrapped_angle_subtract_00438b10(bearing, reference_bearing);
        if (relative > widest_left) { // 009D8792 FCOMI, 009D8796 JBE
            widest_left = relative;
            left_index = i;
        }
        if (relative < widest_right) { // 009D87A4
            widest_right = relative;
            right_index = i;
        }
    }

    // 009D87CA..009D8819: the two turns, with 25 degrees added to whichever the
    // other party already claimed.
    double turn_left = static_cast<double>(clear_sign(widest_left));
    double turn_right = static_cast<double>(clear_sign(widest_right));
    if (other_side == 1) {
        turn_left += static_cast<double>(kShipAiSectorSideBias);
    } else if (other_side == 2) {
        turn_right += static_cast<double>(kShipAiSectorSideBias);
    }

    // 009D8825 FCOMIP, 009D8829 JC: the strictly smaller right turn wins.
    if (static_cast<float>(turn_right) < static_cast<float>(turn_left)) {
        out.take_max = false;
        out.corner_x = corner[right_index].x;
        out.corner_z = corner[right_index].z;
    } else {
        out.take_max = true;
        out.corner_x = corner[left_index].x;
        out.corner_z = corner[left_index].z;
    }
    return out;
}

// ---------------------------------------------------------------------------
// 009EB660's probe geometry
// ---------------------------------------------------------------------------
ShipAiSectorProbe ship_ai_sector_probe_009eb660(const ShipAiObstacleSector& sector,
                                                const ShipAiSectorHullPose& pose,
                                                float margin) noexcept
{
    ShipAiSectorProbe probe;
    probe.ahead = sector.kind != 0;                       // 009EB6B4, 009EB9B3
    probe.radius = sector.half_width;                     // the turn radius
    probe.is_arc = !(sector.half_width < kShipAiSectorLateralSignPivot); // 009EB69F/009EB6AE

    const Vec2 hull{pose.x, pose.z};
    const float lateral = sector.lateral;                 // 009EB6B7, 009EB9BC
    const float clearance = sector.reach;                 // 009EB7B1, 009EBA7F

    if (!probe.is_arc) {
        // 009EB6B4..009EB790. One beam vector only, blk+19Ch/+1A0h, with the
        // sign of the offset and of the direction both taken from the kind byte.
        const Vec2 origin = probe.ahead
            ? scaled_add(hull, lateral, pose.port_x, pose.port_z)      // 009EB6ED..009EB71F
            : scaled_add(hull, -lateral, pose.port_x, pose.port_z);    // 009EB725..009EB774
        probe.origin_x = origin.x;
        probe.origin_z = origin.z;
        probe.direction_x = probe.ahead ? pose.forward_x
                                        : (kShipAiSectorNegateZero - pose.forward_x);
        probe.direction_z = probe.ahead ? pose.forward_z
                                        : (kShipAiSectorNegateZero - pose.forward_z);
        // 009EB783..009EB790 and 009EB871..009EB89B.
        probe.range = sector.braking_distance + margin;
        probe.probe_x = origin.x + clearance * probe.direction_x;
        probe.probe_z = origin.z + clearance * probe.direction_z;
        return probe;
    }

    // 009EB92E..009EBD48, the arc. The two beam bearings the turn centre is
    // described from.
    const float right_bearing = wrapped_angle_subtract_00438b10(pose.heading,
                                                                kShipAiSectorBeamOffset);
    const float left_bearing = wrapped_angle_add_00438aa0(pose.heading,
                                                          kShipAiSectorBeamOffset);
    // 009EB980..009EB9B6: the swept arc is at most half a circle, and at most
    // the braking range.
    const float radius = sector.half_width;
    const float two_radius = radius + radius;
    const float limit = sector.braking_distance + margin;
    const float arc_length = (two_radius <= limit) ? two_radius : limit;
    probe.range = arc_length;

    // 009EB9E1 / 009EBB9B COMISS against +0.0f with JC: the sign of the lateral
    // offset picks the beam, and the kind byte flips which one.
    const bool offset_positive = !(lateral < kShipAiSectorLateralSignPivot);
    const bool use_port = (probe.ahead == offset_positive);
    const float side_x = use_port ? pose.port_x : pose.starboard_x;
    const float side_z = use_port ? pose.port_z : pose.starboard_z;
    probe.reference_bearing = use_port ? left_bearing : right_bearing;
    const bool sweep_positive = !offset_positive;

    // 009EBA11..009EBA22 and the three siblings: the start point sits |lateral|
    // off the beam and the centre one radius further out.
    const float offset = offset_positive ? lateral : -lateral;
    const Vec2 start = scaled_add(hull, offset, side_x, side_z);
    const Vec2 centre = scaled_add(hull, offset + radius, side_x, side_z);
    probe.centre_x = centre.x;
    probe.centre_z = centre.z;

    // 009EBA7F..009EBA8E and the siblings: the clearance becomes an angle, and
    // the arc length becomes the swept angle.
    const float half_angle = sweep_positive ? (clearance / radius) : -(clearance / radius);
    probe.half_angle = half_angle;
    const float theta = arc_length / radius;
    probe.swept_bearing = sweep_positive
        ? wrapped_angle_add_00438aa0(probe.reference_bearing, theta)
        : wrapped_angle_subtract_00438b10(probe.reference_bearing, theta);

    // 009EBB61..009EBB96 adds the clearance along the hull forward vector,
    // 009EBD17..009EBD48 subtracts it.
    const float along = probe.ahead ? clearance : -clearance;
    probe.probe_x = start.x + along * pose.forward_x;
    probe.probe_z = start.z + along * pose.forward_z;
    return probe;
}

// ---------------------------------------------------------------------------
// 009EB660
// ---------------------------------------------------------------------------
ShipAiSectorScanResult ship_ai_scan_obstacle_sector_009eb660(
    ShipAiObstacleSector& sector,
    const ShipAiSectorScanInputs& inputs,
    const std::vector<ShipAiObstacleNode*>& neighbours,
    ShipAiSectorScanHost& host)
{
    ShipAiSectorScanResult result;

    // 009EB66E..009EB696: a sector that was blocked last frame keeps looking
    // one settings field further, which is the hysteresis on the latch.
    const float margin = sector.blocked ? host.settings_blocked_margin_1d8() : 0.0f;
    // 009EB6A3, 009EB6A7.
    sector.blocked = false;
    sector.blocker = nullptr;

    ShipAiSectorHullPose pose = inputs.pose;
    if (!(sector.half_width < kShipAiSectorLateralSignPivot)) {
        // 009EB939: the arc arm reads the heading before anything else.
        pose.heading = host.unit_heading_vtable50();
    }
    ShipAiSectorProbe probe = ship_ai_sector_probe_009eb660(sector, pose, margin);
    const float clearance = sector.reach;
    const std::array<float, 2> probe_point{probe.probe_x, probe.probe_z};
    const std::array<float, 2> hull_point{pose.x, pose.z};

    int blocking = -1;      // [ESP+18h], 009EB677 seeds it with -1
    bool zone_hit = false;  // [ESP+0Eh], 009EB672

    if (!probe.is_arc) {
        const std::array<float, 2> origin{probe.origin_x, probe.origin_z};
        const std::array<float, 2> direction{probe.direction_x, probe.direction_z};
        float range = probe.range;

        if (inputs.avoid_zones_present) { // 009EB778
            const std::array<float, 2> far_point{origin[0] + range * direction[0],
                                                 origin[1] + range * direction[1]};
            std::array<float, 2> hit{far_point};
            if (inputs.avoid_zones_enabled // 009EB7AA
                && host.avoid_zone_segment_crossing_004158e0(probe_point, far_point, hit)) {
                zone_hit = true; // 009EB833
                // 009EB827..009EB84D.
                range = length_2d_00414c60({hit[0] - origin[0], hit[1] - origin[1]});
            }
        }

        // 009EB85B..009EB91E.
        for (std::size_t i = 0; i < neighbours.size(); ++i) {
            const ShipAiObstacleNode& node = *neighbours[i];
            if (host.point_in_avoid_box_009d8160(node, probe_point)) { // 009EB8C0
                blocking = static_cast<int>(i);                        // 009EB925
                break;
            }
            if (host.point_in_near_box_009d80c0(node, hull_point)) { // 009EB8CA
                continue;
            }
            float candidate = range;
            if (host.clip_ray_against_node_009dd540(*neighbours[i], origin, direction, candidate)
                && candidate > clearance) { // 009EB8EB..009EB8F6
                range = candidate;
                blocking = static_cast<int>(i); // 009EB8F8
            }
        }
    } else {
        const std::array<float, 2> centre{probe.centre_x, probe.centre_z};
        float swept = probe.swept_bearing;

        if (inputs.avoid_zones_present) { // 009EBD4C
            // 009EBD59..009EBD6B: the arc starts one clearance angle off the
            // reference bearing.
            const float start_bearing =
                wrapped_angle_add_00438aa0(probe.reference_bearing, probe.half_angle);
            if (inputs.avoid_zones_enabled // 009EBD70
                && host.clip_arc_against_avoid_zones_00415970(centre, probe.radius,
                                                              start_bearing, swept)) {
                zone_hit = true; // 009EBDB8
            }
        }

        // 009EBDBD..009EBEAB.
        for (std::size_t i = 0; i < neighbours.size(); ++i) {
            ShipAiObstacleNode& node = *neighbours[i];
            if (node.owner == nullptr || node.owner_gone_5e) { // 009EBDE2, 009EBDFB
                continue;
            }
            if (host.point_in_avoid_box_009d8160(node, probe_point)) { // 009EBE0E
                blocking = static_cast<int>(i);                        // 009EBEB3
                break;
            }
            float candidate = swept;
            if (host.clip_arc_against_node_009dd010(node, centre, probe.radius,
                                                    probe.reference_bearing, candidate)) {
                // 009EBE3F..009EBE7D: the arc length to the hit, against the
                // sector's clearance.
                const float travelled =
                    clear_sign(wrapped_angle_subtract_00438b10(candidate,
                                                               probe.reference_bearing))
                    * probe.radius;
                if (travelled > clearance) {
                    swept = candidate;
                    blocking = static_cast<int>(i); // 009EBE7F
                }
            }
        }
    }

    result.avoid_zone_hit = zone_hit;
    result.blocking_node = blocking;

    bool take_max = false; // [ESP+0Fh], cleared at 009EBEC1
    if (blocking >= 0) {
        // 009EBECC..009EBFDF, the unit arm.
        sector.blocked = true;
        ShipAiObstacleNode& node = *neighbours[static_cast<std::size_t>(blocking)];
        // 009EBEDE..009EBF0A: the blocking node is kept alive a little longer.
        host.raise_node_lifetime_78(
            node,
            static_cast<float>(static_cast<double>(host.settings_neighbour_memory_194())
                               + kShipAiSectorBlockerLifetimeBonus));
        // 009EBF0F..009EBF3E: the astern fan reasons about the reversed heading.
        float reference = host.unit_heading_vtable50();
        if (sector.kind == 0) {
            reference = wrapped_angle_add_00438aa0(reference, kShipAiObstacleHalfTurn);
        }
        const ShipAiSectorPassChoice choice =
            ship_ai_node_passing_corner_009d84e0(node, pose.x, pose.z, reference,
                                                 node.pass_side_88); // 009EBF67
        take_max = choice.take_max;
        sector.hit_x = choice.corner_x;                     // 009EBF6E
        sector.hit_z = choice.corner_z;                     // 009EBF7A
        sector.pass_side = take_max ? 2 : 1;                // 009EBF83
        sector.avoid_bearing = ship_ai_sector_bearing_009ebfa1(sector.hit_x - pose.x,
                                                               sector.hit_z - pose.z);
    } else if (zone_hit) {
        // 009EBFE4..009EC1A0, the avoid-zone arm.
        sector.blocked = true;
        float reference = host.unit_heading_vtable50(); // 009EBFFE
        if (sector.kind == 0) {
            reference = wrapped_angle_add_00438aa0(reference, kShipAiObstacleHalfTurn);
        }
        const std::array<float, 2> ahead = ship_ai_sector_direction_009ec056(reference);
        ShipAiSectorFreeBearingQuery query;
        query.origin_x = pose.x;
        query.origin_z = pose.z;
        query.direction_x = ahead[0];
        query.direction_z = ahead[1];
        query.range = sector.braking_distance + margin; // 009EC070
        if (host.avoid_zone_free_bearing_009dc2e0(query)) { // 009EC0C1
            sector.avoid_bearing = query.bearing;           // 009EC0DE
            const std::array<float, 2> clear = ship_ai_sector_direction_009ec056(query.bearing);
            sector.hit_x = pose.x + query.range * clear[0]; // 009EC135..009EC151
            sector.hit_z = pose.z + query.range * clear[1];
        } else {
            // 009EC15D..009EC1A3: straight on at the full range.
            sector.avoid_bearing = reference;
            sector.hit_x = pose.x + query.range * ahead[0];
            sector.hit_z = pose.z + query.range * ahead[1];
        }
    } else {
        return result; // 009EBFE9
    }

    result.blocked = sector.blocked;
    result.pass_on_max_side = take_max;

    // 009EC1A8..009EC26C: bias the bearing away from the obstacle, by less the
    // further away it is, and toward the side 009D84E0 chose.
    const float distance = planar_length(sector.hit_x - pose.x, sector.hit_z - pose.z);
    const float bias = clamped_interpolate_00419010(kShipAiSectorBiasNearDistance, 0.0f,
                                                    kShipAiSectorBiasFarDistance,
                                                    kShipAiSectorBiasFarAngle, distance);
    sector.avoid_bearing = take_max
        ? wrapped_angle_add_00438aa0(sector.avoid_bearing, bias)      // 009EC256
        : wrapped_angle_subtract_00438b10(sector.avoid_bearing, bias); // 009EC267
    return result;
}

// ---------------------------------------------------------------------------
// The neighbour list
// ---------------------------------------------------------------------------
ShipAiNeighbourAddResult ship_ai_neighbour_list_add_009f0d20(
    std::vector<ShipAiObstacleNode*>& list,
    const void* candidate_owner,
    ShipAiObstacleNode* fresh,
    float now_plus_memory) noexcept
{
    ShipAiNeighbourAddResult out;
    if (list.size() >= static_cast<std::size_t>(kShipAiNeighbourListCapacity)) {
        out.rejected = true; // 009F0D39, 009F0D43
        return out;
    }
    // 009F0DDB..009F0E1D: an existing node for the same owner is refreshed and
    // nothing is appended.
    bool already_listed = false; // BL, 009F0DC6 sets it, 009F0E0F clears it
    for (ShipAiObstacleNode* node : list) {
        if (node->owner == candidate_owner) { // 009F0DFA
            if (now_plus_memory > node->lifetime_78) { // 009F0E04
                node->lifetime_78 = now_plus_memory;   // 009F0E0A
            }
            already_listed = true;
        }
    }
    if (already_listed) {
        out.refreshed = true;
        return out;
    }
    // 009F0E2A..009F0E69: operator new(0x90), 009E52E0, then the append.
    if (fresh != nullptr) {
        fresh->owner = candidate_owner;
        fresh->lifetime_78 = now_plus_memory;
    }
    list.push_back(fresh);
    out.appended = true;
    return out;
}

ShipAiNeighbourRefreshResult ship_ai_neighbour_list_refresh_009f0ea0(
    std::vector<ShipAiObstacleNode*>& list,
    float dt,
    std::vector<ShipAiObstacleNode*>& expired) noexcept
{
    ShipAiNeighbourRefreshResult out;
    std::size_t write = 0;
    for (std::size_t read = 0; read < list.size(); ++read) {
        ShipAiObstacleNode* node = list[read];
        // 009F1009..009F1018: the store happens before the test, so an expired
        // node is freed carrying its negative remainder.
        node->lifetime_78 -= dt;
        if (!(node->lifetime_78 > 0.0f)) { // 009F101F FCOMIP, 009F1023 JBE
            expired.push_back(node);       // 009F1121, 009F1127
            ++out.removed;                 // 009F112F, inside the free gap
            continue;
        }
        if (node->owner == nullptr || node->owner_gone_5e) { // 009F1029, 009F1034
            node->no_pose_68 = true;                        // 009F110A
        }
        // 009F1112..009F1116: the survivors close the gap the removals left.
        list[write] = node;
        ++write;
    }
    list.resize(write); // 009F114E SUB [ESI+604h],EBP
    out.survivors = static_cast<int>(write);
    return out;
}

float ship_ai_neighbour_admission_radius_009f1987(float self_half_length_9c8,
                                                  float other_half_length_9c8,
                                                  float self_class_top_speed_500,
                                                  float other_class_top_speed_500,
                                                  float settings_lookahead_seconds_19c,
                                                  float settings_min_range_198) noexcept
{
    // 009F1987..009F1999, the double at 00D7A280.
    const float hull = static_cast<float>((static_cast<double>(other_half_length_9c8)
                                           + static_cast<double>(self_half_length_9c8)) * 0.5);
    // 009F19A2..009F19B4.
    const float closing = (other_class_top_speed_500 + self_class_top_speed_500)
                          * settings_lookahead_seconds_19c;
    // 009F19CF FCOMIP, 009F19D3 JBE: the larger of the two.
    const float reach = (settings_min_range_198 <= closing) ? closing : settings_min_range_198;
    // 009F19E3..009F19F1.
    return hull + reach;
}

} // namespace bsp
