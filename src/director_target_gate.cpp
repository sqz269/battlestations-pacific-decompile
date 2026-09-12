// The auto-target hold timer at director+40h and the two routines that read it.
// Evidence: docs/DIRECTOR_TARGET_GATE.md. Packet cc2_director_target_gate.
#include "bsp/director_target_gate.hpp"

namespace bsp {

bool auto_target_hold_active(float hold) noexcept {
    // 0071DF75 COMISS XMM0,[00D7A218]; 0071DF7C JBE 0071DF81. The jump to the
    // slot scan is taken on less-than, equal and unordered, so only a strictly
    // greater value falls through to 0071DF7E XOR AL,AL.
    return hold > kAutoTargetHoldThreshold;
}

bool command_category_owns_fire_target(int category) noexcept {
    // 0071DFB0 CMP EAX,1 / 0071DFB5 CMP EAX,2, both jumping to the reject tail.
    return category == static_cast<int>(DirectorCommandCategory::kGunnery) ||
           category == static_cast<int>(DirectorCommandCategory::kWeaponRun);
}

int occupied_command_slot_prefix(
    const CommandSlot (&slots)[kDirectorCommandSlotCount]) noexcept {
    // 0071DF88..0071DF9E: EAX walks +54h in 1Ch steps, ESI counts, and the
    // CMP dword ptr [EAX],0 at 0071DF90 ends the walk at the first empty slot.
    int count = 0;
    while (count < kDirectorCommandSlotCount && slots[count].command != 0) {
        ++count;
    }
    return count;
}

bool director_accepts_new_target(
    float hold, const CommandSlot (&slots)[kDirectorCommandSlotCount],
    const int (&categories)[kDirectorCommandSlotCount]) noexcept {
    if (auto_target_hold_active(hold)) {
        return false;
    }
    const int occupied = occupied_command_slot_prefix(slots);
    for (int i = 0; i < occupied; ++i) {
        if (command_category_owns_fire_target(categories[i])) {
            return false;
        }
    }
    // 0071DFC6 MOV AL,1.
    return true;
}

float update_auto_target_hold(float hold, float frame_delta) noexcept {
    // 0071F2FD COMISS XMM0,[00D7A218]; 0071F30A JB 0071F317 skips the x87
    // subtract at 0071F30C..0071F314 for a value below zero (and for a NaN,
    // which sets CF). Everything else counts down by the frame delta.
    if (hold < kAutoTargetHoldThreshold) {
        return hold;
    }
    return hold - frame_delta;
}

bool may_send_override_command(float hold) noexcept {
    // 0071D995 XORPS XMM0,XMM0; 0071D99E COMISS XMM0,[ESI+40h]; 0071D9A2 JBE
    // skips the send. The compare is zero against the hold, so the send needs
    // 0.0f > hold.
    return kAutoTargetHoldThreshold > hold;
}

bool director_accepts_new_target(
    float hold, const CommandSlot (&slots)[kDirectorCommandSlotCount],
    DirectorTargetGateHost& host) {
    if (auto_target_hold_active(hold)) {
        return false;
    }
    const int occupied = occupied_command_slot_prefix(slots);
    for (int i = 0; i < occupied; ++i) {
        if (command_category_owns_fire_target(host.command_category(slots[i].command))) {
            return false;
        }
    }
    return true;
}

bool send_override_command(const DirectorOverrideCommandRequest& request,
                           DirectorOverrideCommandHost& host) {
    if (!may_send_override_command(request.auto_target_hold)) {
        return false;
    }
    const std::uint32_t message = host.build_set_command_message(
        request.command, request.command_target, kSetCommandMessageOverride);
    host.route_message(request.session, message, kDirectorRouteFlags, 0);
    return true;
}

} // namespace bsp
