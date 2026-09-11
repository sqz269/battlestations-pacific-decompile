# In-mission subsystem tick (the fixed four-call sequence 004C40A0)

Addresses: 004c40a0 00875bb0 004c3cb0 00447b80 00904bf0 004462d0 008079b0 004c1130 004bfdf0 004bf8e0

Packet `cc_mission_tick`, worktree `agent/cc-mission-tick`. Ghidra was read-only for this packet;
every name below is a hypothesis, not a recovered symbol.

`docs/MISSION_STATE_FRAME.md` step 9 is one call, `004E5133 -> 004C40A0`, and it had no
reconstruction. This document opens it. `004C40A0` is the whole simulated part of an in-mission
frame: four calls, no test, no local state. Everything the mission simulates per frame is behind
one of those four.

## `004C40A0` itself

`__thiscall void(GGame*)`, body `004C40A0..004C40E3`, `RET` (no stack argument). Sole caller
`BSP_Game_OnMove 004E4A40` at `004E5133`, the first call inside the state-0Dh simulation block
(`docs/GAME_SIMULATION_GATE.md`). It reads exactly one field, `game+21F0h` (the scaled delta the
spine wrote), reloads it before each of the three calls that take it, and tests nothing. Coverage:
complete.

| # | call site | callee | `this` (ECX) | stack argument |
| --- | --- | --- | --- | --- |
| 1 | `004C40AD` | `00875BB0` | `game` at the site, **ignored by the callee** | `game+21F0h` |
| 2 | `004C40B4` | `004C3CB0` | `game` | none |
| 3 | `004C40CE` | `[[game+19CCh]]+0Ch` = `00904BF0` | `[game+19CCh]`, the world object | `game+21F0h` |
| 4 | `004C40DD` | `00447B80` | `[game+30h]`, the dynamics list | `game+21F0h` |

Every float argument uses the same `PUSH ECX / FSTP [ESP]` idiom (`004C40A9`, `004C40CA`,
`004C40D6`): the push only reserves the slot, the pushed register value is overwritten. All four
callees clean their own argument (`RET 4` on the three that take one), so the sequence needs no
`ADD ESP` and there is none in the listing.

Ordering matters and is load-bearing: call 1 advances the fixed-step physics clock and leaves the
leftover accumulator in `00F876AC`; call 4 divides that leftover by the fixed step to interpolate
every debris body's render transform. Call 1 also runs the dynamics list's *fixed-step* pass
(`004462D0`, below), so the list is integrated before call 4 reads it.

## Call 1, `00875BB0`: the fixed-step simulation driver

`__thiscall void(ignored, float rawStepDelta)`, body `00875BB0..0087601E`, `RET 4` at `0087601C`
and `00875FFD`. The `this` register is dead: `00875BB1 MOV ECX,[00E188A8]` overwrites it before any
use. Treat the routine as `void(float)` with a vestigial `this`; see Corrections.

**Gate** (`00875BB1..00875C02`). It zeroes the three clocks and returns when any of these holds:
the game singleton `[00E188A8]` is null; the local-player slot
`[game+18CCh + game+18ECh*4]` is null; that slot's `+10h` `short` is `< 1`; or
`game+1FE4h == 2` and `[[game+207Ch]+9Ch] < 8`. The zeroing at `00876003..0087601B` clears
`00F876AC`, `00F876A4` and `00F876A8`.

**The clocks.** Six globals, all floats except the buffer index:

| global | role | evidence |
| --- | --- | --- |
| `00F876AC` | the step accumulator; the leftover after the loop is the interpolation numerator | `00875C49`, decremented at `00875EFA`, consumed at `00447D8F` |
| `00F876A4` | the simulation clock; `-= accumulator` before, `+= accumulator` after | `00875C08`, `00875C80`, `00875F26`; the deadline base in `004D87B0` |
| `00F876A8` | the raw accumulated delta, never decremented | `00875C3D` |
| `00F876B0` | the step counter, `+1` per step | `00875C6E`; read by the spine as `max_step_clamp_disabled` |
| `00F876A1` | set while the step loop runs, cleared after | `00875C86`, cleared at `00875F0E` and only when the loop ran |
| `00F876B4` | zeroed inside every step, set to the leftover after the interpolation wave | `00875EF0`, `00875FC1` |
| `00E0B6CC` | the double-buffer index, `1 - index` per step; the previous value is saved to `00F876B8` | `00875C5F`, `00875C7A` |

The fixed step is the double at `00D7A270`, `0x3FA999999A000000` = **0.05 s (20 Hz)**; the per-step
callees receive the float at `00D0DE84`, `0x3D4CCCCD` = `0.05f`, the same constant `00447B80`
divides the leftover accumulator by.

**The step loop** (`00875C5F..00875F08`, `do { } while (step <= accumulator)`), per step:

1. `00875CA5` `008079B0(0.05f)` - a separate countdown global at `00F874B8`.
2. Three job waves, each five groups. The group heads are the intrusive circular lists at
   `00F876C8` with stride `68h` (`&DAT_00F876F4 - 2Ch`, five iterations); an element's payload is
   `[elem+28h]` and the walk follows `elem+8h`. Wave 1 (`00875CDD`, factory `008755A0`) and wave 3
   (`00875DBD`, factory `008754D0`) admit a payload whose `+5Eh` is clear **and** `+BDh` is set;
   wave 2 (`00875D4D`, factory `00875750`) tests the same two bytes in the opposite order. Each
   wave dispatches through `BSP_FrameJobPool_GetSingleton 004C1130`, `[[pool+4]+4](job, elem)` to
   queue and `[[pool+4]+8](byte at 00E0B6CE)` to run, and only when at least one element was
   queued. That byte is a different field from the flipped index at `00E0B6CC`; the flip is a word
   store (`00875C9A`) and never touches it.
3. The per-step fan-out, `00875E0C..00875EDF`:

   | site | callee | `this` | argument | contract |
   | --- | --- | --- | --- | --- |
   | `00875E0C` | `00C5C540` | none | `0.05f` | unread (physics block) |
   | `00875E24` | `004462D0` | `[00E188A8]+30h` | `0.05f` | the dynamics list buoyancy step, `docs/GAME_DYNAMICS_LIST.md` |
   | `00875E33` | `0042E630` | none | `0.05f` | unread; returns an object used as the next `this` |
   | `00875E3A` | `0098BDB0` | `0042E630`'s result | `0.05f` | unread |
   | `00875E3F` | `00874DE0` | none | none | unread |
   | `00875E44` | `00926700` | none | none | unread |
   | `00875E55` | `00888230` | `[00E188A8]+1A08h` | none | unread |
   | `00875E64` | `00929460` | none | `0.05f` | unread |
   | `00875E91` | `00778450` | none | `0.05f` | unread, gated |
   | `00875E96` | `0077EC20` | none | none | unread, gated |
   | `00875E9B` | `00874C90` | none | none | unread, gated |
   | `00875EA2` | `00925F20` | none | none | unread, gated |
   | `00875EBF` | `0076FFC0` | none | `0.05f, 1` | unread, gated |
   | `00875EC4` | `00926700` | none | none | unread (second call) |
   | `00875EC9` | `009273A0` | none | none | unread |
   | `00875EDA` | `00903610` | none | none | unread |

   The five gated rows run only when `[00E188A8]` is non-null and `[[game+19CCh]+4ACh]` is set,
   the world object's active byte (`docs/MISSION_RESULT_DECISION.md` calls the same byte the gate
   both kill reporters use).
4. `accumulator -= 0.05f`, `00F876B4 = 0.0f`.

**The interpolation wave** (`00875F32..00875FCC`). After the loop, `00F876A4 += accumulator`; then,
only when the frame delta and the accumulator are both `> 0`, the same five groups are walked once
more with the factory `00875670`, whose descriptor takes the leftover accumulator in `[job+8h]`
(`00875F50`). This wave admits any payload with `+5Eh` clear, without the `+BDh` test. Afterwards
`00F876B4 = accumulator`.

**Tail.** `00875FF7` calls `00A317F0` when `00F8AB04`, `00E198C4`, `[00E198C4+54h]` and
`[[00E198C4+54h]+5h]` are all set.

Coverage: **partial**. The gate, the clock arithmetic, the step count and the double-buffer flip
are reconstructed (`advance_fixed_step_clock_00875bb0`). The three job waves
(`00875CC0..00875DFC`), the sixteen-call per-step fan-out (`00875E0C..00875EDF`) and the
interpolation wave (`00875F32..00875FCC`) are documented as one host method each and are **not**
reconstructed; none of their callees except `004462D0` was read.

## Call 2, `004C3CB0`: the local-player unit lists, built once per scene load

`__thiscall void(GGame*)`, body `004C3CB0..004C409D`. Guard at `004C3CB7..004C3CDC`: it runs only
when `game+193Ch` is clear and `0 <= game+18ECh < 8`, and the first thing it does is set
`game+193Ch = 1`. **`game+193Ch` is cleared in exactly one place**: `BSP_Game_LoadMissionScene
004DFB70`, at `004E0360` and at `004E05A2` (the second immediately before its own `004C3CB0` call
at `004E05A9`). A byte-pattern search of the image for `C6 /r 3C 19 00 00` over every base register
finds no other writer inside the game class. So step 2 of the tick does its work on the first
simulated frame after a scene load and returns at `004C3CC5` on every frame after that, and the
same is true of the other four callers (`0045F600`, `004C9CA0`, `004D7EA0`, `004DFB70`): whichever
runs first after the load pays for it.

What it builds: `004BFDF0` (`004C3CE3`) pops the head off each of eight `{count, head, tail}` lists
at `game+1964h, +1970h, +197Ch, +1988h, +1994h, +19A0h, +19ACh, +19B8h` through `004BF8E0`, then
three linked-list walks over the local player's world lists at
`[[game+18CCh + game+18ECh*4]+30h] + DDCh / +DE8h / +E00h` (next at `node+4h`, value at
`node+8h`, unit at `value+4h`) classify each unit by four flag bytes (`unit+5Ch`, `+5Dh`, `+60h`,
`+5Eh`) and the virtual predicate `unit->vtable[5Ch](kind)` with kinds `2Ah, 6, 18h, 0Fh, 45h,
46h, 1Bh, 35h`, appending through `00484540` (doubly linked push_back onto a `{count, head, tail}`
head, `0Ch`-byte nodes, `prev/next/value`). The tail at `004C404C..004C407D` merges lists into
`game+19B8h` with `004C2BE0` (append every value of one list to another).

Coverage: **partial**. The guard, the eight-list identity, the list ABI and the merge tail are
established. The per-kind classification (`004C3D5B..004C404B`) is described but not reconstructed,
and `008DDF90` (one of the predicates) was not read.

## Call 3, `00904BF0`: the world entity tick

`[game+19CCh]` is the world object of `docs/GAME_WORLD_CONSTRUCT.md`, vtable `00CE7784`; slot
`+0Ch` of that vtable holds `00904BF0`, read from the image at `00CE7790`. It is already named and
described in `docs/GAME_WORLD_ENTITIES.md` (`BSP_World_UpdateEntities`: walk the world node's child
chain from `[world+4]` through `entity+38h`, gate on `entity+5Ch`, call `entity->vtable[0DCh]`
with the scaled delta; for `MDestroyer` that slot is the vehicle base update `008255B0`). This
packet does not redo it; the host method forwards to it. Coverage: referenced, not re-read.

## Call 4, `00447B80`: the dynamics list frame pass

`docs/GAME_DYNAMICS_LIST.md`.

## The host

`include/bsp/in_mission_subsystem_tick.hpp` / `src/in_mission_subsystem_tick.cpp`.
`run_in_mission_subsystem_tick_004c40a0(scaled_delta, host)` is the four-call sequence;
`in_mission_tick_step(i)` is the same table as above with the owner area per row.
`advance_fixed_step_clock_00875bb0(clock, gate, delta)` is the pure part of call 1: it returns the
number of fixed steps to run and leaves the clock globals in the state the listing leaves them,
including the buffer flip and the `00F876B4` reset. `FixedStepHost` carries the three job waves and
the per-step fan-out as one method each, so a caller that has those subsystems can run them without
re-deriving the loop.

| routine | reconstruction | coverage |
| --- | --- | --- |
| `004C40A0` | `run_in_mission_subsystem_tick_004c40a0` | complete |
| `00875BB0` | `advance_fixed_step_clock_00875bb0`, `run_fixed_step_driver_00875bb0` | partial: `00875CC0..00875DFC`, `00875E0C..00875EDF`, `00875F1D..00875FB9` are host methods, not rules |
| `004C3CB0` | `local_player_unit_lists_run_004c3cb0` (the guard only) | partial: `004C3D5B..004C404B` documented, not reconstructed |
| `00904BF0` | referenced (`docs/GAME_WORLD_ENTITIES.md`) | not re-read |
| `00447B80` | `docs/GAME_DYNAMICS_LIST.md` | complete for the two loops |

## Corrections

- **`00C34F70` is a function, not a vtable.** `docs/MISSION_STATE_FRAME.md`'s follow-up row lists
  "the vtable `00C34F70`". Body `00C34F70..00C35001`, `__thiscall void(owner, dword)`, `RET 4`: a
  growable dword array `push_back` on `owner+438h` (data), `+43Ch` (size), `+440h` (capacity), with
  `capacity = 2*capacity + 2` and `operator new` / `_free` around the copy. Was: "the vtable
  `00c34f70`". Is: the deferred-release queue push. Evidence: the listing at `00C34F81..00C34FFF`.
- **`00875BB0` ignores its `this`.** `004C40A0` passes `game` in ECX at `004C40AD`; `00875BB1`
  overwrites ECX with `[00E188A8]` before reading it. The site's `__thiscall` shape is real, the
  parameter is not. Was: "`00875bb0` with ECX = game" (the `004C40A0` ledger evidence, which
  describes the site correctly). Is: the site passes it, the callee discards it.
- **`004C3CB0` is not per-frame work.** Its latch `game+193Ch` is cleared only by
  `004DFB70`. Was: implied per-frame by its position in the tick. Is: once per mission scene load.
- **The per-frame integration of the dynamics list is `00447B80`, and its fixed-step integration is
  `004462D0`; `00447060` is the release.** `docs/MISSION_STATE_FRAME.md`'s follow-up row for
  `game_dynamics_list` names "the per-frame integration 00447060". Evidence: `00447060`'s two
  callers are `004DA6C0` and `004DA780` (entry and teardown), never the tick; `004462D0` is reached
  from `00875E24` with `ECX = [00E188A8]+30h`.

## no_ghidra_function

none. Every address in this document is the start of, or inside, a Ghidra function with a stored
body: `004C40A0..004C40E3`, `00875BB0..0087601E`, `004C3CB0..004C409D`, `00447B80..0044831C`,
`004462D0..00446525`, `00904BF0..00904C34`, `008079B0..00807A43`, `004BFDF0..004BFE4B`,
`004BF8E0..004BF925`, `004C1130..004C11EF`.

## Follow-up packets

| packet | addresses | what is open |
| --- | --- | --- |
| `fixed_step_job_waves` | 004c1130 008754d0 008755a0 00875670 00875750 00f876c8 | The five `68h`-stride group heads at `00F876C8`, the four job factories and the `[[pool+4]+4]` / `+8` job-pool ABI. Four waves per frame go through them and none of the four factories was read. |
| `fixed_step_subsystem_fanout` | 00c5c540 0042e630 0098bdb0 00874de0 00926700 00888230 00929460 00778450 0077ec20 00874c90 00925f20 0076ffc0 009273a0 00903610 00a317f0 | The sixteen per-step calls of `00875E0C..00875EDF` plus the tail. Each is `contract: unread`; five of them are gated on the world's `+4ACh` byte. |
| `local_player_unit_lists` | 004c3cb0 00484540 004c2be0 004bf8e0 008ddf90 | Which of the eight lists at `game+1964h..+19B8h` each unit kind lands in, and the meaning of the eight `vtable[5Ch]` kind codes. `004BFDF0` pops one element per list rather than clearing it, which looks like a defect worth confirming against a run. |
| `fixed_step_countdown_008079b0` | 008079b0 00f874b8 00d7a2b0 | The countdown global the first per-step call maintains. |
