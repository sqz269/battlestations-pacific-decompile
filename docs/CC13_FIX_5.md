# CC13 fix 5: hosting the unhosted script natives (cc13-fix5)

Packet cc13-fix5 (cc13 lineage, Claude, 2026-10-13). cc13-ver12's helpers listed Lua script natives that the host
does not serve. Each native here gets one OFF switch and one commit. cc9 pairs and flips them.

Status words follow AGENTS.md:
- **reconstructed** means the body was read from disk bytes and the Ghidra pseudocode;
- **build-tested** means `scripts/build.ps1` built it and ctest passed;
- nothing here has been game-run. Every switch is OFF, so main's behaviour is byte-for-byte unchanged.

## 0. Which WIN rows call these natives

The WIN rows (docs/WINS_TABLE.md) load these scripts in this installation:
- USN01: `usn/usn_1_marshall.lua`
- USN02: `usn/usn_2_java.lua`
- USN12: `usn/usn_12_augusta.lua`
- LOMP06 and LOMP10: `usn/LOMP/06_crucial_cargo.lua` and `usn/LOMP/10_san_jose.lua`
- BSM01: `bsm/bsm_01_stationed_at_pearl.lua`
- USN15: `COTP-USN/PRCPUS/usn_15_battle_of_surigao_strait.lua`
- USNOS: `usn/usn_ormoc.lua` and `usn/usn_sibuyan.lua`
- JM06: `jm06.lua` (all three copies)
- JM08: `COTP-IJN/PRCPIJN/prcpjm08.lua` (docs/CC13_JM08_HQ.md section 1)

USN01 is ConTBD1's mission, so it is `usn_1_marshall.lua`, not `COTP-USN/usn_01_battle_of_eastern_solomons.lua`.

A comment-aware grep of those files for every native in this packet (local/fix5_wingrep.py) finds one direct call:
`RemoveAllAirBasePlanes(Mission.Hosho)` at prcpjm08.lua:908.

The WIN scripts can also reach these natives through helpers in `scripts/global/commandhelpers.lua`
(local/fix5_helpers.py):
- `luaMessageHandler` (PlaneReloadBombPlatforms, ReloadTorpedoes): only behind the Ctrl+Alt+Shift cheat keys.
- `luaGetSquadronPlaneNum` (GetSquadronPlanes): called only from `luaCountUnits`, which nothing calls.
- `luaPlayerAirbaseManager` (LaunchAirBaseSlot): its init is commented out in chg_6_forcez.lua, its only caller.
- `luaRestorePlanesAndStocks` (RemoveAllAirBasePlanes) and `luaCheckpointSpawned` (SquadronSetHomeBase): checkpoint
  restore only. A WIN run does not load a checkpoint.
- The convoy helpers `luaStartConvoy` and `luaStopConvoy`: no WIN script calls them.

So every native here except RemoveAllAirBasePlanes is predicted **identical on every WIN row** when flipped ON.

## 1. RemoveAllAirBasePlanes 008975B0

Switch: `kLuaRemoveAllAirBasePlanesBound` (include/bsp/game_hosts_lua.hpp). Model:
`air_base_remove_all_stock_006c8840` and `air_base_remove_stock_006c7b10` (src/script_natives_fix5_cc13.cpp).

The image body, from disk bytes:
- 008976AF: 00888AA0 on argument 0.
- 008976CA: 006BCD20 with DL = 0 (008976C6), the air-ops block.
- 008976D1: 006C8840 on the block.
- 008976DA: 00B66400. There is no result.

006C8840 walks the stock list at block+44h in node order and calls 006C7B10(node+8h, node+0Ch) for each node. It then
unlinks every node, zeroes block+48h and frees the nodes.

006C7B10 is `__thiscall(block, class, count)`, RET 8. It does two things.
1. The first node of the class loses min(count, node count). 006C8840 passes the node's own count, so the node drops
   to zero.
2. Under block+0Ch's lock, while 006BF230(class).available is below zero, it empties a slot. The slot is the LAST one
   whose state is 1 or 5 and whose +4h is the class (the walk overwrites its pick).
   - +4h and +10h are zeroed, +8h = 0, +2Ch = 1, +30h = 0.
   - When +34h is set, +30h = 5.0 and +34h = 0.
   - The slot index is removed from the launch list at block+10h (006C7C66, 006C48F0).
   - Then the GUI notify 00696350 runs and the slot message 006BD520 goes to the peers (0077C7B0).
   - slot+28h is not touched.

After the loop it issues the stock notice 00984EB0.

LABELLED substitutions:
- The block is the deck registry's entry for the entity, as AddAirBaseStock does it.
- The live counts for launched slots come from the orders host's `air_ops_squadron_plane_count`.
- The notify, the peer message and the stock notice are records.

**JM08 prediction.** Today the host keeps the Hosho's authored stock, and the script's AddAirBaseStock adds to it:
150 -> 31, 158 -> 22, 162 -> 28 (CC13_JM08_HQ section 4, l50_w3_j8.log). ON, the stock becomes exactly
21 / 12 / 18. That is 10 fewer planes of each of the three classes, including the torpedo bombers.

Any Hosho slot in state 1 or 5 that holds one of the scene's classes when line 908 runs is also emptied. The deck tick
then refills it from the new stock, so the Hosho launches the same classes with fewer planes in reserve.

Every other WIN row is unaffected. RNG: neither the native nor its callees draw, so no stream changes.

## 3. StartLanding2 0089AC30

Switch: `kLuaStartLanding2Bound`.

The image body, from disk bytes:
- 0089AD2F: 00888AA0 on argument 0. There is no kind test.
- 0089AD48..0089AD50: `CALL [EDX+238h]` on the entity.
- 0089AD57: 00B664B0 pushes EAX as an integer. There is one result.

Slot 238h holds 0074A4C0 only in the MLandingShip vtable 00CFFA30 (its one `.rdata` copy is 00CFFC68). It holds
008206F0 in the eight other ship vtables (scan-bytes on both pointers). These are the receivers of the 94h
`MT_SHIP_STARTLANDING` message, and the ship AI host already models both:
- `landing_ship_request_landing_0074a4c0`
- `transport_launch_craft_008206f0`

The host calls them at the native, as 00A11B80's 94h routing already does.

LABELLED substitution: a non-ship argument, or one that does not resolve, pushes 0. Its slot 238h is another class's
virtual, which was not read.

**Prediction.** No WIN row calls it. Calls in this installation: `ijn/JM/jm08.lua` and `COTP-IJN/jm08.lua` (617,
653, 1549), jm05.lua and jm07.lua.

OFF, jm08.lua:653's `tonumber(debuginfo) < 0` compares nil, which raises a Lua error each pass. ON, the landing ships
and cargo transports begin their landings on the script's schedule.

RNG: each landing begun, and each landing craft launched, takes one `ship_ai_draw(0, 0.75)` through the host's
gunnery draws. That is 0074A990's landing-time draw, which shares the 00BD2F10 stream as the existing 94h path does.
Only the JM05, JM07 and JM08 (ijn and COTP-IJN copies) scripts draw more. No WIN row does.
