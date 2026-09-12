// The unconditional last step of every arm of the ship AI controls step.
//
// Address: 009DE5B0, body 009DE5B0-009DF117 (0B67h bytes), Ghidra function
// FUN_009de5b0.  `void __thiscall(blk)(float unused)`, RET 4 at 009DF115.
// Exactly one call site: 009EF213, inside BSP_ShipAi_ControlsStep 009ED6B0
// (body 009ED6B0-009EF228), reached after the arm tail 009EEAAB..009EF226 has
// finished, so it is the last thing that touches the control block before
// 009F3F80 reads it on the same tick.
//
// The pushed stack argument at 009EF206..009EF210 (`FLD [ESP+0D0h]`, `PUSH ECX`,
// `FSTP [ESP]`) is never read inside the body: the only reads of the incoming
// frame are of `this` in ECX.  The routine still balances it with RET 4.  Every
// value it consumes comes out of the block, the unit at blk+3FCh, or the
// entity controller at [blk+3FCh]+284h.
//
// What it writes: blk+324h, the heading target (four separate rules, in
// order), blk+354h, the clearance hold, and the unit's turn-assist load latch
// unit+102Ch.  It writes nothing else.  It does NOT write blk+1D0h (desired
// throttle), blk+1D4h (desired rudder), blk+1D8h (desired heading), blk+35Ch
// (the ahead/astern latch) or blk+30Ch (the avoid-zone layer key), so it cannot
// be the reason an AI ship's desired throttle stays zero.  It can and does
// override the heading the rudder law produced: see
// ship_ai_arm_final_avoidance_override_009de8f1 below.
//
// Evidence: the stored Ghidra listing of 009DE5B0 read instruction by
// instruction over 009DE5B0..009DF115, the listings of the callees named in the
// host below, and the listing of the producer 009ECA20 for the three fields
// blk+14Ch / blk+150h / blk+160h.  Every constant is read from the image.
// Every name below is a hypothesis, not a recovered symbol.
#ifndef BSP_SHIP_AI_ARM_FINAL_STEP_HPP
#define BSP_SHIP_AI_ARM_FINAL_STEP_HPP

#include <array>
#include <cstddef>

#include "bsp/ship_ai_navigation.hpp"   // ShipAiNavState, ShipAiNavTurnSide
#include "bsp/ship_ai_sector_scan.hpp"  // ShipAiSectorFreeBearingQuery
#include "bsp/ship_ai_states.hpp"       // ShipAiControlBlock, ShipAiThrottleDirection
#include "bsp/unit_rudder.hpp"          // wrapped_angle_add/subtract, clamped_interpolate

namespace bsp {

// ---------------------------------------------------------------------------
// Literals, all read from the image at the addresses given.
// ---------------------------------------------------------------------------

// 009DE5DE, the float at 00D7A24C.  The whole step is skipped below this
// absolute body-axis speed; also the floor under a separation vector at
// 009DEBF8, and the seed both controller extents start from (0070D40D).
inline constexpr float kShipAiArmFinalMinSpeed = 1.0f;
// 009DE5F9, the double at 00CEC9D8: how much of blk+318h the free-bearing
// query is allowed to look along before the path point caps it.
inline constexpr double kShipAiArmFinalLookAheadShare = 0.75;
// 009DE6BE / 009DE94D, the float at 00D7A264: the half turn an astern latch
// adds to the unit's own heading.
inline constexpr float kShipAiArmFinalHalfTurn = 3.14159265f;
// 009DE8A0 / 009DE8B4 / 009DED21, the float at 00D7A208.  The image spells
// every negation as `-0.0f - x`.
inline constexpr float kShipAiArmFinalNegativeZero = -0.0f;

// The avoid-zone escape blend, 009DE6DB..009DE8ED.
// 009DE739 / 009DE729, the floats at 00CEB5A8 and 00CF8858: full weight while
// the ship is within 45 degrees of its heading target, none past 80 degrees.
inline constexpr float kShipAiArmFinalEscapeErrorFull = 0.7853982f;  // pi/4
inline constexpr float kShipAiArmFinalEscapeErrorNone = 1.3963f;     // 80 deg
// 009DE75B, the float at 00CE3854: what blk+354h is raised to, and the low
// knee of the remaining-path ramp at 009DE7CA.
inline constexpr float kShipAiArmFinalClearanceHold = 3.0f;
// 009DE7BA, the float at 00CE3850: the high knee of that ramp.
inline constexpr float kShipAiArmFinalPathLengthsHigh = 5.0f;
// 009DE780 / 009DEB3A, the float at 00CE3D08: the floor under the hull radius
// the remaining path is measured in, and the high y of the separation falloff.
inline constexpr float kShipAiArmFinalHullFloor = 100.0f;
// 009DE805, the float at 00CE7818: the escape weight at zero escape distance.
inline constexpr float kShipAiArmFinalEscapeFloorWeight = 0.15f;
// 009DE831, the double at 00CE3D78: the turn-assist load the escape blend
// requests, and the divisor of the summed hull radii at 009DEAA8.
inline constexpr double kShipAiArmFinalEscapeLoadScale = 1.5;
// 009DE85B, the double at 00CEC730: the largest turn one tick of escape
// steering may add.
inline constexpr double kShipAiArmFinalEscapeMaxTurn = 0.52359879; // 30 deg

// The avoidance-vector override, 009DE8F1..009DE96C.
// 009DE922, the double at 00D7A268: the squared length the avoidance vector
// must exceed before it is steered at.
inline constexpr double kShipAiArmFinalAvoidanceMinSquare = 1.0e-4;

// The traffic separation turn, 009DE96C..009DEE07.
// 009DE9CB / 009DEB8E, the floats at 00CE69D0 and 00CE3800: how far ahead of
// its own pose the ship probes, as a fraction of the way to the goal.
inline constexpr float kShipAiArmFinalProbeMin = -0.5f;
inline constexpr float kShipAiArmFinalProbeMax = 0.5f;
// 009DEA62, the float at 00D7A218: a neighbour whose lifetime has run out.
inline constexpr float kShipAiArmFinalZero = 0.0f;
// 009DEAE4, FLD1: the squared distance below which a neighbour is ignored.
inline constexpr float kShipAiArmFinalSeparationNearSquare = 1.0f;
// 009DEB3E, the float at 00CE7804: the low knee of the separation falloff.
inline constexpr float kShipAiArmFinalSeparationKnee = 0.4f;
// 009DEBCD, the double at 00CE3820: the squared separation vector length
// below which nothing is steered.
inline constexpr double kShipAiArmFinalSeparationMinSquare = 1.0e-10;
// 009DEC05, the double at 00D7A220: the separation vector is scaled down to
// this length before its bearing is taken.
inline constexpr double kShipAiArmFinalSeparationMaxLength = 100.0;
// 009DEC87 / 009DEC97 / 009DEC9F / 009DECA9, the doubles at 00CE3830 (pi/2),
// 00CE3D28 (pi), 00CF48A0 (-pi/2) and 00CE3D18 (-pi): a separation bearing
// outside the beam is folded back onto the near side.
inline constexpr double kShipAiArmFinalQuarterTurn = 1.5707963705062866;
inline constexpr double kShipAiArmFinalHalfTurnDouble = 3.1415927410125732;
inline constexpr double kShipAiArmFinalQuarterTurnNegative = -1.5707963705062866;
inline constexpr double kShipAiArmFinalHalfTurnNegative = -3.1415927410125732;
// 009DECB7, the double at 00CED5D8: the separation bearing error is divided by
// this before it is clamped.
inline constexpr double kShipAiArmFinalSeparationGainDivisor = 7.0;
// 009DECC7, the double at 00D049A8: the per-tick separation turn limit is this
// many times blk+3D0h.
inline constexpr double kShipAiArmFinalSeparationTurnScale = 1.8;

// The free-bearing query, 009DEE0B..009DF10F.
// 009DEE81 / 009DF089, the double at 00CE3DE0: the corridor half-width is this
// many times the widest controller extent, or the hull half-width.
inline constexpr double kShipAiArmFinalCorridorScale = 2.5;
// 009DEE99, the double at 00CE3DB0: the floor under the corridor range, in
// hull radii.
inline constexpr double kShipAiArmFinalCorridorHullFloor = 8.0;
// 009DEF1E and the five other sites, the double at 00CE3938: the margin added
// to each controller extent before it becomes a query width.
inline constexpr double kShipAiArmFinalWidthMargin = 50.0;

// The three 20h-byte avoid-zone searchers in the block.  009EC0C1 uses index 0
// (`LEA ECX,[ESI+0A24h]`); this routine uses index 2 (009DF056,
// `LEA ECX,[ESI+0A64h]`) when the controller's area key has moved and index 1
// (009DF0F4, `LEA ECX,[ESI+0A44h]`) otherwise.  What distinguishes them is not
// read: 009DC2E0's body was not projected here and neither searcher has a
// producer in any routine read so far.
inline constexpr int kShipAiArmFinalSearcherCurrent = 1;  // blk+0A44h
inline constexpr int kShipAiArmFinalSearcherMoved = 2;    // blk+0A64h

// ---------------------------------------------------------------------------
// The block fields this routine touches that no other header declares.
// Offsets are relative to blk = brain+8h, the base ShipAiControlBlock uses.
// ---------------------------------------------------------------------------

struct ShipAiArmFinalStepState {
    // +318h, the look-ahead the free-bearing query starts from.  009E461F /
    // 009E462B put a floor of 250.0f under it (bsp/ship_ai_nav_block_ctor.hpp).
    // Read once, at 009DE5EB.
    float look_ahead_318{0.0f};
    // +30Ch, the avoid-zone layer key.  Read once at 009DE5FF into the query's
    // +20h word.  009ECA20 is the writer (009ED067, 009ED0E7, 009ED1A9,
    // 009ED3B8); this routine never stores it back.
    int layer_key_30c{0};
    // +184h / +188h, the hull pose 009DE2F0 publishes.  Read at 009DE60B,
    // 009DE623, 009DE9E6, 009DEA1A (and again at 009DEF83).
    float pose_x_184{0.0f};
    float pose_z_188{0.0f};
    // +174h / +178h, the point the separation probe reaches toward.  Read at
    // 009DE9E0 and 009DE9F0, and nowhere else in this body.
    float goal_x_174{0.0f};
    float goal_z_178{0.0f};
    // +34Ch / +350h, the avoidance vector 009E04E0 fills every step
    // (docs/SHIP_AI_CLEARANCE_PROFILE.md, "nothing reads it back").  009DE908
    // and 009DE932 are its reader.
    float avoidance_x_34c{0.0f};
    float avoidance_z_350{0.0f};
    // +14Ch, how far the ship must travel to leave the avoid zone it is in.
    // 009ED2E9 writes it from the length of (exit point - pose), forces 1.0f
    // when that length is at most 1.0f, and caps it at blk+3C8h (009ED356,
    // 009ED368).  Read at 009DE7EB.
    float escape_distance_14c{0.0f};
    // +150h / +154h, that same delta normalised, or (0, 0) for the short case.
    // 009ED33E / 009ED348 write it; 009DE861 reads it.
    std::array<float, 2> escape_direction_150{{0.0f, 0.0f}};
    // +160h, whether the pose is inside an avoid zone.  009ED296 sets it when
    // 004178F0 answers yes for the layer the walk reached; 009ED060 clears it.
    // The single test at 009DE6DB gates the escape blend AND suppresses the
    // free-bearing query for the whole tick.
    bool inside_avoid_zone_160{false};
    // +604h, the neighbour count, and +608h, the inline array of node pointers
    // (capacity 80h, docs/SHIP_AI_SECTOR_SCAN.md section 9).  009DE96C tests
    // the count; 009DEA3D..009DEBB2 walks it.
    int neighbour_count_604{0};
};

// One entry of the neighbour list.  The walk reads six fields through the node
// pointer and two through the observed unit; there is no call inside the loop,
// so this is data rather than a host method.
struct ShipAiArmFinalNeighbour {
    // n+14h, the observed unit.  009DEA52 skips the node when it is null.
    bool has_unit{false};
    // n+78h, the lifetime in seconds.  009DEA5D requires it above 0.0f.
    float lifetime_78{0.0f};
    // n+20h / n+24h, the near box centre.  009DEAB6, 009DEAC1.
    float centre_x_20{0.0f};
    float centre_z_24{0.0f};
    // The four unit bytes at 009DEA6F, 009DEA79, 009DEA83 and 009DEA8D.  The
    // node is used only when +5Ch is set and +5Dh, +60h and +5Eh are all
    // clear; +5Eh is the gone flag docs/SHIP_AI_SECTOR_SCAN.md names.
    bool unit_flag_5c{false};
    bool unit_flag_5d{false};
    bool unit_gone_5e{false};
    bool unit_flag_60{false};
    // [n+14h]+9C8h, the observed unit's hull radius.  009DEA9C.
    float unit_hull_radius_9c8{0.0f};
};

// The per-unit constants the routine reads directly off the unit at blk+3FCh.
struct ShipAiArmFinalStepTuning {
    // unit+9C8h, the hull radius.  009DE78C, 009DEAA2, 009DEE8B.
    float hull_radius_9c8{0.0f};
    // unit+9CCh, the hull half-width (docs/SHIP_AI_OBSTACLE_TABLES.md line
    // 155).  009DF083, only on the "member of someone else's controller" path.
    float hull_half_width_9cc{0.0f};
};

// ---------------------------------------------------------------------------
// Pure rules.
// ---------------------------------------------------------------------------

// 009DE6DB..009DE8ED, the avoid-zone escape blend, as a value.
struct ShipAiArmFinalEscapeTurn {
    // 009DE755: the weight ramp closed, so nothing at all happened.
    bool weight_positive{false};
    // 009DE763: blk+354h is raised to 3.0f whenever the weight is positive,
    // before the three further gates.
    bool raise_clearance_hold{false};
    // 009DE82B: the product of all three ramps was not positive.
    bool applied{false};
    // 009DE84D: the turn-assist load request, 1.5 * the product.
    float load_request{0.0f};
    // 009DE8CD..009DE8E7: what is added to blk+324h, already clamped.
    float turn{0.0f};
};

// `heading` is the unit heading, already shifted by pi when the latch is
// astern.  `reverse_sense` is the 009DE6AB flag.  The escape heading comes from
// 00414EB0 on blk+150h, which this function calls through
// heading_angle_00414eb0.
ShipAiArmFinalEscapeTurn ship_ai_arm_final_escape_turn_009de6db(
    float heading, float heading_target_324, float remaining_330,
    float escape_distance_14c, float look_ahead_max_3c8, float hull_radius_9c8,
    const std::array<float, 2>& escape_direction_150, bool reverse_sense) noexcept;

// 009DE8F1..009DE96C, the avoidance-vector override.  `gate` is what 009DA1D0
// answered.  Returns whether blk+324h is replaced, and by what.  This is the
// one rule in the body that discards the heading the arm's rudder law produced
// instead of nudging it.
struct ShipAiArmFinalOverride {
    bool applied{false};
    float heading_target{0.0f};
};
ShipAiArmFinalOverride ship_ai_arm_final_avoidance_override_009de8f1(
    bool gate, float clearance_hold_354, float avoidance_x_34c,
    float avoidance_z_350, ShipAiThrottleDirection direction_35c) noexcept;

// 009DEBB9..009DEE07, the turn a finished separation vector asks for.
// `heading` is the same shifted unit heading the blend used.  `entry_target`
// is blk+324h as it stood at 009DE5F1, before any of this body's writes.
struct ShipAiArmFinalSeparationTurn {
    bool applied{false};
    float turn{0.0f};
};
ShipAiArmFinalSeparationTurn ship_ai_arm_final_separation_turn_009debb9(
    float separation_x, float separation_z, float heading, float heading_target_324,
    float entry_target, float turn_window_3d0, ShipAiNavTurnSide side_304,
    bool reverse_sense) noexcept;

// 009DEE7D..009DEEE3, the range the free-bearing query is given on the
// controller-leader path: the widest controller extent times 2.5, floored at
// eight hull radii and capped at one hull radius plus the distance to the path
// point.
float ship_ai_arm_final_query_range_009dee7d(float extent_a, float extent_b,
                                             float hull_radius_9c8,
                                             float distance_32c) noexcept;

// ---------------------------------------------------------------------------
// The host: one pure virtual per native call site the projection cannot make
// pure.  The declaration order is the order 009DE5B0 first reaches them.
// ---------------------------------------------------------------------------

struct ShipAiArmFinalStepHost {
    virtual ~ShipAiArmFinalStepHost() = default;

    // 009DE5C2, 009DE67D and 009DE994: `0092D730([blk+3FCh]+1018h)`, RET 0,
    // body 0092D730-0092D76E, complete.  The controller's velocity dotted with
    // its body forward axis: [ctl+2Ch] goes to 00C32000 for the axis record
    // and to 00C31F40 for the velocity, then
    // axis+18h*v[0] + axis+1Ch*v[1] + axis+20h*v[2].  Signed, so its sign is
    // the direction the hull is actually travelling.  Called three times, not
    // cached: the value is re-read at each site.
    virtual float body_axis_speed_0092d730() = 0;

    // 009DE6B0: `[[blk+3FCh]]+50h()` on the unit, result in ST0.  The same
    // virtual slot 009EF0D6 and 009ED95D use for the unit's own heading.
    virtual float unit_heading_vtable_0050() = 0;

    // 009DE853, the inlined copy of 009D4FB0 (docs/UNIT_COMMAND_PRODUCERS.md,
    // "the load latches"): raise unit+102Ch to `request` when it is strictly
    // greater.  Reached only from the escape blend.
    virtual void raise_turn_assist_load_009de853(float request) = 0;

    // 009DE8F3: `009DA1D0(blk)`, RET 0, body 009DA1D0-009DA244, complete.
    // False when the unit answers vtable[5Ch](14); otherwise it refreshes the
    // pose through 00414DB0 when unit+C8h is clear, requires unit+100h to be
    // at least -15.0 (the double at 00CE3D58), requires blk+3ECh, and requires
    // 0080E160(unit)->+240h.  docs/SHIP_AI_CLEARANCE_PROFILE.md owns it.
    virtual bool clearance_gate_009da1d0() = 0;

    // 009DE9A7 / 009DE9AD: `[[blk+3FCh]+538h]+500h`, the per-class reference
    // speed the body-axis speed is divided by to size the separation probe.
    // The neighbouring constant +508h is the deceleration blk+32Ch uses
    // (docs/SHIP_AI_STATES.md line 158).  Provisional: only the division is
    // established, not the field's own producer.
    virtual float class_reference_speed_500() = 0;

    // 009DEA50: `blk+608h[index]`, the inline neighbour array.  The walk reads
    // the node and the observed unit directly; nothing is called.
    virtual const ShipAiArmFinalNeighbour& neighbour_608(int index) = 0;

    // 009DEE1E: `00778890(blk+3FCh)`, RET 0, body 00778890-007788A7,
    // complete.  False when the entity has no controller at +284h, otherwise
    // whether [entity+284h]+14h is the entity itself, i.e. whether this entity
    // is the one its controller points back at.
    virtual bool unit_leads_controller_00778890() = 0;

    // 009DEE37, 009DEF34, 009DEF54, 009DF018 and 009DF02B:
    // `0070D400([blk+3FCh]+284h)`, RET 0, body 0070D400-0070D5C3.  PARTIAL:
    // the head and the walk are read, the four-way unrolled tail is not.  It
    // starts from 1.0f and returns the largest, over the controller's +4F8h
    // slots of stride 34h, of the float at slot+10h+[ctl+500h]*4 clamped to
    // [0, 1200.0f] (00CFD714).  0070D5D0 is the same walk over the same column
    // negated, so the pair is the extent of the formation to either side of
    // its own axis.  Which side is which is not established.
    virtual float controller_extent_0070d400() = 0;

    // 009DEE4C, 009DEF19, 009DEF6F, 009DEFFD and 009DF046:
    // `0070D5D0([blk+3FCh]+284h)`, RET 0, body 0070D5D0-0070D7A4.  PARTIAL,
    // as above; the only difference from 0070D400 is the `-0.0f - x` at
    // 0070D5E9 before the clamp.
    virtual float controller_extent_0070d5d0() = 0;

    // 009DEEE9 and 009DEFD3: `0070E450([blk+3FCh]+284h)`, RET 0, body
    // 0070E450-0070E4B2, complete.  Starts from 0 and returns the largest
    // `member->vtable[214h]()` over the controller members at ctl+18h stride
    // 34h that answer `member->vtable[5Ch](6)`.  009ECA20 seeds blk+30Ch from
    // the same call at 009ECA51, which is why 009DEEEE compares the two.
    // Called twice on the "moved" path; the second result is what reaches the
    // query, and nothing stores it back into blk+30Ch.
    virtual int controller_area_key_0070e450() = 0;

    // 009DF06B: `007788B0(blk+3FCh)`, RET 0, body 007788B0-007788C7,
    // complete, already named BSP_Entity_ControllerBelongsToAnother.  The exact
    // complement of 00778890 for a non-null controller: whether the controller
    // points back at some other entity.
    virtual bool controller_belongs_to_another_007788b0() = 0;

    // 009DF0FA: `009DC2E0(blk+0A24h + index*20h)(&query)`, `char __thiscall`,
    // RET 4, body 009DC2E0-009DCEA2, not projected here.  The avoid-zone free
    // bearing query docs/SHIP_AI_SECTOR_SCAN.md section 14 describes; only
    // `query.bearing` is read back.  `index` is 1 or 2, never 0.
    //
    // `area_key_20` is the block's +20h word.  ShipAiSectorFreeBearingQuery
    // declares that word as `const void* context`; both writers this routine
    // has are integer layer keys (blk+30Ch at 009DE5FF, 0070E450's result at
    // 009DEFE5), so it is passed separately here rather than redeclaring the
    // record.  See the Corrections section of docs/SHIP_AI_ARM_FINAL_STEP.md.
    virtual bool avoid_zone_free_bearing_009dc2e0(int searcher_index,
                                                  ShipAiSectorFreeBearingQuery& query,
                                                  int area_key_20) = 0;
};

// ---------------------------------------------------------------------------
// The sequence routine.
// ---------------------------------------------------------------------------

struct ShipAiArmFinalStepResult {
    // 009DE5E5: the body-axis speed was at most 1.0 in magnitude and the whole
    // step returned without reading anything else.
    bool ran{false};
    // 009DE6DB: the pose was inside an avoid zone.  Suppresses the free
    // bearing query for the whole tick whatever the blend then decides.
    bool inside_avoid_zone{false};
    // The escape blend actually moved blk+324h.
    bool escape_applied{false};
    // 009DE932: blk+324h was replaced outright by the avoidance vector's
    // bearing.
    bool avoidance_override{false};
    // 009DEDFC: a separation turn was added to blk+324h.
    bool separation_applied{false};
    // 009DF0FA ran, with which searcher, and what it answered.
    bool free_bearing_queried{false};
    int searcher_index{0};
    bool free_bearing_accepted{false};
};

// 009DE5B0 whole.  `blk` supplies and receives +324h, +32Ch, +330h, +354h,
// +35Ch and +3A5h; `nav` supplies +304h, +3C8h and +3D0h; `state` supplies the
// rest of the block; `tuning` the two unit radii.  `query` is the 20h-byte
// stack block at [ESP+3Ch], built here and handed to the host.
ShipAiArmFinalStepResult ship_ai_arm_final_step_009de5b0(
    ShipAiControlBlock& blk, const ShipAiNavState& nav,
    const ShipAiArmFinalStepState& state, const ShipAiArmFinalStepTuning& tuning,
    ShipAiSectorFreeBearingQuery& query, ShipAiArmFinalStepHost& host);

} // namespace bsp

#endif // BSP_SHIP_AI_ARM_FINAL_STEP_HPP
