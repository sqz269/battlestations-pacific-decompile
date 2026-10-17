# CC13 fix13: GetCommandBuildingLevel and SetCommandBuildingOwnerPlayer

Packet cc13-fix13 (resumed as cc13-fix13b, 2026-10-17; grant from the cc9 gameplay lead).
Two Lua natives that the host recorded as UNIMPLEMENTED and answered with nothing now have
host bodies, each behind its own switch. Both switches are OFF in this commit: with them off
`GameScriptOrdersHost::handles` returns false for the two names and behaviour is unchanged.

| Switch | File | Default |
| --- | --- | --- |
| `kLuaGetCommandBuildingLevelBound` | include/bsp/game_hosts_script_orders.hpp | false |
| `kLuaSetCommandBuildingOwnerPlayerBound` | include/bsp/game_hosts_script_orders.hpp | false |

## GetCommandBuildingLevel 0089B6D0

Binary: `__fastcall`, ECX the Lua state, plain RET. 0089B7CF 00888AA0 resolves argument 0 to
an entity; 0089B7E6 `MOV ESI,[ESI+770h]` with no null or class test; 0089B7F1 00B664B0
PushInteger; one result.

Host (`GameScriptOrdersHost::dispatch`, src/game_hosts_script_orders.cpp): the level comes from
`GameShipAiHost::command_building_level_0770`, which returns the capture record's `level_770`
(seeded from the scene `Level` by 006F2780 at build, moved only by 006F38E0's level-ups and
the flip's D4h level 0). Before the capture records are built it falls back to
`GameUnitsHost::command_building_level_0770` (the same scene value; 0 for a unit that is not a
CommandBuilding). No new field.

Labelled substitution: an argument with no units-host slot pushes 0 and counts as unresolved;
the image would read +770h of whatever object 00888AA0 returned.

## SetCommandBuildingOwnerPlayer 008A8D90

Binary: `__fastcall`, ECX the Lua state, plain RET, no Lua result. 00888AA0 on argument 0,
008A8EC1 00B66290 argument 1 as an integer (the player slot), 008A8EE1 vtable[5Ch](1Ch) (is a
CommandBuilding). When true: message D3h built at 008A8EE7..008A8F13 (vtable 00CFADBC, +4h = 1,
word +18h = 0, byte +1Ah = 0, +1Ch = player, byte +20h = 1) and routed by 008A8F2A
0077C2A0(&message, 7, 0) with ECX the entity. The building's dispatcher 006F5460 loads
the message's +1Ch and byte +20h at 006F549E/006F54A2 and passes them at 006F54A9 to
006F4D10.

006F4D10 with byte 1: +7A8h = 0, +528h takes the slot if -1, the cblost / cbneutralized
announcements are skipped (they post only for byte 0). Slot 8 or 9: party 2 (neutral). Other
slots: party = [[game+18CCh]+slot*4]+28h through vtable[2Ch], the retake test on +7B0h/+7B4h,
the 006F47F0 repair, then D4h with level 0.

Host: `GameShipAiHost::command_building_set_owner_player_006f4d10` (src/game_hosts_ship_ai.cpp)
calls the existing `bsp::command_building_flip_006f4d10` and then the capture tick's own flip
effects, now factored into `Impl::apply_capture_flip_006f4d10` (side change, party mirror,
repair, D4h level 0). The capture tick calls the same helper; its behaviour is unchanged.

Labelled substitutions: the slot record's Party is `scene_slot_party(player)` under
`kLocalPartyFromSceneBound` (slot = party with it off), as in the capture tick; a player
outside 0..9 or a slot with no published party does nothing (the image indexes past the eight
slot pointers); the message is applied at the call rather than through the session queue; the
prior slot is -1, so the retake exemption never applies (as in the tick); a unit that is not
one of the host's kind-1Ch capture buildings does nothing (the vtable[5Ch](1Ch) test); nothing
happens while `kCommandBuildingCaptureBound` is off.

Not modelled (same as the capture tick's flip): the slot-8/9 arm's float +7D8h =
[+538h]+190h (006F50A5) and game-mode-2 +7B0h record, vtable[144h] and the garrison loop
(006F50CC..), the HUD selected-unit refresh, the warning manager's capture-point report,
0095D3C0 and the flag node.

No image RNG draw in either native or in 006F4D10's modelled path.

## Missions

- GetCommandBuildingLevel: LIVE. PRCPIJN/jm05.lua:2779, COTP-IJN/jm05.lua:2596,
  ijn/JM/jm05.lua:2582 `Mission.RadarStation.Party == PARTY_JAPANESE and
  GetCommandBuildingLevel(Mission.RadarStation) >= 2` in luaJM5CheckObjectives. Today the nil
  result raises once the RadarStation is Japanese and aborts the pass.
- SetCommandBuildingOwnerPlayer: present in the same three files (PRCPIJN :1262/:1263/:1283,
  COTP-IJN :1157/:1158/:1178, ijn/JM :1143/:1144/:1164) but only under `this.Cheat_PhaseShift`,
  so it does not run in a normal mission.

## Pair prediction (switch on vs off, JM05)

With `kLuaGetCommandBuildingLevelBound` on, the comparison returns a number instead of raising,
so luaJM5CheckObjectives (PRCPIJN :2676) completes its pass after the RadarStation turns
Japanese: once the station's level reaches 2 the "Fortressbuilt" hint shows, secondary 1 is
added (luaJM5AddObjS1, :2789) and later completed when luaJM5DisabledBunkers() holds (:2795),
and the primary 1 / 2 HiddenFailed checks after it (:2803..:2816) now run every pass. While the
station's level stays below 2 only the abort disappears. Expect the
JM05 log's Lua error lines for that compare to disappear and objective state changes after
the capture. `kLuaSetCommandBuildingOwnerPlayerBound` should leave a normal JM05 run unchanged
(cheat-only callers). Not run: no game runs in this packet.

## Counters

`summary mission script command building level ...` in the script-orders report: level calls
and unresolved, owner calls and applied.
