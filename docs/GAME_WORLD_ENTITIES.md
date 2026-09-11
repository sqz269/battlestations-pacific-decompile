# World tick of the in-mission frame

Addresses: 00481640, 00914ef0, 006dc1a0, 00740e10, 00903670, 00987590, 008eb110, 004d8cd0,
004e4a40, 004a43c0, 0049d690, 004cb030, 006deca0, 006d6200, 0091d640, 0091b2e0, 00922fd0,
004d6900, 004de610, 004dc6a0, 004e3aa0

Packet `game_world_entities`. The entity, marker, decal, bot, power-up and mission-event
updates `BSP_Game_OnMove` (004e4a40) drives from the scaled delta at `game+0x21F0`. Every
call in this packet takes its ECX from a game field or a plain global rather than from a
singleton getter, which is what separates it from the surrounding phases.

Reconstruction lives in `include/bsp/world_entities.hpp` and `src/world_entities.cpp`.
The simulation gate it sits behind is `game_simulation_gate`; the delta that feeds it is
`game_frame_control`; the ocean and effect calls interleaved with it are
`game_world_ocean_effects`. None of those are duplicated here.

## Call sites in 004e4a40

Phase 20, 004e52ba-004e5389, read from `python tools/bsp.py disasm-raw 004e4a40 --length 2808`
because the stored Ghidra body of 004e4a40 is eight bytes.

| Address | Call | ECX | Argument |
| --- | --- | --- | --- |
| 004e52ca | 00914ef0 | `game+0x21A0` | scaled delta |
| 004e52df | 006dc1a0 | `game+0x21D4` | scaled delta |
| 004e52f4 | 00481640 | `game+0x21D0` | scaled delta |
| 004e533a | 00740e10 | `DAT_00e1aea0` | scaled delta |
| 004e534f | 0094c8f0 | `DAT_00f89b3c` | scaled delta, trivial body, other packet |
| 004e535a | 008eb110 | `DAT_00f88c30` | none |
| 004e5382 | 00903670 | `game+0x19CC` | none |

The map in `docs/GAME_ON_MOVE_MAP.md` lists 00740e10, 008eb110 and 00903670 without an ECX;
the three globals and `game+0x19CC` above are new and come from the raw listing.

Two more calls in the packet sit outside the simulation gate:

- 004e4e67 `00987590(scaledDelta)` with ECX `game+0x21E0`, inside the state-0x0D block,
  behind `game+0x634 == 0`, a non-null `game+0x21E0` and `scaledDelta > 0.0f` (00d7a218).
- 004e4feb `004d8cd0()` with ECX `game`, on the join point every path reaches, immediately
  before `game+0x648 += 1`.

The gate the seven phase-20 calls sit behind is 004e50b0: `game+0x634 == 0 || game+0x635 != 0`,
then `game+0x5D4 == 0x0D && game+0x7184 == 0`. The `+0x635` escape is absent from the
effect-list gate at 004e4e31, so the mission-event update is strictly the narrower condition.

## Subsystem objects

Found by scanning for the field stores (`bsp.py scan-bytes "89 ?? A0 21 00 00"` and friends)
and reading the allocation site around each.

| Field | Size | Constructor | Follow-up | Allocation site |
| --- | --- | --- | --- | --- |
| `game+0x21A0` | 0x14B8 | 0091d640 | 0091b2e0 | 004e3e74, in `BSP_Game_OnInit` |
| `game+0x21D0` | 0x58 | 004a43c0 | 0049d690 | 004dc80f, in 004dc6a0 |
| `game+0x21D4` | 0x50 | 006deca0 | 006d6200 | 004df9de, in 004de610 |
| `game+0x19CC` | not recovered | 004cb030 | 0x9037f0(1,1) | 004de686, in 004de610 |

None of the four vtables (00d18acc, 00ce68d0, 00cf8ff8, 00ce7784) has an RTTI locator at
`vtable-4`, so the image ships without RTTI and no class name is recoverable. The names below
are hypotheses drawn from segment keywords and behaviour, not recovered symbols.

`game+0x21A0`, its constructor and all four of 00914ef0's callees live in segment 59, whose
keywords are `pilotbot`, `tailgunnerbot`, `torpedobot`, `thinktimeleft`, `bulletthrowmul`,
`vertangleerror`, `horzangleerror` and `depthchargebot`. 006dc1a0 is in segment 32
(`entitymarkers`, `positionmarkers`, `guihighlights`, `recursiveguihighlights`,
`markerclasses`). 00481640 is in segment 7 (`entity`, `soldiertypes`, `landvehicleclasses`,
`maxentity`). 008eb110 is in segment 58 (`pup_gain`, `pum1stget`, `uspumicon`) and builds the
literal `pup_ready`. 00987590 is in segment 65 (`entitykilled`, `musicover`, `repair`,
`surrender`).

Every list in this packet is an MSVC `std::list` compiled with `_SECURE_SCL` on: the base is
`{_Myfirstiter, _Myhead, _Mysize}` and a node is `{_Next, _Prev, value}`. The many
`LIBCRT_unmatched_00bf6713` calls in the pseudocode are that build's iterator validations, not
program logic.

## 00481640 - entity manager forwarder

`__thiscall(this, float)`, RET 4, ECX `game+0x21D0`. State: analyzed and reconstructed.

```
EAX = [00e188a8]              ; the game singleton
EDX = [EAX + 0x19CC]          ; the world object 00903670 also runs on
if ([EDX + 0x4AC] == 0) return
ECX = [this + 0x8]
[ECX]->vtable[1](ECX, delta)  ; slot at vtable+4
```

The gate byte belongs to `game+0x19CC`, not to the entity manager, and it is read through the
game singleton rather than through ECX. Constructor 004a43c0 fills `this+0x10`, `+0x1C`,
`+0x28`, `+0x34`, `+0x40` and one more list with heads from 00486070, 00486100, 00486180,
00486210 and 00485f50 (all in the entity segment), leaving `+0x4` and `+0x8` to 0049d690;
`+0x8` is the only field the forwarder touches. The dispatched slot, vtable+4, is the same
slot 006dc1a0 uses on each marker, so "update with the frame delta" is slot 1 across these
object families.

## 00914ef0 - bot scheduler

`__thiscall(this, float)`, RET 4, ECX `game+0x21A0`. State: analyzed and reconstructed.

Two countdowns and one accumulator, all gated on `game+0x1FE4` (the local player mode that
`bsp/game_frame_control.hpp` already carries):

- Outer gate 00914f0d: mode is 0 or 1, and `this+0x1498` is set.
- Retarget countdown `this+0x149C -= delta`. At or below zero: `0075b430(0x15)`, then eight
  passes over the 0x284-stride records at `this+0x1E4` (00914f88, `ADD EDI,0x284`, `CMP ESI,8`).
  Each pass calls 00914390 when `this+0x14A4` is set, and when the mode is exactly 1 fills a
  stack object whose vtable is `PTR_LAB_00d035b8` with the record's first dword, the code 0x12
  and the slot index, then calls `CG_adjustor_thunk_0076a9f0(&obj, 0)`. Reload from `this+0x14A8`.
- Think countdown `this+0x1494 -= delta`. At or below zero: eight passes of
  `00911e80(); 00912a60();` over the same stride. Reload from `this+0x14AC`.
- Independently, when the mode is not 2 and `this+0x14A4` is set, `this+0x14A0 += delta`.

Uncertainty: eight records of 0x284 bytes starting at `+0x1E4` would end at `+0x1604`, past
the 0x14B8 allocation. Only the first dword of each record is read (max touched byte `+0x141B`),
so the stride and the count are solid but "array of eight 0x284-byte records" is not; the
records may be shorter than their stride, or the tail may overlap other fields.

## 006dc1a0 - marker and GUI-highlight manager

`__thiscall(this, float)`, RET 4 (006dc597), ECX `game+0x21D4`. State: analyzed and reconstructed.

Five list walks, in order. The `_Myfirstiter` offsets are given; `_Myhead` is four bytes later.

1. `this+0x14`. Each element's value carries a nested list at value+0x8; each inner element's
   value has an object pointer at +0xC. Non-null objects get `vtable[1](obj, delta)` (006dc270).
2. `this+0x20`, the same shape with the inner list at value+0x10 (006dc2ff, 006dc35f).
3. `this+0x2C`. For each node, `target = node+0xC` (value+0x4) and
   `target->float_0x94 = |cos(2 * (game+0x64C - 00e19970))| * 0.28` (006dc423, `FMUL qword [00cf8fc8]`,
   whose cell holds 3FD1EB8520000000h, the double promotion of 0.28f).
4. `this+0x38`. For each node, `target->006dbac0(1, 0, 1.0f)` (006dc496).
5. `this+0x44`. For each node, `target->vtable[0x54](&buf)` fills four floats, the fourth is
   overwritten with `|cos(...)| * 0.75 + 0.25` (00cec9d8 and 00d7a348, both qword), then
   `target->vtable[0x50](&buf)` writes it back. The alpha slot is `buf+0xC` (006dc56a stores at
   `ESP+0x48` after a push that moved ESP down four from the `LEA ECX,[ESP+0x38]` buffer).

The pulse epoch 00e19970 ships as 0.0f and nothing in this packet writes it. The cosine is
computed at float precision with `FCOS` and the sign is stripped with `AND EDX,0x7FFFFFFF`.

## 00740e10 - decal manager

`__thiscall(this, float)`, RET 4 (00740ea7), ECX `DAT_00e1aea0`. Already named
`BSP_DecalManager_Update`. State: analyzed and reconstructed.

The delta argument is never read. The body is a one-shot profiler-label registration behind
bit 0 of `DAT_00e1aeb0` (`BSP_NativeString_Assign("cDecalManager::Update")`, `_DAT_00e1aeac = 0`,
then the storage-pool release of the temporary), followed by a walk over a plain array: begin
at `this+0x4`, count at `this+0x8`, stride 4. **The loop body is empty in the shipped image**
(00740e91-00740e96 is `ADD EAX,4` / `CMP` / `JNZ` and nothing else), so the per-decal work was
compiled out and the only observable effect left is the label registration. A reconstruction
that invents per-decal work here would not match the binary.

## 008eb110 - power-up manager

`__thiscall(this)`, RET (008eb341), no argument, ECX `DAT_00f88c30`. State: analyzed and
reconstructed (partially: the string construction around the notification is summarised).

- `008eac80()` first.
- Sixteen channel lists at `this+0x84`, stride 0xC (`piVar8 += 3` dwords, `local_30` counts down
  from 16). For each node whose `value+0x18` float is below the clock `DAT_00f876a4`, call
  `008e8c30(value)`. Nothing is erased.
- Then the per-player list at `this+0x14C + slot*0xC`, where `slot` is `game+0x18EC`, the active
  local-player slot. For each element the deadline is `value+0x18` when `value+0x1C` is clear,
  and `*(value+0x10) + 0x80` **plus the clock** when it is set, which re-derives the deadline
  every frame and therefore fires only once that source float goes negative. When the deadline
  is below the clock and `value+0x8` is clear, `value+0x8` is latched to 1 and, if the ready
  list's `_Mysize` at `this+0x28 + slot*0xC` is non-zero, the routine builds the literal
  `pup_ready` and calls `009789a0(slot, readyHead->_Prev + 8, &string)` before releasing the
  temporary. `00613760()` runs at the end of every iteration.

## 00903670 - entity activation flush

`__thiscall(this)`, RET (009036a8), no argument, ECX `game+0x19CC`. Three callers: 004e4a40,
`BSP_Game_CheckMissionCompletion` (004d7ea0) and 0046b730, so it is a flush that several sites
can force, not a per-frame-only pass. State: analyzed and reconstructed.

The chain starts at `[this+0x4]` and links through `node+0x38` (an intrusive chain, not a
`std::list`). A node is processed when `node+0x5E != 0`, `node+0x6C == 0`, and either
`node+0x3C == 0` or `node+0x3C->+0x5E == 0`; then `00922fd0(node)` runs.

00922fd0 sets `node+0x5E = 1`, `node+0x5D = 1`, `node+0x5C = 0` and `node+0x6C = 1`, recurses
over children from `node+0x48` linked through `+0x44` for children whose `+0x6C` is clear, and
tail-calls the node's virtual at `+0x84`. Because it latches `+0x6C` on the whole subtree, the
outer walk skips any node it already reached as a child.

## 00987590 - mission event director

`__thiscall(this, float)`, RET 4 (00987748), ECX `game+0x21E0`. State: analyzed and reconstructed.

1. `00982540()`.
2. `this+0x198 += delta`; when the result exceeds 4.0f (00ce3d34) it is **reset to zero**, not
   decremented, and `00977990()` runs. The period is a floor, not an exact interval.
3. `0096d540()`, `00968550()`.
4. Return immediately when the queue size `this+0xE8` is zero. The queue is a `std::list` at
   `this+0xE0` whose value is a pointer to an event object.
5. Walk. For each event: `duration = event+0x10`, `start = event->vtable[7]()`.
   - `now - start >= duration`: call `event->vtable[0](1)` (the scalar deleting destructor),
     null the node's value, unlink the node, `free` it and **return**. At most one event is
     retired per frame, and any selection made earlier in the walk is discarded unapplied.
   - Otherwise, if `005b71d0(event+0x14)` reports ready, keep the event when no candidate is
     held yet or when the held candidate's `vtable[6]()` priority is strictly lower.
6. On reaching the end of the list, `00974070(best)` when a candidate was held.

## 004d8cd0 - input action deadline table

`__thiscall(this)`, RET (004d92aa), no argument, ECX `game`. State: analyzed and reconstructed.

Sixteen identical blocks. Each reads one input action record through the input singleton
`004bec00`, and when the action passes the full 004c43c0 edge test writes
`game+0x64C + 0.4` into a `std::map<int,float>` at `DAT_00e18a7c` through `operator[]` at
004d6900, whose ECX is the literal `0xE18A7C`.

The edge test is exactly the one `bsp/game_frame_control.hpp` documents for 004c43c0:
`record+0x28` set, `record+0x24 > 0`, and either `record+0x20` clear or `record+0x1C <= 0`.
Records are 0x30 bytes from `inputSingleton+0x4`, so the inline offsets 0xE08, 0xE38, 0xEC8 …
0x10A8 divide out to the action ids

`0x4A 0x4B 0x4E 0x4F 0x50 0x51 0x52 0x53 0x54 0x55 0x46 0x47 0x4C 0x4D 0x57 0x58`

in that order. The last block confirms the divisor: 004d9251 computes `EAX = input+4 + 0x1080`
and then tests `[EAX+0x28]`, and `0x58 * 0x30 == 0x1080`. The added constant 00ce65d0 holds
3FD99999A0000000h, the double promotion of 0.4f, so the map stores an absolute deadline on the
`game+0x64C` clock rather than a timestamp.

## Reconstruction

`bsp::run_world_tick` in `src/world_entities.cpp` sequences the mission-event update, the
deadline table, the frame counter and then the seven gated calls in native order, over a
`bsp::WorldTickHost` with one virtual per native call site. Container layouts are structs with
the native offsets in comments. `bsp::update_mission_events_00987590` returns which event was
retired and which was selected so the "retire wins over apply" rule is observable.

Nothing here is a drop-in binary replacement: the native routines are `__thiscall` on objects
whose full layouts are not recovered, and the reconstruction takes projections of the fields
this packet establishes.

## State per address

| Address | Name recorded | State |
| --- | --- | --- |
| 00481640 | BSP_EntityManager_Update | exported, analyzed, reconstructed, build-tested |
| 00914ef0 | BSP_BotScheduler_Update | exported, analyzed, reconstructed, build-tested |
| 006dc1a0 | BSP_MarkerManager_Update | exported, analyzed, reconstructed, build-tested |
| 00740e10 | BSP_DecalManager_Update (already named) | exported, analyzed, reconstructed, build-tested |
| 00903670 | BSP_EntityWorld_FlushActivations | exported, analyzed, reconstructed, build-tested |
| 00987590 | BSP_MissionEvents_Update | exported, analyzed, reconstructed, build-tested |
| 008eb110 | BSP_PowerUpManager_Update | exported, analyzed, reconstructed, build-tested |
| 004d8cd0 | BSP_Game_RecordActionDeadlines | exported, analyzed, reconstructed, build-tested |

Nothing in this packet is fixture-tested, ABI-compatible or game-validated. Every routine
above has a real Ghidra function, so the orchestrator can apply the names without defining
anything first.

## Uncertainties and what remains

- No RTTI in the image, so all eight names are behavioural hypotheses.
- The 8 x 0x284 record array at `game+0x21A0 + 0x1E4` overflows the object's 0x14B8 allocation;
  see the 00914ef0 section.
- `game+0x19CC`'s allocation size and constructor argument were not recovered; only its
  constructor 004cb030 and its `0x9037f0(1, 1)` initialiser are established.
- `game+0x21D0 + 0x8`, the object the entity forwarder dispatches on, is written by 0049d690,
  which was not read. Its class is unknown.
- The stack object 00914ef0 hands to `CG_adjustor_thunk_0076a9f0` (vtable `PTR_LAB_00d035b8`,
  fields at +0x0, +0x184 = 0x12, +0x180 = slot) is a dispatch payload whose consumer was not
  traced. The meanings of `0075b430(0x15)` and the code 0x12 are open.
- 00987590's `event->vtable[6]` and `vtable[7]` are read as priority and start time from how
  they are combined; their bodies were not read. `005b71d0` is read as a readiness predicate.
- The marker vtable slots 0x50 and 0x54 are read as a colour setter and getter from the
  four-float buffer they share; neither body was read.
- `DAT_00e1aea0`, `DAT_00f88c30`, `DAT_00f89b3c` and `DAT_00e18a7c` were not traced to their
  initialisers, and 00e19970's writer is unknown.
- 008eb110's `009789a0(slot, node, &"pup_ready")` and 008e8c30 were not read; the ready list at
  `this+0x20` is only observed through its `_Mysize` gate and its `_Prev` element.

## Correction from docs/UNIT_INSTANCE_UPDATE.md

Unit instances do not go through `00481640`: it dispatches `00487270`, a `std::list` walk calling vtable slot 4, and in a unit vtable that slot is `0042b970`, a this-returning accessor with `RET 0`, so no unit is on that list. Units are updated by `BSP_Game_UpdateInMissionSubsystems` (`004c40a0`) through `world->vtable[0Ch]` = `00904bf0` `BSP_World_UpdateEntities`, which walks the world node's child chain from `[world+4]` through `entity+38h`, gates on the byte at `entity+5Ch`, and calls `vtable[0DCh](scaledDelta)`; for `MDestroyer` (vtable `00cfc3d0`) that slot is the vehicle base update `008255b0`.

## Correction from docs/MISSION_EVENTS_UPDATE.md

The object the world tick runs at `game+21E0h` through `00987590` is the WarningManager, the in-mission warning and radio-chatter director (class name from its own literals), not an objective or win/lose director; mission completion and failure are decided elsewhere (segments 61 and 62, proposed as a follow-up). In the event record, `event+10h` receives the clock (a start time) and the virtual at `+1Ch` supplies the duration, the reverse of the reading above; the inequality is numerically identical, so `src/world_entities.cpp` still computes the right result.
