// Packet cc13-ver10: 0084E010's tail. docs/CC13_VER_10.md section 2.
#include "bsp/airops_ver_cc13.hpp"

namespace bsp {
namespace {
// 0084E4CC-0084E4DF: retreat, returntobase, land.
bool returning_command(std::uint32_t command) noexcept {
    return command == 0x00E08F90u || command == 0x00E08F98u || command == 0x00E08FA0u;
}
}  // namespace

SquadronStepTailPlan squadron_step_tail_plan_0084e4b1(const SquadronStepTailInputs& in) noexcept {
    SquadronStepTailPlan p;
    p.b4 = in.head_arm;                                                  // 0084E05C
    // 0084E4B1-0084E4E8: stage 2 with at most one queued command.
    if (in.stage_48 == 2 && in.filled_slots_71be60 <= 1) {
        if (!in.latch_3c && !returning_command(in.command_54)) p.b4 = true;   // 0084E4E1
        else p.b5 = true;                                                // 0084E4E8
    }
    if (in.override_stage_50 == 2 && in.latch_3c) p.b5 = true;          // 0084E4ED-0084E4FA
    // 0084E4FF-0084E52D: with ReloadEnabled clear, the idle arm, no command or a
    // command whose category is neither 1 nor 2 resolves; otherwise b5 decides.
    bool resolve = false;
    if (!in.reload_enabled_369) {
        if (p.b4 || in.command_54 == 0) {
            resolve = true;                                              // 0084E511, 0084E518
        } else if (in.command_category_0c != 1 && in.command_category_0c != 2) {
            resolve = true;                                              // 0084E527
        }
    }
    if (!resolve && p.b5) resolve = true;                                // 0084E529
    if (resolve) {
        // 0084E52F-0084E54B: both reload bytes set, or +368h, skip 007F16D0.
        const bool gated = (in.reload_enabled_369 && in.device_reload_e17bf2) || in.flag_368;
        p.outcome = gated ? SquadronStepTailOutcome::kMoveToSelfIfRoom
                          : SquadronStepTailOutcome::kResolve;
        return p;
    }
    // 0084E58F-0084E593: only the idle arm reaches the fallback from here.
    p.outcome = p.b4 ? SquadronStepTailOutcome::kMoveToSelfIfRoom
                     : SquadronStepTailOutcome::kNothing;
    return p;
}

bool squadron_step_moveto_self_0084e595(int filled_slots, int stage) noexcept {
    return filled_slots <= (stage == 2 ? 1 : 0);                         // 0084E59C-0084E5A6 JG
}

bool squadron_step_issues_moveto_self(const SquadronStepTailInputs& in,
                                      bool rtb_answer_nonnull) noexcept {
    const SquadronStepTailPlan p = squadron_step_tail_plan_0084e4b1(in);
    switch (p.outcome) {
        case SquadronStepTailOutcome::kNothing:
            return false;
        case SquadronStepTailOutcome::kResolve:
            if (rtb_answer_nonnull) return false;                        // 0084E557 issued
            [[fallthrough]];                                             // 0084E55B JE 0084E595
        case SquadronStepTailOutcome::kMoveToSelfIfRoom:
            return squadron_step_moveto_self_0084e595(in.filled_slots_71be60, in.stage_48);
    }
    return false;
}

}  // namespace bsp
