// Packet cc13-ver19. See include/bsp/aa_pick_ver_cc13.hpp and docs/CC13_VER_19.md.
#include "bsp/aa_pick_ver_cc13.hpp"

namespace bsp {

AaPickPolicies aa_pick_install_policies_008636a0(const AaPickPolicyInputs& in) noexcept {
    AaPickPolicies out;
    // 008636AD..008636F4: the director-backed provider needs all three answers;
    // otherwise 008636F8 finds this+5Ch empty and installs 00D0D314.
    if (in.unit_is_kind_2 && in.unit_has_director && in.director_answers_48h_2) {
        out.fire_target = AaPickFireTargetProvider::kDirector_00D0D324;
    }
    // 0086371E..00863743: a kind-8 unit (a submarine) gets 00D0D32C; otherwise
    // 00863746 finds this+60h empty and installs 00D0D31C.
    if (in.unit_is_kind_8) {
        out.gate = AaPickCategoryGate::kSubmarine_00D0D32C;
    }
    return out;
}

const void* aa_pick_director_fire_target_00863640(
    const AaPickDirectorFireTargetInputs& in) noexcept {
    // 00863656 TEST EDI,EDI; JNZ 0086368D: the director's own target wins.
    if (in.own != nullptr) return in.own;
    // 00863660 TEST ECX,ECX; JE: no unit behind the director.
    if (!in.director_has_unit) return nullptr;
    // 0086366B TEST ESI,ESI; JE: no formation leader.
    if (in.formation_leader == nullptr) return nullptr;
    // 0086367C JE: the leader must answer IsKindOf(6). No self test: a leader's
    // own null target falls through to its own null answer.
    if (!in.leader_is_ship) return nullptr;
    return in.leader_fire_target;
}

bool aa_pick_submarine_category_gate_008627a0(int category,
                                              bool above_deck_gun_depth_008527a0,
                                              bool shallow_enough_008527e0) noexcept {
    // 008627A7 CMP EAX,7; JNE.
    if (category == 7) return shallow_enough_008527e0;
    // 008627BE..008627DA: 1, 5, 6, 2, 3, 4 share 008527A0.
    if (category == 1 || category == 5 || category == 6 || category == 2 ||
        category == 3 || category == 4) {
        return above_deck_gun_depth_008527a0;
    }
    return false;   // 008627EA MOV AL,BL with BL = 0 from 008627A5
}

AaPickGunSlotResult aa_pick_gun_slot_pre_dispatch_00729bc0(
    const AaPickGunSlotInputs& in) noexcept {
    AaPickGunSlotResult out;
    // 00729BD4..00729BEC. The image asks the target before its null test, so a
    // kind-6 gun with a null target would fault; callers never pass one.
    out.ammo_index = (in.gun_weapon_kind == 6 && in.target_is_plane) ? 1 : 0;
    if (!in.target_present) return out;                            // 00729BF3
    if (in.target_is_kind_5 && in.target_side == 2) return out;    // 00729C13
    // 00729C81..00729C8A: FLD range; FXCH; FCOMIP; JA 00729D0A.
    if (in.gun_to_target_distance > in.ammo_max_range[out.ammo_index]) return out;
    out.reaches_dispatch = true;
    return out;
}

}  // namespace bsp
