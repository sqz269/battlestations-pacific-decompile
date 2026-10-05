// The strafe bot task's rule and approach range test. Evidence in
// docs/SQUADRON_LAND_TASK.md sections 5bw and 5bx; every address in a comment
// is a listing address.

#include "bsp/strafe_task.hpp"

#include <cmath>

#include "bsp/unit_rudder.hpp"

namespace bsp {
namespace {

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

void strafe_approach_update_009cced0(StrafeApproachUpdate& ap, float dt,
                                     const StrafeRangeInputs& in) noexcept {
    ap.elapsed_44 = ap.elapsed_44 + dt;                    // 009CCEE6-009CCEF3
    const float countdown = ap.countdown_d0;
    if (dt < countdown) {                                  // 009CCF06 FCOMI, JB
        ap.countdown_d0 = countdown - dt;                  // 009CCF9F
        return;
    }
    ap.countdown_d0 = (ap.period_cc - dt) + countdown;     // 009CCF0E-009CCF16
    if (!in.has_target || in.target_disabled_5d) {         // 009CCF1C-009CCF3B
        ap.in_range_50 = false;                            // 009CD015
        return;
    }
    // 009CCF58-009CCF86: (unit y - aim y) / approach+30h + 2 * [+8]+188h. The
    // height is spilled as a double (009CCF60 FSTP qword) and the subtraction
    // runs in x87 precision before the float store at 009CCF86.
    const double climb = (static_cast<double>(in.unit_height_100) - in.aim_height) /
                         in.glide_tan_30;
    float threshold = static_cast<float>(
        climb + (static_cast<double>(in.field_188) + in.field_188));
    // 009CCF8A-009CCFE3: floored at tuning+658h times 1.0 ([00D7A24C]) or, for
    // a kind 10h/16h plane, 1.4 ([00D06874]).
    const float scale = in.not_bomber_kind ? 1.0f : 1.39999997615814208984375f;
    const float floor = in.attack_dist_658 * scale;
    if (!(threshold > floor)) threshold = floor;
    // 009CCFE9-009CCFF8: in range when the threshold exceeds 009CAD00.
    ap.in_range_50 = threshold > in.horizontal_distance;
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

bool point_off_nose_00996510(const float local[3]) noexcept {
    // BSP_Vector2f_ReciprocalLength over (x, z), then rz < |rx|.
    const double len2 = static_cast<double>(local[0]) * local[0] +
                        static_cast<double>(local[2]) * local[2];
    if (!(len2 > 0.0)) return false;
    const float rl = static_cast<float>(1.0 / std::sqrt(len2));
    return rl * local[2] < std::fabs(rl * local[0]);
}

bool point_distance_reached_00996300(float plane_x, float plane_z, float point_x,
                                     float point_z, float dist2) noexcept {
    // 0099631A-00996374: d2 = (point - plane+FCh/+104h) squared, stored float;
    // FCOMI against +28h: below it answers 0.
    const float dx = point_x - plane_x;
    const float dz = point_z - plane_z;
    const float d2 = static_cast<float>(static_cast<double>(dz) * dz + static_cast<double>(dx) * dx);
    return !(d2 < dist2);
}

}  // namespace bsp
