# CC13 fix18: mission Lua natives (packet cc13-fix18)

Packet cc13-fix18 answers mission Lua natives that had no answer in the Lua host and corrects the
plane arm of GetEntitySpeed (fix12c). Each native sits behind its own new switch, and every switch
is **OFF**. With a switch off, the row is handled exactly as before: UNIMPLEMENTED, nothing
written, nothing pushed. With the reload multiplier off, the rack step is `dt` exactly, as before.

Nothing here was run. There were no flips, no game runs and no Ghidra writes, and nothing is
game-validated. The work is build-tested only.

Status of the eight items:

| # | Native | Address | Switch | Status |
|---|--------|---------|--------|--------|
| 0 | GetEntitySpeed, plane arm (fix12c) | 008C0820 / 007B8E60 / 007D8020 | `kLuaGetEntitySpeedBound` (existing) | done |
| 1 | PutRelTo | 008AA560 | none | **skipped** |
| 2 | ClearForcedReconLevel | 008AAB90 | `kLuaClearForcedReconLevelBound` | done |
| 3 | SquadronSetTravelSpeed | 008A0630 | none | **skipped** |
| 4 | SquadronSetHomeBase | 0089E220 | none | **skipped** |
| 5 | SetDeviceReloadTimeMul | 008C14B0 | `kLuaDeviceReloadTimeMulBound` | done |
| 6 | SetCrewLevel | 00894F20 | `kLuaSetCrewLevelBound` | done |
| 7 | SetRace | 008A8720 | none | **skipped** |

No native in this packet draws from the image RNG (00BD2F10). That covers the four that are bound
and the three that were read and skipped.

The game root is this installation's `scripts/` directory under the path in
`config/target.json`. Mission counts below come from a grep of that directory.

## 0. GetEntitySpeed plane arm (fix12c)

`run_get_entity_speed_008c0820` keeps its switch, `kLuaGetEntitySpeedBound`. Only
`plane_cached_speed_007b8e60` (in `src/game_hosts_lua.cpp`) changes. That helper serves both the
plane arm and the squadron arm, which reads through the leader.

- **Source.** 007B8E60 is `FLD [unit+0B1Ch]`, and +B1Ch is ctl+6Ch, where ctl = unit+AB0h.
  007D8020 BSP_PlaneFlightController_UpdateSpeedTerms writes it at 007D807C, computing it from the
  controller's world velocity ctl+18h. 007D8020 runs as the tail of 007D9C10 and 007D9C80, which
  are its only callers. The host's copy of ctl+18h is the slot's `plane_world_velocity`. The new
  read-only accessor `GameUnitsHost::unit_plane_world_velocity` returns it. It is the only change
  to `game_hosts_units.*` for this item.
- **Why not the linear velocity.** While a plane is deck-parented, 007C71E0's 007D9CE0
  (007C7277) subtracts the owner's velocity from ctl+18h. The host's
  `carrier_deck_reparent_007c71e0` does the same to `plane_world_velocity` and leaves
  `motion.linear_velocity` alone. That leaves `unit_linear_velocity`, which the old code read,
  as the wrong field while the plane is parented.
- **Chain.** This runs on the main thread at 24-bit precision control, so each x87 operation
  rounds to float:
  1. 007D8024-007D803B compute `s2 = (y*y + x*x) + z*z`. Each product and each sum is rounded to
     float, in that order.
  2. 007D803D stores `s2` as a float.
  3. 007D8041-007D804F compare `s2` with the double 1e-10 at 00CE3820 (bytes `bb bd d7 d9 df 7c
     db 3d`) using FCOMI, then JBE.
  4. If `s2` is at or below the threshold, or is NaN, the result is 0.0f (007D8064).
  5. Otherwise the result is `float(sqrt(s2))`, from 00BF7030 and stored at 007D8056.
- **LABELLED.** The image reads the value cached at the last 007D9C10 or 007D9C80 call. The host
  computes it when the native runs.

`docs/CC13_FIX12.md` is corrected:
- Hammerhead (usn_11 `:1294`, `:1363`) is a squadron and takes the squadron arm.
- The jm05 call sites (`COTP-IJN :1236`, `PRCPIJN :1341`, `luaJM5CheckCapPt`) pass the enemy
  planes that `luaGetPlanesAround(Mission.CapPt, 300, "enemy")` returns. They do not pass a
  captured PT boat.
- 007D807C belongs to 007D8020.
- The precision bullet now describes the x87 chain.

## 1. PutRelTo 008AA560: skipped

The binding works in this order (standalone body: `lua_put_rel_to`, `src/cc13_left55_seg56b.cpp`):
1. It resolves A (argument 0), B (argument 1) and an offset (argument 2, 00888760).
2. It copies B's world matrix +CCh and sets its translation row to B's matrix applied to the
   offset (004142E0).
3. It zeroes y when A is a ship (IsKindOf(6) and not 8).
4. It calls A->vtable[114h], whose result is unused, then A->vtable[120h](matrix).
5. With four arguments, it calls A->vtable[11Ch](argument 3 * pi / 180).
6. It calls A->vtable[D8h].
7. For a group leader (IsKindOf(2), then the leader test 00778890), every other member gets
   vtable[118h] with the global zero vector at 00F87574 as its argument, then vtable[D8h].

The slots by class, read from the PE:

| Class (vtable) | 120h | 11Ch | D8h | 114h |
|---|---|---|---|---|
| Ship 00CF90B0 | 00819840 (009585B0, 00414DB0, 0092D620, 00818EA0) | 008196B0 | 00955970 | 0080E150 |
| Plane 00D00070 | 009585B0 | 009584D0 | 007BB310 | 0047F180 |
| Squadron 00D087C0 | 007F2F90 (matrix copy only) | 007F2EB0 | 0042BAA0 | 007ECFD0 |

**Reason for the skip.** The units host has no world-matrix setter. Its
`place_at_world_position_008193a0` sets a position only and refills the wake along the *current*
heading. The image refills it after the new orientation is in place. The host also has none of
the three 11Ch heading setters; the PutTo host logs 11Ch as unimplemented. A follower's
`vtable[118h](0)` station snap (0081945D) is not modelled either. A position-only stand-in would
leave A facing the wrong way, so the native stays unbound. While this packet ran, cc9-lua52 held
the units host for PutTo work (`cc9_l66_putto`). PutRelTo should build on that work: one
`set_world_matrix_00819840` per class plus the heading setters.

**Reach.** 94 calls in 42 files. The heaviest users are bsm_06 (15 calls) and bsm_02 and bsm_11
(7 each); bsm_02's invasion groups are at `:1081-1095`. jm02, jm05 and bsm_07 have 3 each.

## 2. ClearForcedReconLevel 008AAB90

**Image** (bytes and decompile):
- 008AAC90: 00888AA0 resolves argument 0. There is no null test.
- 008AACC1: 00B66290 reads argument 1 as the observing party. It is not range-checked: 008AACD8
  does `IMUL EDI,EDI,34h`.
- 008AACE2: 00803B50 runs on the entity's own record at entity+1E8h+party*34h. This happens
  whatever the class.
- A squadron (vtable[5Ch](18h)) loops over i < +3CCh (008AACF6-008AAD27) and calls 00803B50 on
  [+3D0h+4i]'s record. When i > 4 it reads a null base (008AAD04 `CMP EBP,4 / JA`).
- A LandConvoy (vtable[5Ch](1Ah)) walks the vector at +394h (begin +398h, end +39Ch) with checked
  iterators (008AAD83-008AADE4) and calls 00803B50 on each element (008AADD2).
- There is no result (008AAD2D 00B66400).

**00803B50** is `__fastcall(ECX = record)`, RET. Its only caller is this binding.
1. 00803B52-00803B5E: `old = +10h ? +8h : +4h`.
2. 00803B66: `+10h = 0`.
3. 00803B64 / 00803B6A: when `+4h != old`, it calls `[+2Ch]->vt[0](+30h, old, +4h)`, the
   record's listener.

**Host.**
- `run_clear_forced_recon_level_008aab90` resolves the entity as the other entity natives do and
  takes the party from `lua_tonumber(2)`.
- It calls `bsp::clear_forced_recon_level_00803b50(target, party)` for the slot and then for each
  live member of the squadron registry record whose fused slot it is. That is the same target
  set SetForcedReconLevel's host forces.
- `clear_forced_recon_level_00803b50` (`src/recon_sensor_pass.cpp`) removes the (party, target)
  pair from the forced table.
- At the next records rebuild (`recon_sensor_pass_rebuild_008073c0`), a cleared pair whose
  persistent record is still forced gets 00803B50's effect:
  1. `old = effective level`.
  2. `forced = false`.
  3. When `level != old`, the transition `(old, level)` is queued ahead of the pass's own calls.
     The existing dispatch then fires the `recon` listeners.
- Before this packet nothing could clear the force byte, so the comment there now names 00803B50.

**Fields written.** The host's forced table entry for (party, target) is removed, and the
persistent record's `forced` flag (+10h) is cleared.

**Readers.**
- The records rebuild (forced table to record +10h/+8h).
- 00806840's skip of the sensor test on a forced record (`subject.detection_forced`).
- The pass dispatch's forced publication (`dispatch_recon_listeners_00980e50`).

**LABELLED.**
- An unresolved entity faults in the image. Here it is counted as `unresolved`, and nothing is
  cleared.
- A LandConvoy's element vector is not carried. Its own slot is cleared, mirroring
  SetForcedReconLevel's host.
- The notify reaches the listeners at the next pass rather than inside the native.
- A Clear followed by a Set before the next pass gives the image two notifies. The host gives one
  queued transition plus the Set's own path.

**Missions reached.** 28 calls in 18 files:
- commandhelpers.lua `:4238`-`:4300` (nine sites), the `ForcedRecon` helpers.
- jm04, jm06, jm09, jm12 and jm14, in both `ijn/JM` and `COTP-IJN` (and PRCPIJN).
- prcpjm12, `ijn/ESMP/02_MI` `:1317` and chg_4_might `:333`.

**ON prediction.** A pair forced by SetForcedReconLevel stops being forced. The unit then
publishes its sensor level again, typically going from 2 (forced identified) to the level the
observers actually reach. Each change fires one `recon` listener call. With the switch off, a
forced unit stays forced for the rest of the mission.

## 3. SquadronSetTravelSpeed 008A0630: skipped

**Image** (standalone body: `lua_squadron_set_travel_speed`, `src/cc13_left55_seg56a.cpp`):
1. It reads argument 1 as a number and stores it as a float (008A076C FSTP float).
2. It reads argument 2 as a boolean only when exactly three arguments are passed. Otherwise the
   flag is false.
3. It writes, on the squadron:

   | Offset | Value | Address |
   |---|---|---|
   | +388h | 0.5f ([00CE3800]) | 008A07E2 |
   | +38Fh | flag | 008A07F0 |
   | +3A0h | speed | 008A07FA |
   | +3ACh | 1 | 008A0802 |

There are two readers of these fields:
- 007ED5D0 BSP_PlaneSquadron_RefreshTravelSpeed, called from 0099ACD0 BSP_PilotBot_Tick. When
  +38Fh is 0, it copies `+3A0h = [+35Ch]+190h` provided `0.0 > +388h` and +3ACh is 0. It then
  clears +3ACh.
- 007ED590 ResetTravelSpeed, called from SquadronRestoreTravelSpeed 008A1A00. It restores all four
  fields.

+3A0h itself is the squadron cruise speed. The host reads it at 009C1850's moveto speed blend
(`moveto_speed_009c1850`'s `travel_scaled`) and at `retreat_squadron_speed_3a0` (three call
sites).

**Host field it would write: none exists.** The units host keeps no squadron +3A0h. Every reader
recomputes `unit.plane_travel_speed x NewTravelSpeedMul`, the class value, at the point of use.
`game_hosts_units.cpp` has `squadron+3A0h` only in comments. Unlike the travel-altitude block
(`sq_alt_394`, `sq_freeze_38d`, `sq_lock_3a9`, `sq_timer_expiry_380`), there are no
`sq_*` fields for +388h, +38Fh, +3A0h or +3ACh.

Binding the native would need these fields on the squadron slot, plus the 007ED5D0 refresh at
the pilot-bot tick and an override at each of the readers above. Those readers belong to the
cc9 physics lane's planner and cruise code. Writing the speed anywhere else would store it into
a dead field, so the native is left unbound.

**Open point.** The writer that counts +388h down was not found. A displacement scan of
`88 03 00 00` in 007E-007F and 0099-009C shows only 007ED590, 007ED5D0, 007F2026, 008A07E6 and
the AvoidTerrain reads at 009A0FC0/009A0FEA. 0092B350's +388h belongs to the generic tickable
entity, and nothing here shows that it is the squadron's field. With no countdown, the script
speed persists until SquadronRestoreTravelSpeed.

**Reach.** 38 calls in 27 files. Examples: bsm_04 `:1417`/`:1418`/`:1434` and bsm_09 `:323`.

## 4. SquadronSetHomeBase 0089E220: skipped

**Image.**
- 0089E320 and 0089E351 resolve argument 0 (ESI) and argument 1 (EDI) through 00888AA0.
- 0089E36C calls 007F1C00 BSP_PlaneSquadron_SetHomeAirBase(ECX = squadron, base, flag) with
  flag = EBP. EBP is zeroed at 0089E243 and not written again before the call, so the flag is 0.
- 007F1C00 then does the following:
  1. It swaps the observer pair and stores `+404h = base`.
  2. With a base, it takes the block through 006BCD20.
  3. With the session word [00E188A8]+1FE4h at zero, it pushes the squadron onto the block+74h
     queue through 006CC7B0. Since flag is 0, +408h is not written and 007ED6E0 is not called.
  4. A null base clears +408h.

**Host field it would write.** The host's +404h is `GameUnitSlot::scene_home_base_404`, the
resolved deck name. Only the squadron pass C path (around `game_hosts_units.cpp:39166`) writes
it, inline, together with the 006CC7B0 push (`bsp::air_ops_push_assign_queue_006cc7b0`).
The reader that matters reads other fields. That reader is `record_return_to_base_007f16d0`'s
home arm (packet cc9_rtb_home_arm), and it reads `PlaneSquadronHostRecord::bag_home_base` (air-ops
launched) or `GameUnitSlot::scene_home_base_key` (the authored scene key), not
`scene_home_base_404`.

Binding the native needs a public units-host method that does three things: replace +404h at
runtime, push the queue entry, and make the RTB home arm read the runtime +404h. Without that
change, a write to `scene_home_base_404` would land where no reader looks. The native is left
unbound.

**Reach.** 22 calls in 12 files. Examples: commandhelpers `:16831`, jm04 `:2160` (the player
plane to Shoho) and jm08 `:1785` (a squadron to the airfield).

## 5. SetDeviceReloadTimeMul 008C14B0

**Image.**
- 008C15AF: 00B66270 (lua_tonumber) reads argument 0.
- 008C15B4: `FSTP float [00E08178]`.
- There is no entity, no clamp and no result.
- The float is written elsewhere too:
  - 004DFBEB in BSP_Game_LoadMissionScene writes 1.0f ([00D7A24C], `00 00 80 3f`).
  - 0076FE73 in BSP_Session_SetMode also writes it.
- It is read in two places:
  - 006E41D5, in 006E41C0 BSP_MBombPlatform_AdvanceReload.
  - 006E44D5, in 006E44C0 BSP_MMultipleBombPlatform_AdvanceReload.

  Both do `FLD [00E08178]` / `FMUL dt` / `FSTP float` and then `+490h = float(+490h - step)`.

**Host.**
- `run_set_device_reload_time_mul_008c14b0` stores `float(lua_tonumber(1))` in the process-wide
  float behind `lua_device_reload_time_mul_00e08178()`. It starts at 1.0, which matches the
  mission load.
- The units host's rack reload slice (`rack_device_reload_slice_006e57a4`, 006E57A4 -> 006E41C0)
  now subtracts `step = float(mul * dt)` instead of `dt`. This change is guarded by the same
  switch, and it is the one reader edit in `game_hosts_units.cpp`.
- With the switch off, or when no script sets the float, `step == dt` exactly.

**Field written.** 00E08178. The only reader is the rack reload slice's countdown
`rack_reload_left_per_rack`, the host's +490h. The host's single slice stands in for both the
single and the multiple bomb platform.

**Missions reached.** 43 calls in 28 files:
- COTP-USN usn_04 `:873` (2.0), usn_07 `:527` (1.5), usn_09 `:268` (1.8) and usn_13 `:56` (2).
  Each of these sets `SetDeviceReloadEnabled(true)` on the line before.
- The PRCPUS copies, prcp_07, prcp_08 and prcp_13.
- jm01 (3) and jm02 (3.0 then 2.0), jm03 `:762` (0), and the two airtraining missions.

**ON prediction.** On USN04 and USN13, an empty bomb rack refills in half its ReloadTime; on
USN07 it takes 1/1.5 of it. On jm03, a rack never refills once the multiplier is 0. A mission
that never calls the native is unchanged.

**Correction to `include/bsp/gameplay_seg33_cc13.hpp`.** Its adoption note says USN04 and USN13
never set the flag. In this installation the COTP-USN copies do set it.

## 6. SetCrewLevel 00894F20

**Image.**
- 00895022: 00888AA0 reads argument 0.
- 00895053: 00B66290 reads argument 1.
- 0089506C: 006E6210 is called with ECX = level.
- 0089507C: `entity->vtable[128h](skill)`, with no null test.
- There is no result (00895082 00B66400).

006E6210 maps the crew level to a skill. With m = ([00E188A8]+1FE4h != 0), which is 0 in single
player, the mapping is as follows (006E6210-006E6265):

| Level | Skill | Skill with m = 0 |
|---|---|---|
| 0 | m | 0 |
| 1 | 2m+1 | 1 |
| 2 | 2m+2 | 2 |
| 3 | 5 | 5 |
| anything else | 1 | 1 |

Slot 128h is 006D1EB0 on ships and planes (the +38Ch sub-object's vtable[0]) and 007ECF80 on the
squadron (it fans out to every member).

**Host.** The host function `run_set_crew_level_00894f20` hands the skill to the units host's
`set_skill_level_007b8ae0` (index, skill). That is the leaf SetSkillLevel 00895250 already reaches through the same slot
(`GameScriptOrdersHost::entity_set_skill_level`).

**Field written.** `GameUnitSlot::pilot_skill_index`, on the slot and on its squadron's members.
Its readers are the existing `skill_level()` consumers:
- the gun bots (`units.skill_level()`, read live);
- the dive-bomb approach capture (009F9D22).

**LABELLED.** An unresolved entity faults in the image and is counted here. The session word is
taken as 0.

**Missions reached.** 92 calls in 12 files:
- commandhelpers.lua `:3930`-`:3979`, eight sites: the Alt+F7 cheat key (`:3926`) and the
  `Mission.AlliedCrewLevelCheat` loops (`:3950`-`:3979`).
- chg_1, chg_3, chg_5, chg_7, chg_8, chg_9 and PC_cargo.
- multi/competitive01 and siege02.

**ON prediction.** In the chg missions, the scripted units get the mapped skill index. Level 3
becomes skill 5, the top index. A row that answered nothing before now changes the skill the gun
bots and the dive approach read. No base campaign mission (USN01, USN04, USN13, JM08) calls it
outside commandhelpers' cheat path.

## 7. SetRace 008A8720: skipped

**Image.**
- 008A8824: 00B66290 reads argument 1 (the race), before the entity.
- 008A8854: 00888AA0 reads argument 0.
- 008A886D-008A8873: vtable[5Ch](2).
- A class-2 entity builds message 80h (MT_VEHICLE_SET_RACE, 008A8879 / 0075B430) and routes it
  through 0077C2A0(entity, message, 7, 0) at 008A88B7. Its arm is
  `BaseUnitMessageArm::SetRace`, which is `set_party_and_race(current party, race)`.
- Any other entity gets vtable[2Ch]([entity+54h], race, &out) at 008A88DA. That is 00951F30,
  which jumps to 00928F50 SetPartyRaceLuaMirror: 00923B80 stores party +54h and race +58h, and
  then thisTable[key].Race and .Party are rewritten.

**Host field it would write: none exists.** The units host carries a party (`set_unit_side_0054`)
but no race. `mirror_party_race_00928f50` writes only `Party`, and its log says the host "does
not carry" a race. No shipped script reads an entity's `.Race`; the only two hits, in
chg_2_java `:442`/`:443`, read a template table's string. The image's other race readers were
not traced. With no race state and no reader, the native is left unbound.

**Reach.** 30 calls in 13 files. Examples: usn_07 `:1029` (HQ, 0) and `:2722` (Airfield, 2),
prcp_13 `:1304` and usn_11 `:2490` (a building, 1).

## Files

- `include/bsp/game_hosts_lua.hpp`, `src/game_hosts_lua.cpp`: the three new switches, their
  rows and run functions, the 00E08178 accessor, and the fix12c helper.
- `include/bsp/recon_sensor_pass.hpp`, `src/recon_sensor_pass.cpp`:
  `clear_forced_recon_level_00803b50` and the rebuild's clear step.
- `include/bsp/game_hosts_units.hpp`, `src/game_hosts_units.cpp`: the const accessor
  `unit_plane_world_velocity` and the switch-guarded reload-step scale.
- `docs/CC13_FIX12.md`: the corrections listed in section 0.
