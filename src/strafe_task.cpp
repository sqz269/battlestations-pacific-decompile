// The strafe bot task's rule and approach range test. Evidence in
// docs/SQUADRON_LAND_TASK.md sections 5bw and 5bx; every address in a comment
// is a listing address.

#include "bsp/strafe_task.hpp"

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

}  // namespace bsp
