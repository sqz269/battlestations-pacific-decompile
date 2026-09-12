// The tail of the ship AI's navigation arm, 009EEAAB..009EF228.
//
// Address: inside BSP_ShipAi_ControlsStep 009ED6B0 (body 009ED6B0-009EF228),
// __thiscall(blk)(float seconds), RET 4, called at 009F5209.
// The range runs after the output block 009EE671..009EEAA2 has written blk+324h
// (heading target), blk+32Ch (distance to the path point), blk+330h (remaining
// path length), blk+328h (the applied turn lead) and blk+340h (the look-ahead
// radius).  It turns those into the two values 009F3F80 then consumes:
//   blk+344h, the throttle ceiling for the approach, and
//   blk+35Ch, the latched ahead/astern direction.
// It also publishes blk+2FDh and blk+2FEh, the two arrival bytes
// BSP_ShipAi_GoalReachedTest 009DA590 and 009DA610 read.
//
// Evidence: the stored Ghidra listing of 009ED6B0 read instruction by
// instruction over 009EEAAB..009EF226, plus the disk bytes for 009D5240.
// Every name below is a hypothesis, not a recovered symbol.
#ifndef BSP_SHIP_AI_NAVIGATION_ARM_TAIL_HPP
#define BSP_SHIP_AI_NAVIGATION_ARM_TAIL_HPP

#include <array>

#include "bsp/ship_ai_navigation.hpp"
#include "bsp/ship_ai_states.hpp"

namespace bsp {

// ---------------------------------------------------------------------------
// Literals, all read from the image.
// ---------------------------------------------------------------------------

// 009EEAF0, the double at 00CF0DD8.  The traffic walk only runs while the
// distance to the path point is under this.
inline constexpr double kShipAiArmTailTrafficRange = 2000.0;
// 009EEBE6, the float at 00CEB4B0: the floor under the two hull radii.
inline constexpr float kShipAiArmTailClearanceFloor = 60.0f;
// 009EEC01 and 009EED39, the double at 00D7A280.
inline constexpr double kShipAiArmTailHalf = 0.5;
// 009EEE84, the double at 00CE3D88: how far each walk step backs off.
inline constexpr double kShipAiArmTailWalkStep = 20.0;
// 009EED31, the float at 00CE3868: the floor under the approach ceiling.
inline constexpr float kShipAiArmTailCeilingFloor = 0.25f;
// 009EED14 / 009EEEF8, the float at 00D7A24C.
inline constexpr float kShipAiArmTailCeilingCap = 1.0f;
// 009EEFFA, the float at 00D7A260: what a latch change stores in blk+374h.
inline constexpr float kShipAiArmTailLatchMark = -1.0f;
// 009EEF97, the double at 00D7A3A0, against blk+1F0h.
inline constexpr double kShipAiArmTailPathLengthShare = 0.1;
// 009EEFAB, the double at 00D7A348, against blk+3CCh.
inline constexpr double kShipAiArmTailTurnDistanceShare = 0.25;
// 009EF131, the double at 00D7A2B0: the hull radius term of the astern test.
inline constexpr double kShipAiArmTailHullRadiusScale = 3.0;
// 009EF153, the double at 00CF1440: the extra length an already-astern ship
// keeps before it will go ahead again.
inline constexpr double kShipAiArmTailAsternHysteresis = 80.0;
// 009EF17A..009EF18E: 120 degrees while astern, 130 degrees otherwise, both
// turned into radians by the doubles at 00CE3D28 (pi) and 00CE3D20 (180).
inline constexpr int kShipAiArmTailAsternAngleAsternDeg = 120;
inline constexpr int kShipAiArmTailAsternAngleAheadDeg = 130;
// 009EEBC4, the argument the entity kind test takes at 009EEBC8.
inline constexpr int kShipAiArmTailNeighbourKind = 6;
// 009EF03B: the plan search state the parked byte needs to exceed.
// docs/SHIP_AI_PATH_PLANNER.md: 4 Extracting, 5 ExtendSearch, 6 Extracted,
// 7 Ready, so "the plan has produced or is producing a path".
inline constexpr int kShipAiArmTailPlanStateFloor = 3;

// ---------------------------------------------------------------------------
// The fields of `blk` the tail uses that no other header declares.
// Offsets are relative to blk = brain+8h, the base ShipAiControlBlock uses.
// ---------------------------------------------------------------------------
struct ShipAiArmTailState {
    // +3A6h, the second half of the 009EDA34 / 009EDA41 station-keeping gate.
    // Read at 009EF0B1 only.
    bool leader_snapshot_3a6{false};
    // +2FCh, cleared by the path-plan reset 009DA4E0.  Read at 009EF022.
    bool plan_reset_2fc{false};
    // +2FDh, the "parked on a usable plan" byte.  009DA610 answers false
    // unless it is set; this range writes 1 at 009EF045 and the arm ahead of
    // it writes it at 009EE718 / 009EE732 / 009EE750.  Read at 009EEF72.
    bool parked_2fd{false};
    // +2FEh, the arrival latch BSP_ShipAi_GoalReachedTest 009DA590 requires.
    // This range is its only writer inside 009ED6B0 after the 009ED779 clear.
    bool goal_reached_2fe{false};
    // [blk+2F4h]+1Ch, the plan's search state (docs/SHIP_AI_PATH_PLANNER.md).
    int plan_search_state_1c{0};
    // +344h, the approach throttle ceiling.  009F4439 and 009F4462 limit the
    // AI's throttle against it (docs/SHIP_AI_THROTTLE_TO_RING.md).
    float throttle_ceiling_344{0.0f};
};

// The per-ship tuning the tail reads but never writes.
struct ShipAiArmTailTuning {
    // +3C4h, the cached reference speed (docs/SHIP_AI_OBSTACLE_TABLES.md).
    float reference_speed_3c4{0.0f};
    // [[blk+3FCh]+538h]+508h, the class deceleration the stopping distance
    // divides by; the same pair 009ED8EC uses.
    float class_deceleration_508{1.0f};
    // +3CCh, written at 009E4568 from 0082E960(class, 0.9f).
    float turn_distance_3cc{0.0f};
    // +3D4h = max(1.5f * class MaxSpeed, 0.4f * hull radius), 009E4537.
    float stop_radius_3d4{0.0f};
    // +3D8h = 1.5f * +3D4h, 009E453F.
    float start_radius_3d8{0.0f};
    // [blk+3FCh]+9C8h, the unit's hull radius.
    float hull_radius_9c8{0.0f};
    // +604h, the neighbour count written at 009E4659.  Only its sign is used
    // here: 009EEAD5 requires it to be positive before the walk runs.
    int neighbour_count_604{0};
    // [[blk+3FCh]+73Ch]+21h, the navigatorParams byte read at 009EEAB3.
    // docs/UNIT_COMMANDED_SPEED.md has its producers.  Clearing it switches
    // the traffic setback off entirely.
    bool keep_clear_of_traffic_21{true};
};

// ---------------------------------------------------------------------------
// Pure rules.
// ---------------------------------------------------------------------------

// 009D5240, the canonical direction-latch setter.
// __thiscall(blk)(int direction, char force), RET 8, body 009D5240-009D5291,
// complete; no Ghidra function and no decoded caller, so every use in the
// image is an inlined copy.  009EEFE4, 009EF087, 009EF1B2 and 009EF1D4 are
// four such copies, all with force = false.
// Returns whether the latch actually moved.
bool ship_ai_set_direction_latch_009d5240(ShipAiControlBlock& blk,
                                          ShipAiThrottleDirection direction,
                                          bool force) noexcept;

// 009EECDB..009EED50, the approach throttle ceiling that lands in blk+344h.
// remaining / ((speed / deceleration) * speed * 0.5), clamped to
// [0.25f, 1.0f].  The denominator is the stopping distance from the reference
// speed, the same shape 009ED8EC builds for blk+32Ch without its hull term.
float ship_ai_approach_ceiling_009eecdb(float remaining_330, float reference_speed_3c4,
                                        float class_deceleration_508) noexcept;

// 009EEC15..009EEC37, the clearance radius one neighbour is tested against.
// max(60.0f, (own + other) * 0.5f) + max(0.0f, allowance - walked), where
// `allowance` is blk+32Ch * |blk+328h| from 009EEAFE..009EEB26: the arc the
// applied turn lead sweeps at the current distance.
float ship_ai_traffic_clearance_009eec15(float allowance, float walked,
                                         float own_hull_radius,
                                         float other_hull_radius) noexcept;

// One step of the walk, 009EED62..009EEEDC.  `point` starts at the goal and
// moves back along `dir` (the unit vector from the ship's pose to the goal)
// until nothing blocks it; `walked` accumulates how far back it has moved.
struct ShipAiSetbackStep {
    float walked{0.0f};
    float point_x{0.0f};
    float point_z{0.0f};
};
ShipAiSetbackStep ship_ai_traffic_setback_step_009eed62(float walked,
                                                        float point_x, float point_z,
                                                        float dir_x, float dir_z,
                                                        float other_x, float other_z,
                                                        float clearance) noexcept;

// 009EF0A8..009EF1D4, which way the ship should face.
// threshold = max(hull * 3.0, turn_radius * 2), plus 80.0 while already
// astern; astern is chosen only when the remaining path is inside that
// threshold and the heading error exceeds 120 degrees (astern) or 130
// degrees (otherwise).
ShipAiThrottleDirection ship_ai_astern_choice_009ef0a8(float heading_error_abs,
                                                       float remaining_330,
                                                       float turn_radius,
                                                       float hull_radius,
                                                       ShipAiThrottleDirection latched) noexcept;

// ---------------------------------------------------------------------------
// The host: one method per native call site the tail makes.
// ---------------------------------------------------------------------------

// An entity out of the global list at [[00E188A8]+19CCh]+60h.  Opaque: the
// tail only compares it against the unit and asks the host about it.
using ShipAiArmTailEntity = const void*;

struct ShipAiArmTailHost {
    virtual ~ShipAiArmTailHost() = default;

    // 009EEB63, 004192E0(&out, &in), RET 0, body 004192E0-0041930B, complete:
    // it calls 00419260(&in) and multiplies both components of `in` by the
    // float that comes back.  00419260 (body 00419260-004192D5, read but not
    // annotated: it is leased to orch3_tracer_math_and_trail_ak) answers
    // 1 / sqrt(x*x + z*z) through 00BF7030, and +0.0f for a zero-length
    // vector, so the pair is a 2D normalise whose zero case is (0, 0).
    // The call stays on the host because it is a native call site.
    virtual ShipAiNavPose normalize_004192e0_009eeb63(float x, float z) = 0;

    // 009EEB8B, MOV EBX,[ECX+60h] on [[00E188A8]+19CCh]: the list length.
    // Re-read at the top of every walk iteration.
    virtual int neighbour_list_count_009eeb8b() = 0;

    // 009EEBB0, 009DBBC0([[00E188A8]+19CCh]+60h)(index), RET 4, body
    // 009DBBC0-009DBBF2, complete: a singly linked list indexer over
    // {count at +0h, head at +4h, next at +4h, payload at +8h}.  Null for an
    // out-of-range index or a short list.
    virtual ShipAiArmTailEntity list_element_009dbbc0(int index) = 0;

    // 009EEBC8, entity->vtable[5Ch](6).  The site tests AL only.
    virtual bool entity_is_kind_009eebc8(ShipAiArmTailEntity entity, int kind) = 0;

    // 009EEBD2, MOV EAX,[ESI+3FCh]: the unit the block drives, used to skip
    // the ship itself.
    virtual ShipAiArmTailEntity own_unit_009eebd2() = 0;

    // 009EEBE0 and 009EEBEE, [entity+9C8h] on the neighbour and on the unit.
    virtual float entity_hull_radius_009eebe0(ShipAiArmTailEntity entity) = 0;

    // 009EEC41, BSP_EntityPose_GetWorldPositionRefreshed 00427EB0(entity);
    // the site takes [EAX] and [EAX+8h], the x and z of the translation row.
    virtual ShipAiNavPose entity_position_00427eb0_009eec41(ShipAiArmTailEntity entity) = 0;

    // 009EF0D6, CALL EAX = unit->vtable[50h]() on [blk+3FCh], no argument,
    // the result taken as a float.  The same slot the output block calls at
    // 009EE8C7 and the direct-control arm at 009ED95D; read as the unit's own
    // heading, which is provisional (docs/UNIT_AI_ORDER_SLOT_READER.md).
    virtual float unit_heading_vtable_0050_009ef0d6() = 0;

    // 009EF112, BSP_ShipClass_GetTurnRadius 0082E850([[blk+3FCh]+538h]).
    virtual float ship_class_turn_radius_0082e850_009ef112() = 0;

    // 009EF213, 009DE5B0(blk)(seconds), RET 4, body 009DE5B0-009DF117.
    // The unconditional last step of 009ED6B0 on every arm.  contract: unread.
    virtual void after_arm_009de5b0(float seconds) = 0;
};

// ---------------------------------------------------------------------------
// The sequence routine.
// ---------------------------------------------------------------------------

// What the walk over 009EEAAB..009EEEEC produced.
struct ShipAiTrafficSetback {
    // [ESP+18h] at 009EEF2E: how far short of the goal the ship must stop.
    float distance{0.0f};
    // whether the four-way gate at 009EEAB7..009EEAF8 opened at all.
    bool scanned{false};
    // how many times the walk pushed the stop point back.
    int steps{0};
};

// What the whole tail did, so a caller can watch it without reading `blk`.
struct ShipAiArmTailResult {
    ShipAiTrafficSetback setback{};
    // blk+344h as written at 009EEF0A.
    float throttle_ceiling{1.0f};
    // the BL at 009EEF3E: the stop was released this tick.
    bool release_stop{false};
    // the path through 009EEFD3 was taken: the ship asked to stop.
    bool request_stop{false};
    // blk+35Ch after the range.
    ShipAiThrottleDirection direction{ShipAiThrottleDirection::Stopped};
    // AL at 009EF218, the routine's return value.  009F520E zeroes
    // brain+0B14h when it is set.  True means a direction change was *wanted*,
    // not that the latch moved: 009EF1AB and 009EF1CD set the byte before the
    // 009EF360h timer test that can still refuse the write.
    bool direction_change_requested{false};
};

// 009EEAAB..009EF226 as one routine.
// `nav` supplies blk+340h and blk+328h, which the output block wrote;
// `waypoint` supplies the two bytes 009E3C00 returned at record+20h / +21h
// ([ESP+98h] and [ESP+99h]); `goal` is blk+1DCh / blk+1E0h and `pose` is
// blk+184h / blk+188h.  `goal_is_destination` is [ESP+43h], the local
// 009EE6E5 / 009EE6EC set from waypoint.more_path && blk+1E4h.
ShipAiArmTailResult ship_ai_navigation_arm_tail_009eeaab(
    ShipAiControlBlock& blk,
    ShipAiArmTailState& tail,
    const ShipAiNavState& nav,
    const ShipAiArmTailTuning& tuning,
    const ShipAiNavWaypoint& waypoint,
    const ShipAiNavPose& goal,
    const ShipAiNavPose& pose,
    bool goal_is_destination,
    float seconds,
    ShipAiArmTailHost& host);

}  // namespace bsp

#endif  // BSP_SHIP_AI_NAVIGATION_ARM_TAIL_HPP
