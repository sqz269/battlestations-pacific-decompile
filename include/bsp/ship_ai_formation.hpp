// The unit group (the ship "formation" object at entity+284h): its layout, its
// three producers of the member record, and the whole of the follow step's
// station producer 009DF2D0.
//
// Addresses: 0070DAB0, 0070D7B0, 0070DB20, 0070EF30, 0070ED30, 0070EFD0,
// 0070E620, 0070ECF0, 0070E4C0, 0070ECA0, 0070DA00, 0070D030, 0070D070,
// 0070D080, 0070D0C0, 0070D0F0, 0070D100, 0070D140, 0070D1B0, 0070E450,
// 0070D290, 00811150, 00811180, 00810630, 009DACD0, 009DF2D0, 0077F940,
// 00E08FE0, 00E090A8, 00E09170.
//
// Packet cc_ai_formation. Every descriptive name here is a hypothesis, not a
// recovered symbol. docs/SHIP_AI_FORMATION.md carries the evidence per line.
//
// The object is 508h bytes (operator_new(0x508) at 0070DB29 and 0070ECF9).
// bsp/ship_ai_path_corridor.hpp already declares its five offsets and the
// 34h-byte member record; this file never redefines them, it only adds the
// fields that file left unread (+4h relativePosition, +30h published speed)
// and the group-wide fields past the record array.
//
// The two column meanings that bsp/ship_ai_path_corridor.hpp had to leave
// provisional are settled here by the producers:
//   * record+10h + 4*col is the member's **across-track** offset, signed. It
//     is the second out-parameter of 00811180 (0070EEB6 stores it), which is
//     the perpendicular component of the member's position against the
//     leader's wake, signed by the cross product at 008117xx; and it is
//     table column `x` in the Lua reader (0070E803 pushes "x", 00CEB488).
//   * record+20h + 4*col is the member's **along-track** distance back along
//     the leader's wake. It is the third out-parameter of 00811180 (0070EEB1
//     stores it), an accumulated arc length; it is table column `z`
//     (0070E82C pushes "z", 00CFD718); and 0070D290 hands it straight to
//     00810630 as the distance to walk back along the trail.
//   * exactly four columns exist: the Lua `dist` loop runs four times
//     (0070E84B CMP EDI,4) and 0070ED30 writes exactly four pairs.
#pragma once

#include <array>
#include <cstddef>
#include <cstdint>

#include "bsp/ship_ai_follow_land.hpp"
#include "bsp/ship_ai_path_corridor.hpp"

namespace bsp {

// ---------------------------------------------------------------------------
// The rest of the unit group object
// ---------------------------------------------------------------------------

// +04h..+0Fh, the member's position in the leader's local frame, clamped to
// the FollowerMaxDist radius. 0070ED30 writes it at 0070EDB9/0070EDC3/0070EDCE;
// the Lua reader reads it as `relativePosition` (00CFD724, type code 5, at
// 0070E78D); the session message path copies msg+0CCh..+0D4h into it at
// 0077FB9A when the group type is 18h.
inline constexpr std::uint32_t kShipAiFormationMemberRelative = 0x0004u;
// +30h, the per-member speed a follower publishes back into its own record
// (0070D100, called from 009DF6AA) and the group throttle ceiling reduces
// (0070D140, called from 009F4E1F in BSP_ShipAi_ThrottleCeilingStep).
inline constexpr std::uint32_t kShipAiFormationMemberSpeed = 0x0030u;
// +4FCh, the formation "type". 0070D7B0 sets 6 for a ship-led runtime group
// (0070D842); the Lua reader takes it from the field `type` (00CE4AD0, at
// 0070E679); the session path sets it from a message byte at 0077FB3F and
// treats 18h as "the message carries relativePosition, not the columns".
inline constexpr std::uint32_t kShipAiFormationType = 0x04FCu;
// +504h, the group speed ceiling. The Lua reader takes it from `maxSpeed`
// (00CFD74C, type code 2, at 0070E6CF) but 0070DA00 recomputes it as the
// minimum of the members' own class maxima on every join and leave, and
// 0070D0F0 is its getter (009F4E34 reads it).
inline constexpr std::uint32_t kShipAiFormationMaxSpeed = 0x0504u;
// The allocation size, operator_new(0x508) at 0070DB29 and 0070ECF9.
inline constexpr std::size_t kShipAiFormationObjectSize = 0x508u;
// (4F8h - 18h) / 34h = 24 records fit before the count, and the constructor
// 0070DAB0 clears exactly 24 of them (0070DAF3 seeds the counter with 17h and
// the loop runs while the post-decrement value is >= 0).
inline constexpr std::size_t kShipAiFormationMemberCapacity = 24u;

// ---------------------------------------------------------------------------
// The wake trail the station point is measured along
// ---------------------------------------------------------------------------
//
// 00811180 reads it with the entity as `this` (entity+0BD8h, head index at
// entity+0F98h); 00810630 reads the same buffer with entity+0BD0h as `this`
// (00811150 adds 0BD0h at 00811164), so its offsets are 0BD0h lower.

inline constexpr std::uint32_t kShipAiWakeSamples = 0x0BD8u;      // 00811180
inline constexpr std::uint32_t kShipAiWakeHeadIndex = 0x0F98u;    // 00811180
inline constexpr std::uint32_t kShipAiWakeSampleStride = 0x18u;   // *0x18 throughout
inline constexpr std::size_t kShipAiWakeSampleCount = 0x28u;      // % 0x28, 0081073x

// One 18h-byte trail sample. Offsets are relative to the sample base.
struct ShipAiWakeSample {
    float x{0.0f};           // +00h, 008106DD / 00811180's distance terms
    float y{0.0f};           // +04h
    float z{0.0f};           // +08h
    float heading{0.0f};     // +0Ch, 0081064E FLD [ECX+EAX*8+14h] with base 0BD0h
    float segment{0.0f};     // +10h, the arc length to the next sample
    float yaw_rate{0.0f};    // +14h, 008108D7 / 008108E2, what 00810630 blends
                             //       into its fifth out-parameter
};
static_assert(sizeof(ShipAiWakeSample) == 0x18u,
              "the trail sample is 18h bytes: every index is scaled by 0x18");

// What 00810630 answers for one point on the trail. The names are the uses at
// the two call sites, not recovered symbols.
struct ShipAiWakePoint {
    float x{0.0f};           // param_3[0], 00810705 / 008108xx
    float y{0.0f};           // param_3[1]
    float z{0.0f};           // param_3[2]
    float dir_x{0.0f};       // param_4[0] = cos(wrap_2pi(pi/2 - heading))
    float dir_y{0.0f};       // param_4[1], always 0 (008106AE XORPS)
    float dir_z{0.0f};       // param_4[2] = sin(...)
    float yaw_rate{0.0f};    // param_5, written **only** on the along > 0 path
    bool yaw_rate_written{false};  // 00810645..00810718 never stores param_5
};

// ---------------------------------------------------------------------------
// The three canned formation patterns
// ---------------------------------------------------------------------------
//
// Three tables of 25 (x, z) float pairs, read at 0070EED0/0070EEF0/0070EF08
// (0070ED30, indexed by the member's join index) and at 0070F04x (0070EFD0,
// base 00E08F18 + shape*0C8h, indexed by a leader-skipping counter). Only 24
// members can exist, so entry 24 of each table is unreachable; it is kept here
// because the stride 0C8h between the three tables proves the table length.
//
// The HUD menu names the three (00536A90: "ingame.formation_line",
// "ingame.formation_column", "ingame.formation_diamond", plus a fourth entry
// "ingame.formation_disband" that is not a shape), and the table contents
// match those names: shape 1 alternates across, shape 2 alternates along, and
// shape 3 puts one member ahead, two on the flanks slightly ahead and one
// astern before degenerating into a line ahead.

enum class ShipAiFormationShape : int {
    kCustom = 0,   // 0070DAB0 leaves +500h zero; column 0 is the live column
    kLine = 1,     // 00E08FE0
    kColumn = 2,   // 00E090A8
    kDiamond = 3,  // 00E09170
};

// One table entry, in units of FormationShipDist.
struct ShipAiFormationOffset {
    float across{0.0f};  // the `x` column of the table
    float along{0.0f};   // the `z` column of the table
};

inline constexpr std::size_t kShipAiFormationPatternLength = 25u;

// 00E08FE0, shape 1. Alternates right and left of the leader, all along = 0.
inline constexpr std::array<ShipAiFormationOffset, kShipAiFormationPatternLength>
    kShipAiFormationPatternLine{{
        {0.0f, 0.0f},   {1.0f, 0.0f},   {-1.0f, 0.0f},  {2.0f, 0.0f},
        {-2.0f, 0.0f},  {3.0f, 0.0f},   {-3.0f, 0.0f},  {4.0f, 0.0f},
        {-4.0f, 0.0f},  {5.0f, 0.0f},   {-5.0f, 0.0f},  {6.0f, 0.0f},
        {-6.0f, 0.0f},  {7.0f, 0.0f},   {-7.0f, 0.0f},  {8.0f, 0.0f},
        {-8.0f, 0.0f},  {9.0f, 0.0f},   {-9.0f, 0.0f},  {10.0f, 0.0f},
        {-10.0f, 0.0f}, {11.0f, 0.0f},  {-11.0f, 0.0f}, {12.0f, 0.0f},
        {-12.0f, 0.0f},
    }};

// 00E090A8, shape 2. Alternates astern and ahead, all across = 0.
inline constexpr std::array<ShipAiFormationOffset, kShipAiFormationPatternLength>
    kShipAiFormationPatternColumn{{
        {0.0f, 0.0f},  {0.0f, 1.0f},   {0.0f, -1.0f},  {0.0f, 2.0f},
        {0.0f, -2.0f}, {0.0f, 3.0f},   {0.0f, -3.0f},  {0.0f, 4.0f},
        {0.0f, -4.0f}, {0.0f, 5.0f},   {0.0f, -5.0f},  {0.0f, 6.0f},
        {0.0f, -6.0f}, {0.0f, 7.0f},   {0.0f, -7.0f},  {0.0f, 8.0f},
        {0.0f, -8.0f}, {0.0f, 9.0f},   {0.0f, -9.0f},  {0.0f, 10.0f},
        {0.0f, -10.0f},{0.0f, 11.0f},  {0.0f, -11.0f}, {0.0f, 12.0f},
        {0.0f, -12.0f},
    }};

// 00E09170, shape 3. Entry 12 repeats entry 2; that duplication is in the
// image, not a transcription error.
inline constexpr std::array<ShipAiFormationOffset, kShipAiFormationPatternLength>
    kShipAiFormationPatternDiamond{{
        {0.0f, 0.0f},   {0.0f, -1.0f},  {-1.0f, -0.5f}, {1.0f, -0.5f},
        {0.0f, 1.0f},   {0.0f, -2.0f},  {-1.0f, 0.5f},  {1.0f, 0.5f},
        {-1.0f, -1.5f}, {1.0f, -1.5f},  {-2.0f, -0.5f}, {2.0f, -0.5f},
        {-1.0f, -0.5f}, {0.0f, -3.0f},  {0.0f, -4.0f},  {0.0f, -5.0f},
        {0.0f, -6.0f},  {0.0f, -7.0f},  {0.0f, -8.0f},  {0.0f, -9.0f},
        {0.0f, -10.0f}, {0.0f, -11.0f}, {0.0f, -12.0f}, {0.0f, -13.0f},
        {0.0f, -14.0f},
    }};

// The table for one shape, or nullptr for kCustom (0070EFD0 with shape 0 would
// index 00E08F18, which holds pointers, not floats; no read call site passes
// 0, so this file refuses it instead of reproducing the out-of-range read).
const std::array<ShipAiFormationOffset, kShipAiFormationPatternLength>*
ship_ai_formation_pattern(ShipAiFormationShape shape) noexcept;

// ---------------------------------------------------------------------------
// Literals, all read from the image at the addresses given
// ---------------------------------------------------------------------------

// 00CF4888, the float the constructor (0070DAF9), 0070D7B0 (0070D7EC),
// 0070EF30 (0070EF7A) and 0070DA00 (0070DA1D) put into record+30h.
inline constexpr float kShipAiFormationMemberSpeedUnset = 999.0f;
// 00CFD6F4, the seed of the 0070D140 minimum (0070D148) and of the 0070DA00
// maximum-speed reduction (0070DA06).
inline constexpr float kShipAiFormationSpeedSeed = 9999999.0f;
// 0070EFD0 scales the `z` column by this extra factor (00CE3D78, a double, at
// 0070F057). 0070ED30 does **not**: its four table columns are scaled by
// FormationShipDist alone (0070EED7).
inline constexpr double kShipAiFormationReshapeAlongScale = 1.5;
// The kind a leader must answer for the ship branch of 0070DA00 / 0070D1B0
// (the literal 6 pushed at 0070DA35 / 0070D1BB). Named separately from
// `kShipAiFollowLeaderKind` only in the doc; the value is the same constant.

// shipglobals.lua, loaded by BSP_GameSettings_LoadFromLuaGlobals 0083B5E0:
// "FormationMaxCount" -> settings+420h (0083F0F4), "FollowerMaxDist" ->
// settings+424h (0083F02E), "FormationShipDist" -> settings+42Ch (0083F070).
inline constexpr float kShipAiFormationMaxCountDefault = 24.0f;    // = 24
inline constexpr float kShipAiFollowerMaxDistDefault = 4000.0f;
inline constexpr float kShipAiFormationShipDistDefault = 250.0f;

// 009DF62E, the float at 00CFD710: the along-track distance at which the
// follower's speed blend reaches the wake-point ratio.
inline constexpr float kShipAiFormationSpeedBlendDistance = 400.0f;
// 009DF664, the float at 00CE3868: the floor under the blend before it divides
// the reference speed.
inline constexpr float kShipAiFormationSpeedBlendFloor = 0.25f;
// 009DF68F, the double at 00CF87C0: the reference speed is scaled by this
// before the divide. `kShipAiFollowRequestRadiusScale` in
// bsp/ship_ai_follow_land.hpp is the same literal used for a different purpose.
inline constexpr double kShipAiFormationPublishSpeedScale = 1.25;
// 009DACD0's two gates: the double at 00D7A268 on |yaw rate| and the float at
// 00D7A24C on |speed|, and the value returned when either gate fails (the same
// 00D7A24C). bsp/ship_ai_path_corridor.hpp spells 00D7A24C
// `kShipAiGroupExtentSeed` for its own use; this file does not redefine it.
inline constexpr double kShipAiFormationTurnRateEpsilon = 9.9999997473787516e-05;
inline constexpr float kShipAiFormationSpeedRatioUnity = 1.0f;
inline constexpr float kShipAiFormationSpeedGate = 1.0f;
// 009DF41C / 009DF4BE, the float at 00CE3930: the margin 00417B10 pushes the
// station point out of the class's zone set by. Spelled
// `kShipAiPathCorridorWidthDefault` in bsp/ship_ai_path_refresh.hpp for the
// corridor use of the same literal; this file does not redefine that name.
inline constexpr float kShipAiFormationZoneMargin = 20.0f;
// 009DF4D6, the squared distance above which 009DF2D0 re-derives the slot
// direction from its two pushed-out points (the literal 1.0 at 009DF4DE).
inline constexpr float kShipAiFormationDirectionRederiveDistanceSq = 1.0f;

// ---------------------------------------------------------------------------
// Pure rules
// ---------------------------------------------------------------------------

// 0070ED30's table half, 0070EED0-0070EF1B: the four columns of one member
// record computed from the three tables at the member's join index. Column 0
// is **not** produced here; it comes from the wake decomposition (see
// ShipAiFormationSlotHost).
struct ShipAiFormationColumns {
    std::array<float, kShipAiUnitGroupColumnCount> across{};
    std::array<float, kShipAiUnitGroupColumnCount> along{};
};

// 0070EED0..0070EF1B. `index` is the member's join index; `ship_dist` is
// settings+42Ch. Columns 1..3 only; column 0 is left at its incoming value.
void ship_ai_formation_fill_table_columns(ShipAiFormationColumns& columns,
                                          std::size_t index,
                                          float ship_dist) noexcept;

// 0070EFD0's per-member half, 0070F03A-0070F063: column 0 of one member from
// one table, with the extra 1.5 on the along column. `slot` is the
// leader-skipping counter, which starts at 1 (0070F00B MOV ESI,1... the
// increment at 0070F038).
ShipAiFormationOffset ship_ai_formation_reshape_slot(ShipAiFormationShape shape,
                                                     std::size_t slot,
                                                     float ship_dist) noexcept;

// The seven floats 0070D290 writes through its second argument.
struct ShipAiFormationStation {
    float x{0.0f};        // +00h, 0070D342
    float z{0.0f};        // +04h, 0070D348
    float dir_x{0.0f};    // +08h, 0070D34B
    float dir_z{0.0f};    // +0Ch, 0070D356
    float yaw_rate{0.0f}; // +10h, written by 00810630 only when along > 0
    float across{0.0f};   // +14h, 0070D2C9
    float along{0.0f};    // +18h, 0070D2EA
    bool yaw_rate_valid{false};  // see ShipAiWakePoint::yaw_rate_written
};

// 0070D2F9-0070D356, the member branch's arithmetic once the wake point is in
// hand: the station is the wake point pushed sideways by `across` along the
// left-hand normal of the wake direction.
ShipAiFormationStation ship_ai_formation_station_from_wake(
    const ShipAiWakePoint& wake, float across, float along) noexcept;

// 0070D362-0070D3F8, the leader branch: the leader answers its own world
// position and (cos, sin) of wrap_2pi(pi/2 - heading), with all three offsets
// zeroed (0070D3E1, 0070D3E9, 0070D3F1).
ShipAiFormationStation ship_ai_formation_station_for_leader(float x, float z,
                                                            float heading) noexcept;

// 009DACD0, body 009DACD0-009DAD9F, read complete. __cdecl(yaw_rate, speed,
// across), RET 0Ch, ST0 result. The ratio between the arc radius the member
// must fly and the one the leader is flying: 1 when the leader is not turning
// or is stopped, otherwise (R - across) / R for a left turn (yaw_rate > 0) and
// (R + across) / R for a right turn, with R = |speed / yaw_rate|.
float ship_ai_formation_speed_ratio(float yaw_rate, float speed,
                                    float across) noexcept;

// 009DF61B-009DF64A. The follower blends the ratio taken from the leader's
// **current** yaw rate (at along = 0) into the ratio taken from the yaw rate
// recorded at the **wake point it is sitting on** (at along >= 400), through
// BSP_Math_InterpolateClamped 00419010.
float ship_ai_formation_speed_blend(float ratio_now, float ratio_at_wake,
                                    float along) noexcept;

// 009DF664-009DF69E. What the follower publishes into its own record+30h.
float ship_ai_formation_published_speed(float reference_speed,
                                        float blend) noexcept;

// 0070D140, body 0070D140-0070D19F, read complete. __fastcall(group), RET 0,
// ST0 result. The group throttle ceiling: the minimum of record+30h over
// members with a live entity, seeded at 9999999. A record whose entity is null
// is **written** back to 999 and skipped, so this routine mutates the array.
float ship_ai_formation_speed_ceiling(ShipAiUnitGroupMember* members,
                                      std::int32_t count) noexcept;

// record+30h is declared `std::uint32_t field_30` by
// bsp/ship_ai_path_corridor.hpp, which had not read it. These two round-trip it
// as the float every writer and reader treats it as.
float ship_ai_formation_member_speed(const ShipAiUnitGroupMember& member) noexcept;
void ship_ai_formation_set_member_speed(ShipAiUnitGroupMember& member,
                                        float speed) noexcept;

// ---------------------------------------------------------------------------
// Sequence routines over an injected host
// ---------------------------------------------------------------------------

// 0070ED30, body 0070ED30-0070EF27, read complete. __thiscall(group)(int
// index), RET 4. It is called from exactly one site, 0070EF85 in 0070EF30, with
// the index of the record just filled in and before the count is bumped.
struct ShipAiFormationSlotHost {
    virtual ~ShipAiFormationSlotHost() = default;

    // 0070ED60, 0070ED90 and 0070EE55, 00414DB0 BSP_EntityPose_RefreshWorld
    // with ECX = the leader (twice, guarded by leader+10Ch and leader+0C8h) and
    // ECX = the member (guarded by member+0C8h). contract: the callee is named
    // in the ledger; this packet did not re-read its body.
    virtual void refresh_world_pose_00414db0(std::uint32_t entity) = 0;

    // 0070ED7C, 00B63D50 BSP_Matrix_BuildOrthogonalScaledAffineInverse with
    // ECX = leader+110h and EDX = leader+0CCh: the inverse of the leader's
    // world matrix, built once and latched by the byte at leader+10Ch.
    virtual void build_world_inverse_00b63d50(std::uint32_t leader) = 0;

    // 0070EDA5, the field read the first transform takes as its `this`: the
    // member's world position at member+0FCh.
    virtual void world_position_0fc(std::uint32_t entity, float out[3]) = 0;

    // 0070EDAB, 004142E0 BSP_Vector3f_TransformAffinePoint with ECX = the point
    // and leader+110h pushed: the leader's inverse world matrix.
    virtual void transform_by_leader_inverse_004142e0(std::uint32_t leader,
                                                      const float in[3],
                                                      float out[3]) = 0;

    // 0070EE6B, the same routine with leader+0CCh pushed: the leader's world
    // matrix, applied to the clamped relative position.
    virtual void transform_by_leader_world_004142e0(std::uint32_t leader,
                                                    const float in[3],
                                                    float out[3]) = 0;

    // 0070EDD3, 0070EEC1, 00424C40 BSP_GameSettings_GetSingleton, read for
    // settings+424h (FollowerMaxDist) and settings+42Ch (FormationShipDist).
    virtual float game_settings_follower_max_dist_00424c40() = 0;
    virtual float game_settings_formation_ship_dist_00424c40() = 0;

    // 0070EE20, 0042B260 BSP_Geometry_NormalizeVectorWithFloor with ECX = the
    // relative position, taken only when its squared length exceeds
    // FollowerMaxDist squared. contract: normalises in place; the caller then
    // scales by FollowerMaxDist (0070EE2B..0070EE3A).
    virtual void normalize_in_place_0042b260(float v[3]) = 0;

    // 0070EEA6, 00811180 with ECX = the leader: the wake decomposition that
    // produces column 0. Out-parameters in the image's order: param_3 is the
    // across-track component and param_4 the along-track distance.
    virtual void wake_decompose_00811180(std::uint32_t leader,
                                         const float point[3], float& across,
                                         float& along) = 0;
};

// The member record as this routine writes it. `entity` and the record's own
// address come from the caller.
struct ShipAiFormationMemberSlots {
    float relative[3]{0.0f, 0.0f, 0.0f};  // +04h..+0Ch
    ShipAiFormationColumns columns{};      // +10h..+2Ch
};

// 0070ED30 whole. Returns false when the record's entity is null (0070ED4A),
// in which case nothing is written.
bool ship_ai_formation_produce_member_slots(ShipAiFormationSlotHost& host,
                                            std::uint32_t leader,
                                            std::uint32_t member_entity,
                                            std::size_t index,
                                            ShipAiFormationMemberSlots& out);

// 0070EFD0, body 0070EFD0-0070F08C, read complete. __thiscall(group)(int
// shape). Rewrites **column 0 only** for every member: the leader's record
// gets (0, 0) and every other live member gets the next entry of the shape's
// table. Returns the number of records it wrote.
struct ShipAiFormationReshapeHost {
    virtual ~ShipAiFormationReshapeHost() = default;
    // 0070EFF9, 00424C40 BSP_GameSettings_GetSingleton, read for settings+42Ch.
    virtual float game_settings_formation_ship_dist_00424c40() = 0;
};

std::size_t ship_ai_formation_reshape(ShipAiFormationReshapeHost& host,
                                      ShipAiUnitGroupMember* members,
                                      std::int32_t count,
                                      std::uint32_t leader,
                                      ShipAiFormationShape shape);

// 0070D290, body 0070D290-0070D3FE, read complete. __thiscall(group)(entity
// unit, float out[7], float across_scale, float along_scale), RET 10h.
struct ShipAiFormationStationHost {
    virtual ~ShipAiFormationStationHost() = default;

    // 0070D2F4, 00811150 with ECX = [group+14h], which adds 0BD0h and tail
    // calls 00810630 (00811164, 0081116D). It walks `along` metres back along
    // the leader's wake and answers the point, the direction there and the
    // yaw rate recorded there; the yaw rate is left untouched when along <= 0.
    virtual ShipAiWakePoint wake_point_00811150(std::uint32_t leader,
                                                float along) = 0;

    // 0070D36D, 00414DB0 BSP_EntityPose_RefreshWorld with ECX = the unit,
    // guarded by unit+0C8h. Leader branch only.
    virtual void refresh_world_pose_00414db0(std::uint32_t entity) = 0;

    // 0070D372 and 0070D37A, the two field reads that follow it: the unit's
    // world x at unit+0FCh and world z at unit+104h. Leader branch only.
    virtual void world_position_xz_0fc(std::uint32_t entity, float& x,
                                       float& z) = 0;

    // 0070D397, unit->vtable[50h]() on the leader branch: the unit's world
    // heading in radians. contract: unread; named for the arithmetic at
    // 0070D399 (pi/2 - result, wrapped into [0, 2pi)).
    virtual float unit_heading_vtable_50(std::uint32_t entity) = 0;
};

// 0070D290 whole. `record` is the member record 0070D080 found, or nullptr.
ShipAiFormationStation ship_ai_formation_station_point(
    ShipAiFormationStationHost& host, std::uint32_t unit,
    std::uint32_t leader, const ShipAiUnitGroupMember* record,
    std::int32_t column, float across_scale, float along_scale);

// 0070EF30, body 0070EF30-0070EFC0, read complete. __thiscall(group)(entity),
// the join: it writes entity+284h before anything else, appends the record,
// fills all four columns through 0070ED30 with the **join index**, bumps the
// count and re-reduces the group speed ceiling.
struct ShipAiFormationJoinHost {
    virtual ~ShipAiFormationJoinHost() = default;

    // 0070EF52 and 0070EFA9, 006AEE30-family observer helpers named
    // BSP_Observer_IsPairRegistered / BSP_Observer_RegisterPair in the ledger.
    // contract: the membership notification; bodies not re-read here.
    virtual bool observer_pair_registered(std::uint32_t group,
                                          std::uint32_t entity) = 0;
    virtual void observer_register_pair(std::uint32_t group,
                                        std::uint32_t entity) = 0;

    // 0070EF85, 0070ED30 with the index of the new record.
    virtual void produce_member_slots_0070ed30(std::size_t index) = 0;

    // 0070EF9C, 0070DA00: re-reduce group+504h over the new membership.
    virtual void refresh_speed_ceiling_0070da00() = 0;

    // The single store at 0070EF30 itself: entity+284h = group.
    virtual void set_entity_group_284(std::uint32_t entity,
                                      std::uint32_t group) = 0;
};

// 0070EF30 whole. Returns the index the entity occupies, or -1 when it was
// already a member (in which case only the observer registration runs).
std::int32_t ship_ai_formation_add_member(ShipAiFormationJoinHost& host,
                                          ShipAiUnitGroupMember* members,
                                          std::int32_t& count,
                                          std::uint32_t group,
                                          std::uint32_t entity);

// ---------------------------------------------------------------------------
// 009DF2D0 whole
// ---------------------------------------------------------------------------
//
// __fastcall(state), RET 0, body 009DF2D0-009DF6B4, read complete. Its one
// caller is 009E1689 in 009E1610 BSP_ShipAi_FollowStateStep; it refills every
// field of ShipAiFollowState except the two latches, and its tail publishes
// this ship's speed into its own formation record. bsp/ship_ai_follow_land.hpp
// declares ShipAiFollowState and calls this routine
// `update_formation_point_009df2d0`; this file supplies its body.

struct ShipAiFollowFormationPointHost {
    virtual ~ShipAiFollowFormationPointHost() = default;

    // 009DF307, 0070D290 with ECX = [unit+284h] and (unit, &out, 1.0f, 1.0f).
    virtual ShipAiFormationStation station_point_0070d290(
        std::uint32_t unit, float across_scale, float along_scale) = 0;

    // 009DF359 and 009DF3DB and 009DF5EA, 0092D730
    // BSP_UnitController_GetBodyAxisSpeed with ECX = [leader+1018h]: the
    // **leader's** controller on all three sites.
    virtual float leader_body_axis_speed_0092d730() = 0;

    // 009DF32E, the follower's hull radius at unit+9C8h.
    virtual float unit_hull_radius_9c8() = 0;

    // 009DF41A, unit->vtable[218h](): the zone set 00417B10 is called on.
    // contract: unread; the return value is only ever used as 00417B10's this.
    virtual std::uint32_t zone_set_vtable_218(std::uint32_t unit) = 0;

    // 009DF432 and 009DF4C5, 00417B10 with ECX = that zone set and
    // (out, in, 20.0f, 1). It answers a point pushed out of the zone set by
    // the margin. contract: unread body; named from the two call sites and
    // from docs/SHIP_AI_FOLLOW_LAND.md, which records the same use.
    virtual ShipAiFollowLandXZ push_out_of_zones_00417b10(
        std::uint32_t zone_set, ShipAiFollowLandXZ point, float margin) = 0;

    // 009DF44A, 0082E850 BSP_ShipClass_GetTurnRadius with ECX = brain+0AACh
    // (the ship class, not the unit at +0AA8h).
    virtual float ship_class_turn_radius_0082e850() = 0;

    // 009DF607, 00811940 BSP_UnitInstance_GetCurrentCommandYawRate with
    // ECX = the leader and (leader speed, across) pushed. It is RET 0, so its
    // two arguments stay on the stack and become arguments two and three of
    // the 009DACD0 call at 009DF612.
    virtual float leader_command_yaw_rate_00811940(float leader_speed,
                                                   float across) = 0;

    // 009DF68A, 0080FC30 BSP_UnitInstance_GetReferenceSpeed with ECX = unit.
    virtual float unit_reference_speed_0080fc30() = 0;

    // 009DF6AA, 0070D100 with ECX = [unit+284h] and (unit, speed): store the
    // published speed into this unit's own member record at +30h.
    virtual void publish_member_speed_0070d100(std::uint32_t unit,
                                               float speed) = 0;
};

// 009DF2D0 whole. `group` is [unit+284h] as the routine reads it at 009DF2E0.
// Returns false when it is null (009DF2EC), in which case the state is
// untouched and nothing at all runs.
bool ship_ai_follow_update_formation_point(ShipAiFollowFormationPointHost& host,
                                           std::uint32_t unit,
                                           std::uint32_t group,
                                           ShipAiFollowState& state);

}  // namespace bsp
