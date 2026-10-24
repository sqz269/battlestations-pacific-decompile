#pragma once

// The kamikaze bot task (kind 0Ch, 6A0h bytes; factory 009AF720, constructor
// 009AEEF0, approach 009AED20 at task+3F8h) and the drop-kamikaze bot task
// (kind 5, 74Ch bytes; factory 009AEBE0, constructor 009ADBF0, approach 009AD720
// at task+3F8h). Window 009AB740-009AF830. Packet cc13-kamikaze1; evidence,
// listings and the host glue in docs/CC13_KAMIKAZE_TASK.md. Every name here is a
// hypothesis, not a recovered symbol. These are new C++ interfaces, not binary
// replacements; build-tested only, not wired into any host.
//
// Offsets: "approach+N" is relative to task+3F8h; "task+N" is the whole object;
// "plan+N" is the pilot plan (approach+18h == task+4h, so plan+N == task+N+4).
// Precision: the image keeps x87 intermediates in extended precision and rounds
// at each float store; this code computes intermediates in double and rounds at
// the same stores. Constants loaded as qwords stay double, as dwords stay float.
//
// RNG: every draw in this window is 00BD2F10 BSP_Random_UniformFloatRange with
// ECX = 1 (stream 1, docs/CC13_RNG_STREAM_CENSUS.md sections 10-11). They go
// through KamikazeRandom with a per-site key, so a host routes them to a keyed
// per-unit stream (H-U, `unit name + "#kz" + key`) and never to stream 0.

#include <array>
#include <cstdint>

#include "bsp/approach_target_ref.hpp"

namespace bsp {

// cc13-D1 OFF switch. These functions run on live host paths
// (game_hosts_units.cpp, packet cc9_lua52_kamikaze_wiring), so the faithful
// change waits behind a switch that cc9 pairs and flips.
//
// The main thread's x87 runs at 24-bit precision, so every step of 009AC0B0's
// and 009ADE00's distance sums and of 009AE9D0's two distances rounds to
// float. True: per-step float rounding. False: double intermediates rounded
// at the stores, as before.
inline constexpr bool kKamikazeX87FloatCc13Bound = false;

// ---------------------------------------------------------------------------
// Shared pieces
// ---------------------------------------------------------------------------

// 00BD2F10 with ECX = 1. `key` is the site's suffix (listed per function); the
// draw is U(low, high). The image draws all of them from one stream in tick
// order; a keyed host stream is a labelled substitution for that order.
struct KamikazeRandom {
    virtual ~KamikazeRandom() = default;
    virtual float uniform_00bd2f10(const char* key, float low, float high) = 0;
};

// What one state tick writes into the pilot plan (approach+18h). Each `writes_`
// flag says the field is stored this tick; fields not flagged are untouched.
// Slot layout per include/bsp/pilot_plan_slots.hpp: throttle +274h/+278h/+27Ch,
// yaw +280h/+284h/+288h, pitch +298h/+29Ch/+2A0h, air brake +2A4h/+2A8h/+2ACh.
struct KamikazePlanCommand {
    bool writes_heading_2c0 = false;   // plan+2C0h heading target, plan+2CCh = 2
    float heading_2c0 = 0.0f;
    bool writes_bank_2c4 = false;      // plan+2C4h bank target, plan+2CCh = 1
    float bank_2c4 = 0.0f;
    bool writes_turn_scale_2e8 = false;// plan+2E8h
    float turn_scale_2e8 = 1.0f;
    bool writes_pitch_target_2bc = false;  // plan+2BCh, with plan+2D0h = pitch_mode_2d0
    float pitch_target_2bc = 0.0f;
    int pitch_mode_2d0 = 1;
    bool writes_yaw_slot_284 = false;  // plan+284h desired, +288h = 1, plan+2D4h = 0
    float yaw_284 = 0.0f;
    bool writes_pitch_slot_29c = false;// plan+29Ch desired, +2A0h = 1, plan+2D0h = 0
    float pitch_29c = 0.0f;
    bool writes_throttle_slots = false;// plan+278h = 1.0, +27Ch = 1, +2A8h = 0.0, +2ACh = 1, plan+2D8h = 0
    bool writes_speed_2b4 = false;     // plan+2B4h speed demand, plan+2B0h = 0, plan+2D8h = 1
    float speed_2b4 = 0.0f;
    bool boost_2e4 = false;            // plan+2E4h = 1 (the boost/WEP request byte)
    // 009FBA50 BSP_PilotBot_CommandCruiseAltitude (cruise_altitude_command_009fba50,
    // include/bsp/plane_flight.hpp) with ECX = state+4h and these four arguments.
    bool calls_cruise_altitude_009fba50 = false;
    float cruise_base = 0.0f, cruise_low = 0.0f, cruise_high = 0.0f, cruise_scale = 0.0f;
    // 009FB800 (pitch_command_009fb800) with (desired altitude, reference).
    bool calls_pitch_command_009fb800 = false;
    float pitch_altitude = 0.0f, pitch_reference = 1.0f;
    bool clears_gun_block_40 = false;  // (approach+1Ch)+40h = 0.0
};

// The robots row the kamikaze reads through [approach+14h] (robot_config.hpp
// suffixes _08c.._0b4, robots.lua "Kamikaze*" of the PilotBot level).
struct KamikazeRobotRow {
    float section_damage_chance_80 = 0.0f;  // KamikazeSectionDamageChance
    float engine_room_weight_84 = 0.0f;     // KamikazeEngineRoomWeight
    float magazine_weight_88 = 0.0f;        // KamikazeMagazineWeight
    float fueltank_weight_8c = 0.0f;        // KamikazeFueltankWeight
    float target_precision_90 = 0.0f;       // KamikazeTargetPrecision
    float target_h_error_94 = 0.0f;         // KamikazeTargetHError
    float target_v_error_98 = 0.0f;         // KamikazeTargetVError
    float proj_time_error_9c = 0.0f;        // KamikazeTargetProjTimeError
    float maneuver_mul_lo_a0 = 1.0f;        // KamikazeManeuverPrecisionMul[1]
    float maneuver_mul_hi_a4 = 1.0f;        // KamikazeManeuverPrecisionMul[2]
    float maneuver_timer_a8 = 0.0f;         // KamikazeManeuverPrecisionTimer
};

// One Pilot/Kamikaze/<variant> block of the game tuning singleton, copied to
// approach+2Ch..+44h by REP MOVSD at 009ABA09 (include/bsp/game_tuning_singleton.hpp).
struct KamikazeProfile {
    float cruising_alt_2c = 0.0f;   // CruisingAlt
    float drop_dist_30 = 0.0f;      // DropDist
    float attack_range_34 = 0.0f;   // AttackRange
    float attack_alt_38 = 0.0f;     // AttackAlt
    float turbo_range_3c = 0.0f;    // TurboRange
    float turbo_angle_40 = 0.0f;    // TurboAngle
    float reference_speed_44 = 0.0f;// ReferenceSpeed
};
// The singleton's defaults (game_tuning_singleton.hpp): RocketLike +45Ch..+474h,
// FighterLike +478h..+490h. A host reads the live singleton.
inline constexpr KamikazeProfile kKamikazeRocketLikeDefaults{1200.0f, 2000.0f, 1400.0f, 500.0f,
                                                             1000.0f, 0.2f, 180.0f};
inline constexpr KamikazeProfile kKamikazeFighterLikeDefaults{1200.0f, 1200.0f, 1200.0f, 600.0f,
                                                              500.0f, 0.0f, 100.0f};

// 009AB93E-009AB959 (and again at 009AC1A3, 009AEFE3): the unit answers
// IsKindOf(17h) and its byte +C24h (PilotFires, strafe_task.hpp) is clear. True
// selects RocketLike (the released Ohka/Funryu carry no guns), false FighterLike.
bool kamikaze_rocket_like(bool unit_kind_17, bool pilot_fires_c24) noexcept;

// ---------------------------------------------------------------------------
// The kamikaze task (kind 0Ch)
// ---------------------------------------------------------------------------

// task+310h, whole-object offsets, named by the strings 009AED20 registers.
enum class KamikazeState : int {
    kNone = 0,
    kMoveTo = 0x4E8,     // "Kamikaze/moveto"   (approach+F0h)
    kFollow = 0x524,     // "Kamikaze/follow"   (approach+12Ch)
    kPrepare = 0x5BC,    // "Kamikaze/prepare"  (approach+1C4h), never chosen by 009AF310
    kGoTowards = 0x654,  // "Kamikaze/gotowards" vtable 00D1FB84 (approach+25Ch)
    kAim = 0x678,        // "Kamikaze/aim"      vtable 00D1FBA0 (approach+280h)
};
const char* kamikaze_state_name(KamikazeState s) noexcept;

// The approach (009AB920 base, 009AED20 derived), approach-relative offsets.
struct KamikazeApproach {
    float speed_ratio_24 = 1.0f;     // 009F9CE0: max(1, class+188h / profile.reference_speed)
    KamikazeProfile profile;         // +2Ch..+44h
    float attack_range_48 = 0.0f;    // AttackRange * ratio (009AB850)
    float climb_alt_4c = 0.0f;       // ctl+398h + U(-40, 80), floor 15
    float cruise_alt_50 = 0.0f;      // ctl+394h + U(-80, 80), floor 20
    float planar_dist_54 = 0.0f;     // 009AF0A0
    int band_58 = 0;                 // 1 / 3 / FFh from gotowards, 0 in aim; task+2E4h copies it
    bool in_range_5c = false;        // rocket-like: always 1; else planar < attack_range_48
    bool rocket_like_5d = false;
    float eta_60 = 0.0f;             // planar / max(forward speed, 10), unclamped
    float proj_time_error_64 = 0.0f; // U(-row+9C, row+9C); no reader in the window
    // +68h: the target-reference sub-object (approach_target_ref.hpp). Its sub+18h
    // (approach+80h, task+478h) is the target; sub+44h (approach+ACh) the projtime.
    ApproachTargetRefState ref;
};

struct KamikazeSeedInputs {
    bool unit_kind_17 = true;
    bool pilot_fires_c24 = false;
    float class_speed_188 = 0.0f;    // [approach+8]+188h, 009F9CE0's numerator
    KamikazeProfile rocket_like = kKamikazeRocketLikeDefaults;
    KamikazeProfile fighter_like = kKamikazeFighterLikeDefaults;
    float ctl_alt_cruise_394 = 0.0f; // (unit+9D4h)+394h
    float ctl_alt_second_398 = 0.0f; // (unit+9D4h)+398h
};

// 009AB920 (body 009AB920-009ABA25, RET 8) without 009F9CE0's other stores and
// without 009FB200 (the host constructs `ap.ref`). Ends in 009AB850.
// Draw keys: "4c", "50".
void kamikaze_approach_seed_009ab920(KamikazeApproach& ap, const KamikazeSeedInputs& in,
                                     KamikazeRandom& rng);

// 009AB850 (009AB850-009AB91F): +48h, +4Ch, +50h. Two draws, in this order:
// 009AB885 U(-40, 80) key "4c", 009AB8DF U(-80, 80) key "50".
void kamikaze_approach_redraw_009ab850(KamikazeApproach& ap, float ctl_alt_cruise_394,
                                       float ctl_alt_second_398, KamikazeRandom& rng);

// 009ABF60 (009ABF60-009AC0A2, tail-jumps to 009FA260): the aim-error redraw.
// Three draws: 009ABFB2 U(-row+94h, row+94h) key "h", 009ABFE5 U(-row+98h,
// row+98h) key "v", 009AC018 U(-row+9Ch, row+9Ch) key "64". Leaves ref.dirty_41
// set; the host's 009FA260 pick (approach_target_ref_pick_009fa260) follows.
void kamikaze_aim_error_009abf60(KamikazeApproach& ap, const KamikazeRobotRow& row,
                                 KamikazeRandom& rng);

// 009ABA30, approach vtable slot 0: the point the tasks fly at. `ref_point` is
// sub+1Ch..+24h. y is raised to 1.0 and then lifted by 00419010(8, 40, 2, 0,
// eta_60): 40 m above the target while the eta is 8 s or more, 0 at 2 s.
std::array<float, 3> kamikaze_aim_point_009aba30(const std::array<float, 3>& ref_point,
                                                 float eta_60) noexcept;

struct KamikazeUpdateInputs {
    bool has_target = false;          // sub+18h != 0 after 009FADA0
    bool target_kind_0e = false;      // target->vtable[5Ch](0Eh)
    std::array<float, 3> aim_point{}; // 009ABA30 with the PREVIOUS eta_60
    std::array<float, 3> unit_position{};  // unit+FCh..+104h
    float forward_speed_b2c = 0.0f;   // unit vtable[204h] = unit+B2Ch
};
struct KamikazeUpdateResult {
    bool clears_gun_block_34_38 = true;   // (approach+1Ch)+34h/+38h = 0.0
};
// 009AF0A0 (009AF0A0-009AF228, RET 4), the approach update the arm runs first.
// The host runs 009FADA0 on `ap.ref` with dt before calling this. No draws.
KamikazeUpdateResult kamikaze_approach_update_009af0a0(KamikazeApproach& ap,
                                                       const KamikazeUpdateInputs& in);

// The gotowards state (vtable 00D1FB84): +18h distance, +1Ch aligned,
// +1Dh turnaround latch, +20h pitch to the aim point.
struct KamikazeGoTowardsState {
    float dist_18 = 0.0f;
    bool aligned_1c = false;
    bool turnaround_1d = false;
    float pitch_20 = 0.0f;
};

// 009AC160 (enter): clears +1Ch/+1Dh/+20h, then 009AB850 and 009ABF60 on the
// approach. Draw keys "4c", "50", "h", "v", "64", in that order.
void kamikaze_gotowards_enter_009ac160(KamikazeGoTowardsState& st, KamikazeApproach& ap,
                                       const KamikazeRobotRow& row, float ctl_alt_cruise_394,
                                       float ctl_alt_second_398, KamikazeRandom& rng);

struct KamikazeGoTowardsInputs {
    bool rocket_like = false;          // recomputed at 009AC1A3 from the unit
    std::array<float, 3> aim_point{};  // 009ABA30 (approach vtable 0)
    std::array<float, 3> unit_position{};
    float heading_50 = 0.0f;           // unit vtable[50h]
    float pitch_c64 = 0.0f;            // unit+C64h
    float class_speed_188 = 0.0f;      // [approach+8]+188h
    float speed_b1c = 0.0f;            // unit vtable[38h] = unit+B1Ch
    float level_flight_speed = 0.0f;   // 007C47F0([approach+8])
};
// 009AC190 (009AC190-009AC7B9, RET 4), the gotowards tick. No draws.
KamikazePlanCommand kamikaze_gotowards_tick_009ac190(KamikazeGoTowardsState& st,
                                                     KamikazeApproach& ap,
                                                     const KamikazeGoTowardsInputs& in);

// 009AEFC0 on the gotowards state: the rule's go-to-aim test.
bool kamikaze_gotowards_ready_009aefc0(const KamikazeGoTowardsState& st,
                                       const KamikazeApproach& ap, bool rocket_like) noexcept;

// The aim state (vtable 00D1FBA0): +18h terminal lock, +19h bank side latch,
// +1Ch lock-window time, +20h precision timer, +24h precision multiplier.
struct KamikazeAimState {
    bool locked_18 = false;
    bool bank_side_19 = false;
    float window_time_1c = 0.0f;
    float precision_timer_20 = 0.0f;
    float precision_mul_24 = 1.0f;
};

// 009AC8D0 (enter). `roll_c68` is unit+C68h.
void kamikaze_aim_enter_009ac8d0(KamikazeAimState& st, float roll_c68) noexcept;

struct KamikazeAimInputs {
    bool pilot_fires_c24 = false;      // unit+C24h, 009AC98C
    std::array<float, 3> aim_body{};   // 004142E0(009ABA30 point, unit+110h): x right, y up, z forward
    float distance = 0.0f;             // 009AC0B0: 3-D distance to the 009ABA30 point
    float roll_c68 = 0.0f;
    float dt = 0.0f;
};
// 009AC910 (009AC910-009ACD89, RET 4), the terminal dive. Two draws when the
// precision timer runs out: 009ACA03 U(0.8, 1.25) key "t", 009ACA2F
// U(row+A0h, row+A4h) key "mul".
KamikazePlanCommand kamikaze_aim_tick_009ac910(KamikazeAimState& st, KamikazeApproach& ap,
                                               const KamikazeRobotRow& row,
                                               const KamikazeAimInputs& in, KamikazeRandom& rng);

// 009AEEF0 step 7: not a flight leader -> follow; leader and not rocket-like ->
// moveto; leader and rocket-like -> gotowards (the LEA ESI+654h at 009AEF69).
KamikazeState kamikaze_initial_state_009aeef0(bool flight_leader, bool rocket_like) noexcept;

struct KamikazeRuleInputs {
    KamikazeState current = KamikazeState::kNone;
    bool rocket_like_455 = false;      // task+455h (approach+5Dh)
    bool in_range_454 = false;         // task+454h (approach+5Ch)
    bool has_target_478 = false;       // task+478h (approach+80h) != 0
    bool target_marked_5d = false;     // [task+478h]+5Dh
    int attack_mode_370 = 0;           // (task+404h)+370h
    bool flight_leader = false;        // 007B8AD0
    bool gotowards_ready = false;      // 009AEFC0 on the gotowards state
};
// 009AF310 (009AF310-009AF475, RET 4). Returns the state task+310h holds after
// the rule; a change runs the old state's vtable[8] and the new one's vtable[4].
KamikazeState kamikaze_rule_009af310(const KamikazeRuleInputs& in) noexcept;

// 009AF480 (task vtable +54h), the altitude half. The base 0099B660 is a bare
// RET. The channel bytes are those of bot_tasks.hpp's PilotAltitudeChannel. The
// image then runs 009AB850 (keys "4c", "50"), 009AF0A0 with dt = 0 and
// tail-JMPs 0099B740 (abandon if stale); kamikaze_task_cruise_profile_009af480
// below is that whole body.
struct KamikazeAltitudeWrite {
    bool writes_cruise_394 = false;    // ctl+394h = task+424h (profile.cruising_alt_2c)
    float cruise_394 = 0.0f;
    bool writes_second_398 = false;    // ctl+398h = task+430h (profile.attack_alt_38)
    float second_398 = 0.0f;
    bool dirty_3ad = false;
    // An unlocked channel always clears its one-shot byte afterwards
    // (009AF4B7 +3A9h, 009AF4E0 +3AAh; 009AEA1C/009AEA52 in 009AE9D0).
    bool clears_one_shot_3a9 = false;
    bool clears_one_shot_3aa = false;
};
struct KamikazeAltitudeChannels {
    bool cruise_locked_38d = false;   float cruise_current_380 = 0.0f;  bool cruise_one_shot_3a9 = false;
    bool second_locked_38c = false;   float second_current_37c = 0.0f;  bool second_one_shot_3aa = false;
};
KamikazeAltitudeWrite kamikaze_cruise_profile_009af480(const KamikazeApproach& ap,
                                                       const KamikazeAltitudeChannels& ch) noexcept;

// 009AF560 (task vtable +40h) and 009AE140 (drop task +40h), the still-valid
// test. 2: no control block; 1: the current command is this class or 00E08F78
// and its target is the latched one (or none is latched); 0 otherwise.
struct KamikazeCommandCheck {
    bool has_control_block = true;     // task+404h != 0
    bool control_has_command = false;  // ctl->vtable[114h]() != 0
    std::uint32_t current_command = 0; // 0071xxxx BSP_WeaponDirector_CurrentCommand
    const void* command_target = nullptr;   // BSP_CommandTarget_ResolveObject
    const void* latched_target = nullptr;   // task+478h (kamikaze) / task+440h (drop)
};
int kamikaze_command_still_valid(const KamikazeCommandCheck& in,
                                 std::uint32_t own_class) noexcept;
inline constexpr std::uint32_t kKamikazeCommandClass = 0x00E08F50u;      // 009AF060
inline constexpr std::uint32_t kDropKamikazeCommandClass = 0x00E08F30u;  // 009ADDC0
inline constexpr std::uint32_t kAttackAnyCommandClass = 0x00E08F78u;     // 009AF58F, 009AE1x

// The whole task, as one value a host can keep per unit.
struct KamikazeTask {
    KamikazeState state = KamikazeState::kNone;
    KamikazeApproach ap;
    KamikazeGoTowardsState gotowards;
    KamikazeAimState aim;
    int task_2e4 = 0;                  // task+2E4h = approach+58h after each arm
};

// The host calls the arm can not compute: the target reference, the body-frame
// transform and the formation states the task shares with the other tasks.
struct KamikazeTaskHost {
    virtual ~KamikazeTaskHost() = default;
    // 009FADA0 on ap.ref with dt (approach_target_ref.hpp); returns sub+1Ch..+24h
    // and whether sub+18h (the target) is still set.
    virtual bool update_target_ref_009fada0(ApproachTargetRefState& ref, float dt,
                                            std::array<float, 3>& point) = 0;
    virtual bool target_kind_0e() = 0;          // target->vtable[5Ch](0Eh)
    virtual bool target_marked_5d() = 0;        // target+5Dh
    virtual std::array<float, 3> unit_position() = 0;
    virtual float heading_50() = 0;
    virtual float pitch_c64() = 0;
    virtual float roll_c68() = 0;
    virtual float speed_b1c() = 0;
    virtual float forward_speed_b2c() = 0;
    virtual float level_flight_speed_007c47f0() = 0;
    virtual float class_speed_188() = 0;
    virtual bool unit_kind_17() = 0;
    virtual bool pilot_fires_c24() = 0;
    virtual bool flight_leader_007b8ad0() = 0;
    virtual int attack_mode_370() = 0;
    virtual float ctl_alt_cruise_394() = 0;
    virtual float ctl_alt_second_398() = 0;
    virtual KamikazeRobotRow robot_row() = 0;
    // 004142E0 with unit+110h, the world inverse (the host's follow_to_body).
    virtual std::array<float, 3> world_to_body(const std::array<float, 3>& p) = 0;
    // The shared formation states: their enter/exit/tick (BotStateMoveTo 009BDxxx,
    // BotStateFollow 009C1xxx) are the host's existing ones.
    virtual void enter_formation_state(KamikazeState s) = 0;
    virtual void exit_formation_state(KamikazeState s) = 0;
    virtual void tick_formation_state(KamikazeState s, float dt) = 0;
    // The plan sink.
    virtual void apply_plan(const KamikazePlanCommand& c) = 0;
    virtual void clear_gun_block_34_38() = 0;
};

// 009AEEF0 after the base and the approach: seeds the approach and enters the
// initial state (the host has constructed `task.ap.ref` with 009FB200).
void kamikaze_task_construct_009aeef0(KamikazeTask& task, KamikazeTaskHost& host,
                                      const KamikazeSeedInputs& seed, KamikazeRandom& rng);

struct KamikazeArmResult {
    KamikazeState before = KamikazeState::kNone;
    KamikazeState after = KamikazeState::kNone;
    bool has_target = false;
    KamikazePlanCommand plan;          // also handed to host.apply_plan
};
// 009AF510 (task vtable +64h, 009AF510-009AF557, RET 4): 009AF0A0, 009AF310,
// the current state's vtable[0Ch](dt), task+2E4h = task+450h.
KamikazeArmResult kamikaze_task_arm_009af510(KamikazeTask& task, KamikazeTaskHost& host,
                                             KamikazeRandom& rng, float dt);

// The slot +54h cruise profiles as whole bodies (cc13-D1). Not wired: the live
// host runs its own once-at-install substitution of both.
struct KamikazeCruiseProfileHost {
    virtual ~KamikazeCruiseProfileHost() = default;
    virtual KamikazeAltitudeChannels altitude_channels() = 0;   // ctl+37Ch..+3AAh
    // ctl+394h / +398h, +3ADh = 1 and the +3A9h / +3AAh clears, as `w` says.
    virtual void apply_altitude_write(const KamikazeAltitudeWrite& w) = 0;
    virtual float ctl_alt_cruise_394() = 0;
    virtual float ctl_alt_second_398() = 0;
    virtual void abandon_if_stale_0099b740() = 0;               // the tail JMP
};
struct KamikazeCruiseProfileTaskHost : KamikazeCruiseProfileHost {
    // 009AF0A0's inputs for `dt`: the host runs 009FADA0 on ap.ref (and the
    // gun-block clear) and returns aim_point as the 009FADA0 point; the caller
    // applies 009ABA30.
    virtual KamikazeUpdateInputs approach_update_inputs_009af0a0(KamikazeApproach& ap,
                                                                 float dt) = 0;
};
// 009AF480-009AF505: 0099B660 (a bare RET), the two altitude channels, 009AB850,
// 009AF0A0(0), JMP 0099B740.
void kamikaze_task_cruise_profile_009af480(KamikazeTask& task,
                                           KamikazeCruiseProfileTaskHost& host,
                                           KamikazeRandom& rng);

// ---------------------------------------------------------------------------
// The drop-kamikaze task (kind 5): the Ohka/Funryu carriers
// ---------------------------------------------------------------------------

// task+310h, named by 009AB740's strings.
enum class DropKamikazeState : int {
    kNone = 0,
    kMoveTo = 0x4D0,     // "moveto (DropKamikaze)"   (approach+D8h)
    kFollow = 0x50C,     // "follow (DropKamikaze)"   (approach+114h)
    kAttackRun = 0x5A4,  // "DropKamikaze/attackrun"  vtable 00D1FBEC (approach+1ACh)
    kPrepare = 0x5C0,    // "DropKamikaze/prepare"    vtable 00D1FC08, a follow variant
    kDone = 0x664,       // "DropKamikaze/done"       vtable 00D1FC08, a follow variant
    kRelease = 0x708,    // "DropKamikaze/release"    vtable 00D1FC40
    kGoAway = 0x72C,     // "DropKamikaze/goAway"     vtable 00D1FC24
};
const char* drop_kamikaze_state_name(DropKamikazeState s) noexcept;

// 009ADD40: the five attack states.
bool drop_kamikaze_in_attack_set_009add40(DropKamikazeState s) noexcept;

// The approach (009ABAB0 base, 009AD720 derived). Names from 009ABBE0/009AE2D0.
struct DropKamikazeApproach {
    float speed_ratio_24 = 1.0f;     // 009F9CE0 with tuning+458h Pilot/LevelBomb/ReferenceSpeed
    int rounds_pending_2c = 0;       // task+424h, manual releases owed
    ApproachTargetRefState ref;      // +30h; sub+18h = approach+48h = task+440h is the target
    float class_18c_a4 = 0.0f;       // [approach+8]+18Ch
    float alt_a8 = 1200.0f;          // ctl+398h, refreshed by 009ADE00
    float start_dist_ac = 0.0f;      // "startDist"
    float attack_dist_b0 = 0.0f;     // "attackDist"
    float target_dist_b4 = 99999.0f; // "targetDist", planar
    float target_hdg_b8 = 0.0f;      // "targetHdg"
    int avoid_flag_bc = 0xFF;        // "currentAvoidFlag", task+4B4h -> task+2E4h
    bool inside_c0 = false;          // "insideAttackRange", task+4B8h
    bool has_ordnance_c1 = true;     // 007B93E0, task+4B9h
};

struct DropKamikazeSeedInputs {
    float class_speed_188 = 0.0f;
    float class_18c = 0.0f;
    float level_bomb_reference_speed_458 = 0.0f;  // tuning+458h
};
// 009ABAB0 (009ABAB0-009ABBB6, RET 8) without 009FB200 (host) and 009F9CE0's
// other stores. No draws.
void drop_kamikaze_approach_seed_009abab0(DropKamikazeApproach& ap,
                                          const DropKamikazeSeedInputs& in);

struct DropKamikazeUpdateInputs {
    bool has_target = false;          // approach+48h after 009FADA0
    bool has_ordnance = false;        // 007B93E0(0) on the unit
    float ctl_alt_second_398 = 0.0f;
    bool reload_enabled = false;      // 007B58D0: ctl+369h && [00E17BF2]
    std::array<float, 3> aim_point{}; // 009ABBC0: sub+1Ch..+24h as is
    std::array<float, 3> unit_position{};
    bool leader_is_self = true;       // (ctl+3D0h) == unit
    std::array<float, 3> leader_position{};  // (ctl+3D0h)+FCh..+104h
};
// 009ADE00 (009ADE00-009AE05C, RET 4). The host runs 009FADA0 on ap.ref first.
void drop_kamikaze_approach_update_009ade00(DropKamikazeApproach& ap,
                                            const DropKamikazeUpdateInputs& in);

struct DropKamikazeProfileWrite {
    KamikazeAltitudeWrite altitude;  // leader only, both channels from tuning+45Ch
};
// 009AE9D0 (task vtable +54h). One draw: 009AEA82 U(0.9, 1.2) key "ac". Then
// 009ADE00(0) and 0099EBB0, which the caller runs.
DropKamikazeProfileWrite drop_kamikaze_cruise_profile_009ae9d0(
    DropKamikazeApproach& ap, bool flight_leader, const KamikazeAltitudeChannels& ch,
    float rocket_cruising_alt_45c, float rocket_drop_dist_460, KamikazeRandom& rng);

// The prepare/done follow variant (vtable 00D1FC08): +8Ch, +98h release timer,
// +9Ch speed, +A0h released.
struct DropKamikazeFollowState {
    float follow_8c = 1.0f;
    float release_timer_98 = -1.0f;
    float speed_9c = 0.0f;
    bool released_a0 = false;
};
// 009AD180 (enter) after BSP_BotStateFollow_Enter: ctl+3E4h = 3, formation
// indices, +8Ch = 1.0, +9Ch = 007C47F0.
void drop_kamikaze_follow_enter_009ad180(DropKamikazeFollowState& st,
                                         float level_flight_speed) noexcept;
// 009AE060 on the prepare state, from vtable +24h: +98h = 3.0, +9Ch = max(|v|, level).
void drop_kamikaze_arm_release_009ae060(DropKamikazeFollowState& st, float speed_b1c,
                                        float level_flight_speed) noexcept;
// 009ADCC0: a release countdown is running.
bool drop_kamikaze_release_pending_009adcc0(const DropKamikazeFollowState& st) noexcept;

struct DropKamikazeFollowTick {
    bool runs_follow_base = false;   // 009C1FD0(dt), the host's follow tick
    bool sets_avoid_3 = false;       // approach+BCh = 3 (ctl+3D0h set)
    bool sets_avoid_1 = false;       // approach+BCh = 1
    bool releases = false;           // 007BBBA0 on the unit, approach+2Ch -= 1
    KamikazePlanCommand plan;
};
// 009AD1F0 (009AD1F0-009AD386, RET 4).
DropKamikazeFollowTick drop_kamikaze_follow_tick_009ad1f0(DropKamikazeFollowState& st,
                                                          bool has_leader_3d0, float roll_c68,
                                                          float pitch_c64, float dt) noexcept;

// 009ACFC0 (attackrun tick; enter 009ACFB0 zeroes +18h). No draws.
KamikazePlanCommand drop_kamikaze_attackrun_tick_009acfc0(const DropKamikazeApproach& ap,
                                                          float unit_y) noexcept;

// The release state (vtable 00D1FC40): +18h released, +1Ch timer, +20h speed.
struct DropKamikazeReleaseState {
    bool released_18 = false;
    float timer_1c = 3.0f;
    float speed_20 = 0.0f;
};
// 009AD6A0 (enter): +18h = unit+C25h, +1Ch = 3.0, +20h = max(|v|, level).
void drop_kamikaze_release_enter_009ad6a0(DropKamikazeReleaseState& st, bool unit_c25,
                                          float speed_b1c, float level_flight_speed) noexcept;
struct DropKamikazeReleaseTick {
    bool releases = false;           // 007BBBA0, approach+2Ch -= 1
    KamikazePlanCommand plan;
};
// 009AD5E0 (009AD5E0-009AD69B, RET 4). Sets approach+BCh = 1.
DropKamikazeReleaseTick drop_kamikaze_release_tick_009ad5e0(DropKamikazeReleaseState& st,
                                                            bool unit_c25, float dt) noexcept;

// The goAway state (vtable 00D1FC24): +18h "re-engage", +1Ch turn side.
struct DropKamikazeGoAwayState {
    bool reengage_18 = false;
    float side_1c = 1.0f;
};
// 009AD430: side from the parity of [00F876B0]: odd +1.0, even -1.0.
void drop_kamikaze_goaway_enter_009ad430(DropKamikazeGoAwayState& st, bool step_odd) noexcept;
// 009AD460 (exit): plan+2E8h = 1.0.
KamikazePlanCommand drop_kamikaze_goaway_exit_009ad460() noexcept;
struct DropKamikazeGoAwayTick {
    KamikazePlanCommand plan;        // throttle slots, 009FB800(alt_a8, 1.0), turn scale 0.5
    // 009FD570 (plane_fly_to_solver.hpp) at 009AD54C with ECX = the state:
    // (point = 009ABBC0, unit, standoff = start_dist_ac, &side_1c, range =
    // target_dist_b4, offset scale 1.0). Its result is plan+2C0h, mode 2.
    float flyto_standoff = 0.0f;
    float flyto_range = 0.0f;
    float flyto_offset_scale = 1.0f;
};
// 009AD480 (009AD480-009AD595, RET 4). The heading comes from the host's 009FD570.
DropKamikazeGoAwayTick drop_kamikaze_goaway_tick_009ad480(DropKamikazeGoAwayState& st,
                                                          const DropKamikazeApproach& ap,
                                                          bool reload_enabled) noexcept;

struct DropKamikazeEntryInputs {
    bool unit_c25 = false;           // unit+C25h release pending (007C0D90 sets it at 007C0EE2)
    int attack_mode_370 = 0;
    bool has_ordnance_4b9 = false;
    bool inside_4b8 = false;
    bool flight_leader = false;
};
// 009AE530: the attack state the rule enters.
DropKamikazeState drop_kamikaze_engaged_entry_009ae530(const DropKamikazeEntryInputs& in) noexcept;

struct DropKamikazeBreakOffInputs {
    bool base_allows = false;        // BSP_BotTask_BreakOffAllowedForUnit
    bool has_target_440 = false;
    bool target_marked_5d = false;
    bool reload_enabled = false;
    DropKamikazeState current = DropKamikazeState::kNone;
    bool has_ordnance_4b9 = false;
};
// 009AE1B0 BSP_BotTaskDropKamikaze_ShouldBreakOff (task vtable +1Ch).
bool drop_kamikaze_should_break_off_009ae1b0(const DropKamikazeBreakOffInputs& in) noexcept;

struct DropKamikazeRuleInputs {
    DropKamikazeState current = DropKamikazeState::kNone;
    bool inside_4b8 = false;
    bool has_target_440 = false;
    int attack_mode_370 = 0;
    bool flight_leader = false;
    bool reload_enabled = false;     // 007B58D0
    bool has_ordnance_4b9 = false;
    bool goaway_reengage_744 = false;// goAway +18h
    bool should_break_off = false;   // vtable +1Ch, 009AE1B0
    bool prepare_released_660 = false;
    bool prepare_pending = false;    // 009ADCC0 on the prepare state
    float release_timer_724 = 0.0f;  // release +1Ch
    bool unit_c25 = false;
};
// 009AE610 (009AE610-009AE897, RET 4). Returns the state after the rule.
DropKamikazeState drop_kamikaze_rule_009ae610(const DropKamikazeRuleInputs& in) noexcept;

// 009ADBF0 step 7: leader -> moveto, else follow.
DropKamikazeState drop_kamikaze_initial_state_009adbf0(bool flight_leader) noexcept;

// 009ADDB0 (task vtable +50h): 2 while approach+C1h, else 0.
int drop_kamikaze_step_result_009addb0(bool has_ordnance_c1) noexcept;

// 009AE210 (task vtable +24h), the manual drop: with approach+C1h and
// (unit+72Ch)->vtable[38h]() it adds one owed release, and on the prepare state
// arms its countdown (009AE060). Returns true (the image returns 1).
bool drop_kamikaze_manual_release_009ae210(DropKamikazeApproach& ap, DropKamikazeState current,
                                           bool manual_control_72c,
                                           DropKamikazeFollowState& prepare, float speed_b1c,
                                           float level_flight_speed) noexcept;

// The whole drop task, one value per unit.
struct DropKamikazeTask {
    DropKamikazeState state = DropKamikazeState::kNone;
    DropKamikazeApproach ap;
    DropKamikazeFollowState prepare;   // approach+1C8h
    DropKamikazeFollowState done;      // approach+26Ch
    DropKamikazeReleaseState release;  // approach+310h
    DropKamikazeGoAwayState goaway;    // approach+334h
    int task_2e4 = 0xFF;
};

struct DropKamikazeTaskHost {
    virtual ~DropKamikazeTaskHost() = default;
    virtual bool update_target_ref_009fada0(ApproachTargetRefState& ref, float dt,
                                            std::array<float, 3>& point) = 0;
    virtual bool target_marked_5d() = 0;
    virtual std::array<float, 3> unit_position() = 0;
    virtual float pitch_c64() = 0;
    virtual float roll_c68() = 0;
    virtual float speed_b1c() = 0;
    virtual float level_flight_speed_007c47f0() = 0;
    virtual bool unit_c25() = 0;                  // release pending (007C0EE2 sets it)
    virtual bool has_drop_ordnance_007b93e0() = 0;
    virtual bool reload_enabled_007b58d0() = 0;   // ctl+369h && [00E17BF2]
    virtual bool flight_leader_007b8ad0() = 0;
    virtual int attack_mode_370() = 0;
    virtual float ctl_alt_second_398() = 0;
    virtual bool leader_is_self() = 0;            // (ctl+3D0h) == unit
    virtual bool has_leader_3d0() = 0;            // ctl+3D0h != 0
    virtual std::array<float, 3> leader_position() = 0;
    virtual bool manual_control_72c() = 0;        // (unit+72Ch)->vtable[38h]()
    virtual bool break_off_allowed() = 0;         // BSP_BotTask_BreakOffAllowedForUnit
    virtual bool step_odd_00f876b0() = 0;         // [00F876B0] parity
    virtual void request_ordnance_release_007bbba0() = 0;
    // 009FD570 (plane_fly_to_solver.hpp) for the goAway heading.
    virtual float fly_to_heading_009fd570(const std::array<float, 3>& point, float standoff,
                                          float& side, float range, float offset_scale) = 0;
    // 009BDE80 on the moveto state, each arm (near, far, third).
    virtual void set_moveto_ranges_009bde80(float near_range, float far_range, float third) = 0;
    // moveto/follow and the follow base of prepare/done (009C1FD0, exit 009BDE40).
    virtual void enter_formation_state(DropKamikazeState s) = 0;
    // [[state+4]+0Ch]+3E4h = shape, then 007ED260 on that squadron (009AD1A5/009AD1AF
    // in the prepare/done enter, after the follow enter's own shape 1).
    virtual void set_squadron_formation_shape_007ed260(int shape) { (void)shape; }
    virtual void exit_formation_state(DropKamikazeState s) = 0;
    virtual void tick_formation_state(DropKamikazeState s, float dt) = 0;
    virtual void apply_plan(const KamikazePlanCommand& c) = 0;
};

// 009ADBF0 after the base and the approach (the host built `task.ap.ref`).
void drop_kamikaze_task_construct_009adbf0(DropKamikazeTask& task, DropKamikazeTaskHost& host,
                                           const DropKamikazeSeedInputs& seed);

struct DropKamikazeCruiseProfileHost : KamikazeCruiseProfileHost {
    virtual bool flight_leader_007b8ad0() = 0;
    virtual float rocket_cruising_alt_45c() = 0;   // tuning+45Ch
    virtual float rocket_drop_dist_460() = 0;      // tuning+460h
    // 009ADE00's inputs for `dt`, after the host's 009FADA0 on ap.ref.
    virtual DropKamikazeUpdateInputs approach_update_inputs_009ade00(DropKamikazeApproach& ap,
                                                                     float dt) = 0;
};
// 009AE9D0-009AEAD4 as a whole body (cc13-D1, not wired): 0099B660 (a bare
// RET), 007B8AD0, the leader's two channels from tuning+45Ch, the "ac" draw and
// the two distances, 009ADE00(0), JMP 0099B740.
void drop_kamikaze_task_cruise_profile_009ae9d0(DropKamikazeTask& task,
                                                DropKamikazeCruiseProfileHost& host,
                                                KamikazeRandom& rng);

struct DropKamikazeArmResult {
    DropKamikazeState before = DropKamikazeState::kNone;
    DropKamikazeState after = DropKamikazeState::kNone;
    int releases = 0;                  // 007BBBA0 requests this tick
};
// 009AE8A0 (task vtable +64h, 009AE8A0-009AE98D, RET 4).
DropKamikazeArmResult drop_kamikaze_task_arm_009ae8a0(DropKamikazeTask& task,
                                                      DropKamikazeTaskHost& host, float dt);

}  // namespace bsp
