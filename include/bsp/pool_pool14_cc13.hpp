#pragma once
// Pool packet cc13-pool14 (segment 67, 009C7240..009EC770): the dive-bomb done/prepare bot
// state's enter and tick, the ship-AI path plan block constructor, the contact track
// constructor and the obstacle side-acceptance test.
//
// Every body was read whole from the listing (bsp.py show --asm). Names are hypotheses, not
// recovered symbols. Objects are raw image storage at the image's offsets, except the path
// plan block, which reuses ShipAiPathPlanBlock (bsp/ship_ai_path_planner.hpp). Image routines
// these bodies call go through Pool14Host, each method named by its address; nothing is
// stubbed. New C++ interfaces, build-tested only: not ABI-compatible, not game-validated, not
// wired into any host path. The live hosts that already project these routines are named per
// function; nothing here replaces them.

#include "bsp/ship_ai_path_planner.hpp"

#include <cstdint>

namespace bsp::pool_pool14_cc13 {

using Object = void*;

struct Pool14Host {
    virtual ~Pool14Host() = default;
    // 009BED80 BSP_BotStateFollow_Enter_Provisional, __thiscall(ECX = state), RET.
    virtual void bot_state_follow_enter_009bed80(Object state) = 0;
    // 009C1FD0 BSP_BotStateFollow_Tick, __thiscall(ECX = state, float dt), RET 4.
    virtual void bot_state_follow_tick_009c1fd0(Object state, float dt) = 0;
    // 00424C40 BSP_GameSettings_GetSingleton -> EAX.
    virtual Object game_settings_00424c40() = 0;
    // 006952A0 BSP_Observer_UnregisterPair / 00694A60 BSP_Observer_RegisterPair,
    // __fastcall(ECX = observed object, EDX = observer handle).
    virtual void observer_unregister_pair_006952a0(Object observed, Object handle) = 0;
    virtual void observer_register_pair_00694a60(Object observed, Object handle) = 0;
    // 009DC060 BSP_ShipAi_RefreshContactTrack_Provisional, __thiscall(ECX = track).
    virtual void ship_ai_refresh_contact_track_009dc060(Object track) = 0;
    // 0080E160 BSP_UnitInstance_WeaponDirectorDirect, __thiscall(ECX = unit) -> EAX director.
    virtual Object unit_weapon_director_0080e160(Object unit) = 0;
};

// 009C7240 BSP_BotStateDiveBombDonePrepare_Enter, __thiscall(ECX = state), RET (4
// instructions): +9Ch byte = 0, +98h = -1.0f (00D7A260), then JMP 009BED80 (the Follow base
// enter, same ECX, no stack arguments). Host overlap: game_hosts_units.cpp
// run_dive_bomb_done_prepare_enter_009c7240 projects this state for the live unit host
// without a state object; not replaced.
void dive_bomb_done_prepare_enter_009c7240(Object state, Pool14Host& host);
// 009C7270 BSP_BotStateDiveBombDone_Tick, __thiscall(ECX = state, float dt), RET 4 (5
// instructions): FLD / PUSH ECX / FSTP re-pushes dt unchanged (a float round trip is exact)
// and calls 009C1FD0 (the Follow tick). Host overlap: game_hosts_units.cpp
// run_dive_bomb_done_prepare_tick_009c7270; not replaced.
void dive_bomb_done_prepare_tick_009c7270(Object state, float dt, Pool14Host& host);
// 009D9CC0 BSP_ShipAiPathPlan_Init, __thiscall(ECX = plan), RET, EAX = plan (24
// instructions): vtable 00D2152C; +04h, +08h = 20.0f (00CE3930); +0Ch = 1600.0f (00D05A40);
// +10h = 2500.0f (00D20278); +14h = 0.5f (00CE3800); +18h, +2Ch = 0.0f; +1Ch, +20h, +24h,
// +34h, +38h, +3Ch, +48h = 0; +30h = 1.0e7f (00CF58EC). +28h, +40h, +44h and +4Ch..+64h are
// not written. The stores are made in image order to an existing ShipAiPathPlanBlock (its
// default member initialisers describe the same constructor but also zero the unwritten
// fields).
ShipAiPathPlanBlock& ship_ai_path_plan_construct_009d9cc0(ShipAiPathPlanBlock& plan) noexcept;
// 009EACA0 BSP_ShipAi_ContactTrackConstruct_Provisional, __thiscall(ECX = track, float a0,
// float a1, float a2, observed, float a4, float a5, BYTE kind), RET 1Ch, EAX = track (95
// instructions; SEH 00CB0AD6, state 0 after the handle at +34h, 1 after +4Ch). Two observer
// handles: +34h (vtable 00D21868) and +4Ch (vtable 00CF5C94), each {vtable, +4/+8/+0Ch = 0,
// +10h byte 1, +14h observed = 0}. +0h = a0; +4h = float(a1 * 0.5) (FLD float, FMUL double
// 00D7A280 = 0.5, FSTP float: exact); +0Ch = a2; +8h = a1; +1Ch = a5; +10h = a4; +64h byte = 0.
// +14h = float(settings value + 1.0) (FADD double 00D7A210, FSTP float), the value being
// settings +1F0h when kind != 0, else +194h. Then the observed object is attached to the +34h
// handle when kind != 0, else to the +4Ch handle: unchanged -> nothing; the old one (if any)
// unregistered (006952A0), the new one stored and, when non-null, registered (00694A60).
// Finally 009DC060 on the track. Host overlap: ship_ai_torpedo_response.cpp builds the torpedo
// arm (kind 1) as a semantic ShipAiTorpedoTrack inside ship_ai_admit_torpedo_track_009f0ad0;
// not replaced.
Object ship_ai_contact_track_construct_009eaca0(Object track, float a0, float a1, float a2, Object observed,
                                                float a4, float a5, std::uint8_t kind, Pool14Host& host);
// 009EC770 BSP_ShipAi_ObstacleSideAccepted, __thiscall(ECX = blk, other party), RET 4 -> AL
// (25 instructions). The three gates run lazily in image order: party filter blk+3F0h < 0
// (JL) -> false; 0080E160(blk+3FCh)+241h == 0 -> false (the director is fetched only after
// the filter passes); 00424C40()+4h == 0 -> false (fetched only after the director passes).
// The tail is the existing ship_ai_avoidance_party_accepted_009ec770 (ship_ai_avoidance_request
// .hpp), which this body reuses with the gates already evaluated.
bool ship_ai_obstacle_side_accepted_009ec770(Object blk, int other_party, Pool14Host& host);

} // namespace bsp::pool_pool14_cc13
