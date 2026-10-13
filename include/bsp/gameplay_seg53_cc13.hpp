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

#include <cstdint>
#include <functional>
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
enum class TorpedoWaterEntry {
    AlreadyInWater,   // shot+44h set: nothing happens
    Enters,           // 006E6450(arg): the round swims
    BreaksUpServer,   // 00926D90(1): the entity is killed
    BreaksUpClient,   // network client (world+1FE4h == 2): 006E6450(arg) instead of the kill
};
struct TorpedoWaterEntryInputs {
    bool in_water_44{false};
    Vec3 velocity_318{};          // shot+8h
    float heading_46c{10000.0f};  // shot+15Ch
    bool spawn_effect_arg{false};
    bool network_client{false};
};
struct TorpedoWaterEntryHost {
    std::function<void(float)> set_weapon_scale;       // shot vtable[20h](1.0)
    std::function<float(const Vec3&)> length;          // 0042B2F0
    // Splash for a surviving round (class+D8h): water crossing 0078D1B0
    // between the pose and shot+60h, then a point effect, effect+9 = 1.
    std::function<void()> spawn_splash_effect;
    // Break-up effect (class+24h): the pose moves to the water crossing,
    // +C8h = 1, +10Ch = 0, then a point effect at the pose, effect+9 = 1.
    std::function<void()> spawn_breakup_effect_and_snap_pose;
    // 00521370 on the world matrix's forward row (+CCh+20h): the yaw.
    std::function<float()> current_yaw;
};
struct TorpedoWaterEntryResult {
    TorpedoWaterEntry outcome{TorpedoWaterEntry::AlreadyInWater};
    float heading_46c{10000.0f};
};
TorpedoWaterEntryResult torpedo_enter_water_008568e0(const TorpedoClassFields& cls,
                                                     float terminal_fall_speed_ec,
                                                     const TorpedoWaterEntryInputs& in,
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
// existing src/bomb_torpedo_tick.cpp torpedo_steer_00856bb0 feeds the pitch
// error to both turns (its comment at 00856F51); see docs/CC13_GPR_53.md.
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
// +46Ch = 1 and a 1ACh marker object (007135C0(mine, +464h)) is stored at
// +468h and attached under world+19ECh (00B6D890). Other types go to 006E09F0.
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

}  // namespace gameplay_seg53_cc13
}  // namespace bsp
