// The squadron rules the AI path depends on. docs/PLANE_SQUADRON_ENTITY.md
// carries the listing for every address named here.

#include "bsp/plane_squadron_entity.hpp"

namespace bsp {

int plane_squadron_wing_count_007f4754(bool present, int authored) noexcept {
    // 007F4735: 008F2260 returned 0 for `WingCount`, so the immediate 3 at
    // 007F473B is what reaches +3C8h.
    if (!present) return kPlaneSquadronDefaultWingCount;
    // 007F4754 reads prop+0Ch and 007F4760..007F4770 clamps it up to 1.
    return authored < 1 ? 1 : authored;
}

bool plane_squadron_attach_plane_007f4b43(PlaneSquadronEntity& squadron, void* plane,
                                          int* spawn_index) noexcept {
    if (plane == nullptr) return false;
    if (squadron.live_count < 0) return false;
    // 007F4B55 MOV [ESI+EDI*4+3D0h],EAX has no bound test; the authored
    // WingCount enum (" 1".." 5") is what keeps a sixth wing unreachable. This
    // process refuses instead of writing over +3E4h.
    if (static_cast<std::size_t>(squadron.live_count) >= kPlaneSquadronMaxWings) return false;
    // 007F4B43: plane+9D8h = the CURRENT +3CCh, a spawn-order id.
    if (spawn_index != nullptr) *spawn_index = squadron.live_count;
    squadron.members[static_cast<std::size_t>(squadron.live_count)] = plane;  // 007F4B55
    squadron.live_count += 1;                                                 // 007F4B60
    squadron.dirty = true;                                                    // 007F4B6E
    return true;
}

void* plane_squadron_flight_leader(const PlaneSquadronEntity& squadron) noexcept {
    // 007EDA91 MOV ESI,[ECX+3D0h]: slot 0, whatever the live count.
    return squadron.live_count > 0 ? squadron.members[0] : nullptr;
}

bool plane_squadron_promote_flight_leader_007ed610(PlaneSquadronEntity& squadron,
                                                   int index) noexcept {
    // 007ED614 TEST EDX,EDX / JLE and 007ED618 CMP [ECX+3CCh],EDX / JLE.
    if (index <= 0 || index >= squadron.live_count) return false;
    if (static_cast<std::size_t>(index) >= kPlaneSquadronMaxWings) return false;
    // 007ED621 saves members[index], 007ED630..007ED63D shifts the prefix up
    // one slot from the top down, 007ED63F writes the saved plane into slot 0.
    void* promoted = squadron.members[static_cast<std::size_t>(index)];
    for (int i = index; i > 0; --i) {
        squadron.members[static_cast<std::size_t>(i)] =
            squadron.members[static_cast<std::size_t>(i - 1)];
    }
    squadron.members[0] = promoted;
    // 007ED645 CALL 007ED260 on the squadron; contract: unread.
    return true;
}

bool plane_squadron_remove_plane_007f39ed(PlaneSquadronEntity& squadron,
                                          void* plane) noexcept {
    if (plane == nullptr || squadron.live_count <= 0) return false;
    int slot = -1;
    for (int i = 0; i < squadron.live_count; ++i) {
        if (squadron.members[static_cast<std::size_t>(i)] == plane) { slot = i; break; }
    }
    if (slot < 0) return false;
    // 007F39D0's memmove of the tail towards the removed slot.
    for (int i = slot; i + 1 < squadron.live_count; ++i) {
        squadron.members[static_cast<std::size_t>(i)] =
            squadron.members[static_cast<std::size_t>(i + 1)];
    }
    squadron.live_count -= 1;                    // 007F39ED ADD [ESI+3CCh],-1
    squadron.members[static_cast<std::size_t>(squadron.live_count)] = nullptr;  // 007F39FA
    squadron.dirty = true;                       // 007F3A1D
    return true;
}

bool plane_squadron_excluded_007eda90(const PlaneSquadronLeadPlaneFacts& lead) noexcept {
    if (!lead.has_lead_plane) return false;       // 007EDA99 JZ
    if (!lead.lead_is_kamikaze_17) return false;  // 007EDAA8 JZ
    if (lead.lead_pilot_fires_0c24) return false; // 007EDAB1 JNZ
    return true;                                  // 007EDAB3 MOV AL,1
}

AiGroupableCombatantFacts plane_squadron_combatant_facts(
    const PlaneSquadronLeadPlaneFacts& lead) noexcept {
    AiGroupableCombatantFacts facts;
    // 009FE088 PUSH 18h answers true for every squadron, so 009FE0A0's
    // ship-base tail is never reached and is_ship_base stays false.
    facts.is_plane_squadron = true;
    facts.is_ship_base = false;
    facts.squadron_has_carrier = lead.has_lead_plane;
    facts.squadron_carrier_is_kind_17 = lead.lead_is_kamikaze_17;
    facts.squadron_carrier_flag_0c24 = lead.lead_pilot_fires_0c24;
    return facts;
}

std::size_t plane_squadron_broadcast_to_members_007ecf80(
    const PlaneSquadronEntity& squadron,
    void (*visit)(void* member, void* context), void* context) noexcept {
    // 007ECF86 CMP [EBX+3CCh],ESI / 007ECF8C JLE: nothing to do at zero.
    if (visit == nullptr || squadron.live_count <= 0) return 0;
    std::size_t reached = 0;
    const int live = squadron.live_count < static_cast<int>(kPlaneSquadronMaxWings)
        ? squadron.live_count : static_cast<int>(kPlaneSquadronMaxWings);
    // 007ECFA0..007ECFB9: EDI walks &members[0] upwards, ESI counts against
    // +3CCh re-read every iteration.
    for (int i = 0; i < live; ++i) {
        void* member = squadron.members[static_cast<std::size_t>(i)];
        if (member == nullptr) continue;
        visit(member, context);
        ++reached;
    }
    return reached;
}

}  // namespace bsp
