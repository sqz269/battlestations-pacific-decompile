#ifndef BSP_STRIKE_RELEASE_VER_CC13_HPP
#define BSP_STRIKE_RELEASE_VER_CC13_HPP

// Packet cc13-ver6: the release-order paths the host's strike tasks lack, read
// from the bytes for the strike-release verification (docs/CC13_VER_6.md).
//
// Every name is a hypothesis, not a recovered symbol. The structs are not
// binary layouts; the offsets in the comments are the native ones. These are
// pure models: nothing in the host calls them yet. Wiring them is cc9's call,
// because the callers live in src/game_hosts_units.cpp.
//
// The shared gate. (unit+72Ch)->vt[38h] is 0074E210
// BSP_PlaneControlMode_IsFreeFlight, `[ECX+1D4h] == 7`, that is unit+900h == 7.
// The MPlaneBomber constructor installs vtable 00D065B0 at unit+72Ch
// (007D776B), and that vtable's slot 38h (00D065E8) is 0074E210. An airborne AI
// aircraft is in mode 7, so the gate is NOT a player-control test.

#include "bsp/torpedo_release_orders.hpp"
#include "bsp/torpedo_task_arm.hpp"

namespace bsp {

// kLeaderBlockOrderCount_007eefdb and kNoPartySlot_007eefb4 live in
// torpedo_release_orders.hpp beside the host binding of the same block.

// ---------------------------------------------------------------------------
// 0099B6A0 BSP_BotTask_RequestOrdnanceReleaseBase, task vtable slot +24h in the
// base table 00D05708 and in 19 task vtables (scan for A0 B6 99 00 in .rdata).
// __thiscall(task) -> bool, RET 0.
//   0099B6A3  CMP [ESI+2FCh],0 / JNE     no squadron -> return 0
//   0099B6B6  (task+2F4h)+72Ch vt[38h]   free flight?
//   0099B6CD  CALL 007BBBA0(task+2F4h)   the bay release, when it is
//   0099B6D2  MOV AL,1                   returns 1 either way
// ---------------------------------------------------------------------------
struct BaseReleaseRequestResult {
    bool release_007bbba0 = false;  // 0099B6CD reached
    bool returned_true = false;     // AL at RET; 0099AFB6 spends the order on it
};
BaseReleaseRequestResult bot_task_request_release_base_0099b6a0(
    bool has_squadron_2fc, bool unit_free_flight_72c) noexcept;

// ---------------------------------------------------------------------------
// 009D49A0 as one offer, with its 009D49DA tail jump into 0099B6A0 resolved.
// torpedo_arm_drop_009d49a0 (torpedo_task_arm.cpp) stops at the jump and
// reports `consumed = false`; the image's AL is 0099B6A0's.
// ---------------------------------------------------------------------------
struct TorpedoOfferResult {
    TorpedoArmResult arm;           // the 009D49A0 arms, unchanged
    bool release_007bbba0 = false;  // the wingman drop through 0099B6CD
    bool consumed = false;          // AL at RET
};
TorpedoOfferResult torpedo_offer_release_order_009d49a0(
    const TorpedoArmInputs& in, bool has_squadron_2fc,
    bool unit_free_flight_72c) noexcept;

// ---------------------------------------------------------------------------
// 007EEF96-007EEFEE, the second block of 007EEF30
// BSP_PilotControl_IssueReleaseOrders. It runs after the 999 loop, or after
// that loop is skipped by the 390h/374h gate (007EEF50 JBE lands on 007EEF96):
//   007EEF96  TEST EBP,EBP              the issuing unit
//   007EEF9C  CALL 007B8AD0             the unit leads its flight
//   007EEFA5  CMP [ESI+370h],2 / JE     not attack mode 2
//   007EEFB4  CMP unit+1B0h,8 / JE      it has a party slot
//   007EEFBC  CALL 00927F10(unit+1B0h)  BSP_PartySlot_IsAiHeld, JNE skips
//   007EEFD5-007EEFEE                   007BCBE0(member, 1) for every member
//                                       of ctl+3D0h but the unit itself
// 007BCBE0 still applies its own 5Ch / 007B9140 gates per member.
// ---------------------------------------------------------------------------
struct LeaderBlockInputs {
    bool unit_present = false;            // EBP != 0
    bool unit_is_flight_leader = false;   // 007B8AD0
    int attack_mode_370 = 0;              // ctl+370h
    int party_slot_1b0 = kNoPartySlot_007eefb4;  // unit+1B0h
    bool party_slot_ai_held = true;       // 00927F10(unit+1B0h)
};
bool release_orders_leader_block_007eef96(const LeaderBlockInputs& in) noexcept;

// ---------------------------------------------------------------------------
// 009B83D0 BSP_BotTaskLevelBomb_OfferReleaseOrder, the level-bomb task's slot
// +24h (vtable 00D20210, slot read at 00D20234). __thiscall(task) -> bool.
//   009B83D3  CMP [ESI+4C6h],0 / JE       -> return 1
//   009B83DC  task+3FCh (the unit) null   -> return 1
//   009B83F1  (unit+72Ch) vt[38h] false   -> return 1
//   009B83F7  ADD [ESI+424h],1
//   009B8404  state == +610h (prepare)    -> CALL 009B8100, the prepare arm
//   009B8411  MOV AL,1
// The +424h it raises is spent by 009B8BD0-009B8C3D (007BBBA0 per tick in any
// state but aim 5F4h, attackrun 5CCh, release 6B4h and prepare 610h).
// ---------------------------------------------------------------------------
struct LevelBombOfferResult {
    bool queued_round_424 = false;     // 009B83F7
    bool arm_prepare_009b8100 = false; // 009B840C
    bool returned_true = true;         // every path leaves AL = 1
};
LevelBombOfferResult level_bomb_offer_release_order_009b83d0(
    bool ordnance_4c6, bool has_unit_3fc, bool unit_free_flight_72c,
    bool in_prepare_610) noexcept;

}  // namespace bsp

#endif  // BSP_STRIKE_RELEASE_VER_CC13_HPP
