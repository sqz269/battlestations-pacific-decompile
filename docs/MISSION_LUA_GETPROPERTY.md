# GetProperty, 0088BF80

Addresses: 0088BF80, 00888AA0, 006C6630, 006C19D0, 00758340, 006D0E60, 00815870, 00425850,
00BF7FBF, 00B664B0, 00B677E0, 00B662B0, 00D01768, 00CF8D40, 00D112FC.

Evidence: the listing of 0088BF80 and 006C6630, the vtable data references of 00758340 and
006D0E60, the name constants read from the shipped image, and the property-name list 006C19D0
declares. Names in this document that appear in quotes are the binary's own strings, not
hypotheses.

## 1. What the native is

`GetProperty(entity, key)` is a dispatcher and nothing else. It holds no property knowledge.

```
0088C09D  the call frame's argument 0 goes to 00888AA0 BSP_ObjectHandle_FromLuaTable
0088C0A1  argument 1 is taken with 00B677E0 and its string with 00B662B0
0088C0BB  that string is assigned into a native string
0088C0C0  MOV EAX,[ESI]          the entity's vtable
0088C0C2  MOV EAX,[EAX + 0x138]  the reader
0088C0DC  CALL EAX               __thiscall(entity, frame, key)
          the return value is the frame's own result count
```

The reader's ABI is fixed by its own prologue: 006C6630 reads `[EBP+8]` as the frame and
`[EBP+0Ch]` as the key and ends `RET 8`, so the virtual is
`__thiscall reader(this, LuaFrame* frame, NativeString* key)`. The native pushes nothing itself,
which is why an unanswered key reaches Lua as no value at all rather than as a nil the dispatcher
pushed.

**The failure literal is dead.** `00D112FC "luaMW_GetProperty failed:"` is built at 0088BFB3 and
released at 0088BFDA on every call, with no branch around it and no arm that reports it. The
shipped build constructs the message and throws it away. It is used here only as the name of the
unanswered-key arm, because it is the only name the native carries for it.

## 2. Which reader serves a deck

The reader is the class's own vtable slot, and two classes own a deck:

| class | creator | observer table | reader | slot |
| --- | --- | --- | --- | --- |
| MotherShip | `00758D30` | `00D01630` | `00758340` at `00D01768` | +138h |
| AirField | `006D3110` | `00CF8C08` | `006D0E60` at `00CF8D40` | +138h |

Both differences are exactly 138h, which is the offset 0088C0C2 loads, so the dispatch is proven
from the data side as well as the code side. `00758340` is two calls: the ship base reader
`00815870` first, then the air-operations reader `006C6630`. A class without a deck has some other
routine in that slot and never reaches `006C6630`, which is why `GetProperty(plane, "slots")` is
nil in the shipped game.

## 3. The four keys 006C6630 answers

| key | site | what it pushes |
| --- | --- | --- |
| `planes` | `006C6661` | the stock list; the arm is shared with `stock` |
| `stock` | `006C667C` | one entry per stock record, `classid` and `count` (`006C6A0F`, `006C6A93`) |
| `slots` | `006C6690` | the slot array, below |
| `numSlots` | `006C6908` | the live slot count at block+50h (`006C6929`), pushed with `00B664B0` |

Anything else falls to `006C6B26` and pushes nothing.

**Every comparison is case-insensitive.** `00425850` is
`BSP_NativeString_EqualsCStringInsensitive`, which delegates to the CRT `stricmp`, and the `planes`
arm calls `00BF7FBF __stricmp` directly. That is why this installation's scripts reach the same
arms writing `NumSlots` and `Stock` as the binary does writing `numSlots` and `stock`.

## 4. The slot table

`006C66DE` tests the count at block+50h, `006C66F8` loads the array at block+4Ch, `006C66ED` seeds
the Lua index at 1 and `006C68D3` advances it, and `006C68D9` advances the record cursor by 58h. So
`slots` is a 1-based array with one table per slot, and the value `LaunchSquadron` returns indexes
it directly.

| key | slot offset | read at |
| --- | --- | --- |
| `state` | +2Ch | `006C672A` |
| `classid` | +04h | `006C6782` |
| `count` | +08h | `006C67D2` |
| `equipment` | +10h | `006C681F` |
| `squadron` | +28h | `006C6895` |

Every one of those offsets already had a name in `include/bsp/air_operations.hpp`, recovered by an
earlier packet from the launch routines rather than from this reader, and all five agree. The
stride 58h agrees with `kAirOpsSlotStride`. `squadron` is the launched squadron entity, not a
number, and it is absent until a launch fills slot+28h, which is exactly the test
`luaGetSlotsAndSquads` makes.

## 5. The declared property names

`006C19D0` is the other half of the pair: the air-base group's declaration, called by `00758DC0`
between the group markers `_airBase` and `_motherShip`. Its `{0, pointer}` descriptors name
sixteen properties, read from the image:

`readySlots`, `landingSquadrons`, `waitingPos`, `landingPlanes`, `distance`, `speedmul`,
`leftside`, `abort`, `maxInAirPlanes`, `suppplanetypes`, `slots`, `classID`, `count`, `equipment`,
`stock`, `squadLimit`.

These are the original identifiers. Only the four of section 3 are readable through `GetProperty`;
the rest belong to the declaration side.

## 6. What the scripts ask for

A census of this installation's `usn_19_coralus.lua` and `scripts/global/*.lua`:

| key | calls |
| --- | --- |
| `slots` | 37 |
| `TorpedoStock` | 9 |
| `ammoType` | 7 |
| `unitcommand` | 6 |
| `planes` | 3 |
| `NumSlots` | 2 |
| `state`, `reconlevel`, `Stock` | 1 each |

`slots` dominates, and it is the one the carrier launch path needs.

## 7. The host implementation

`src/game_hosts_lua.cpp` now runs row `0088BF80` in `binding_trampoline` instead of recording it
unimplemented. `GameMissionLuaHost::run_get_property_0088bf80` reproduces section 3's dispatch,
matches the key case-insensitively for the same reason the native does, builds section 4's table in
the native's key order and index base, and returns no value for any other key while recording the
literal of section 1. A new summary line reports the counts:

```
summary mission getproperty 0088bf80: calls=N served=N unserved=N slots_rows=N
```

### Contracts

* **The deck is empty.** This process builds no air-operations block: nothing in it calls
  `006CADD0 BSP_AirOps_LoadFromScene`, and no host holds an `AirOpsSlot` array. The walk is the
  native's and runs zero times, so `slots` answers with an empty table and `numSlots` with 0. That
  is what stops `commandhelpers.lua:2496` calling `pairs` on nil; it is not a claim that the
  carrier has no aircraft.
* **The key is served for every entity.** The native picks the reader by class, and this host
  cannot: the Lua host has no access to a unit's creator, so it answers the four keys for any
  entity table. The native would push nothing for a class without a deck. This is the one
  deviation, and it is more permissive than the original rather than different in value.
* **`stock` and `planes` return an empty list** for the same reason as the deck.

## 8. What still blocks the carrier launch

Serving `slots` stops the mission think aborting. It does not launch the strike, and the reason is
one function further on. `IsReadyToSendPlanes` (`00895D20`) resolves the entity, calls
`BSP_AirOps_GetBlock`, tests the class through vtable+5Ch against id 45h and reads a byte on the
entity before it answers. With no air-operations block in this process it cannot be answered
truthfully, and `LaunchSquadron` (`0089E3C0`) has to create a squadron entity and register it in
the script's `thisTable` for the line that follows it to resolve. Both are their own packets, and
`006CADD0 BSP_AirOps_LoadFromScene` reading the scene's authored `NumSlots` and `slots` is the
piece that would give all three real data.

## Uncertainty

* The stock branch at `006C6949` was read only far enough to establish that it serves both
  `planes` and `stock` and that its entries carry `classid` and `count`. Its record layout was not
  read.
* `00815870`, the ship base reader that runs before `006C6630` on a mother ship, was not read, so
  the keys it serves are unknown. `state`, `ammoType`, `unitcommand` and `reconlevel` are
  candidates for it.
* `equipment` is slot+10h, the copy of class+134h. What that field means is unknown; `equipment` is
  the only name recovered for it, and it is the reader's name, not a symbol.

## Host methods

`GameMissionLuaHost::run_get_property_0088bf80`, standing in for `0088BF80` and the
air-operations arm of the reader at vtable+138h.

## Corrections

One to this packet's own first draft, caught before it was committed: `equipment` was served from
the requested count at slot+0Ch, which is the wrong field. The reader loads slot+10h, and
`AirOpsSlot` had no member for it at all, so this packet added `class_field_134` to
`include/bsp/air_operations.hpp` rather than publish a neighbouring field under that name. No
correction to earlier work.

## no_ghidra_function

None.

## Validation

**Run, and it did what this section said it would.** The USN04 run of 2026-09-18, `local/usn04_gates.log`, exit 0: `summary mission getproperty 0088bf80: calls=164 served=164 unserved=0 slots_rows=3394 decks=6`, and zero `script call Think failed` lines where there had been 41. No launched strike, as this section predicted. The full table is in `docs/AIROPS_LOAD_FROM_SCENE.md`. The command was:

```
./tools/run_game.ps1 -Log local\usn04_getproperty.log -WaitSeconds 2400 -- --frames 3200 `
    --press-start-frame 30 --menu-select USN04 --mission-frames 3000 --mission-frame-seconds 0.05
```

What it should show: `summary mission getproperty 0088bf80` with `served` greater than zero and
`unserved` carrying the keys section 6 counts; no `script call Think failed` line at
`commandhelpers.lua:2496`; and the mission running its full 3000 frames instead of aborting its
think 41 times. It should **not** show a launched strike, for the reason section 8 gives.

Two consecutive attempts on a clean environment failed identically before any window was created,
and so did a run of another worktree's binary, which separates the machine from this tree:

```
startup failed: FMOD bank raw-length output unavailable: path=sound/gui/error.fsb
bytes=2688 mode=2634 create_result=78 length_result=37 bank_returned=0
summary window_created=0 device_created=0 device_hr=0x80004005 frames_presented=0 exit_code=1
```

The cause is the Windows session. `query session` reports session 1, the one the agents run in, as
`Disc`, with the console now on session 2. A disconnected session has no display and no audio
endpoint, so the window is not created, the D3D device returns `0x80004005` and FMOD cannot
initialise its output, which is what makes a 2688-byte bank that is otherwise intact fail to
create. The bank is unmodified (13 Jul 2024) and nothing under the game root changed after 15:00.
The build is clean and both ctest suites pass.

## 9. Every reader behind vtable+138h (packet `cc9_get_property_keys`, a read)

Worker cc9-lua2, 2026-09-27. Docs only; no code changes. This section answers the Uncertainty
bullet on `00815870` above, and finds the reader behind the `reconlevel` risk that
`docs/RECON_SENSOR_PASS_BINDING.md` raises.

### 9.1 How the readers were found

- **The readers chain.** Each derived reader first calls its base reader with the same
  `(frame, key)`, then tests its own keys case-insensitively (`__stricmp` for the first key,
  `BSP_NativeString_EqualsCStringInsensitive` for the rest).
  - A class answers the union of its chain's keys.
  - Every reader returns through `RET 8`, as section 1 says.
- **The census is by disk bytes, not by Ghidra's callers.** The script
  `local/cc9-lua2-readers.py` in the cc9-lua2 tree scans the image for two things:
  - every rel32 `CALL`/`JMP` to a reader;
  - every absolute dword equal to a reader. Each such dword is a vtable slot, so the vtable
    starts at slot-138h.
- **Classes come from `docs/ENTITY_CLASS_IDS.md`,** matched on the vtable column. The image
  carries no RTTI names.
- **The root is `00927AD0`.** It calls no other reader. The census from it and from
  `006D0E60` closes on the nine readers below.

### 9.2 The readers and their keys

| reader | calls first | classes (vtables) | keys, in test order |
| --- | --- | --- | --- |
| `00927AD0` | none | 11 vtables: Path 47, CameraPath 4A, NavPoint 41, MovieCamPos 42, MovieCamLookat 43, LandingPoint 1D, Landscape 44, 4 unmapped | `unitcommand` |
| `00779BB0` | `00927AD0` (`00779BD7`) | 29 vtables: planes 10-17, SpawnPoint 4D, gun and launcher emplacements 21-28, MCommandBuilding 1C, MLandFort 1B, MLandVehicle 19, 9 unmapped | `reconlevel` |
| `006E2AC0` (no Ghidra function) | `00779BB0` (`006E2AE7`) | 10 vtables: MBomb 2A, MTorpedo 2B, MDepthCharge 2C, MDummyTarget 2E, MDummyKamikazePlane 2F, MDummySubmarine 30, MParatrooper 31, MRocket 33, MWaterMine 34, 1 unmapped | `owner` |
| `007416F0` | `00779BB0` (`007416FF`) | LandConvoy 1A (`00CEA570`) | `InitialSize`, `ActualSize`, `CollectedDamage`, `LastHit`, `Reverse`, `Speed`, `Offset`, `DistanceTraveled` |
| `007EF1C0` | `00779BB0` (`007EF1CF`) | PlaneSquadronGen 18 (`00D087C0`) | `ammoType`, `state`, `TargetIsHome` |
| `00815870` | `00779BB0` (`0081587F`) | 8 ship vtables: MDestroyer 07, MSubmarine 08, MCruiser 0A, MCargo 0B, MLandingShip 0C, MBattleship 0D, MTorpedoBoat 0E, 1 unmapped (`00D09678`) | `TorpedoStock` |
| `00758340` | `00815870` (`0075834F`), then `006C6630` (`0075835C`) | MMothership 09 (`00D01630`) | `TorpedoStock`, then section 3's four keys |
| `006D0E60` | `00779BB0` (`006D0E6F`), then `006C6630` (`006D0E7C`) | MAirfield 45 (`00CF8C08`) | section 3's four keys |
| `00846640` `BSP_Shipyard_ReadStockProperties` | `00779BB0` (`00846667`) | MShipyard 46 (`00D0B770`) | `stock`, with its own layout |

A class inherits every key above it. A ship answers `unitcommand`, `reconlevel` and `TorpedoStock`.
A squadron answers `unitcommand`, `reconlevel`, `ammoType`, `state` and `TargetIsHome`.

### 9.3 What each key pushes

- **`unitcommand`, `00927AD0`.**
  - The reader calls the entity's `vtable[114h]()`, its director.
  - With no director it pushes nothing, so the script sees nil.
  - With a director it calls `0071BE40 BSP_WeaponDirector_CurrentCommand(director)`. A current
    command pushes that command's `vtable[4]()` name string through `00B66710`.
  - With a director and no current command, it pushes the literal `nocommand` (`00D1926C`).
  - The names are the command objects' own strings: `moveto`, `attackmove`, `retreat`, `land`
    and the rest of `src/entity_orders.cpp`'s table.
- **`reconlevel`, `00779BB0`.**
  - `00B67930` opens a new table as the frame's result (`00779C03`).
  - For party 0, 1 and 2 it then pushes `table[party] = level` through `00B665D0`, a number key
    and a number value (`00779C12`..`00779C33`).
  - Each record is `34h` bytes from `unit+1E8h`. The level is the forced level `+8h` when the
    force byte `+10h` is set, else the detected level `+4h`. These are the same records
    `00805AF0` writes (`docs/RECON_SENSOR_PASS_BINDING.md`).
  - **It always writes all three parties, 0 included.** The self table that `0077B0C0` fills
    keeps a party nil until that party first detects the unit. The two differ for a unit no one
    has seen.
- **`owner`, `006E2AC0`.**
  - It reads the projectile's owner at `+3BCh`. If that owner is null, it pushes nil through
    `00B66430` at `006E2BE5`.
  - It then tests `owner->vtable[5Ch](1)`. On false it pushes nothing.
  - On true it pushes `thisTable[tostring(owner+174h)]`, the owner's Lua self table: `0x174`
    is the entity id word, formatted by `004260B0`.
- **`ammoType`, `007EDAD0`.** This belongs to the squadron.
  - It walks the `+3CCh` planes at `+3D0h` and returns the first ordnance kind it finds: torpedo
    2, depth charge 3, `007B9400` 4, level bomb 5, drop kamikaze 6, general bomb 1.
  - It returns 0 when no plane carries any ordnance.
- **`state`, squadron.** It counts the planes at `+3D0h` for which `plane+72Ch->vtable[38h]()`
  is true (A) or false (B). It pushes 2 when B is 0, else 1 when A is non-zero, else 0.
- **`TargetIsHome`, `007EEE20`.** It pushes true when `+348h` is set and `+404h` equals the
  resolved command target.
- **`TorpedoStock`, `00815870`.**
  - It reads the spare stock `unit+104Ch`. A negative spare stock is pushed as it is.
  - Otherwise it pushes `spare + 00810E90(unit)`. `00810E90` counts the tubes in every launcher
    on the `+3ECh` list whose reload value is below `FLT_MAX`: the double at `00D7A278` is
    `3.4028234663852886e38`.
- **The LandConvoy keys** read `+3B8h`, `+3BCh`, `+3C0h`, `+3C4h` (as `now - value`), `+3A9h`,
  `+368h`, `+3ACh` and `+3B4h`. The last three are pushed as booleans (value `!= 0.0f`, from
  `00D7A218`). No mission this packet measures reads them.

### 9.4 What the host serves today

`GameMissionLuaHost::run_get_property_0088bf80` serves `slots`, `numSlots`, `stock` and `planes`
for every entity. That is section 7's permissive deviation, and it is unchanged. Every other key
returns no value and counts as `unserved`. The keys the host does not serve are:

- `unitcommand`
- `reconlevel`
- `ammoType`, `state` and `TargetIsHome`
- `TorpedoStock`
- `owner`
- the eight LandConvoy keys

The shipyard's `stock` is also answered with the deck layout, not with `00846640`'s own layout.

### 9.5 Which measured missions ask, and for what

Two sources were checked. The first is `GetProperty` in each mission's own script, in this
installation (read-only). The second is the host's own counts in cc9-ships2's latest OFF logs.
The `rb6_*` logs are from 2026-09-27 13:27-13:31, and `sdl_off_lomp06` is from 15:02.

| mission | script | keys in the mission file | measured calls / served / unserved |
| --- | --- | --- | --- |
| USN01 | `usn_1_marshall.lua` | none | 0 / 0 / 0 |
| USN02 | `usn_2_java.lua` | none | 0 / 0 / 0 |
| USN04 | `usn_19_coralus.lua` | `slots` x26 | 136 / 136 / 0 |
| BSM01 | `bsm_01_stationed_at_pearl.lua` | `ammoType` (:1302, :1346) | 0 / 0 / 0 |
| JM06 | `COTP-IJN/PRCPIJN/JM06.lua` | `ammoType` (:1013, :1057), `unitcommand` (:1056, :1162), `TorpedoStock` (:1482, :2342) | 17 / 0 / 17 |
| JM08 | `COTP-IJN/PRCPIJN/prcpjm08.lua` | none | 0 / 0 / 0 |
| USN13 | `usn_13_truk.lua` | `slots` (:293, :350, :401) | 9 / 9 / 0 |
| LOMP06 | `LOMP/06_crucial_cargo.lua` | `unitcommand`, `ammoType` (:746, :747); `luaGetReconLevel` (:701) | 0 / 0 / 0 |

The shared helpers in `scripts/global/commandhelpers.lua` and `messagesender.lua` also ask for
`ammoType`, `TorpedoStock`, `unitcommand`, `state`, `reconlevel`, `planes`, `slots`, `NumSlots`
and `Stock`. None of them reaches a GetProperty call in these runs, beyond the counts above.

- **JM06's 17 are all `unitcommand` at :1162,** in `luaJM6CheckUSNSubs`. The line is
  `GetProperty(unit,"unitcommand") ~= "attackmove"`.
  - nil is never equal to `"attackmove"`, so every check within 3000 m re-issues
    `NavigatorAttackMove(unit, Mission.PlayerUnit, {})`.
  - The log confirms this: 17 `NavigatorAttackMove` calls, all on
    `Narwhal-class Submarine 01 -> PlayerSub 01`. The first 12 unserved-key notes, which is the
    host's log cap, all name `unitcommand`.
- **LOMP06 reaches neither line in 1000 frames.**
  - :701 runs only when `UnitGetAttackTarget(Mission.PlayerUnit)` is not nil. The player is idle.
  - :746 is in `luaSubC1CheckSeaPlanes`, a later stage.
  - The `reconlevel` risk is on the target unit's table, not on `Mission.PlayerUnit`'s. The
    :531 read is `Mission.PlayerUnit.reconlevel[PARTY_JAPANESE]`, which `luaGetReconLevel` does
    not touch unless the player is its argument.
  - So a LOMP06 1200/1000 pair **cannot resolve** "reconlevel survives luaGetReconLevel". It
    needs a run in which the player has an attack target.

**Ranking by measured demand:**
1. `unitcommand`: JM06 17, the only unserved key any measured mission asks for.
2. `reconlevel`: no measured call. It is the correctness risk behind `kReconLevelTableBound`.
3. `ammoType`: no measured call. JM06, BSM01 and LOMP06 ask for it in code that does not run.
4. `TorpedoStock`: no measured call, JM06 code only.
5. `state` and `TargetIsHome` (commandhelpers only), then `owner` and the LandConvoy keys (no
   caller in these missions).

### 9.6 The binding plan (not yet written; `src/game_hosts_lua.cpp` is held)

A single switch, `kGetPropertyClassReadersBound`, in `include/bsp/game_hosts_lua.hpp`, committed
OFF.

- **Dispatch.** The host resolves the entity id to a units-host slot and its class id, and walks
  9.2's chain for that class. A key outside the chain returns no value, as now. The deck keys
  keep section 7's any-entity deviation; narrowing them to classes 09 and 45 is a separate
  change.
- **`unitcommand`.** This uses `GameUnitsHost::director_current_command_0071be40(index)` and
  `command_name_of(object)`.
  - An empty name or no current command gives `nocommand`.
  - An entity with no units-host slot gives nothing. This is a labelled substitution: every
    host unit slot is assumed to have a director.
- **`reconlevel`.** This builds a new table with number keys 0, 1 and 2.
  - The own side gets 2, or 0 once dead, and every other side gets `recon_pass.level(side, unit)`.
    These are the same sources `sync_recon_level_tables_0077b0c0` uses.
  - A side the pass does not cover gets 0.
  - Forced levels are not modelled. This is labelled.
- **`ammoType` and `state`.** These use the squadron's planes and their ordnance, and are bound
  only if the units host carries the ordnance flags 007EDAD0 tests. Otherwise they stay unserved
  and are named in the report.
- **`TorpedoStock`.** This is the gunnery host's spare stock plus its loaded tubes (packet
  `cc9_torpedo_stock`).
- **Not bound.** `TargetIsHome`, `owner`, the LandConvoy keys and the shipyard's `stock` have no
  measured caller.
- **Summary line.** The existing summary gains `class_keys=N`, with a per-key count.

### 9.7 Predictions, written before any code

The switch is ON against OFF, both on the same tree. The run lines are those of the cc9-ships2
logs named above, with lockstep 0.05 and an idle player.

| row | prediction |
| --- | --- |
| USN01 3200/3000 | identical: 0 GetProperty calls (exit 0, or 1 for masked noise) |
| USN02 9200/9000 | identical: 0 calls |
| USN04 4700/4500 | identical: 136 calls, all `slots`, all already served |
| LOMP06 1200/1000 | identical: 0 calls. The `reconlevel` row is unresolved here; see 9.5 |
| JM06 3200/3000 | served 0 -> 17, unserved 17 -> 0, all `unitcommand` |
| JM06, the orders | `NavigatorAttackMove` 17 -> fewer. After the first order the key reads `attackmove` while that command is current, so a check re-issues only after the command ends |
| JM06, gameplay | may move, but only through `Narwhal-class Submarine 01`'s order stream. Every other moved row would be a failed prediction |

The rows are resolved from `summary mission getproperty 0088bf80`, the `NavigatorAttackMove`
census row, and `pair_diff`'s per-entity tables.

### 9.8 no_ghidra_function

- **`006E2AC0`..`006E2BFD`, inclusive.** This is the projectile `owner` reader,
  `__thiscall(this, LuaFrame* frame, NativeString* key)`.
  - It has two `RET 8` exits, at `006E2BE0` and `006E2BFB`, followed by `INT3` at
    `006E2BFE`..`006E2BFF`.
  - Ghidra reports no function containing `006E2AC0`. The index's nearest preceding start is
    `006E2A20`, and the bytes before `006E2AC0` end in a `RET` at `006E2AB8` and `INT3` padding.
    The body was read from disk (`disasm-raw`), not from a Ghidra listing.
  - It sits in 10 vtables at +138h (9.2).

### 9.9 Ledger names

Each is a descriptive hypothesis, not a recovered symbol.
- Ten names were added to the ledger under packet `cc9_get_property_names`, after the read landed.
- `006E2AC0`'s name is proposed only, and is left to the lead's function definition.
- No Ghidra function was renamed.

| address | proposed name |
| --- | --- |
| `00927AD0` | `BSP_Entity_GetPropertyUnitCommand` |
| `00779BB0` | `BSP_Unit_GetPropertyReconLevel` |
| `006E2AC0` | `BSP_Projectile_GetPropertyOwner` |
| `007416F0` | `BSP_LandConvoy_GetProperty` |
| `007EF1C0` | `BSP_PlaneSquadron_GetProperty` |
| `007EDAD0` | `BSP_PlaneSquadron_AmmoType` |
| `007EEE20` | `BSP_PlaneSquadron_TargetIsHome` |
| `00815870` | `BSP_Ship_GetPropertyTorpedoStock` |
| `00810E90` | `BSP_Ship_CountLoadedTorpedoTubes` |
| `00758340` | `BSP_Mothership_GetProperty` |
| `006D0E60` | `BSP_Airfield_GetProperty` |

### 9.10 Uncertainty

- **Readers that do not chain.** The census finds only readers that chain back to `00927AD0`,
  plus the airfield. A class whose +138h slot holds an unrelated routine would be missed. The
  table in `docs/ENTITY_CLASS_IDS.md` was not walked slot by slot.
- **Unconfirmed callee ABIs.** `00B66710` (push string), `00B67930` (new table as result) and
  `00B665D0` (set number key) are named here from their use at these sites, not from their bodies.
- **`007B9400`** is the fourth ordnance test in `007EDAD0` (kind 4). It was not read, and a rocket
  test is only a guess.

### 9.11 The binding (packet `cc9_get_property_class_readers`)

`kGetPropertyClassReadersBound`, in `include/bsp/game_hosts_lua.hpp`, is committed OFF.

- `GameMissionLuaHost::run_get_property_class_readers` runs before the deck keys, because
  `00927AD0` and `00779BB0` run before any class reader.
- It answers `unitcommand` and `reconlevel` for a units-host slot. The slot is the entity's `ID`
  minus one.
- **`unitcommand`:**
  - it takes `GameUnitsHost::director_current_command_0071be40`;
  - no command pushes `nocommand`;
  - a named command pushes `command_name_of`'s name;
  - a command the host's class table does not name pushes nothing and is counted `unnamed`.
- **`reconlevel`:**
  - only a class past `00927AD0` reaches it (not 47h, 4Ah, 41h, 42h, 43h, 1Dh or 44h);
  - it pushes a new table with number keys 0, 1 and 2 from the gunnery host's recon pass;
  - the rule is the one `sync_recon_level_tables_0077b0c0` uses.
- **A new summary line**, `summary mission getproperty class readers`, counts both keys whether
  the switch is on or off. It also counts what each bound reader pushed.

**Changed from 9.6.**
- **`ammoType` and `state` are not bound.** The units host keeps an ordnance mask per slot
  (`unit_ordnance`), but the squadron's member walk and 007EDAD0's kind order are not exposed
  to the Lua host.
- **`TorpedoStock` is not bound.** The gunnery host exposes no spare-stock or loaded-tube reader.
  Its files are leased to cc9-ships2 for another packet.
- No measured mission reaches any of the three (9.5). They stay unserved, as `TargetIsHome`,
  `owner` and the LandConvoy keys do.

**Labelled substitutions:**
- An entity with no units-host slot gets no value. The image would still run `00927AD0` on it.
- Every units-host slot is taken to have a director.
- `reconlevel`'s levels come from the host's recon pass rather than from the records at
  `unit+1E8h`. Forced levels are not modelled. The own side reads 2 even before the first pass.

### 9.12 The pairs and the verdict

**Setup.**
- OFF is this tree's build of `6d62bcaf6`.
- ON is `tools/pair_export.py --commit 6d62bcaf6 --flip kGetPropertyClassReadersBound=true`
  (`local/gp_on`, SHA-256 `A955AC1F04EF`).
- Every run used `BSP_GUNNERY_RNG_STREAMS=1` and `BSP_DEATH_TABLE=1`, lockstep 0.05 and an idle
  player. The frame counts are those of 9.7, and each header was checked.
- The logs are `local/gp_{off,on}_<mission>.log` in the cc9-lua2 tree.

| row | OFF | ON | prediction | verdict |
| --- | --- | --- | --- | --- |
| USN01 3200/3000 | 0 calls | 0 calls | identical | held: pair_diff exit 1, gameplay identical; the only difference is the new summary line's `bound` |
| USN02 9200/9000 | 0 calls | 0 calls | identical | held: exit 1, as USN01 |
| USN04 4700/4500 | 136/136/0 | 136/136/0 | identical | held: exit 1, as USN01; 44 death rows and 81 unit rows identical |
| LOMP06 1200/1000 | 0 calls | 0 calls | identical, `reconlevel` unresolved | held: exit 1, gameplay identical; the `bound` line, masked sector-scan noise, and one movie-camera pose line at HUD frame 441 differ |
| JM06 3200/3000, keys | 17 calls, 0 served, 17 unserved | 18 calls, 18 served (18 `unitcommand`, all named) | served 0 -> 17 | held; the one extra call is below |
| JM06, the orders | `NavigatorAttackMove` 17 | 0 | 17 -> fewer | held; see below |
| JM06, gameplay | deaths 5, hits 424, damage 12144, shots 668 | deaths 4, hits 405, damage 10109, shots 652 | may move, but only through the Narwhal | **failed** (exit 3): the whole convoy battle moved |

**Why the orders fell to 0 rather than to 1.**
- The Narwhal already carries a current `attackmove` from its stage-init order, which targets
  `PlayerSub 03`.
- So the first check at :1162 reads `"attackmove"` and never re-issues.
- OFF re-targeted it at `Mission.PlayerUnit` (`PlayerSub 01`) 17 times.
- The binding summary shows `attackmove 17 -> 0` and `issued 22 -> 5`.
- The extra GetProperty call (18 against 17) is one more check within 3000 m on the changed
  course.

**The failed prediction.** Gameplay moved well beyond the Narwhal's own rows. The Narwhal keeps
its authored target instead of being re-aimed at the player every second, and the rest follows
from that:
- it closes to 70 m, not 424 m;
- it takes 0 damage instead of 1482, and survives: the only-OFF death row;
- it shoots 13 times, not 20.

As a consequence:
- `PlayerSub 03` sinks 9.35 s earlier;
- `US Cargo Transport 01`'s killer changes;
- 14 unit rows change;
- the other units' periodic `command target` resolves shift.

No other script order changed:
- orders issued go from 22 to 5, which is exactly the 17 re-issues;
- binding calls go from 198 to 183, which is those 17 less two more `GetPosition` calls
  (104 -> 106).

No other GetProperty key was asked. The prediction was wrong
about scope, because one submarine's survival cascades through a small convoy battle. It was not
wrong about the mechanism.

**The switch stays OFF, pending the lead's ruling.** The JM06 move is the image's own behaviour,
because `00927AD0` reports the current command name. Every identity row held. By the rule
"flip only if the verdict holds", the stated gameplay prediction failed. The recommendation is
ON.
