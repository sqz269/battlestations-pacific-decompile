#pragma once

// Segment 47 (007B2DD0-007D1DC0) reference models: plane damage and crash.
// Packet cc13-gpr47; docs/CC13_GPR_47.md, docs/PLANE_DEATH_MODES.md,
// docs/CC13_GP_007A_007F.md.
//
// These are semantic interfaces read from the disk bytes (Capstone disasm-raw)
// and the Ghidra pseudocode, not native layouts or ABI-compatible
// replacements. Field names are hypotheses; the "_NNN" suffix is the native
// byte offset in the plane unit (or the message) the image reads. Every engine
// service the image calls is a host callback, so nothing here invents a global
// or stubs a call. Gameplay runs at 24-bit x87 precision
// (docs/CC13_FPU_PRECISION.md); every value here is stored to a float by the
// image, so the models compute in float. Build-tested only; not wired, not
// game-run.
//
// ADOPTION NOTE (cc9 terms; the full table is docs/CC13_GPR_47.md section 2):
// - Replaces:
//   * named_effect_007bbfa0: plane_death_timer_c10_007bbfa0
//     (src/plane_death_modes.cpp:39) and the mode-to-message step of the host's
//     death block (src/game_hosts_units.cpp:17261). It adds the "forced
//     explosion" predicate (any effect on a plane outside flight states 4..7,
//     or slower than 1 km/h while the controller gate is clear, explodes at
//     once), the splash and engine-fire arms, and the session messages.
//   * state_message_007d0b80: plane_death_flags_007d0b80
//     (src/plane_death_modes.cpp:51). The host's version differs in three
//     places: the delayed explosion takes the power-lost arm only when the
//     controller gate (unit+72Ch)->vt[38h] is set, the engine-fire arm and the
//     model-less spinning arm skip the C3Ah tail, and every message copies its
//     +20h into unit+C10h (splash 256.0, engine fire 99.0).
//   * out_of_action_countdown_007c6c30: the empty override at
//     src/game_hosts_units.cpp:17364 and the retreat-only SUBSTITUTION at
//     src/game_hosts_units.cpp:31102.
//   * plane_hit_record_007bbcf0: the state writes of
//     notify_plane_hit_007bbcf0 (src/game_hosts_gunnery.cpp:2815).
//   * plane_on_destroyed_007bcaa0: squadron_leave_on_death_007bcaa0
//     (src/game_hosts_units.cpp:6730), which models only the squadron removal.
//   * failure_query_007bc460, exit_zone_007ca790, debris_spec_007caad0: none.
// - Inputs: the unit fields named in each struct; the planepartclasses.lua
//   globals [00E18710]/[00E1870C] (ExplosionExplosionDelay) and
//   [00E18718]/[00E18714] (the dead-engine-fire delay pair); GameTuning +348h,
//   +34Ch, +350h; the session mode [00E188A8]+1FE4h.
// - RNG, through the host's Rng callback, in image order:
//   * 007BC3C7: U([00E18710], [00E1870C]), STREAM 0, once per
//     "explosion_delayed" that is not forced and not rammed (007BBFA0).
//   * 007BC1F2: U(0, 1), STREAM 1, once per "spinning" while unit+C36h is clear.
//   * 007BC0FC: NextU32, STREAM 1, once per "enginefire" with no Lua index.
//   * 007BCABB: U([00E18718], [00E18714]), STREAM 0, once per plane destroy
//     (007BCAA0).
//   * 007CAC5D/007CAC8B/007CACB9: three U(-0.001, 0.001), then 007CAD58/007CAD78/
//     007CAD98: three U(-1, 1), all STREAM 1,
//     per debris part that 007CAAD0 finds; 007D0B80's spinning arm calls it
//     once per "balszarny/jobbszarny letorik" node of the model.
// - OFF-switch prediction: OFF changes nothing. ON, the stream-0 draws are the
//   ones the host's death block already makes (key (unit, 0) in
//   src/game_hosts_gunnery.cpp:1702) plus one per destroyed plane at 007BCAA0,
//   which the host does not draw: every stream-0 consumer after the first plane
//   death shifts by one draw per death. The forced-explosion predicate turns a
//   landed, parked or deck plane's death into an immediate Kill(1). The
//   countdown model starts destroying stray planes that have no squadron.

#include <cstdint>
#include <functional>
#include <optional>
#include <vector>

namespace bsp {
namespace gameplay_seg47_cc13 {

struct Vec3 {
    float x{0.0f};
    float y{0.0f};
    float z{0.0f};
};

// The image's random service. stream is the ECX value: 0 for the gameplay
// stream, 1 for the second thread stream. uniform is 00BD2F10
// BSP_Random_UniformFloatRange(a, b) with the arguments in the image's push
// order; next_u32 is 00BD2FC0 BSP_RandomThreads_NextU32.
struct Rng {
    std::function<float(int stream, float a, float b)> uniform;
    std::function<std::uint32_t(int stream)> next_u32;
};

// The planepartclasses.lua globals 004A9BD0 loads (docs/PLANE_DEATH_MODES.md).
struct PlaneFailureGlobals {
    float explosion_delay_a_e18710{0.0f};  // ExplosionExplosionDelay
    float explosion_delay_b_e1870c{0.0f};
    float engine_fire_delay_a_e18718{0.0f};  // 007BCAA0's pair
    float engine_fire_delay_b_e18714{0.0f};
};

// ---------------------------------------------------------------------------
// 1. Session messages (base 0075B430; kind byte at +10h).
// ---------------------------------------------------------------------------

// Each class's IsKindOf slot +0Ch answers its own kind, 62h and 49h
// (0075C400, 0075C4D0, 0075A1A0), so the handler's tests are exclusive.
enum class FailureMessageKind : int {
    Explosion = 0x63,     // 007BC3EC inline, vtable 00D02F6C, +20h the timer
    Splash = 0x64,        // 0075C450, vtable 00D02F80, +20h 256.0f (00CF8B3C)
    EngineFire = 0x65,    // 007BA170, vtable 00D02CF4, +20h 99.0f, +24h index
    EnginesLost = 0x66,   // 007BA1C0, vtable 00D02F94, +20h the timer
    Spinning = 0x67,      // 007BA200, vtable 00D02D08, +20h timer, +24h side
    DeadMeatRot = 0xC8,   // 007BA8D0, multiplayer only (session mode != 0)
};

struct FailureMessage {
    FailureMessageKind kind{FailureMessageKind::Explosion};
    float value_20{0.0f};
    std::uint32_t arg_24{0};  // engine index, or the side byte (0 left, 1 right)
};

inline constexpr float kSplashMessageTimer = 256.0f;      // 00CF8B3C
inline constexpr float kEngineFireMessageTimer = 99.0f;   // 00D059A0
inline constexpr float kFailureTimer = 1000.0f;           // 00CE3804
inline constexpr float kImmediateTimer = -1.0f;           // 00D7A260
inline constexpr float kForcedExplosionSpeed = 0.2777778f;  // 00CF8918, 1 km/h
inline constexpr float kSpinSideSplit = 0.5f;             // 00CE3800

// ---------------------------------------------------------------------------
// 2. 007BBFA0 BSP_Plane_StartNamedEffect, plane vtable +194h,
//    __thiscall(unit, NativeString* name, LuaObject* args), RET 8, AL = 1.
// ---------------------------------------------------------------------------

// The names, in the order the image compares them. The first two use
// __stricmp on the string's pointer (a null pointer matches neither), the rest
// 00425850 BSP_NativeString_EqualsCStringInsensitive.
enum class NamedEffect : int {
    Explosion,         // "explosion" 00D05A18
    ExplosionDelayed,  // "explosion_delayed" 00CE54C4
    Splash,            // "splash" 00D05A10
    EngineFire,        // "enginefire" 00D05A04
    PowerLost,         // "powerlost" 00D059F8
    Spinning,          // "spinning" 00D059EC
    LeftSpinning,      // "leftspinning" 00D059DC
    RightSpinning,     // "rightspinning" 00D059CC
    Other,
};

NamedEffect named_effect_from_name(const char* name) noexcept;

struct NamedEffectInputs {
    NamedEffect effect{NamedEffect::Other};
    int flight_state_900{0};
    // (unit+72Ch)->vtable[38h](), called only when the state is 4..7.
    std::function<bool()> controller_gate_72c;
    float cached_speed_b1c{0.0f};   // unit->vtable[38h] = 007B8E60
    bool rammed_dfc{false};         // unit+DFCh
    bool spinning_c36{false};       // unit+C36h
    float timer_c10{0.0f};          // unit+C10h on entry
    std::int32_t engine_count_7c{0};  // [unit+538h]+7Ch
    // The Lua call's argument 2 (00B677E0, 00B66290) when args is non-null.
    // -1 (or no args) draws the engine.
    std::optional<std::int32_t> lua_engine_index;
    int session_mode_1fe4{0};       // [00E188A8]+1FE4h
};

struct NamedEffectResult {
    bool forced_explosion{false};   // BL at 007BC031
    float timer_c10{0.0f};          // unit+C10h on exit
    std::vector<FailureMessage> routed;  // 0077C2A0(msg, 7, 0), in order
};

NamedEffectResult named_effect_007bbfa0(const NamedEffectInputs& in,
                                        const PlaneFailureGlobals& g,
                                        const Rng& rng);

// 007BC460, plane vtable +198h, __thiscall(unit, name, args, char* out):
// "enginefire" writes 0 and answers false; "powerlost" writes unit+C39h and
// answers true; "spinning" writes unit+C36h and answers true; any other name
// leaves *out alone and answers false.
struct FailureQueryResult {
    bool answered{false};
    std::optional<bool> out;
};
FailureQueryResult failure_query_007bc460(NamedEffect effect, bool powerlost_c39,
                                          bool spinning_c36) noexcept;

// ---------------------------------------------------------------------------
// 3. 007D0B80 BSP_Plane_HandleStateMessageKinds, plane vtable +208h,
//    __thiscall(unit, msg), RET 4.
// ---------------------------------------------------------------------------

struct PlaneFailureState {
    float timer_c10{0.0f};
    bool dead_5d{false};
    bool removed_5e{false};
    bool spinning_c36{false};
    bool right_wing_c37{false};   // set with "jobbszarny letorik" (right wing)
    bool left_wing_c38{false};    // set with "balszarny letorik" (left wing)
    bool powerlost_c39{false};
    bool death_raised_c3a{false};
    bool flag_969{false};
    bool has_marker_c4c{false};   // unit+C4Ch, 00498F80's result
    float bank_c68{0.0f};
    float pitch_input_9e8{0.0f};
};

struct StateMessageEnv {
    int session_mode_1fe4{0};
    // EDI at 007D0BE6: [unit+360h] ? [[unit+360h]+160h] : 0, the model.
    bool has_model_360{true};
    std::function<bool()> controller_gate_72c;  // called only for kind 63h
    float pos_y_100{0.0f};          // unit+100h after the pose refresh
    bool marker_service_on{false};  // [[00E188A8]+19CCh]+4ACh
};

struct StateMessageHost {
    std::function<void()> stop_all_engines;            // 007DE600(unit+C98h)
    std::function<void(std::uint32_t)> destroy_engine_at;  // 007EB0B0
    std::function<void()> bind_crash_effect;           // 007BC4E0
    std::function<bool()> create_marker;               // 00498F80 non-null
    std::function<void()> update_marker;               // 0049A760
    std::function<void()> spawn_explosion_effect;      // 00868420, class+214h
    std::function<void()> kill;                        // 00926D90(unit, 1)
    // The wing-break block: hide the "rwing"/"lwing" parts, a point effect at
    // their centre, then 007CAAD0 per "jobbszarny/balszarny letorik" node, then
    // 007D0820 (hide the engine, rotor, aileron and cockpit holders).
    std::function<void(bool right)> break_wing;
    std::function<void()> raise_dead;                  // unit->vtable[70h](1)
};

void state_message_007d0b80(PlaneFailureState& s, const FailureMessage& msg,
                            const StateMessageEnv& env, const StateMessageHost& host);

// ---------------------------------------------------------------------------
// 4. 007BBCF0 BSP_PlaneEntity_ApplyHitRecord, vtable +ECh, and 007BCAA0
//    BSP_Plane_OnDestroyed, vtable +7Ch (00D19DA4).
// ---------------------------------------------------------------------------

struct PlaneHitInputs {
    bool shooter_is_kind5{false};   // hit+4h non-null and its vtable[5Ch](5)
    bool player_squadron{false};    // [00E188D8] == unit+9D4h (both may be 0)
    bool death_raised_c3a{false};
    bool dead_5d{false};
    bool has_squadron_9d4{false};
    bool has_pilot_bot_df4{false};
    std::int32_t hit_segment_34{-1};
    std::int32_t hit_side_38_4{-1};  // [[hit+38h]+4h]
};

struct PlaneHitHost {
    std::function<void()> hud_damage_direction;   // 004704E0 then 0064B170
    std::function<void()> squadron_on_hit;        // [unit+9D4h]->vtable[ECh](hit)
    std::function<void()> pilot_bot_on_hit;       // 00999AA0
    std::function<void()> unit_apply_hit;         // 008777D0
};

struct PlaneHitState {
    bool hit_latch_520{false};
    bool rammed_dfc{false};
    std::int32_t spin_side_800{-1};
};

void plane_hit_record_007bbcf0(PlaneHitState& s, const PlaneHitInputs& in,
                               const PlaneHitHost& host);

struct PlaneDestroyHost {
    std::function<void()> unit_on_destroyed;   // 00959450
    std::function<void()> squadron_remove;     // 007F3970(unit, 0)
    std::function<void()> retire_pilot_bot;    // [unit+DF4h]->vtable[10h]()
    std::function<void()> retire_sub_6dc;      // [unit+6DCh]->vtable[10h]()
};

struct PlaneDestroyState {
    float engine_fire_timer_c14{0.0f};
    std::int32_t kind_1b0{0};
    std::int32_t last_kind_748{0};
    bool has_squadron_9d4{false};
    bool has_pilot_bot_df4{false};
    bool has_sub_6dc{false};
};

void plane_on_destroyed_007bcaa0(PlaneDestroyState& s, const PlaneFailureGlobals& g,
                                 const Rng& rng, const PlaneDestroyHost& host);

// ---------------------------------------------------------------------------
// 5. 007C6C30 BSP_Plane_OutOfActionCountdown, vtable +1D8h,
//    __thiscall(unit, float step), RET 4; and 007CA790 BSP_Plane_OnExitZone.
// ---------------------------------------------------------------------------

struct ExitCountdownTuning {
    float margin_348{0.0f};     // GameTuning +348h, 0059C9B0's margin
    float period_34c{0.0f};     // GameTuning +34Ch, the reset value of +6F0h
    float warn_period_350{0.0f};  // GameTuning +350h, the squadron reseed
};

struct ExitCountdownState {
    float timer_6f0{0.0f};
    std::int32_t state_6ec{0};      // 1 inside, 2 leaving, 3 gone
    bool has_squadron_9d4{false};
    bool flag_9d8{false};           // unit+9D8h non-zero
    float squadron_warn_36c{0.0f};  // [unit+9D4h]+36Ch
};

struct ExitCountdownHost {
    std::function<bool()> outside_map;            // 0071C4F0(unit+FCh)
    std::function<bool()> squadron_leaving;       // 007EE2E0(squadron, 1)
    std::function<bool(float margin)> beyond_margin;  // 0059C9B0(unit+FCh, margin)
    std::function<bool()> local_player_role;      // 00927F30(unit, 0)
    std::function<void()> warn_exit_zone;         // 00982120([00F8A0C4], unit, 0)
    // 007EE410(squadron, unit, 00489C40(unit+FCh)); ECX is [unit+9D4h] even
    // when it is null.
    std::function<void()> send_exit_zone;
};

void out_of_action_countdown_007c6c30(ExitCountdownState& s, bool dead_5d, float step,
                                      const ExitCountdownTuning& t,
                                      const ExitCountdownHost& host);

// 007CA790, the handler of the message 007EE410 sends: under the mission
// entity lock it sets the unit's Lua self.ExitZoneParty = party, then calls
// 007F31A0 BSP_PlaneSquadron_OnPlaneLeftMap(unit, party) when unit+9D4h is set.
struct ExitZoneHost {
    std::function<void(std::int32_t party)> set_lua_exit_zone_party;
    std::function<void(std::int32_t party)> squadron_left_map;
};
void exit_zone_007ca790(bool has_squadron_9d4, std::int32_t party, const ExitZoneHost& host);

// ---------------------------------------------------------------------------
// 6. 007CAAD0 BSP_Plane_SpawnDebrisParts, __thiscall(unit, SceneNode* node).
// ---------------------------------------------------------------------------

// The spec 00447510 BSP_GameDynamicsList_Add receives (docs/GAME_DYNAMICS_LIST.md
// reads +0h, +4h, +8h, +24h..+2Ch, +30h, +54h), for the node's record in the
// visual class list [model+50h..+54h] (pairs: record, node).
struct DebrisSpec {
    std::int32_t field_00{0};
    float lifetime_04{15.0f};        // 00CE5380
    Vec3 velocity_08;                // unit->vtable[34h], the plane's velocity
    Vec3 spin_14;                    // three U(-1, 1), stream 1
    Vec3 record_24;                  // record+14h..+1Ch, meaning unread
    float floor_30{-50.0f};          // 00CECA0C
    float limit_54{3.4028235e38f};   // 00D7A248
    Vec3 position;                   // node world translation + jitter
};

// Returns nothing (no draw) when the node has no record or a null record.
std::optional<DebrisSpec> debris_spec_007caad0(bool record_found, const Vec3& node_translation,
                                               const Vec3& plane_velocity,
                                               const Vec3& record_14, const Rng& rng);

// ---------------------------------------------------------------------------
// 7. Control factors: plane vtable [1A0h] 007BBDC0 (set, RET 0Ch) and [1A4h]
//    007BBF10 (get, RET 0Ch); neither has a Ghidra function. Their callers are
//    the Lua natives SetControlFactor / GetControlFactor (0088EB50, 0088EDC0).
//    The ship pair is gameplay_seg50_cc13 (008131E0 / 00813250).
// ---------------------------------------------------------------------------

// The block unit+BCCh..BE4h. 007B97F0 BSP_Plane_RestoreFullHealth sets all
// seven to 1.0f (00D7A24C). 007CAF10 subtracts BCCh, BD0h and BD4h from its
// three latched control magnitudes (007CB226, 007CB27A, 007CB2B1; host:
// effective_control_007caf10, src/pilot_controls.cpp:130).
struct PlaneControlFactors {
    float roll_bcc{1.0f};
    float pitch_bd0{1.0f};
    float power_mean_bd4{1.0f};                   // 007B8140's mean, +8h
    float engine_power_bd8[4]{1.0f, 1.0f, 1.0f, 1.0f};  // +0Ch + index*4
};

struct ControlFactorHost {
    std::function<void(std::int32_t index)> stop_engine_at;  // 007DE5E0(unit+C98h)
    // 0077C7B0 BSP_SessionOwner_BroadcastMessage(msg, 0): kind C9h
    // (PLANE_CONTROLFACTOR, vtable 00D02E20) carrying the seven floats.
    std::function<void(const PlaneControlFactors&)> broadcast;
};

// "roll" (__stricmp on the pointer), "pitch", "power" (00425850); any other
// name answers false with no write and no message. "power" stores the engine's
// value and recomputes the mean of the first engine_count_140 entries
// ([unit+538h]+140h; float sum in index order, FIDIV by the count), then stops
// that engine when 0.0f >= value (COMISS/JB, so a NaN does not stop it). The
// image writes index unchecked; the model ignores an index outside 0..3, which
// in the image lands past BE4h.
bool set_control_factor_007bbdc0(PlaneControlFactors& f, const char* name, std::int32_t index,
                                 float value, std::int32_t engine_count_140,
                                 const ControlFactorHost& host);
// "roll" (__stricmp), "Pitch", "Power" (unchecked index, BD8h + index*4).
bool get_control_factor_007bbf10(const PlaneControlFactors& f, const char* name,
                                 std::int32_t index, float& out);

// 007B97F0 BSP_Plane_RestoreFullHealth: 00878340 (the unit's own restore),
// the seven factors = 1.0f, the three values 007CAF10 writes at BE8h..BF0h =
// 0.0f, then a tail jump to 007DE630 (the engine list's ResetAll, whose rotor
// Resets draw stream 1: gameplay_seg48_cc13).
struct RestoreFullHealthHost {
    std::function<void()> unit_restore;       // 00878340
    std::function<void()> engines_reset_all;  // 007DE630(unit+C98h)
};
void restore_full_health_007b97f0(PlaneControlFactors& f, float (&limits_be8)[3],
                                  const RestoreFullHealthHost& host);

// ---------------------------------------------------------------------------
// 8. The dead branch of 007CC2F0 BSP_Plane_FreeFlightArm (007CC375-007CC41F).
// ---------------------------------------------------------------------------

// A live plane (unit+5Dh clear), or any plane in session mode 2 with the game
// object [00E188A8] set, takes the live branch (007CC431: the landing-site
// probe, the carrier touch message C5h and the water contact 007CB7F0, all
// outside this model). A dead plane whose splash latch unit+911h is clear and
// whose y unit+100h is below 3.0f (00CE3854, COMISS/JBE: a NaN y does not
// splash) raises "splash" through vtable +194h (007BBFA0, args 0), then sets
// the latch.
enum class FreeFlightTail : int { LiveBranch, DeadSplash, Nothing };
struct FreeFlightTailInputs {
    bool dead_5d{false};
    bool has_game_e188a8{true};
    int session_mode_1fe4{0};
    bool splashed_911{false};
    float pos_y_100{0.0f};
};
FreeFlightTail free_flight_tail_007cc2f0(FreeFlightTailInputs& in) noexcept;

}  // namespace gameplay_seg47_cc13
}  // namespace bsp
