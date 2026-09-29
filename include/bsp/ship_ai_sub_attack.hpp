#pragma once

// The ship brain's `sub_attack` state and its two sub-states, "approach" and
// "fire" (packet cc9_submarine_ai_states, docs/SHIP_AI_SUB_ATTACK.md).
//
// Semantic interfaces over an injected host, not native object layouts or
// binary replacements. Names are hypotheses. Offsets in comments are relative
// to the object each routine receives in ECX:
//   brain  = ai+58h, the object 009F39C0 constructs (BrainRecordConstruct is
//            its head, 009F1160);
//   parent = brain+2124h (ai+217Ch), vtable 00D2195C, built by 009E4F90;
//   approach = parent+34h, vtable 00D218F0; fire = parent+58h, vtable 00D21920.
// Every sub-state object holds the brain at +4h and a cached torpedo device
// behind a reference node at +8h..+1Ch (009E9CE0).

#include <array>
#include <cstdint>

namespace bsp {

// ---------------------------------------------------------------------------
// 009F3D73: the attack arm of 009F3D00 (BSP_Bot_SubControllerForCommand).
// ---------------------------------------------------------------------------
// [ai+0B0Ch] is brain+0AB4h, which 009F1160 writes once at 009F11C9:
// `unit->vtable[5Ch](8) ? unit : 0` (NEG AL / SBB / AND). So the "attack
// subject" is the brain's own unit when it is a submarine and null otherwise.
// With it non-null, 00779AA0(unit) (class KamikazeDamage +510h > 0 or
// KamikazeBlastDamage +514h > 0) picks kamikaze_attack (ai+2254h), else
// sub_attack (ai+217Ch); with it null the arm takes attackmove (ai+0C70h).
enum class ShipAiAttackArm : int {
    AttackMove = 0,  // 009F3D96, ai+0C70h
    SubAttack = 1,   // 009F3D8E, ai+217Ch
    Kamikaze = 2,    // 009F3D86, ai+2254h
};
ShipAiAttackArm ship_ai_attack_arm_009f3d73(bool unit_is_submarine,
                                            bool kamikaze_class_00779aa0) noexcept;

// ---------------------------------------------------------------------------
// Constants, each read from the PE on disk.
// ---------------------------------------------------------------------------
inline constexpr float kSubAttackSwitchInterval = 1.0f;       // 00D7A24C -> parent+C8h
inline constexpr double kSubAttackEnterFireMargin = 150.0;    // 00CE3DD8 (double), 009E9DBA
inline constexpr double kSubAttackLeaveFireMargin = 400.0;    // 00CE3D90 (double), 009EA9F1
inline constexpr double kSubAttackRangeEpsilonSq = 1.0e-10;   // 00CE3820 (double), 009E4AD4
inline constexpr double kSubAttackHalf = 0.5;                 // 00D7A280 (double), 009E4B17
inline constexpr float kSubAttackDefaultTargetLength = 100.0f;// 00CE3D08, 009E9D37
inline constexpr float kSubAttackNoTubeRange = 500.0f;        // 00CE397C, 009E9D80
inline constexpr float kSubAttackReadyHorizon = 10.0f;        // 00CE38B8 -> fire+5Ch, 009EABFC
inline constexpr float kSubAttackIdleSeed = 100.0f;           // 00CE3D08 -> fire+24h, 009EAC4A
inline constexpr float kSubAttackBandTimerSeed = 10.0f;       // 00CE38B8 -> fire+3Ch, 009EAC67
inline constexpr float kSubAttackSwapLow = 30.0f;             // 00CE38C8, 009EAC3F
inline constexpr float kSubAttackSwapHigh = 50.0f;            // 00CEB4D4, 009EAC25
inline constexpr float kSubAttackOuterLow = 0.8f;             // 00CE74F8, 009E9E50
inline constexpr float kSubAttackInnerLow = 0.3f;             // 00CE69C8, 009E9E50
inline constexpr float kSubAttackInnerHigh = 0.5f;            // 00CE3800, 009E9E50
inline constexpr double kSubAttackCloseEnter = 50.0;           // 00CE3938 (double), 009E9F4C
inline constexpr float kSubAttackCloseLeave = 80.0f;          // 00CE5444, 009E9F28
inline constexpr float kSubAttackEndCheckPeriod = 5.0f;       // 00CE3850, 009E9F92
inline constexpr double kSubAttackSwapFraction = 0.6000000238418579; // 00CEFF98 (double), 009E9FB9
inline constexpr float kSubAttackEmptyEndSeconds = 3.0f;      // 00CE3854, 009E9F72
inline constexpr float kSubAttackBandPeriod = 3.0f;           // 00CE3854, 009EA2C4
inline constexpr float kSubAttackMaxLead = 12.0f;             // 00CEB4B8, 009EA38E
inline constexpr float kSubAttackRatioLow = -2.0f;            // 00CE7D7C, 009EA584
inline constexpr float kSubAttackRatioHigh = 2.0f;            // 00CE3958, 009EA5CC
inline constexpr float kSubAttackReverseHold = 3.0f;          // 00CE3854 -> fire+64h, 009EA723
inline constexpr int kSubAttackReverseDebounce = 3;           // 009EA7FE
inline constexpr float kSubAttackAimTolerance = 1.2f;         // 00CE3814, 009EA926 / 009EA96D
inline constexpr double kSubAttackBigError = 1.600000023841858; // 00CE3D48 (double), 009EA6B6
inline constexpr double kSubAttackBandSlack = 0.800000011920929;  // 00CE3D40 (double)
inline constexpr double kSubAttackBandNudge = 0.10000000149011612;// 00D7A3A0 (double)
inline constexpr float kSubAttackPeriscopeFloor = -4.0f;      // 00CF1430, 009E4D1F
inline constexpr double kSubAttackPeriscopeSlack = 2.0;       // 00D7A308 (double), 009E4D4D

// ---------------------------------------------------------------------------
// The host: one virtual per native leaf the four routines reach.
// ---------------------------------------------------------------------------
struct ShipAiSubAttackHost {
    virtual ~ShipAiSubAttackHost() = default;
    // [brain+0B20h] != 0, the attack target 009F1420 latches.
    virtual bool target_present_0b20() = 0;
    // 00414DB0 then +FCh/+100h/+104h: the refreshed world position.
    virtual std::array<float, 3> self_position() = 0;
    virtual std::array<float, 3> target_position() = 0;
    // target->vtable[5Ch](kind).
    virtual bool target_is_kind(int kind) = 0;
    // [[brain+0AACh]+0A0h] (unit+538h, the class Length) and the target's.
    virtual float self_class_length_00a0() = 0;
    virtual float target_class_length_00a0() = 0;
    // unit->vtable[50h](): the current heading.
    virtual float self_heading_vtable50() = 0;
    // target->vtable[34h](out): the world velocity.
    virtual std::array<float, 3> target_velocity_vtable34() = 0;
    // [[brain+0AACh]+500h], the class MaxSpeed (009EA576).
    virtual float self_class_max_speed_0500() = 0;
    // 009E9910 / 009E9A50: the fore (brain+0AD0h) or aft (brain+0AE0h) tubes with
    // a barrel timer <= horizon (00727D30) that are operational (00729F10).
    virtual int tubes_ready(bool fore, float horizon) = 0;
    // 009E99A0 / 009E9AE0: the least barrel timer (00729920) over the operational
    // fore or aft tubes, 3600.0f (00CFDEB0) when there is none.
    virtual float tubes_min_reload(bool fore) = 0;
    // brain+0AECh after 009E97B0: the least WaterTravelSpeed of the tubes.
    virtual float tubes_min_water_speed_0aec() = 0;
    // 009E9BC0 then 00901BA0(bot, length, 1): false when the brain has no tube
    // (the first fore tube, else the first aft tube) or the tube has no bot.
    virtual bool torpedo_bot_range_00901ba0(float target_length, float& range) = 0;
    // 00BD2F10 on stream 1.
    virtual float uniform_00bd2f10(float low, float high) = 0;
    // 008528B0 on [brain+0AB4h].
    virtual void set_depth_level_008528b0(int level) = 0;
    // unit+122Ch periscopeState, unit+1214h the periscope node, unit+1268h
    // depthLevel, unit+0A8h the local Y, unit+100h the world Y, unit+1204h bands[1].
    virtual int periscope_state_122c() = 0;
    virtual void set_periscope_state_122c(int state) = 0;
    virtual bool has_periscope_1214() = 0;
    virtual int depth_level_1268() = 0;
    virtual float local_y_00a8() = 0;
    virtual float periscope_band_1204() = 0;
    // [[brain+0AB0h]+20h] = [unit+73Ch]+20h, AllowMaxDepth.
    virtual bool allow_max_depth_0020() = 0;
    // The brain's setters on blk = brain+8h.
    virtual void set_navigation_goal_009de050(float x, float z) = 0;  // (xz, 0, 1)
    virtual void hold_heading_and_stop_009e00a0() = 0;
    virtual void set_desired_heading_009e0040(float heading) = 0;
    virtual void set_desired_throttle_009dbf90(float throttle) = 0;
    // 00419510 on a 3-vector.
    virtual std::array<float, 3> normalize_00419510(const std::array<float, 3>& v) = 0;
};

// ---------------------------------------------------------------------------
// 009DB8F0, the brain pre-pass's submarine helper (packet
// cc9_submarine_periscope_substate). 009F1B57 calls it with ECX = brain+0AC4h,
// a 4-byte holder of [brain+0AB4h]; 009F11DD..009F11F7 allocate it only when
// unit->vtable[5Ch](8) answered true at construction, so only a submarine has
// it. __thiscall(holder, float seconds), RET 4; the float is never read.
// ---------------------------------------------------------------------------
inline constexpr double kSubPeriscopePrepassBand = 2.5;      // 00CE3DE0 (double), 009DB922 / 009DB95A

struct ShipAiPeriscopePrepassHost {
    virtual ~ShipAiPeriscopePrepassHost() = default;
    // unit+1268h depthLevel, unit+100h the world Y after 00414DB0, unit+1204h bands[1].
    virtual int depth_level_1268() = 0;
    virtual float world_y_0100() = 0;
    virtual float periscope_band_1204() = 0;
    // [unit+1ACh + role*4] == 8 || 00927F10(slot): role 1 is read inline
    // (009DB96E..009DB981), role 0 through 00521E70(0) (009DB9BF). A false
    // return means the answer is unavailable to the host (no write is made).
    virtual bool role_ai_held(int role, bool& held) = 0;
    // [ai+2264h] != null && [ai+2264h]->vtable[20h](): 009DAA80 (XOR AL,AL) for
    // cruise/stop/follow/land/movetopos/moveonpath/attackmove, 009E4910 (MOV AL,1)
    // for sub_attack and 009DB310 (MOV AL,1) for kamikaze_attack.
    virtual bool active_state_wants_periscope_0020() = 0;
    virtual int periscope_state_122c() = 0;
    virtual void set_periscope_state_122c(int state) = 0;
};

enum class ShipAiPeriscopePrepassArm : int {
    lowered_out_of_band,   // 009DB9E3..009DB9EC
    raised_by_state,       // 009DB9AC
    lowered_ai_held,       // 009DB9D4
    kept_player_role1,     // 009DB981 JZ 009DB9F6
    kept_player_role0,     // 009DB9C6 JZ 009DB9F6
    kept_broken,           // any arm that met +122Ch == 2
    role_unavailable,      // host could not answer a role; nothing written
};

ShipAiPeriscopePrepassArm ship_ai_periscope_prepass_009db8f0(ShipAiPeriscopePrepassHost& host);

// ---------------------------------------------------------------------------
// The state objects' fields.
// ---------------------------------------------------------------------------
struct ShipAiSubAttackTubeCache {  // +8h..+1Ch, 009E9CE0
    bool cached{false};            // [this+1Ch] != 0
};

struct ShipAiSubAttackApproach {   // parent+34h
    ShipAiSubAttackTubeCache tube{};
    float range_20{0.0f};          // +20h, 009E4BBC; no reader was found
};

// parent+58h. 009E4F90 initialises only +4h..+1Ch; every other field is set by
// the enter routine 009EABE0 except +34h, which no routine seeds (LABELLED: 0).
struct ShipAiSubAttackFire {
    ShipAiSubAttackTubeCache tube{};
    int ready_total_20{0};         // +20h, fore + aft ready tubes
    float since_shot_24{0.0f};     // +24h, reset when +20h drops
    float fore_reload_28{0.0f};    // +28h
    float aft_reload_2c{0.0f};     // +2Ch
    bool fore_30{true};            // +30h, 1 = bow tubes, 0 = stern tubes
    float swap_timer_34{0.0f};     // +34h
    bool closing_38{true};         // +38h, 1 = run in to the inner radius
    float band_timer_3c{0.0f};     // +3Ch
    float close_40{0.0f};          // +40h, 1.0f inside 50 m until beyond 80 m
    float close_time_44{0.0f};     // +44h
    bool left_close_48{false};     // +48h
    int fore_ready_4c{0};          // +4Ch
    int aft_ready_50{0};           // +50h
    float inner_54{0.0f};          // +54h
    float outer_58{0.0f};          // +58h
    float horizon_5c{0.0f};        // +5Ch
    float swap_period_60{0.0f};    // +60h
    float reverse_64{0.0f};        // +64h
    float throttle_68{0.0f};       // +68h
    int flip_count_6c{0};          // +6Ch
};

struct ShipAiSubAttackState {      // parent, brain+2124h
    float interval_c8{kSubAttackSwitchInterval};  // +C8h
    float countdown_cc{0.0f};      // +CCh, -UniformFloat(0, 1) at 009E5061
    bool seeded{false};            // host bookkeeping: +CCh drawn
    bool fire_current{false};      // +D4h == parent+58h
    ShipAiSubAttackApproach approach{};
    ShipAiSubAttackFire fire{};
};

// What one parent tick did, for the census.
struct ShipAiSubAttackTick {
    bool ran{false};               // a target was present
    bool switched_to_fire{false};
    bool switched_to_approach{false};
    bool fire_step{false};
    bool approach_step{false};
    float range{0.0f};             // 009E4A60 on the tick's current state
    float throttle{0.0f};          // what the fire step stored, before the clamp
};

// ---------------------------------------------------------------------------
// The routines.
// ---------------------------------------------------------------------------
// 009E4A60: the edge distance to the target. See the .cpp for the dead store.
float ship_ai_sub_attack_range_009e4a60(ShipAiSubAttackHost& host);
// 009E9CE0: the torpedo range for the target, or 500.
float ship_ai_sub_attack_torpedo_range_009e9ce0(ShipAiSubAttackTubeCache& cache,
                                                ShipAiSubAttackHost& host);
// 009E9D90 with ECX = the approach object (009EAA44) and 009EA9C0 with ECX =
// the fire object (009EAA71); each uses its own tube cache.
bool ship_ai_sub_attack_approach_to_fire_009e9d90(ShipAiSubAttackTubeCache& approach_tube,
                                                 ShipAiSubAttackHost& host);
bool ship_ai_sub_attack_fire_to_approach_009ea9c0(ShipAiSubAttackTubeCache& fire_tube,
                                                 ShipAiSubAttackHost& host);
// 009E4B90, approach vtable[0Ch].
void ship_ai_sub_attack_approach_step_009e4b90(ShipAiSubAttackApproach& approach,
                                               ShipAiSubAttackHost& host);
// 009EABE0, fire vtable[4], and its tail 009E9E50; 009E9DE0.
void ship_ai_sub_attack_fire_enter_009eabe0(ShipAiSubAttackFire& fire,
                                            ShipAiSubAttackHost& host);
void ship_ai_sub_attack_fire_new_band_009e9e50(ShipAiSubAttackFire& fire,
                                               ShipAiSubAttackHost& host);
void ship_ai_sub_attack_fire_refresh_009e9de0(ShipAiSubAttackFire& fire,
                                              ShipAiSubAttackHost& host);
// 009E9EB0, fire vtable[0Ch]. Returns the throttle stored at fire+68h.
float ship_ai_sub_attack_fire_step_009e9eb0(ShipAiSubAttackFire& fire, float dt,
                                            ShipAiSubAttackHost& host,
                                            ShipAiSubAttackTick* tick = nullptr);
// The periscope helpers: 009E4EE0, 009E4C70, 009E4D00, 009E4D90.
void ship_ai_sub_attack_periscope_depth_009e4ee0(ShipAiSubAttackHost& host);
void ship_ai_sub_attack_deep_depth_009e4c70(ShipAiSubAttackHost& host);
bool ship_ai_sub_attack_at_periscope_depth_009e4d00(ShipAiSubAttackHost& host);
void ship_ai_sub_attack_raise_periscope_009e4d90(bool raise, ShipAiSubAttackHost& host);
// 009EAA90, parent vtable[0Ch], with its switch 009EAA10.
ShipAiSubAttackTick ship_ai_sub_attack_tick_009eaa90(ShipAiSubAttackState& parent, float dt,
                                                     ShipAiSubAttackHost& host);

}  // namespace bsp
