#pragma once

// Segment 53 (00851E10-008740E0) reference models: the MSubmarine depth helpers
// (unit vtable 00D0BF34, class id 08h), the MTorpedo projectile's create,
// activate, water entry and network create message (entity vtable 00D0C3E8,
// class id 2Bh, 51Ch bytes; shot interface +310h with vtable 00D0C378).
// Packet cc13-gpr53; docs/CC13_GPR_53.md.
//
// These are semantic interfaces read from the disk bytes (Capstone) and the
// Ghidra pseudocode, not native layouts or ABI-compatible replacements. Field
// names are hypotheses; the "_NNN" suffix is the native byte offset. Every
// engine service the image calls is a host callback, so nothing here invents a
// global or stubs a call. Gameplay x87 runs at 24-bit precision
// (docs/CC13_FPU_PRECISION.md): each intermediate below is a float, and the
// statement order follows the listing. Build-tested only; not wired, not
// game-run.
//
// ADOPTION NOTE (cc9 terms; the full table is docs/CC13_GPR_53.md section 2):
// - 00852410 submarine_predict_depth_after: replaces the labelled
//   substitution "0072C970's swim depth (00852410) is not modelled" at
//   src/game_hosts_gunnery.cpp:730 and :11781. Inputs: the target hull's world
//   y, its four depth bands, clearance +123Ch, depthLevel, sullyesztoEro and
//   needAir (all in the host's submarine row, submarine_model.hpp), the body's
//   linear velocity y (00C31F40), the class dive rates (+824h/+830h/+834h) and
//   008522C0. No RNG. OFF prediction: unchanged. ON: a torpedo a submarine
//   fires (0072C970 path, Function 7) runs at the predicted depth instead of
//   the -2.5 default (00D0C314); JM06 submarine torpedo runs change depth, and
//   with it the 00855F00 mode-2 threshold (y below 2 * swimdepth).
// - 008522C0 submarine_catapult_needs_surface: replaces the `catapult = false`
//   substitutions at src/game_hosts_script_orders.cpp:3500 and
//   src/game_hosts_units.cpp:9529. Inputs: unit+1279h and the hull's child
//   list (IsKindOf(28h), child+490h). No RNG. ON changes only seaplane-carrying
//   submarines (none in JM06).
// - 00852740 / 008527A0 / 00852820: depth predicates. 00852820 is already
//   inlined by the gunnery host (game_hosts_gunnery.cpp:752); the other two
//   have no host consumer. No RNG.
// - 00856050 / 00856420 / 00857780 / 008568E0 / 00855CC0 / 00857160: the
//   torpedo round. The gunnery host keeps its own torpedo row; binding would
//   replace its create/launch defaults and the water-entry break-up test
//   (game_hosts_gunnery.cpp:9399, which applies only the MaxWaterHitVel
//   limit). RNG: ONE draw, 00856132 in the constructor, U(0, 1) on STREAM 1
//   (ECX = 1 from 008560C8), negated into the off-scan countdown +518h.
//   Gameplay (it phases the first homing scan); not stream 0. OFF prediction:
//   unchanged. ON: stream-1 consumers shift by one draw per torpedo created;
//   torpedoes dropped from above MaxFall (vertical speed below
//   -sqrt(2 g MaxFall)) break up on water entry, which JM06 air-dropped
//   torpedoes may hit when released high.

#include "bsp/effects_cc13.hpp"

#include <array>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <string>
#include <vector>

namespace bsp {
namespace gameplay_seg53_cc13 {

struct Vec3 {
    float x{0.0f};
    float y{0.0f};
    float z{0.0f};
};

// ===========================================================================
// MSubmarine depth helpers (00852200-00853010)
// ===========================================================================

// The unit fields these helpers read. `world_y_100` is unit+100h AFTER the
// pose refresh 00414DB0 that every helper runs when unit+C8h is clear.
struct SubmarineDepthFields {
    float world_y_100{0.0f};
    float bands_1200[4]{};          // depth bands 0..3 (surface, periscope, ...)
    float clearance_123c{0.0f};     // seabed floor published by 00855420
    std::int32_t depth_level_1268{0};
    float sink_1270{0.0f};          // "sullyesztoEro"
    bool need_air_1281{false};      // "needAir"
};

// Class dive rates (submarine class row, 00854230).
struct SubmarineDiveRates {
    float accel_824{0.0f};          // UpDownAccel
    float up_speed_830{0.0f};       // UpSpeed
    float down_speed_834{0.0f};     // DownSpeed
};

// 008522C0, __thiscall(unit), RET -> AL. One entry per child of unit+48h in
// list order (next at child+44h).
struct SubmarineChild {
    bool is_catapult{false};          // child vtable[5Ch](28h)
    std::int32_t launch_state_490{-1};
};
bool submarine_catapult_needs_surface_008522c0(
    bool catapult_surface_enable_1279, const std::vector<SubmarineChild>& children) noexcept;

// 00852410, __thiscall(unit, float dt), RET 4 -> ST0. The world y the hull will
// have after `dt` seconds of the dive law's velocity ramp toward the band the
// unit is commanded to, clamped at the band edge +/- 2.0, at the seabed
// clearance, and (unless the clearance wins) at -2.5 from above.
// `body_velocity_y` is 0092D6A0(body = unit+1018h)'s y: the body's linear
// velocity (00C31F40). `catapult_needs_surface` is 008522C0, only consulted
// when needAir is clear (00852499).
float submarine_predict_depth_after_00852410(const SubmarineDepthFields& unit,
                                             const SubmarineDiveRates& rates,
                                             float body_velocity_y,
                                             bool catapult_needs_surface,
                                             float dt) noexcept;

// 00852740, __thiscall(unit, desc*), RET 4 -> AL/EAX. False for a null
// argument or one whose +8h is not 0Eh (the torpedo-boat kind); otherwise
// world y < float(band1 + 2.0).
bool submarine_is_below_band1_for_kind0e_00852740(const SubmarineDepthFields& unit,
                                                  bool has_argument,
                                                  std::int32_t argument_kind_8) noexcept;

// 008527A0, __thiscall(unit), RET -> AL: world y > float(band0 - 3.0).
bool submarine_is_above_deck_gun_depth_008527a0(const SubmarineDepthFields& unit) noexcept;

// 00852820, __thiscall(unit), RET -> EAX: world y > (band1 + band0) / 3.0.
bool submarine_is_above_depth_charge_depth_00852820(const SubmarineDepthFields& unit) noexcept;

// 00852FF0, __thiscall(unit), RET -> EAX: the class's navigation group for the
// current depthLevel, 0082ADA0(class = unit+538h)(depthLevel), unclamped.
template <class NavGroupAtDepth>
auto submarine_navigation_group_at_depth_00852ff0(const SubmarineDepthFields& unit,
                                                  NavGroupAtDepth&& class_group_at_depth) {
    return class_group_at_depth(unit.depth_level_1268);
}

// 00853E10, submarine vtable 00D0BF34 slot F0h (00D0C024), __thiscall(unit),
// RET 0. Not a serializer: after the ship's 0081F980 it LOADS state.
// - creation kind 3 (+C0h, saved game): the "_submarine" table (009238A0):
//   airSupply.{air +127Ch, needAir +1281h, unlimitedAir +1280h}, depthLevel
//   +1268h, periscopeState +122Ch, periscopeY +1230h, periscopeOut +1234h,
//   sullyesztoEro +1270h.
// - creation kind 1 (scene spawn): property "AutoAttackTarget" (integer >= 0)
//   -> IssueCommand(attackmove 00E08F78, target 00465080(00521E30(id), 0.0), 1).
// - off a network client, with DummyObjectID +6B8h >= 0 only (the host and
//   0095CDBE leave -1 on ordinary units): r = U(0, 1) on STREAM 1 (008540EA,
//   ECX = 1); heading' = 00438AA0(vtable[50h](), 0.5 > r ? -0.3 : +0.3);
//   dir = 007BA2E0(heading') * 250; IssueCommand(moveto 00E08F68, point
//   (x + dir.x, band1 +1204h, z + dir.z), 1).
// - finally unit+1279h = some child answers IsKindOf(28h) (the flag 008522C0
//   gates on).
struct SubmarineActivateInputs {
    std::int32_t creation_kind{0};
    std::int32_t auto_attack_target{-1};   // kind 1 property, -1 when absent
    bool network_client{false};
    std::int32_t dummy_object_id_6b8{-1};
    float heading{0.0f};                   // vtable[50h]()
    Vec3 world_position{};                 // +FCh..+104h
    float band1_1204{0.0f};
    bool has_catapult_child{false};
};
struct SubmarineActivateHost {
    std::function<void()> read_saved_submarine_table;            // kind 3
    std::function<void(std::int32_t target_id)> issue_attackmove; // kind 1
    std::function<float(int stream, float lo, float hi)> uniform; // 00BD2F10
    std::function<float(float, float)> add_wrapped_angle;         // 00438AA0
    std::function<Vec3(float heading)> heading_to_direction_xz;   // 007BA2E0
    std::function<void(const Vec3& point)> issue_moveto;
};
// Returns unit+1279h.
bool submarine_activate_00853e10(const SubmarineActivateInputs& in, const SubmarineActivateHost& host);

// 00852B50 / 00852B70 (no Ghidra function; INT3-padded leaves after
// 00852970): the one-step depth commands. Shallower: depthLevel > 0 ->
// 008528B0(depthLevel - 1). Deeper: depthLevel < 3 -> 008528B0(depthLevel + 1).
// Returns the level passed to 008528B0, or -1 when no call is made.
std::int32_t submarine_depth_step_shallower_00852b50(std::int32_t depth_level_1268) noexcept;
std::int32_t submarine_depth_step_deeper_00852b70(std::int32_t depth_level_1268) noexcept;

// 00852970, __thiscall(debris, float dt), RET 4: the periscope debris piece
// ("PeriszkopSullyesztoDolog:", built by 008533E0). Presentation only; no RNG.
// State 0 captures the node's world matrix and the owner's velocity
// (vtable[34h]); state 1 flies it: position += v dt, the basis turns about
// the angular velocity by |w| dt when that exceeds 0.001, the node gets the
// matrix, then vy -= 2.5 dt and v, w decay by 0.45 and 0.51 per second.
struct PeriscopeDebris {
    std::int32_t state_10{0};
    Vec3 velocity_14{};
    Vec3 angular_velocity_20{};
    Vec3 position_5c{};   // row 3 of the matrix at +2Ch
};
struct PeriscopeDebrisHost {
    std::function<void()> capture_node_matrix;     // 00B6DB70 when node+5Ch bit 1 clear, 004134F0
    std::function<Vec3()> owner_velocity;          // [owner] vtable[34h]
    std::function<float(const Vec3&)> length;      // 0042B2F0
    std::function<void(const Vec3& axis, float angle)> rotate_basis;  // 0042B260 then 0085E880
    std::function<void()> apply_matrix_to_node;    // node vtable[34h](+2Ch)
};
void periscope_debris_step_00852970(PeriscopeDebris& d, float dt, const PeriscopeDebrisHost& host);

// ===========================================================================
// MTorpedo (00855A90-00857C10)
// ===========================================================================

// The torpedo class row fields (008566B0 after the bomb reader 006E1BE0).
struct TorpedoClassFields {
    float muzzle_speed_50{0.0f};        // "V0" (bullet base row), read at 00856538
    float fly_time_54{0.0f};            // "FlyTime" (bullet base row), read at 00855AA0
    float max_water_hit_vel_dc{0.0f};   // "MaxWaterHitVel"
    float max_fall_e0{0.0f};            // "MaxFall"
    float water_travel_speed_e4{0.0f};  // "WaterTravelSpeed"
    float heading_turn_e8{0.0f};        // "HeadingTurn"
    bool has_water_splash_efx_d8{false};  // "WaterSplashEfx" handle != 0
    bool has_expl_water_efx_24{false};    // "ExplWaterEfx" (bullet base row) handle != 0
    float homing_horz_turn_speed_f0{0.0f};  // "HomingHorzTurnSpeed", default 0
    float homing_vert_turn_speed_f4{0.0f};  // "HomingVertTurnSpeed", default 0
};

// 00855A90, __fastcall(class), RET -> EAX. Class vtable finalise hook.
struct TorpedoClassDerived {
    float cached_range_60{0.0f};        // WaterTravelSpeed * fly time * 0.6
    float terminal_fall_speed_ec{0.0f}; // sqrt(2 * MaxFall * 9.81)
    bool ok{false};                     // base 006E9890 && terminal > 0
};
TorpedoClassDerived torpedo_class_derive_00855a90(const TorpedoClassFields& cls,
                                                  bool base_finalise_ok) noexcept;

// The per-round torpedo state ("_torpedo" table, 00855D30 / 00857780).
struct TorpedoState {
    float heading_46c{10000.0f};        // "heading"; 10000 (00D0C310) = unset
    float accel_470{0.0f};              // "accel"
    float drag_front_474{0.0f};         // "dragfront"
    float drag_side_478{0.0f};          // "dragside"
    float swim_depth_47c{-2.5f};        // "swimdepth", 00D0C314
    float depth_smooth_rate_480{0.0f};  // "depthsmoothrate"
    float wave_rate_484{0.0f};          // "waverate"
    float field_488{0.0f};
    Vec3 start_position_48c{1.0f, 1.0f, 1.0f};
    float run_time_498{0.0f};           // += dt by the wake update 008571F0
    bool launched_in_water_49c{false};
    std::uint32_t propeller_node_4a0{0};
    float off_scan_interval_514{1.0f};
    float off_scan_countdown_518{0.0f};
    bool flying_458{false};
    bool flag_468{false};
    std::uint32_t target_entity_4f8{0};  // the observer link +4E4h's target (+14h)
    Vec3 bubble_point_33c{};            // the model's "bubor" point (wake emitter)
    std::int32_t class_id_c4{0x2B};
    float shot_weapon_scale_3e0{1.0f};  // shot+D0h (006E2550)
};

// 00856050, __thiscall(this, int), RET 4: the field defaults after the bomb
// constructor 006E2670, and the one random draw. `uniform_stream1(lo, hi)` is
// 00BD2F10 with ECX = 1 (stream 1); it is called exactly once, with (0, 1).
TorpedoState torpedo_construct_00856050(
    const std::function<float(int stream, float lo, float hi)>& uniform) ;

// 00856420's optional launch parameters (its seventh argument).
struct TorpedoLaunchParams {
    float heading_4{0.0f};
    float swim_depth_8{-2.5f};
    std::uint32_t target_entity_20{0};
};

// 00856420, __thiscall(class, a1..a7), RET 1Ch. `direction` is a unit vector
// (argument 4), `position` the spawn point (argument 3).
struct TorpedoCreateResult {
    Vec3 velocity_318{};       // direction * V0 (class+50h)
    Vec3 forward_94{};         // matrix row +20h of +74h, before 0085DC80 re-orthonormalises
    Vec3 position_a4{};        // also shot+60h and shot+6Ch
    float heading_46c{10000.0f};
    float swim_depth_47c{-2.5f};
    std::uint32_t target_entity_4f8{0};
};
struct TorpedoCreateHost {
    // vtable[98h](0, world+19CCh, identity) right after the constructor.
    std::function<void()> attach_identity_pose;
    // 0085DC80 on the +74h matrix after the forward row is written.
    std::function<void()> orthonormalise_pose;
    // 0042ED50 for every child of +48h (next +44h).
    std::function<void()> invalidate_child_poses;
};
TorpedoCreateResult torpedo_create_00856420(const TorpedoClassFields& cls,
                                            const TorpedoState& constructed,
                                            const Vec3& position,
                                            const Vec3& direction,
                                            const TorpedoLaunchParams* params,
                                            const TorpedoCreateHost& host);

// 00857780, __thiscall(this), RET 0. The activation after creation or load.
struct TorpedoActivateInputs {
    Vec3 world_position{};          // +FCh..+104h after 00414DB0
    bool shot_in_water_354{false};  // shot+44h
    float water_travel_speed_e4{0.0f};
    // [+C0h] creation context: 0 none, 2 network message, 3 saved game.
    std::int32_t creation_kind{0};
    float message_swim_depth_124{0.0f};  // kind 2: the message's +124h
    bool has_bubble_point{false};        // class+A8h model "bubor" group with >= 1 point
    Vec3 bubble_point{};
};
struct TorpedoActivateHost {
    std::function<void()> base_activate;            // 006E2E10
    std::function<void(float)> set_weapon_scale;    // shot vtable[20h] (006E2550)
    // kind 3: read the "_torpedo" table of the saved entity data (009238A0)
    // into heading, swimdepth, accel, dragfront, dragside, swimdepth (again),
    // depthsmoothrate, waverate, in that order (00BD6830 per field).
    std::function<void(TorpedoState&)> read_saved_torpedo_table;
};
void torpedo_activate_00857780(const TorpedoActivateInputs& in, TorpedoState& state,
                               const TorpedoActivateHost& host);

// 008568E0, __thiscall(shot, char spawn_effect), RET 4: the water entry.
// shot is the projectile sub-object at entity+310h. Every call the image
// makes goes through a host hook at the image's point: 006E6450 (enter
// water) before the splash on the surviving arm, 00926D90(1) on the entity
// (shot-310h) or 006E6450(arg) on a network client after the break-up
// effect. The heading shot+15Ch is written in place.
struct TorpedoWaterEntryInputs {
    bool in_water_44{false};
    Vec3 velocity_318{};          // shot+8h
    bool spawn_effect_arg{false};
    bool network_client{false};   // [00E188A8]+1FE4h == 2, read at 00856B33
};
struct TorpedoWaterEntryHost {
    std::function<void(float)> set_weapon_scale;       // shot vtable[20h](1.0)
    std::function<float(const Vec3&)> length;          // 0042B2F0
    std::function<void(bool)> enter_water;             // 006E6450(char), __thiscall(shot), RET 4
    std::function<void(bool)> kill_entity;             // 00926D90(char) on shot-310h; passed 1
    // Splash for a surviving round (class+D8h): water crossing 0078D1B0
    // between the pose and shot+60h, then a point effect, effect+9 = 1.
    std::function<void()> spawn_splash_effect;
    // Break-up effect (class+24h): the pose moves to the water crossing,
    // +C8h = 1, +10Ch = 0, then a point effect at the pose, effect+9 = 1.
    std::function<void()> spawn_breakup_effect_and_snap_pose;
    // 00414DB0 if the pose is stale, then 00521370 on the world matrix's
    // forward row (+CCh+20h): the yaw.
    std::function<float()> current_yaw;
};
void torpedo_enter_water_008568e0(const TorpedoClassFields& cls,
                                  float terminal_fall_speed_ec,
                                  const TorpedoWaterEntryInputs& in,
                                  float& heading_46c,  // shot+15Ch
                                  const TorpedoWaterEntryHost& host);

// 00855CC0 / 00857160: MT_TORPEDO_CREATE (the bomb create message, type B8h,
// vtable 00D037BC) and its apply. The apply copies heading and the target
// link; swimdepth +124h is copied only by 00857780 (creation kind 2).
struct TorpedoCreateMessage {
    float heading_120{0.0f};
    float swim_depth_124{0.0f};
    std::uint16_t target_net_id_128{0};  // target +174h, 0 when no target
};
TorpedoCreateMessage torpedo_create_message_00855cc0(const TorpedoState& state,
                                                     bool has_target,
                                                     std::uint16_t target_net_id_174) noexcept;
void torpedo_create_message_apply_00857160(
    const TorpedoCreateMessage& msg, TorpedoState& state,
    const std::function<std::uint32_t(std::uint16_t)>& entity_by_net_id);

// 008571F0, entity vtable 00D0C3E8 slot DCh (data ref 00D0C4C4), __thiscall
// (this, float dt), RET 4: the wake ("bubor") emitter step. Presentation: no
// RNG, and nothing it writes is read by the round's motion. +498h += dt
// always. Then, only while the shot's water mode (vtable[2Ch], 00855F00) is
// non-zero and the emitter +334h exists: the emitter is placed at the bubble
// point in world space, its +54h takes the speed (vtable[38h]), its +68h..+70h
// the forward row and +2Ch = 0. For a class with HomingVertTurnSpeed (+F4h) > 0
// only: below 2 * swimdepth every type-4 child is retired (00866E60 /
// 008679D0 loop); otherwise, with no type-4 child, 00867B10 then 00866B70.
// It is not homing guidance: HomingVertTurnSpeed only selects this branch.
enum class TorpedoWakeAction {
    None,              // air, or no emitter
    Placed,            // emitter placed; no homing branch
    RetireType4,       // homing class, y < 2 * swimdepth
    StopEmitter,       // homing class, shallower, no type-4 child: 00867B10, 00866B70
    KeepType4,         // homing class, shallower, a type-4 child exists
};
struct TorpedoWakeInputs {
    float dt{0.0f};
    float run_time_498{0.0f};
    bool water_mode_nonzero{false};
    bool has_emitter_334{false};
    float homing_vert_turn_speed_f4{0.0f};
    float world_y_100{0.0f};
    float swim_depth_47c{-2.5f};
    bool has_type4_child{false};
};
struct TorpedoWakeHost {
    std::function<void()> place_emitter;      // 004142E0(+33Ch, pose) then 004842C0
    std::function<float()> speed;             // vtable[38h]
    std::function<void(float)> set_emitter_rate;     // emitter+54h
    std::function<void()> set_emitter_direction;     // emitter+68h..+70h = pose +ECh..+F4h, +2Ch = 0
};
struct TorpedoWakeResult {
    float run_time_498{0.0f};
    TorpedoWakeAction action{TorpedoWakeAction::None};
};
TorpedoWakeResult torpedo_wake_step_008571f0(const TorpedoWakeInputs& in, const TorpedoWakeHost& host);

// 00856BB0 homing block (00856BB0..00856F7F), the torpedo steering's first half.
// Only "Torpedo Fido Homing Airplane" (bulletclasses.lua RealisticTable[4]
// and its arcade twin in this installation) has HomingHorzTurnSpeed and
// HomingVertTurnSpeed (0.5 each); JM06 is a submarine mission, so this is the
// rule that steers aircraft-dropped Fidos at the player's boat.
//
// Countdown (+518h, interval +514h): dt < countdown (FCOMI, JB) -> countdown -=
// dt, no scan; else countdown = countdown + (interval - dt) and a scan runs.
// Scan (only with +3BCh set): the link +4FCh/+510h is cleared, then the list
// [008053C0([+3BCh]+54h)]+DE8h is walked (node +8h -> record, record +4h ->
// entity, next +4h); only entities answering IsKindOf(8) (submarines) count,
// and the nearest by squared distance (004193E0, strict >) wins.
// Turn (with a target): own pitch/yaw from the forward row (00521370), target
// pitch/yaw from the normalised direction; +46Ch = target yaw. With
// HomingVertTurnSpeed > 0, a submarine target and 00852820(target) false
// (submerged): rotate about the right row (+74h) by clamp(wrap(own_pitch -
// target_pitch), +/- HomingVertTurnSpeed * dt), then about the up row (+84h)
// by clamp(-wrap(own_yaw - target_yaw), +/- HomingHorzTurnSpeed * dt), and
// RETURN: the depth and heading controllers (00856F83..0085714E) are skipped.
// Otherwise rotate about the right row by own_pitch and fall through.
//
// The second turn reads the YAW slots: 0085E880 returns with RET 8, so after
// the first rotation ESP is back at E-30h, and 00856EE8 / 00856EF3 load E-20h
// (target yaw) and E-1Ch (own yaw), not the pitch slots E-28h / E-24h. The
// host model src/bomb_torpedo_tick.cpp torpedo_steer_00856bb0 reads the yaw
// pair for the second turn behind kTorpedoHomingYawPairBound (commit
// 853396a79, switched ON in 6e7850ad1); with the switch OFF it keeps the old
// pitch-error reading. See docs/CC13_GPR_53.md section 1.6.
struct TorpedoHomingCandidate {
    bool is_submarine{false};   // entity vtable[5Ch](8)
    Vec3 position{};
};
// Index of the candidate the scan keeps, or -1.
int torpedo_homing_pick_target_00856c50(const Vec3& own_position,
                                        const std::vector<TorpedoHomingCandidate>& list) noexcept;
struct TorpedoHomingInputs {
    float dt{0.0f};
    float scan_interval_514{1.0f};
    float scan_countdown_518{0.0f};
    float homing_horz_turn_speed_f0{0.0f};
    float homing_vert_turn_speed_f4{0.0f};
    bool has_target{false};               // +510h after any scan
    bool target_is_submarine{false};      // target vtable[5Ch](8)
    bool target_above_depth_charge_depth{false};  // 00852820(target)
    float own_pitch{0.0f};
    float own_yaw{0.0f};
    float target_pitch{0.0f};
    float target_yaw{0.0f};
};
struct TorpedoHomingResult {
    bool active{false};             // HomingHorzTurnSpeed > 0
    bool scan{false};
    float scan_countdown_518{0.0f};
    bool steered{false};            // a target existed after the scan
    float commanded_heading_46c{0.0f};
    bool homed{false};              // both turns made; the caller returns
    float right_axis_turn{0.0f};    // rotation about +74h (pitch turn, or own_pitch when not homed)
    float up_axis_turn{0.0f};       // rotation about +84h (homed only)
};
// `wrap_subtract` is 00438B10 (BSP_Math_SubtractWrappedAngle(a, b)). The scan
// itself (when `scan` comes back true) is the caller's: it decides has_target.
TorpedoHomingResult torpedo_homing_countdown_00856bb0(const TorpedoHomingInputs& in) noexcept;
TorpedoHomingResult torpedo_homing_turn_00856bb0(const TorpedoHomingInputs& in,
                                                 TorpedoHomingResult after_countdown,
                                                 const std::function<float(float, float)>& wrap_subtract);

// 00855B90 (vtable 00D0C3E8 slot 68h, data ref 00D0C450) and 00855BB0: node
// +464h shown / hidden through 00B6DA70(1.0 / 0.0, 0). 00855BD0 (no
// reference in the image): the create message's target, net id +128h through
// the 00F89A0C / 00F89A60 tables, 0 for id 0 (the same lookup 00857160 inlines).

// 00855B30, __thiscall(this, float dt), RET 4: 006E2220 extrapolation, then,
// with a propeller node at +4A0h, the node matrix is cached at +4A4h and its
// basis turned about the axis +4C4h by dt * 26.1799 rad (1500 deg/s).
// Presentation only; the returned angle is what 0085E880 receives.
float torpedo_propeller_angle_00855b30(float dt) noexcept;

// 00857C80, the torpedo boat's vtable slot A0h (SEntityInit) override: the
// unit init 00822C20, then, when world+19D0h is set, 0071DD30(director
// [+738h], [director+1CCh] & ~4); then +1188h = class+808h "TurboTime"
// (00857F40 reads it) and +118Ch = 0. Returns the two fields; the director
// message is the caller's.
struct TorpedoBoatTurboState {
    float turbo_time_1188{0.0f};
    bool turbo_flag_118c{false};
    bool send_director_flags{false};
    std::uint32_t director_flags{0};
};
TorpedoBoatTurboState torpedo_boat_sentity_init_00857c80(float class_turbo_time_808,
                                                         bool world_flag_19d0,
                                                         std::uint32_t director_flags_1cc) noexcept;

// 00852300 / 00852350 (HUD helpers; callers 0064DD30, 005A6AD0, 0064AE20):
// over the unit's children (+48h, next +44h), count those answering
// vtable[5Ch](25h); 00852300 also needs vtable[210h](2Dh, 0) true.
struct UnitChildDevice {
    bool is_kind_25{false};
    bool answers_210_2d{false};
};
int unit_count_devices25_ready_00852300(const std::vector<UnitChildDevice>& children) noexcept;
int unit_count_bomb_platform_children_00852350(const std::vector<UnitChildDevice>& children) noexcept;

// 00857D40, __thiscall(boat), RET -> EAX: vtable[38h]() (the body speed)
// > 3.0 (00CE3854; FCOMPI, JBE).
bool torpedo_boat_is_body_speed_above_3_00857d40(float body_speed) noexcept;

// ===========================================================================
// MWaterMine (0085E9C0-0085F8CC; class id 34h, 470h bytes, entity vtable
// 00D0D130, shot interface +310h with vtable 00D0D0C0).
// ===========================================================================

// 0085F500 after the bomb reader.
struct WaterMineClassFields {
    float v0_random_factor_d8{0.0f};  // "V0RandomFactor"
    float dive_speed_dc{0.0f};        // "DiveSpeed"
    bool has_water_splash_efx_e0{false};  // "WaterSplashEfx"
    float floating_depth_e4{-3.0f};   // "FloatingDepth", default 00CE3D50
    float trigger_radius_e8{5.0f};    // "TriggerRadius", default 00CE3850
    float blast_damage_min_b4{0.0f};  // "Blast.BlastDamageMin"
    float blast_damage_max_b8{0.0f};  // "Blast.BlastDamageMax"
};

// 0085EA40 (the water mine's sensor-category slot): MOV EAX, 4; RET. The mine
// is always category 4 (underwater, the value 00852B90 gives a submarine
// between periscope and deep depth).
constexpr int water_mine_sensor_category_0085ea40() noexcept { return 4; }

// 0085F500, __thiscall(class, LuaObject* row), RET 4, after the bomb reader
// 006E1BE0. The first two keys use 00B66270 (no default argument), the
// effect 00B66380(0, 1), the last two 00B66330 with the defaults above.
struct WaterMineClassRow {
    std::function<float(const char* key)> number;                       // 00B66270
    std::function<float(const char* key, float fallback)> number_or;    // 00B66330
    std::function<bool(const char* key)> effect_present;                // 00B66380 + 00870CD0
};
void water_mine_read_class_fields_0085f500(const WaterMineClassRow& row,
                                           WaterMineClassFields& cls);

// 0085F6D0, the shot's water entry, __thiscall(shot, char arg), RET 4:
// SetWeaponScale(1.0) always; a shot not yet in water runs 006E6450(arg) and,
// with WaterSplashEfx set, a point effect at the pose (effect+9 = 1). No
// break-up test, unlike the torpedo's 008568E0. Returns whether 006E6450 ran.
struct WaterMineEnterWaterHost {
    std::function<void(float)> set_weapon_scale;
    std::function<void()> set_water_mode;       // 006E6450(arg)
    std::function<void()> spawn_splash_effect;  // 00868420
};
bool water_mine_enter_water_0085f6d0(const WaterMineClassFields& cls, bool in_water_44,
                                     const WaterMineEnterWaterHost& host);

// 0085F2F0 / 0085F1E0: a message of type B5h (MT_WATERMINE_ARM) arms the mine:
// +46Ch = 1 and a 1ACh part instance (007135C0(mine, +464h)) is stored at
// +468h and attached under world+19ECh (00B6D890); water_mine_arm_0085f1e0
// below models that body. Other types go to 006E09F0.
// Returns true when the message is the arm message (AL = 1).
inline bool water_mine_message_arms_0085f2f0(std::uint8_t message_type_10) noexcept {
    return message_type_10 == 0xB5;
}

// 0085EBC0, __thiscall(this), RET 0. After 006E2E10: a mine whose shot is not
// in water gets SetWeaponScale(0.25); off a network client its velocity is
// scaled by U(1 - V0RandomFactor, 1 + V0RandomFactor) drawn on STREAM 1 (ECX
// = 1 at 0085EC26); then shot vtable[34h](00F87574, 00F87574), +46Ch armed =
// 0 and +468h marker = 0.
struct WaterMineActivateHost {
    std::function<void()> base_activate;                       // 006E2E10
    std::function<void(float)> set_weapon_scale;               // shot vtable[20h]
    std::function<float(int stream, float lo, float hi)> uniform;  // 00BD2F10
    std::function<void()> shot_slot34_with_zero_vectors;       // shot vtable[34h]
};
Vec3 water_mine_activate_0085ebc0(const WaterMineClassFields& cls, const Vec3& velocity,
                                  bool shot_in_water, bool network_client,
                                  const WaterMineActivateHost& host);

// 0085ECD0, __thiscall(this, float dt), RET 4: the swim step. Drag toward
// rest (-9.81 / DiveSpeed per unit velocity) plus a vertical spring that pulls
// y toward FloatingDepth + 0.25 * water height; then, off a network client,
// arming or the trigger test.
struct WaterMineShip {
    Vec3 world_position{};   // ship +FCh..+104h
    Vec3 mine_in_ship{};     // 004142E0(mine pose, ship inverse +110h)
    float length_a0{0.0f};   // ship class +A0h
    float width_a4{0.0f};    // ship class +A4h
    float height_a8{0.0f};   // ship class +A8h
};
struct WaterMineStepInputs {
    Vec3 velocity_318{};
    float sim_y_a8{0.0f};        // +A8h, the simulated height
    Vec3 world_position{};       // +FCh..+104h after 00414DB0
    float water_height{0.0f};    // 0078CF20(world, x, z)
    bool armed_46c{false};
    bool network_client{false};
    float dt{0.0f};
    std::vector<WaterMineShip> ships;            // world+60h list (head +64h)
    std::vector<Vec3> depth_charge_positions;    // world+228h list (head +22Ch)
};
struct WaterMineStepResult {
    Vec3 velocity_318{};
    bool send_arm_message{false};  // MT_WATERMINE_ARM (B5h) via 0077C2A0(msg, 2, 0)
    bool detonate{false};          // 00926D90(1)
};
WaterMineStepResult water_mine_step_0085ecd0(const WaterMineClassFields& cls,
                                             const WaterMineStepInputs& in) noexcept;

// 0085F160, __thiscall(this, record*), RET 4 -> AL = 0: an armed mine hit by
// anything that is not a water mine (record+4h null or not IsKindOf(34h))
// detonates (00926D90(1)).
bool water_mine_hit_detonates_0085f160(bool armed_46c, bool has_source,
                                       bool source_is_water_mine) noexcept;

// 0085F7A0, the vtable +7Ch override (tail-calls 0077D270). Only for kill
// cause +70h == 1: damage U(BlastDamageMin, BlastDamageMax) on STREAM 1 (ECX
// = 1 at 0085F7C7), radial damage 0084BAD0 at the pose with BlastRange, then
// ExplWaterEfx at the pose with y moved to the water height.
struct WaterMineKilledHost {
    std::function<float(int stream, float lo, float hi)> uniform;   // 00BD2F10
    std::function<void(float damage)> apply_radial_damage;         // 0084BAD0
    std::function<void()> spawn_explosion_at_water_surface;        // 00868420, effect+9 = 1
    std::function<void()> base_on_killed;                          // 0077D270
};
void water_mine_on_killed_0085f7a0(const WaterMineClassFields& cls, std::int32_t kill_cause_70,
                                   const WaterMineKilledHost& host);

// ===========================================================================
// MSubmarine lifecycle (cc13-gpr53d): create, construct, state writer, world
// list detach, on-killed effect release, destructor; periscope debris init.
// No RNG anywhere in this group.
// ===========================================================================

// 00852F10, __thiscall(instance, int flag), RET 4. After the five-level base
// 0081ED40(flag) it writes the eight submarine vptrs (table below; the leaf
// has no vptr rewrite of its own in 00853110) and these leaf defaults. Every
// other leaf field is zero from 008531A0's memset.
struct LeafVptrStore {
    std::uint32_t offset;
    std::uint32_t table;
};
inline constexpr LeafVptrStore kSubmarineVptrs_00852f10[8] = {
    {0x000, 0x00d0bf80}, {0x010, 0x00d0bf68}, {0x024, 0x00d0bf60}, {0x170, 0x00d0bf5c},
    {0x1e4, 0x00d0bf54}, {0x310, 0x00d0bf3c}, {0x38c, 0x00d0bf38}, {0x72c, 0x00d0bf34},
};
struct SubmarineConstructDefaults {
    std::uintptr_t periscope_effect_1228{0};
    float air_127c{1.0f};                    // MOVSS from 00D7A24C (1.0f)
    bool unlimited_air_1280{false};
    bool need_air_1281{false};
    bool periscope_auto_raise_1235{false};
    std::int32_t scan_step_1238{0};
    std::int32_t class_id_c4{8};             // 00852F96
    float submerge_force_aux_1274{0.0f};     // XORPS, 00852FA0
};
SubmarineConstructDefaults submarine_construct_defaults_00852f10() noexcept;

// 008531A0, class descriptor vtable 00D1AE38 slot 28h, __thiscall(descriptor,
// int flag), RET 4: operator new(1288h) (00BF55BE), memset of the RAW return
// (00BF79F0, before the null test), 00852F10(flag) when non-null, then
// 009553D0(descriptor) with ECX = the unit and unit+354h = descriptor. A
// failed allocation reaches 009553D0 and the +354h store with a null unit
// (no guard in the image); the model reports that case instead of crashing.
struct SubmarineCreateInstanceHost {
    std::function<void*(std::uint32_t size)> allocate;           // 00BF55BE
    std::function<void(void* block, std::uint32_t size)> zero;    // 00BF79F0
    std::function<void*(void* block, std::int32_t flag)> construct; // 00852F10
    std::function<void(void* unit)> set_vehicle_class;           // 009553D0(descriptor)
    std::function<void(void* unit)> store_descriptor_354;
};
inline constexpr std::uint32_t kSubmarineInstanceSize = 0x1288;
struct SubmarineCreateInstanceResult {
    void* unit{nullptr};
    bool null_unit_dereferenced{false};      // the image would fault here
};
SubmarineCreateInstanceResult submarine_create_instance_008531a0(std::int32_t flag,
                                                                 const SubmarineCreateInstanceHost& host);

// 00852D30, submarine vtable 00D0BF80 slot A8h, __thiscall(unit, sink), RET 4.
// After the ship writer 00818340(sink) it emits the "_submarine" table that
// 00853E10 reads back on a saved-game load (creation kind 3). Every key and
// value is an 8-byte struct passed by value: key {0 (string), const char*},
// value {type, payload} with type 0 string, 1 int, 2 float, 3 bool (a byte).
// Sink vtable +4h(key) opens a table, +0Ch(key, value) writes a field, +8h()
// closes the table. Emit order (00852D3F..00852F0A):
//   open "_submarine"; open "airSupply"; "air" float +127Ch; "needAir" bool
//   +1281h; "unlimitedAir" bool +1280h; close; "depthLevel" int +1268h;
//   "periscopeState" int +122Ch; "periscopeY" float +1230h; "periscopeOut"
//   bool +1234h; "sullyesztoEro" float +1270h; close.
enum class StateSinkType : std::int32_t { string = 0, integer = 1, real = 2, boolean = 3 };
struct StateSinkEmit {
    enum class Op : std::uint8_t { open_table, field, close_table };
    Op op{Op::field};
    const char* key{nullptr};                // nullptr for close_table
    StateSinkType type{StateSinkType::integer};
    std::int32_t integer{0};
    float real{0.0f};
    bool boolean{false};
};
struct SubmarineSavedState {
    float air_127c{1.0f};
    bool need_air_1281{false};
    bool unlimited_air_1280{false};
    std::int32_t depth_level_1268{0};
    std::int32_t periscope_state_122c{0};
    float periscope_y_1230{0.0f};
    bool periscope_out_1234{false};
    float submerge_force_1270{0.0f};         // "sullyesztoEro"
};
// The submarine's own emits; the caller runs the ship writer 00818340 first.
std::vector<StateSinkEmit> submarine_describe_state_00852d30(const SubmarineSavedState& s);

// World-list registration and detach, slots 130h / 134h of the leaf vtables
// (__fastcall(entity), RET 0). Registration: the parent-entity list
// (00928560), then 00484540 push_back of the entity on each world list at
// [entity+30h]+offset in order. Detach: the base detach, then the class list
// at [entity+30h]+offset (head at offset+4h; node next +4h, payload +8h) is
// walked and the FIRST node whose payload is the entity is erased with
// 004837D0. The detach returns that node's index, or -1 when there is none.
// The submarine's and torpedo boat's registrars (00853090, 00857EB0) are in
// src/unit_world_registration.cpp.
struct WorldListRegistration {
    std::uint32_t function;
    std::uint32_t list_offsets[3];
};
inline constexpr WorldListRegistration kTorpedoRegistration_00856360{0x00856360, {0x030, 0x210, 0x21c}};
inline constexpr WorldListRegistration kWaterMineRegistration_0085f270{0x0085f270, {0x030, 0x210, 0x288}};
struct WorldListRegisterHost {
    std::function<void()> register_in_parent_entity_list;        // 00928560
    std::function<void(std::uint32_t list_offset)> push_back;     // 00484540
};
void torpedo_register_in_world_lists_00856360(const WorldListRegisterHost& host);
void water_mine_register_in_world_lists_0085f270(const WorldListRegisterHost& host);

struct ClassWorldListDetach {
    std::uint32_t function;
    std::uint32_t base_detach;   // 006DFFC0 for the ship family, 006E29C0 for the bomb family
    std::uint32_t list_offset;
};
inline constexpr ClassWorldListDetach kSubmarineDetach_008530e0{0x008530e0, 0x006dffc0, 0x078};
inline constexpr ClassWorldListDetach kTorpedoDetach_008563a0{0x008563a0, 0x006e29c0, 0x21c};
inline constexpr ClassWorldListDetach kTorpedoBoatDetach_00857f00{0x00857f00, 0x006dffc0, 0x0c0};
inline constexpr ClassWorldListDetach kWaterMineDetach_0085f2b0{0x0085f2b0, 0x006e29c0, 0x288};
struct WorldListDetachHost {
    std::function<void()> base_detach;                            // row's base_detach
    std::function<void(std::size_t node_index)> erase_node;       // 004837D0
};
int submarine_detach_from_world_lists_008530e0(const void* unit,
                                               const std::vector<const void*>& class_list_payloads,
                                               const WorldListDetachHost& host);
int torpedo_detach_from_world_lists_008563a0(const void* torpedo,
                                             const std::vector<const void*>& class_list_payloads,
                                             const WorldListDetachHost& host);
int torpedo_boat_detach_from_world_lists_00857f00(const void* unit,
                                                  const std::vector<const void*>& class_list_payloads,
                                                  const WorldListDetachHost& host);
int water_mine_detach_from_world_lists_0085f2b0(const void* mine,
                                                const std::vector<const void*>& class_list_payloads,
                                                const WorldListDetachHost& host);

// The periscope effect at +1228h is reference counted (count at +4h,
// InterlockedDecrement 00CE2220, vtable[0] destroys at zero).
struct SubmarineLifecycleSlots {
    std::uintptr_t periscope_effect_1228{0};
    std::uintptr_t periscope_debris_1224{0}; // the 6Ch PeriszkopSullyesztoDolog piece
    bool debris_name_has_storage{false};     // debris+0Ch (string data) non-null
};

// 00853110, __thiscall(unit), RET 0, sole caller the scalar deleting dtor
// 00853380 (adjustor thunks 00853010/20/30/40 for the +24h/+72Ch/+310h/+10h
// subobjects). Releases +1228h and clears it, then runs the vehicle base
// 0081F3A0 (bsp/unit_destructor_levels.hpp). It does NOT free the debris
// +1224h: only 00853590 does (scan of disp32 1224h: 00853590, 00853630,
// 00854650 only), so a submarine destroyed without being killed leaks it.
struct SubmarineDestructHost {
    std::function<void(std::uintptr_t effect)> release_reference; // refs at +4h
    std::function<void()> destruct_vehicle_base;                  // 0081F3A0
};
void submarine_destruct_00853110(SubmarineLifecycleSlots& slots, const SubmarineDestructHost& host);

// 00853590, submarine vtable 00D0BF34 slot D0h, __fastcall(unit). With an
// effect at +1228h: 00867B10 stops its children, byte effect+9 = 1, the
// reference is released and +1228h cleared. With debris at +1224h: its name
// string's block (data +0Ch, size +8h + 1) goes back to the sized pool
// (00419CC0 / 00BD1510) when non-null, the 6Ch piece is freed (_free) and
// +1224h cleared. Then tail JMP 00819880 (the ship family's on-killed).
// Presentation only.
struct SubmarineOnKilledHost {
    std::function<void(std::uintptr_t effect)> stop_effect_children;  // 00867B10
    std::function<void(std::uintptr_t effect)> mark_effect_finished;  // byte +9 = 1
    std::function<void(std::uintptr_t effect)> release_reference;
    std::function<void(std::uintptr_t debris)> return_name_storage;   // 00419CC0 + 00BD1510
    std::function<void(std::uintptr_t debris)> free_debris;           // _free 00BF65AC
    std::function<void()> base_on_killed;                             // 00819880
};
void submarine_on_killed_release_periscope_00853590(SubmarineLifecycleSlots& slots,
                                                    const SubmarineOnKilledHost& host);

// 008533E0, __thiscall(debris, node), RET 4, sole caller SEntityInit 00853630
// (00853CE6) right after operator new(6Ch) set +0h = the submarine and
// cleared the name (+8h/+0Ch). Layout: +0h owner, +4h node (the "periszkop"
// node +1214h), +8h name string, +10h state, +14h velocity, +20h angular
// velocity, +2Ch 4x4 matrix. Velocity and angular velocity copy the zero
// vector 00F87574 (no literal-address writer), state = 0, name =
// "PeriszkopSullyesztoDolog:" + [owner] vtable[10h]() (004261A0 concat), then
// the node's world matrix (00B6DB70 when node+5Ch bit 1 is clear) is copied
// from node+F0h (004134F0). Presentation only.
struct PeriscopeDebrisInitHost {
    std::function<std::string()> owner_name;        // [owner] vtable[10h]
    std::function<bool()> node_world_matrix_current; // node+5Ch bit 1
    std::function<void()> refresh_node_world_matrix; // 00B6DB70
    std::function<void()> copy_node_world_matrix;    // 004134F0(node+F0h) -> +2Ch
};
inline constexpr char kPeriscopeDebrisNamePrefix[] = "PeriszkopSullyesztoDolog:";
inline constexpr std::uint32_t kPeriscopeDebrisSize = 0x6c;
// Returns the name at +8h.
std::string periscope_debris_init_008533e0(PeriscopeDebris& d, const PeriscopeDebrisInitHost& host);

// ===========================================================================
// MTorpedo / MWaterMine lifecycle (cc13-gpr53d). No RNG in this group.
// ===========================================================================

// 00855D30, torpedo vtable 00D0C3E8 slot A8h, __thiscall(torpedo, sink),
// RET 4: after the bomb writer 006E0F40(sink), the "_torpedo" table (key
// 00D0C350) that 00857780 reads back on creation kind 3. Every value is type
// 2 (float), MOVSS from the field. "swimdepth" (00CFE74C) is written TWICE,
// at 00855D8C and again at 00855E58; the second write repeats +47Ch.
//   open "_torpedo"; heading +46Ch; swimdepth +47Ch; accel +470h; dragfront
//   +474h; dragside +478h; swimdepth +47Ch; depthsmoothrate +480h; waverate
//   +484h; close.
struct TorpedoSavedState {
    float heading_46c{0.0f};
    float accel_470{0.0f};
    float drag_front_474{0.0f};
    float drag_side_478{0.0f};
    float swim_depth_47c{-2.5f};
    float depth_smooth_rate_480{0.0f};
    float wave_rate_484{0.0f};
};
// The torpedo's own emits; the caller runs the bomb writer 006E0F40 first.
std::vector<StateSinkEmit> torpedo_write_state_00855d30(const TorpedoSavedState& s);

// 008562A0, __thiscall(torpedo), RET 0, sole caller the scalar deleting dtor
// 00856340. Step order: the seven torpedo vptrs (table below), the member at
// +4FCh (0042BCA0), the member at +4E4h (004B7EF0), then the bomb base
// 006E2A20. The constructor 00856050 writes the same seven.
inline constexpr LeafVptrStore kTorpedoVptrs_008562a0[7] = {
    {0x000, 0x00d0c3e8}, {0x010, 0x00d0c3cc}, {0x024, 0x00d0c3c4}, {0x170, 0x00d0c3c0},
    {0x1e4, 0x00d0c3b8}, {0x310, 0x00d0c378}, {0x3e4, 0x00d0c35c},
};
enum class TorpedoDestructStep : std::uint8_t {
    rewrite_vptrs,             // 008562BD..008562EF
    destruct_member_4fc,       // 00856307 -> 0042BCA0
    destruct_member_4e4,       // 00856317 -> 004B7EF0
    destruct_bomb_base,        // 00856326 -> 006E2A20
};
inline constexpr TorpedoDestructStep kTorpedoDestructSteps_008562a0[4] = {
    TorpedoDestructStep::rewrite_vptrs, TorpedoDestructStep::destruct_member_4fc,
    TorpedoDestructStep::destruct_member_4e4, TorpedoDestructStep::destruct_bomb_base,
};

// 0085E9C0, __thiscall(mine, flag), RET 4: the bomb base 006E2670(flag), the
// seven water-mine vptrs, then class id +C4h = 34h (MWaterMine). The image
// writes no other field here.
inline constexpr LeafVptrStore kWaterMineVptrs_0085e9c0[7] = {
    {0x000, 0x00d0d130}, {0x010, 0x00d0d114}, {0x024, 0x00d0d10c}, {0x170, 0x00d0d108},
    {0x1e4, 0x00d0d100}, {0x310, 0x00d0d0c0}, {0x3e4, 0x00d0d0a8},
};
inline constexpr std::int32_t kWaterMineClassId_0085e9c0 = 0x34;

// 00857CD0, __thiscall(boat, flag), RET 4: the five-level base 0081ED40(flag),
// the eight torpedo-boat vptrs, then class id +C4h = 0Eh (MTorpedoBoat).
// 00857E20 (vehicle class vtable 00D1AE74 slot 2Ch) is the 008531A0 pattern
// with operator new(1190h) and this constructor.
inline constexpr LeafVptrStore kTorpedoBoatVptrs_00857cd0[8] = {
    {0x000, 0x00d0c648}, {0x010, 0x00d0c630}, {0x024, 0x00d0c628}, {0x170, 0x00d0c624},
    {0x1e4, 0x00d0c61c}, {0x310, 0x00d0c604}, {0x38c, 0x00d0c600}, {0x72c, 0x00d0c5fc},
};
inline constexpr std::int32_t kTorpedoBoatClassId_00857cd0 = 0x0e;
inline constexpr std::uint32_t kTorpedoBoatInstanceSize_00857e20 = 0x1190;

// 0085F310, water-mine class vtable 00CFA59C slot 20h, __thiscall(class, a1..a7),
// RET 1Ch; only a1..a4 are read. The 00856420 pattern without launch params:
// operator new(470h), memset of the raw return, 0085E9C0(a1) when non-null
// (a null mine is dereferenced at 0085F379; no guard), mine+54h = a2,
// mine+314h = class, vtable[98h](0, [[00E188A8]+19CCh], identity), then
// velocity +318h = (dir.x * V0, dir.y * V0, V0 * dir.z) with V0 = float
// class+50h (each product stored as a float), forward row +94h = dir (a4),
// 0085DC80, position a3 into +A4h, shot+60h and shot+6Ch, bytes +C8h and
// +10Ch = 0, and 0042ED50 on every child of +48h (next +44h). Returns the
// shot interface +310h. No RNG (the activate 0085EBC0 draws later).
struct WaterMineCreateResult {
    Vec3 velocity_318{};
    Vec3 forward_94{};
    Vec3 position_a4{};      // also shot+60h and shot+6Ch
    std::uint32_t arg2_54{0};
};
WaterMineCreateResult water_mine_create_0085f310(float class_v0_50, std::uint32_t arg2,
                                                 const Vec3& position, const Vec3& direction,
                                                 const TorpedoCreateHost& host);

// The armed part of a water mine: +464h the mine's node/model, +468h the 1ACh
// part instance built on arming, +46Ch armed.
struct WaterMineArmState {
    std::uintptr_t model_464{0};
    std::uintptr_t armed_part_468{0};
    bool armed_46c{false};
};
// 0085F1E0, __fastcall(mine), reached from the B5h (MT_WATERMINE_ARM) message
// handler 0085F2F0: armed = 1 (0085F1FE), then operator new(1ACh) (00BF681B)
// and the part construct 007135C0(mine, [+464h]) into +468h (0085F230; null
// when the allocation fails), then 00B6D890, the root registration of node
// [[00E188A8]+19ECh].
struct WaterMineArmHost {
    std::function<std::uintptr_t(std::uintptr_t model)> construct_part;  // new(1ACh) + construct
    std::function<void()> propagate_root_registration;                   // [[00E188A8]+19ECh]
};
void water_mine_arm_0085f1e0(WaterMineArmState& s, const WaterMineArmHost& host);
// 0085EA50, slot B0h: the armed part when armed, else null.
std::uintptr_t water_mine_model_pointer_0085ea50(const WaterMineArmState& s) noexcept;
// 0085F120, slot 84h: with a part at +468h, +464h = 0 FIRST, then the part's
// vtable+4(1) (deleting destructor) and +468h = 0; then tail JMP 006E1860
// (the bomb family's release). The armed flag is left set.
struct WaterMineReleaseHost {
    std::function<void(std::uintptr_t part)> delete_part;   // vtable+4(1)
    std::function<void()> base_release;                     // 006E1860
};
void water_mine_release_sub_objects_0085f120(WaterMineArmState& s, const WaterMineReleaseHost& host);

// ===========================================================================
// Effect-instance Update bodies (slot 11, primary +28h, __thiscall(instance,
// float dt, node reference), RET 8), the three docs/CC13_EFFECTS.md left
// open. Presentation only; none of them draws from any RNG. Instance layout
// as src/effects_cc13.cpp: +10h the point effect (its node at +110h), +14h
// the definition, +1Ch the payload, +20h the per-kind field.
// ===========================================================================

// 008738B0, light (vtable 00D0DBEC). With a light at +1Ch: the point node's
// world position (00B6DB70 when node+5Ch bit 1 is clear; +120h..+128h) goes
// to light vtable+30h, 00B7B090([[00E188A8]+19ECh]) links it if unlinked,
// then light+1F8h = max(float(float(+20h / def+20h) * def+24h), 0.01f
// (00D7A238)); the FCOMPI/JBE also takes 0.01 when the product is NaN. Then
// +20h = float(+20h - dt), and 0 when that is below 0 (a NaN stays). The
// node-reference argument is not read.
struct EffectLightUpdateHost {
    std::function<Vec3()> point_node_world_position;
    std::function<void(const Vec3&)> set_light_position;   // light vtable+30h
    std::function<void()> link_if_unlinked;                // 00B7B090
    std::function<void(float)> set_intensity_1f8;
};
struct EffectLightInstanceState {
    bool has_light_1c{false};
    float remaining_20{0.0f};
};
// Returns the intensity written to light+1F8h, or 0 with no light.
float update_effect_light_instance_008738b0(EffectLightInstanceState& s, float def_duration_20,
                                            float def_intensity_24, float dt,
                                            const EffectLightUpdateHost& host);

// 00873100, waterdrops (vtable 00D0DBB4). Byte 00F889FC clear forces the
// pending flag +20h to 0. While pending: d = point node pos - reference node
// pos (both refreshed first; each component a float), len = 00419440(d),
// x = len - FadeDistance (def+34h), 0 when not above 0 (FSUBP DE EA, JBE),
// f = x / FadeDistance (FDIVRP DE F2) clamped to [0, 1] (FCOMPI/JBE then
// COMISS/JBE: a NaN passes both), g = float(1 - f) (FSUBRP DE E1), count =
// FILD def+44h (+2^32 as a float, 00CE3978, when negative), FMUL g, then
// FISTP qword under RC = truncate (OR 0C00h); the low dword is the count.
// A non-zero count creates the drops (00BC2D00) into +1Ch and runs, in order:
// size range (def+28h, +2Ch) 00BC1160, life range (+20h, +24h) 00BC1180,
// gravity +30h 00BC11B0, texture +48h 00BC1990, count 00BC11A0, sticky
// percent +3Ch 00BC11D0, bytes +40h 00BC11F0, +41h 00BC1200, +42h 00BC1210,
// +43h 00BC1220, then spawn 00BC1A20([[00E188A8]+19ECh], [..+19FCh]). The
// old +1Ch is overwritten without a release. Pending = 0 either way. Then,
// with drops at +1Ch: 00BC12F0(reference node, camera pos, dt), the camera
// position from node 00E188DC (+120h..+128h, refreshed) or zero when null.
struct WaterdropsDefinitionFields {
    float life_min_20{0.0f};
    float life_max_24{0.0f};
    float size_min_28{0.0f};
    float size_max_2c{0.0f};
    float gravity_30{0.0f};
    float fade_distance_34{0.0f};
    float sticky_percent_3c{0.0f};
    bool apply_at_bottom_only_40{false};
    bool start_top_off_screen_41{false};
    bool apply_at_top_only_42{false};
    bool no_camera_move_43{false};
    std::int32_t count_44{0};
    std::uint32_t texture_48{0};
};
// The spawn count arithmetic alone (0087317D..00873293).
std::int32_t waterdrops_spawn_count_00873100(float distance, float fade_distance_34,
                                             std::int32_t count_44) noexcept;
struct WaterdropsUpdateHost {
    std::function<bool()> enabled_f889fc;
    std::function<Vec3()> point_node_position;
    std::function<Vec3()> reference_node_position;
    std::function<float(const Vec3&)> length;                       // 00419440
    // 00BC2D00, the ten setters and 00BC1A20 in the order above.
    std::function<std::uintptr_t(const WaterdropsDefinitionFields&, std::int32_t count)> create_and_spawn;
    std::function<bool(Vec3& position)> camera_position;            // node 00E188DC, false when null
    std::function<void(std::uintptr_t drops, const Vec3& camera, float dt)> update;  // 00BC12F0
};
struct EffectWaterdropsState {
    std::uintptr_t drops_1c{0};
    bool pending_20{true};      // the constructor 00873090 sets it
};
void update_effect_waterdrops_instance_00873100(EffectWaterdropsState& s,
                                                const WaterdropsDefinitionFields& def, float dt,
                                                const WaterdropsUpdateHost& host);

// 00873C80, thunderstorm (vtable 00D0DDCC). With +1Ch: its vtable+5Ch
// ([[00E188A8]+19FCh], dt). With the storm +20h: 00BC0510([..+19FCh], dt,
// &strike) returns AL; on a strike a point effect is created by 00868420 from
// the identity matrix whose translation row is the strike position (root
// [..+19ECh], definition [..+19E0h] with its count incremented, options 0, 0);
// a non-null result gets byte +9 = 1 (finish when done) and the local
// reference is released. Returns whether it struck.
struct ThunderStormUpdateHost {
    std::function<void(float dt)> update_atlas_1c;                         // vtable+5Ch
    std::function<bool(float dt, Vec3& strike)> update_storm;              // 00BC0510
    std::function<std::uintptr_t(const float (&matrix)[16])> spawn_strike; // 00868420
    std::function<void(std::uintptr_t effect)> mark_finished;              // byte +9 = 1
    std::function<void(std::uintptr_t effect)> release_reference;
};
struct EffectThunderStormState {
    bool has_atlas_1c{false};
    bool has_storm_20{false};
};
bool update_effect_thunderstorm_instance_00873c80(const EffectThunderStormState& s, float dt,
                                                  const ThunderStormUpdateHost& host);

// ===========================================================================
// Effect instance constructors and destructors (the lifecycle around the
// src/effects_cc13.cpp virtuals). Every constructor is __thiscall(instance,
// definition, point), RET 8, after operator new in its kind's CreateInstance
// (flare 00869300 20h, thunderstorm 00869370 24h, waterdrops 008693E0 24h,
// splash 008695B0 20h). The common prefix: table 00CEB130, references 1,
// secondary 00D0C8C0, active byte +0Ch = 1, +10h point, +14h definition,
// then the kind's two tables and +18h type.
// ===========================================================================

// 00873010: tables 00D0DC60 / 00D0DC5C, type 5, payload +1Ch = 0 (the flare
// itself is not created here).
void construct_effect_flare_instance_00873010(effects_cc13::EffectNodeInstanceStorage& instance,
                                              void* definition, PointEffectInstanceStorage* point) noexcept;
// 00873090: tables 00D0DBB8 / 00D0DBB4, type 0, payload 0, byte +20h = 1
// (the spawn-pending flag 00873100 consumes).
void construct_effect_waterdrops_instance_00873090(effects_cc13::EffectNodeInstanceStorage& instance,
                                                   void* definition, PointEffectInstanceStorage* point) noexcept;
// 008739A0: type 0 (written before the table switch), tables 00D0DC28 /
// 00D0DC24, +1Ch = 0, then the point node world position (00B6DB70 when +5Ch
// bit 1 is clear) and +1Ch = 00B9E3F0 BSP_OceanWaveField_AddWaterRing(&pos,
// def+20h, def+24h, def+28h) on ([00E188A8]+19F0h)+A8h, with no null test of
// the ocean (unlike the destructor 00872D60).
struct EffectSplashConstructHost {
    std::function<Vec3()> point_node_world_position;
    std::function<effects_cc13::EffectSplashRingFields*(const Vec3& pos, float a20, float a24, float a28)>
        add_water_ring;
};
void construct_effect_splash_instance_008739a0(effects_cc13::EffectSplashInstanceStorage& instance,
                                               void* definition, float def_20, float def_24, float def_28,
                                               PointEffectInstanceStorage* point,
                                               const EffectSplashConstructHost& host);

// 00872100, thunderstorm slot 13 (primary +30h, Stop): a bare RET.
inline void stop_effect_thunderstorm_instance_00872100() noexcept {}
// 00872110, thunderstorm slot 3 (primary +08h, IsComplete): XOR AL,AL; RET.
constexpr bool effect_thunderstorm_instance_is_complete_00872110() noexcept { return false; }

// The thunderstorm instance (24h): +1Ch the sky effect, +20h the storm (54h).
struct EffectThunderStormInstanceStorage {
    NativeRegisteredEffectPrefixStorage prefix;   // type_18 = 0
    std::uintptr_t sky_effect_1c{0};
    std::uintptr_t storm_20{0};
};
// 00873EB0: type 0, tables 00D0DDD0 / 00D0DDCC, the atlas
// "Effects\Lightning\lightning.ats" is loaded (00AF0060), +1Ch =
// [010909B0] vtable+10h(point node +110h, def+24h), +20h = 0; with byte
// def+54h set, +20h = new(54h) 00BC0D40(point node +110h) (null when the
// allocation fails).
struct EffectThunderStormConstructHost {
    std::function<void()> load_atlas;                          // 00AF0060
    std::function<std::uintptr_t()> create_sky_effect;         // [010909B0] vtable+10h
    std::function<std::uintptr_t()> create_storm;              // new(54h) + 00BC0D40
};
void construct_effect_thunderstorm_instance_00873eb0(EffectThunderStormInstanceStorage& instance,
                                                     void* definition, bool def_storm_54,
                                                     PointEffectInstanceStorage* point,
                                                     const EffectThunderStormConstructHost& host);
// 00873FF0, between its table writes (00D0DDD0/00D0DDCC first, 00D0C88C /
// 00D0C888 then 00BD30F0 last): a storm at +20h is destroyed (00BC04C0
// BSP_LightningStorm_Destroy) and freed (00BF65AC), +20h = 0, then the atlas is
// unloaded. +1Ch (the sky effect) is NOT released here.
struct EffectThunderStormDestructHost {
    std::function<void(std::uintptr_t storm)> destroy_and_free_storm;
    std::function<void()> unload_atlas;
};
void destruct_effect_thunderstorm_instance_00873ff0(EffectThunderStormInstanceStorage& instance,
                                                    const EffectThunderStormDestructHost& host);
// 00872C10 waterdrops and 00872CB0 light: between the table writes they run
// the same body as their Stop slots 00872BF0 / 00872D30
// (src/effects_cc13.cpp); these wrappers call those.
void destruct_effect_waterdrops_instance_00872c10(effects_cc13::EffectNodeInstanceStorage& instance,
                                                  effects_cc13::EffectInstanceServices& services);
void destruct_effect_light_instance_00872cb0(effects_cc13::EffectNodeInstanceStorage& instance,
                                             effects_cc13::EffectInstanceServices& services);
// 00872D60 splash: the ring at +1Ch is removed (00B9E4A0 on ([00E188A8]+19F0h)
// +A8h) only when the ocean +19F0h exists and the ring is set. Unlike the
// Stop slot 00872DF0 it neither zeroes the ring timer nor clears +1Ch.
struct EffectSplashDestructHost {
    std::function<bool()> ocean_present;                       // [[00E188A8]+19F0h] != 0
    std::function<void(effects_cc13::EffectSplashRingFields* ring)> remove_water_ring;  // 00B9E4A0
};
void destruct_effect_splash_instance_00872d60(const effects_cc13::EffectSplashInstanceStorage& instance,
                                              const EffectSplashDestructHost& host);

// ===========================================================================
// Point effect handle and child retirement, under the effect manager lock
// (00866440()+4: EnterCriticalSection, depth counter at CS+18h ++ / --).
// ===========================================================================

// The three handle bytes 00866B70 touches; 00865FE0 (effects_cc13) treats a
// set +0Ah as "not finished".
struct EffectHandleFlags {
    bool active_08{false};
    bool finish_when_done_09{false};
    bool stop_requested_0a{false};
};
// 00866B70, __fastcall(handle), RET 0: +9 = 0, and +0Ah = 1 when +8 is set.
void effect_handle_request_stop_00866b70(EffectHandleFlags& h) noexcept;

// 008679D0, __thiscall(point, child ref by value), RET 4; sole caller
// 00857160 (call site 008573ED). Searches the entry array +0Ch (count +10h)
// for the child. Found: a child whose byte +0Ch is set gets it cleared and
// its vtable+30h (stop) called; the slot is appended to the pending array
// +18h (00867320, which retains), the slot's reference is released and the
// slot nulled IN PLACE (the array keeps its count). Either way the by-value
// argument reference is released after the lock is left. Returns found.
struct PointEffectChildSlot {
    std::uintptr_t child{0};
    bool active_0c{false};
};
struct PointEffectRetireHost {
    std::function<void(std::uintptr_t child)> stop_child;   // child vtable+30h
    std::function<void(std::uintptr_t child)> retain;       // 00867320 publish/retain
    std::function<void(std::uintptr_t child)> release;      // InterlockedDecrement +4h, vtable[0] at zero
};
bool point_effect_retire_child_008679d0(std::vector<PointEffectChildSlot>& entries_0c,
                                        std::vector<std::uintptr_t>& pending_18, std::uintptr_t child,
                                        const PointEffectRetireHost& host);

// ===========================================================================
// Sound effect component (vtable 00D0DA18; samples at +20h, count +24h)
// ===========================================================================

// One sample's view through 00A818D0 / 00A81860 (its options record).
struct EffectSoundSampleView {
    bool resource_word08{false};   // 00A818D0 (BSP_SoundSample_HasResourceWord08_Provisional)
    bool flag_15{false};           // options +15h
    bool use_max_distance_20{false};  // options +20h
    float distance_0c{0.0f};       // options +0Ch, times 128.0
    float max_distance_10{0.0f};   // options +10h
};
// 00869E10, slot 0Ch, __thiscall(component), RET 0: true when any sample's
// options +15h byte is set (walk +20h[+24h], 00A81860).
bool effect_sound_any_sample_flag15_00869e10(const std::vector<EffectSoundSampleView>& samples) noexcept;
// 0086FA80, slot 10h: MOV EAX,2.
constexpr int effect_sound_category_0086fa80() noexcept { return 2; }
// 0086A740, slot 1Ch, __thiscall(component, const pos*, node), RET 8. Byte
// +2Ch set -> true. Otherwise distance = 0042B2F0(pos - node world position)
// (node refreshed first; each difference a float), then per sample in order:
// 00A818D0 true -> true; options +15h -> true; limit = options +20h ?
// options +10h : f(options +0Ch * 128.0 (00CEF258)); limit > distance
// (FCOMPI/JA, so a NaN does not admit) -> true. No sample admits -> false.
bool effect_sound_admit_at_position_0086a740(bool always_2c, float distance,
                                             const std::vector<EffectSoundSampleView>& samples) noexcept;

// ===========================================================================
// Weather: the rain descriptor singleton 00F875C0 (docs/CC13_SCENE_WEATHER.md)
// ===========================================================================

// The descriptor fields 00865900 writes. +4h is the RainModelParams block
// 008023B0 builds from (src/engine_seg49_64_78_cc13.cpp).
struct RainDescriptorFields {
    float drop_count_0c{0.0f};
    float min_length_18{0.0f};
    float length_span_1c{0.0f};
    Vec3 speed_20{};
    std::uint32_t colour_34{0};
    std::uintptr_t emitter_38{0};
};
// The scene weather block 00865900 reads (pointer from 004DF890).
struct WeatherPresetBlock {
    bool is_rain_60{false};
    std::int32_t drop_count_64{0};
    Vec3 speed_68{};
    std::uint32_t colour_74{0};
    float min_length_78{0.0f};
    float max_length_7c{0.0f};
    float fog_colour_80[4]{};
    float directional_colours_90[4][4]{};
    float fog_d0{0.0f};
    float fog_d4{0.0f};
    float fog_d8{0.0f};
    float fog_dc{0.0f};
    float fog_e0{0.0f};
    std::uint8_t cloud_wind_25c{0};
};

// 00865900, __thiscall(descriptor, const preset*), RET 4, sole caller
// BSP_Game_ConstructWorld. An emitter at +38h is released (00B6DFA0) and
// cleared. With IsRain (+60h): +20h..+28h = speed +68h..+70h, +0Ch =
// CVTSI2SS +64h, +18h = +78h, +1Ch = float(+7Ch - +78h) (FSUB), +34h =
// +74h, then +38h = 008023B0(descriptor+4h). With the game object and its
// camera ([[00E188A8]+19FCh]): system fog setters 00B84D00(+D0h),
// 00B84D40(+DCh), 00B84D10(+D4h), 00B84D20(+D8h), 00B84D30(+E0h) in that
// order, 00B84C40(+80h colour), 00B84FA0(+90h + 10h*i, i) for i = 0..3, the
// camera clear colour 00B6FE50(camera, 004FB850(+80h) ARGB), then the cloud
// manager's wind switch 00BBDDF0 / 00BBCE60 with byte +25Ch. No RNG here;
// the emitter's per-frame draws are 00801400's (stream 1, see 00865AB0).
struct WorldAtmospherePresetHost {
    std::function<void(std::uintptr_t node)> release_node;            // 00B6DFA0
    std::function<std::uintptr_t()> build_rain_emitter;              // 008023B0(descriptor+4h)
    std::function<bool()> camera_present;                            // game and [+19FCh] non-null
    std::function<void(int setter, float value)> set_fog_scalar;     // setter = 0x68/0x78/0x6c/0x70/0x74
    std::function<void(const float (&rgba)[4])> set_fog_colour;      // 00B84C40
    std::function<void(int index, const float (&rgba)[4])> set_directional_colour;  // 00B84FA0
    std::function<void(const float (&rgba)[4])> set_camera_clear_colour;  // 004FB850 + 00B6FE50
    std::function<void(std::uint8_t enabled)> set_cloud_wind;        // 00BBDDF0 + 00BBCE60
};
void world_atmosphere_apply_preset_00865900(RainDescriptorFields& d, const WeatherPresetBlock& preset,
                                            const WorldAtmospherePresetHost& host);

// 00865AB0, __thiscall on the descriptor, RET 0, every frame from
// BSP_Game_OnMove (004E5309). With an emitter at +38h: the camera node
// [[00E188A8]+19FCh] is refreshed (00B6DB70 when +5Ch bit 1 is clear) and
// 00801400(emitter; f([[00E188A8]+21F0h]), &camera position (+120h..+128h),
// camera+110h) runs; rain_emitter_step_00801400 (src/engine_seg49_64_78_cc13.cpp)
// models that step. Its draw is on STREAM 1 (00801439 MOV ECX,1;
// docs/CC13_RNG_STREAM_CENSUS.md), not stream 0. Returns whether it stepped.
struct RainDescriptorUpdateHost {
    std::function<Vec3()> camera_position;                           // refreshed node +120h..+128h
    std::function<float()> frame_dt_21f0;                            // [[00E188A8]+21F0h]
    std::function<void(float dt, const Vec3& camera)> emitter_step;  // 00801400
};
bool rain_descriptor_update_00865ab0(const RainDescriptorFields& d, const RainDescriptorUpdateHost& host);

// ===========================================================================
// Rotation helpers used by the torpedo homing (00856BB0) and the periscope
// debris step (00852970). Gameplay x87 at 24-bit precision: every product
// and sum below rounds like a float, in the listing's order.
// ===========================================================================

// 0085C3F0, __fastcall(out ECX, const v EDX, const axis* [ESP+4], float
// angle [ESP+8]), RET 8; 7 callers (00730160 BSP_Gun_Fire among them).
// Rodrigues about a unit axis a:
//   d = (a.y*v.y + a.x*v.x) + a.z*v.z;  p = d*a;  r = v - p
//   rr = (r.y*r.y + r.x*r.x) + r.z*r.z
//   if (double)1e-4f (00D7A268) > rr: out = v   (FCOMPI/JBE; a NaN rotates)
//   else c = a x r (004F9B30), s = f(FSIN angle), k = f(FCOS angle),
//        out = ((r*k) + p) + c*s, each component stored as a float.
// FSIN/FCOS are taken here as double sin/cos rounded to float, which can
// differ from the x87 result in the last bit.
Vec3 rotate_vector_about_axis_0085c3f0(const Vec3& v, const Vec3& axis, float angle) noexcept;

// 0085E880, __fastcall(out ECX, const in EDX, const axis* [ESP+4], float
// angle [ESP+8]), RET 8: out row 2 (+20h) = 0085C3F0(in row 2), then out row
// 1 (+10h) = 0085C3F0(in row 1), then 0085DC80(out)
// (orthonormalize_pose_matrix_0085dc80, src/plane_pose_commit.cpp). Out rows
// 0 and 3 are not written before 0085DC80. `out` may alias `in`.
void rotate_basis_about_axis_0085e880(float out[16], const float in[16], const Vec3& axis,
                                      float angle);

// 0085E8C0, __fastcall(out ECX, const in EDX, const pivot* [ESP+4], const
// axis* [ESP+8], float angle [ESP+0Ch]), RET 0Ch; sole caller 0042D610. The
// 0085E880 steps (rows 2 and 1, then 0085DC80), then t = in row 3 - pivot
// (each a float), t' = 0085C3F0(t), out row 3 = pivot + t' (pivot first in
// each FADD). Out may alias in: row 3 is read after 0085DC80, which never
// writes it.
void rotate_about_axis_through_point_0085e8c0(float out[16], const float in[16], const Vec3& pivot,
                                              const Vec3& axis, float angle);

// 0085D3D0 / 0085D770 / 0085D920 / 0085DAD0, __fastcall(float m[16] ECX),
// plain RET, in place on rows 0..2 (elements 0..2, 4..6, 8..10; the fourth
// column and row 3 are never addressed). One shape in all four:
//   normalise A; d = (A.y*B.y + A.x*B.x) + A.z*B.z; B -= A*d; normalise B;
//   C = cross(P, Q) (004F9B30); normalise C.
// normalise: L = 00419440 length (f(sqrt((f(x*x) + f(y*y)) + f(z*z))));
// inv = L > 0 ? f(1 / L) : 0 (FLDZ / FCOMI / JBE, FLD1 / FDIVRP DE F1; a NaN
// length gives 0); each component times inv, stored as a float.
//   0085D3D0  A row 0, B row 1, C row 2 = row0 x row1   (1 caller, 0085E4D0)
//   0085D770  A row 2, B row 0, C row 1 = row2 x row0   (1 caller, 007DF4F0)
//   0085D920  A row 1, B row 0, C row 2 = row0 x row1   (no caller)
//   0085DAD0  A row 1, B row 2, C row 0 = row1 x row2   (20 callers)
void orthonormalize_from_row_x_0085d3d0(float m[16]) noexcept;
void orthonormalize_from_row_z_0085d770(float m[16]) noexcept;
void orthonormalize_from_row_y_then_x_0085d920(float m[16]) noexcept;
void orthonormalize_from_row_y_then_z_0085dad0(float m[16]) noexcept;

// ---------------------------------------------------------------------------
// Pass cc13-gpr53e: the "Tracer" effect component (the trace line).
//
// The "Tracer" row of 00870400 allocates A0h bytes and constructs them with
// 0086B8B0 (construct_effect_tracer_0086b8b0, src/gameplay_effect_components.cpp):
// a trace-line definition at +0 (vtable 00D0D5B0) and the effect component at
// +80h (vtable 00D0D590). The component pointer (allocation + 80h) is what the
// loader passes to the reader slot (vtable +14h), so the reader's ECX is the
// component and its [ESI-6Ch]..[ESI-4] stores land at definition +14h..+7Ch.
// Each field below carries its definition offset; the 0086B8B0 default is
// what a key the reader skips leaves in place.

// The {data, count, capacity} pointer array at definition +4Ch/+50h/+54h.
// Append (inline in 00858700, and as 00858660): when count == capacity,
// n = 2 * capacity + 2 and, when n > capacity, capacity = n and the items
// move to a new operator new(n * 4) block (00BF55BE; a size overflow asks for
// FFFFFFFFh), the old block freed (00BF6989). The item is then stored at
// [count] and count += 1. Only the capacity is modelled, not the blocks.
struct TraceLinePointerArray {
    std::vector<std::uintptr_t> items;   // +4Ch data, +50h count
    std::uint32_t capacity_54{0};
};
void trace_line_pointer_array_append_00858660(TraceLinePointerArray& a, std::uintptr_t item);

struct TraceLineDefinition {
    bool anim_on_off_04{false};                  // 0086B8B0: 0
    bool anim_loop_05{true};                     //           1
    std::int32_t anim_start_frame_08{0};
    std::int32_t anim_end_frame_0c{0};
    float anim_play_speed_10{1.0f};              // 00D7A24C
    float half_width_14{0.1f};                   // 00D7A2F0; Width * 0.5
    float width_speed_18{0.0f};
    float width_speed_end_1c{0.0f};
    std::int32_t max_segment_num_20{3};
    float min_segment_length_24{0.0f};           // not written by 0086B8B0
    float segment_life_time_28{3.40282347e38f};  // 00D7A248 FLT_MAX
    float uv_tile_length_2c{10.0f};              // 00CE38B8
    std::uint32_t outer_color_30{0};             // not written by 0086B8B0
    std::uintptr_t vertex_format_40{0};
    std::uintptr_t vertex_binding_44{0};
    TraceLinePointerArray textures_4c;
    bool show_bullet_58{false};                  // the fields from +58h on are
    std::uintptr_t bullet_tail_texture_5c{0};    // not written by 0086B8B0
    std::uintptr_t bullet_head_texture_60{0};
    std::uint32_t tail_color_64{0};
    std::uint32_t head_color_68{0};
    float tail_length_6c{0.0f};
    float head_radius_70{0.0f};
    float bullet_life_time_74{0.0f};
    float bullet_fade_begin_78{0.0f};
    float bullet_fade_span_7c{0.0f};             // life - fade begin
};

// The Lua accessors 00858700 uses, each on row[key] (00B67800), the temporary
// released (00B67700) before the next key.
struct TraceLineLuaRow {
    std::function<float(const char* key, float fallback)> number_or;              // 00B66330
    std::function<std::int32_t(const char* key, std::int32_t fallback)> integer_or;  // 00B66380
    std::function<bool(const char* key, bool fallback)> boolean_or;               // 00B662F0
    std::function<std::uint32_t(const char* key)> packed_bgra;                     // 00B67F10
    std::function<bool(const char* key)> is_string;                                // 00B660A0
    std::function<bool(const char* key)> is_nil;                                   // 00B65FB0
    // 00B662B0 into a NativeString (0041E870); a null buffer reads as the
    // zero-filled 00F87570, so the host returns "" for it.
    std::function<std::string(const char* key)> string;
};
struct TraceLineTextureHost {
    std::function<std::int32_t(const std::string& name)> count_frames;           // 00AF3A20
    std::function<std::string(const std::string& name, std::int32_t i)> frame_name;  // 00AF3B50
    std::function<std::uintptr_t(const std::string& name)> find_atlas_item;       // 00AEFB20 on [00F8C26C]
    std::function<std::uintptr_t(const char* name)> vertex_format;                // [00F8D394] vtable+38h
    // [00F8D394] vtable+40h(&block): a stack block with +0 = format and
    // +10h = 1; +4h..+0Ch are never written (stack garbage in the image).
    std::function<std::uintptr_t(std::uintptr_t format)> vertex_binding;
};

// 00858700 BSP_EffectTraceLine_ReadLua, __thiscall(component ECX, LuaObject*
// row), RET 4. 00868BF0 (read_effect_component_base_00868bf0) on the
// component first; the host runs it, it is not repeated here. Then, in
// listing order (default in brackets):
//   Width [0] * 0.5 (FMUL qword 00D7A280; exact) -> +14h, WidthSpeed [0],
//   WidthSpeedEnd [0], MaxSegmentNum [10] (integer), MinSegmentLength [1],
//   SegmentLifeTime [1], OuterColor (00B67F10, no default), AnimOnOff [0],
//   AnimLoop [0] (booleans), AnimStartFrame [0], AnimEndFrame [the start just
//   read] (integers), AnimPlaySpeed [0].
//   TextureName: when a string, n = 00AF3A20(name). n == 0: the atlas item of
//   the name, appended only when non-null. n > 0: for i in 0..n-1 the frame
//   name 00AF3B50(name, i) and its item; the FIRST null item ends the loop
//   (00858BC7 -> 00858C81 -> 00858CA8) and the later frames are not tried.
//   n < 0 appends nothing. When TextureName is not a string, the item of
//   "cloud_snake.dds" is appended WITHOUT a null test (00858CD7).
//   ShowBullet [0]; BulletTailTexture (string -> its item, else
//   "tr_bullet.dds"); BulletHeadTexture (else "tr_glow_add.dds"); neither
//   item is null-tested. TailLength [16 (00CE6454)], HeadRadius [0.8
//   (00CE74F8)], BulletLifeTime [2 (00CE3958)], BulletFadeBegin [1.6
//   (00D06BB4)], then +7Ch = +74h - +78h (D9 46 F4 / D8 66 F8, FSUB m32).
//   TailColor: nil -> FFFFFFFFh, else 00B67F10. HeadColor: nil -> bytes
//   32 80 FF 87 (87FF8032h), else 00B67F10. UVTileLength [1] -> +2Ch.
//   Last, +40h = vertex format "pf43uf43ccuf44uf41.mvfm" (a 17h-reserved
//   NativeString, 0041DD40 + _memcpy) and +44h = the vtable+40h binding of it.
// The texture array is appended to, never cleared.
void trace_line_read_lua_00858700(TraceLineDefinition& def, const TraceLineLuaRow& row,
                                  const TraceLineTextureHost& host);

// The trace-line effect instance (20h bytes, vtables 00D0C98C / 00D0C988):
// +4 references, +0Ch byte, +10h the effect owner (EffectInstance), +14h
// the component, +18h kind 3, +1Ch the NativeTraceline node (1B8h, vtable
// 00D0C928).
struct TraceLineInstance {
    std::uint32_t references_04{1};
    bool flag_0c{true};
    std::uintptr_t owner_10{0};
    std::uintptr_t component_14{0};
    std::int32_t kind_18{3};
    std::uintptr_t trace_line_1c{0};
};

// 00859240 BSP_EffectTraceLine_CreateInstance, __fastcall(definition ECX,
// owner EDX), plain RET, returns the instance in EAX. 00859370 is the
// component slot: EDX = [ESP+4], ECX -= 80h, CALL, RET 4.
//   instance = operator new(20h) (00BF681B); when it succeeds: +0/+8 vptrs
//   (00CEB130 / 00D0C8C0, then 00D0C98C / 00D0C988), +4 = 1, +0Ch = 1,
//   +10h = owner, +14h = definition ? definition + 80h : 0, +18h = 3, +1Ch = 0.
//   node = 00AF3430(1B8h) (model pool); when non-null, 00858260(node,
//   NativeString "TraceLine") and vtable 00D0C928; the name temporary is
//   destroyed only when it was built. instance+1Ch = node, then node
//   vtable+5Ch(definition, [[00E188A8]+19ECh]).
// Neither allocation is null-tested on the later path: a failed instance
// allocation writes +1Ch of a null pointer (0085930C), and a failed node
// allocation calls vtable+5Ch through a null pointer (00859346).
struct TraceLineCreateHost {
    std::function<bool()> allocate_instance;                                    // 00BF681B(20h)
    std::function<std::uintptr_t()> construct_node;                              // 00AF3430 + 00858260
    std::function<void(std::uintptr_t node, std::uintptr_t definition)> attach;  // vtable+5Ch(def, root)
};
struct TraceLineCreateResult {
    bool instance_allocated{false};
    TraceLineInstance instance;
    // The image faults here: a null instance (0085930C) or a null node
    // (00859346). The model stops without calling attach.
    bool faults{false};
};
TraceLineCreateResult trace_line_create_instance_00859240(std::uintptr_t definition, std::uintptr_t owner,
                                                          const TraceLineCreateHost& host);
// 00859370: the component-slot adjustor of 00859240.
TraceLineCreateResult trace_line_create_instance_slot_00859370(std::uintptr_t component,
                                                               std::uintptr_t owner,
                                                               const TraceLineCreateHost& host);

// 008585D0 BSP_TraceLineEffectInstance_Update, vtable 00D0C98C +28h,
// __thiscall(instance, float dt, unused), RET 8. The owner's node
// ([+10h]+110h) is refreshed (00B6DB70) when its +5Ch bit 1 is clear; its
// world position +120h..+128h goes to 00AF2630(node +1Ch, &position, dt),
// then 00AF1EB0(node +1Ch, dt); when the owner's stopping byte +9 is set,
// node+194h = 1. The node pointer is not null-tested.
struct TraceLineUpdateHost {
    std::function<bool()> owner_node_world_current;       // [[+10h]+110h]+5Ch & 2
    std::function<void()> refresh_owner_node_world;       // 00B6DB70
    std::function<Vec3()> owner_node_world_position;      // +120h..+128h
    std::function<void(const Vec3& point, float dt)> set_point;  // 00AF2630
    std::function<void(float dt)> update_submodel;        // 00AF1EB0
    std::function<bool()> owner_stopping;                 // [+10h]+9
    std::function<void()> mark_node_finishing;            // node+194h = 1
};
void trace_line_instance_update_008585d0(float dt, const TraceLineUpdateHost& host);

// 00858530 BSP_TraceLineEffectInstance_Destruct, __thiscall, plain RET.
// vptrs 00D0C98C / 00D0C988; with a node at +1Ch: when node+A4h equals the
// world root [[00E188A8]+19ECh], 00B6D890(node, root); then (re-tested)
// 00B6DFA0(node) and +1Ch = 0. vptrs 00D0C88C / 00D0C888 and 00BD30F0 last.
struct TraceLineDestructHost {
    std::function<std::uintptr_t(std::uintptr_t node)> node_parent_a4;
    std::function<std::uintptr_t()> world_root;                     // [[00E188A8]+19ECh]
    std::function<void(std::uintptr_t node, std::uintptr_t root)> propagate_root_registration;  // 00B6D890
    std::function<void(std::uintptr_t node)> unlink_and_release;   // 00B6DFA0
    std::function<void()> destroy_base;                            // 00BD30F0
};
void trace_line_instance_destruct_00858530(TraceLineInstance& inst, const TraceLineDestructHost& host);

// The other non-trivial slots of vtable 00D0C98C (slot 0 00BD30E0, slot 1
// the deleting destructor 008586E0, slots 3/4 and 6..9 the shared leaves
// 00858180..008581E0 of section 7.2):
// 008580A0 +08h IsComplete: node+190h == 0. The node is not null-tested.
// 00858500 +14h: node+19Ch = the float argument (RET 4); not null-tested.
// 008583A0 +2Ch: the destructor's first step, with a node: when node+A4h is
//   the world root [[00E188A8]+19ECh], 00B6D890(node, root).
// 00858520 +30h Stop: with a node, node+1A8h = 1.
struct TraceLineNodeView {
    std::function<std::uint32_t()> count_190;
    std::function<void(float)> set_19c;
    std::function<void()> set_stop_1a8;
};
bool trace_line_instance_is_complete_008580a0(const TraceLineNodeView& node);
void trace_line_instance_set_19c_00858500(const TraceLineNodeView& node, float value);
void trace_line_instance_detach_from_root_008583a0(const TraceLineInstance& inst,
                                                   const TraceLineDestructHost& host);
void trace_line_instance_stop_00858520(const TraceLineInstance& inst, const TraceLineNodeView& node);

// ---------------------------------------------------------------------------
// Pass cc13-gpr53e: the "WaterTracer" effect component (C4h bytes, vtable
// 00D0D5D4, constructed by 0086BCC0, which zeroes +48h..+54h, +64h..+70h and
// +8Ch only). Here ECX is the component itself: the offsets are positive.

// The six arrays 0086D180 fills from the WaterSpray table and passes to
// 00BAE2E0 (w4_water_spray_resource_construct_00bae2e0). The local is
// destroyed afterwards by 0086C120 (in reverse field order, +3Ch first).
struct WaterSprayDefinition {
    std::vector<float> point_00;   // Vec2 pairs: (row[1][1], row[1][2])
    std::vector<float> offset_0c;  // Vec2 pairs: (row[2][1], row[2][2])
    std::vector<float> size_18;    // row[6]
    std::vector<float> a_24;       // row[3]
    std::vector<float> b_30;       // row[4]
    std::vector<std::int32_t> count_3c;  // row[5], 00B66290
};
// One WaterSpray row. The image reads, per row: value[1] then value[1][2],
// value[1] again then value[1][1]; number([1][1]) then number([1][2]); the
// same for [2]; then [3], [4] (00B66270), [5] (00B66290) and [6] (00B66270).
struct WaterSprayRow {
    float p1{0.0f}, p2{0.0f}, q1{0.0f}, q2{0.0f}, a3{0.0f}, b4{0.0f};
    std::int32_t count5{0};
    float size6{0.0f};
};

struct WaterTracerComponent {
    std::uint32_t show_forward_20{0};     // 00B66250, MOVZX
    std::uint32_t show_backwards_24{0};
    std::uint32_t fit_to_water_28{0};
    float lifetime_2c{0.0f};
    std::uint32_t divisions_34{0};
    float line_distance_38{0.0f};
    std::uint32_t lod_3c{0};
    std::int32_t full_lines_minus_one_40{0};  // written only when LOD and FullLines != 0
    float texture_rate_44{0.0f};
    std::string texture_file_48;          // NativeString
    std::string texture_file2_50;         // NativeString, set only from a string
    float alpha_scale_58{0.0f};
    float width_scale_5c{0.0f};
    float displacement_scale_60{0.0f};    // HeightScale first, then DisplacementScale
    std::uintptr_t alpha_curve_64{0};
    std::uintptr_t width_curve_68{0};
    std::uintptr_t height_x_curve_6c{0};
    std::uintptr_t height_y_curve_70{0};
    float fade_in_74{0.0f};
    float start_time_78{0.0f};
    float start_speed_7c{0.0f};
    float stop_time_80{0.0f};
    float stop_speed_84{0.0f};
    float width_wave_88{0.0f};
    std::uintptr_t width_speed_curve_8c{0};
    std::int32_t heading_lines_90{0};
    float heading_length_94{0.0f};
    std::uintptr_t texture_98{0};
    std::uintptr_t texture2_9c{0};
    bool local_space_a0{false};
    std::uintptr_t tracer_resource_a4{0};
    float width_offset_a8{0.0f};
    float width_scaler_ac{0.0f};
    float bone_length_b0{0.0f};
    std::int32_t bone_count_b4{0};
    float length_b8{0.0f};
    bool has_water_spray_bc{false};
    std::uintptr_t spray_resource_c0{0};
};

struct WaterTracerLuaRow {
    std::function<bool(const char* key)> boolean;                                  // 00B66250
    std::function<std::int32_t(const char* key)> integer;                          // 00B66290
    std::function<float(const char* key)> number;                                  // 00B66270
    std::function<float(const char* key, float fallback)> number_or;               // 00B66330
    std::function<std::int32_t(const char* key, std::int32_t fallback)> integer_or;  // 00B66380
    std::function<bool(const char* key, bool fallback)> boolean_or;                // 00B662F0
    std::function<bool(const char* key)> is_string;                                // 00B660A0
    std::function<bool(const char* key)> is_table;                                 // 00B661B0
    // 00B662B0; a null pointer is a zero-length string (0041DD40(len, 0) and
    // a _memcpy of the length).
    std::function<std::string(const char* key)> string;
    // 00869E40(row[key], &local, scale) with the local zeroed first
    // (load_tracer_parameter_curve_00869e40); returns the local.
    std::function<std::uintptr_t(const char* key, float scale)> load_curve;
    // The WaterSpray table in 00B67080 / 00B67190 order, ended by 00B66420.
    std::function<std::vector<WaterSprayRow>(const char* key)> rows;
};
struct WaterTracerHost {
    // [00F8D394] vtable+64h(&NativeString, 0).
    std::function<std::uintptr_t(const std::string& name)> load_texture;
    // InterlockedDecrement ([00CE2220]) of curve+4, vtable[0] when it reaches 0.
    std::function<void(std::uintptr_t curve)> release_curve;
    // operator new(50h) + 00BADC70(resource, block) (by value, RET 44h); 0 when
    // the allocation fails.
    std::function<std::uintptr_t(const std::array<std::uint32_t, 17>& block)> create_tracer_resource;
    // operator new(2Ch) + 00BAE2E0(resource, &definition); 0 when it fails.
    std::function<std::uintptr_t(const WaterSprayDefinition& definition)> create_spray_resource;
};

// 0086D180 BSP_EffectWaterTracer_ReadLua, __thiscall(component ECX,
// LuaObject* row), RET 4, vtable 00D0D5D4 +14h. 00868BF0 first (the host
// runs it). Then +98h = +9Ch = +C0h = 0 (no release), and in listing order:
//   ShowForward, ShowBackwards, FitToWater (00B66250); Lifetime [1];
//   TimeScale [1] into a local (the curve scale below; not stored);
//   Divisions [4] (integer); LineDistance [4 (00CE3D34)]; LOD [0]
//   (boolean, stored as a DWORD); with LOD: n = FullLines (00B66290),
//   n == 0 clears LOD, else +40h = n - 1. TextureRate [0.066 (00D0D918)].
//   TextureFile -> +48h and +98h = its texture; TextureFile2, only when a
//   string, -> +50h and +9Ch. AlphaScale [1], AlphaScaler [1] (local),
//   WidthScale [6 (00CE6630)], WidthScaler [1] -> +ACh, WidthOffset [0] ->
//   +A8h, BoneLength [6] -> +B0h, BoneCount = _ftol2(number [25 (00CE89CC)])
//   -> +B4h, Length [8 (00CE3918)] -> +B8h, HeightScale [1] -> +60h.
//   Curves, each: local = 0; when the GATE key is a table, local = 00869E40
//   (KEY, TimeScale); then the old field is released and the field = local
//   (null when the gate is not a table):
//     +64h Alpha / Alpha, +68h Width / Width, +6Ch HeightX / HeightX,
//     +70h gate **HeightX**, key HeightY (0086D9C8 pushes "HeightX" for the
//     table test, 0086DA0C "HeightY" for the load).
//   FadeIn, StartTime, StartSpeed, StopTime, StopSpeed (00B66270, no
//   default) -> +74h..+84h; LocalSpace [0] -> +A0h; WidthWave [0] -> +88h;
//   +8Ch = 00869E40("WidthSpeed", scale 1.0) with no table test (that Lua
//   temporary lives to the end of the function); HeadingLines [0] -> +90h;
//   HeadingLength [0] -> +94h.
//   Clamps: Divisions = 2 when below 2 UNSIGNED (a negative count stays);
//   Lifetime and LineDistance = 0.1f when below (double)0.1 (00D7A3A0;
//   FCOMI / JBE, so a NaN stays).
//   SegmentCount [50 (00CEB4D4)], VertTessPower [1], HorizDivisions [5
//   (00CE3850)], HorizTessPower [0]: the two counts by FISTP qword under a
//   truncating control word (low DWORD kept).
//   FitToWater == 1: BoneCount = 40, t = f(Length / 39.0) (00D0D7F0),
//   BoneLength = (3.0 (00D7A2B0) > t) ? 3.0f (00CE3854) : t.
//   DisplacementScale [0.1 (00D7A2F0)] -> +60h (overwrites HeightScale).
//   +A4h = the tracer resource of the 44h block (see below).
//   +BCh = WaterSpray is a table; when set, the rows fill a
//   WaterSprayDefinition and +C0h = its spray resource.
// The block, by DWORD: [0] BoneCount, [1] FitToWater != 0, [2] LocalSpace,
// [3..5] &+68h, &+6Ch, &+70h (the curve holders; host addresses), [6]
// SegmentCount, [7] VertTessPower, [8] HorizDivisions, [9] HorizTessPower,
// [10] WidthScaler, [11] WidthOffset, [12] WidthScale, [13]
// DisplacementScale, [14] Length, [15] TextureRate, [16] AlphaScaler.
struct WaterTracerCurveHolders {
    std::uint32_t width_68{0};
    std::uint32_t height_x_6c{0};
    std::uint32_t height_y_70{0};
};
void water_tracer_read_lua_0086d180(WaterTracerComponent& c, const WaterTracerLuaRow& row,
                                    const WaterTracerHost& host,
                                    const WaterTracerCurveHolders& holder_addresses);

// ---------------------------------------------------------------------------
// Pass cc13-gpr53e: the light instance constructor (segment end address).

// The light instance: the prefix, the point light +1Ch, the countdown +20h
// that 008738B0 (update_effect_light_instance_008738b0) runs down.
struct EffectLightInstanceStorage {
    NativeRegisteredEffectPrefixStorage prefix;   // type_18 = 0
    std::uintptr_t light_1c{0};
    float remaining_20{0.0f};
};
// The light definition rows read by 00869FD0 (read_effect_light_00869fd0):
// +20h Duration, +24h Radius, +28h DiffuseColor, +38h SpecularColor.
struct EffectLightDefinitionFields {
    float duration_20{0.0f};
    float radius_24{0.0f};
    std::array<float, 4> diffuse_28{};
    std::array<std::uint32_t, 4> specular_38{};   // copied as raw DWORDs
};
// 008740E0 BSP_EffectLightInstance_Construct, __thiscall(instance,
// definition, point), RET 8. The prefix, +18h = 0, tables 00D0DBF0 /
// 00D0DBEC; light = 00B7BD30 (ECX 1FCh, the point-light slot); when
// non-null, 00B7C710(light, "dynamic_light_" (00D0DE04) + the NativeString
// at [point+84h]+1Ch) (0041E870 then 004261A0 BSP_NativeString_Concat; both
// temporaries destroyed afterwards). +1Ch = light (null on failure). Then,
// with NO null test: 004B62E0(light, &def+28h) (base diffuse), light+1C4h..
// +1D0h = def+38h..+44h, light+1F8h = def+24h when it is above 0.01f
// (00D7A238; COMISS / JA, so a NaN radius gives 0.01f), and instance+20h =
// def+20h (FLD / FSTP).
struct EffectLightConstructHost {
    std::function<std::uintptr_t()> allocate_light;                                   // 00B7BD30(1FCh)
    std::function<std::string()> point_name;                                          // [[point+84h]+1Ch]
    std::function<void(std::uintptr_t light, const std::string& name)> construct_light;  // 00B7C710
    std::function<void(std::uintptr_t light, const std::array<float, 4>& diffuse)> set_base_diffuse;  // 004B62E0
    std::function<void(std::uintptr_t light, const std::array<std::uint32_t, 4>& specular)> set_specular_1c4;
    std::function<void(std::uintptr_t light, float radius)> set_radius_1f8;
};
struct EffectLightConstructResult {
    // A null light faults at 00874212 (004B62E0 with ECX 0) in the image;
    // the model stops there.
    bool faults{false};
};
EffectLightConstructResult construct_effect_light_instance_008740e0(
    EffectLightInstanceStorage& instance, void* definition, const EffectLightDefinitionFields& def,
    PointEffectInstanceStorage* point, const EffectLightConstructHost& host);

// The per-kind CreateInstance slots, each __thiscall(definition ECX, point
// [ESP+4]), RET 4: operator new(bytes) (00BF681B); when it succeeds, the
// kind's constructor(instance, definition, point) and the instance in EAX;
// otherwise 0. No other work.
struct EffectInstanceKind {
    std::uint32_t create_instance;
    std::uint32_t bytes;
    std::uint32_t constructor;
};
constexpr std::array<EffectInstanceKind, 6> kEffectInstanceKinds{{
    {0x00868d00u, 0x20u, 0x00873ae0u},  // sound
    {0x00869300u, 0x20u, 0x00873010u},  // flare
    {0x00869370u, 0x24u, 0x00873eb0u},  // thunderstorm
    {0x008693e0u, 0x24u, 0x00873090u},  // waterdrops
    {0x00869450u, 0x24u, 0x008740e0u},  // light
    {0x008695b0u, 0x20u, 0x008739a0u},  // splash
}};
struct EffectCreateInstanceHost {
    std::function<std::uintptr_t(std::uint32_t bytes)> allocate;                     // 00BF681B
    std::function<void(std::uintptr_t instance, std::uint32_t constructor)> construct;
};
std::uintptr_t effect_create_instance_00869300_family(const EffectInstanceKind& kind,
                                                      const EffectCreateInstanceHost& host);

// ---------------------------------------------------------------------------
// Pass cc13-gpr53e: torpedo and water-mine wrappers.

// 00855B00 BSP_TorpedoProjectile_CommitStepPose, __thiscall, plain RET:
// 006E1010 (the bomb step commit) first; then, with a node at +BCh, the
// node's matrix +74h is copied to +C0h by 004134F0 (sixteen x87 FLD/FSTP
// pairs; a float copy).
struct TorpedoCommitStepPoseHost {
    std::function<void()> base_commit;                 // 006E1010
    std::function<bool()> has_node_bc;
    std::function<void()> copy_node_matrix_to_c0;      // 004134F0(+C0h, node+74h)
};
void torpedo_commit_step_pose_00855b00(const TorpedoCommitStepPoseHost& host);

// 008559D0 (torpedo) and 0085EAC0 (water mine), __thiscall(float dt), RET 4:
// the entity's own vtable+194h(dt), nothing else.
void forward_slot_198_to_194_008559d0(float dt, const std::function<void(float)>& slot_194);

// 00855BB0 (BSP_TorpedoProjectile_HideNode) and 00855B90 (no Ghidra
// function: it follows the RET 4 of 00855B30), __thiscall(unused), RET 4:
// with a model at +464h, 00B6DA70(model+0Ch node, factor, 0) with factor 0.0
// (FLDZ) or 1.0 (FLD1).
void torpedo_set_node_visibility_00855bb0(bool has_model_464, bool visible,
                                          const std::function<void(float factor)>& set_visibility);

// 0085F1A0 BSP_WaterMineShot_PlaceStepPose, __thiscall(shot, float dt),
// RET 4: 006E1060(dt) (the bomb placement); then, with +84h set, the object
// [[+84h]+160h]+0Ch gets vtable+38h(shot - 370h), the owning entity.
struct WaterMinePlaceStepPoseHost {
    std::function<void(float dt)> base_place;          // 006E1060
    std::function<bool()> has_link_84;
    std::function<void()> notify_owner;                // [[[+84h]+160h]+0Ch] vtable+38h(this - 370h)
};
void water_mine_place_step_pose_0085f1a0(float dt, const WaterMinePlaceStepPoseHost& host);

// 00855F50 BSP_TorpedoProjectile_SendCreateMessage, __thiscall(entity, peer),
// RET 4: the 00855CC0 message ([class+0Ch], entity) on the stack, then
// 00779FC0 BSP_Entity_SendMessageToPeer(peer, &message), then the message's
// base destructor (vtable 00CF9324, 007668E0).
void torpedo_send_create_message_00855f50(
    const TorpedoState& state, bool has_target, std::uint16_t target_net_id_174, std::uint32_t peer,
    const std::function<void(std::uint32_t peer, const TorpedoCreateMessage&)>& send);

// The unit motion-controller factories (docs/UNIT_CONTROLLER_UPDATE.md),
// __thiscall(unit), plain RET. The ledger's "operator new(0x390)" is wrong
// for both: the PUSH immediates are 39Ch (00852228) and 394h (00857C37).
// 00857C20 (variant C): controller = new(394h) + 00939F20(controller, unit),
//   or 0; unit+1018h = controller.
// 00852210 (variant B, submarine): controller = new(39Ch) + 00939E50, or 0;
//   unit+1018h = controller, unit+1210h = 0; then, with NO null test,
//   body = 00C31DC0([controller+2Ch]); while body: when 00C32450(body) has
//   bit 1 set, unit+1210h = body and stop; else body = body+208h.
struct MotionControllerCreateHost {
    std::function<std::uintptr_t(std::uint32_t bytes)> allocate;         // 00BF681B
    std::function<void(std::uintptr_t controller)> construct;           // 00939E50 / 00939F20(unit)
    std::function<std::uintptr_t(std::uintptr_t controller)> first_body;  // 00C31DC0([+2Ch])
    std::function<std::uint32_t(std::uintptr_t body)> body_flags;         // 00C32450
    std::function<std::uintptr_t(std::uintptr_t body)> next_body;         // body+208h
};
struct MotionControllerCreateResult {
    std::uintptr_t controller_1018{0};
    std::uintptr_t flagged_body_1210{0};
    bool faults{false};   // variant B with a failed allocation (00852263)
};
MotionControllerCreateResult unit_create_motion_controller_c_00857c20(const MotionControllerCreateHost& host);
MotionControllerCreateResult unit_create_motion_controller_b_00852210(const MotionControllerCreateHost& host);

// ---------------------------------------------------------------------------
// Pass cc13-gpr53f: component leaves, the water tracer destructor and the
// thunderstorm component.

// Component category slots (MOV EAX, imm / XOR EAX, EAX; RET). Each value
// equals the type +18h that the kind's instance constructor writes (flare
// 5, the rest 0; section 7.7).
constexpr std::int32_t effect_water_tracer_category_0086bd10() noexcept { return 4; }
constexpr bool effect_water_tracer_restart_0086bd20() noexcept { return true; }  // MOV AL, 1
constexpr std::int32_t effect_flare_category_0086bdb0() noexcept { return 5; }
constexpr std::int32_t effect_waterdrops_category_0086bdf0() noexcept { return 0; }
constexpr std::int32_t effect_light_category_0086be30() noexcept { return 0; }
constexpr std::int32_t effect_splash_category_0086be70() noexcept { return 0; }
constexpr std::int32_t effect_thunderstorm_category_0086fc20() noexcept { return 0; }

// 0086BF10 BSP_EffectWaterTracer_Destruct, __thiscall, plain RET: vtable
// 00D0D5D4, then each non-null reference is released (InterlockedDecrement
// [00CE2220] of +4, vtable[0] at zero) and nulled, in this order: +98h,
// +9Ch, +A4h, +C0h, +8Ch, +70h, +6Ch, +68h, +64h. Then the NativeStrings
// +50h and +48h, and the component base 0086B7E0 last.
struct WaterTracerDestructHost {
    std::function<void(std::uintptr_t reference)> release;
    std::function<void()> destroy_base;   // 0086B7E0
};
void water_tracer_destruct_0086bf10(WaterTracerComponent& c, const WaterTracerDestructHost& host);

// The "ThunderStorm" component (58h, vtable 00D0DA38, constructed by
// 0086FEC0). The members from +24h form one sub-object that 0086FB40
// destroys.
struct ThunderStormTexture {   // a 14h record (0086E670 / 0086CDB0)
    float origo_x{0.0f};       // Origo[1]
    float origo_y{0.0f};       // Origo[2]
    std::string name;          // Name; null or empty -> no buffer
};
struct ThunderStormComponent {
    float min_life_time_24{0.0f};
    float max_life_time_28{0.0f};
    float min_scale_2c{0.0f};
    float max_scale_30{0.0f};
    float constant_34{0.0f};          // 2.5f (00CF87C8), not a Lua key
    std::int32_t max_spawn_count_38{0};
    float spawn_delay_3c{0.0f};       // CVTSI2SS of the integer
    std::string glow_texture_40;      // NativeString
    std::vector<ThunderStormTexture> textures_48;
    bool has_3d_lightning_54{false};
};
// The accessors on row["ParamsTable"]; every key below is read from that
// table, not from the row.
struct ThunderStormParams {
    std::function<std::string(const char* key)> string;                            // 00B662B0
    std::function<bool(const char* key, bool fallback)> boolean_or;                // 00B662F0
    std::function<float(const char* key, float fallback)> number_or;               // 00B66330
    std::function<std::int32_t(const char* key, std::int32_t fallback)> integer_or;  // 00B66380
    // Textures in 00B67080 / 00B67190 order: Name (00B662B0) and
    // number_or(Origo[1], 0.5), number_or(Origo[2], 0.5). The image fetches
    // Origo[2] first, then Origo[1], and converts [1] first.
    std::function<std::vector<ThunderStormTexture>(const char* key)> textures;
};
// 0086F3B0 BSP_EffectThunderStorm_ReadLua, __thiscall(component, LuaObject*
// row), RET 4. 00868BF0 first (the host runs it). params = row.ParamsTable;
// GlowTexture -> +40h (0041DD40 + _memcpy; null -> ""); Has3Dlightning
// [true] -> +54h. For each Min/Max pair the Max key is fetched first, but
// the Min value is converted first: MinLifeTime [0.5 (00CE3800)] -> +24h,
// MaxLifeTime [4 (00CE3D34)] -> +28h, MinScale [0.5] -> +2Ch, MaxScale [4] ->
// +30h. MaxSpawnCount [15] -> +38h; SpawnDelay [1] -> +3Ch as a float;
// +34h = 2.5f. Textures: the array +48h is first resized to 0 (0086E6F0),
// then each row is appended. The name copy writes its terminator before the
// pool allocation's null test (0086F71D), so a failed allocation faults.
void thunderstorm_read_lua_0086f3b0(ThunderStormComponent& c, const ThunderStormParams& params);

// 0086FB40, __thiscall(component + 24h): the texture array (0086E6F0(0),
// then the block freed by 00BF6989), then the GlowTexture string.
// 0086FBC0 BSP_EffectThunderStorm_Destruct: vtable 00D0DA38, 0086FB40 on
// +24h, then the base 0086B7E0.
void thunderstorm_destroy_members_0086fb40(ThunderStormComponent& c);
void thunderstorm_destruct_0086fbc0(ThunderStormComponent& c, const std::function<void()>& destroy_base);

// ---------------------------------------------------------------------------
// Pass cc13-ver2: the bomb release velocity the host substitutes.
//
// 006E0A70 BSP_BombProjectile_ReleaseFromOwner, __thiscall(bomb, owner,
// float* flight[16]), RET 8. It is the bomb's vtable+190h and is called by
// 006E1F00 on a copy of the bomb's world matrix (bomb+CCh). When owner
// vtable+5Ch(0Fh) is true AND owner vtable+38h (speed) > 1.3888890f
// (00CF8AAC; FCOMI / JBE): w = owner vtable+34h (world velocity), w.y =
// f(w.y - 3.0f) ([00E08E54], read-only); velocity +318h..+320h = w, matrix row
// 2 (+20h..+28h) = w, then 0085DC80 (row 2 normalised, rows 1 and 0 rebuilt
// with row 1 as the hint). Otherwise velocity = the owner's world velocity
// unchanged and the matrix is left alone.
// 006E1F00 BSP_BombRack_DropCone then copies that matrix to the bomb (+74h)
// and, unless D == (1, 1, 1) exactly (three UCOMISS against 1.0f), turns the
// velocity: L = f(sqrt(f((vy*vy + vx*vx) + vz*vz))) when that square is
// above (double)1e-10 (00CE3820), else 0, and per component
//   v_i = f(f(f(f(Dx*r0_i)*L) + f(f(Dy*r1_i)*L)) + f(f(r2_i*Dz)*L)).
// D comes from 006E4D50: (t cos a, -t sin a, 1) with a, t drawn on stream 1.
// So with no cone (t = 0) the bomb keeps w exactly. The rows are the
// velocity-aligned frame from 0085DC80, NOT the plane's own pose.
struct BombReleaseOwner {
    bool is_plane_5c_0f{false};
    float speed_38{0.0f};
    Vec3 world_velocity_34;
};
struct BombReleaseResult {
    Vec3 velocity;     // +318h..+320h after 006E1F00
    float matrix[16];  // the release frame written to bomb+74h
};
BombReleaseResult bomb_release_velocity_006e0a70_006e1f00(const float rack_matrix[16],
                                                         const BombReleaseOwner& owner,
                                                         const Vec3& cone_d);

}  // namespace gameplay_seg53_cc13
}  // namespace bsp
