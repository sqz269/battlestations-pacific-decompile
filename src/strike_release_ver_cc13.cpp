// Packet cc13-ver6. Evidence and uncertainty: docs/CC13_VER_6.md and the
// header. Pure models of the release-order paths the host lacks.

#include "bsp/strike_release_ver_cc13.hpp"

namespace bsp {

BaseReleaseRequestResult bot_task_request_release_base_0099b6a0(
    bool has_squadron_2fc, bool unit_free_flight_72c) noexcept {
    BaseReleaseRequestResult out;
    // 0099B6A3 CMP [ESI+2FCh],0 / 0099B6AA JNE; the fall-through is XOR AL,AL.
    if (!has_squadron_2fc) return out;
    // 0099B6C3 TEST AL,AL / 0099B6C5 JE skips only the call, not the MOV AL,1.
    out.release_007bbba0 = unit_free_flight_72c;  // 0099B6CD
    out.returned_true = true;                     // 0099B6D2
    return out;
}

TorpedoOfferResult torpedo_offer_release_order_009d49a0(
    const TorpedoArmInputs& in, bool has_squadron_2fc,
    bool unit_free_flight_72c) noexcept {
    TorpedoOfferResult out;
    out.arm = torpedo_arm_drop_009d49a0(in);
    out.consumed = out.arm.consumed;
    // 009D49D7 MOV ECX,ESI / 009D49D9 POP ESI / 009D49DA JMP 0099B6A0: a tail
    // jump, so the caller at 0099AF9B sees 0099B6A0's AL.
    if (out.arm.fell_through_to_0099b6a0) {
        const BaseReleaseRequestResult base = bot_task_request_release_base_0099b6a0(
            has_squadron_2fc, unit_free_flight_72c);
        out.release_007bbba0 = base.release_007bbba0;
        out.consumed = base.returned_true;
    }
    return out;
}

bool release_orders_leader_block_007eef96(const LeaderBlockInputs& in) noexcept {
    if (!in.unit_present) return false;                         // 007EEF98 JE
    if (!in.unit_is_flight_leader) return false;                // 007EEFA3 JE
    if (in.attack_mode_370 == 2) return false;                  // 007EEFAC JE
    if (in.party_slot_1b0 == kNoPartySlot_007eefb4) return false;  // 007EEFB7 JE
    return !in.party_slot_ai_held;                              // 007EEFC3 JNE
}

LevelBombOfferResult level_bomb_offer_release_order_009b83d0(
    bool ordnance_4c6, bool has_unit_3fc, bool unit_free_flight_72c,
    bool in_prepare_610) noexcept {
    LevelBombOfferResult out;
    // 009B83DA, 009B83E4 and 009B83F5 all jump to 009B8411 MOV AL,1.
    if (!ordnance_4c6 || !has_unit_3fc || !unit_free_flight_72c) return out;
    out.queued_round_424 = true;              // 009B83F7 ADD [ESI+424h],1
    out.arm_prepare_009b8100 = in_prepare_610;  // 009B8404 CMP / 009B840C CALL
    return out;
}

}  // namespace bsp
