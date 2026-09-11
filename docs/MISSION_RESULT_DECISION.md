# Mission result decision (packet `mission_result_decision`)

Addresses: 008b8ad0, 00906460, 00927f60, 004cd390, 0089a390, 0089a480, 004d7ea0, 004d7970,
009205e0, 008cd440, 008bd340, 008bd900, 008e1f80, 008dd5c0, 008e20d0, 008e2200, 008e1d30,
008dfe50, 00905350, 0090f0e0, 00910480, 00959450, 007f3b10, 0093bed0, 0093c300, 0095abe0

## Summary

**Nothing in the image decides that a mission is won or lost.** The decision is entirely
scripted. The native side offers the mission script four write paths (objective state,
per-slot scoring record, end-of-mission movie, end-of-mission request) and one poll that
turns the script's last write into a state transition. The five addresses
`docs/MISSION_EVENTS_UPDATE.md` proposed as `mission_failure_decision` are **not** mission
failure: three of them are unit-subsystem damage simulation and two are unit-death
reporting. They feed the warning event director, which a mission script subscribes to.

The full chain, from the script to the debrief:

| Step | Routine | What it does |
| --- | --- | --- |
| script sets an objective | `008cd440` / `008bd340` / `008bd900` bindings | writes objective `+1Ch` = 1 or 2 and announces it |
| script scores the slot | `008b8ad0` -> `00906460` | writes record `+0h` = completed, `+10h` = mission clock |
| script raises the end movie | `0089a480` -> `0089a390` -> `004cd390` | allocates the 24h result object at `game+7188h` with `+21h` = "go to debrief" |
| OnMove polls | `004d7ea0` `BSP_Game_CheckMissionCompletion` | plays the movie, and when `+21h` is set enqueues **state request 0Fh** |
| drain dispatches 0Fh | `004e4430` -> `004d7970(0)` | `GGame::EndScene`: commits the scoring record into the profile, enqueues **10h** |
| drain dispatches 10h | teardown arm `004e458a` | leaves state 0Dh, enters state 11h, then enqueues 04h |
| drain dispatches 04h | `004e4000` `BSP_Game_EnterFrontEndShell` | the debrief front end |

**The request that leaves state 0Dh is 0Fh.** It is the only request `004d7ea0` ever
enqueues, and `004d7970` only enqueues 10h when the current request `game+5D4h` is 0Dh or
0Fh, so 0Fh is both the trigger and the guard.

## The objective model

### The record, constructor `008dd5c0`

`__thiscall(Objective* this, const NativeString* id, const NativeString* text, int kind,
char flag)`, `RET 10h` (008dd69d). `operator new(0x2Ch)` at 008e1f96 sizes it.

| Offset | Field | Evidence |
| --- | --- | --- |
| `+00h` | vtable `00D16038` | `MOV dword ptr [ESI],0xd16038` at 008dd5e1 |
| `+04h` | `id` length | zeroed at 008dd5eb, filled by `0041dd40` at 008dd62a |
| `+08h` | `id` pointer | `_memcpy` destination `[EDI+4]` at 008dd63a |
| `+0Ch` | `text` length | zeroed at 008dd5f7, filled at 008dd657 |
| `+10h` | `text` pointer | `_memcpy` destination `[EBX+4]` at 008dd666 |
| `+14h` | byte flag (5th argument) | `MOV byte ptr [ESI + 0x14],AL` at 008dd680 |
| `+18h` | `int kind` | `MOV dword ptr [ESI + 0x18],EDX` at 008dd684 |
| `+1Ch` | `int state`, initialised 0 | `MOV dword ptr [ESI + 0x1c],0x0` at 008dd687 |
| `+20h..+28h` | unit list (`008db4f0` builds the head at `+24h`, size `+28h`) | 008dd5fc..008dd60e |

A `NativeString` here is `{int length; char* pointer}`: the key comparison in `008e20d0`
reads `*param_2` as the length and `param_2[1]` as the pointer, and `004cd390` frees its
string with length `[ESI+14h]` and pointer `[ESI+18h]`.

**Kind is primary / secondary / hidden.** Three independent witnesses agree:

- `008dfe50` refreshes the HUD markers only when `objective+18h != 2` (008dfe63).
- `008e1d30` announces the state change only when `objective+18h` is 0 or 1.
- `include/bsp/mission_progress.hpp` already carries `objectives_204` as six persisted
  trees, "allied primary/secondary/hidden, then Japanese primary/secondary/hidden".

**State is active / completed / failed**: `008e20d0` stores 1 at 008e2181, `008e2200`
stores 2 at the matching site. `+1Ch` is the only field either of them writes.

### The per-slot objective set

The set is `__thiscall` with `this+14h` = the local player slot (0..7) and `this+28h` =
the head of an MSVC `std::list<Objective*>` (node `+0h` next, `+8h` value).
`008cd440` reaches eight of them through `game+21A4h + 4*slot` (the loop bound
`iVar11 < 0x21c4` at the tail of `008cd440`), selected by a bitmask the binding builds by
comparing each Lua integer argument against `[game+18CCh + 4*i] + 28h`, the party id of
local player slot `i`.

`008e1f80` `ObjectiveSet::Add`, `__thiscall`, `RET 10h` (008e20c2):
allocate 2Ch, construct, replicate, `push_back`, clear the HUD dirty byte at
`[[00e198c4]+60h]+65h`, and when the 4th stack argument is zero also run `008e1b90`.

`008e20d0` (completed) and `008e2200` (failed), `__thiscall(ObjectiveSet* this,
const NativeString* id, X, Y)`, `RET 0Ch` (008e21f8 / 008e2328). Both walk the list,
compare the key against `objective+4h/+8h` with the same three-way rule
(equal lengths, then both-empty / one-empty, then `__stricmp`), and on a match:

```
008e2160  MOV EDI,[ESP+0x54]     ; Y
008e2164  MOV EBX,[ESP+0x50]     ; X
008e2168  PUSH EDI
008e2169  PUSH 1                 ; 0 in 008e2200
008e216b  PUSH EBX
008e216c  PUSH ESI               ; the objective
008e216d  MOV ECX,EBP            ; the objective set
008e216f  CALL 0x008e1d30
008e2174  PUSH ESI
008e2175  MOV ECX,EBP
008e2177  CALL 0x008dfe50
008e217c  MOV ECX,0x1
008e2181  MOV dword ptr [ESI + 0x1c],ECX
```

so `008e1d30` is `__thiscall(ObjectiveSet*, Objective*, X, int completed, char silent)`,
`RET 10h` (008e1e51) - Ghidra's decompiled argument list is shifted by one and its
callers appear to drop an argument; the listing above is the authority.

`008e1d30` gates on `0 <= game+18ECh < 8`, `set+14h == game+18ECh` (the local player owns
this set) and `objective+18h` in {0,1}. When the `silent` byte is clear it computes
`2 - (completed != 0)` - **1 for completed, 2 for failed** - and passes it to `00432650`
and `008dd460`, then builds a message from `00CE3A0C` through `00449af0`/`00734870`.

Replication: when `game+1FE4h == 1` and `[game+18CCh + 4*set+14h] + 50h` is non-null,
`008dda20` (completed) or `008ddb00` (failed) builds a message and
`BSP_Session_SendMessageToNonlocalPeer` sends it.

### Where the objective text is read

The runtime set above is **not** what the briefing shows. `0058f5b0`, the
`_pri_objective_Group` / `_sec_objective_Group` builder, reads the 154h side block of the
434h mission-tree record (`0051cc90(missionKey*0x154 + 0xBC + base)`), i.e. the
`MissionTree` Lua data of `docs/MISSION_TREE_LUA_READER.md`. The HUD pause page
`GUI_pause_objectives` (slot 3Ch, `00602280`) is the in-mission reader. Only the runtime
set carries state; the briefing carries text.

## The win path

### `Scoring_SetMissionCompleted`, binding `008b8ad0`

`__fastcall(LuaState*)`, `RET` with no immediate (008b8c9f). The Lua name comes from the
registration pair at `00e0c380` = `{00d0f52c "Scoring_SetMissionCompleted", 008b8ad0}`,
corroborated by the body's own `luaMW_Scoring_SetMissionCompleted failed:` literal.
Argument handling, 008b8bad..008b8c54:

```
008b8bad  MOV EAX,[0x00e188a8]
008b8bb2  CMP dword ptr [EAX + 0x1fe4],ESI   ; local player mode
008b8bb8  MOV EDI,dword ptr [EAX + 0x18ec]   ; default slot = the local slot
008b8bc7  JZ  0x008b8bfe                     ; mode 0 -> no explicit slot argument
...       EDI = LuaObject_GetInteger(arg 0); ESI = 1
008b8bfe  ...  if (argc > ESI) AL = LuaObject_GetBoolean(arg ESI) else AL = 1
008b8c46  MOV ECX,[0x00e188a8]
008b8c4c  MOV ECX,dword ptr [ECX + 0x21a0]   ; ECX = the scoring manager
008b8c54  CALL 0x00906460
```

So in single player (`game+1FE4h == 0`) the call is `SetMissionCompleted([completed])` on
the local slot; in the other modes it is `SetMissionCompleted(slot [, completed])`.
`completed` defaults to **true**.

### `00906460`, the completion write

`__thiscall(ScoringManager* this, int slot, byte completed)`, `RET 8` (009064c0).

```
00906460  MOV EAX,[ESP+4]
00906464  IMUL EAX,EAX,0x284
0090646a  CMP dword ptr [EAX + ECX + 0x4],0x0
0090646f  LEA EAX,[EAX + ECX + 0x4]          ; record = this + 4 + slot*284h
00906473  MOV CL,byte ptr [ESP+8]
00906477  JNZ 0x0090648a
00906479  TEST CL,CL
0090647b  JZ  0x0090648a
0090647d  MOVSS XMM0,dword ptr [0x00f876a4]  ; the mission clock
00906485  MOVSS dword ptr [EAX + 0x10],XMM0
0090648a  MOVZX ECX,CL
0090648d  MOV dword ptr [EAX],ECX
```

The completion timestamp is written **only on the 0 -> set edge**; re-setting an already
completed record leaves `+10h` alone, and clearing never touches it. The flag itself is
the zero-extended byte, so it is 0 or 1.

Then, unconditionally, it walks `[[game+19CCh]+28h]` (next at `+4h`, value at `+8h`) and
calls `00927f60` on every value whose `+180h` dword is `<= 7` (`CMP ..,0x7 / JA`). That
routine stamps the clock into `entity+1D0h` and, through the entity's virtual `+5Ch`
queries 0Fh and 5, re-runs `0090ed40`; it reads like an AI re-target kick and is left at
that.

### The scoring record, `[game+21A0h] + 4 + slot*284h`

`004d7970` indexes the array as `[obj]+4+[obj+1424h]*284h`, and `1424h = 4 + 8*284h`
exactly, so the manager holds **eight 284h records at `+4h`** followed by the index of the
record to commit at `+1424h`. This closes the open question in
`docs/GAME_WORLD_ENTITIES.md`: the "0x284-stride records at `this+0x1E4`" it found are the
same eight records read at field `+1E0h`, not a second array overflowing the allocation,
and `00916980`'s "eight records at `this+4h`" (`docs/GAME_TITLE_INIT.md`) is the reset.

The record is the one `include/bsp/mission_progress.hpp` already persists as
`MissionScoreRecord`, confirmed field by field by the Lua setters:

| Native write | Offset in the record | `MissionScoreRecord` field |
| --- | --- | --- |
| `00906460` | `+00h` | `mission_completed_00` |
| `00905350` `Scoring_SetRanking` (`[EAX+ECX+8]`, `RET 8`) | `+04h` | `ranking_04` |
| `00906460` | `+10h` | `completion_time_10` |
| `00910480` `Scoring_AddMissionScore` (`LEA ECX,[EDX+ECX+0x1c]`, `RET 0Ch`) | `+18h` | `score_maps[Mission]` |
| `0090f0e0` `Scoring_SetMissionMedal` (`LEA ECX,[EDX+ECX+0x58]`, `RET 0Ch`) | `+54h` | `score_maps[MissionMedals]` |
| read by `009205e0` | `+1E0h` | `totals_1c8[6]`, the total score |

The keyed setters go through the map indexer `005070c0`.

### `GGame::EndScene`, `004d7970`

`__thiscall(GGame* this, char aborted)`, `RET 4` (004d7b01). The name is the routine's own
`GGame::EndScene()` profiler literal at 004d7986. It is the handler for request 0Fh and
also the multiplayer/quit end.

Body, once per scene (`game+1EE2h` latches it):

- `game+1EE1h = aborted`.
- `00920a20(aborted)` - the debrief bring-up.
- **`aborted == 0`** (the normal end):
  ```
  004d79cc  MOV ECX,dword ptr [EDX + 0x6b4]   ; the MissionProgress object
  004d79d3  MOV EDI,dword ptr [EAX + 0x1424]  ; the slot to commit
  004d79df  LEA EAX,[EDI*0x284 + EAX + 0x4]   ; its record
  004d79e3  PUSH EAX
  004d79e4  ADD EDX,0x2198                    ; the current mission key string
  004d79ea  PUSH EDX
  004d79eb  CALL 0x009205e0
  ```
  then `BSP_Game_AdjustMissionStartCounters(0)` (`004bcaa0`) when `game+1FE4h != 0`,
  `game+218Ch == 0` and `game+624h == 0`.
- **`aborted != 0`**: `007fa1b0(0, BSP_StorageOperation_EmptyContinuation)` on `game+650h`
  and, in single player, `00916980` on `game+21A0h` (`BSP_MissionPlayerRecords_Reset`) -
  the records are thrown away rather than committed.
- Then, when `game+5D4h` is 0Dh or 0Fh: broadcast end-of-scene in mode 1, send session
  message 13h to the first secondary peer in mode 2, and `BSP_Game_EnqueueStateRequest(10h)`.

### `009205e0`, the commit

`__thiscall(MissionProgress* this, const NativeString* missionKey,
const MissionScoreRecord* record)`, `RET 8` (00920683). ECX comes from `game+6B4h` and the
key from `game+2198h`, which `docs/MISSION_TREE_BRIEFING_SCREENS.md` establishes as the
mission id the mission start writes.

```
dst = mission_record_00594a70(this, key);   // std::map<NativeString,284h record>
0091a080(dst, record);                      // copy the record in
dst[+284h] += 1;                            // the play counter, count_284
if (game+1FE4h == 0)
    single_best_scores_0c[key] = max(single_best_scores_0c[key], record[+1E0h]);
else if (00f8a2fc->vtable[198h]() && [00f8a2fc]+4Dh)
    multi_scores_18[key] = multi_scores_18[key] + record[+1E0h];
```

Single player keeps the **best** total; multiplayer **accumulates** it. Both maps and the
record map already exist in `include/bsp/mission_progress.hpp`, which also carries the
archive readers and writers, so nothing here is re-modelled.

### The end-of-mission movie, `004cd390`

`__thiscall(GGame* this, const NativeString* name, char goToDebrief)`, `RET 8` (004cd43e).
It first frees any existing `game+7188h`, destroying its embedded string
(length `+14h`, pointer `+18h`), then `operator new(0x24h)`, `004cc3f0(name)`,
`+8h = [00ce77e4]` (a float constant), `+21h = goToDebrief`.

Its only caller is `0089a390`, which prefixes `movies/` to the script's name, and that
caller's only caller is `0089a480`, the Lua binding registered at `00e0ba60` as
`{00d10b84 "PlayBinkMovie", 0089a480}`. So the script says
`PlayBinkMovie(name, goToDebrief)` and the debrief is a **parameter of the movie call**.

`include/bsp/game_frame_control.hpp` already models the polled projection as
`bsp::MissionResult { present, requests_debrief, score }`; this packet adds the producer
side and the name string.

### The poll, `004d7ea0`

`BSP_Game_CheckMissionCompletion`, `__thiscall(GGame*)`, `RET`. Already recorded by
`game_frame_control`. It runs only with an empty request queue and a non-null `game+7188h`,
ticks `[game+19CCh]` three times with a zero delta around
`BSP_EntityWorld_FlushActivations`, plays the movie
(`BSP_MoviePlayer_Play(result+14h, 0, result+8h, 0)`), and enqueues 0Fh with
`game+5ECh = 1` when `result+21h` is set. It then releases `game+7188h`.

A `PlayBinkMovie(name, false)` therefore plays a movie mid-mission and the mission
continues; `PlayBinkMovie(name, true)` is the end of the mission, win or lose. **The
native side never distinguishes the two outcomes at this point** - victory and defeat
differ only in what the script wrote into the objectives and the scoring record first.

## The loss path: what the five proposed addresses actually are

| Address | Convention | What it is |
| --- | --- | --- |
| `0093bed0` | `__thiscall(this, param, float delta)`, `RET 8` (0093c116) | random subsystem failure roll |
| `0093c300` | `__fastcall(this)`, `RET` (0093c51e) | random pick from the failure descriptor table |
| `0095abe0` | `__thiscall(this, Message*)`, `RET 4` (0095ae1d) | a unit's message handler, one arm of which is `InferiorFailure` |
| `007f3b10` | `__fastcall(this)`, `RET` | aircraft destruction: reports the kill |
| `00959450` | `__fastcall(this)`, `RET` | unit destruction: reports the kill and raises the limbo screen |

None of them reads or writes an objective, the scoring record, `game+7188h` or the state
request queue.

### `0093bed0` and `0093c300`, the `failure` channel

`0093bed0` resolves a component from `008782a0(param+30h, param+34h)`, forms
`p = (c[+28h] * delta) / c[+2Ch]` (or, when either float is negative, the ratio of two
globals at `[00424c40()+3DCh]` and `[..+3E0h]` times delta), draws
`00bd2f10(0, 1.0f)` and fires when the draw is `<= p`. That is a per-frame hazard rate,
i.e. mean-time-between-failures for a ship component, gated on `game+1FE4h` being 0 or 1.
On a fire it builds a failure record, calls `BSP_WarningManager_FireFailure` (`00982c50`)
and `00913d80`, and runs the owner's virtual `+21Ch`. `0093c300` picks a uniformly random
enabled row from the 14h-stride descriptor vector at `[00424c40()+3E8h..+3ECh]` and fires
the same channel. Segment 61's keywords (`hajobelso`, `periszkop`, `fizika_`) place both in
ship-interior damage.

### `007f3b10` and `00959450`, the `kill` channel

`007f3b10` (aircraft) reports when `unit+70h == 1`, the world gate byte
`[game+19CCh]+4ACh` is set, and the controller at `unit+35Ch` answers false to virtual
`+18h(17h)` or `unit+36Ah` is clear. `00959450` (unit) has the same shape plus a mission
start grace period taken straight from the listing:

```
00959460  MOVSS XMM0,dword ptr [0x00f876a4]
00959468  COMISS XMM0,dword ptr [0x00d7a24c]   ; 1.0f (00 00 80 3f)
0095946f  JBE 0x0095951e                        ; clock <= 1.0f -> no report
00959475  CMP dword ptr [EDI + 0x70],0x1
00959479  JNZ 0x0095951e
0095948a  CMP byte ptr [ECX + 0x4ac],0x0
00959491  JZ  0x0095951e
```

so a unit destroyed in the first second of a mission is not reported at all. The report
itself is `009813a0`, the dispatcher `docs/MISSION_EVENTS_UPDATE.md` lists for the `kill`
and `globals.warn_uslost` channels; `0091bda0` is the alternative for a unit whose owner
answers the `17h`/`6` queries with a set byte at `unit+C41h` or `unit+100Ah`.

The player-facing half runs only when the dead unit is the controlled unit
(`00959450` compares against `004b4b00()`):

- with the world gate set and the front-end state `[00e198c4]+20h` not in
  {29h, 2Bh, 2Ch, 2Dh}: register the limbo page with `00565fb0(unit+70h)`; then, when
  `0068a120()` is false and the front end is idle
  (`[+4h] == [+20h] && [+1Ch] == [+38h]`), **`BSP_FrontEndManager_PushInterfaceRequest(34h, 0)`**.
  Interface 34h is the one that raises HUD slot 32h, `GUI_limbo`
  (`docs/HUD_SCREEN_PAGES.md`), i.e. the death / respawn screen, not a defeat screen.
  When the front end is mid-transition it calls its virtual `+10h` with the pending pair
  instead; when `0068a120()` is true it only raises the byte `[00e198c4]+FDh`.
- then, when the unit's virtual `+5Ch(0Fh)` is false or `[unit+9D4h]+3CCh < 2`,
  `BSP_Game_SetControlledUnit()`.

So the native answer to "the player died" is **respawn**, never defeat. Defeat is a
script reaction to the `kill` warning.

## Calling conventions and RET sizes

| Address | Convention | RET | Site |
| --- | --- | --- | --- |
| 008b8ad0 | `__fastcall(LuaState*) -> int` | `RET` | 008b8c9f |
| 00906460 | `__thiscall(ScoringManager*, int slot, byte)` | `RET 8` | 009064c0 |
| 00927f60 | `__fastcall(Entity*)` | `RET` | 0092802a |
| 004cd390 | `__thiscall(GGame*, const NativeString*, char)` | `RET 8` | 004cd43e |
| 0089a390 | `__fastcall(GGame*, ...)` | `RET` | 0089a47c |
| 0089a480 | `__fastcall(LuaState*) -> int` | `RET` | 0089a658 |
| 004d7ea0 | `__thiscall(GGame*)` | `RET` | - |
| 004d7970 | `__thiscall(GGame*, char aborted)` | `RET 4` | 004d7b03 |
| 009205e0 | `__thiscall(MissionProgress*, const NativeString*, const record*)` | `RET 8` | 00920685 |
| 008cd440 | `__fastcall(LuaState*) -> int` | `RET` | 008cdd59 |
| 008bd340 | `__fastcall(LuaState*) -> int` | `RET` | 008bd8fe |
| 008bd900 | `__fastcall(LuaState*) -> int` | `RET` | 008bdebe |
| 008e1f80 | `__thiscall(ObjectiveSet*, 4 stack args)` | `RET 10h` | 008e20c4 |
| 008dd5c0 | `__thiscall(Objective*, id, text, int, char)` | `RET 10h` | 008dd69d |
| 008e20d0 | `__thiscall(ObjectiveSet*, const NativeString*, X, Y)` | `RET 0Ch` | 008e21fa |
| 008e2200 | `__thiscall(ObjectiveSet*, const NativeString*, X, Y)` | `RET 0Ch` | 008e232a |
| 008e1d30 | `__thiscall(ObjectiveSet*, Objective*, X, int, char)` | `RET 10h` | 008e1e53 |
| 008dfe50 | `__stdcall(Objective*)` | `RET 4` | 008e005c |
| 00905350 | `__thiscall(ScoringManager*, int slot, int)` | `RET 8` | 00905362 |
| 0090f0e0 | `__thiscall(ScoringManager*, int slot, key, byte)` | `RET 0Ch` | 0090f0ff |
| 00910480 | `__thiscall(ScoringManager*, int slot, key, int)` | `RET 0Ch` | 009104ff |
| 00959450 | `__fastcall(Unit*)` | `RET` | 00959608 |
| 007f3b10 | `__fastcall(Aircraft*)` | `RET` | 007f3b98 |
| 0093bed0 | `__thiscall(this, param, float)` | `RET 8` | 0093c118 |
| 0093c300 | `__fastcall(this)` | `RET` | 0093c51e |
| 0095abe0 | `__thiscall(Unit*, Message*) -> int` | `RET 4` | 0095ae1f |

Every routine in this packet has a Ghidra function; there is no `no_ghidra_function` entry.

**Two fall-through gaps after `_free` are reported and not repaired** (Ghidra is read-only
for this packet):

- `004cd390`: `bsp.py ghidra flow 004cd390` reports one gap, `004cd3d7..004cd3e4`
  (13 bytes) after `CALL 0x00bf65ac` at 004cd3d2. `00bf65ac` is annotated no-return, so the
  decompiler ends the free branch with a `return` and shows the routine as "free the old
  result **or** create a new one". The listing has no `RET` there: the branch falls through
  into the `PUSH 0x24 / CALL operator new` at 004cd3e4. The behaviour is "free the old
  result **then** create a new one", which is what this document describes.
- `008cd440`: one 3-byte gap at 008cdcbd after `CALL 0x00bf65ac` at 008cdcb8, plus a
  3-byte non-call gap after a `JMP` at 008cd67b.

`008e1f80`, `008e20d0`, `004d7970` and `0093c300` report no gaps.

## Callers and callees that matter

- `004d7ea0` <- `004e4a40` `BSP_Game_OnMove` only.
- `004d7970` <- `004e4430` (request 0Fh), `004d87b0`, `004db190`, `006890b0`,
  `00772990`, `0088b1b0`, `0089a390`, `008b01b0`, `008b0470`.
- `004cd390` <- `0089a390` only; `0089a390` <- `0089a480` only.
- `00906460` <- `008b8ad0` only; `00906460` -> `00927f60` only.
- `009205e0` <- `004d7970` only.
- `00959450` <- `0074db70`, `00824b60`. `0095abe0` <- `00744be0`, `007ccfa0`, `00821e80`.
- `0093bed0` and `0093c300` and `007f3b10` have no Ghidra caller; they are virtual or
  table targets.

## Uncertainties

- `00927f60`'s role. It stamps the clock at `entity+1D0h`, and when `entity+9D4h` holds a
  formation it writes the same value into every member's `+1D0h` (capped at 5 members by
  `uVar5 < 5`). "AI re-target kick" is a reading of the shape, not established behaviour.
- The two trailing arguments of `008e20d0` / `008e2200` (X and Y above) pass through
  `008e1d30` and the replication message builders unread by anything decoded here.
- `008cd440`'s Lua argument order. It reads an integer, an optional integer, three
  strings and an optional boolean, and passes only two strings, an int and a byte to the
  record constructor; the third string goes to `008dbf40`. The mapping from Lua positions
  to constructor arguments is not settled, so the header models the record, not the call.
- `[game+19CCh]+4ACh`, the gate both kill reporters gate on, is the world object's byte
  that `docs/UNIT_INSTANCE_UPDATE.md` also names but whose writer is not identified.
- `004cd390`'s `+8h` float comes from `00ce77e4`; the constant's value was not read and
  `game_frame_control` already calls the field `score`, which is a guess inherited here.
- `0093bed0`'s component floats at `+28h`/`+2Ch` are called a rate and a period on the
  strength of the division; the descriptor was not decoded.

## What remains

- The mission script side. The actual win and loss rules live in the installed
  `Scripts/missions/*.lua`, which this packet does not read. Every native hook they use is
  now identified.
- `00920a20`, the debrief bring-up `004d7970` calls before the commit, and the
  `GUI_scoring` page (slot 5Eh, `0060D740`) that displays the record.
- `0091a080`, the 284h record copy, and whether it is a full `memcpy` or a member-wise
  copy that skips runtime-only fields.
- `008e1b90` and `008dd460`, the objective announcement and its sound.
- The remaining 30-odd `Scoring_*` bindings, which write the rest of the record.

## Follow-up packets

| Packet | Addresses | Files | Contract |
| --- | --- | --- | --- |
| `mission_debrief_bringup` | 00920a20, 0060d740, 0060f210, 0091a080 | docs/MISSION_DEBRIEF_BRINGUP.md, include/bsp/mission_debrief.hpp | The debrief `004d7970` raises before the commit, the `GUI_scoring` page that renders the 284h record, and the record copy |
| `scoring_binding_table` | 008b8640, 008b9190, 008b95c0, 008b9a30, 008ba0f0, 008ba7b0, 009103f0, 00910480 | docs/SCORING_BINDING_TABLE.md, include/bsp/scoring_bindings.hpp | The 30 `Scoring_*` bindings and the record offset each writes, completing `MissionScoreRecord`'s runtime side |
| `objective_unit_list` | 008cdd60, 008ce510, 008dfe50, 006de4c0, 00647c20 | docs/OBJECTIVE_UNIT_LIST.md, include/bsp/objective_units.hpp | The `objective+20h` unit list, `Objectives_AddUnit`/`RemoveUnit`, and the `SzurkeNyil` HUD marker refresh |
| `unit_failure_simulation` | 0093bed0, 0093c300, 00424c40, 008782a0, 00913d80 | docs/UNIT_FAILURE_SIMULATION.md, include/bsp/unit_failures.hpp | The 14h-stride failure descriptor table, the per-frame hazard roll and the `failure` warning it fires |

## State per address

| Address | Name recorded | State |
| --- | --- | --- |
| 008b8ad0 | `BSP_LuaScoring_SetMissionCompleted` | exported, analyzed |
| 00906460 | `BSP_MissionScoring_SetSlotCompleted` | exported, analyzed, reconstructed, build-tested |
| 00927f60 | `BSP_MissionScoring_NotifyEntity` | exported, analyzed |
| 004cd390 | `BSP_Game_SetEndOfMissionMovie` | exported, analyzed, reconstructed, build-tested |
| 0089a390 | `BSP_Game_PlayNamedMovie` | exported, analyzed |
| 0089a480 | `BSP_LuaGame_PlayBinkMovie` | exported, analyzed |
| 004d7ea0 | `BSP_Game_CheckMissionCompletion` (existing) | exported, analyzed, reconstructed, build-tested |
| 004d7970 | `BSP_Game_EndScene` | exported, analyzed, reconstructed, build-tested |
| 009205e0 | `BSP_MissionProgress_CommitMissionRecord` | exported, analyzed, reconstructed, build-tested |
| 008cd440 | `BSP_LuaObjectives_Add` | exported, analyzed |
| 008bd340 | `BSP_LuaObjectives_Completed` | exported, analyzed |
| 008bd900 | `BSP_LuaObjectives_Failed` | exported, analyzed |
| 008e1f80 | `BSP_ObjectiveSet_Add` | exported, analyzed |
| 008dd5c0 | `BSP_Objective_Construct` | exported, analyzed, reconstructed, build-tested |
| 008e20d0 | `BSP_ObjectiveSet_SetCompleted` | exported, analyzed, reconstructed, build-tested |
| 008e2200 | `BSP_ObjectiveSet_SetFailed` | exported, analyzed, reconstructed, build-tested |
| 008e1d30 | `BSP_ObjectiveSet_AnnounceStateChange` | exported, analyzed, reconstructed, build-tested |
| 008dfe50 | `BSP_Objective_RefreshUnitMarkers` | exported, analyzed |
| 00905350 | `BSP_MissionScoring_SetSlotRanking` | exported, analyzed |
| 0090f0e0 | `BSP_MissionScoring_SetSlotMissionMedal` | exported, analyzed |
| 00910480 | `BSP_MissionScoring_AddSlotMissionScore` | exported, analyzed |
| 00959450 | `BSP_Unit_OnDestroyed` | exported, analyzed, reconstructed, build-tested |
| 007f3b10 | `BSP_Aircraft_OnDestroyed` | exported, analyzed |
| 0093bed0 | `BSP_ShipSystems_RollComponentFailure` | exported, analyzed |
| 0093c300 | `BSP_ShipSystems_PickRandomFailure` | exported, analyzed |
| 0095abe0 | `BSP_Unit_HandleMessage` | exported, analyzed |
