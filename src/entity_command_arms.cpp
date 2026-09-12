// The arm cascade of 00816E30, 00816EA6 to 0081732E. Addresses, ABI, coverage
// and uncertainty: docs/ENTITY_COMMAND_ARMS.md. Names are hypotheses, not
// recovered symbols.

#include "bsp/entity_command_arms.hpp"

namespace bsp {

std::uint32_t entity_path_interface_offset_007ac9d0(bool entity_present,
                                                    EntityCommandKindHost& host) {
    if (!entity_present) {
        return 0u;                                          // 007AC9D5
    }
    if (host.entity_is_kind_of_vtable5c(0x47)) {             // 007AC9DE
        return 0x1E4u;                                       // 007AC9E4
    }
    if (host.entity_is_kind_of_vtable5c(0x48)) {             // 007AC9F5
        return 0x170u;                                       // 007AC9FB
    }
    if (host.entity_is_kind_of_vtable5c(0x49)) {             // 007ACA0C
        return 0x310u;                                       // 007ACA12
    }
    if (host.entity_is_kind_of_vtable5c(0x4A)) {             // 007ACA21
        return 0x1E4u;                                       // 007ACA27, the 47h offset again
    }
    return 0u;                                               // 007ACA29
}

namespace {

// 00816EAF..00816ED0 and 00816F01..00816F22 are the same three instructions:
// resolve the descriptor's target, and drop it unless it answers kind 2.
std::uint32_t resolve_kind2_target(EntityCommandArmsHost& host) {
    const std::uint32_t target = host.resolve_target_00521ea0();
    if (target == 0u || !host.target_is_kind_of_vtable5c(target, 2)) {
        return 0u;
    }
    return target;
}

}  // namespace

EntityCommandArmDecision entity_command_arm_cascade_00816ea6(
    EntityCommandArmId command, const SceneCommandTarget& descriptor,
    EntityCommandArmsHost& host) {
    EntityCommandArmDecision out{};
    out.command = command;

    switch (command) {
    case EntityCommandArmId::Follow: {                         // 00816EA6
        const std::uint32_t target = resolve_kind2_target(host);
        host.request_join_formation_0077c8d0(target);          // 00816ED5
        host.call_0064a8e0();                                  // 00816EE2
        out.result = EntityCommandArmResult::HandledWithoutQueueing;
        return out;
    }
    case EntityCommandArmId::Leave: {                          // 00816EFC
        const std::uint32_t target = resolve_kind2_target(host);
        host.call_0077c980(target);                            // 00816F27
        out.result = EntityCommandArmResult::HandledWithoutQueueing;
        return out;
    }
    case EntityCommandArmId::Disband:                          // 00816F41
        host.call_0077ca60();                                  // 00816F4B
        out.result = EntityCommandArmResult::HandledWithoutQueueing;
        return out;

    case EntityCommandArmId::Cruise:                           // 00816F65
    case EntityCommandArmId::Stop:                             // 00816F71
    case EntityCommandArmId::MoveOnPath:                       // 00816F7D
        out.result = EntityCommandArmResult::IssueChecked;      // straight to 00817330
        return out;

    case EntityCommandArmId::MoveTo: {                         // 00816F89
        // A moveto aimed at an entity that offers the path interface becomes a
        // moveonpath. A targetless moveto, or one whose target offers no
        // interface, stays a moveto.
        if (descriptor.kind != 0) {                            // 00816F91
            const std::uint32_t target = host.resolve_target_00521ea0();   // 00816FA0
            if (host.path_interface_007ac9d0(target) != 0u) {   // 00816FA7
                out.command = EntityCommandArmId::MoveOnPath;   // 00816FB4
                out.result = EntityCommandArmResult::IssueUnchecked;  // 00816FB9
                return out;
            }
        }
        out.result = EntityCommandArmResult::IssueChecked;
        return out;
    }
    case EntityCommandArmId::Land:                             // 00816FBE
        // A land order given to a unit that is not kind 0Ch becomes attackmove.
        if (!host.self_is_kind_of_vtable5c(0x0C)) {             // 00816FCF
            out.command = EntityCommandArmId::AttackMove;       // 00816FD9
            out.result = EntityCommandArmResult::IssueUnchecked; // 00816FDE
            return out;
        }
        out.result = EntityCommandArmResult::IssueChecked;
        return out;

    case EntityCommandArmId::SetTarget: {                      // 00816FE3
        const std::uint32_t target = host.resolve_target_00521ea0();   // 00816FF1
        host.set_fire_target_00835860(target, 1);              // 00816FFD, force 1
        out.result = EntityCommandArmResult::HandledWithoutQueueing;
        return out;
    }
    case EntityCommandArmId::ClearTarget:                      // 00817017
        // 00817023..008171BB is not projected; every path in it returns.
        host.clear_target_block_00817023();
        out.result = EntityCommandArmResult::HandledWithoutQueueing;
        return out;

    case EntityCommandArmId::ClearOrders:                      // 008171BD
        host.set_fire_target_00835860(0u, 1);                  // 008171CF
        if (host.controller_belongs_to_another_007788b0()) {    // 008171D6
            out.result = EntityCommandArmResult::HandledWithoutQueueing;
            return out;
        }
        host.send_clear_commands_0071d880();                   // 008171E9
        if (host.call_0080dc70()) {                            // 008171F6
            host.free_fire_0071bf20();                         // 00817205
        }
        out.result = EntityCommandArmResult::HandledWithoutQueueing;
        return out;

    case EntityCommandArmId::AttackMove:                       // 0081721F
    case EntityCommandArmId::Artillery: {                      // 00817227
        // Both become attackmove. With no entity target the routine
        // manufactures one at the descriptor's position and aims at that.
        out.command = EntityCommandArmId::AttackMove;          // 00817238
        out.result = EntityCommandArmResult::IssueUnchecked;
        if (descriptor.kind != 0) {                            // 00817233
            return out;                                        // 0081723D
        }
        const std::uint32_t memory =
            host.allocate_zeroed_00470b80(kEntityCommandThrowawayTargetSize);  // 00817248
        std::uint32_t entity = 0u;
        if (memory != 0u) {
            entity = host.construct_entity_004e5980(memory);   // 00817264
        }
        // 0081726B..008172E3 build a 4x4 identity on the stack from 00D7A24C
        // and a zeroed register; 008172E9 reads the session field.
        host.place_entity_vtable98(entity, host.session_field_19cc());  // 008172FF
        const std::uint32_t transform = host.transform_from_position_0059bd20();  // 0081730A
        host.set_entity_transform_006e8040(entity, transform);  // 00817312
        // 00817322 gives the throwaway entity its side before the descriptor
        // is rewritten to name it.
        host.set_descriptor_target_00464f70(entity, 0.0f);      // 00817329
        out.made_throwaway_target = true;
        return out;                                             // 0081732E
    }

    default:
        // Every command the cascade does not name reaches 00817330 unchanged,
        // including the null the ordinal lookup returns for an unknown message.
        out.result = EntityCommandArmResult::IssueChecked;
        return out;
    }
}

}  // namespace bsp
