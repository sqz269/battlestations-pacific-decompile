#pragma once

// Segment 40 (00728A90-00735EC0) and segment 59 (008EE670-00922240) reference
// models: the gun's fire gates, re-arm and ammunition mode, the line-of-fire
// cache, and the gun bots' accept tests, target set, skill set and the
// TorpedoBot tick's think timer and range gates. Packet cc13-gpr40;
// docs/CC13_GPR_40.md carries the tables.
//
// These are semantic interfaces read from the disk bytes (Capstone disasm-raw),
// not native layouts or ABI-compatible replacements. Field names are
// hypotheses; the "_NNN" suffix is the native byte offset. Every engine service
// the image calls (virtual answers, the RNG, the hit-accuracy table) is an
// input or a host callback, so nothing here invents a global or stubs a call.
// Gameplay runs at 24-bit x87 precision (docs/CC13_FPU_PRECISION.md): every
// float the image stores is rounded here by a float store at the same point.
// Build-tested only; not wired, not game-run.
//
// RNG convention: `uniform(stream, lo, hi)` is 00BD2F10 with ECX = stream, and
// `next_u32(stream)` is 00BD2FC0 (one raw MT19937 output of the same stream).
// Stream 1 is the gameplay stream (docs/RANDOM_STREAMS.md). Every draw below is
// on stream 1, ECX = 1 read at the call site.
//
// ADOPTION NOTE (cc9 terms; the full table is docs/CC13_GPR_40.md section 2):
// - Replaces:
//   * 00728A90 / the 00729A80 test 7 composite: `gate.muzzle_submerged = false`
//     at src/game_hosts_gunnery.cpp:7854 and :11776. 00728A90 answers kinds
//     1, 5 and 6 (the AA set), so an AA gun (kind 1 or 5) whose muzzle is below
//     y = 1.0 cannot fire. The host's AA guns fire under water.
//   * 008FFF20's think timer and two range gates, and its no-solution exit:
//     the host torpedo path src/game_hosts_gunnery.cpp:6933-6952 (lead = target
//     when 008FBB00 has no solution) and the "0.2 s think timer stays
//     unmodelled" note at :837.
//   * 008FC010, the TorpedoBot's slot accept: the range-only slot answer the
//     host gives category 7 at src/game_hosts_gunnery.cpp:5594-5609.
//   * 008FF200, the AAFlakBot target set: the host rolls 008FDBE0 only after a
//     shot (src/game_hosts_gunnery.cpp:7249, `fe.fired`), never at acquisition.
//   * 008FBEF0, the TailGunnerBot skill set: no host draw.
//   * 0072D520's reload draw: the labelled omission at
//     src/game_hosts_gunnery.cpp:3088.
//   * 0072D5B0: no host runs an ammunition mode switch (cadence item 8).
// - Inputs: the gun's weapon kind [[gun+3F4h]+80h] (the host's category), the
//   muzzle world y, barrel timers, the bot's skill index bot+34h, the robot rows
//   (include/bsp/robot_config.hpp), virtual answers passed in as values.
// - RNG (stream 1, image order):
//   * 0072D579: one U(ReloadTime[1], ReloadTime[2]) per shot that finds a round
//     (after the two throw draws 00730557 / 00730575 of the same shot);
//   * 0072F76D: one U(0.8, 1.2) per line-of-fire cache miss, after the predicate;
//   * 008FF245 -> 008FDBE0: seven per AAFlakBot target set (U(0, 100), then
//     U, MT bit, U, MT bit, U, MT bit);
//   * 008FBF5C: one U(AimPeriodMin, AimPeriodMax) per TailGunnerBot skill set;
//   * none in the torpedo gates, the accept tests or the gun gates.
// - OFF-switch prediction: OFF changes nothing. ON, per model, in
//   docs/CC13_GPR_40.md section 2.

#include "bsp/robot_config.hpp"

#include <cstdint>
#include <functional>
#include <vector>

namespace bsp {
namespace gameplay_seg40_59_cc13 {

using UniformFn = std::function<float(int stream, float lo, float hi)>;
using NextU32Fn = std::function<std::uint32_t(int stream)>;

struct Vec3f {
    float x = 0.0f;
    float y = 0.0f;
    float z = 0.0f;
};

// ---------------------------------------------------------------------------
// Segment 40: gun queries and gates.

// 00728A90 BSP_Gun_IsAntiAirKind, __thiscall(gun) -> bool. [[gun+3F4h]+80h]
// is 1, 5 or 6. Body 00728A90-00728AB3.
bool gun_is_anti_air_kind_00728a90(int weapon_kind) noexcept;

// 005459B0 (seg 16-28, hosted as gun_artillery_kind_005459b0): 2, 3, 4 or 6.
// Restated here only so the test 7 composite below is self-contained.
bool gun_is_artillery_kind_005459b0(int weapon_kind) noexcept;

// 00729A80 test 7, 00729B2A..00729B51: the muzzle's world y (00427EB0 +4) is
// compared with *00D7A24C = 1.0f by COMISS / JBE, so y >= 1.0 passes and a
// NaN y falls through to the two kind predicates. True = CanFire refuses.
inline constexpr float kGunMuzzleWaterlineY = 1.0f;  // float 00D7A24C
bool gun_submerged_fire_blocked_00729b2a(float muzzle_world_y, int weapon_kind) noexcept;

// 00729920 BSP_Gun_LeastBarrelTimer, __thiscall(gun) -> float. The least of
// gun[+414h][0 .. gun+448h). Seed *00CFDEB0 = 3600.0f. The step is FCOMI /
// JA: a timer replaces the running value unless it is strictly greater, so a
// NaN timer replaces it. The 4-way unrolled loop and the tail loop apply the
// same step in index order.
inline constexpr float kLeastBarrelTimerSeed = 3600.0f;  // float 00CFDEB0
float gun_least_barrel_timer_00729920(const float* timers, int barrel_count) noexcept;

// 00729B90 BSP_Gun_MinimumAirEngagementRange, __thiscall(gun) -> float: the
// MinRange (+58h) of the bullet class of fire record 0 ([[gun+3F4h]+74h]+34h),
// or of record 1 (+74h then +7Ch = 48h + 34h) when the kind is 6. No null test.
int gun_min_air_range_record_00729b90(int weapon_kind) noexcept;

// 00729F10 BSP_Gun_IsOperational, __thiscall(gun) -> bool: [[gun+3F0h]+720h],
// gun+3B8h and gun+5Dh all clear.
bool gun_is_operational_00729f10(bool unit_720, bool gun_disabled_3b8,
                                 bool gun_dead_5d) noexcept;

// 00729F40 BSP_Gun_TorpedoBotAccuracyOrOne, __thiscall(gun)(int kind) -> float,
// RET 4. Kind 7 with a bot at gun+39Ch: bot->vtable[20h](), which for the
// TorpedoBot vtable 00D182B0 is 008FB530, the level row's FireTargetAccuracy.
// Otherwise FLD1.
float gun_torpedo_bot_accuracy_or_one_00729f40(int kind, const TorpedoBotParameters* bot_row) noexcept;

// 00731020 / 00731040 / 00731050, __thiscall(gun CLASS). All three read fire
// record 0 ([class+74h], present when [class+78h] != 0), not the selected record
// gun+3F8h: MaxRange (bullet +60h) or 1.0f, the bullet class or null, the
// record or null.
struct GunClassFirstRecord {
    bool present = false;      // [class+78h] != 0
    bool bullet_present = false;
    float bullet_max_range_060 = 0.0f;
};
float gun_class_max_weapon_range_00731020(const GunClassFirstRecord& r) noexcept;

// 0072D520 BSP_Gun_RearmBarrel, __thiscall(gun)(int barrel, bool apply_reload),
// RET 8. The unit's provider answers vtable[1F4h] (a round available); with a
// round, vtable[1F8h] consumes it, and only then (apply_reload) is the reload
// drawn: U(stream 1, rec+28h, rec+2Ch) at 0072D579, written by
// 0072CF00(barrel, t, set_full = 1). With no round the barrel is pinned at
// *00CFDBF8 = FLT_MAX by 0072CF00(barrel, FLT_MAX, 0) and nothing is drawn.
struct GunRearmResult {
    bool consumed_round = false;
    bool timer_written = false;
    float timer = 0.0f;
    bool set_full = false;
    int draws = 0;
};
GunRearmResult gun_rearm_barrel_0072d520(bool round_available, bool apply_reload,
    float reload_lo_028, float reload_hi_02c, const UniformFn& uniform);

// 0072D5B0 BSP_Gun_SelectFireMode, __thiscall(gun)(int mode, bool keep_timers),
// RET 8. Returns whether the mode changed. Mode 0 with gun+3ACh == 0, or the
// current mode gun+3FCh, is a no-op. Otherwise gun+3B4h = gun+3A8h + 14h*mode,
// gun+3FCh = mode, gun+3F8h = [class+74h] + 48h*mode (so ReloadTime and
// BarrelDelayTime follow), and unless keep_timers each barrel 0 .. gun+448h
// either consumes a round (vtable[1F8h]; its timer is NOT touched) or, with no
// round, is pinned at FLT_MAX. Then a 0ACh session message carrying the mode is
// sent through 0077C7B0 (not modelled: the caller's).
struct GunFireModeState {
    int mode_3fc = 0;
    int mode_buffer_count_3ac = 0;
    std::vector<float> barrel_timers_414;  // size = gun+448h
};
bool gun_select_fire_mode_0072d5b0(GunFireModeState& gun, int mode, bool keep_timers,
    const std::function<bool()>& round_available_then_consume);

// 0072F6E0 via 00730A20 (gun vtable[1D4h], this = gun+424h): the line-of-fire
// cache. Records are 0Ch bytes {target, answer, expiry}. No predicate at +8 ->
// true. A record for the target -> its answer. Otherwise the answer is
// 00729670 (the predicate's negation), then expiry = U(stream 1, 0.8, 1.2) *
// [00432650()+8Ch] (0072F76D, 0072F772), and the record is appended (capacity
// 2n + 2). 0072AD40 ages gun+114h's records, not these.
struct LineOfFireRecord {
    std::uint32_t target = 0;
    bool answer = false;
    float expiry = 0.0f;
};
inline constexpr float kLineOfFireExpiryLo = 0.8f;  // float 00CE74F8
inline constexpr float kLineOfFireExpiryHi = 1.2f;  // float 00CE3814
bool gun_line_of_fire_decision_0072f6e0(std::vector<LineOfFireRecord>& cache,
    bool predicate_installed, std::uint32_t target,
    const std::function<bool()>& line_of_fire_clear_00729670,
    float think_time_08c, const UniformFn& uniform);

// ---------------------------------------------------------------------------
// Segment 59: the gun bots.

// The BulletThrowMul getters, vtable[24h] of each bot, and the TorpedoBot's
// FireTargetAccuracy getter (vtable[20h]). No bounds test on the level.
float aa_gunner_bot_bullet_throw_mul_008fb4c0(const AAGunnerBotConfig& c, int level) noexcept;
float tail_gunner_bot_bullet_throw_mul_008fb4e0(const TailGunnerBotConfig& c, int level) noexcept;
float aa_flak_bot_bullet_throw_mul_008fb500(const AAFlakBotConfig& c, int level) noexcept;
float torpedo_bot_fire_target_accuracy_008fb530(const TorpedoBotConfig& c, int level) noexcept;
float torpedo_bot_bullet_throw_mul_008fb550(const TorpedoBotConfig& c, int level) noexcept;
float depth_charge_bot_bullet_throw_mul_008fb570(const DepthChargeBotConfig& c, int level) noexcept;

// The bots' slot accept tests, vtable[1Ch]. `line_of_fire` is the gun's
// vtable[1D4h] answer (0072F6E0), asked last; `fire_window` is 0085A9A0.
// 008FB590 (DepthChargeBot): a target answering IsKindOf(8).
bool depth_charge_bot_accepts_008fb590(bool has_target, bool target_kind_8) noexcept;
// 008FBE00 (AAGunnerBot, gun at bot+5Ch): a target answering IsKindOf(4) is
// refused when its class Armour (+4Ch) >= max(DamageMax +B0h, BlastDamageMax
// +B8h) of the bullet class of [gun+354h]'s fire record 0 (FCOMI / JAE: a NaN
// armour passes). The max takes DamageMax only when it is strictly greater.
struct AaGunnerAcceptInputs {
    bool has_target = false;
    bool target_kind_4 = false;
    float target_armour_04c = 0.0f;
    float bullet_damage_max_0b0 = 0.0f;
    float bullet_blast_damage_max_0b8 = 0.0f;
};
bool aa_gunner_bot_accepts_008fbe00(const AaGunnerAcceptInputs& in,
    const std::function<bool()>& fire_window_0085a9a0,
    const std::function<bool()>& line_of_fire) ;
// 008FBF70 (TailGunnerBot) and 008FBFC0 (AAFlakBot), gun at bot+68h: the fire
// window, then the line of fire. Byte-identical bodies.
bool turret_or_flak_bot_accepts_008fbf70(bool has_target,
    const std::function<bool()>& fire_window_0085a9a0,
    const std::function<bool()>& line_of_fire);
// 008FC010 (TorpedoBot, gun at bot+58h): IsKindOf(6), then 00828EC0 on the
// target class (+538h) must answer false, then +5Dh clear, then the line of fire.
struct TorpedoAcceptInputs {
    bool has_target = false;
    bool target_kind_6 = false;
    bool torpedo_proof_small_craft_00828ec0 = false;
    bool target_dead_5d = false;
};
bool torpedo_bot_accepts_008fc010(const TorpedoAcceptInputs& in,
    const std::function<bool()>& line_of_fire);

// 008FDBE0 BSP_AAFlakBot_RollError, __fastcall(bot). Seven stream-1 draws in
// this order: r = U(0, 100); then for h, v, d: U(lo, hi) and one raw MT output
// whose bit 0 picks the sign (set -> +1, clear -> -1). r < GoodRatio * 100
// (FMUL by the double 100.0 at 00D7A220, FCOMI / JBE) takes [AngleErrMin,
// AngleErrMax] and [DistErrMin, DistErrMax]; otherwise [AngleErrMax,
// AngleErrBad] and [DistErrMax, DistErrBad]. The angles become radians as
// (sign * U) * pi / 180 with both constants doubles (00CE3D28, 00CE3D20); the
// distance stays metres. bot+64h then takes [00F876A4] and the distance is
// handed to the gun in a fresh 8-byte object at gun+41Ch (vtable 00CFE4F8).
struct AaFlakError {
    float horz_058 = 0.0f;
    float vert_05c = 0.0f;
    float dist_060 = 0.0f;
    int draws = 0;
};
AaFlakError aa_flak_bot_roll_error_008fdbe0(const AAFlakBotParameters& row,
    const UniformFn& uniform, const NextU32Fn& next_u32);

// 008FF200 BSP_GunBot_SetFireTargetBallisticClass (AAFlakBot vtable slot), RET 4.
// 006DF170 stores the target first. With a target and the role slot
// [[bot+50h]+1ACh] AI-held (8, or 00927F10 answers): a kind-6 gun switches to
// fire mode 1 with keep_timers = 0 (0072D5B0), then the error is rolled.
struct AaFlakTargetSetResult {
    bool mode_switch_requested = false;  // 0072D5B0(1, 0)
    bool rolled = false;
    AaFlakError error;
};
AaFlakTargetSetResult aa_flak_bot_set_fire_target_008ff200(bool has_target, bool role_ai_held,
    int gun_kind, const AAFlakBotParameters& row, const UniformFn& uniform,
    const NextU32Fn& next_u32);

// 008FBEF0 BSP_GunBot_SetSkillTurretClass (TailGunnerBot vtable[18h]), RET 4:
// bot+34h = level; copies AngleError -> +84h, AimPeriodMin -> +88h,
// AimPeriodMax -> +8Ch, ShootRange -> +90h; then aimTime +6Ch = U(stream 1,
// +88h, +8Ch). 008FEA40 (the class's SetFireTarget) overwrites +6Ch with -1.0f
// (00D7A260), so the draw's lasting effect is the stream advance.
struct TailGunnerSkill {
    int level_034 = 0;
    float angle_error_084 = 0.0f;
    float aim_period_min_088 = 0.0f;
    float aim_period_max_08c = 0.0f;
    float shoot_range_090 = 0.0f;
    float aim_time_06c = 0.0f;
};
TailGunnerSkill tail_gunner_bot_set_skill_008fbef0(const TailGunnerBotConfig& c, int level,
    const UniformFn& uniform);

// --- TorpedoBot -------------------------------------------------------------

// 00901BA0, __thiscall(bot)(float target_size, bool fire_target) -> float,
// RET 8: 008387B0(settings, 7, size, 00731020(gun class), accuracy), accuracy =
// FireTargetAccuracy (row +14h) when fire_target, else AnyTargetAccuracy (+18h).
using HitAccuracyRangeFn = std::function<float(int kind, float size, float range, float accuracy)>;
float torpedo_bot_attack_range_00901ba0(const TorpedoBotParameters& row, float target_size,
    bool fire_target_06c, float gun_class_max_range_00731020,
    const HitAccuracyRangeFn& hit_accuracy_008387b0);

// 00903420, __thiscall(bot): bot+68h = 00901BA0(size, bot+6Ch). size is 100.0f
// (00CE3D08) without a target or a bounds object; a unit target (IsKindOf(5))
// gives its class +0A0h; any other target gives the largest component of its
// bounds extent (0098A710's two corners): max(|x|, |z|) taking |z| on a tie or
// NaN, then |y| only when strictly greater.
struct TorpedoTargetSizeInputs {
    bool has_target = false;
    bool target_is_unit_kind_5 = false;
    float unit_class_0a0 = 0.0f;
    bool has_bounds_0b0 = false;
    Vec3f corner_a;
    Vec3f corner_b;
};
inline constexpr float kTorpedoDefaultTargetSize = 100.0f;  // float 00CE3D08
float torpedo_bot_target_size_00903420(const TorpedoTargetSizeInputs& in) noexcept;

// 008FF310 (attach) sets bot+58h = the payload (the gun), thinkTimeLeft +5Ch =
// 0, and bot+64h = the payload when it answers IsKindOf(8), else 0. 008FF2B0
// (clear) drops the gun's trigger (vtable[1E8h](0)) when the role is AI-held
// and clears bot+6Ch.
//
// The tick 008FFF20 after its target and role tests (009000DD..0090041C):
//  1. thinkTimeLeft -= dt (stored to float); > 0 -> return (FCOMI / JA; NaN
//     proceeds); else thinkTimeLeft = 0.2f (00CE54A0).
//  2. bot+64h set and 008527E0(bot+64h) false -> return, trigger untouched.
//  3. d = |target - gun| in 3D (0042B2F0); limit = min(unit+44Ch * 1.5 (the
//     double 00CE3D78), bot+68h + 100.0 (00D7A220)) through 00415510;
//     d > limit -> trigger off (vtable[1E8h](0)), return.
//  4. 008FBB00 has no intercept -> trigger off, return.
//  5. P = intercept + unit+6D4h * target forward (x, z); d2 = |P - gun| in
//     x, z with y = 0; limit2 = min(unit+44Ch, bot+68h); d2 > limit2 -> trigger
//     off, return.
//  6. The heading, the 1-degree test and CanFire (hosted).
// 00415510 BSP_Math_MinFloatByRef(ecx = &a, edx = &b) answers b > a ? a : b,
// so ties and a NaN b answer b.
enum class TorpedoTickGate {
    NotThinking,     // step 1
    ShallowRefused,  // step 2
    OutOfRangeA,     // step 3, trigger dropped
    NoIntercept,     // step 4, trigger dropped
    OutOfRangeB,     // step 5, trigger dropped
    Proceed,         // step 6 runs
};
struct TorpedoTickInputs {
    float think_time_left_05c = 0.0f;
    float dt = 0.0f;
    bool shallow_holder_064 = false;
    bool shallow_ok_008527e0 = true;
    Vec3f gun_position;
    Vec3f target_position;
    float unit_torpedo_range_44c = 0.0f;
    float attack_range_068 = 0.0f;
    bool intercept_found_008fbb00 = false;
    Vec3f intercept;
    float unit_spread_6d4 = 0.0f;
    Vec3f target_forward;
};
struct TorpedoTickGateResult {
    TorpedoTickGate gate = TorpedoTickGate::NotThinking;
    float think_time_left_05c = 0.0f;
    bool trigger_dropped = false;
    float limit_a = 0.0f;
    float distance_a = 0.0f;
    float limit_b = 0.0f;
    float distance_b = 0.0f;
    Vec3f lead_point;
};
inline constexpr float kTorpedoBotThinkPeriod = 0.2f;  // float 00CE54A0
TorpedoTickGateResult torpedo_bot_tick_gates_008fff20(const TorpedoTickInputs& in) noexcept;
float min_float_by_ref_00415510(float a_ecx, float b_edx) noexcept;

}  // namespace gameplay_seg40_59_cc13
}  // namespace bsp
