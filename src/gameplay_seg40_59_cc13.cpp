// Segment 40 (00728A90-00735EC0) and segment 59 (008EE670-00922240) reference
// models. Packet cc13-gpr40; include/bsp/gameplay_seg40_59_cc13.hpp carries the
// per-function evidence and the adoption note, docs/CC13_GPR_40.md the tables.
// Reconstructed and build-tested only: not wired, not game-run.

#include "bsp/gameplay_seg40_59_cc13.hpp"

#include <cmath>
#include <cstddef>
#include <limits>

namespace bsp {
namespace gameplay_seg40_59_cc13 {

namespace {

constexpr float kFltMax = std::numeric_limits<float>::max();  // float 00CFDBF8
constexpr double kHundred = 100.0;        // double 00D7A220
constexpr double kOneAndHalf = 1.5;       // double 00CE3D78
// double 00CE3D28 = 00 00 00 60 FB 21 09 40: float pi widened to double, not
// exact pi. Loaded as an m64 by FMUL double ptr [00CE3D28] at 008FDC73, 008FDCCB,
// 008FDD6A and 008FDDCD.
constexpr double kPi = 3.1415927410125732;
constexpr double kHalfTurnDegrees = 180.0;       // double 00CE3D20
constexpr int kStream = 1;                // ECX = 1 at every call site below

// The image's FCOMI / JA "greater" test: false for NaN.
bool greater(float a, float b) noexcept { return a > b; }

float length3(float x, float y, float z) noexcept {
    // 0042B2F0: the root of the float sum; the result is stored to a float.
    return static_cast<float>(std::sqrt(static_cast<double>(x) * x
        + static_cast<double>(y) * y + static_cast<double>(z) * z));
}

float signed_error(float magnitude, std::uint32_t raw) noexcept {
    // 008FDC4D..008FDC7F: AND AL,1 / NEG / SBB / AND 2 / ADD -1 gives +1 for a
    // set bit 0 and -1 for a clear one; FILD, FMUL the double magnitude, FMUL
    // pi, FDIV 180, FSTP float. 24-bit precision rounds each step to a float.
    const float sign = (raw & 1u) != 0 ? 1.0f : -1.0f;
    const float signed_magnitude = sign * magnitude;
    const float times_pi = static_cast<float>(static_cast<double>(signed_magnitude) * kPi);
    return static_cast<float>(static_cast<double>(times_pi) / kHalfTurnDegrees);
}

}  // namespace

// --- segment 40 ---------------------------------------------------------------

bool gun_is_anti_air_kind_00728a90(int weapon_kind) noexcept {
    // 00728A9C / 00728AA1 / 00728AA6: CMP 1, 5, 6.
    return weapon_kind == 1 || weapon_kind == 5 || weapon_kind == 6;
}

bool gun_is_artillery_kind_005459b0(int weapon_kind) noexcept {
    return weapon_kind == 2 || weapon_kind == 3 || weapon_kind == 4 || weapon_kind == 6;
}

bool gun_submerged_fire_blocked_00729b2a(float muzzle_world_y, int weapon_kind) noexcept {
    // 00729B2F: COMISS 1.0, [y]; JBE passes when 1.0 <= y (not for NaN).
    if (kGunMuzzleWaterlineY <= muzzle_world_y) return false;
    // 00729B3F 00728A90 then 00729B4A 005459B0; either one refuses.
    return gun_is_anti_air_kind_00728a90(weapon_kind)
        || gun_is_artillery_kind_005459b0(weapon_kind);
}

float gun_least_barrel_timer_00729920(const float* timers, int barrel_count) noexcept {
    float least = kLeastBarrelTimerSeed;  // 00729923
    for (int i = 0; i < barrel_count; ++i) {
        // 0072996F FCOMI t, least; JA keeps `least` only when t > least.
        if (!greater(timers[i], least)) least = timers[i];
    }
    return least;
}

int gun_min_air_range_record_00729b90(int weapon_kind) noexcept {
    // 00729B96: CMP [class+80h], 6; JE reads +7Ch (record 1's bullet class).
    return weapon_kind == 6 ? 1 : 0;
}

bool gun_is_operational_00729f10(bool unit_720, bool gun_disabled_3b8,
                                 bool gun_dead_5d) noexcept {
    return !unit_720 && !gun_disabled_3b8 && !gun_dead_5d;  // 00729F16 / 1F / 28
}

float gun_torpedo_bot_accuracy_or_one_00729f40(int kind, const TorpedoBotParameters* bot_row) noexcept {
    // 00729F40 CMP [ESP+4], 7; 00729F47 the bot at gun+39Ch; 00729F5B vtable[20h].
    if (kind == 7 && bot_row != nullptr) return bot_row->fire_target_accuracy_014;
    return 1.0f;  // 00729F60 FLD1
}

float gun_class_max_weapon_range_00731020(const GunClassFirstRecord& r) noexcept {
    // 00731020 CMP [class+78h], 0; JNE reads [[class+74h]+34h]+60h; else FLD1.
    // There is no null test on the bullet class: a present record is assumed
    // to carry one, as 00729A80's test 1 requires for a gun that fires.
    if (!r.present) return 1.0f;
    return r.bullet_max_range_060;
}

GunRearmResult gun_rearm_barrel_0072d520(bool round_available, bool apply_reload,
    float reload_lo_028, float reload_hi_02c, const UniformFn& uniform) {
    GunRearmResult out;
    if (!round_available) {
        // 0072D592..0072D5A5: 0072CF00(barrel, FLT_MAX, 0).
        out.timer_written = true;
        out.timer = kFltMax;
        out.set_full = false;
        return out;
    }
    out.consumed_round = true;            // 0072D553 vtable[1F8h]
    if (!apply_reload) return out;        // 0072D555 / 0072D55A
    // 0072D562..0072D579: hi = +2Ch, lo = +28h, ECX = 1.
    out.timer = uniform(kStream, reload_lo_028, reload_hi_02c);
    out.draws = 1;
    out.timer_written = true;
    out.set_full = true;                  // 0072D565 PUSH 1
    return out;
}

bool gun_select_fire_mode_0072d5b0(GunFireModeState& gun, int mode, bool keep_timers,
    const std::function<bool()>& round_available_then_consume) {
    // 0072D5CF..0072D5DC: mode 0 with no mode buffers returns.
    if (mode == 0 && gun.mode_buffer_count_3ac == 0) return false;
    // 0072D5E2: the current mode returns.
    if (gun.mode_3fc == mode) return false;
    gun.mode_3fc = mode;                  // 0072D610; +3B4h and +3F8h re-pointed
    if (!keep_timers) {                   // 0072D609 / 0072D622
        for (float& timer : gun.barrel_timers_414) {
            // 0072D645 vtable[1F4h]; with a round 0072D660 vtable[1F8h] only.
            if (!round_available_then_consume()) timer = kFltMax;  // 0072D672
        }
    }
    return true;
}

bool gun_line_of_fire_decision_0072f6e0(std::vector<LineOfFireRecord>& cache,
    bool predicate_installed, std::uint32_t target,
    const std::function<bool()>& line_of_fire_clear_00729670,
    float think_time_08c, const UniformFn& uniform) {
    if (!predicate_installed) return true;  // 0072F6E6 / 0072F6EC
    for (const LineOfFireRecord& r : cache) {  // 0072F717 the forward scan
        if (r.target == target) return r.answer;  // 0072F821
    }
    const bool answer = line_of_fire_clear_00729670();  // 0072F73A
    // 0072F748..0072F772: U(0.8, 1.2) on stream 1, FMUL [settings+8Ch].
    const float u = uniform(kStream, kLineOfFireExpiryLo, kLineOfFireExpiryHi);
    const float expiry = static_cast<float>(static_cast<double>(u) * think_time_08c);
    cache.push_back(LineOfFireRecord{target, answer, expiry});  // 0072F809..0072F811
    return answer;
}

// --- segment 59: bot getters --------------------------------------------------

float aa_gunner_bot_bullet_throw_mul_008fb4c0(const AAGunnerBotConfig& c, int level) noexcept {
    return c.levels[static_cast<std::size_t>(level)].bullet_throw_mul_018;  // SHL 4, +18h
}
float tail_gunner_bot_bullet_throw_mul_008fb4e0(const TailGunnerBotConfig& c, int level) noexcept {
    return c.levels[static_cast<std::size_t>(level)].bullet_throw_mul_01c;  // *24h, +1Ch
}
float aa_flak_bot_bullet_throw_mul_008fb500(const AAFlakBotConfig& c, int level) noexcept {
    return c.levels[static_cast<std::size_t>(level)].bullet_throw_mul_028;  // SHL 5, +28h
}
float torpedo_bot_fire_target_accuracy_008fb530(const TorpedoBotConfig& c, int level) noexcept {
    // 008FB539..008FB53F: (level + 1) * 14h = level * 14h + 14h.
    return c.levels[static_cast<std::size_t>(level)].fire_target_accuracy_014;
}
float torpedo_bot_bullet_throw_mul_008fb550(const TorpedoBotConfig& c, int level) noexcept {
    return c.levels[static_cast<std::size_t>(level)].bullet_throw_mul_01c;  // *14h, +1Ch
}
float depth_charge_bot_bullet_throw_mul_008fb570(const DepthChargeBotConfig& c, int level) noexcept {
    return c.levels[static_cast<std::size_t>(level)].bullet_throw_mul_010;  // *14h, +10h
}

// --- segment 59: accept tests -------------------------------------------------

bool depth_charge_bot_accepts_008fb590(bool has_target, bool target_kind_8) noexcept {
    return has_target && target_kind_8;  // 008FB594 / 008FB59D IsKindOf(8)
}

bool aa_gunner_bot_accepts_008fbe00(const AaGunnerAcceptInputs& in,
    const std::function<bool()>& fire_window_0085a9a0,
    const std::function<bool()>& line_of_fire) {
    if (!in.has_target) return false;  // 008FBE08
    if (in.target_kind_4) {            // 008FBE1E IsKindOf(4)
        // 008FBE4B..008FBE5F: DamageMax only when strictly greater than Blast.
        const float strongest = greater(in.bullet_damage_max_0b0, in.bullet_blast_damage_max_0b8)
            ? in.bullet_damage_max_0b0 : in.bullet_blast_damage_max_0b8;
        // 008FBE77..008FBE7E: FCOMI armour, strongest; JAE refuses (not NaN).
        if (in.target_armour_04c >= strongest) return false;
    }
    if (!fire_window_0085a9a0()) return false;  // 008FBE93
    return line_of_fire();                       // 008FBEAC vtable[1D4h]
}

bool turret_or_flak_bot_accepts_008fbf70(bool has_target,
    const std::function<bool()>& fire_window_0085a9a0,
    const std::function<bool()>& line_of_fire) {
    if (!has_target) return false;
    if (!fire_window_0085a9a0()) return false;  // 008FBF9C / 008FBFEC
    return line_of_fire();                       // 008FBFB1 / 008FC001
}

bool torpedo_bot_accepts_008fc010(const TorpedoAcceptInputs& in,
    const std::function<bool()>& line_of_fire) {
    if (!in.has_target) return false;                        // 008FC01A
    if (!in.target_kind_6) return false;                     // 008FC021 IsKindOf(6)
    if (in.torpedo_proof_small_craft_00828ec0) return false;  // 008FC031
    if (in.target_dead_5d) return false;                     // 008FC03A
    return line_of_fire();                                   // 008FC04B
}

// --- segment 59: AAFlakBot ----------------------------------------------------

AaFlakError aa_flak_bot_roll_error_008fdbe0(const AAFlakBotParameters& row,
    const UniformFn& uniform, const NextU32Fn& next_u32) {
    AaFlakError e;
    const float r = uniform(kStream, 0.0f, 100.0f);  // 008FDC0D, hi = 00CE3D08
    e.draws = 1;
    // 008FDC12..008FDC24: GoodRatio * 100.0 (double) against r; JBE takes the
    // bad branch at 008FDD26 when good * 100 <= r.
    const float good_limit = static_cast<float>(static_cast<double>(row.good_ratio_00c) * kHundred);
    const bool good = greater(good_limit, r);
    const float angle_lo = good ? row.angle_err_min_010 : row.angle_err_min_014;
    const float angle_hi = good ? row.angle_err_min_014 : row.angle_err_bad_018;
    const float dist_lo = good ? row.dist_err_min_01c : row.dist_err_max_020;
    const float dist_hi = good ? row.dist_err_max_020 : row.dist_err_bad_024;
    // (robot_config.hpp names +14h `angle_err_min_014`; the reader stores
    // AngleErrMax there.)
    const float h = uniform(kStream, angle_lo, angle_hi);
    e.horz_058 = signed_error(h, next_u32(kStream));
    const float v = uniform(kStream, angle_lo, angle_hi);
    e.vert_05c = signed_error(v, next_u32(kStream));
    const float d = uniform(kStream, dist_lo, dist_hi);
    const std::uint32_t d_bit = next_u32(kStream);
    e.dist_060 = ((d_bit & 1u) != 0 ? 1.0f : -1.0f) * d;  // metres, no conversion
    e.draws += 6;
    return e;
}

AaFlakTargetSetResult aa_flak_bot_set_fire_target_008ff200(bool has_target, bool role_ai_held,
    int gun_kind, const AAFlakBotParameters& row, const UniformFn& uniform,
    const NextU32Fn& next_u32) {
    AaFlakTargetSetResult out;
    // 008FF209 006DF170 first; 008FF210 no target; 008FF21B..008FF228 the role.
    if (!has_target || !role_ai_held) return out;
    out.mode_switch_requested = gun_kind == 6;  // 008FF233 / 008FF240
    out.error = aa_flak_bot_roll_error_008fdbe0(row, uniform, next_u32);  // 008FF247
    out.rolled = true;
    return out;
}

TailGunnerSkill tail_gunner_bot_set_skill_008fbef0(const TailGunnerBotConfig& c, int level,
    const UniformFn& uniform) {
    const TailGunnerBotParameters& row = c.levels[static_cast<std::size_t>(level)];
    TailGunnerSkill s;
    s.level_034 = level;                          // 008FBEF7
    s.aim_period_min_088 = row.aim_period_min_010;  // 008FBF0E
    s.aim_period_max_08c = row.aim_period_max_014;  // 008FBF1E
    s.angle_error_084 = row.angle_error_00c;        // 008FBF33
    s.shoot_range_090 = row.shoot_range_018;        // 008FBF43
    s.aim_time_06c = uniform(kStream, s.aim_period_min_088, s.aim_period_max_08c);  // 008FBF5C
    return s;
}

// --- segment 59: TorpedoBot ---------------------------------------------------

float torpedo_bot_attack_range_00901ba0(const TorpedoBotParameters& row, float target_size,
    bool fire_target_06c, float gun_class_max_range_00731020,
    const HitAccuracyRangeFn& hit_accuracy_008387b0) {
    // 00901BBE..00901BDC: +14h (FireTargetAccuracy) or +18h (AnyTargetAccuracy).
    const float accuracy = fire_target_06c ? row.fire_target_accuracy_014
                                           : row.any_target_accuracy_018;
    // 00901BED..00901C0B: 008387B0(7, size, range, accuracy).
    return hit_accuracy_008387b0(7, target_size, gun_class_max_range_00731020, accuracy);
}

float torpedo_bot_target_size_00903420(const TorpedoTargetSizeInputs& in) noexcept {
    if (!in.has_target) return kTorpedoDefaultTargetSize;              // 00903441
    if (in.target_is_unit_kind_5) return in.unit_class_0a0;            // 00903464
    if (!in.has_bounds_0b0) return kTorpedoDefaultTargetSize;          // 0090348C
    // 009034B6..009034D6: the corner difference, stored to floats.
    const float ex = in.corner_a.x - in.corner_b.x;
    const float ey = in.corner_a.y - in.corner_b.y;
    const float ez = in.corner_a.z - in.corner_b.z;
    const float ax = std::fabs(ex), ay = std::fabs(ey), az = std::fabs(ez);
    // 00903506: FCOMI |x|, |z|; JBE takes |z| unless |x| is strictly greater.
    float size = greater(ax, az) ? ax : az;
    // 00903541: FCOMI |y|, size; JBE keeps size unless |y| is strictly greater.
    if (greater(ay, size)) size = ay;
    return size;
}

float min_float_by_ref_00415510(float a_ecx, float b_edx) noexcept {
    // 00415525 FCOMI b, a; JBE answers b; otherwise a.
    return greater(b_edx, a_ecx) ? a_ecx : b_edx;
}

TorpedoTickGateResult torpedo_bot_tick_gates_008fff20(const TorpedoTickInputs& in) noexcept {
    TorpedoTickGateResult out;
    // 1. 009000DD..009000F6, then 00900101 the reset.
    const float left = in.think_time_left_05c - in.dt;
    out.think_time_left_05c = left;
    if (greater(left, 0.0f)) {
        out.gate = TorpedoTickGate::NotThinking;
        return out;
    }
    out.think_time_left_05c = kTorpedoBotThinkPeriod;
    // 2. 009000FC..00900117.
    if (in.shallow_holder_064 && !in.shallow_ok_008527e0) {
        out.gate = TorpedoTickGate::ShallowRefused;
        return out;
    }
    // 3. 0090016C..009001F3. The three differences and the length are floats;
    // the length is widened to a double only for the compare, which does not
    // change its value.
    out.distance_a = length3(in.gun_position.x - in.target_position.x,
        in.gun_position.y - in.target_position.y, in.gun_position.z - in.target_position.z);
    const float range_scaled = static_cast<float>(
        static_cast<double>(in.unit_torpedo_range_44c) * kOneAndHalf);           // 009001C7
    const float attack_plus = static_cast<float>(
        static_cast<double>(in.attack_range_068) + kHundred);                     // 009001D4
    out.limit_a = min_float_by_ref_00415510(attack_plus, range_scaled);           // 009001E6
    if (greater(out.distance_a, out.limit_a)) {
        out.gate = TorpedoTickGate::OutOfRangeA;
        out.trigger_dropped = true;  // 009003A6..009003B5 vtable[1E8h](0)
        return out;
    }
    // 4. 0090025E..0090026B.
    if (!in.intercept_found_008fbb00) {
        out.gate = TorpedoTickGate::NoIntercept;
        out.trigger_dropped = true;  // 0090097B vtable[1E8h](0)
        return out;
    }
    // 5. 00900280..0090031E.
    const float ox = in.unit_spread_6d4 * in.target_forward.x;  // 0090029D
    const float oz = in.unit_spread_6d4 * in.target_forward.z;  // 009002A4
    out.lead_point = Vec3f{ox + in.intercept.x, in.intercept.y, in.intercept.z + oz};
    out.distance_b = length3(out.lead_point.x - in.gun_position.x, 0.0f,
        out.lead_point.z - in.gun_position.z);                   // 009002EB, y = 0
    out.limit_b = min_float_by_ref_00415510(in.unit_torpedo_range_44c,
        in.attack_range_068);                                    // 00900311
    if (greater(out.distance_b, out.limit_b)) {
        out.gate = TorpedoTickGate::OutOfRangeB;
        out.trigger_dropped = true;  // 00900974 vtable[1E8h](0)
        return out;
    }
    out.gate = TorpedoTickGate::Proceed;
    return out;
}

}  // namespace gameplay_seg40_59_cc13
}  // namespace bsp
