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

**Blocked, not skipped.** The run is one call once the session is back:

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
