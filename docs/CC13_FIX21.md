# CC13 fix21: GetSquadTorpedoes stopgap, GetSubmarineAirSupply, census rows, PilotBomb class

Packet cc13-fix21 (cc9 approved every item). Every new switch is committed OFF, and OFF is
unchanged: the two new binding rows are only reached through `GameScriptOrdersHost::handles`,
which refuses them while their switch is false. No game runs were made for this packet.

| Item | Address | Switch | State |
| --- | --- | --- | --- |
| GetSquadTorpedoes stopgap | 008BEF80 | `kLuaGetSquadTorpedoesBound` (new) | OFF, PARTIAL |
| GetSubmarineAirSupply | 008938E0 | `kLuaGetSubmarineAirSupplyBound` (new) | OFF |
| Census rows (GetLastCatapulted, GetFormationLeader) | 00892860, 00899AF0 | existing, ON | census only |
| PilotBomb squadron class | 008A4F00 | `kLuaPilotBombBound` (existing) | OFF |

Both new switches are in `include/bsp/game_hosts_script_orders.hpp`. The arms are in
`GameScriptOrdersHost::dispatch` (`src/game_hosts_script_orders.cpp`).

## 1. GetSquadTorpedoes 008BEF80 (PARTIAL stopgap)

**Image** (bytes and the existing reconstruction `lua_get_squad_torpedoes`,
`src/cc13_left55_seg56c.cpp`):

- Argument 0 goes through 00888AA0, with no null or class test.
- The flag is read through 00B66250 only when there are exactly two arguments (008BF0A7);
  otherwise it is 0.
- 008BF0E9 00B67930 NewTable is unconditional.
- It then walks `[game+19CCh]`: count `+21Ch`, node `+224h`, element `[node+8]`.
- An element qualifies when byte `+5Eh` is 0 and either `(e+310h)->vtable[38h]()` or the flag
  is set. The virtual call is made first.
- A qualifying element is added (00927B40 self object, 00B666C0 at keys 1, 2, ...) when one of
  the squadron's `+3CCh` planes at `+3D0h` equals `[e+3BCh]`. A slot above 4 reads 0.
- One result (00B66400). Every path answers a table:
  - a nil argument, or one that is not a squadron, still gets the table;
  - a null squadron faults only once an element qualifies, because `[sq+3CCh]` is read only
    then;
  - an argument that is not a squadron has its `+3CCh` / `+3D0h` read as whatever that class
    keeps there.

With the flag set, the table includes the rounds still on the racks. 006E3E70, the rack's
vtable[224h], builds each attached round as an entity and binds it to the rack's plane
(006E6BB0).

**Host, ON.** An empty Lua table, one result. **LABELLED PARTIAL:** the host keeps a plane's
payload as an ordnance set with no Lua objects. The torpedoes the image lists stay on the racks,
so the squadron stays armed where the image's callers Kill them and disarm it.

**Host, OFF (today).** An unimplemented record that pushes nothing. Every caller below then
raises in `pairs(nil)`, which aborts the rest of its function.

### Callers in this installation

All the files are dated 2024-07-13 except prcp_11_patrol.lua (2024-08-26). PRCPUS
usn_11_protecting_iowa.lua is byte-identical to the COTP-USN copy (same hash and size).

| File:line | Function | Argument | What the result is used for |
| --- | --- | --- | --- |
| COTP-USN/usn_11_protecting_iowa.lua:1296 | PrepareS1Phase4 | `Mission.Hammerhead`, true | `for idx,unit in pairs(t) do Kill(unit, true) end` |
| COTP-USN/usn_11_protecting_iowa.lua:1533 | PrepareS2Phase4 | `Mission.Monteray`, true | same loop |
| COTP-USN/PRCPUS/usn_11_protecting_iowa.lua:1296 / :1533 | same two | same | same |
| COTP-USN/PRCPUS/prcp_11_patrol.lua:1302 / :1539 | PrepareS1Phase4 (1281) / PrepareS2Phase4 (1528) | Hammerhead / Monteray, true | same loop |
| the three files, line 90 | (comment) | none | `--` comment, not live |

No other file in this installation names GetSquadTorpedoes. That includes
global/commandhelpers.lua and the COTP-IJN tree.

In both functions the squadron is created on the line before (or a few lines before) the call,
with `GenerateObject(luaFindHidden(...).Name)`.

**What follows the loop, and what OFF loses today:**

- **PrepareS1Phase4:**
  - `SetDeviceReloadEnabled(false)`
  - `SquadronSetTravelAlt(Hammerhead, 250, true)`
  - `PilotMoveTo(Hammerhead, PlayerSub3)`
  - `SetRoleAvailable(PlayerSub3, EROLF_ALL, PLAYER_AI)`
  - `SetForcedReconLevel(PlayerSub3, 2, PARTY_JAPANESE)`
  - `luaDelay(S1Phase4Movie, 1)`

  With OFF, none of these run. The phase-4 movie is never scheduled.
- **PrepareS2Phase4:**
  - `SetDeviceReloadEnabled(false)`
  - `SetShipSpeed(PlayerSub4, 12.5)`
  - `S2Phase4Movie()`

  With OFF, none of these run.

**Where empty-but-armed diverges from the image (ON):**

- In the image, the Hammerhead and Monteray squadrons lose every torpedo still on their racks.
  `SetDeviceReloadEnabled(false)`, which follows, keeps them from reloading. The squadrons
  then fly as unarmed escorts or patrols.
- In the host, each keeps its payload. If the squadron's AI or the player picks a target, it
  can still drop a torpedo the image would have removed.
- The scripted continuation (the travel altitude, the move order, the roles, the speed and the
  movies) runs in both.

### OPEN: the full fix

Payload torpedoes must become Lua-addressable entities: a self object per attached round, keyed
by its owner plane (`+3BCh`). The table can then list them. Kill on such an entity must also
empty the owning plane's ordnance: the rack count, the kind-2Bh bit, and whatever
`rack_holds_round` reads. Until both exist, this stopgap stays PARTIAL.

## 2. GetSubmarineAirSupply 008938E0

**Image:**

- 008939DF 00888AA0 on argument 0, with no null test and no class test.
- 008939F6 `FLD float [ESI+127Ch]`, then 008939FC `FSTP float`.
- 00B66480 PushFloat, which is `lua_pushnumber((double)float)`.
- One result. There is no other arm. A non-submarine has whatever its class keeps at `+127Ch`
  read and pushed. A null entity faults.

**Host, ON.**

- `GameUnitsHost::submarine_air_008938e0` answers the slot's `sub_air.air`. That is the field
  SetSubmarineAirSupply 00893A60 writes (`set_submarine_air_00893a60`, packet cc13-fix17, not on
  main when this was written) and the air model 00855250 steps.
- The accessor has the same `sub_dive_ready` test as the setter.
- **LABELLED:** an entity with no seeded submarine air state pushes nothing and records
  UNIMPLEMENTED.

**Caller:** jm10.lua:1764 `Mission.Checkpoint.Air = GetSubmarineAirSupply(Mission.PlayerUnit)`
in luaJM10SaveMissionData. Both copies (ijn/JM and COTP-IJN) have it. The restore is
`SetSubmarineAirSupply(sub, Mission.Checkpoint.Air)` at jm10.lua:1862, in
luaJM10RestoreUnits.

**Round trip.**

- The image stores `+127Ch` unclamped at the setter. 00855250 clamps it to 1.0 on the next
  force step (0085537A).
- The host's `submarine_step_air_00855250` clamps at the same point and nowhere else. The
  setter stores the value as given.
- A float widened to a double and narrowed back is exact. So
  `SetSubmarineAirSupply(GetSubmarineAirSupply(u))` stores the value that was read, in both the
  image and the host.
- With kSubmarineAirBound ON (it is), the value read is the model's current air.

## 3. PilotBomb 008A4F00: the class of a squadron argument

**Image:**

- 008A505C-008A5065 are `mov eax,[esi]; push 10h; mov ecx,esi; call [eax+5Ch]`. ESI is the
  00888AA0 result for argument 0, the argument itself.
- For a squadron, vtable 00D087C0+5Ch is 007EFB00 (dword 00D0881C = 007EFB00).
  007EFB00 answers true only for 18h, 2, 1, 0 and `[this+C4h]`.
- 007F2DEC (`mov dword [esi+C4h], 18h`) is the `+C4h` writer the brief names. So a squadron
  never answers 10h, and gets divebomb 00E08F20 unless 007ED880 answers dropkamikaze.

**Host (under `kLuaPilotBombBound`, OFF).** A slot that a registry record names as its
`squadron_unit` (the fused wing 0, the same test as `GameHudHost::Impl::is_squadron_slot`) now
answers false. Any other slot still asks `unit_is_kind_of(unit, 0x10)`, which is the plane arm.

**SUBSTITUTION, labelled:** the fused slot cannot be told apart from its wing-0 plane named on
its own, so that plane is taken as the squadron.

Before this fix, Betty and Kate squadrons (level-bomber classes) got levelbomb 00E08F28.

**Callers:** none live in this installation. See docs/CC13_FIX16.md, which this packet
corrects. The usn_10 calls are in luaUSN10_InitAttack, whose only call is commented out. The
commandhelpers call is inside a `--[[` block.

## 4. Census rows in the `handled` OR-list

In `src/game_hosts_lua.cpp`, `last_catapulted_row` (`kLuaLastCatapultedBound`, 00892860) and
`formation_leader_row` (`kLuaFormationLeaderBound`, 00899AF0) are dispatched further down. They
were missing from the `handled` OR-list, so `note_native_call` recorded both as sticky
UNIMPLEMENTED although they are answered.

They are now in the list. This changes only the census and the log. The early returns that
answer the two rows run either way, so behaviour does not change.

## Missions reached

- **GetSquadTorpedoes:** usn_11_protecting_iowa (COTP-USN and PRCPUS) and prcp_11_patrol
  (PRCPUS).
- **GetSubmarineAirSupply:** jm10 (ijn/JM and COTP-IJN), at its checkpoint save.
- **PilotBomb:** none.
- **The census rows:** every mission that calls GetLastCatapulted or GetFormationLeader. Only
  the log changes.
