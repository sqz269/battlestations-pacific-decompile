# BSP_AirOps_LoadFromScene, 006CADD0

Addresses: 006CADD0, 008F2260, 00467310, 006C0F00, 007B8A80, 00964790, 006BCF40, 006D3C10,
007593D0, 00CE3850.

Evidence: the listing of 006CADD0 (1613 instructions), the string constants read from the shipped
image, and the decompilation of 006C0F00. The slot offsets agree with
`include/bsp/air_operations.hpp`, which an earlier packet recovered from the launch routines, and
with the Lua reader in `docs/MISSION_LUA_GETPROPERTY.md`, which recovered them from a third place.

## 1. It is a three-mode serializer, not a loader

The function takes a session object; `session+4h` is the mode and `session+8h` the context.

| mode | tested at | what it is |
| --- | --- | --- |
| 1 | `006CADF9` | the scene property-bag load, read whole below |
| 2 | `006CB2C6` | reached through a virtual at context vtable+14h; not read |
| 3 | `006CB894`, `006CC35C` | the property walk that carries the `_airBase` group marker and the sixteen Lua property names; not read |

Only mode 1 is reconstructed. Mode 3 uses the same names as the declaration list at `006C19D0`
that `docs/MISSION_LUA_GETPROPERTY.md` section 5 recovered, which is consistent with it being the
save or transfer side of the same block, but that was not established and is a contract.

The two callers are the two classes that own a deck: `006D3C10 BSP_AirField_ReadRunwayProperties`
and `007593D0`. Those are the same two whose Lua reader sits at vtable+138h, so what mode 1 builds
is exactly what `GetProperty(carrier, "slots")` reads back.

## 2. Mode 1, in order

Every lookup goes through `008F2260`, which answers with a property record whose payload is at
`+0Ch` and whose type letter is at `+4h`.

1. **`NumSlots`** (`006CAE37`). The count. The existing array is then walked at stride 58h with a
   virtual call per record and the live count at block+50h is zeroed (`006CAE7E`). A deck with no
   authored `NumSlots` is an emptied deck, not an untouched one: `006CAE81` takes the exit only
   after the clear.
2. **`MaxInAirPlanes`** (`006CB0B3`) goes straight into **block+58h** (`006CB0C9`), with no clamp
   and no default.
3. **`PlaneStock %d`** (`006CB0D5`), composed with `00467310` and 1-based. Each is a sub-block of
   `Type`, `Count` and `SquadLimit` (`006CB108`, `006CB117`, `006CB126`).
4. **`Slot %d`** (`006CB1B2`), composed the same way and 1-based, so the index
   `LaunchSquadron` returns and the index the Lua `slots` array uses are the same index the scene
   authored.

A sub-block key must answer with property type 6 and a non-null payload, or the loop stops
(`006CB0FB` and `006CB1E0` for the type, `006CB0F9` and `006CB1DA` for the null).

## 3. Every slot field's authored source

| slot field | offset | authored as | written at |
| --- | --- | --- | --- |
| class | +04h | `Type`, through `007B8A80` | `006CB260` via `006C0F00` |
| count | +08h | `Count` | `006CB260` via `006C0F00` |
| `equipment` | +10h | **`Arm`** | `006CB277` |
| state | +2Ch | 6, only when `FakeAllocated` | `006CB28B` |
| timer | +30h | 0, or 5.0 from `00CE3850` when the launch-requested byte was set | `006CB292`, `006CB2A1` |
| launch requested | +34h | cleared on that same path | `006CB2A6` |

`FakeAllocated` is read as property type 3 with its byte at `+0Ch` (`006CB236`), so any other
shape leaves it false.

Two things this settles that were open elsewhere. `equipment`, the key the Lua reader publishes
from slot+10h, is authored as **`Arm`**; that is the only other name the field has. And **state 6**
is a value none of the launch routines showed: `AirOpsSlotState` records 1, 2 and 5, and the scene
loader writes 6. It is recorded because the loader writes it, not because its meaning is known.

`006C0F00`, which performs the class and count assignment, is not a plain store. It clamps the
authored count twice, against `BSP_AirOps_StockAvailable` for that class and against
`FUN_006BD460` for that slot, and when the slot already holds the same class it adds the slot's
current count to the available figure before comparing.

## 4. The host

`src/game_hosts_scene_contents.cpp` reads the four key groups out of the merged bag when a scene row
of class `MotherShipGen` (09h) or `AirField` (45h) is instantiated, and
`bsp::air_ops_load_from_scene_006cadd0` in `src/air_operations.cpp` turns them into the block. Each
deck is logged:

```
air ops deck: unit=<name> class=<id> NumSlots=N MaxInAirPlanes=N slots=N stock=N (006cadd0 mode 1)
```

The executable hangs the block off the entity. This process has no entity object, so the decks live
in one process-wide table keyed by the authored unit name, and
`attach_scene_entities_00928a00` binds the mission Lua's entity id to that name, because it is the
one place that holds both. The id is the unit index plus one, which is the convention the objective
bindings already read back as `ID - 1`.

`GetProperty` then serves the real deck: `slots` and `numSlots` from the slot array, `stock` and
`planes` from the `PlaneStock` records.

### Contracts

* **The `Type` resolver.** `007B8A80` is a thunk to `00964790 BSP_VehicleClass_GetOrCreate`. Its
  argument form was not read. The scene authors these tokens as numeric class ids, so the host
  scans an integer, and a token that is not a number resolves to zero, which is what an
  unresolvable token does.
* **`SquadLimit` is read and not placed.** `006CB126` reads it; where it lands was not read, so the
  host carries it unused.
* **The `006C0F00` clamp is not modelled.** This process has no live stock accounting, so the
  authored count stands where the native would reduce it.
* **Modes 2 and 3 are unread**, as section 1 says.
* The deviation from `docs/MISSION_LUA_GETPROPERTY.md` is unchanged: this host cannot pick the
  reader by class, so a unit with no deck still reaches the four keys and answers with an empty
  deck rather than with the nothing the native pushes.

## 5. What this does and does not unblock

It gives `IsReadyToSendPlanes` and `LaunchSquadron` something real to read, and it makes
`luaGetSlotsAndSquads` return a genuine slot count for a carrier instead of zero. It does not by
itself launch anything: `00895D20` calls `BSP_AirOps_GetBlock` and tests the class through
vtable+5Ch against 45h before it answers, and `0089E3C0` has to create a squadron entity and
register it in the script's `thisTable`. Those are the next packet.

## Measured: the USN04 run of 2026-09-18

`local/usn04_gates.log`, exit 0, 3000 mission frames. The binary carried the deck, both launch
gates and the stationary fix, and it also carried the block+38h correction of
`docs/AIROPS_LAUNCH_START.md`, which had been built before the run took the lock.

| measurement | value |
| --- | --- |
| decks built | 6 |
| each deck | `NumSlots=4 MaxInAirPlanes=12 slots=4 stock=4` |
| `GetProperty` calls / served / unserved | 164 / 164 / 0 |
| slot tables published | 3394 |
| `IsReadyToSendPlanes` calls / true | 82 / 82 |
| `LaunchSquadron` calls / started / queued | 82 / 82 / 0 |
| `script call Think failed` | **0**, was 41 |
| ordered aircraft | 2, unchanged |
| torpedo task built | still none |

**The carriers do author `Slot %d` blocks.** That was the open question, and the answer is four
slots and four stock entries on every one of the six, with a plane limit of twelve. A sample line:

```
air ops deck: unit=Zuikaku-class01 class=9 NumSlots=4 MaxInAirPlanes=12 slots=4 stock=4 (006cadd0 mode 1)
```

**The mission think no longer fails.** It ran all 3000 frames without one failure, where before it
aborted 41 times at `commandhelpers.lua:2496`. That is what serving `slots` was for.

**The gates run and the script tolerates the nil striker.** Every readiness check answered true and
every launch started, so the script reached and passed its launch line, and the line after it,
which reads the absent `squadron` key, raised nothing. That was the one thing
`docs/AIROPS_LAUNCH_GATES.md` section 5 could not predict.

**No strike, exactly as predicted.** `ordered` is still 2 and no torpedo task is built, because
nothing creates the squadron entity. That is the remaining step named in
`docs/AIROPS_LAUNCH_START.md` section 4.

### One thing the run found that no reading did

The slot index grows without bound:

```
LaunchSquadron 0089e3c0: class=101 count=3 arm=0 (class default) -> slot 0 (started), returns 1
... slot 1 ... slot 2 ... slot 3 ... slot 4 ... slot 5 ...
```

`006C7210` takes the first slot in state 1 or 5 and otherwise continues into the array-growth path,
which is the native's own behaviour. In the native a launched slot returns to state 1 when its
cooldown completes, so growth is rare. This process has no launch tick, so every slot it writes
stays in state 3 for ever and the next launch grows the array again: 82 launches over two carriers
left roughly forty slots each where four were authored. Nothing reads those slots, so nothing is
wrong downstream, but the deck is no longer a faithful projection after the first four launches on
a carrier. The tick that would return a slot to state 1 is the same one that fills slot+28h.

## 6. The `Type` token is an enum symbol (cc9-lua45, `kSceneDeckTypeEnumBound`)

Found by cc9-ships39 (USNOS probe `s39_osprobe3`): every scene deck slot and stock row resolved to
class 0, so `006BF230`'s class match never hit and `006C0F00` left every slot it was asked to fill
empty. The section 4 contract ("the scene authors these tokens as numeric class ids") is wrong.

**Authored form.** This installation's 259 `.scn` files (2024-10-29) carry 43,054
`Type = E PlaneClasses : <symbol>` lines (deck sub-blocks and squadron rows alike) and no
`Type = I <number>` line at all (a `Select-String` census; `local/l45_deck_census.py` counts 5,380
`PlaneStock` and 11,752 `Slot` blocks). `PlaneClasses` is the enum at
`universe/library/global.enums` line 589 (F2G 810, AD2 339, BTD_Destroyer 331, BlackCat 343).

**Image.** `006CB108..006CB114` (stock) and `006CB1F5..006CB201` (slot) read the `Type` property
through `008F2260` and take its `+0Ch` payload (`MOV EBP,[EAX+0Ch]`) without a type test;
`006CB139` / `006CB24E` move it to ECX and call `007B8A80`, which is `MOV DL,1; JMP 00964790`
(disk bytes): `BSP_VehicleClass_GetOrCreate(__fastcall int typeId, bool readRace)`, which indexes
`registry[10h + typeId*4]`. For an `E` property the reader stores the resolved integer at `+0Ch`
(`008F33A6` with a key declaration, `008F5F94` through the global enum registry without one), so
the argument is the PlaneClasses integer. ABI: ECX type id, DL 1, result EAX (descriptor); this
process carries the type id itself as the class, as every other caller of the deck does.

**Binding.** `kSceneDeckTypeEnumBound` (`include/bsp/game_hosts_scene_contents.hpp`): the reading
half (`scene_deck_type`, `src/game_hosts_scene_contents.cpp`) resolves the symbol through
`PropertyLibrary::resolve_symbol` with the authored table, on the created path and the held-back
path, and hands the loader decimal text. An unresolved symbol stays raw and scans to 0, the old
answer (what the image stores for an unresolvable symbol was not read: contract). A new census
line `air ops deck classes: unit=<name> stock=<class>x<count>,... slots=<class>x<count>,...`
prints what each deck resolved, in both states.

**Correction (loop exits).** Section 2 says a missing sub-block stops the loop. The listing says
otherwise: `006CB0F9` / `006CB0FF` / `006CB106` jump to `006CB158`, the index increment, and the
stock loop runs `PlaneStock 1..12` (`006CB15F CMP EAX,0Ch; JLE`); `006CB1DA` / `006CB1E4` /
`006CB1EF` jump to `006CB2A9`, the slot index increment, bounded by the live count at block+50h
(`006CB192`, `006CB1A2`). The reading half still stops at the first gap. The census found no gap
and no `PlaneStock` index above 12 in this installation, so the difference cannot move a run;
recorded, not changed.

**Predictions (written before the pairs).** OFF is identical to the base. ON: every
`air ops deck classes` line shows non-zero ids (Enterprise in USNOS `810x40,339x..,331x..`).
`GetProperty(unit, "slots")` and the stock rows publish real classes, so the Lua support managers
(commandhelpers) that launch from scene decks can now find stock and fill slots: expect new
`LaunchSquadron` successes and new airborne squadrons from scene carriers and airfields on every
row with an AI deck (USNOS, ESMP08, USN13, JM08, LOMP10, E2), so death, shot and kill rows move on
those rows; USN04 (player carriers launched by script with explicit classes) and the five
scripted-win rows (USN02, USN01, LOMP06, LOMP10, USN12) are where a mission-timing change would
show. A mechanism failure is any deck whose authored symbol stays 0 ON, or a launch whose class
the stock does not answer for.

## Uncertainty

* Whether USN04's carriers author `Slot %d` blocks at all. The scene is binary and this process
  does not dump the bag, so the first run will say: the per-deck log line above prints `NumSlots`
  and the slot count it built, and a carrier with no authored slots will print zeros.
* Modes 2 and 3, and whether mode 3 is the save side of the same block.
* What state 6 means.
* Where `SquadLimit` lands.

## Host methods

`bsp::air_ops_load_from_scene_006cadd0` standing in for `006CADD0` mode 1, and
`bsp::AirOpsDeckRegistry`, which is not a native structure but this process's stand-in for the
block the executable hangs off the entity.

## Corrections

None. `include/bsp/air_operations.hpp` gained `class_field_134` in the previous packet, and this
packet names its authored source, `Arm`, which is the first name recovered for it.

## no_ghidra_function

None.

## Validation

**Blocked.** No run is possible: session 1, where the agents run, is disconnected, so no window,
D3D device or FMOD output can be created. The build is clean and both ctest suites pass. The run is
one call once the session is back:

```
./tools/run_game.ps1 -Log local\usn04_airops.log -WaitSeconds 2400 -- --frames 3200 `
    --press-start-frame 30 --menu-select USN04 --mission-frames 3000 --mission-frame-seconds 0.05
```

What it should show: an `air ops deck` line for each of the four carriers USN04 spawns, with the
authored `NumSlots`; `summary mission getproperty 0088bf80` with `slots_rows` no longer zero if any
deck has slots; and no `script call Think failed` at `commandhelpers.lua:2496`. It should still not
launch the strike, for the reason section 5 gives.
