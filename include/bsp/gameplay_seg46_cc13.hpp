#pragma once

// Segment 46 (007A49A0-007B2D80) reference models: the MParatrooper entity
// (class id 31h, 4BCh bytes, primary vtable 00D05060, tick sub-object +3E4h with
// vtable 00D04FD4) and the paratrooper branch of the level-bomb drop
// (006E3F90 and 006E541F-006E5605 in BSP_MBombPlatform_Drop). Packet cc13-gpr46;
// docs/CC13_GPR_46.md, docs/CC13_SOLDIER_TRAFFIC.md section 10.
//
// These are semantic interfaces read from the disk bytes (Capstone) and the
// Ghidra pseudocode, not native layouts or ABI-compatible replacements. Field
// names are hypotheses; the "_NNN" suffix is the native byte offset. Every
// engine service the image calls is a ParatrooperHost callback, so nothing here
// invents a global or stubs a call. Build-tested only; not wired, not game-run.
//
// ADOPTION NOTE (cc9 terms; the full table is in docs/CC13_GPR_46.md):
// - Replaces: nothing that exists. The host makes no paratroopers today:
//   src/game_hosts_units.cpp:22656 passes `has_paratroopers = false` (007B9500),
//   :23130 and :23231 leave the target point 006E3F90 / rack+4F8h unmodelled, and
//   src/game_hosts_ship_ai.cpp:572/12443 treat the building's paratrooper list
//   +7DCh as always empty. Binding means a new entity type in the units host plus
//   the rack branch in Rack::drop (lb_rack_drop_006e4d50) behind a switch.
// - Inputs: the class row (bulletclasses.lua "Paratrooper": CapturePower,
//   SlowFactorClosed/Opened, SoldierClass, SoldierAnim, CaptureDuration, Damage,
//   and the BombClass step scale +5Ch); SoldierGlobals (MaxParatrooperGradient,
//   ParachuteOpenAltitudeMin/Max); the round's pose and velocity; the
//   world's ground height/normal, avoid zones and command buildings.
// - RNG: three draws, all through ParatrooperHost::uniform in image order.
//   * 007AB94A, constructor: U(ParachuteOpenAltitudeMin, Max), STREAM 1
//     (ECX = EBX+1 with EBX = 0). Gameplay.
//   * 006E5479 then 006E549E, the drop: U(-50, 50) for x then z, STREAM 0.
//     Gameplay (the landing point).
//   * 004B102F inside 004B0F80 (soldier animation start phase) from 007AB590,
//     STREAM 0 in the image. Presentation; see LandSoldierPlan.
// - OFF-switch prediction: with the switch off nothing changes. With it on, a
//   mission whose level bombers carry Paratrooper rounds (JM05) gets rounds that
//   fall with drag, steer toward a jittered landing point from 2 s of flight
//   (full authority at 3 s), open the chute below a drawn height, land, find the
//   nearest enemy command building, damage it and add CapturePower to its capture
//   while CaptureDuration runs. Stream-0 consumers after the first drop shift (two
//   draws per round), and stream-1 consumers shift by one draw per round created.
//   Only rounds with +458h "flying" set steer, open or land; the rack drop sets
//   it (006E1FA3 in 006E1F00, reached from 006E529C), so the host sets
//   flying_458 = true when it drops the round.

#include "bsp/soldier_traffic_cc13.hpp"

#include <array>
#include <cstdint>
#include <functional>
#include <vector>

namespace bsp {
namespace gameplay_seg46_cc13 {

struct Vec3 {
    float x{0.0f};
    float y{0.0f};
    float z{0.0f};
};

// ---------------------------------------------------------------------------
// The paratrooper class row (descriptor vtable 00CFA604; reader 007AC780 after
// BSP_BombClass_ReadLuaFields). Offsets per CC13_SOLDIER_TRAFFIC.md 10.1.
// ---------------------------------------------------------------------------
struct ParatrooperClassFields {
    float step_scale_5c{1.0f};       // BombClass +5Ch; the step 007AB360 passes to +19Ch is dt * this
    float capture_power_d8{0.0f};    // CapturePower, read by 006F6760, not here
    float slow_factor_closed_dc{0.0f};
    float slow_factor_opened_e0{0.0f};
    float open_duration_e4{0.0f};    // read by 007AC780; no reader in this entity's code
    float close_duration_e8{0.0f};   // read by 007AC780; no reader in this entity's code
    std::uint32_t soldier_class_ec{0};  // SoldierClass handle (004B1400)
    std::uint32_t soldier_anim_f0{0};   // SoldierAnim handle (class+F0h is passed by address)
    float capture_duration_f8{0.0f};
    float damage_fc{0.0f};           // per second, before the building's +368h is subtracted
};

// ---------------------------------------------------------------------------
// The round's own state. Offsets from the constructor 007AB8C0 and the readers.
// ---------------------------------------------------------------------------
struct ParatrooperState {
    int side_54{0};                  // party; 007AB4E6, 007AC05E
    std::uint32_t class_id_c4{0};    // 31h after the constructor
    std::uint16_t net_id_174{0};     // seeds the avoid-zone radius (007ABC41)
    Vec3 landing_pose_a4{};          // (x, ground, z) at touchdown (007AC211-007AC22A)
    Vec3 velocity_318{};
    bool flying_458{false};          // set by the drop 006E1FA3; also saved/restored as "flying" (006E30D8)
    float open_altitude_468{0.0f};
    float chute_46c{0.0f};           // 0 closed, 1 open
    bool landed_470{false};
    bool bad_landing_471{false};
    bool in_water_472{false};
    float capture_timer_474{0.0f};
    std::uint32_t soldier_478{0};    // 0 = none
    Vec3 target_47c{};               // landing target; y = ground height there
    bool avoid_zone_done_488{false};
    std::uint32_t target_building_48c{0};  // 0 = none
    bool spawn_soldier_pending_490{false};
    float flight_time_494{0.0f};
    std::uint32_t anim_current_498{0};
    std::uint32_t anim_fall_49c{0};
    float fall_length_4a0{0.0f};
    std::uint32_t anim_open_4a4{0};
    float open_length_4a8{0.0f};
    std::uint32_t anim_close_4ac{0};
    float close_length_4b0{0.0f};
    float anim_time_4b4{0.0f};
    std::int32_t dropper_4b8{0};     // 8 from the constructor; [+3BCh]+180h in MP mode 1 (provisional)
};

// The engine services the entity calls. A host implements them over its own
// world; the model owns no engine state.
struct ParatrooperHost {
    virtual ~ParatrooperHost() = default;
    // 00BD2F10 BSP_Random_UniformFloatRange(ECX = stream, lo, hi).
    virtual float uniform(int stream, float lo, float hi) = 0;
    // Session mode [[00E188A8]+1FE4h]: 1 sends message 71h, 2 skips the kills.
    virtual int session_mode() const = 0;
    // [[00E188A8]+218Ch] byte, the gate on message 71h.
    virtual bool session_flag_218c() const = 0;
    // 00414DB0 BSP_EntityPose_RefreshWorld when +C8h is clear, then +FCh.
    virtual Vec3 world_position() = 0;
    // 00903860 BSP_World_GroundHeightAt([game+19CCh], point*, out*).
    virtual float ground_height_at(const Vec3& point) = 0;
    // 009038F0 BSP_World_GroundNormalAt; false when there is no ground there.
    virtual bool ground_normal_at(const Vec3& point, Vec3& normal) = 0;
    // 004218E0 -> 004120D0(0) -> 0041B840: the nearest avoid-zone boundary point
    // to (x, z) within `search` of radius `radius`; returns the new (x, z).
    virtual void avoid_zone_nearest_boundary(float xz[2], float search, float radius) = 0;
    // 006E1510 BSP_BombProjectile_AdvanceInAir(dt): the MBomb base step. It may
    // write state.velocity_318 (the model reads it back after the call).
    virtual void bomb_advance_in_air(ParatrooperState& s, float dt) = 0;
    // 006E2220 BSP_Projectile_ExtrapolatePosition(dt).
    virtual void extrapolate_position(ParatrooperState& s, float dt) = 0;
    // 0085DAD0 with row Y = 00F8758C: re-orthonormalise the pose upright.
    virtual void orthonormalize_upright() = 0;
    // [[+464h]+0Ch] vslot +24h (anim, time, 1). `checked` is the 007AB3A4/3C4 null test.
    virtual bool chute_anim_present() = 0;
    virtual void set_chute_anim(std::uint32_t anim, float time) = 0;
    // 00926D90 BSP_MissionEntity_Kill(cause) on the round itself or on `entity`.
    virtual void kill_self(int cause) = 0;
    virtual void kill_entity(std::uint32_t entity, int cause) = 0;
    // 0077D270 BSP_UnitInstance_OnKilledLua.
    virtual void on_killed_lua() = 0;
    // Building fields: +54h side, +368h damage threshold, vslot +1ACh(damage).
    virtual int building_side(std::uint32_t building) = 0;
    virtual float building_368(std::uint32_t building) = 0;
    virtual void building_damage(std::uint32_t building, float amount) = 0;
    virtual Vec3 building_position(std::uint32_t building) = 0;
    // 00803510 BSP_Party_RelativeTo(subject, reference): 0 same, 1 enemy, 2 neutral.
    virtual int party_relative(int subject, int reference) = 0;
    // 006F2C30(ECX = &pos, EDX = side, 0, &out): nearest enemy command building in its radius.
    virtual std::uint32_t find_enemy_building(const Vec3& pos, int side) = 0;
    // 00875A80 BSP_FixedStepCallback_Register(+3E4h, 0): run 007AB590 once.
    virtual void register_fixed_step_once() = 0;
    // 006F39B0 / 006F34B0: the building's paratrooper (capturer) list +7DCh/+7E0h.
    virtual void add_capturer(std::uint32_t building) = 0;
    virtual void remove_capturer(std::uint32_t building) = 0;
    // Soldier presentation (0048A010 clip test, soldier vslot +D8h, 0048A1E0 muzzle).
    virtual bool soldier_clip_visible(std::uint32_t soldier, const Vec3& pos) = 0;
    virtual void soldier_show(std::uint32_t soldier) = 0;
    virtual void soldier_muzzle_effect(std::uint32_t soldier) = 0;
    // MP mode 1 only: message 71h with the target, and [+3BCh]+180h when +3BCh is set.
    virtual void send_landing_message_71(const Vec3& target) = 0;
    virtual bool dropper_from_owner(std::int32_t& dropper) = 0;
};

// 007AB8C0 BSP_MParatrooper_Construct, after BSP_BombProjectile_Construct 006E2670.
// __thiscall(this, descriptor), returns this. Draws the chute-opening height on
// STREAM 1 (007AB94A); lo = SoldierGlobals+14h, hi = +18h ([esp], [esp+4]).
void construct_007ab8c0(ParatrooperState& s, const soldier_traffic_cc13::SoldierGlobals& g,
                        ParatrooperHost& host);

// 007ABA50 BSP_MParatrooper_IsKindOf(kind), __thiscall, RET 4: 31h, 2Ah, 2, 1, 0
// or the object's own +C4h.
bool is_kind_of_007aba50(const ParatrooperState& s, std::uint32_t kind) noexcept;

// 007AC530 BSP_MParatrooper_LoadAnimations (+A0h), the anim part after
// BSP_BombProjectile_Activate: the "fall"/"open"/"close" handles and lengths
// (00B78670 and 00B6DC90 are host work), current = fall, both clocks zero, and
// the fall anim started at time 0.
void load_animations_007ac530(ParatrooperState& s, std::uint32_t fall, float fall_length,
                              std::uint32_t open, float open_length, std::uint32_t close,
                              float close_length, ParatrooperHost& host);

// 007AB230 BSP_MParatrooper_SetLandingPos(point*), __thiscall, RET 4. Also reached
// through 007AB330 (message 71h, point at msg+20h) and 007AB8A0 (point at ECX+120h).
void set_landing_pos_007ab230(ParatrooperState& s, const Vec3& point, ParatrooperHost& host);
// Newly defined message slot; ECX=paratrooper, message stack pointer, RET4.
// Message 71h consumes its Vec3 at +20h; every other type tails to MBomb.
bool handle_message_007ab330(ParatrooperState&, const void* message, ParatrooperHost&,
                            bool (*bomb_base_006e09f0)(const void*, void*), void* context);
void path_entity_noop_007b0b90(std::uint32_t unused) noexcept;

// 007ABB30 BSP_MParatrooper_AdvanceInAir (+19Ch), __thiscall(dt), RET 4. Chute
// drag, the base step, then (flying only) the steer toward +47Ch.
void advance_in_air_007abb30(ParatrooperState& s, const ParatrooperClassFields& cls, float dt,
                             ParatrooperHost& host);

// 007AB360 BSP_MParatrooper_UpdateChuteAndTarget, sub-object slot +8h,
// __thiscall(ECX = this+3E4h, dt), RET 4. Ends with +19Ch(dt * class+5Ch).
void update_chute_and_target_007ab360(ParatrooperState& s, const ParatrooperClassFields& cls,
                                      float dt, ParatrooperHost& host);

// 007AB0B0 (+DCh; no Ghidra function), __thiscall(dt), RET 4: the animation clock
// and, once "close" has played out, the capture timer.
void anim_clock_007ab0b0(ParatrooperState& s, float dt, ParatrooperHost& host);

// 007AC000 BSP_MParatrooper_StepLanding (+194h), __thiscall(dt), RET 4.
void step_landing_007ac000(ParatrooperState& s, const ParatrooperClassFields& cls,
                           const soldier_traffic_cc13::SoldierGlobals& g, float dt,
                           ParatrooperHost& host);

// 007AB590 BSP_MParatrooper_LandAndSpawnSoldier, sub-object slot +10h (the fixed
// step callback), __fastcall(ECX = this+3E4h). The soldier entity and its pose are
// renderer/scene work, so the model returns the plan a host executes in order.
struct LandSoldierPlan {
    bool run{false};
    std::uint32_t building{0};        // 006F39B0 target (add_capturer already called)
    std::uint32_t soldier_class{0};   // [[class+ECh]] vslot +8h creates the soldier
    std::uint32_t soldier_anim{0};    // 004B0F80(&class+F0h, random_start = 1, 0)
    // 004B0F80 draws U(0, duration) on STREAM 0 in the image (004B102F). A host
    // must route it to a keyed presentation stream or keep the bind OFF (RNG rule).
    bool anim_draw_stream0{true};
    Vec3 facing{};                    // building - self, the row the soldier faces (007AB64C-007AB688)
};
LandSoldierPlan land_and_spawn_soldier_007ab590(ParatrooperState& s, const ParatrooperClassFields& cls,
                                                ParatrooperHost& host);

// 007AAE10 BSP_MParatrooper_OnKilled, __fastcall(this).
void on_killed_007aae10(ParatrooperState& s, ParatrooperHost& host);

// ---------------------------------------------------------------------------
// The level-bomb rack's paratrooper branch.
// ---------------------------------------------------------------------------
struct RackParatrooperTarget {
    bool set_4f8{false};
    Vec3 point_4fc{};
    Vec3 velocity_508{};
};

// 006E3F90 BSP_MBombPlatform_SetTargetPoint(point*, velocity*), __thiscall(rack),
// RET 8. Its one caller 007C0EC9 (007C0D90, a level bomber IsKindOf 10h whose rack
// descriptor answers 31h) passes 007BCCF0(unit) and the unit's vslot +34h.
void set_target_point_006e3f90(RackParatrooperTarget& rack, const Vec3& point,
                               const Vec3& velocity) noexcept;

// 006E541F-006E5515 in BSP_MBombPlatform_Drop: with rack+4F8h set and the round
// IsKindOf(31h), the landing point is the rack point plus U(-50, 50) on x then z
// (STREAM 0, 006E5479 / 006E549E; y is replaced by the ground in 007AB230), and
// the rack point then moves by velocity * toRepeatTime (rack+494h). Returns false
// (no draws) when the branch is not taken. 006E55E7-006E55FE: the drop clears
// rack+4F8h when rack+484h <= 0 (signed); `rounds_left_484` reports that.
bool paratrooper_drop_target_006e541f(RackParatrooperTarget& rack, bool round_is_31h,
                                      float repeat_time_494, ParatrooperHost& host, Vec3& point);
void paratrooper_drop_clear_006e55e7(RackParatrooperTarget& rack, std::int32_t rounds_left_484) noexcept;

// ---------------------------------------------------------------------------
// Path join and path geometry (the authored-path source 00D0534C and the
// Point3PtrArray 00D053A4). These read what include/bsp/ship_ai_path_cursor.hpp
// labels a hypothesis: its ship_ai_path_nearest_index_007b1100 is a planar
// search, and its 007B11F0 is head-only.
//
// ADOPTION NOTE: would replace the planar search in
// src/game_hosts_commands.cpp:3142 (GameCommandsHost's path join for
// 007B1C50) and supply the unread 007B11F0 tail. Inputs: the path's points in
// the path entity's local frame, the query position transformed by the path
// entity's inverse world matrix (007B0900), and 00419260. No RNG. OFF-switch
// prediction: a ship joining a path whose nearest point is not the 2-D nearest
// (points or ship off y = 0) can pick another index; and with the tail, a join
// in the travel direction starts one point later whenever the ship is already
// past the nearest point along the leg, so ships stop doubling back to it.
// ---------------------------------------------------------------------------

// 007AED10 BSP_Point3PtrArray_NearestPointIndex, __thiscall(array)(pos*, float* out),
// RET 8. 3-D distances; a squared sum stored to float and <= 1e-10 counts as 0,
// else f(sqrt). The first point sums x, y, z; the loop sums y, x, z. Strictly
// smaller wins (the first of equals). `distance` (may be null) gets the best.
// Precondition, as in the image: at least one point (an empty array calls the
// CRT invalid-parameter handler 00BF6713).
int nearest_point_index_007aed10(const Vec3* points, int count, const Vec3& pos, float* distance);

// 007B0900 BSP_ScenePath_NearestPointIndexWorld, the homogeneous divide applied
// to `h` = 00B62D10(path entity's inverse world matrix, (pos, 1)):
// inv = f(1 / w), then f(x * inv), f(y * inv), f(inv * z).
Vec3 path_local_from_homogeneous_007b0900(const float h[4]) noexcept;

// The path source's slots that 007B11F0 calls through its own vtable.
struct PathJoinSource {
    virtual ~PathJoinSource() = default;
    virtual int point_count() = 0;                  // vslot +0Ch
    virtual int nearest_index(const Vec3& pos) = 0;  // vslot +14h (007B1100 -> 007B0900 -> 007AED10)
    virtual Vec3 point(int index) = 0;              // vslot +08h
    virtual float reciprocal_length_00419260(float x, float z) = 0;
};

// 007ADA60 BSP_PathSource_NearestIndex, __thiscall(source)(pos*), RET 4: the
// nearest-index slot +14h of the base (00D052DC), SIMPLE (00D05318, built by
// 007B2250 on the kind-0 moveonpath arm) and owned-points (00D05440) sources.
// IMAGE QUIRK, read from the bytes: the loop's point fetch passes the constant
// 1 (007ADAD5 PUSH 1), not the loop index, so only points 0 and 1 are ever
// compared. The answer is -1 with no points (007ADB86), 0 when point 1 is not
// strictly nearer than point 0 (or there is one point), else 1. Point 0's
// distance is 0042B2F0(f(pos - p0)) stored to float (`length`); point 1's is
// the inline 1e-10 / sqrt form shared with 007AED10.
int simple_source_nearest_index_007ada60(PathJoinSource& source, const Vec3& pos,
                                         const std::function<float(const std::array<float, 3>&)>& length);

// 007B11F0 BSP_EntityPathSource_StartIndexForPosition, __thiscall(source)(pos*,
// float unused, char forward), RET 0Ch, read whole. With fewer than two points,
// or the nearest already at the end in the travel direction, the nearest. Else
// the neighbour in the travel direction when the position lies strictly ahead of
// the nearest point along the planar (x, z) leg toward that neighbour:
// d = f(p - a), u = f(f(b - a) * rcp), s = f(u.z * d.z + u.x * d.x) > 0.
// The float argument at [esp+3Ch] is never read.
int start_index_for_position_007b11f0(PathJoinSource& source, const Vec3& pos, bool forward);

// 007B15F0 BSP_ScenePath_DropPointsOntoLandscape, __fastcall(path component).
// Only when the path entity's parent (+3Ch) answers IsKindOf(44h) (Landscape).
// Per point: world = divide(00B62D10(entity +CCh world matrix, (p, 1))) on x and
// z only; y = f(0087FA50(landscape, x, z)) (the forward height); local =
// divide(00B62D10(entity +110h inverse matrix, (x, y, z, 1))); then vslot +4h
// (index, &local) stores it. Afterwards, with the byte +18h clear, 007AF150
// derives the knots (`derive_knots` returns that decision).
struct LandscapeDropHost {
    virtual ~LandscapeDropHost() = default;
    virtual bool parent_is_landscape() = 0;
    virtual void to_world_h(const Vec3& local, float out[4]) = 0;     // 00B62D10, +CCh
    virtual float forward_height_at(float x, float z) = 0;            // 0087FA50
    virtual void to_local_h(const Vec3& world, float out[4]) = 0;     // 00B62D10, +110h
    virtual void set_point(int index, const Vec3& local) = 0;         // vslot +4h
};
bool drop_points_onto_landscape_007b15f0(const std::vector<Vec3>& points, bool knots_fixed_18,
                                         LandscapeDropHost& host);

// 007AE6D0 BSP_Point3PtrArray_Centroid, __thiscall(array)(out*), RET 4. Each
// running sum is stored to float; inv = f(1 / (double)count); out = f(inv * sum).
// An empty array divides by zero as the image does (inf * 0 = NaN).
Vec3 centroid_007ae6d0(const Vec3* points, int count) noexcept;

// 007AE7B0 BSP_Point3PtrArray_DistanceToPolyline, __thiscall(array)(pos*), RET 4,
// ST0. One point: 0042B2F0(f(pos - p0)). Otherwise the minimum over the legs of
// 0046FF70(ECX = pos, EDX = p[i], p[i+1]), each stored to float, from FLT_MAX
// (00D7A248); no legs answers FLT_MAX. `length` is 0042B2F0.
// 007B1EC0 BSP_Point3Vector_ClipToWorldBounds (the ledger's "clip" is an erase),
// __fastcall(ECX = vector<float[3]>), read whole. With a game object, every point
// with x < [game+711Ch], x > [game+7128h], z > [game+7124h] or z < [game+7130h]
// is erased (0071F3E0) and the walk restarts at index 0; an unordered coordinate
// keeps its point. Caller 0071FDE0 BSP_DirectorPathObject_AddPoint.
struct WorldBounds711c {
    float x_min_711c{0.0f};
    float z_max_7124{0.0f};
    float x_max_7128{0.0f};
    float z_min_7130{0.0f};
};
void erase_points_outside_world_007b1ec0(std::vector<Vec3>& points, const WorldBounds711c* bounds);

float distance_to_polyline_007ae7b0(const Vec3* points, int count, const Vec3& pos,
                                    const std::function<float(const std::array<float, 3>&)>& length);

}  // namespace gameplay_seg46_cc13
}  // namespace bsp
