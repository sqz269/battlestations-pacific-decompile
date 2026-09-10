# Award / hint trackers in GGame::OnMove (packet `game_award_trackers`)

Addresses: 004e1ca0, 004e18e0, 0068ec10, 00692b00, 00692b60, 006926f0, 00692580, 00692fd0,
00692960, 00690fd0, 0068e600, 0068e5b0, 00690b00, 0068e450, 0068e4c0, 0068e730, 0068e780

## What the subsystem is

The 0A0h byte singleton cached in `00E198D0` is the in-game **hint** system, not an achievement
tracker. The identifiers the seven passes push (`BASICSHIP`, `ARTILLERY`, `CAPTURE1STGET`, …) are
keys into a definition map the singleton owns at `+30h`; `00690fd0` resolves a key through that map,
through the localisation table and through the `HintSystem` Lua binding, and its body holds the
string `hints.missionhint`. `docs/GAME_FRONTEND_STATES.md` records the same entry point being used
by `BSP_TutorialHint_AdvanceBasicStep` 0054e440 to walk `BASICPLANE` to `BASICPLANE2` to
`BASICPLANE3` when the pause menu opens, which is the same key space. The reviewed name already on
004e1ca0 is `BSP_AwardTracker_GetSingleton`, so the `BSP_AwardTracker_` prefix is kept for
continuity; read "award" as "hint" throughout.

## Where the block sits in the frame

`BSP_Game_OnMove` 004e4a40 is `__thiscall(this, float delta)`; the delta is the stack argument the
prologue leaves at `[esp+48h]`. The award block is 004e525e..004e52ba and is **not**
unconditional: it sits behind the simulation gate at 004e50b0, whose failing tests all jump to
004e53b4 and skip it (004e50c0, 004e511e, 004e512b). Both branches of the pause test reach it,
one by the jump at 004e517a and one by falling through at 004e5240.

The block is seven repetitions of `call 004e1ca0` / `mov ecx,eax` / `call <pass>`:

| # | Address | Pass | Argument |
| --- | --- | --- | --- |
| 1 | 0068ec10 | cooldown tick | raw delta, pushed as a float at 004e5263 |
| 2 | 00692b00 | queued hint drain | none |
| 3 | 00692b60 | unit class hints | none |
| 4 | 006926f0 | weapon hints | none |
| 5 | 00692580 | environment hints | none |
| 6 | 00692fd0 | capture / landing first-get | none |
| 7 | 00692960 | strategic map first-get | none |

Pass 1 is the only one that receives the delta, and it receives the **raw** OnMove argument, not the
scaled delta the world tick at 004e52ba uses.

## Singleton and lifetime

`004e1ca0`, no arguments, `RET`, returns `DAT_00E198D0`. Double-checked lazy singleton: outside the
lock it tests `00E198D0`, then takes the critical section at `BSP_SingletonLifetime_GetManager()+10h`
(with the manual recursion counter at `lock[1].DebugInfo`), re-tests, allocates 0A0h bytes through
00bf681b, constructs with 004e18e0 and registers the instance with `BSP_SingletonLifetime_Register`
00bd0c30. Allocation failure stores 0 and still registers 0. 40 callers.

`004e18e0`, `__fastcall(this)`, returns `this`, `RET`. It writes the vtable `00CE7F08` (one slot,
the scalar deleting destructor 004e1c80; there is no RTTI pointer at `00CE7F04`, the preceding bytes
are the string `screen_scene`), zeroes a run of scalars and builds the containers.

## Recovered layout of the 0A0h singleton

Container shapes come from the head nodes the constructor allocates and the `_Isnil` byte it sets:
a `std::_Tree` member is `{allocator pad, _Myhead, _Mysize}` and the node is
`{_Left, _Parent, _Right, _Myval, _Color, _Isnil}`, so the `_Isnil` offset gives the value size.

| Offset | Type | Evidence |
| --- | --- | --- |
| +00h | vtable pointer | 004e1902 stores 00CE7F08 |
| +04h | byte flag | **not written by the constructor**; 00690fd0 reads it and clears it on the first call |
| +08h..+14h | four zeroed dwords | 004e190c..004e1915 |
| +18h | int, last unit class id | **not written by the constructor**; only 00692b60 reads and writes it |
| +1Ch, +20h, +28h, +2Ch | zeroed dwords | 004e1918..004e1921 |
| +30h | `map<NativeString, HintDefinition>` | head from 004c2a80, `_Isnil` at node+13Dh so the value is 130h bytes (8 byte key + 128h byte definition) |
| +3Ch | `map<NativeString, float>` cooldowns | head from 00443e20, `_Isnil` at node+19h so the value is 0Ch bytes |
| +48h | `list<NativeString>` | head from 004c3020; walked by 00690fd0 and 0068e600, node value at +8h |
| +54h | `list<NativeString>` | head from 004c3020; no routine in this packet reads it |
| +60h | `NativeString` active hint name | length at +60h, buffer at +64h; 0068e5b0 copies it out |
| +68h | `deque<NativeString>` | `_Map` +6Ch, `_Mapsize` +70h, `_Myoff` +74h, `_Mysize` +78h; 0068e730 indexes it two elements per block with an 8 byte stride |
| +7Ch, +88h, +94h | three `map`s with 14h byte values | heads from 004c2ad0, `_Isnil` at node+21h; unread by this packet |

The object ends at 0A0h, which the last container fills exactly.

## Shared gates

Six of the seven passes open with the same two tests.

- `004bca50` on `00E188A8` returns the effective game mode: `game+614h`, except that when
  `game+61Ch` is zero and `game+1FE4h` is zero and the value is neither 8 nor 9 it returns 8.
  Every pass but 00692fd0 returns immediately when the result is **9**.
- `005b5d50` on `*(00E198C4+A4h)` returns `*(float*)(this+C0h) != 0.0f` (the constant at 00D7A218 is
  `00 00 00 00`; the `LAHF` / `TEST AH,44h` / `JNP` sequence yields "not equal, or unordered").
  Every pass but 00692fd0 returns when it is true. Reading: a screen transition or fade is running.
- Most passes also require `00E188D8` (the local player unit) to be non-null, and call its vtable
  slot at `+5Ch` with a category index. That index space is **not** the class id space of the name
  table: 6 is the ship category, 8 submarine, 0Fh and 18h the two plane categories, 0Bh recon plane,
  1Bh a gunnery seat.

## Pass 1 — cooldown tick 0068ec10

`__thiscall(this, float delta)`, `RET`, no return value.

Walks the map at `+3Ch` from `_Myhead->_Left` to `_Myhead`. For each entry it loads the float at
`value+8h`, subtracts the delta and writes it back through the mapped-value accessor 00444be0
(0068eca6..0068ecbc). It then tests `COMISS xmm0(0.0), [node+14h]` with `JBE` to skip, so an entry
is collected only when it is **strictly below zero**; an entry resting on exactly `0.0f` survives.
Collected pairs go into a temporary `std::list` built by 004c3020 / 004ce6f0. A second loop walks
that list, re-finds each key with 00443d60, confirms the match with `_stricmp` 00bf7fbf on the key
buffer (guarded by a `key.length != 0` test) and erases the entry with 00444490.

Cooldown entries are inserted by 00690fd0: when a hint definition's type string compares equal to
`"cooldown"` under `_stricmp` and its duration field is non-zero, 00690fd0 inserts
`(float)duration` under the hint name, and refuses the hint entirely while the key is present.

## Pass 2 — queued hint drain 00692b00

`__fastcall(this)`, `RET`. Runs when the game mode is not 9, no transition is blending,
`this+78h` (the deque size) is non-zero and `this+60h` (the active hint length) is zero. It calls
`front()` 0068e730 on the deque at `+68h`, hands the resulting `NativeString*` to 00690fd0 with the
third argument **1**, then tail-jumps to `pop_front()` 0068e780. The `push 1` at 00692b37 is not
consumed by `front()`, which takes no stack argument, so it becomes 00690fd0's forced flag; that is
worth stating because the pseudocode makes it look like an argument to the wrong call.

## Pass 3 — unit class hints 00692b60

`__fastcall(this)`, `RET`. Gates, in order: mode != 9; no transition; `00E188D8` non-null; the class
id from the virtual at slot 0 of the sub-object at `player+170h` differs from `this+18h`; if
category 18h then `*(00E198C4+68h)+8h` must be set; if category 6 then `*(00E198C4+78h)+8h` must be
set.

It then tries the three category hints, each recorded with the forced flag clear and each skipped
when `007f8e00(gGame+650h, name)` already reports it. `gGame+650h` is a profile-side set; 007f8e00
lives in a segment whose keywords include `unlocks` and `seenunlocks`.

| Test | Hint | Literal |
| --- | --- | --- |
| category 8 | `BASICSUB` | 00CEE384 |
| not 0Fh and not 18h, then category 6 | `BASICSHIP` | 00CEE39C |
| category 0Fh or 18h | `BASICPLANE` | 00CEE3C0 |

If none of those fires it reaches 00692dd0, where it builds an empty `NativeString` (00CE3A0C is
four zero bytes) and computes
`activeName != "" && BSP_AwardTracker_IsActiveHintBlocking(&activeName)`. 00690b00 returns the byte
at `+114h` of the active hint's definition, looked up with 005ffb20 in the map at `+30h`. When both
hold, the pass returns: a blocking hint is on screen. Otherwise it stores the new class id at
`+18h` and records the class name.

Class ids, in the order the native compares them (00692e4b..00692f72):

| Id | Hint | Literal |
| --- | --- | --- |
| 0 | `MOTHERSHIP` | 00CF7E38 |
| 1 | `DESTROYER` | 00CF7E2C |
| 2 | `PT` | 00CF7E28 |
| 0Dh | `SUBMARINE` | 00CF7E1C |
| 3 | `BATTLESHIP` | 00CF7E10 |
| 4 | `CRUISER` | 00CF7E08 |
| 5 | `TROOP_TRANSPORT` | 00CF7DF8 |
| 6 | `LST` | 00CF7DF4 |
| 7 | 007edad0 == 5 → `PARATROOPER` (00CF7D40); == 6 → `OHKA_PAYLOAD` (00CF7DE4); else `LEVELBOMBER` (00CF7DD8) | |
| 8 | `DIVEBOMBER` | 00CF7DCC |
| 9 | `TORPEDOBOMBER` | 00CF7DBC |
| 0Ah | `FIGHTER` | 00CF7DB4 |
| 0Bh | `RECONPLANE` | 00CF7DA8 |
| 0Ch | `*(*(player+3D0h)+538h)+144h` → `OHKA` (00CF7DA0) else `KAMIKAZE` (00CF7D94) | |
| 0Fh | `COMMANDBUILDING` | 00CF7D84 |

Any other id falls through to the destructor label and records nothing.

## Pass 4 — weapon hints 006926f0

`__fastcall(this)`, `RET`. Extra gates before the shared pair: the byte at `+19h` of the active
player slot record `*(gGame+18CCh + *(gGame+18ECh)*4)` must be clear; after the shared pair, a copy
of the active hint name taken with 0068e5b0 must be **empty** (`CMP [EAX],0` at 00692764); then the
same 18h / 6 sub-checks as pass 3.

Two branches. Categories 6 and 1Bh take the surface branch, a jump table at 006928e3 on
`*(*(00E198C4+50h)+44h)`: 1 and 2 → `AAFLAK`, 3 → `ARTILLERY`, 4 → `TORPEDO_SHIP`, 5 → `DC_SHIP`.
Otherwise categories 0Fh and 18h take the air branch, an if-chain on `007bca50` called on the local
unit, or on `*(player+3D0h)` when the category is 18h: 9 → `BOMB`, 0Ah → `TORPEDO_PLANE`,
12h → `ROCKET`, 0Bh → `DC_PLANE`, 0Ch or 0Fh → `PARATROOPER`, else `MACHINEGUN` when the byte at
`unit+C24h` is set. Everything else falls through.

The pass builds an **empty** name at 006927e3 and always reaches the 00690fd0 call at
00692916, so a fall-through path calls 00690fd0 with an empty key; the definition-map find inside
00690fd0 then returns end() and the call does nothing.

## Pass 5 — environment hints 00692580

`__fastcall(this)`, `RET`. Requires mode != 9, no transition, `00E188D8` non-null, category 6, and
the byte at `*(player+538h)+D0h` set. It reads two floats off the sub-object at `player+A20h`:
00939f80 returns `+34h` and 00939f70 returns `+38h`. Each is converted with `_ftol` 00bf7420 before
the test, so a reading inside `(-1.0, 1.0)` truncates to zero and does not trip.

Priority: `WATER` (00CF7D00) when the `+34h` reading is non-zero, else `FIRE` (00CF7CF8) when the
`+38h` reading is non-zero, else `PERISCOPE` (00CF7CEC) when the unit is category 8 and the dword at
`player+122Ch` is 2, else `ENGINE` (00CF7CE4) when the byte at `player+9E5h` is set, else nothing.
The chosen name is recorded only when `BSP_NativeString_NotEqualsInsensitive(this+60h, name)` is
true, that is, when it is not already the active hint.

## Pass 6 — capture and landing first-get 00692fd0

`__fastcall(this)`, `RET`. This is the only pass that does **not** consult the game mode or the
transition flag. It requires `00E188D8` non-null and category 6.

It first asks `BSP_AwardTracker_HasHintBeenShown` 0068e600 for `LANDING1STGET` and, only if that
says no, for `CAPTURE1STGET`. If either has already been shown the pass returns. 0068e600 checks
`007f8e00(gGame+650h, name)` first and then walks the list at `this+48h`.

It then walks the zone list at `*(*(gGame+19CCh)+16Ch)`, each node being `{?, next, payload}` with
`next` at +4h and the payload pointer at +8h. For each payload, both the player and the payload get
a lazy transform refresh (`00414db0` when the byte at +C8h is clear), and the squared distance uses
the floats at +FCh, +100h and +104h of each. Both radii are **ints**, loaded with `FILD` and squared
on the x87 stack:

- `distance² <= (float)(payload+7C4h)²` and `payload+54h == 2` marks the player as inside the
  landing radius.
- inside that, `(float)(payload+7A0h)² < distance²` **breaks the whole loop** at 00693282 rather
  than continuing to the next zone; otherwise the player is inside the capture radius.

The distance is recomputed identically for the second test. If the capture radius was reached the
pass records `CAPTURE1STGET`; otherwise, if only the landing radius was reached and the unit is
category 0Bh, it records `LANDING1STGET`. Each record is guarded again by 0068e600 and by the
not-equal test against the active hint name.

## Pass 7 — strategic map first-get 00692960

`__fastcall(this)`, `RET`. Requires mode != 9, no transition, `SM1STGET` (00CF7D78) not already
shown per 0068e600, `0066ffa0` on `*(00E198C4+BCh)` true, `00652a30` on the same object false (it is
one instruction: the byte at `+8h`), and `this+60h` empty. It then records `SM1STGET` with the
forced flag clear, subject to the same not-equal test.

## Callees outside the packet

| Address | Role established here |
| --- | --- |
| 00690fd0 | the record/show entry point; `__thiscall(this, NativeString* name, char forced)`. Order: clear `+4h`, find `name` in `+30h` and return when absent, walk `+60h`/`+48h`, consult `007f8e00`, insert the `"cooldown"` timer into `+3Ch`, then localisation and the GUI notification. 24 callers |
| 0068e600 | "has this hint already been shown"; `RET 4` |
| 0068e5b0 | copies `this+60h` into a caller string and returns it; `RET 4` |
| 00690b00 | returns the byte at `+114h` of the active hint's definition; `RET 4` |
| 0068e450 / 0068e4c0 | `find` on the maps at `+30h` and `+3Ch` |
| 0068e730 / 0068e780 | `front()` and `pop_front()` on the deque at `+68h`; STL instantiations |
| 005ffb20 | mapped-value lookup in the definition map |
| 007f8e00 | profile-side "already unlocked / seen" query on `gGame+650h` |
| 00449af0 | `NativeString` case-insensitive **not-equal**, `RET 4`; the empty-string cases return "different" |

## Calling conventions and RET sizes

All seven passes are `__fastcall`/`__thiscall` with ECX holding the singleton, and all end in a
plain `RET`. Only 0068ec10 takes a stack argument (one float, pushed with the `push ecx` / `fstp
[esp]` idiom at 004e5263), and it does not clean it up: the caller's stack discipline is cdecl-style
for that one float. 004e1ca0 takes no arguments and returns the pointer in EAX. 004e18e0 is
`__fastcall(this)` and returns `this`. 0068e600, 0068e5b0, 00690b00 and 00449af0 are `__thiscall`
with one stack argument and `RET 4`. 00690fd0 is `__thiscall` with two stack arguments.

Every pass except 00692b00 installs an SEH frame (`00C7E178`, `00C7E7E8`, `00C7E881`, `00C7E7C8`,
`00C7E904`, `00C7E821`) for the temporary `NativeString`s; those unwind paths are not modelled.

## Reconstruction

`include/bsp/award_trackers.hpp` and `src/award_trackers.cpp` carry the layout offsets, the four
string tables in native order, one function per pass decision with explicit inputs, and
`run_award_tracker_frame`, a sequence routine over `AwardTrackerFrameHost` with one method per
native call site in the block, in the style of `bsp::run_application_frame`. Nothing there allocates
or touches Ghidra state, and no global or type was invented: the definition map, the localisation
path and the GUI notification are left to the host.

## Uncertainties and what remains

- The subsystem's real class name is unrecovered. The vtable has no RTTI pointer and the singleton
  has no string of its own. `AwardTracker` is inherited from the existing reviewed name.
- `+4h` and `+18h` are read before anything writes them on a fresh instance. Either a loader writes
  them between 004e1ca0 and the first update, or the native genuinely reads uninitialised heap. Not
  resolved.
- The 128h byte hint definition is only partly mapped: the type string and duration used for
  `"cooldown"`, and the blocking byte at `+114h`. The rest of 00690fd0 (localisation,
  `hints.missionhint`, the GUI notification, the profile write) was read for context only.
- The three maps at `+7Ch`, `+88h` and `+94h` and the list at `+54h` are untouched by this packet.
- `005b5d50`'s owner `*(00E198C4+A4h)` and `0066ffa0`'s owner `*(00E198C4+BCh)` are named from
  segment keywords (`sm_cp`, `ingamegui`), not from evidence inside this packet.
- The category index space of the vtable slot at `+5Ch` is only partly known: 6, 8, 0Bh, 0Fh, 18h
  and 1Bh appear here, with no enumeration of the rest.
- The break at 00693282 looks like a bug in the original (one zone outside its capture radius stops
  the scan), but it is what the binary does and is reproduced as such.

## State reached

| Address | State |
| --- | --- |
| 004e1ca0 | analyzed (already reviewed by `game_on_move_map`), evidence extended |
| 004e18e0 | analyzed, full layout recovered |
| 0068ec10 | reconstructed, build-tested, one boundary case in `tests/math_tests.cpp` |
| 00692b00 | reconstructed, build-tested |
| 00692b60 | reconstructed (decision and tables), build-tested |
| 006926f0 | reconstructed (decision and tables), build-tested |
| 00692580 | reconstructed, build-tested |
| 00692fd0 | reconstructed (zone scan), build-tested |
| 00692960 | reconstructed (gate), build-tested |
| 00690fd0, 0068e600, 0068e5b0, 00690b00 | analyzed only |

All nine packet addresses already have Ghidra functions; none needed to be defined. Ghidra was read
only, and no rename, comment or save was applied from this worktree.
