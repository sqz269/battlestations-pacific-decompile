#pragma once
// Pool packet cc13-pool02 (segment 33, 006EA290..006F8010): the bomb class name and kind test,
// the cargo world-list detach, three CommandBuilding routines (neutralize message, D5h
// capture-progress store, nearest free landing pad) and three constant virtual slots.
//
// Every body was read whole from the listing (bsp.py show --asm). Names are hypotheses, not
// recovered symbols. Objects are raw image storage at the image's offsets; image routines
// these bodies call are reached through Pool02Host, each method named by its address, and
// nothing is stubbed. New C++ interfaces, build-tested only: not ABI-compatible, not
// game-validated, and not wired into any host path. Live hosts already model three of these
// routines (see the per-function notes); nothing here replaces them.

#include <cstdint>

namespace bsp::pool_pool02_cc13 {

using Object = void*;

struct Pool02Host {
    virtual ~Pool02Host() = default;
    // 006DFFC0 BSP_UnitInstance_DetachFromMidWorldLists, __thiscall(ECX = unit), RET.
    virtual void unit_detach_from_mid_world_lists_006dffc0(Object unit) = 0;
    // 004837D0 BSP_UnitList_EraseNode, __thiscall(ECX = list, node), RET 4.
    virtual void unit_list_erase_node_004837d0(Object list, Object node) = 0;
    // 0075B430 BSP_SessionMessage_ConstructBase, __thiscall(ECX = message, type), RET 4.
    virtual void session_message_construct_base_0075b430(Object message, std::int32_t type) = 0;
    // 0077C2A0 BSP_Session_RouteMessage, __thiscall(ECX = entity, message, a, b), RET 0Ch.
    virtual void session_route_message_0077c2a0(Object entity, Object message, std::int32_t a,
                                                std::int32_t b) = 0;
    // [00E188A8]: the game object (local player slot +18ECh, slot players +18CCh[8]).
    virtual Object game_00e188a8() = 0;
    // 00977DA0 BSP_WarningManager_ReportCaptureShip, __thiscall(ECX = [00F8A0C4], building,
    // flag), RET 8.
    virtual void warning_report_capture_ship_00977da0(Object building, std::int32_t flag) = 0;
    // [00CE2218] EnterCriticalSection / [00CE2210] LeaveCriticalSection (IAT, __stdcall).
    virtual void enter_critical_section(Object section) = 0;
    virtual void leave_critical_section(Object section) = 0;
    // 00414DB0 BSP_EntityPose_RefreshWorld, __thiscall(ECX = entity), RET.
    virtual void entity_pose_refresh_world_00414db0(Object entity) = 0;
    // 00F87574: the static zero vector (three floats).
    virtual const float* static_vector_00f87574() = 0;
    // 006AC220, __fastcall(ECX = pad) -> EAX: the pad's occupant (null when free).
    virtual Object landing_pad_occupant_006ac220(Object pad) = 0;
    // 006AC490, __thiscall(ECX = pad, ship), RET 4: give the pad to the ship.
    virtual void landing_pad_set_occupant_006ac490(Object pad, Object ship) = 0;
};

// 006EA290 BSP_BombClass_GetName: MOV EAX,00CFA4D4 / RET (6 bytes). __cdecl/none -> EAX =
// "MBomb" (bytes 4D 42 6F 6D 62 00 at 00CFA4D4). Class-table slot; no direct callers.
const char* bomb_class_get_name_006ea290() noexcept;
// 006EA2A0 BSP_BombClass_IsKindOf: (kind) [ESP+4], RET 4 -> EAX = 1 for 2Ah or 29h, else 0
// (9 instructions). Bomb kind 2Ah inherits ordnance kind 29h.
bool bomb_class_is_kind_of_006ea2a0(std::int32_t kind) noexcept;
// 006EB460 BSP_Cargo_DetachFromWorldLists, __thiscall(ECX = cargo), RET (20 instructions):
// 006DFFC0 on the unit; then the list at [+30h]+9Ch, whose head is at [+30h]+0A0h, is walked
// by +4h until a node holds the cargo at +8h, and that node is erased (004837D0, ECX = list).
// No match, or an empty list, does nothing. 006EB47D..006EB47F is the 3-byte LEA ECX,[ECX+0]
// alignment filler (8D 49 00) behind a JMP. Host overlap: game_hosts_units.cpp models the
// cargo detach in its own unit lists; this body does not replace it.
void cargo_detach_from_world_lists_006eb460(Object cargo, Pool02Host& host);
// 006F2940 BSP_CommandBuilding_SendNeutralize, __thiscall(ECX = building), RET (31
// instructions; SEH 00C83288, state 0 once the message is built, no destructor call). +7A8h
// (capture progress) = 0.0f first; then a building whose party +54h is 2 (neutral) returns.
// Otherwise a 24h stack message: 0075B430(D3h), +18h word 0, +4h = 1, +1Ah byte 0, vtable
// 00CFADBC, +1Ch = 9 (the neutralize slot), +20h byte 0, routed with 0077C2A0(building,
// &message, 7, 0). Host overlap: command_building_capture.cpp's
// command_building_neutralize_006f3270 models the effect of this route (the D3h slot-9 arm);
// this body only builds and routes the message.
void command_building_send_neutralize_006f2940(Object building, Pool02Host& host);
// 006F29D0 BSP_CommandBuilding_HandleMessageD5, __thiscall(ECX = building, float progress,
// value, side), RET 0Ch (29 instructions). When side equals the local player's side
// ([game +18CCh + 4 * game +18ECh] +28h), the stored progress +7A8h is 0.0, the new progress is
// not 0.0 (a NaN counts as not 0.0: UCOMISS / LAHF / TEST AH,44h / JNP) and the party +54h is
// 2, 00977DA0(building, 0) reports the capture. Then +7ACh = side, +7A8h = progress, +7A4h =
// value. A NaN stored progress skips the report (JP). Host overlap: cc13_seg33b's D5h host
// call and game_hosts_ship_ai.cpp's capture tick store these fields today.
void command_building_handle_message_d5_006f29d0(Object building, float progress, std::uint32_t value,
                                                 std::int32_t side, Pool02Host& host);
// 006F2A50 BSP_CommandBuilding_TakeLandingPad, __thiscall(ECX = site, ship), RET 4 -> EAX = the
// pad or null (107 instructions). No pads (+798h == 0) -> null without locking. Otherwise the
// critical section [+764h] is entered and its depth +18h raised. The reference point is the
// ship's world position +0FCh (00414DB0 first when its pose byte +0C8h is 0), or 00F87574 for a
// null ship. Pads are the +794h pointer array of +798h entries, the count re-read each step.
// A pad with an occupant (006AC220) is skipped; with a null ship the first free pad is taken;
// otherwise each free pad's position (00414DB0 when +0C8h is 0) gives d = pad - ref, each
// component FSTP float, and d.y*d.y + d.x*d.x + d.z*d.z in x87 order ((x^2 + y^2) + z^2 after
// the stack shuffle) FSTP float; a pad replaces the best when best > dist (FCOMIP / JBE, so
// ties keep the first and NaN never wins), best starting at FLT_MAX (00D7A248). The chosen pad
// gets 006AC490(pad, ship); the depth is lowered and the section left. Precision: x87 under
// the main thread's 24-bit precision control, modelled as one float rounding per operation;
// PROVISIONAL if a fixed-step pool worker (53-bit) ever runs it. Host overlap: the landing AI
// in game_hosts_ship_ai.cpp (0074A540 site) chooses pads with its own model; not replaced.
Object command_building_take_landing_pad_006f2a50(Object site, Object ship, Pool02Host& host);
// 006F57D0 BSP_LandFort_ShowsOnMinimap_False: XOR AL,AL / RET -> AL = 0 (2 instructions). The
// minimap "never" body (hud_minimap.hpp kHudMinimapShowsFortBody).
bool land_fort_shows_on_minimap_006f57d0() noexcept;
// 006F7F50 BSP_CommandClass_BaseFourthConstant: XOR EAX,EAX / RET -> 0. The base command
// class's vtable slot +10h (gameplay_seg33_cc13.hpp kCommandClassSlot10).
std::int32_t command_class_base_slot10_006f7f50() noexcept;
// 006F8010 BSP_CommandClass_SetTarget_Name: MOV EAX,00CF8B0C / RET -> "settarget" (bytes at
// 00CF8B0C). The settarget command class's name slot.
const char* command_class_settarget_name_006f8010() noexcept;

} // namespace bsp::pool_pool02_cc13
