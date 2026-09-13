# Why `FindEntity` answered nil, and what the mission script does once it does not

Addresses: 00925F20 0088D8E0 00B672B0 00928A00 00928BA5 00B67630 0057C1A0 008A8930 00803A40 00806B10

Packet `cc_lua_find_entity`, worktree `agent/cc-lua-find-entity`. Ghidra was read-only for this
packet; every descriptive name below is a hypothesis, not a recovered symbol. It continues
`docs/MISSION_BLACKOUT.md`, which left `usn_2_java.lua` stopped at line 563 with
`Mission.EscapePoint` nil, and it closes that document's follow-up 1.

Reconstruction: `include/bsp/lua_binding_mission_2.hpp`, `src/lua_binding_mission_2.cpp`.
Run-time wiring: `src/game_hosts_lua.cpp`, `src/game_hosts_script_orders.cpp`,
`src/game_hosts_mission_frame.cpp`.

## The binding was never the problem

`00898E30` and `0088B1B0` were both already complete in `docs/LUA_BINDING_ENTITY_LOOKUP.md`, and
both readings hold. What the binding pushes is `thisTable[<decimal of entity+174h>]`, and for the
entity `usn_2_java` asks for that slot did not exist. Three separate facts had to be established
before it could:

1. **Who calls the attach.** `docs/MISSION_ENTITY_LUA_ATTACH.md` established that the self table is
   built by entity virtual slot 39 (`+9Ch`) and closed with "the host has to reach slot 39 for it",
   naming no caller. The caller is **`00925F20`**, whose own literal at `00D19244` is
   `SEntity::InitAll`: its first pass calls `[vtable+9Ch]` on **every** node of the pending-entity
   list at `0092604E`, with no class filter. That site is the only `[reg+9Ch]` virtual call in the
   image (below).
2. **Which classes carry an attach there.** `NavPoint`'s vtable is `00CE8550` and the dword at
   `00CE85EC` (`+9Ch`) is `00928A00`, the base attach itself. So a NavPoint gets a `thisTable` slot
   exactly as a ship does.
3. **`ID` is a string.** `00928BA5` seeds it through `00B67630`, whose second push is `00A67A10`
   `lua_pushlstring`. The executable seeded a number, and every shipped helper indexes
   `thisTable[Obj.ID]`, so the lookup silently missed.

## `00925F20`, `SEntity::InitAll`

`__fastcall void(char force_recon_refresh)`, `RET`, body `00925F20`-`0092638A`. The argument is the
low byte only (`00925F45 MOV byte ptr [ESP+8],CL`) and is read once, at `00926367`.

`[00F899D0]` is the sentinel of a circular list of `{next +0h, prev +4h, entity +8h}` nodes and
`[00F899D4]` is its length. An empty list returns at `00925F49` without running anything, which is
the arm the fixed-step caller `00875BB0` takes on an ordinary frame. Five passes follow, then the
list is emptied and the count zeroed (`00926335`..`00926365`).

| Pass | Range | Per entity |
| --- | --- | --- |
| A | `00925FC4`..`00926062` | `[vt+10h]`, progress, **`[vt+9Ch]`** — the Lua self-table attach |
| B | `009260A0`..`0092611E` | `[vt+10h]`, progress, `[vt+0A0h]` |
| C | `00926136`..`00926230` | `[vt+10h]`, progress, `[vt+0A4h]`, then the start-enabled branch |
| D | `00926250`..`009262BF` | `[vt+5Ch](2)`; when true, `[vt+10h]` then `0077F090` |
| E | `009262D0`..`0092632B` | destroy the spawn descriptor at `+C0h` and null the field |

`[vt+10h]` is the name accessor `0088B1B0` compares against; its result is **discarded** at all four
sites, so the calls are kept in the reconstruction as calls the native makes and nothing reads them.

Pass C's branch, `009261A3`..`00926222`: when `entity+C0h` is non-null and its `+4h` is `2`
(`00926131 MOV EBP,2`), the byte at `[[entity+C0h]+8h]+3Ch` decides. Set, and with `+5Eh` and `+5Ch`
both clear, the routine sets `+5Ch = 1`, calls `[vt+68h](0)` and walks the `+48h`/`+44h` child chain
through `00922F30(1)`. Clear, and with `+5Eh` clear and `+5Ch` set, it sets `+5Ch = 0`, calls
`[vt+6Ch](0)` and walks the same chain through `00922F80(1)`. `+5Ch` and `+5Eh` are the same two
bytes `0088B1B0` gates its candidates on, so an entity that starts disabled is also unfindable.

The progress denominator is `count * 3` (`00925FB3 LEA EAX,[EAX+EAX*2]`), the fraction is
`FILD step / FIDIV denominator` narrowed to float32 once at the store, and pass A clamps its counter
at `count` (`0092601E`'s `0x55555556` reciprocal divides the denominator by three).
`0057C1A0` is `__fastcall(ECX = phase, float fraction)`, `RET 4`, and every site passes phase 3; its
callee `0057BF10` sums the per-phase weights at `00E08764` below `phase` and adds
`weights[phase] * fraction`, which is what makes it a loading bar rather than a log line.

### The `[reg+9Ch]` scan

`CALL dword ptr [reg+disp32]` does not occur anywhere in `battlestationspacific.exe`: MSVC emits
`MOV reg,[vtable+disp32]` then `CALL reg`. Scanning the image for `8B /r disp32` immediately
followed by `FF D0`..`FF D7` gives **one** site for `+9Ch` (`0092604E`, in `00925F20`) and one for
`+110h` (`00923BF1`, the unit health accessor), against 13 for `+80h`. So slot 39 is reached from
exactly one place, and that place runs for every pending entity.

## The class table

`include/bsp/lua_binding_mission_2.hpp` carries eleven rows, one per scene class whose creator
stores a vtable with an attach at `+9Ch`. The vtables were read from the shipped image by scanning
each creator of the 26-row class table of `004F2800` (and one call deep) for `.rdata` immediates,
then reading `vtable+9Ch`. The `world_kind` column is the bucket column of
`docs/LUA_BINDING_ENTITY_LOOKUP.md`, and the two readings agree on every vtable they share.

| class | id | vtable | `+9Ch` | world kind | findable |
| --- | --- | --- | --- | --- | --- |
| `PlaneSquadronGen` | `18h` | `00D087C0` | `007F4580` | `12h` | yes |
| `LandConvoy` | `1Ah` | `00CEA570` | `00743450` | `14h` | yes |
| `LandingPoint` | `1Dh` | `00CE90E0` | `00928A00` | unread | no |
| `NavPoint` | `41h` | `00CE8550` | `00928A00` | `3Bh` | yes |
| `MovieCamPos` | `42h` | `00CE86D8` | `00928A00` | `3Ch` | no |
| `MovieCamLookat` | `43h` | `00CE8860` | `00928A00` | `3Dh` | no |
| `Path` | `47h` | `00CE6290` | `00928A00` | `41h` | yes |
| `CameraPath` | `4Ah` | `00CE8390` | `00928A00` | `44h` | yes |
| `SpawnPoint` | `4Dh` | `00CEA218` | `0077E830` | `47h` | yes |
| `SimpleEffect` | `5Bh` | `00CE8BD0` | `00928A00` | unread | no |
| `PeriodicEffect` | `5Ch` | `00CE8D68` | `00928A00` | unread | no |

The fifteen classes with no row are the `UnitClassFactory` and `TypedResource` creators, whose
vtable is installed deeper than the scan follows; those are the unit classes, which the executable's
unit host already owns. They are absent from the table rather than recorded as "no attach".

**Having a slot and being findable are different questions**, and milestone 2l ran them together.
`MovieCamPos` and `MovieCamLookat` get a slot and `0088B1B0` does not walk their buckets, so the
executable now builds the slot table and the name index from different sets.

## Per-binding sections

### `FindEntity` `00898E30` — was a record, now answers

The binding body is not re-reconstructed: `bsp::lua_binding_find_entity` in
`include/bsp/lua_binding_entity_lookup.hpp` already carries it, and this packet supplies the two
things it was missing. The executable now builds a `thisTable` slot for every scene entity that
reaches slot 39 and indexes by name only the ones on a walked bucket. All 38 calls the mission makes
resolve: `entity_resolves=38`, against `self_table_entities=34` (32 created units and the scene's two
`NavPoint`s).

Host method, in call order:

| step | method | call site | native |
| --- | --- | --- | --- |
| 1 | attach, once per pending entity | `0092604E` | `00CE8550+9c` (indirect, `00928A00` for a NavPoint) |
| 2 | the lookup | `00898F8B` | `0088B1B0` |
| 3 | the id | `00898F98` | — (`MOVZX EAX, word ptr [entity+174h]`) |
| 4 | the decimal key | `00898FA7` | `004260B0` |
| 5 | `thisTable` | `00898FC4` | `00B67910` |
| 6 | `thisTable[key]` | `00898FDA` | `00B678E0` |
| 7 | push it | `00898FE9` | `00B663D0` |

### `GetMeasure` `0088D8E0` — was concrete and wrong, now concrete

`0088D9BD CMP byte ptr [00F88988],0x0` picks the arm; each arm assigns one literal to a NativeString
through `0041E870` and hands it to `00B672B0`. The metric arm at `0088DA04` uses `00CF5900`
"globals.kilometer" and the imperial arm at `0088D9CF` uses `00CF5914` "globals.mile".
`00B672B0` is `__thiscall(LuaObject, const NativeString*)`, `RET 4`, and its only two callees are
`00A672F0` `lua_checkstack(L, 1)` and `00A67A10` `lua_pushlstring(L, data, length)`, with the empty
literal at `0108FF2C` standing in for a null data pointer at `00B672CD`. It pushes the string; it
does not look anything up. The reconstruction's contract said the opposite, and the mission's own
`luaMetric` (`usn_2_java.lua:923`) compares the result against those two literals, so the old
contract made `luaMetric` return nil.

### `GetPosition` `008A7B00` — extended to a scene marker

`008A7C3C` reads the world matrix translation at `entity+0FCh`, for whichever entity the argument
resolved to. The executable now answers it for the two `NavPoint`s from the composed `localframe`
the scene file authored, which for a `FixedInstance` marker class nothing in the mission moves:
`DRGoTo` at `(250, 0, 9000)` and `EscapePoint` at `(0, 0, -7500)`, matching `usn_2_java.scn:1742`
and `:1751`.

### `SetParty` `008A8930` — was a record, now runs its own body

`usn_2_java.lua:54` is `this.Party = SetParty(this, PARTY_ALLIED)`, and no shipped script assigns
`Mission.Party` anywhere else, so the mission's party number is this binding's **return value**. As
a record it returned nothing and `Mission.Party` stayed nil, which would have made
`recon[Mission.Party]` nil even with the shell installed. The body is already reconstructed as
`bsp::lua_binding_set_party` (`docs/LUA_BINDING_CORE.md`); this packet only routes the row to it.
Both entity virtuals it dispatches through (`+5Ch` at `008A8A83`, `+2Ch` at `008A8AE3`) still have
no resolved concrete vtable, so the executable reports both and takes the no-session-message arm by
choice, exactly as that document records. The run confirms `Party=0`, which is `PARTY_ALLIED`
(`luamw_init.lua:73`).

### The `recon` shell `00803A40` — new, and not reached by the current run

`004E0305` sets the global `recon` to nil on the mission-load pass. On that path the native's route
back to a table is `00806B10`, whose `006B8190` / `00803750` / `008037D0` descent recreates whatever
is nil before `00805D90` fills the nineteen category maps (`docs/RECON_SLOT_LISTS.md` section 4).
`00806B10` runs off the recon slot lists, which this process does not build, so it stays a record and
the executable installs the shell `00803A40` builds instead, through the existing
`bsp::install_recon_values_00803a40`. Every category map is empty, so
`luaGetOwnUnits` (`commandhelpers.lua:12343`) returns an empty list rather than raising. That call is
on the mission-end path and the current run does not reach it; the evidence that it is the next gate
is the traceback quoted below. What the run does confirm is the shape the helper walks:
`recon_own_categories=19` in the summary is `recon[Mission.Party].own` counted at the end of the
run, so with `Party=0` the index resolves and the nineteen maps are there.

## Run log

```
build/win32/Release/bsp_game.exe --frames 2700 --press-start-frame 30 --menu-select USN02
  --mission-frames 2500 --mission-frame-seconds 0.05 --log local/find_entity_r6.log
  --xlive-dll build/win32/Release/xlive_stub.dll
  --game-root "I:/SteamLibrary/steamapps/common/Battlestations Pacific"
```

```
  scene marker DRGoTo         class=NavPoint id=33 findable=1 attach=00928a00 pos=(250.0,0.0,9000.0)
  scene marker EscapePoint    class=NavPoint id=34 findable=1 attach=00928a00 pos=(0.0,0.0,-7500.0)
thisTable: 34 per-entity slot(s) built for the created scene instances
recon shell built by 00803a40: three party indices, each with enemy, neutral, unknown and own,
  each of those with the nineteen category maps of 00E0B590

summary mission script timers created=32 think_registrations=32 waits=31 clears=0 deletes=30
        passes=2500 fires=71 failures=0
summary mission script bindings calls=371 attackmove=20 moveto=1 issued=21 reached_director=21
        units=15 formations=0/14 skills=22 repairs=12 roles=4
summary mission script state: MissionPhase=2 EndMission=nil Party=0 Distance=3.22
        Measure=globals.kilometer MissionCompleteRan=nil MissionFailedRan=nil
        entity_resolves=38 self_table_entities=34 recon_own_categories=19
host methods 636 concrete, 483 unimplemented
```

**`failures=0`.** The mission script raised 29 times per run before this packet and raises not once
now. `entity_resolves=38` is every `FindEntity` call the mission makes; `self_table_entities=34` is
32 created units plus the scene's two `NavPoint`s. The script also creates 32 delayed calls against
11 before, because the objective checker now runs to its own `luaDelay` tail instead of unwinding
out of `luaTimetable` on every pass.

Three intermediate states are worth recording, because each was a different failure:

| Run | State |
| --- | --- |
| before | `usn_2_java.lua:563`, 29x, `luaGetDistance's second param is a non-table parameter`. `Mission.EscapePoint` nil. |
| markers wired | `usn_2_java.lua:565`, 29x, `bad argument #2 to 'format' (number expected, got nil)`. `luaMetric` returned nil because `GetMeasure` answered the value at the key, not the key. |
| `GetMeasure` fixed | `commandhelpers.lua:12343`, 29x, `attempt to index global 'recon' (a nil value)`, through this traceback, which is the whole reason the recon shell is in this packet: `luaGetOwnUnits` <- `luaInitMissionEnd` <- `luaMissionCompletedNew` <- `luaMissionComplete` (`usn_2_java.lua:817`) <- `usn_2_java.lua:593`. `luaMissionComplete` ran because `ID` was a number, `thisTable[Obj.ID]` was nil, `GetPosition(nil)` gave the origin for both arguments and `NavDist` was 0. |
| `ID` fixed | no failure; the state above. |

**`MissionComplete` does not run, and should not.** `usn_2_java.lua:577` completes phase 2's primary
objective when the escorted `Houston` is within 500 m of `EscapePoint`. `Houston` starts at
`(1200, 0, -6000)` (`usn_2_java.scn:1145`), 1921 m from the marker, and it is the unit
`SetSelectedUnit` hands the player. Headless it keeps its authored `Cruise` command and sails away:
after 2500 mission frames the script's own `Mission.Distance` is 3.22 km, and a 12000-frame run
(`--frames 12300 --mission-frames 12000`, 600 simulated seconds) reaches 8.77 km with still no
failure. Nothing in the Lua layer gates this any more.

One caveat carries over unchanged from `docs/MISSION_BLACKOUT.md`: `GetHpPercentage` answers 0
because the unit health vtable `+110h` is unimplemented, so phase 1 still closed on a false
positive. The phase machinery is proven; that predicate is not.

## Coverage

| Routine | Address | Reconstruction | Coverage |
| --- | --- | --- | --- |
| `SEntity::InitAll` | `00925F20` | `sentity_init_all_00925f20` | complete for the five passes and the tail; the SEH frame and the one-time label init at `00925F5F`..`00925F99` are not modelled |
| loading progress | `0057C1A0` | none | complete: forwards `(phase, fraction)` to `0057BF10` on `[00E194B4]` when that is non-null |
| progress weights | `0057BF10` | none | read only far enough to establish that `phase` selects a weighted band |
| `GetMeasure` | `0088D8E0` | `bsp::lua_binding_get_measure` (corrected contract) | complete for both arms |
| string push | `00B672B0` | none | complete, `00B672B0`..`00B672E2` |
| `ID` seeding | `00928BA5` | `bsp/mission_entity_lua_attach.hpp` (already correct) | complete for the one field |
| `FindEntity` | `00898E30` | `bsp::lua_binding_find_entity` (unchanged) | unchanged from `docs/LUA_BINDING_ENTITY_LOOKUP.md` |
| `0077F090` | `0077F090` | none | **contract: unread**; named by address, one callee `0077EB50` |
| `[vt+0A0h]`, `[vt+0A4h]` | — | none | **contract: unread**; named by slot |
| `SetParty` | `008A8930` | `bsp::lua_binding_set_party` (unchanged) | wired, not re-read; the two entity virtuals stay **contract: unread** |
| `recon` shell | `00803A40` | `bsp::install_recon_values_00803a40` (unchanged) | wired, not re-read |
| `Recon::publish_slot_table` | `00806B10` | none | record; the slot lists it walks are a separate subsystem |

Host methods in call order, with native call sites, are the `host_steps` list of
`reports/lua_binding_mission_2.json`.

## Corrections

- **`GetMeasure` answers the key, not the value.** Was
  (`include/bsp/lua_binding_mission.hpp`): "Both arms push the *value at a dotted globals path*,
  through 00B672B0, so the binding answers the localised unit name the globals table carries, not a
  literal of its own." Is: `00B672B0`'s only callees are `lua_checkstack` and `lua_pushlstring`, so
  it pushes the literal. Evidence: `00B672B0`..`00B672E2`, and the shipped `luaMetric` comparing
  `GetMeasure()` against `"globals.mile"` / `"globals.kilometer"`.
- **`ID` in a `thisTable` slot is a string.** Was (`src/game_hosts_lua.cpp` milestone 2l):
  `lua_pushnumber(entity.id)`. Is: `00928BA5` hands `00B67630` the same NativeString the key was
  formatted into, and `00B67630`'s value push is `lua_pushlstring`.
  `docs/MISSION_LUA_SELF_TABLE.md` line 93 already said so; the executable did not follow it. The
  consequence was silent and total: every shipped helper that takes an entity table indexes
  `thisTable[Obj.ID]`, a number key never matched the string key, and `luaGetDistance3D` therefore
  compared two all-zero positions and answered a distance of 0. The 2500-frame run before the fix
  reported `luaMissionComplete` at mission frame 793 on that false zero.
- **Having a `thisTable` slot is not the same as being findable by name.** Was
  (`src/game_hosts_lua.cpp`): one `scene_entity_ids_` map filled from the same list as the slots. Is:
  slot 39 is called on every pending entity and `0088B1B0` walks 14 of 97 buckets, so
  `MovieCamPos`/`MovieCamLookat` have a slot and are not findable. Evidence: `0092604E` has no class
  filter; `docs/LUA_BINDING_ENTITY_LOOKUP.md`'s bucket table.
- **`00928A00`'s "sole caller 0077E830" is about direct calls only.** Was
  (`config/names`, `00928A00`): "sole caller 0077E830". Is: that is the direct-call graph;
  the routine is also entered through vtable slot 39 from `0092604E`, which is how a class that
  installs the base implementation unchanged (every `00928A00` row of the class table above) reaches
  it at all. The ledger evidence is appended, not replaced.

## Follow-up packets

1. **`recon_slot_publication`** (`00806B10`, `00805D90`, `008073C0`). The nineteen category maps are
   empty here. Filling them needs the recon slot lists of `docs/RECON_SLOT_LISTS.md`, a subsystem
   this packet deliberately did not enter. Until then any shipped helper that asks "which units do I
   see" answers an empty list, which is a wrong answer rather than an error.
2. **`sentity_init_slots_a0_a4`** (`[vt+0A0h]` at `0092610A`, `[vt+0A4h]` at `00926194`, and
   `0077F090` at `009262AE`). The three passes of `00925F20` this packet named by slot and did not
   read. Pass C's start-enabled branch is read; what the two virtuals themselves do is not.
3. **`mission_player_control`**. `usn_2_java`'s phase-2 primary objective completes when the escorted
   `Houston` comes within 500 m of `EscapePoint`, and `Houston` is the unit `SetSelectedUnit` hands
   the player. Headless, it keeps its authored `Cruise` command and the distance grows. Nothing in
   the Lua layer gates this; reaching `MissionComplete` needs whatever drives the controlled unit.
4. **`unit_health_accessor`** (`00923BE0`, unit vtable `+110h`), still open from
   `docs/LUA_BINDING_MISSION.md`. `GetHpPercentage` answers 0, so phase 1 still closes on a false
   positive. `00923BF1` is the one virtual call site, from the scan above.
5. **`lua_binding_entity_party_vtable`** (`008A8A83`, `008A8AE3`). Already a follow-up of
   `docs/LUA_BINDING_CORE.md`, and now on a path the executable runs: `SetParty`'s two entity
   virtuals have no resolved concrete vtable, so the executable reports both and no party is
   actually written to any entity.
6. **`landing_point_effect_buckets`** (`00CE90E0`, `00CE8BD0`, `00CE8D68`). Three classes that carry
   an attach at `+9Ch` and whose world bucket was not read, so whether `FindEntity` can name a
   `LandingPoint`, a `SimpleEffect` or a `PeriodicEffect` is open.

## no_ghidra_function

none. Every address this packet names is inside a Ghidra function: `00925F20` and `0057C1A0` are
functions in the stored listing, and the class vtables are data.
