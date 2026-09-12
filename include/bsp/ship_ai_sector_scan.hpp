#pragma once
#include <array>
#include <cstddef>
#include <cstdint>
#include <vector>

#include "bsp/ship_ai_obstacle_tables.hpp" // ShipAiObstacleSector, the sector array constants
#include "bsp/unit_rudder.hpp"             // wrapped_angle_add/subtract, clamped_interpolate
#include "bsp/vector_helpers.hpp"          // length_2d_00414c60

// 009EB660, the obstacle sector scan, and the neighbour list it walks.
//
// docs/SHIP_AI_OBSTACLE_TABLES.md read 009EB660 partially: the entry, the two hit
// arms and the bearing tail. This header projects the part it left unread,
// 009EB6B7..009EBECC, plus the producers of the two inputs the scan consumes:
// the twelve sector shapes (009E0270) and the neighbour list at
// blk+604h / blk+608h (009F0D20 appends, 009F0EA0 ages and compacts,
// 009E52E0 constructs one node).
//
// The shape of 009EB660, in the routine's own order:
//
//   1. 009EB660..009EB6AE  the hysteresis margin, the two clears, and the one
//                          branch that picks the probe shape: the sector's
//                          turn radius sector+4h decides straight or arc.
//   2. 009EB6B4..009EB929  the straight probe: a ray from a laterally offset
//                          origin along +/- the hull forward vector, clipped
//                          first by the avoid-zone segment list and then by
//                          every neighbour.
//   3. 009EB92E..009EBEB6  the arc probe: a turning circle of radius sector+4h
//                          whose centre sits abeam, swept from the beam bearing
//                          by arc_length / radius, clipped the same two ways.
//   4. 009EBEBB..009EBFDF  the unit-hit arm: mark blocked, extend the blocking
//                          node's lifetime, pick the passing side and the corner
//                          to steer at (009D84E0).
//   5. 009EBFE4..009EC1A0  the avoid-zone arm: mark blocked and ask 009DC2E0 for
//                          a free bearing, falling back to straight ahead.
//   6. 009EC1A8..009EC275  the common tail: bias the avoidance bearing away from
//                          the hit by an angle that shrinks with distance.
//
// Every offset and constant below carries the address it was read at. Names are
// hypotheses, not recovered symbols. These are semantic interfaces for MSVC
// Win32, not drop-in binary replacements.

namespace bsp {

// ---------------------------------------------------------------------------
// Constants
// ---------------------------------------------------------------------------
// 00CE3C64, loaded at 009EB93F and 009EB95F: the beam offset the arc probe
// applies to the hull heading to get the bearing from the turn centre to the
// hull. Float pi/2.
inline constexpr float kShipAiSectorBeamOffset = 1.5707963705062866f;

// 00D7A218, 009EB9E1 and 009EBB9B: the sign test on the sector's lateral
// offset. Exactly +0.0f, so the test is `lateral < 0`.
inline constexpr float kShipAiSectorLateralSignPivot = 0.0f;

// 00D7A208, 009EB72B and 009EB911 (and 009F1911 in the pre-pass): the negation
// idiom, -0.0f. `-0.0f - v` is how the image spells `-v` here.
inline constexpr float kShipAiSectorNegateZero = -0.0f;

// 00CE3830 / 00CE3828, the doubles at 009EBFAE, 009EC030, 009EC0D8 and inside
// 009D84E0 at 009D874B: the world bearing convention. A bearing is
// wrap(pi/2 - atan2(dz, dx)) folded into [0, 2*pi), and a direction is
// (cos(theta), sin(theta)) of theta = wrap(pi/2 - bearing), i.e. (sin b, cos b).
inline constexpr double kShipAiSectorBearingOrigin = 1.5707963705062866;
inline constexpr double kShipAiSectorBearingTurn = 6.2831854820251465;

// 00CE3820, the double at 009EC1D9 and 00415970: the squared-length floor under
// which the length helper returns +0 instead of calling the CRT square root.
inline constexpr double kShipAiSectorLengthEpsilon = 1.0000000116860974e-10;

// 009EC20B..009EC237: the avoidance bearing is biased away from the hit point by
// clamped_interpolate_00419010(10.0f, 0.0f, 150.0f, 8deg, distance), so a hit at
// ten metres or nearer gets no bias and one at 150 metres or further gets the
// full eight degrees. 00CE38B8, 00CE3808 and 00D20A18.
inline constexpr float kShipAiSectorBiasNearDistance = 10.0f;
inline constexpr float kShipAiSectorBiasFarDistance = 150.0f;
inline constexpr float kShipAiSectorBiasFarAngle = 0.13962633907794952f; // 8 degrees

// 00D7A210, the double added at 009EBEE9: when a sector is actually blocked the
// blocking node's lifetime is raised to settings+194h + 1.0 seconds.
inline constexpr double kShipAiSectorBlockerLifetimeBonus = 1.0;

// 00CE3850, 009EC0A6: the pair of 5.0f the avoid-zone free-bearing query carries
// in both of its two scalar slots.
inline constexpr float kShipAiSectorFreeBearingWidth = 5.0f;

// ---------------------------------------------------------------------------
// 009E0270: the twelve sector shapes
// ---------------------------------------------------------------------------
// 009E0270 (body 009E0270-009E04D9, void __thiscall(blk)(float), RET 4) is the
// producer of the four sector fields 009EF230 does not write. EAX = blk+818h at
// 009E02FA is sector[0]+10h and the loop steps 0x108 = six strides, twice
// (009E048B, 009E0490), so the outer index is the direction group and the six
// inner writes are the six rudder buckets docs/SHIP_AI_OBSTACLE_TABLES.md
// indexes with `group * 6 + bucket`.
//
// Per group g in {0, 1} and bucket b in 0..5, with
//   radius_unit = 00811A30(unit, 0.5f)       009E02D7
//   length      = blk+3E4h = unit+9C8h * 0.45  009E44DB, the double at 00CF1748
//   full_beam   = unit+9CCh                  009E032E
// the writes are
//   sector+0h  = (g == 0)                    009E0338 SETZ; stores at009E0347 and five more
//   sector+4h  = {r, 2.5r, -1, -1, 2.5r, r}  009E034A, 009E037D, 009E03B1, ...
//   sector+0Ch = {L/10, L/5, L/2, L/2, L/5, L/10}  009E035D and the five siblings
//   sector+10h = {w/2.2, w/4, w/2.2, -w/2.2, -w/4, -w/2.2}  009E0357 and siblings
// so group 0 is the ahead fan and group 1 the astern fan, buckets 2 and 3 are
// the two straight probes off either beam, and the outer buckets are turning
// circles. The doubles are 00CE3DE0 (2.5), 00D05AC8 (2.2), 00CE3DC0 (10.0),
// 00D7A348 (0.25), 00D7A370 (5.0), 00D7A280 (0.5), and 00D7A260 (-1.0f).
inline constexpr double kShipAiSectorWideRadiusScale = 2.5;   // 00CE3DE0
inline constexpr double kShipAiSectorOuterLateralDivisor = 0x1.19999a0000000p+1; // 00D05AC8, widened 2.2f
inline constexpr double kShipAiSectorInnerLateralScale = 0.25;  // 00D7A348
inline constexpr double kShipAiSectorNearReachDivisor = 10.0;   // 00CE3DC0
inline constexpr double kShipAiSectorMidReachDivisor = 5.0;     // 00D7A370
inline constexpr double kShipAiSectorFarReachScale = 0.5;       // 00D7A280
inline constexpr float kShipAiSectorStraightRadius = -1.0f;     // 00D7A260
inline constexpr double kShipAiSectorReachHullScale = 0x1.cccccc0000000p-2; // 00CF1748, widened 0.45f

// The three unit quantities 009E0270 reads. `turn_radius_reference` is what
// 00811A30 returns for the rudder fraction 0.5f (00CE3800): a class curve
// (0082E960 on unit+538h) divided by the gameplay modifier product for category
// 5 (008E6430) when the modifier table is live. What that curve measures was not
// read; the sector uses it as the radius of a turning circle, which is the
// hypothesis this header states.
struct ShipAiSectorHullMetrics {
    float turn_radius_reference{0.0f}; // 009E02D7, 00811A30(unit, 0.5f)
    float half_length_9c8{0.0f};       // Legacy name: FULL length, unit+9C8h.
    float half_width_9cc{0.0f};        // Legacy name: FULL beam, unit+9CCh.
};

// Fills the twelve ShipAiObstacleSector shape fields (kind, half_width, reach,
// lateral). The names in ShipAiObstacleSector predate this packet: `half_width`
// is the probe's turn radius, `reach` a small longitudinal clearance and `kind`
// the ahead/astern direction. See the Corrections table in
// docs/SHIP_AI_SECTOR_SCAN.md.
void ship_ai_build_sector_shapes_009e0270(
    const ShipAiSectorHullMetrics& hull,
    std::array<ShipAiObstacleSector, 12>& sectors) noexcept;

// Exact x87/SSE shape segment009E02E0..009E0499 and remaining constant pops,
// using actual blk+3E4 directly.
// The legacy metrics overload above first derives constructor0.45*length;
// the full pre-step in ship_ai_hull_geometry.hpp uses this overload instead.
// Writes only sector+00,+04,+0C,+10, in native order. Inputs are stable for
// the loop (no calls occur while these native plain fields are read).
void ship_ai_build_sector_shapes_009e0270(float radius_00811a30,
    float stored_reach_3e4, float full_beam_9cc,
    std::array<ShipAiObstacleSector, 12>& sectors) noexcept;

// ---------------------------------------------------------------------------
// 009EF230: which three sectors refresh this frame, and their braking distance
// ---------------------------------------------------------------------------
// docs/SHIP_AI_OBSTACLE_TABLES.md read 009EF230 but did not project it. The
// round robin on blk+0A18h (009EF2BD, 009EF2CE SETGE, 009EF308, the 0x58 step at
// 009EF339 and the three iterations at 009EF312..009EF323) picks
// (counter & 1) + (counter >= 2 ? 6 : 0) and then two strides at a time.
struct ShipAiSectorRefreshSlots {
    std::array<int, 3> sector{{0, 0, 0}};
    int next_counter{0}; // 009EF2E9..009EF2FD, wraps at 4
};
ShipAiSectorRefreshSlots ship_ai_sector_refresh_slots_009ef230(int counter) noexcept;

// 009EF247..009EF2C9: v = max(body axis speed, class+500h * 0.1) and
// sector+8h = ((v + 3) / class+508h) * (v + 3) * 0.55 + unit+9C8h * 0.6.
// Doubles 00D7A3A0 (0.1), 00D7A2B0 (3.0), 00CEC8F0 (0.55), 00CEFF98 (0.6).
float ship_ai_sector_braking_distance_009ef230(float body_axis_speed,
                                               float class_top_speed_500,
                                               float class_deceleration_508,
                                               float hull_half_length_9c8) noexcept;

// ---------------------------------------------------------------------------
// The neighbour node, the record the list at blk+608h holds
// ---------------------------------------------------------------------------
// operator new(0x90) at 009F0E2A, constructed by 009E52E0 (body
// 009E52E0-009E53A5) or 009E53B0. bsp/ship_ai_obstacle_tables.hpp already
// declares ShipAiNeighbourRecord for the four fields 009F3F80 and 009D8B90
// read; this record is the same native object seen by the scan, which needs the
// two oriented boxes as well. The offsets agree: +14h owner, +44h/+48h centre,
// +54h/+58h forward, +68h flag, +88h passing side.
//
// Two boxes share one pair of axes:
//   009D80C0 tests a point against centre (+20h,+24h) with half extents
//            (+38h,+3Ch) along (+28h,+2Ch) and (+30h,+34h);
//   009D8160 tests a point against centre (+44h,+48h) with half extents
//            (+5Ch,+60h) along the same two axes;
//   009D84E0, 009DD010 and 009DD540 build the corners of the second box from
//            (+4Ch,+50h) and (+54h,+58h) instead.
// That the containment tests reuse (+28h..+34h) for the second box rather than
// (+4Ch..+58h) is what the listing says, not a simplification made here.
struct ShipAiObstacleNode {
    const void* owner{nullptr};  // +14h,       009EBDE2, 009D8B96, 009F1029
    bool owner_gone_5e{false};   // [+14h]+5Eh, 009EBDFB, 009D8BA3, 009F1034

    float near_box_x{0.0f};      // +20h, 009D80D3
    float near_box_z{0.0f};      // +24h, 009D80DC
    float axis_beam_x{0.0f};     // +28h, 009D80EE, 009D8198
    float axis_beam_z{0.0f};     // +2Ch, 009D80E3, 009D818D
    float axis_forward_x{0.0f};  // +30h, 009D811D, 009D81C7
    float axis_forward_z{0.0f};  // +34h, 009D811A, 009D81C4
    float near_half_beam{0.0f};  // +38h, 009D8111
    float near_half_length{0.0f};// +3Ch, 009D813A

    float avoid_box_x{0.0f};     // +44h, 009D817D, 009D84ED, 009D8553, 009D8BDE
    float avoid_box_z{0.0f};     // +48h, 009D8186, 009D84F2, 009D855A, 009D8BE8
    float corner_beam_x{0.0f};   // +4Ch, 009D8537
    float corner_beam_z{0.0f};   // +50h, 009D8548
    float corner_forward_x{0.0f};// +54h, 009D8519, 009D8BCA
    float corner_forward_z{0.0f};// +58h, 009D852F, 009D8BD5
    float avoid_half_beam{0.0f}; // +5Ch, 009D81BB, 009D8514
    float avoid_half_length{0.0f};// +60h, 009D81E4, 009D84FE

    bool no_pose_68{false};      // +68h, 009D80C3, 009D8163, 009D84E3, 009DD043
    bool no_arc_69{false};       // +69h, 009D816D, 009DD04C
    float lifetime_78{0.0f};     // +78h, 009E540C sets, 009F1009 ages,
                                 //       009EBEF7 and 009F0DFF raise
    int pass_side_88{0};         // +88h, 009EBF51 reads; the only writer of a
                                 //       non-zero value in the image is
                                 //       009D912F in BSP_ShipAi_PredictTrackCrossing
};

// ---------------------------------------------------------------------------
// 009D84E0: the corner to steer at, and the side to pass on
// ---------------------------------------------------------------------------
// void* __thiscall(node)(float* out_point, const float* observer,
//                        float reference_bearing, int other_side,
//                        unsigned char* out_flag), RET 14h, body
// 009D84E0-009D885A, complete. Returns out_point in EAX (009D8843).
//
// It bears the four corners of the node's avoid box from the observer, takes the
// most positive and the most negative bearing relative to `reference_bearing`,
// and picks the smaller turn, after adding 25 degrees (00D1F6C0) to whichever
// side `other_side` names. The seeds are -pi (00CE684C) and +pi (00D7A264).
inline constexpr float kShipAiSectorSideBias = 0.43633234500884056f; // 00D1F6C0, 25 degrees

struct ShipAiSectorPassChoice {
    float corner_x{0.0f}; // 009D8847 -> sector+18h at 009EBF6E
    float corner_z{0.0f}; // 009D884C -> sector+1Ch at 009EBF7A
    bool  take_max{false};// *out_flag, 009D882F / 009D883C; sector+28h is
                          // take_max ? 2 : 1 (009EBF7D SETNZ, 009EBF80 ADD 1)
};

ShipAiSectorPassChoice ship_ai_node_passing_corner_009d84e0(
    const ShipAiObstacleNode& node,
    float observer_x, float observer_z,
    float reference_bearing,
    int other_side) noexcept;

// ---------------------------------------------------------------------------
// The bearing convention, pulled out because six sites share it
// ---------------------------------------------------------------------------
// 009EBFA1..009EBFDF, 009D873E..009D876D and inside 00415970: a world bearing.
float ship_ai_sector_bearing_009ebfa1(float dx, float dz) noexcept;
// 009EC030..009EC06C and 009EC0D8..009EC117: the unit direction of a bearing.
std::array<float, 2> ship_ai_sector_direction_009ec056(float bearing) noexcept;

// ---------------------------------------------------------------------------
// The probe one sector describes
// ---------------------------------------------------------------------------
// The hull pose the scan reads out of the control block. blk+184h/+188h is the
// position (docs/SHIP_AI_STATE_STEPS.md). The three direction pairs are named
// from their use here: 009EBA3C uses (+19Ch,+1A0h) as the offset direction of a
// turn centre, which forces it to be a beam vector, and 009EBB6E uses
// (+1ACh,+1B0h) as the longitudinal one. Which of the two beam vectors is port
// was settled by the reference bearing each arm passes: the arm that offsets
// along (+19Ch,+1A0h) passes heading + pi/2 as the bearing from the centre back
// to the hull (009EB9FD, 009EBA9A), so (+19Ch,+1A0h) points at heading - pi/2.
// Their producer (009DE2F0 at 009DE462, defaults in 009DFCB0) was not read.
struct ShipAiSectorHullPose {
    float x{0.0f};          // blk+184h, 009EB6BA
    float z{0.0f};          // blk+188h, 009EB6D2
    float port_x{0.0f};     // blk+19Ch, 009EB6ED
    float port_z{0.0f};     // blk+1A0h, 009EB6F7
    float starboard_x{0.0f};// blk+1A4h, 009EBAAB
    float starboard_z{0.0f};// blk+1A8h, 009EBAC4
    float forward_x{0.0f};  // blk+1ACh, 009EB6DF / 009EBB6E
    float forward_z{0.0f};  // blk+1B0h, 009EB70F / 009EBB78
    float heading{0.0f};    // [blk+3FCh]->vtable[50h](), 009EB939
};

// The derived probe. `is_arc` is false when the sector's turn radius is
// negative (009EB69F COMISS against +0.0f, 009EB6AE JBE).
struct ShipAiSectorProbe {
    bool is_arc{false};
    bool ahead{false};        // sector+0h != 0
    // straight (009EB6B4..009EB790)
    float origin_x{0.0f};
    float origin_z{0.0f};
    float direction_x{0.0f};
    float direction_z{0.0f};
    // arc (009EB92E..009EBD48)
    float centre_x{0.0f};     // 009EBA73 / 009EBB2C / 009EBC29 / 009EBCE2
    float radius{0.0f};       // sector+4h
    float centre_z{0.0f};
    float reference_bearing{0.0f}; // 009EB9FD / 009EBAB1 / 009EBBAE / 009EBC67
    float swept_bearing{0.0f};     // 009EBB5D / 009EBD13
    float half_angle{0.0f};        // +/- reach / radius, 009EBA86 and siblings
    // both (009EBB82 adds, 009EBD38 subtracts)
    float probe_x{0.0f};      // the point 009D8160 is asked about
    float probe_z{0.0f};
    float range{0.0f};        // sector+8h + margin, 009EB790 / 009EB98F
};

ShipAiSectorProbe ship_ai_sector_probe_009eb660(const ShipAiObstacleSector& sector,
                                                const ShipAiSectorHullPose& pose,
                                                float margin) noexcept;

// ---------------------------------------------------------------------------
// The avoid-zone free-bearing query 009DC2E0 takes
// ---------------------------------------------------------------------------
// 009EC089..009EC0C1 builds a 0x24-byte block on the stack and passes its
// address; 009DC2E0 is `char __thiscall(blk+0A24h)(query*)`, RET 4, body
// 009DC2E0-009DCEA2, not read past its call site. Only `bearing` is read back
// (009EC0CE, 009EC0D2).
struct ShipAiSectorFreeBearingQuery {
    float origin_x{0.0f};  // +0h,  009EC091
    float origin_z{0.0f};  // +4h,  009EC0A0
    float direction_x{0.0f}; // +8h,  009EC077
    float direction_z{0.0f}; // +0Ch, 009EC083
    float width_a{kShipAiSectorFreeBearingWidth};  // +10h, 009EC0BB
    float width_b{kShipAiSectorFreeBearingWidth};  // +14h, 009EC0B5
    float range{0.0f};     // +18h, 009EC08D
    float bearing{0.0f};   // +1Ch, output
    const void* context{nullptr}; // +20h, blk+0A38h at 009EC02A
};

// ---------------------------------------------------------------------------
// The host the executable must implement
// ---------------------------------------------------------------------------
// One pure virtual per native call site the projection cannot make pure. The
// order of the declarations is the order 009EB660 reaches them.
struct ShipAiSectorScanHost {
    virtual ~ShipAiSectorScanHost() = default;

    // 009EB681 and 009EBEDE, CALL 00424C40. The scan reads two settings fields
    // from the same singleton: +1D8h at 009EB686 (the hysteresis margin a
    // sector that was blocked last frame adds to its range) and +194h at
    // 009EBEE3 (the neighbour memory, the seconds a blocking node is kept).
    virtual float settings_blocked_margin_1d8() = 0;
    virtual float settings_neighbour_memory_194() = 0;

    // 009EB939, 009EBF1A and 009EBFFE, CALL EDX through [[blk+3FCh]]+50h: the
    // hull heading. Read three times inside one call, so it is a method, not a
    // frame value.
    virtual float unit_heading_vtable50() = 0;

    // 009EB819, CALL 004158E0 with ECX = &blk+0A3Ch: walk the avoid-zone
    // segment tree (node +0h/+4h and +8h/+0Ch are the two endpoints, +10h the
    // child, +18h the sibling, +1Ch a leaf flag) and report the last crossing of
    // the segment from `a` to `b`. Writes the crossing point and returns true.
    // Gated on blk+0A3Ch being non-null (009EB778) and on the byte blk+0A24h
    // (009EB7AA).
    virtual bool avoid_zone_segment_crossing_004158e0(const std::array<float, 2>& a,
                                                      const std::array<float, 2>& b,
                                                      std::array<float, 2>& hit) = 0;

    // 009EB8C0 and 009EBE0E, CALL 009D8160 with ECX = the node: is the point
    // inside the node's avoid box? False when node+68h or node+69h is set.
    virtual bool point_in_avoid_box_009d8160(const ShipAiObstacleNode& node,
                                             const std::array<float, 2>& point) = 0;

    // 009EB8CA, CALL 009D80C0 with ECX = the node: is the point inside the
    // node's near box? False when node+68h is set. The straight probe uses it to
    // drop a neighbour the hull is already inside.
    virtual bool point_in_near_box_009d80c0(const ShipAiObstacleNode& node,
                                            const std::array<float, 2>& point) = 0;

    // 009EB8E2, CALL 009DD540 with ECX = the node, RET 0Ch: clip the ray
    // (origin, direction) against the node's avoid box. Writes the shorter
    // distance into `range` and returns true only then. The native rejects the
    // box with four support-point half-plane tests (009D8860) before clipping
    // the four edges (009D8210).
    virtual bool clip_ray_against_node_009dd540(const ShipAiObstacleNode& node,
                                                const std::array<float, 2>& origin,
                                                const std::array<float, 2>& direction,
                                                float& range) = 0;

    // 009EBDAA, CALL 00415970 with ECX = &blk+0A3Ch, RET 10h: clip the arc of
    // radius `radius` about `centre`, running from `from_bearing` toward
    // `to_bearing`, against the avoid-zone segment tree. Narrows `to_bearing`.
    virtual bool clip_arc_against_avoid_zones_00415970(const std::array<float, 2>& centre,
                                                       float radius,
                                                       float from_bearing,
                                                       float& to_bearing) = 0;

    // 009EBE36, CALL 009DD010 with ECX = the node, RET 10h: the same clip
    // against one node's avoid box. Returns false when node+68h or node+69h is
    // set (009DD043, 009DD04C).
    virtual bool clip_arc_against_node_009dd010(const ShipAiObstacleNode& node,
                                                const std::array<float, 2>& centre,
                                                float radius,
                                                float from_bearing,
                                                float& to_bearing) = 0;

    // 009EBEF7..009EBF0A, an inlined raise on the node the scan just blocked:
    //   if (value > node+78h) node+78h = value;
    // There is no CALL, so the host has to carry the compare and the store.
    virtual void raise_node_lifetime_78(ShipAiObstacleNode& node, float value) = 0;

    // 009EC0C1, CALL 009DC2E0 with ECX = &blk+0A24h: ask the avoid-zone object
    // for a bearing that clears it. Writes query.bearing and returns true.
    virtual bool avoid_zone_free_bearing_009dc2e0(ShipAiSectorFreeBearingQuery& query) = 0;
};

// ---------------------------------------------------------------------------
// 009EB660
// ---------------------------------------------------------------------------
// void __thiscall(sector)(blk), RET 4, body 009EB660-009EC277. `blk+0A3Ch` and
// `blk+0A24h` are the avoid-zone object; the two booleans below are the tests at
// 009EB778 / 009EBD4C (the pointer) and 009EB7AA / 009EBD70 (the byte).
struct ShipAiSectorScanInputs {
    ShipAiSectorHullPose pose{};
    bool avoid_zones_present{false}; // blk+0A3Ch != 0
    bool avoid_zones_enabled{false}; // blk+0A24h != 0
};

struct ShipAiSectorScanResult {
    bool blocked{false};        // sector+14h
    int  blocking_node{-1};     // the index into the neighbour list, -1 when none
    bool avoid_zone_hit{false}; // the byte at [ESP+0Eh]
    bool pass_on_max_side{false}; // the byte at [ESP+0Fh], 009D84E0's flag
};

// Writes sector+14h, +18h, +1Ch, +20h and +28h. It does not write sector+24h:
// the native stores the raw node pointer there (009EBEDB) and the caller owns
// that pointer, so the index is returned instead.
ShipAiSectorScanResult ship_ai_scan_obstacle_sector_009eb660(
    ShipAiObstacleSector& sector,
    const ShipAiSectorScanInputs& inputs,
    const std::vector<ShipAiObstacleNode*>& neighbours,
    ShipAiSectorScanHost& host);

// ---------------------------------------------------------------------------
// The neighbour list at blk+604h / blk+608h
// ---------------------------------------------------------------------------
// blk+604h is the count and blk+608h an inline array of pointers, capacity 0x80
// (009F0D39 CMP against 0x80), so the array spans blk+608h..blk+807h and ends
// exactly where the sector array begins.
inline constexpr int kShipAiNeighbourListCapacity = 0x80; // 009F0D39

// 009F0D20, void __thiscall(blk)(unit* candidate), RET 4, body
// 009F0D20-009F0E83. Sole call site 009F1A25 in BSP_ShipAi_BrainPrePass. The
// three predicate arms before the list walk are indirect calls through the
// candidate's and the own unit's vtable slot +5Ch with the literal 8, plus
// 00827F70 on [unit+538h]; they are the host's, not projected here. What is
// projected is the part after them: a node already holding this candidate has
// its lifetime raised and nothing is appended, otherwise a new node is appended.
inline constexpr double kShipAiNeighbourLifetimeSeconds = 1.2000000476837158; // 00CEC160, 009F0DD3

struct ShipAiNeighbourAddResult {
    bool appended{false};  // 009F0E62, 009F0E69
    bool refreshed{false}; // 009F0E0A on an existing node
    bool rejected{false};  // the capacity arm 009F0D43
};

// `fresh` is the node the caller has already allocated and constructed, which is
// what operator new(0x90) at 009F0E2A plus 009E52E0 at 009F0E53 do; it is
// appended only when `appended` comes back true, and the caller owns it
// otherwise. `now_plus_memory` is GameSettings+194h + 1.2 (009F0DCD, 009F0DD3).
ShipAiNeighbourAddResult ship_ai_neighbour_list_add_009f0d20(
    std::vector<ShipAiObstacleNode*>& list,
    const void* candidate_owner,
    ShipAiObstacleNode* fresh,
    float now_plus_memory) noexcept;

// 009F0EA0, void __thiscall(blk)(float dt), RET 4, body 009F0EA0-009F115D.
// Sole call site 009F51E4 in BSP_ShipAi_ControllerStep (009F50E0), which is one
// slot before the sector refresh at 009F51FA, so the list the scan walks is
// always aged and compacted first. Every
// node's lifetime is decremented by dt (009F1009..009F1018); a node that reaches
// zero is destroyed (009F1121, 009F1127) and the survivors are compacted in
// place (009F1112..009F1116) before the count is reduced (009F114E). A survivor
// whose owner is gone has node+68h set (009F110A) instead of being refreshed.
struct ShipAiNeighbourRefreshResult {
    int removed{0};   // EBP, incremented at 009F1132 inside the free gap
    int survivors{0}; // the new blk+604h
};

ShipAiNeighbourRefreshResult ship_ai_neighbour_list_refresh_009f0ea0(
    std::vector<ShipAiObstacleNode*>& list,
    float dt,
    std::vector<ShipAiObstacleNode*>& expired) noexcept;

// 009F1987..009F1A1F, the admission test in BSP_ShipAi_BrainPrePass that decides
// which units reach 009F0D20:
//   radius = (self+9C8h + other+9C8h) * 0.5
//          + max((self_class+500h + other_class+500h) * settings+19Ch,
//                settings+198h)
//   admit if |self_xz - other_xz|^2 <= radius^2
// with a prior altitude band |self+100h - other+100h| < 15.0 (00CF3F20,
// 009F1929). The candidates come from the linked list at
// [[00E188A8]+19CCh], count +60h, head +64h, payload +8h, next +4h
// (009F1877..009F18B8, 009F1A33).
inline constexpr double kShipAiNeighbourAltitudeBand = 15.0; // 00CF3F20

float ship_ai_neighbour_admission_radius_009f1987(float self_half_length_9c8,
                                                  float other_half_length_9c8,
                                                  float self_class_top_speed_500,
                                                  float other_class_top_speed_500,
                                                  float settings_lookahead_seconds_19c,
                                                  float settings_min_range_198) noexcept;

} // namespace bsp
