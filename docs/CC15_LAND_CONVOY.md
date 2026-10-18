# Land convoy bodies 007420B0, 00742180, 007422A0 (packet cc15-rD1)

Standalone reconstructions with host seams, for the cc9 gameplay lane to wire
later behind OFF switches. Build-tested only: no host is wired, no game run,
not ABI-compatible. Names are hypotheses (the Ghidra names from
cc2_land_and_structures). Source: src/cc15_land_convoy.cpp, contract and seams:
include/bsp/cc15_land_convoy.hpp. docs/CC15_SEG41_TAIL.md ("Rows left for the
cc9 lane") lists what the cc9 hosts already model.

## Layout and the dispatch chain

LandConvoy vtable words: 00CEA5F0 holds 00742180 (slot +080h, the on-killed
slot) and 00CEA610 holds 007420B0 (slot +0A0h). Fields: +030h entity registry,
+070h kill cause, +0C0h scene property bag, +344h path interface, +394h member
vector (VC9 checked vector, +398h first, +39Ch last), +3BCh live member count.
Members are land vehicles (vtable 00CFFDE0): +738h is the owning convoy;
vtable +080h is 0074CD10, vtable +128h is 006D1EB0
(BSP_Unit_SetSkillLevel_Forward).

The two kill entry points, from the listings:

- **00926D90 Kill** (`__thiscall(entity, cause)`, RET 4): returns when +5Fh is
  set; else sets it, stores the cause in +70h (cause 7 is stored as 2) and
  queues the entity on the kill list 00F899B4. Slot +080h runs on a later
  flush (009273A0, CALL at 0092751F), not inside 00926D90
  (docs/UNIT_DAMAGE_AND_DEATH.md, death sequence step 10).
- **Damage death of a member**: the destroy flush calls vtable +074h
  (00926390, base), which tail-jumps to vtable +07Ch. For a land vehicle that
  is 0074DB70, which ends with 00926D90(self, self+70h) at 0074DCAD. So a member
  shot dead reaches the kill list too, and its 0074CD10 runs one flush after its
  destroy flush.
- **0074CD10** (member slot +080h): when member+738h is non-null,
  007422A0(ECX = member+738h, member); then JMP 00951FB0.

## 00742180 convoy kill (slot +080h)

`__fastcall(ECX = convoy)`, no stack arguments, no x87, no register inputs
beyond ECX. Saves EBX/EBP/ESI/EDI.

1. ESI = first, EDI = convoy+394h. Each pass re-reads last (+39Ch); exit when
   ESI equals it (equality, not less-than).
2. For a non-null slot: `member+738h = 0` (007421D5), then the cause is read
   from convoy+70h (007421E2, re-read for every member, after the clear), then
   00926D90(member, cause) (007421EF). The slot word is read three times
   (test, store target, ECX); the model reads it three times.
3. Null slots are skipped; nothing is erased.
4. 00742203-00742209: restore registers, ECX = convoy, **JMP 00779AF0**. A tail
   call: 00779AF0 (the player-unit on-killed override, itself tail-jumping to
   the base 00928C80) returns to 00742180's caller. Modelled as the last call.

Clearing +738h first means the member's own on-killed (0074CD10, on the next
flush) skips 007422A0, so the live count is not touched by a convoy-level kill.
Every member gets the convoy's cause, not its own.

Seam (`LandConvoyKillHost`): member_slot_count, member_slot,
invalid_parameter_noinfo_00bf6713, clear_member_convoy_738,
convoy_kill_cause_70, mission_entity_kill_00926d90,
mission_entity_on_killed_00779af0.

## 007422A0 member removal and live count

`__thiscall(ECX = convoy, stack member)`, RET 4, no x87. Only caller 0074CD10
at 0074CD1E.

1. Walk first..last (last re-read each pass), comparing each slot word with the
   member argument ([ESP+14h], re-read each pass, constant).
2. **Found**: null that slot (00742305), then `ADD [convoy+3BCh], -1`
   (0074230B). The walk stops at the first match.
3. **Not found** (end reached): nothing is nulled or decremented.
4. **Both paths** reach 00742312: if `[convoy+3BCh] == 0` (exact zero),
   00926D90(convoy, 2) (00742320). Otherwise return.

Consequences cc9 asked about:

- The kill at zero fires even when the member was not in the vector, as long as
  the count is already zero. After the first zero-count kill, 00926D90 returns
  at once on +5Fh, so repeats are harmless.
- A negative count never kills.
- A null member argument matches the first slot nulled earlier and decrements
  again. 0074CD10 never passes null (it passes ECX of its own call), so this
  matters only to a host that does.
- The convoy kill is queued, not immediate: the convoy's slot +080h (00742180)
  runs on the next flush. By then the members' slots are all null, so its walk
  kills nothing and only the 00779AF0 tail runs (Lua on-killed release).

Seam (`LandConvoyRemoveHost`): member_slot_count, member_slot,
invalid_parameter_noinfo_00bf6713, null_member_slot, live_count_3bc /
set_live_count_3bc, mission_entity_kill_00926d90. Returns the nulled index or
kMemberNotFound for the host's bookkeeping (the image returns nothing used).

## 007420B0 bind land convoy motion (slot +0A0h)

`__fastcall(ECX = convoy)`, RET, no x87 (00927A80 returns in EAX).

1. 0077F0E0(convoy) (007420B3), result unused.
2. bag = convoy+C0h. If null, or `[bag+4] != 1` (SUB 1 / JNZ), return. **The
   whole rest, the member broadcast included, is behind this gate.**
3. property = 008F2260(ECX = [bag+8], "Path" at 00CEA738); name =
   008F0DF0(property), no null test of property in between; entity =
   00925A90(ECX = convoy+30h, name).
4. convoy+344h = 007AC9D0(entity) when entity is non-null, else 0.
5. skill = 00927A80(convoy), read once before the walk.
6. Walk first..last (last re-read each pass); for each non-null slot,
   member->vtable[128h](skill) (`__thiscall`, callee pops).

Seam (`LandConvoyBindHost`): the slot walk plus
unit_owner_activate_and_register_0077f0e0, property_bag_c0,
property_bag_state, property_bag_records, scene_property_bag_find_008f2260,
scene_property_string_008f0df0, entity_registry_30,
entity_registry_find_by_name_00925a90, entity_path_interface_for_kind_007ac9d0,
set_path_interface_344, property_bag_get_skill_00927a80,
member_set_skill_vt128.

## Checked iterators and the index model

Every `CMP ESI,[EDI+8] / JC / CALL 00BF6713` becomes "if i is not below
member_slot_count(), call invalid_parameter_noinfo_00bf6713()" at the same
place; the image's CALLs fall through, and so does the model. The entry and
per-pass "first <= last" and "same container" (`CMP EDI,EDI`) asserts cannot
fail on a count and are not called. The index model equals the image while
first does not move during a walk (00926D90 only queues, so it does not
reallocate the vector itself); after a reallocation the image keeps its stale
pointer and the model does not.

Precision: no x87 in any of the three bodies, so the 24-bit main-thread
precision control does not matter here.

## Reach list

Scene class `LandConvoy` (template `Entities\LandConvoy`, properties `Path`,
`Rows`, `RowGap`, `Speed`, `TypeN.Type = LandVehicleClasses:...`). Members are
generated from the type rows; scripts reach them as `convoy.Members[i]`. Lua
natives seen: LandConvoyStart, LandConvoyStop, LandConvoySetSpeed,
LandConvoySetPath, plus Kill, GenerateObject and the `.Dead` field. Only seven
scenes have LandConvoy entities. Menu rows from tools/mission_script_reach.py's
missiontree mapping (scratch script local/rD_convoy_reach.py, not committed):

| Rank | Menu id | Script | Convoys | What exercises the three bodies |
|---|---|---|---|---|
| 1 | JM05 | COTP-IJN/PRCPIJN/JM05.lua | Event5Convoy (hidden, 4 rows US apc/tank), Event3Convoy 01-03 (hidden), SecondaryLandConvoy 01 (live), RadarStationLandConvoy 01 (live) | Event 5 "Catch the convoy": GenerateObject("Event5Convoy") (3883, 007420B0 bind), then polls `Convoy.Dead` (4369) for success: the player destroying all members drives 007422A0 to zero and its 00926D90(convoy, 2), then 00742180 on the next flush. Timeout and arrival call `Kill(convoy, true)` (3687, 4162, 4380); objective 1 kills SecondaryLandConvoy 01 the same way (2841). |
| 2 | PRCP08 | COTP-USN/usn_12_Indochina.lua | Train (live, 8 rows: engine, wagons, turret wagon, Japanese) | `AddListener("kill","TrainKilled", entity = {Train.Members[1]})` (1939): a member kill, so each member death reaches 007422A0 (decrement); the zero-count convoy kill only when all 8 members die. Members set invincible and back (668, 917); AddDamage(Members[1], 5000) from oil tanks (1323). LandConvoyStart/Stop/SetSpeed throughout. |
| 3 | USN20 | COTP-USN/PRCPUS/usn_16_endgame_at_kure.lua | same six as JM05 (Japanese party) | Scene only: the script never names them. The two live ones (Secondary, RadarStation) are bound by 007420B0 at load and can be shot (007422A0 per member); no script listens. |
| 4 | IJN10 | Ijn/ijn_10_port.lua | Event3 01-03 (hidden), Secondary, RadarStation (live) | Scene only, as USN20. Its `Mission.Convoy` is a table of troop-transport ships, not a LandConvoy. |

The other three scenes with LandConvoy entities (COTP-IJN/ijn_05,
ijn/JM/ijn_05, COTP-USN/PRCPUS/usn_12) have no missiontree row.

Caveats for wiring:

- `Kill(convoy, hard)` on a LandConvoy does **not** go through 00742180 first.
  The Lua Kill binding at 008AC740 tests IsKindOf 1Ah and calls 00742210
  (BSP_EntityGroup_KillMembersVector, cause 1 or 2 from `hard`), which kills
  and nulls each member slot but neither clears +738h nor kills the convoy
  (008AC753 jumps past the 00926D90 at 008AC756). On the next flush each member's
  0074CD10 calls 007422A0, which no longer finds it (slot nulled): no decrement,
  so the convoy is killed only if the count was already zero. The cc9 host
  (src/game_hosts_lua.cpp, the "008AC740: a LandConvoy" substitution) kills the
  convoy's own slot instead.
- Who sets the Lua `Dead` field is still open (docs/MISSION_ENTITY_LUA_ATTACH.md:
  nothing found in the image sets it true). JM05's event 5 success depends on it.
- 007420B0 runs only with the property bag in state 1; which init pass calls
  slot +0A0h for GenerateObject versus scene load is cc9's
  (`GameUnitsHost::bind_land_convoy_motion`).

## Validation

`scripts/build.ps1` (MSVC Win32 Release plus the existing CTests). See the
commit message for the result.
