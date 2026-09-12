#pragma once
#include <array>
#include <cstdint>

#include "bsp/ship_ai_obstacle_tables.hpp" // ShipAiThrottleProfile, the profile
                                           // constants, kShipAiObstacleHalfTurn,
                                           // kShipAiStallNeighbourSpeed,
                                           // kShipAiObstacleHold

// The two producers the obstacle chain was missing: the turn clearance
// `blk+37Ch` (009EF910) and the 65-bin throttle cost profile `blk+4h`
// (009E04E0 through 009D67F0 / 009D56F0).
//
// Packet cc_ai_clearance_profile. Ghidra was read-only; every name below is a
// hypothesis, not a recovered symbol. docs/SHIP_AI_CLEARANCE_PROFILE.md carries
// the evidence, the coverage table and the uncertainties.
//
// Where the two routines sit in one controller step (009F50E0, the call sites
// are 009F51F3, 009F5209, 009F5227 and 009F5248):
//
//   slot 11  009F51F3  009E04E0  fills blk+4h..blk+44h and clears blk+45h
//   slot 13  009F5209  009ED6B0
//   slot 14  009F5227  009F4D10  -> 009EF350 (blk+33Ch) -> 009EF910 (blk+37Ch)
//   slot 16  009F5248  009F4DA0  -> 009F3F80 reads blk+37Ch (009F4170),
//                                  blk+33Ch (009F4517) and the profile
//                                  (009F44A7, 009F4878)
//
// so both products are made and consumed inside the same step, in that order.

namespace bsp {

// ---------------------------------------------------------------------------
// 009EF910: the turn clearance blk+37Ch
// ---------------------------------------------------------------------------
// `void __thiscall(blk)(float seconds)`, `RET 4`, body 009EF910-009F00F3,
// complete. The only caller is 009F4D10 at 009F4D87.
//
// The routine answers one question: while the hull swings from its current
// heading to the AI's commanded heading, how much room is there on the inside
// of that turn? The answer is a distance in `blk+37Ch`, and 009F3F80 divides it
// by the hull half-width `unit+9CCh` to get the danger level `blk+0A84h`
// (009F416E).

// 00CE3D64, stored at 009EF96F on every refresh. An untouched clearance is this
// value, not zero: 009F3F80's ratio is then 9999/half-width, far past the 4.0 of
// kShipAiDangerClearanceMax, so the danger level ramps to 0.0. A block that
// leaves blk+37Ch at zero reports full danger forever.
inline constexpr float kShipAiClearanceSentinel = 9999.0f;

// 00D0C26C, 009EF9C5/009EF9E6: below two degrees of heading error the routine
// returns with the sentinel still in place and blk+370h at 0.
inline constexpr float kShipAiClearanceDeadband = 0.034906585f;

// 00CE3C64, 009EFB20 and 009EFC4C: the beam bearing is the commanded heading
// plus or minus a quarter turn.
inline constexpr float kShipAiClearanceQuarterTurn = 1.5707964f;

// 00CE3830 read as a float at 009EFA25 (`FSUBR double ptr`), and 00CE3828 at
// 009EFA5B / 009EFB94.
inline constexpr float kShipAiClearanceHalfPiWide = 1.5707963705062866f;
inline constexpr float kShipAiClearanceFullTurn = 6.2831853071795862f;

// 009EFA14 `CMP EDI,3`: 009EC770 is asked for three obstacle categories and the
// neighbour's own category is the index into the answers (009EFD95).
inline constexpr int kShipAiClearanceCategoryCount = 3;

// 00CE3958 at 009F0056. Same constant as kShipAiStallNeighbourSpeed but a
// different role: the far end of the path-length fade below.
inline constexpr float kShipAiClearancePathFadeSpans = 2.0f;

// blk+370h. 009EF969 writes 0, 009F00BF writes 1, 009EFFA5 writes 2 and
// 009EFFB8 writes 3. ship_ai_obstacle_tables.hpp calls the same field
// `escape_mode_370` after 009F3F80's writers (009F4999, 009F4A02); this is the
// other writer of that field and these are the values it can leave.
enum class ShipAiClearanceOutcome : int {
    Clear = 0,          // nothing blocks the swept arc
    HeadingErrorLarge = 1, // the faded heading error passed the settings gate
    BlockedMoving = 2,  // the blocker is under way (|speed| > 2.0)
    BlockedStopped = 3, // the blocker is stopped, or a static zone blocks
};

// The two shoulder points and the axes 009DE2F0 rebuilds each step, plus the
// three per-ship constants the control-block constructor 009E4330 computes.
// Offsets are relative to blk. The producers are outside this packet; these are
// inputs, not fields this module owns.
struct ShipAiClearanceGeometry {
    // 009DE512 / 009DE524 and 009DE4F6 / 009DE500: centre +/- the beam offset.
    float shoulder_port_x{0.0f};  // +18Ch
    float shoulder_port_z{0.0f};  // +190h
    float shoulder_stbd_x{0.0f};  // +194h
    float shoulder_stbd_z{0.0f};  // +198h
    // 009DE452 / 009DE458: the hull's world forward, normalised.
    float forward_x{0.0f};        // +1ACh
    float forward_z{0.0f};        // +1B0h
    // 009E45D1: blk+3E4h / (blk+3CCh * 1.5), a half-angle in radians. The
    // sweep's leading edge is pulled back by it.
    float sweep_half_angle{0.0f}; // +1B4h
    // 009E4568: 0082E960(unit->538h, 0.9f), the hull's own bounding radius. It
    // is the circle radius of the swept-arc tests and is subtracted from every
    // measured distance.
    float hull_radius{1.0f};      // +3CCh
};

// The fields of blk that 009EF910 itself reads and writes.
struct ShipAiClearanceBlock {
    // Written every refresh.
    float refresh_timer_374{0.0f};   // +374h, 009EF91D counts down, 009EF956 resets
    float clearance_37c{kShipAiClearanceSentinel}; // +37Ch, 009EF96F and five lowerings
    ShipAiClearanceOutcome outcome_370{ShipAiClearanceOutcome::Clear}; // +370h
    float hold_354{0.0f};            // +354h, 009F00E3 raises it to kShipAiObstacleHold
    // Read only.
    float heading_target_324{0.0f};  // +324h, the AI's commanded heading
    float path_length_330{0.0f};     // +330h, the remaining path length
    int steering_mode_35c{0};        // +35Ch, 2 is the astern mode (009EF987)
    bool committed_ahead_364{false}; // +364h, 009EFFDC / 009EFFEC
    int category_3f0{3};             // +3F0h, 009EC770's own-side test
    bool avoidance_enabled_3f4{false}; // +3F4h, 009EFCA6
    // +0A3Ch, 009EF9F9 and 009EFCC9: the root of the static avoid-zone segment
    // tree. The constructor 009E4330 clears it at 009E4401, so a session with no
    // zones loaded has both static tests switched off.
    bool static_zone_present_a3c{false};
};

// 00424C40's tuning block, the three fields this routine reads.
struct ShipAiClearanceSettings {
    float refresh_period_1d4{0.0f};  // +1D4h, 009EF948
    float error_gate_committed_214{0.0f}; // +214h, 009F007F
    float error_gate_free_218{0.0f};      // +218h, 009F0089
};

// One pure-virtual per native call site 009EF910 makes. Named from the callee's
// body, never from the call site's arguments.
struct ShipAiClearanceHost {
    virtual ~ShipAiClearanceHost() = default;

    // 009EF97C, `CALL EDX` on unit->vtable[50h]. No arguments (no push before
    // the call) and the result is the hull's current world heading.
    virtual float hull_heading_vtable50() = 0;

    // 009EFA08, 009EC770(blk, category). Its body: the answer is true only when
    // blk+3F0h >= 0, the unit's gameplay object byte +241h is set, the settings
    // byte +4 is set, and the category matches blk+3F0h (3 on either side is a
    // wildcard).
    virtual bool obstacle_category_enabled_009ec770(int category) = 0;

    // 009EFCB8, 0080E160(unit) then the byte at +242h. 0080E160's whole body is
    // `return unit->+738h`, so this is a flag on the unit's gameplay object.
    virtual bool avoidance_globally_enabled_0080e160_242() = 0;

    // 009EFD00, 009D57E0(&blk+0A24h)(&pivot, radius, sweep_from, &sweep_to),
    // RET 10h. Its body tests the byte at blk+0A24h and, when set, forwards to
    // 00415970 with `this` = blk+0A3Ch (009D57FE `ADD ECX,0x18`). 00415970
    // walks the static avoid-zone segment tree and returns the first segment
    // whose distance to the circle is inside the radius. True means the swept
    // arc is already fouled by terrain or a no-go zone.
    virtual bool static_zone_blocks_009d57e0(float pivot_x, float pivot_z,
                                             float radius,
                                             float sweep_from, float sweep_to) = 0;

    // 009EFD4A, 00415D70(&blk+0A3Ch)(&pivot, radius, &dir_a, &dir_b, sweep_from,
    // &sweep_to), RET 18h. Only the first four arguments are used by the body:
    // it walks the same segment tree, clips each segment to the wedge with the
    // two directions as inward normals through the pivot, and returns the
    // smallest (distance - radius) over the survivors, or FLT_MAX (00D7A248)
    // when nothing survives.
    virtual float static_zone_clearance_00415d70(float pivot_x, float pivot_z,
                                                 float radius,
                                                 float dir_a_x, float dir_a_z,
                                                 float dir_b_x, float dir_b_z) = 0;

    // 009EFD5B / 009EFDD0, `blk+604h` and the pointer array at `blk+608h`.
    virtual int neighbour_count_604() = 0;

    // 009EFD80..009EFD9D. `owner` is neighbour->+14h; the neighbour is skipped
    // when it is null, when owner->+5Eh is set, or when its category
    // owner->+54h is not one of the three the host enabled.
    virtual bool neighbour_owner_present_14(int index) = 0;
    virtual bool neighbour_owner_gone_5e(int index) = 0;
    virtual int neighbour_owner_category_54(int index) = 0;

    // 009EFDBD, 009DD010(neighbour)(&pivot, radius, sweep_from, &sweep_to),
    // RET 10h, the same signature as 009D57E0. Its body builds the four corners
    // of the neighbour's footprint rectangle and answers whether that rectangle
    // meets the circle of `radius` swept between the two bearings.
    virtual bool neighbour_blocks_sweep_009dd010(int index,
                                                 float pivot_x, float pivot_z,
                                                 float radius,
                                                 float sweep_from, float sweep_to) = 0;

    // 009EFE0F and 009EFE69, 009D8860(neighbour)(out, dir), RET 8. Its body
    // returns the corner of the footprint rectangle that is extreme along
    // `dir`: centre (+44h,+48h) plus or minus half-extent +5Ch along the axis
    // (+4Ch,+50h) plus or minus half-extent +60h along the axis (+54h,+58h),
    // each sign taken from the sign of the dot product. When the byte at +68h is
    // set the footprint is a point and the centre is returned.
    virtual void neighbour_support_point_009d8860(int index,
                                                  float dir_x, float dir_z,
                                                  float& out_x, float& out_z) = 0;

    // 009EFEC3, 009D8A30(neighbour)(out, point), RET 8. The same rectangle, the
    // point on it closest to `point`: the two local coordinates clamped into
    // [-half, +half] and mapped back.
    virtual void neighbour_closest_point_009d8a30(int index,
                                                  float point_x, float point_z,
                                                  float& out_x, float& out_z) = 0;

    // 009EFF7F, 0092D730 on owner->+1018h, the blocker's signed body-axis speed.
    virtual float neighbour_speed_0092d730(int index) = 0;

    // 009F0000, 00778890(), and 009F0019, the vtable identity test on
    // unit->+738h->+54h against 00E08F80. Together they are the one gate on the
    // path-length fade; the body of 00778890 is a plain global read, so the pair
    // is modelled as one predicate.
    virtual bool path_fade_applies_00778890() = 0;

    // 009F0038, 00811A30(unit, 1.0f). Its body scales the argument by the ship
    // class curve 0082E960 and divides by the gameplay modifier product for
    // channel 5, so with 1.0f it is one class-scaled unit of length.
    virtual float class_length_unit_00811a30() = 0;
};

// 009EFAAF (ahead) and 009EFBDD (astern). Ahead, a heading error at or below
// zero turns to port and uses the port shoulder; astern the test and the arm
// are both mirrored. `true` selects the port shoulder (blk+18Ch/+190h) and the
// adding arm; `false` selects the starboard shoulder (blk+194h/+198h) and the
// subtracting arm.
bool ship_ai_clearance_uses_port_shoulder_009efaaf(bool astern_mode,
                                                   float heading_error) noexcept;

// 009EFB03..009EFC8D. The arc the hull's beam sweeps while it finishes the turn,
// and whether any of that sweep is still ahead of it.
struct ShipAiClearanceSweep {
    float from{0.0f};    // the rotated hull heading, blk+18h in the listing
    float to{0.0f};      // the commanded beam bearing, blk+1Ch in the listing
    bool pending{false}; // BL == 0 at 009EFC8D: the sweep has not been made yet
};
ShipAiClearanceSweep ship_ai_clearance_sweep_009efb03(bool port_arm,
                                                      float hull_heading,
                                                      float heading_target,
                                                      float sweep_half_angle) noexcept;

// 009EFEED..009EFF44. The clearance one neighbour contributes: the distance
// from the pivot to the closest point of its footprint, less the hull radius.
// Returns kShipAiClearanceSentinel-sized "no contribution" as the incoming
// `current` when the footprint is inside the hull circle or behind either plane.
float ship_ai_clearance_from_closest_point_009efeed(float pivot_x, float pivot_z,
                                                    float closest_x, float closest_z,
                                                    float hull_radius,
                                                    float current) noexcept;

// 009F0022..009F0072. Near the end of the path the heading error is faded out so
// a ship that is almost there does not report a heading complaint.
float ship_ai_clearance_faded_error_009f0022(float heading_error,
                                             float path_length,
                                             float class_length_unit) noexcept;

// The whole routine. `seconds` is the frame delta the caller pushes at 009F4D87.
void ship_ai_refresh_turn_clearance_009ef910(ShipAiClearanceBlock& blk,
                                             const ShipAiClearanceGeometry& geometry,
                                             const ShipAiClearanceSettings& settings,
                                             float seconds,
                                             ShipAiClearanceHost& host);

// ---------------------------------------------------------------------------
// 009D56F0 and 009D67F0: the only writers of the 65 bins
// ---------------------------------------------------------------------------
// `void __thiscall(profile)(float low, float high, char cost)`, `RET 0Ch`, body
// 009D56F0-009D575C, complete. `009D67F0` is
// `void __thiscall(blk)(float low, float high)`, `RET 8`, body
// 009D67F0-009D680C: it does `ADD ECX,0x4` (009D67FD) and forwards with cost 1,
// which is why the block itself is the `this` at the three call sites in
// 009E04E0 while the profile lives at blk+4h.
//
// The cost is *added*, not assigned (009D5750 `ADD byte ptr [ESI+EDI],CL`), and
// every call clears the bypass byte at profile+41h (009D5748), which the control
// block constructor 009E4330 sets to 1 at 009E435F. Until some band is written
// the profile is in bypass and 009D6B40 only clamps.

// 009D56F0's own index, distinct from the reader's at 009D6B87: it truncates
// with 00BF7420 and does not add the 0.5 the reader adds.
int ship_ai_throttle_band_index_009d56f0(float value) noexcept;

// The whole of 009D56F0.
void ship_ai_add_throttle_band_009d56f0(ShipAiThrottleProfile& profile,
                                        float low, float high,
                                        std::int8_t cost) noexcept;

// The whole of 009D67F0.
void ship_ai_mark_throttle_band_009d67f0(ShipAiThrottleProfile& profile,
                                         float low, float high) noexcept;

// ---------------------------------------------------------------------------
// 009E04E0: the profile producer
// ---------------------------------------------------------------------------
// `void __thiscall(blk)(float seconds)`, `RET 4`, body 009E04E0-009E1167. The
// only caller is 009F50E0 at 009F51F3, chain slot 11.
//
// The routine walks the contact-track list at blk+404h (count blk+400h). For a
// track whose course crosses ours it works out the window of own-ship speeds
// that would put us in the crossing at the moment the contact gets there,
// divides that window by the reference speed blk+3C4h to make it a throttle
// window, and adds cost 1 to the bins inside it. A track that cannot be avoided
// by throttle alone instead contributes to an avoidance vector in
// blk+34Ch / blk+350h and to two counters. At the end, if either counter fired,
// a cost-10 band is laid over the half of the axis the ship is not currently on.

// 00CE7D7C and 00CE3958, the ends of the axis as this routine uses them.
inline constexpr float kShipAiThrottleAxisLow = -2.0f;  // 00CE7D7C, 009E0C10
inline constexpr float kShipAiThrottleAxisHigh = 2.0f;  // 00CE3958, 009E0BD1
// 00CE69D0, 009E0B70: a window whose low end is below this reaches the astern
// end of the axis and is widened to it.
inline constexpr float kShipAiThrottleWindowAsternEdge = -0.5f;
// 00CF0B58 as a double, 009E0C1E: after a widened band, a window that does not
// reach this height stops the track here.
inline constexpr float kShipAiThrottleWindowSettled = 0.85f;
// 00CE6650 and 00D21858, 009E10FD and 009E1142: the inner edges of the two
// commitment bands.
inline constexpr float kShipAiCommitAheadEdge = 0.98f;
inline constexpr float kShipAiCommitAsternEdge = -0.48f;
// 009E10ED and 009E111D `PUSH 0xA`.
inline constexpr std::int8_t kShipAiCommitBandCost = 10;
// 00D20A18 and 00D2185C, 009E073C and 009E0775: outside this band of relative
// bearing the track is handled by the same-course arm at 009E0C52 instead.
inline constexpr float kShipAiContactBearingMin = 0.13962634f; // 8 degrees
inline constexpr float kShipAiContactBearingMax = 3.0019646f;  // 172 degrees
// 00CEFF98, 00CE3DC0 and 00CE3D88, 009E08F5, 009E090D and 009E0755.
inline constexpr float kShipAiContactBeamFraction = 0.6f;
inline constexpr float kShipAiContactLengthPad = 10.0f;
inline constexpr float kShipAiContactWidthPad = 20.0f;
// 00CE3D78 at 009E0521 and 00CEE07C at 009E06FF.
inline constexpr float kShipAiContactHalfWidthDivisor = 1.5f;
inline constexpr float kShipAiContactGapLengthArg = 0.75f;
// 00D7A24C at 009E0867 and 009E068C.
inline constexpr float kShipAiContactMinHorizon = 1.0f;

// One tracked contact. The fields are the ones 009E04E0 reads; 009DC060
// (009E0631) is the producer of +18h..+30h from the tracked entity, which is
// why the direction pair is already normalised and the speed already carries the
// +1Ch margin.
struct ShipAiContactTrack {
    float half_length_00{0.0f}; // +0h,  009E08E9, the contact's own half length
    float lead_time_04{0.0f};   // +4h,  009E0834, subtracted from the horizon
    float max_horizon_0c{0.0f}; // +0Ch, 009E0851, the horizon cap
    float range_10{0.0f};       // +10h, 009E0606, against target->+488h
    float lifetime_14{0.0f};    // +14h, 009E05C7 counts it down by `seconds`
    float speed_18{0.0f};       // +18h, 009E085C, set by 009DC060
    float heading_20{0.0f};     // +20h, 009E0636
    float dir_x_24{0.0f};       // +24h, 009E0670
    float dir_z_28{0.0f};       // +28h, 009E0663
    float pos_x_2c{0.0f};       // +2Ch, 009E0683
    float pos_z_30{0.0f};       // +30h, 009E06A3
    bool retired_64{false};     // +64h, 009E05E5 tests, 009E07C8 sets
};

// The hull-frame inputs 009E04E0 reads once per step.
struct ShipAiThrottleProfileInputs {
    float reference_speed_3c4{1.0f}; // +3C4h, 009E0519, the throttle divisor
    float own_speed{0.0f};           // 009E053A, 0092D730 on unit->+1018h
    float acceleration{0.0f};        // 009E054F, unit->538h->+504h
    float hull_half_width{0.0f};     // 009E0513, unit->+9CCh
    float hull_beam{0.0f};           // 009E08EF, unit->538h->+A0h
    float position_x{0.0f};          // +184h, 009E0693
    float position_z{0.0f};          // +188h, 009E06A6
    float normal_x{0.0f};            // +19Ch, 009E0793, the port beam normal
    float normal_z{0.0f};            // +1A0h, 009E0785
    float forward_x{0.0f};           // +1ACh, 009E08C1
    float forward_z{0.0f};           // +1B0h, 009E08AA
};

// The block fields 009E04E0 writes.
struct ShipAiThrottleProfileBlock {
    ShipAiThrottleProfile profile{}; // blk+4h
    float avoid_x_34c{0.0f};         // +34Ch, 009E0562 clears, the arms add
    float avoid_z_350{0.0f};         // +350h, 009E056A
    float hold_354{0.0f};            // +354h, 009E0591 counts down while >= 0
};

// One pure-virtual per native call site that is not a pure rule.
struct ShipAiThrottleProfileHost {
    virtual ~ShipAiThrottleProfileHost() = default;

    // 009E0509, unit->vtable[50h] with no arguments. The routine calls it and
    // pops the result at 009E050B without using it; the call is kept because it
    // is the point the pose refresh can happen.
    virtual float hull_heading_vtable50() = 0;

    // 009E0613, the track's own range +10h against target->+488h. The target is
    // track->+48h; when that pointer is null the test is skipped.
    virtual bool track_range_within_target_488(int index) = 0;

    // 009E061B, 009DA1D0(blk). Its body: false when the unit answers the
    // vtable[5Ch] query with 0Eh, false when the unit's world height +100h is
    // below 00CE3D58, otherwise blk+3ECh and the gameplay byte +240h.
    virtual bool avoidance_active_009da1d0() = 0;

    // 009E0631, 009DC060(track). Its body refreshes +18h..+30h of the track
    // from the tracked entity's pose and velocity and returns whether the track
    // survived.
    virtual bool refresh_track_009dc060(int index) = 0;

    // 009E0723, 00811A30(unit, 0.75f), the class-scaled length used to size the
    // lateral gap. Same callee as the clearance host's, different argument.
    virtual float class_length_three_quarters_00811a30() = 0;

    // 009E05C0 and 009E1057, the list at blk+404h with the count at blk+400h.
    virtual int track_count_400() = 0;
    virtual ShipAiContactTrack& track_at(int index) = 0;
    // 009E0FBD..009E107E: the expired or invalid track is destroyed and the last
    // entry moves into its slot.
    virtual void destroy_track(int index) = 0;
    // 009E05EF / 009E05F5: a track with neither observer pair is dropped.
    virtual bool track_has_source(int index) = 0;
};

// 009E07D1..009E085C. The time the contact needs to reach our track line,
// floored at zero and capped by the track's own horizon.
struct ShipAiContactHorizon {
    float distance{0.0f}; // 009E0837, (|lateral| - half_width)/|sin| - lead_time
    float seconds{0.0f};  // 009E087F, max(1.0, distance / speed)
    bool usable{false};   // 009E0856, distance < max_horizon_0c
};
ShipAiContactHorizon ship_ai_contact_horizon_009e07d1(float lateral,
                                                      float hull_half_width,
                                                      float heading_error,
                                                      const ShipAiContactTrack& track) noexcept;

// 009E0923..009E0B37. The window of own-ship throttles that put the hull inside
// the contact's path within the horizon. `low` and `high` are already divided by
// the reference speed.
struct ShipAiThrottleWindow {
    float low{0.0f};
    float high{0.0f};
};
ShipAiThrottleWindow ship_ai_contact_throttle_window_009e0923(float along_distance,
                                                              float margin,
                                                              float horizon_seconds,
                                                              float own_speed,
                                                              float acceleration,
                                                              float reference_speed) noexcept;

// 009E0953..009E09E4 and its mirror at 009E0A30..009E0AF0: the constant-
// acceleration correction both ends share. Returns `average` unchanged when the
// guard fails or the discriminant is negative.
float ship_ai_contact_speed_root_009e0953(float average, float distance,
                                          float own_speed, float acceleration,
                                          float horizon_seconds,
                                          bool upper_end) noexcept;

// The counters the per-track arm feeds and the tail reads.
struct ShipAiContactCounts {
    int banded_84{0};   // [ESP+3Ch], 009E0BD7, 009E0BF9: a real band was written
    int give_way_80{0}; // [ESP+40h], 009E0BBB
    int stand_on_74{0}; // [ESP+4Ch], 009E0BC5
};

// 009E0B37..009E0C4D, the band decision for one contact.
enum class ShipAiContactBandOutcome {
    OffAxis,          // 009E0B4C / 009E0B63: the window misses the axis
    Banded,           // a band was written and the track is finished
    BandedContinue,   // 009E0C1E: widened band, the track carries on to the arms
    NotThrottleable,  // 009E0BB9: the counters take it instead
};
ShipAiContactBandOutcome ship_ai_apply_contact_band_009e0b37(ShipAiThrottleProfile& profile,
                                                             ShipAiThrottleWindow window,
                                                             ShipAiContactCounts& counts,
                                                             bool& stand_on) noexcept;

// 009E10A9..009E114B. When either counter fired, the half of the axis the ship
// is not committed to gets cost 10, which biases 009D6B40 away from crossing
// zero. `snapped_ratio` is the value after the 009D6B40 call at 009E10E2, and
// the `banded == 0` arm never makes that call and uses the counter difference
// instead.
ShipAiThrottleWindow ship_ai_commitment_band_009e10a9(const ShipAiContactCounts& counts,
                                                      float throttle_ratio,
                                                      float snapped_ratio) noexcept;

// 009E0C7C..009E0FBD. A track the throttle cannot dodge contributes a unit
// vector to blk+34Ch / blk+350h instead. The three arms are picked by the
// relative bearing: past a quarter turn and past 172 degrees the contact's own
// left normal is used directly; between them, and inside a quarter turn, the
// lateral gap decides between the normal and twice the contact's course.
//
// 00CE7630, 009E0D22: the band the gap has to land in is
// [stand_on ? -(30 + width) : 0, width + 30).
inline constexpr float kShipAiAvoidBandPad = 30.0f;
struct ShipAiAvoidanceStep {
    float x{0.0f};
    float z{0.0f};
};
ShipAiAvoidanceStep ship_ai_contact_avoidance_step_009e0c7c(float heading_error,
                                                            float lateral_side,
                                                            float gap,
                                                            float width,
                                                            bool stand_on,
                                                            float dir_x,
                                                            float dir_z) noexcept;

// The whole routine, over the host.
void ship_ai_build_throttle_profile_009e04e0(ShipAiThrottleProfileBlock& blk,
                                             const ShipAiThrottleProfileInputs& inputs,
                                             float seconds,
                                             ShipAiThrottleProfileHost& host);

} // namespace bsp
