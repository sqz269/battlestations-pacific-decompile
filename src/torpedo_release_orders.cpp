// The queued release-order count unit+C58h and the pilot control block's attack
// mode ctl+370h. Evidence in docs/TORPEDO_RELEASE_ORDERS.md; every address in a
// comment is the listing line the rule came from.

#include "bsp/torpedo_release_orders.hpp"

namespace bsp {

PilotAttackMode pilot_attack_mode_raise_007ed430(PilotAttackMode current,
                                                 PilotAttackMode requested) noexcept {
    // 007ED434 CMP [ECX+370h],EAX / 007ED43A JGE skips the store: the setter
    // only ever moves the mode up.
    return static_cast<int>(current) < static_cast<int>(requested) ? requested : current;
}

PilotAttackMode pilot_attack_mode_0099b740(PilotAttackMode current,
                                           const PilotAttackModeInputs& in) noexcept {
    // 0099B749, 0099B755, 0099B75D: three tests, each falling through to the
    // return without touching the mode.
    if (!in.has_control_block_2fc) return current;
    if (!in.has_unit_2f4) return current;
    if (!in.unit_is_flight_lead) return current;
    // 0099B766 task->vtable[38h](); 0099B76A JZ skips.
    if (!in.task_authorises_38h) return current;
    // 0099B772 PUSH 1 / 0099B774 CALL 007ED3F0 with ECX = task->+2FCh. The
    // plain setter, so this is an assignment, not a raise: a block sitting at
    // kForced is pulled back down to kAttack by the next lead tick.
    return PilotAttackMode::kAttack;
}

bool unit_can_drop_ordnance_007b9140(const CanDropOrdnanceInputs& in) noexcept {
    // 007B914A unit->vtable[5Ch](17h): a kamikaze plane answers on its own
    // class and short-circuits at 007B9150 with AL = 1.
    if (in.unit_is_kind_17h) return true;
    // 007B915B-007B918F: walk the device array at unit+974h, count unit+994h,
    // stride 4, and ask each for bomb-family ordnance. The torpedo descriptor
    // answers 2Ah as well as 2Bh, so a torpedo bomber passes here.
    if (in.device_holds_2ah == nullptr) return false;
    for (int i = 0; i < in.device_count_994; ++i) {
        if (in.device_holds_2ah[i]) return true;   // 007B9181 JNZ 007B919A
    }
    return false;                                   // 007B9194 XOR AL,AL
}

int release_order_count_007bcbe0(const ReleaseOrderSetInputs& in) noexcept {
    // 007BCBE6 TEST EDI,EDI / 007BCBEA JLE, 007BCBEC and 007BCBFB: three
    // guards, and every failure lands on the same store of zero at 007BCC09
    // rather than leaving the field alone.
    if (in.requested_count <= 0) return 0;
    if (!in.scene_node_enabled_5c) return 0;
    if (!in.can_drop_ordnance) return 0;
    return in.requested_count;                      // 007BCBFD
}

bool release_orders_should_issue_007eef30(const ReleaseOrderIssueInputs& in) noexcept {
    // 007EEF4C FCOMIP ST0,ST1 with ST0 = ctl->+390h and ST1 = ctl->+374h;
    // 007EEF50 JBE skips the whole loop.
    if (!(in.authorise_value_390 > in.authorise_threshold_374)) return false;
    // 007EEF54 CMP [ESI+3CCh],EBX with EBX zeroed; 007EEF5A JLE skips.
    return in.controlled_count_3cc > 0;
}

bool release_order_issue_to_unit_007eef78(bool force_flag_378,
                                          bool unit_lacks_follow_target_007b8ad0) noexcept {
    // 007EEF62 CMP byte [ESI+378h],0 / 007EEF6B JNZ goes straight to the raise.
    if (force_flag_378) return true;
    // 007EEF6F 007B8AD0 / 007EEF76 JNZ skips the raise, so a unit that LACKS a
    // follow target is passed over.
    return !unit_lacks_follow_target_007b8ad0;
}

ReleaseOrderClearResult release_orders_clear_007ed3c0(int controlled_count_3cc) noexcept {
    ReleaseOrderClearResult out;
    // 007ED3C7 MOV byte [ECX+378h],0 runs before the branch.
    out.clear_force_flag_378 = true;
    // 007ED3CE JLE skips; only the first element of the array is touched.
    out.write_lead_count = controlled_count_3cc > 0;
    out.lead_count = 0;   // 007ED3D6 PUSH 0
    return out;
}

ReleaseOrderSpendResult release_order_spend_0099af53(
    const ReleaseOrderSpendInputs& in) noexcept {
    ReleaseOrderSpendResult out;
    out.count_c58 = in.count_c58;
    // 0099AF53 CMP dword [EAX+C58h],0 / 0099AF5A JLE leaves the field alone and
    // jumps past the clear, so a count already at zero is not rewritten.
    if (in.count_c58 <= 0) return out;
    // 0099AF67, 0099AF74, 0099AF7F: the three guards, all landing on 0099AFC2.
    if (!in.manual_release_requested_72c || !in.scene_node_enabled_5c ||
        !in.can_drop_ordnance) {
        out.count_c58 = 0;
        out.cleared = true;
        return out;
    }
    // 0099AF81-0099AFAF: the loop over the bot's task vector.
    out.offered = true;
    if (in.a_task_took_the_order) {
        out.count_c58 = in.count_c58 - 1;   // 0099AFB6 ADD [EAX+C58h],-1
    }
    return out;
}

ReleaseOrderIssueResult torpedo_issue_release_orders_007c0d90(
    TorpedoReleaseOrderHost& host) {
    ReleaseOrderIssueResult out;

    // 007C0D9A-007C0ED3: the walk over the linked list at unit+48h, following
    // +44h. It latches a byte on the stack when it finds a device of class 25h
    // holding bomb-family ordnance whose descriptor answers 2Ch, 2Bh or 33h.
    out.walk_found_device = host.unit_carries_droppable_device_007c0d90();
    if (!out.walk_found_device) return out;   // 007C0EDE JZ past the call

    host.set_release_pending_c25(true);       // 007C0EE2

    // 007C0EFA MOV ECX,[EBP+9D4h] / 007C0F00 PUSH EBP / 007C0F01 CALL 007EEF30:
    // the control block is the receiver and the unit the argument.
    host.pre_issue_hook_007ee7f0();           // 007EEF3B

    const ReleaseOrderIssueInputs gate = host.read_issue_inputs();
    out.gate_passed = release_orders_should_issue_007eef30(gate);
    if (!out.gate_passed) return out;

    const int count = host.controlled_unit_count();
    for (int i = 0; i < count; ++i) {
        const bool issue = release_order_issue_to_unit_007eef78(
            gate.force_flag_378, host.unit_lacks_follow_target_007b8ad0(i));
        if (!issue) continue;                 // 007EEF84, the loop advance
        const ReleaseOrderSetInputs set =
            host.read_set_inputs(i, kReleaseOrderCount_007eef78);
        const int written = release_order_count_007bcbe0(set);
        host.write_release_order_count(i, written);
        if (written > 0) {
            ++out.units_raised;
        } else {
            ++out.units_zeroed;
        }
    }
    return out;
}

}  // namespace bsp
