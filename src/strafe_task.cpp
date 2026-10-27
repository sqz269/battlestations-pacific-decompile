// The strafe bot task's rule and approach range test. Evidence in
// docs/SQUADRON_LAND_TASK.md sections 5bw and 5bx; every address in a comment
// is a listing address.

#include "bsp/strafe_task.hpp"

#include <cmath>

#include "bsp/plane_task_reach_sw3.hpp"
#include "bsp/unit_rudder.hpp"

namespace bsp {
namespace {

// 009CCF58-009CCF86: (unit y - aim y) / approach+30h + 2 * [+8]+188h. The
// height is spilled as a double (009CCF60 FSTP qword) and the subtraction
// runs in x87 precision before the float store at 009CCF86.
// At PC24 (kStrafeX87FaithfulCc13Bound) the FSUBR, FDIV and both FADDs
// each round to float; the OFF arm is the 53-bit (pool-worker) model.
// Both arms are templated so the sw3 reach counter can evaluate the other.
template <bool kPc24>
float strafe_climb_threshold(const StrafeRangeInputs& in) noexcept {
    if constexpr (kPc24) {
        const float climb = (in.unit_height_100 - in.aim_height) / in.glide_tan_30;
        return climb + (in.field_188 + in.field_188);
    } else {
        const double climb = (static_cast<double>(in.unit_height_100) - in.aim_height) /
                             in.glide_tan_30;
        return static_cast<float>(
            climb + (static_cast<double>(in.field_188) + in.field_188));
    }
}

template <bool kFaithful>
bool point_off_nose_impl(const float local[3]) noexcept {
    if constexpr (kFaithful) {
        const float xx = local[0] * local[0];        // 00419276-00419278
        const float zz = local[2] * local[2];        // 00419280-00419286
        const float sum = xx + zz;                   // 0041928A-0041928E
        const float root = std::sqrt(sum);           // 00419296-0041929B
        const float r = (root == 0.0f) ? 0.0f : 1.0f / root;  // 004192B3-004192C9
        const float rx = local[0] * r;               // 00996587-0099658B
        const float rz = r * local[2];               // 0099658F-00996593
        return std::fabs(rx) > rz;                   // 009965A5-009965B5
    } else {
        // BSP_Vector2f_ReciprocalLength over (x, z), then rz < |rx|.
        const double len2 = static_cast<double>(local[0]) * local[0] +
                            static_cast<double>(local[2]) * local[2];
        if (!(len2 > 0.0)) return false;
        const float rl = static_cast<float>(1.0 / std::sqrt(len2));
        return rl * local[2] < std::fabs(rl * local[0]);
    }
}

template <bool kFaithful>
float point_distance_square_impl(float dx, float dz) noexcept {
    if constexpr (kFaithful) {
        const float zz = dz * dz;                    // 0099635D
        const float xx = dx * dx;                    // 0099635F-00996361
        return zz + xx;                              // 00996363-00996365
    } else {
        return static_cast<float>(static_cast<double>(dz) * dz + static_cast<double>(dx) * dx);
    }
}

bool strafe_attack_state(StrafeState s) noexcept {
    // 009CC6A0-009CC6CA: aim, goaway, attackrun, prepare, gotowards.
    return s == StrafeState::kAim || s == StrafeState::kGoAway ||
           s == StrafeState::kAttackRun || s == StrafeState::kPrepare ||
           s == StrafeState::kGoTowards;
}

// 009CC5F0: mode 0 prepares; otherwise in range goes towards the target and
// out of range makes an attack run.
StrafeState strafe_engaged_entry_009cc5f0(const StrafeRuleInputs& in) noexcept {
    if (in.control_mode_370 == 0) return StrafeState::kPrepare;
    return in.in_attack_range_448 ? StrafeState::kGoTowards : StrafeState::kAttackRun;
}

}  // namespace

StrafeState strafe_rule_009cc690(const StrafeRuleInputs& in) noexcept {
    // 009CC6D0-009CC6FF and 009CC7D3-009CC7F5: the same engaged test in both
    // arms.
    const bool engaged = in.in_attack_range_448 ||
        (in.control_mode_370 == 2 && (in.strafe_target_44c || in.ref_target_468));
    if (strafe_attack_state(in.current)) {
        if (engaged) {
            if (in.control_mode_370 == 0) return StrafeState::kPrepare;  // 009CC70B
            switch (in.current) {
                case StrafeState::kPrepare:
                    return strafe_engaged_entry_009cc5f0(in);          // 009CC7FF
                case StrafeState::kAttackRun:
                    return in.in_attack_range_448 ? StrafeState::kGoTowards : in.current;
                case StrafeState::kGoAway:
                    return in.goaway_done_6b4 ? StrafeState::kGoTowards : in.current;  // 009CC764
                case StrafeState::kGoTowards:
                    return in.gotowards_ready ? StrafeState::kAim : in.current;        // 009CC78E
                case StrafeState::kAim:
                    return (in.aim_done_68d || in.aim_done_68c)                        // 009CC7B0
                        ? StrafeState::kGoAway : in.current;
                default:
                    return in.current;
            }
        }
    } else if (engaged) {
        return strafe_engaged_entry_009cc5f0(in);                             // 009CC7FF
    }
    // Not engaged: moveto for the flight leader, follow for a wing member.
    return in.flight_leader ? StrafeState::kMoveTo : StrafeState::kFollow;
}

bool strafe_gotowards_ready_009cc2f0(bool aligned_20, float distance_18,
                                     float field_188, float approach_38) noexcept {
    // 009CC2F0 CMP byte [ECX+20h],0; 009CC2FF..009CC30E: FLD +188h, FADD ST0,
    // FADD approach+38h, FCOMI against +18h, JBE false.
    if (!aligned_20) return false;
    return field_188 + field_188 + approach_38 > distance_18;
}

// 009CCED0-009CD01D, __thiscall(ECX = strafe approach, float dt), RET 4.
// DOCUMENTED DEVIATION (cc13-D2): the first call 009FADA0(dt) on approach+58h
// (009CCEE1, the target ref and its aim point) is made by the units host,
// st_refresh_aim_point, immediately before this, and the kind test 009CA310 and
// 009CAD00 distance arrive as inputs (both pure reads). DEFECT ROW (OFF-switched,
// kStrafeX87FaithfulCc13Bound): the climb chain's PC24 rounding and the NaN
// countdown arm.
void strafe_approach_update_009cced0(StrafeApproachUpdate& ap, float dt,
                                     const StrafeRangeInputs& in) noexcept {
    ap.elapsed_44 = ap.elapsed_44 + dt;                    // 009CCEE6-009CCEF3
    const float countdown = ap.countdown_d0;
    // 009CCF06 FCOMI dt,c / JC: an unordered compare also decrements (ON).
    const bool decrement_on = !(dt >= countdown);
    const bool decrement_off = dt < countdown;
    plane_task_reach_count(g_plane_task_reach_sw3.strafe_countdown,
                           decrement_on != decrement_off);   // sw3 reach
    const bool decrement = kStrafeX87FaithfulCc13Bound ? decrement_on : decrement_off;
    if (decrement) {                                       // 009CCF06 FCOMI, JB
        ap.countdown_d0 = countdown - dt;                  // 009CCF9F
        return;
    }
    ap.countdown_d0 = (ap.period_cc - dt) + countdown;     // 009CCF0E-009CCF16
    if (!in.has_target || in.target_disabled_5d) {         // 009CCF1C-009CCF3B
        ap.in_range_50 = false;                            // 009CD015
        return;
    }
    // 009CCF58-009CCF86, strafe_climb_threshold (kStrafeX87FaithfulCc13Bound).
    const float threshold_on = strafe_climb_threshold<true>(in);
    const float threshold_off = strafe_climb_threshold<false>(in);
    // 009CCF8A-009CCFE3: floored at tuning+658h times 1.0 ([00D7A24C]) or, for
    // a kind 10h/16h plane, 1.4 ([00D06874]).
    const float scale = in.not_bomber_kind ? 1.0f : 1.39999997615814208984375f;
    const float floor = in.attack_dist_658 * scale;
    const auto in_range = [&](float threshold) {
        if (!(threshold > floor)) threshold = floor;
        // 009CCFE9-009CCFF8: in range when the threshold exceeds 009CAD00.
        return threshold > in.horizontal_distance;
    };
    const bool in_range_on = in_range(threshold_on);
    const bool in_range_off = in_range(threshold_off);
    plane_task_reach_count(g_plane_task_reach_sw3.strafe_in_range, in_range_on != in_range_off,
                           plane_task_reach_bits_differ(threshold_on, threshold_off));   // sw3 reach
    ap.in_range_50 = kStrafeX87FaithfulCc13Bound ? in_range_on : in_range_off;
}

float tight_turn_pitch_ref_009bafc0(float altitude, float speed, float pitch,
                                    float climb_angle) noexcept {
    // 009BB018-009BB024: f = unit+100h - 250.0 ([00CF8850] double), stored float.
    const float f = static_cast<float>(static_cast<double>(altitude) - 250.0);
    float floor_400;
    if (f <= 0.0f) {
        floor_400 = climb_angle;   // 009BB0BA-009BB0C8: desc+1ECh
    } else {
        // 009BB038-009BB05B: (f / 10.0 [00CE3DC0]) stored float, then FDIVR by
        // vtable[38h]'s ST0 and stored float.
        const float f10 = static_cast<float>(static_cast<double>(f) / 10.0);
        const float r = static_cast<float>(static_cast<double>(f10) / static_cast<double>(speed));
        float v;
        if (r > 1.0f) {
            v = 0.0f;                                  // 009BB06B
        } else if (-1.0f > r) {
            v = 3.14159274f;                           // 009BB081 [00D7A264]
        } else {
            v = static_cast<float>(std::acos(static_cast<double>(r)));   // 009BB08D
        }
        floor_400 = -0.0f - v;                         // 009BB0A4-009BB0B0
    }
    // 009BB0CE-009BB10A: +3FCh = pitch when pitch > +400h, else +400h.
    return pitch > floor_400 ? pitch : floor_400;
}

TightTurnCommand tight_turn_tick_009ba020(bool side_408, float roll_c68, float pitch_c64,
                                          float ref_3fc) noexcept {
    TightTurnCommand c;
    const float side = side_408 ? 1.0f : -1.0f;               // [00D7A24C] / [00D7A260]
    c.bank_target = side * static_cast<float>(1.5707963267948966);   // [00CE3830]
    float e = wrapped_angle_subtract_00438b10(roll_c68, c.bank_target);
    if (e <= 0.0f) e = -0.0f - e;                             // [00D7A208]
    float p = static_cast<float>(2.0 - static_cast<double>(e) /
                                 static_cast<double>(static_cast<float>(0.17453292519943295)));
    if (0.0f <= p) {
        if (1.0f < p) p = 1.0f;
    } else {
        p = 0.0f;
    }
    c.pitch_desired = p;
    // 00419010(-2 deg [00D20370], 0, 2 deg [00D0C26C], 1.0, +3FCh - unit+C64h).
    c.yaw_desired = clamped_interpolate_00419010(-0.0349065848f, 0.0f, 0.0349065848f, 1.0f,
                                                 ref_3fc - pitch_c64) * side;
    return c;
}

// 00996510-009965C5, __fastcall(ECX = condition) -> AL. The caller supplies
// the point in the body frame (the 004142E0 transform at 00996551 and the pose
// refresh before it). 00996572 calls 00419260 on (x, z); 00996585-009965B5:
// rx = x * r and rz = r * z are stored as float, and the answer is |rx| > rz
// (FCOMIP, JBE false, so a NaN answers false).
// 00419260 (00419260-004192D4): x*x, z*z and their sum are each stored as
// float, the root (00BF7030, FSQRT at the caller's precision) is stored as
// float, and a root equal to 0 returns 0 (FUCOMIP, LAHF/TEST AH,44h/JP), so
// r*x and r*z are 0 and the answer is false; otherwise 1/root, stored.
// DEFECT ROW (cc13-D2): the OFF arm takes 1/sqrt of a double sum and tests
// the sum, which differs from 00419260 only in rounding and at float
// underflow/overflow of the squares. ABI: not ABI-compatible (plain C++).
bool point_off_nose_00996510(const float local[3]) noexcept {
    // point_off_nose_impl holds both arms.
    const bool on = point_off_nose_impl<true>(local);
    const bool off = point_off_nose_impl<false>(local);
    plane_task_reach_count(g_plane_task_reach_sw3.strafe_off_nose, on != off);   // sw3 reach
    return kStrafeX87FaithfulCc13Bound ? on : off;
}

// 00996300-00996385, __fastcall(ECX = condition) -> AL. 0099631A-00996374:
// dx and dz (point +1Ch/+24h minus plane +FCh/+104h) are each stored as float,
// d2 = dz*dz + dx*dx is stored as float at 00996365 and reloaded, and the
// FCOMIP against +28h answers 0 when d2 is below it (JC). The comparison is of
// the stored float, as here; the audit note that the image compares the x87
// value was wrong. DEFECT ROW (cc13-D2): the OFF arm forms the squares and sum
// in double (the 53-bit case); at PC24 each rounds to float.
bool point_distance_reached_00996300(float plane_x, float plane_z, float point_x,
                                     float point_z, float dist2) noexcept {
    const float dx = point_x - plane_x;
    const float dz = point_z - plane_z;
    // point_distance_square_impl holds both arms of the square.
    const float d2_on = point_distance_square_impl<true>(dx, dz);
    const float d2_off = point_distance_square_impl<false>(dx, dz);
    // 00996370 FCOMIP d2,[+28h] / JC 0099637F (XOR EAX,EAX): an unordered compare
    // sets CF, so a NaN answers 0 (ON). OFF answers true on NaN.
    const bool on = d2_on >= dist2;
    const bool off = !(d2_off < dist2);
    plane_task_reach_count(g_plane_task_reach_sw3.strafe_distance, on != off,
                           plane_task_reach_bits_differ(d2_on, d2_off));   // sw3 reach
    return kStrafeX87FaithfulCc13Bound ? on : off;
}

}  // namespace bsp
