// The ship AI `follow` and `land` state steps, 009E1610 and 009E1950.
//
// Packet cc_ai_follow_land, worker agent/cc-ai-follow-land.
// Project C:/Users/sqz269/bsp.gpr, program /battlestationspacific.exe; Ghidra
// was read-only for this packet. Every descriptive name here is a hypothesis,
// not a recovered symbol. docs/SHIP_AI_FOLLOW_LAND.md carries the evidence.
//
// This header builds on bsp/ship_ai_states.hpp (ShipAiControlBlock, the three
// setters, ShipAiSteeringMode, ShipAiThrottleDirection, the vtable slot ids)
// and bsp/ship_ai_state_steps.hpp (ShipAiGoalPlan and 009DE050's own rule) and
// redefines nothing from either. `blk` is brain+8h throughout; a field quoted
// relative to `brain` carries its blk-relative offset next to it.
#pragma once

#include <cstdint>

#include "bsp/ship_ai_state_steps.hpp"
#include "bsp/ship_ai_states.hpp"

namespace bsp {

// A world-plane point. The image keeps the AI's goals as (x, z) pairs and
// drops y; the two step routines here never read a y.
struct ShipAiFollowLandXZ {
    float x{0.0f};
    float z{0.0f};
};

// ---------------------------------------------------------------------------
// 009E1610, the `follow` step
// ---------------------------------------------------------------------------
//
// __thiscall(state)(float seconds), RET 4 at 009E18C0, body 009E1610-009E18C2,
// complete. vtable 00D215F8 slot +0Ch, state object brain+0B8Ch. The `seconds`
// argument is never read: the step is scalar in the state and the leader.
//
// It runs only while the unit is in a formation (`unit+284h`) whose leader
// (`+14h`) answers `vtable[5Ch](6)`. It does not read the command object at
// 00E08F60: the station offset comes from the formation record, not the order.

// The kind the leader must answer for the step to run at all. The literal 6 the
// site at 009E163E pushes into leader->vtable[5Ch].
inline constexpr int kShipAiFollowLeaderKind = 6;
// The leader speed at or below which the "making way" latch clears. 00D7A260,
// a float32, compared against 0092D730's x87 result at 009E1663.
inline constexpr float kShipAiFollowAsternSpeed = -1.0f;
// The turn radius is scaled by this before the station hysteresis. 00CE3DE0,
// a double; 0082E850's float result is widened for the multiply at 009E16F9.
inline constexpr double kShipAiFollowStationRadiusScale = 2.5;
// The sideways back-off is this many turn radii. 00CE3D78, a double, at
// 009E17BA. 009DF2D0 uses the same constant at its own two offsets.
inline constexpr double kShipAiFollowBackOffScale = 1.5;
// Subtracted from the along-track dot before the min. 00D7A208, a float32
// negative zero, so the expression is exactly -dot (009E1799, SUBSS).
inline constexpr float kShipAiFollowBackOffBias = -0.0f;
// The published lateral tolerance is the state's stored radius times this.
// 00CF87C0, a double, at 009E1873.
inline constexpr double kShipAiFollowRequestRadiusScale = 1.25;
// 009DF2D0 seeds the stored radius with the hull radius times this. 00D7A280,
// a double, at 009DF33A.
inline constexpr double kShipAiFollowSlotRadiusScale = 0.5;

// The follow state object, brain+0B8Ch..brain+0BE0h. Its producer is the brain
// constructor 009F39C0: vtable at 009F3A42, owner at 009F3A3C, `making_way`
// **set** at 009F3A4C and `out_of_station` **cleared** at 009F3A53. The rest is
// written by 009DF2D0 before the step reads it.
struct ShipAiFollowState {
    // +8h, the out-of-station latch. A byte, not one of the two -99.0f floats
    // the cruise state carries at the same offsets: 009F3A53 writes byte 0.
    bool out_of_station_08{false};
    // +0Ch / +10h, the formation slot point 0070D290 returns for this unit.
    // 009DF2D0 writes them at 009DF315 and 009DF320; the step never reads them.
    ShipAiFollowLandXZ slot_point_0c{};
    // +14h / +18h, the point handed to 009DE050. 009DF2D0 sets it to the slot
    // point offset by `radius_30` along the slot direction, pushed out of the
    // zone set; this step may back it off sideways again (009E180E, 009E181D).
    ShipAiFollowLandXZ station_point_14{};
    // +1Ch / +20h, the slot direction, unit length. 009DF2D0 takes it from
    // 0070D290 and re-derives it, through 00419260 at 009DF533 and 009DF571,
    // from the two points it pushes out of the zone set when they part by more
    // than 1.0; the store is at 009DF556.
    ShipAiFollowLandXZ slot_direction_1c{};
    // +24h, the slot heading, wrap_2pi(pi/2 - atan2(dz, dx)). The CRT call is
    // at 009DF5AA, the pi/2 at 009DF5B7, the wrap at 009DF5CB, the store at
    // 009DF5DF.
    float slot_heading_24{0.0f};
    // +28h, the leader's body-axis speed (0092D730 on leader+1018h at
    // 009DF359), scaled again by 009DF2D0's own throttle blend at 009DF65B.
    float leader_speed_28{0.0f};
    // +2Ch, the "leader is making way" latch. Constructed set (009F3A4C).
    bool making_way_2c{true};
    // +30h, the station radius: hull radius (unit+9C8h) * 0.5 (009DF33A).
    float radius_30{0.0f};
};

// The 1Bh-byte request 009E1610 builds on its stack and 009DA3B0 copies to
// blk+38Ch..+3A6h. Field order is the copy order in 009DA3B0's body; the
// offsets are the request's own, and the blk offset of each is in the comment.
struct ShipAiStationRequest {
    float direction_x{0.0f};  // +0h  -> blk+38Ch, the frame 009EDAD0 projects on
    float direction_z{0.0f};  // +4h  -> blk+390h
    float heading{0.0f};      // +8h  -> blk+394h, the heading error's reference
    float leader_speed{0.0f}; // +0Ch -> blk+398h
    float unused_10{0.0f};    // +10h -> blk+39Ch, zeroed on both arms
    float radius{0.0f};       // +14h -> blk+3A0h
    bool making_way{true};    // +18h -> blk+3A4h
    bool enable{true};        // +19h -> blk+3A5h, the arm's first gate
    bool suppress{false};     // +1Ah -> blk+3A6h, the arm's second gate
};

// One pure virtual per native call site of 009E1610, in call order.
struct ShipAiFollowStepHost {
    virtual ~ShipAiFollowStepHost() = default;

    // 009E1642, leader->vtable[5Ch](6) with ECX = [[unit+284h]+14h]. The callee
    // is an indirect kind test; its body was not read, so the contract is the
    // literal 6 the site pushes. contract: unread.
    virtual bool leader_matches_kind_5c(int kind) = 0;

    // 009E165E and 009E1675, 0092D730 BSP_UnitController_GetBodyAxisSpeed with
    // ECX = [leader+1018h]: the **leader's** controller, not this ship's.
    virtual float leader_body_axis_speed_0092d730() = 0;

    // 009E1689, 009DF2D0 with ECX = state. It refills every field of
    // ShipAiFollowState except the two latches. Projected as a host method
    // because its own tail (the speed match through 0070D100) is not projected;
    // see docs/SHIP_AI_FOLLOW_LAND.md, coverage `partial`.
    virtual void update_formation_point_009df2d0(ShipAiFollowState& state) = 0;

    // 009E16A2, 00414DB0 BSP_EntityPose_RefreshWorld with ECX = unit, guarded
    // by the byte at unit+0C8h.
    virtual void refresh_world_pose_00414db0() = 0;

    // 009E16A7 and 009E16BA, the unit's world position at unit+0FCh / +104h.
    virtual ShipAiFollowLandXZ unit_position() = 0;

    // 009E16F0 and 009E1790, 0082E850 BSP_ShipClass_GetTurnRadius with
    // ECX = brain+0AACh, the ship class descriptor cached next to the unit.
    virtual float ship_class_turn_radius_0082e850() = 0;

    // 009E17B5, 00415510 BSP_Math_MinFloatByRef(ECX = &a, EDX = &b).
    virtual float min_float_by_ref_00415510(float a, float b) = 0;

    // 009E1837, 009DE050(blk, &state+14h, keep_mode = 0, final_leg = 1) with
    // ECX = brain+8h. The rule itself is ship_ai_set_navigation_goal_009de050.
    virtual void set_navigation_goal_009de050(const ShipAiFollowLandXZ& goal,
                                              bool keep_mode, bool final_leg) = 0;

    // 009E18B6, 009DA3B0(blk, &request) with ECX = brain+8h. The body is the
    // nineteen-byte copy in ship_ai_publish_station_request_009da3b0.
    virtual void publish_station_request_009da3b0(const ShipAiStationRequest& request) = 0;
};

// The "leader is making way" latch, 009E164C..009E1684. Enters on a leader
// speed at or above zero, leaves at or below -1.0f, so a leader backing down
// flips the whole formation to the mirrored slot offset in 009DF2D0.
bool ship_ai_follow_making_way_latch(bool latched, float leader_speed);

// The out-of-station latch, 009E16F5..009E172D. `r` is the scaled turn radius;
// the gate enters above 2r^2 and leaves below r^2, a hysteresis of sqrt(2) on
// distance. Both compares are float32 (009E1717, 009E1727).
bool ship_ai_follow_station_latch(bool latched, float distance_sq, float turn_radius);

// The along-track projection of the station point relative to the ship,
// 009E174C..009E1773. Negative means the station point is behind the ship along
// the slot direction, which is the only case the back-off runs in.
float ship_ai_follow_station_dot(const ShipAiFollowLandXZ& station_point,
                                 const ShipAiFollowLandXZ& slot_direction,
                                 const ShipAiFollowLandXZ& unit_position);

// The sideways back-off, 009E17CE..009E182A. The station point moves along the
// slot direction's left normal (-dz, dx) by `offset`, which the step builds as
// min(-dot, turn_radius) * 1.5 through 00415510. The side is chosen by the
// cross product of that normal with the ship-to-station vector, so the point
// always swings away from the ship rather than through it.
ShipAiFollowLandXZ ship_ai_follow_back_off(const ShipAiFollowLandXZ& station_point,
                                           const ShipAiFollowLandXZ& slot_direction,
                                           const ShipAiFollowLandXZ& unit_position,
                                           float offset);

// The request build, 009E183C..009E18A5. In station it publishes the slot frame
// and lets the station-keeping arm run; out of station it publishes zeros with
// `suppress` set, which shuts the arm off (docs/SHIP_AI_GOAL_VECTOR.md: the arm
// at 009EDA28 needs blk+3A5h set and blk+3A6h clear).
ShipAiStationRequest ship_ai_follow_build_request(const ShipAiFollowState& state);

// 009DA3B0, __thiscall(blk)(const request*), body 009DA3B0-009DA408, complete.
// Six dwords and three bytes, straight through, no test.
void ship_ai_publish_station_request_009da3b0(const ShipAiStationRequest& request,
                                              ShipAiStationRequest& blk_038c);

// The whole step. `state` is brain+0B8Ch; everything else goes through `host`.
void ship_ai_follow_step_009e1610(ShipAiFollowState& state, ShipAiFollowStepHost& host);

// ---------------------------------------------------------------------------
// 009DA610, the "this goal is the one we already arrived at" test
// ---------------------------------------------------------------------------
//
// __thiscall(blk)(const float* goal2d), RET 4, body 009DA610-009DA671,
// complete. Its three callers are the land step at 009E1F83, the kamikaze step
// 009E2020 and the attackmove engage sub-state 009E23B0.
// It answers true only while blk+2FDh is latched and the offered
// goal is within 50 units of the latched goal; any further goal clears the
// latch and answers false. The radius is the same 00D20278 that 009DE050 uses
// to drop the crossing state, so the two agree by construction.
bool ship_ai_nav_goal_already_reached_009da610(ShipAiGoalPlan& blk,
                                               const ShipAiFollowLandXZ& goal);

// ---------------------------------------------------------------------------
// 009E1950, the `land` step
// ---------------------------------------------------------------------------
//
// __thiscall(state)(float seconds), RET 4 at 009E1E54 / 009E1FFB / 009E2018,
// body 009E1950-009E201A, complete. vtable 00D21658 slot +0Ch, state object
// brain+0BE0h. Unlike `follow` it does read `seconds`, twice, as a countdown.
//
// The target is a **landing pad entity**, not a position in the command: the
// unit carries one at unit+1200h, the pad carries its occupant at +1F8h and its
// owning base at +220h, and the base carries a pad vector at +794h / +798h that
// 006F2E60 searches. The approach geometry is the pad's own, clipped against
// the ship class's avoid-zone group by 006AC5D0.

// The kinds the two indirect tests demand: the unit at 009E19BE and the base at
// 009E1A88, both literals pushed into vtable[5Ch].
inline constexpr int kShipAiLandUnitKind = 0x0C;
inline constexpr int kShipAiLandBaseKind = 0x1C;
// The pad re-scan interval, uniform in [3.0f, 5.0f]. 00CE3854 and 00CE3850,
// two float32 pushed at 009E1A3F and 009E1A4E.
inline constexpr float kShipAiLandRescanMin = 3.0f;
inline constexpr float kShipAiLandRescanMax = 5.0f;
// The pad approach corridor half width handed to 006AC5D0. 00CE386C, float32,
// pushed at 009E1AF8.
inline constexpr float kShipAiLandCorridorHalfWidth = 200.0f;
// Below this squared distance the step starts testing the heading. 00D21860,
// a double (350^2); the float32 sum is widened for the compare at 009E1B8C.
inline constexpr double kShipAiLandHeadingTestDistanceSq = 122500.0;
// The heading error under which the final flag latches. 00CEDCD0, a double
// (pi/4), compared against the float32 absolute error at 009E1BFE.
inline constexpr double kShipAiLandFinalHeadingError = 0.78539818525314331;
// The zone push-out margin for the approach goal. 00CE38B8, float32, 009E1E6B.
inline constexpr float kShipAiLandZoneMargin = 10.0f;
// The arrival test radius is two turn-circle radii with this floor. 00CE3CA8
// is the double the doubled radius is compared against and 00CE3AE8 the float32
// substituted for it (009E1F30, 009E1F3C).
inline constexpr double kShipAiLandArrivalRadiusFloorCompare = 300.0;
inline constexpr float kShipAiLandArrivalRadiusFloor = 300.0f;
// The unconditional arrival distance, squared. 00CE3968, float32, 009E1F8C.
inline constexpr float kShipAiLandArrivalDistanceSq = 12000.0f;
// Beyond this squared distance to the landing point the held heading is NOT
// latched, so the step keeps re-aiming. 00CE3D64, float32, 009E1CB8.
inline constexpr float kShipAiLandHoldHeadingDistanceSq = 10000.0f;
// The held-heading carrot is this far ahead. 00D7A220, a double, 009E1CE7.
inline constexpr double kShipAiLandHoldCarrotDistance = 100.0;
// The rescan timer parked on the final arm. 00D059A0, float32, 009E1C1B.
inline constexpr float kShipAiLandRescanParked = 99.0f;
// The final arm's throttle ramp: full ahead under pi/12 of heading error, half
// at pi/4 and beyond. 00D05AA8, 00CEB5A8 and 00CE3800, pushed at 009E1DE6,
// 009E1DD6 and 009E1DC6.
inline constexpr float kShipAiLandThrottleErrorLow = 0.26179939f;   // pi/12
inline constexpr float kShipAiLandThrottleErrorHigh = 0.78539819f;  // pi/4
inline constexpr float kShipAiLandThrottleHigh = 1.0f;
inline constexpr float kShipAiLandThrottleLow = 0.5f;
// The speed-scale ramp on the approach arm, interp(-1, 1, 0, 0, timer).
inline constexpr float kShipAiLandSpeedRampEnd = -1.0f;  // 00D7A260
inline constexpr float kShipAiLandSpeedRampFull = 1.0f;  // 00D7A24C

// The land state object, brain+0BE0h..brain+0C04h. Producer: 009F39C0 writes
// the vtable (009F3A6B), the owner (009F3A68), `rescan_timer` = 1.0f
// (009F3A72), `final` = 0 (009F3A77) and `hold_heading` = 0 (009F3A7B). It does
// **not** write +8h; see the uncertainty in docs/SHIP_AI_FOLLOW_LAND.md.
struct ShipAiLandState {
    // +8h, the speed-ramp countdown. Only 009E1FC6 writes it after
    // construction; at or below -1.0f the ramp is finished and the step leaves
    // through 009E1FFE with the speed scale pinned to 1.0f.
    float speed_ramp_08{0.0f};
    // +0Ch, seconds until the next pad re-scan. 1.0f at construction, refilled
    // uniform in [3, 5] at 009E1A58, forced to -1.0f when the pad was stolen
    // (009E1A02), parked at 99.0f once the final arm is entered (009E1C23).
    float rescan_timer_0c{1.0f};
    // +10h, the final flag: the ship is committed to the last leg.
    bool final_10{false};
    // +11h, the held-heading flag: steer a fixed heading instead of the pad.
    bool hold_heading_11{false};
    // +14h, the held heading, wrap_2pi(pi/2 - atan2(dz, dx)) at 009E1CA3.
    float hold_heading_14{0.0f};
    // +18h / +1Ch / +20h, the landing point the step publishes on the state.
    // +1Ch is always stored as 0.0f (009E1B36, 009E1EDB, 009E1CFB): the triple
    // is (x, 0, z), not (x, z, something).
    float goal_x_18{0.0f};
    float goal_y_1c{0.0f};
    float goal_z_20{0.0f};
};

// The pad's cached approach line, pad+208h..+21Ch, as 006AC5D0 leaves it. The
// cache key is the avoid-zone group id; a new id re-runs the two ray casts in
// everything before 006AC927. The origin is the pad's own world position pulled
// back to the first avoid-zone hit along its facing, the clearance is how far
// astern of that origin the water reaches, and the run is how far ahead of it
// the water reaches, scaled by the double at 00D7A390.
struct ShipAiLandPadLine {
    int zone_group_208{0};      // +208h, the cached id
    float origin_x_20c{0.0f};   // +20Ch, the approach origin, pulled back to
    float origin_y_210{0.0f};   // +210h  the first land hit along the facing
    float origin_z_214{0.0f};   // +214h
    float astern_clearance_218{1000.0f}; // +218h, 00CE3804 before the cast
    float ahead_run_21c{800.0f};         // +21Ch, 00CE3950 before the cast
};

// One pure virtual per native call site of 009E1950, in call order.
struct ShipAiLandStepHost {
    virtual ~ShipAiLandStepHost() = default;

    // 009E1989, 009E1A67 and 009E1AEC, 00414DB0 BSP_EntityPose_RefreshWorld,
    // each guarded by the target's own byte at +0C8h: the unit twice and the
    // pad once.
    virtual void refresh_world_pose_00414db0(int entity) = 0;

    // 009E1978, 009E19A7, 009E1ACF, 009E1B97, 009E1D3A and 009E1EFD, the unit
    // at brain+0AA8h. A field read, not a call; every site reloads it.
    virtual int brain_unit_0aa8() = 0;

    // 009E195B and 009E1963, brain+0B2Ch / brain+0B34h, the AI goal vector that
    // docs/SHIP_AI_GOAL_VECTOR.md owns. It is the fallback landing point when
    // no pad is held.
    virtual ShipAiFollowLandXZ brain_goal_vector_0b2c() = 0;

    // 009E198E and 009E199F, the unit's world position at unit+0FCh / +104h.
    virtual ShipAiFollowLandXZ unit_position() = 0;

    // 009E19C2, unit->vtable[5Ch](0Ch), and 009E1A8C, base->vtable[5Ch](1Ch).
    // Both are the same indirect kind test with different literals; the callee
    // body was not read. contract: unread.
    virtual bool entity_matches_kind_5c(int entity, int kind) = 0;

    // 009E19CA, unit+1200h: the landing pad this unit holds. On a kind-8 entity
    // the same offset is a float pair (docs/SENSOR_TABLES.md); it is a pointer
    // only behind the kind-0Ch gate.
    virtual int unit_landing_pad_1200(int unit) = 0;

    // 009E19E4 and 009E19EF, 006AC220 with ECX = pad. Body 006AC220-006AC226,
    // complete: `MOV EAX,[ECX+1F8h]; RET`, the pad's occupant unit.
    virtual int pad_occupant_006ac220(int pad) = 0;

    // 009E1A51, 00BD2F10 BSP_Random_UniformFloatRange(3.0f, 5.0f).
    virtual float random_uniform_00bd2f10(float low, float high) = 0;

    // 009E1A6C, pad+220h, and 009E1A79, brain+0B20h: the two places the owning
    // base comes from, in that order.
    virtual int pad_owner_220(int pad) = 0;
    virtual int brain_landing_base_0b20() = 0;

    // 009E1A97, 006F2E60(base)(unit, 0). Body 006F2E60-006F2FA2, read: it walks
    // the pad vector at base+794h / +798h, returns the pad this unit already
    // occupies when the second argument is zero, otherwise the nearest free pad
    // by squared 3D distance.
    virtual int pick_landing_pad_006f2e60(int base, int unit, bool skip_owned) = 0;

    // 009E1AAE, 006F2FB0(base)(unit, pad). Body 006F2FB0-006F3009, read: it
    // early-outs when the pad's occupant is already this unit, then does the
    // assignment under the critical section at base+764h. Its two inner calls
    // 006F2DE0 and 006AC490 are contract: unread.
    virtual void assign_landing_pad_006f2fb0(int base, int unit, int pad) = 0;

    // 009E1AC2, blk+300h = 1.0f, written before either arm (brain+308h).
    virtual void set_blk_field_300(float value) = 0;

    // 009E1AE2, [[unit+538h]+570h], the ship class's avoid-zone group id.
    virtual int ship_class_zone_group_570() = 0;

    // 009E1B0A, 006AC5D0(pad)(&out, &unit+0FCh, zone_group, 200.0f). The pure
    // part of its body is ship_ai_land_pad_approach_point_006ac5d0; the cache
    // refresh needs the avoid-zone casts, which stay on the host.
    virtual ShipAiFollowLandXZ pad_approach_point_006ac5d0(int pad,
                                                           const ShipAiFollowLandXZ& from,
                                                           int zone_group,
                                                           float half_width) = 0;

    // 009E1B9D and 009E1C66, the CRT _CIatan2 at 00BF701A with ST1 = dz and
    // ST0 = dx, then pi/2 - result wrapped into [0, 2pi) at 009E1BAA / 009E1C73.
    virtual float heading_from_delta(float dx, float dz) = 0;

    // 009E1BD7 and 009E1D49, unit->vtable[50h](), the unit's heading. The float
    // pushed before each of these calls is 00438B10's second argument, not this
    // callee's: the callee takes none.
    virtual float unit_heading_vtable_50() = 0;

    // 009E1BDD and 009E1D4F, 00438B10 BSP_Math_SubtractWrappedAngle(heading,
    // target). The step takes the absolute value with AND 7FFFFFFFh.
    virtual float subtract_wrapped_angle_00438b10(float a, float b) = 0;

    // 009E1CE0, 006BC0C0 BSP_Geometry_HeadingToDirection(ECX = &out, heading).
    virtual ShipAiFollowLandXZ heading_to_direction_006bc0c0(float heading) = 0;

    // 009E1D9F, 009DA4E0(blk), the path-plan reset docs/SHIP_AI_STATE_STEPS.md
    // reads in full.
    virtual void clear_path_plan_009da4e0() = 0;

    // 009E1DB4, 00605070 with ECX = &blk+1D8h, so it rewrites the field the
    // previous call just set. Body 00605070-006050BD, read: fmod by 2pi through
    // 00BF857A, then +2pi at or below -pi and -2pi above +pi, so the desired
    // heading ends in (-pi, pi].
    virtual void wrap_blk_desired_heading_00605070() = 0;

    // 009E1DEF and 009E1FE6, 00419010 BSP_Math_InterpolateClamped(x0, y0, x1,
    // y1, x).
    virtual float interpolate_clamped_00419010(float x0, float y0, float x1, float y1,
                                               float x) = 0;

    // 009E1E66, 0082ADC0 with ECX = [unit+538h]. Body 0082ADC0-0082ADD7, read:
    // it looks the class's +570h up in the singleton 004218E0 returns and hands
    // the result back in EAX. **That return, not the brain, is the `this` of
    // the 00417B10 call at 009E1E83**; EAX is dead across 0082ADC0 otherwise.
    virtual int nav_zone_set_for_class_0082adc0() = 0;

    // 009E1E83, 00417B10(zone_set)(&out, &in, 10.0f, 1), RET 10h. Body
    // 00417B10-00417BF9, read: it walks the vector at set+4h / set+8h and, for
    // each zone whose bounds at +14h/+18h/+1Ch/+20h contain the point and whose
    // 00416B50 agrees, replaces the point through 00416F30 with the margin.
    // 00416B50 and 00416F30 are contract: unread.
    virtual ShipAiFollowLandXZ push_point_out_of_zones_00417b10(
        int zone_set, const ShipAiFollowLandXZ& point, float margin, bool use_bounds) = 0;

    // 009E1EA2 and 009E1C28, blk+3F4h (brain+3FCh); 009E1EC2 and 009E1C32,
    // blk+3F0h (brain+3F8h). The approach arm sets 1 and 3, the final arm 0 and
    // -1. Their readers were not searched for. contract: unread.
    virtual void set_blk_flag_3f4(bool value) = 0;
    virtual void set_blk_field_3f0(int value) = 0;

    // 009E1EB6, 009DE050(blk, &goal, keep_mode = 0, final_leg = 0) with
    // ECX = brain+8h.
    virtual void set_navigation_goal_009de050(const ShipAiFollowLandXZ& goal,
                                              bool keep_mode, bool final_leg) = 0;

    // 009E1F21, 00811A30(unit)(1.0f). Body 00811A30-00811AAA, read: the ship
    // class's turn-circle radius at that throttle, divided by the gameplay
    // modifier at index 5 when the modifier table is live.
    virtual float turn_circle_radius_00811a30(float throttle) = 0;

    // 009E1F72, state->vtable[2Ch](&goal): the state family's own arrival test,
    // kShipAiStateVtableReached. The land state's slot body was not read.
    // contract: unread.
    virtual bool state_reached_vtable_2c(const ShipAiFollowLandXZ& goal) = 0;

    // 009E1F83, 009DA610(blk, &goal) with ECX = brain+8h; the rule is
    // ship_ai_nav_goal_already_reached_009da610.
    virtual bool goal_already_reached_009da610(const ShipAiFollowLandXZ& goal) = 0;

    // 009E1D72..009E1D97: the heading-mode switch, which also zeroes blk+368h
    // and blk+360h on the transition only.
    virtual ShipAiSteeringMode blk_steering_mode() = 0;
    virtual void enter_heading_mode() = 0;

    // 009E1DB0 blk+1D8h, 009E1DCC and 009E1E34 blk+1CCh, 009E1E12 blk+1C8h,
    // 009E1E3B blk+1D0h. The final arm writes the same four fields 009DBF90 and
    // 009E0040 own, inline, without calling either setter.
    virtual void set_blk_desired_heading_1d8(float heading) = 0;
    virtual void set_blk_requested_direction_1cc(ShipAiThrottleDirection value) = 0;
    virtual void set_blk_throttle_hold_1c8(int value) = 0;
    virtual void set_blk_desired_throttle_1d0(float throttle) = 0;

    // 009E1E48, 009E1FEF and 009E200C, brain+0AF0h, the brain speed scale three
    // attackmove sub-states also write. contract: unread, as there.
    virtual void set_brain_speed_scale_0af0(float scale) = 0;
};

// The per-call arm of 006AC5D0, __thiscall(pad)(float3* out, const float3* from,
// int zone_group, float half_width), body 006AC5D0-006ACB37. `line` is the
// cache the first arm fills and `facing` the pad's own forward, normalised in
// the xz plane by 006AC260. Inside the corridor the answer is a carrot on the
// approach axis ahead of the ship's own projection; outside it, a hold-off point
// one clearance behind the pad. Coverage: this arm only.
ShipAiFollowLandXZ ship_ai_land_pad_approach_point_006ac5d0(const ShipAiLandPadLine& line,
                                                            const ShipAiFollowLandXZ& facing,
                                                            const ShipAiFollowLandXZ& from,
                                                            float half_width);

// 009E1E0E..009E1E31: the final arm clamps the ramp result into [-1, +1] with
// two ordered compares before writing blk+1D0h. The unclamped value still
// reaches brain+0AF0h at 009E1E48, so the two differ whenever the ramp leaves
// the interval.
float ship_ai_land_clamp_throttle(float value);

// The whole step. `state` is brain+0BE0h; `seconds` is the step argument.
void ship_ai_land_step_009e1950(ShipAiLandState& state, ShipAiLandStepHost& host,
                                float seconds);

}  // namespace bsp
