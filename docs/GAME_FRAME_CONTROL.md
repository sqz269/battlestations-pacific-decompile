# Game frame control

Addresses: 004e2200, 004c6e30, 004c6b20, 004e4430, 004c0170, 0053c510, 004d7ea0, 004db920

The control spine of the game update `BSP_Game_OnMove` (004e4a40): the console command pre-tick,
the state-request queue and its drain, the per-frame delta scaling and the two clocks it feeds,
and the call sites the frame reaches unconditionally. The surrounding phases are mapped in
`docs/GAME_ON_MOVE_MAP.md`; the phases between the ones below belong to the other per-frame
packets and are not reconstructed here. Reconstruction lives in
`include/bsp/game_frame_control.hpp` and `src/game_frame_control.cpp`.

All addresses were read from the disk `.text` bytes through `bsp.py disasm-raw`. The stored
Ghidra body for 004e4a40 is eight bytes, so the OnMove listing quoted here comes from the same
raw decode.

## Delta scaling, 004c6e30

`__thiscall`, ECX = game, one stack argument holding a `float*`, `RET 4`. The argument points at
the caller's own delta slot (`lea eax,[esp+48h]` at 004e4d38), and the routine writes back
through it, so every later phase that reads the OnMove argument sees the clamped value, not the
one the application passed. OnMove keeps an unclamped copy at 004e4d3f but never reads it again.

Fields written: game+21ECh (the undilated step), game+64Ch (an accumulator, `+=` the step) and
game+21F0h (the scaled delta the simulation reads). The x87 sequence, not the pseudocode, is the
evidence: the multiplier constants are 64-bit and the products are formed in double.

Path selection reads game+1FE4h, the local view mode:

| Mode | Path |
| --- | --- |
| 0 | Jumps straight to 004c6f3d. No mission filter, no 0.25 clamp. |
| 2 | Split path unconditionally (`mov bl,1` at 004c6e4c). |
| other | Counts active local players and takes the split path when the count exceeds one. |

The count at 004c6e50 walks eight pointer slots starting at game+18CCh in two passes of four
(`ecx-4`, `ecx`, `ecx+4`, `ecx+8`, then `ecx += 10h`). A slot counts when the pointer is
non-null, slot+8h is nonzero and slot+9h is zero. 007713a0 indexes the same array with the
selector at game+18ECh, which corroborates the base.

Non-zero modes then run, in order:

1. In state Dh only, `*delta = 007713a0(*delta)` with ECX = game+1EF0h (004c6ecf). That routine
   returns its argument unchanged unless the mission object's mode word at +F4h is set; the
   replay and time-control modes behind it are not reconstructed.
2. A coarse step clamp: if the delta exceeds 0.25f (00ce3868) and 00f876b0 is zero, it becomes
   0.25f. A nonzero 00f876b0 disables the clamp; the same global gates the network tick at
   004e5000, so it reads as a "not running normally" flag.

The split path (004c6f09) multiplies by 00e0b6c8, stores the product to game+21ECh, adds it to
game+64Ch and stores it to game+21F0h, then returns. The image ships 1.0f in 00e0b6c8 and
nothing in this packet writes it. **The debug time controls and the cinematic override below are
not reached on this path.**

The single-view path (004c6f3d) is entered by mode 0 directly and by the other modes when the
active count is one or less:

1. A second step clamp: if the delta exceeds the double in 00d7a270 it becomes 0.05f (00ce7638).
   00d7a270 holds `3FA99999A0000000h`, which is the *double promotion of the float* 0.05f, not
   the nearest double to 0.05; the compare runs in double because the float is loaded with
   `fld dword`. The reconstruction casts the float constant rather than writing a decimal.
2. game+21ECh, game+64Ch and game+21F0h all take the clamped delta.
3. Three held-action multipliers, first match wins, applied to game+21F0h only
   (004c6f85..004c700d): x10.0 (00ce3dc0), x30.0 (00ce7630), x0.3 (00ce3dc8). Each test reads
   the input singleton through 004bec00, offsets `instance+4h` by 1B0h, 1E0h and 240h, and
   checks `+28h` nonzero with `+24h` greater than zero. With the 30h record stride that
   `BSP_InputAction_WasPressedThisFrame` (004c43c0) uses, those are action indices 9, 10 and 12,
   and the test is the *held* half of 004c43c0 without its edge condition.
4. If game+634h is set (cinematic), game+21F0h becomes 0.0f, and then the full 004c43c0 edge
   test on the record at `instance+4h+210h` (action 11) replaces it with 1/30 s (00ce7628,
   `3D088889h`). That is a single-step control for a cinematic.

Consequence worth keeping: **game+64Ch accumulates the undilated step while the frame clock
accumulates the dilated one.** OnMove at 004e4d4a..004e4d63 does `*(float*)(01090ab0 + 4) +=
*(float*)(game+21F0h)`, and 01090ab0 is the frame clock whose +4h field
`bsp::FrameClock::accumulated` already models. Cinematic and slow-motion frames therefore stop
or slow that clock but not game+64Ch.

## State-request queue, game+5D8h

The queue is an MSVC `std::deque<int>`. The producer 004d3ed0 allocates a block with a 10h-byte
`malloc` at 004d3f13 and the drain indexes it as `map[off / 4][off % 4]`, so `DEQUESIZ` is
`10h / sizeof(int)` = 4. Fields relative to game+5D8h:

| Offset | Field |
| --- | --- |
| +0h (game+5D8h) | allocator slot, untouched by push_back and pop_front |
| +4h (game+5DCh) | block map, `int**`; a null slot is an unallocated block |
| +8h (game+5E0h) | block count |
| +Ch (game+5E4h) | head offset, in elements |
| +10h (game+5E8h) | element count |

Block index for an offset: `off / 4`, minus the block count once when it is not smaller
(004e4486 and 004d3eff). One subtraction suffices because neither caller lets the offset reach
two map lengths.

`enqueue` (004d3ed0, `RET 4`) takes a **pointer** to the value (004d3f35 loads through it), so
call sites that look like they push a literal are pushing the address of a stack slot. It grows
when `(head + count) % 4 == 0` and the block count is not greater than `(count + 4) / 4`, and
allocates the target block lazily. The existing ledger record for 004d3ed0 describes the element
as 10h bytes; 10h is the block size and the element is a 4-byte `int`. That address is outside
this packet's lease, so the correction is recorded here rather than applied.

`grow` (004d20d0) enlarges the map by `max(count, max(block_count / 2, 8))` and rotates the
blocks so the sequence starting at the head block stays contiguous, leaving the new slots behind
it. 004d2211 adds only to the block count: **the head offset is not adjusted**, unlike the usual
`_Growmap`. Both native copy branches (004d216d and 004d21a7) produce the same rotation, which
the reconstruction expresses as one modular copy.

`pop_front` (004e44a4) increments the head, wraps it to zero at `block_count * 4`, decrements the
count and resets the head to zero when the queue empties. Blocks are never released.

### Producers

Every `lea ecx,[game+5D8h]` in `.text`, with the constant each site stores into its stack slot:

| Site | Function | Request |
| --- | --- | --- |
| 004d794b | 004d7940 | 0Ah |
| 004d7ae0 | 004d7970 | 10h |
| 004d7f56 | 004d7ea0 | 0Fh |
| 004d8290, 004d8514 | 004d80d0 `BSP_Game_UpdateMultiplayerInterface` | 10h |
| 004e23c8, 004e23fb, 004e242e, 004e270c | 004e2200 | 10h, 12h, 13h, 0Ah |
| 004e3491, 004e34a9 | 004e27e0 | 0Eh, 0Ah |
| 004e479b | 004e4430, the teardown arm | 04h |
| 004e507b | 004e4a40, the state 11h arm | 04h |
| 004e5788 | 004e5540 `BSP_Game_BeginStartupSequence` | register |
| 004cd192, 004d2b3f, 004dd4ae | 004cd0f0, 004d2b10, 004dcf90 | other container methods, not push_back |

## Drain, 004e4430

`__thiscall`, ECX = game, no arguments, `RET`. Already named
`BSP_Game_DrainStateRequestQueue`.

```
while (count != 0) { game+5D4h = front(); pop_front(); dispatch(game+5D4h); }
```

The loop test is at 004e4866. The **ordering rule** is first in, first out, and because the
count is re-read after each handler, a request a handler enqueues is serviced in the same pass,
behind everything already queued. Two arms leave early:

- Request Dh jumps to the tail at 004e47ec with the rest of the queue intact, so the entries
  behind it wait for the next frame.
- Request 13h sets 00e1ae75 to 1 at 004e48cd and returns without the tail. That byte is the
  application exit request that `bsp::ApplicationFrameHost::exit_requested` already models, so
  13h is the quit path.
- One sub-path of the teardown arm (004e471c to 004e48d7) also returns without the tail.

Dispatch, in the order the compare chain tests it:

| Request | Action |
| --- | --- |
| 13h | 00e1ae75 = 1, return |
| 04h | 004e4000 |
| 05h, 08h, 15h, 0Ch | no call |
| 06h | 00e198ac vtable +Ch |
| 07h | 004bfc70 |
| 09h | 00e198b4 vtable +Ch |
| 0Ah, 0Bh | 004dfb70 |
| 0Eh | 004c6b00 |
| 10h | the teardown arm 004e458a: leaves state 11h at 004e4778, enqueues 04h at 004e47a7 when the queue is empty |
| 0Dh | stop the loop |
| 12h | game+5D4h = 0Dh, then 004cd0f0(1,1,1) and a flag through 00e198c4+DCh |
| 14h | 004bac20 |
| 16h | 00e198b8 vtable +Ch |
| 0Fh | 004d7970(0) |
| 02h | 004d8000 |

Tail (004e4873): in state 0Dh, the full 004c43c0 edge test on the record at `instance+4h+180h`
(action 8) runs 004d7970(0); in state 10h, the state becomes 8 when 00e198b4 exists and its +3Ch
byte is set, otherwise 5. Every path that reaches the tail then calls 00a95960 with the scaled
delta at game+21F0h.

### Drain suspension, game+5ECh

OnMove drains only when game+5ECh is zero (004e4d02). Writers found by scanning `.text` for the
displacement:

| Site | Effect |
| --- | --- |
| 004d7f69 (004d7ea0) | set to 1 after enqueuing the debrief request |
| 004e4784 (drain teardown arm) | set to 1 alongside state 11h |
| 004e505b, 004e506b (OnMove state 11h) | set to 1, then cleared unless game+7184h is set |
| 004c7f00 (004c7ed0) | cleared to 0; that function is outside this packet |
| 004ddca7 (004ddb90) | written from `bl`; constructor-shaped, outside this packet |

The state 11h arm (004e504b) writes both values on the fall-through path, so it raises the latch
on entry and lowers it again whenever game+7184h is clear; only then, and only with an empty
queue, does it enqueue request 04h and set 00e198b0 to `game+1EE1h == 0`.

## Pre-tick, 004e2200

`__thiscall`, ECX = game, no arguments, first call of the frame, sole caller OnMove. Its body
returns immediately unless game+640h (a count) is nonzero **and** game+5E8h is zero, so a queued
console command never races a pending state transition. The command list is at game+638h with a
node head at game+63Ch; the front string is fetched through 004cecf0 and compared with 004beb60,
which takes a length and a literal. Recovered command names and their effect:

| Command | Effect |
| --- | --- |
| `frames` (00ce7f54) | handled only in states 5, 0Dh, 8 or 15h; pops the command |
| `term` (00ce7f4c) | enqueue 10h |
| `pause` (00ce4ab0) | enqueue 12h |
| `quit` (00ce7f44) | enqueue 13h |
| `.scn` (00ce7888), `scenes` (00ce7f3c), `universe/scenes/` (00ce7f28), `command_line_mission` (00ce7f10) | the scene-load branch, enqueue 0Ah |

`pause` mapping to 12h and 12h setting state 0Dh means 12h is the resume-into-mission request,
not a pause request. `quit` mapping to 13h corroborates 13h as the exit path.

## Timing block callees

**004c6b20**, `__thiscall`, ECX = game, one float, `RET 4`. Runs immediately after the frame
clock accumulate with the delta slot 004c6e30 rewrote. Gated by `game+634h == 0 || game+635h !=
0`, the same condition as the simulation gate at 004e50b0. With an object at game+38h it calls
00425d10(0) then 00530630, and unless the object's +60h is 4 it either tears the object down
(004c43c0 action 3 pressed: 0045f600, 004bea40 on a stack copy, `game+38h = 0`) or advances it
with the raw delta (0045f970). Reads as cutscene or movie playback.

**0053c510**, `__thiscall`, ECX = **game+19C8h**, no arguments. The map lists it under GUI/text;
the call site at 004e4de2 loads the value of game+19C8h into ECX, so it is a separate object, not
the game. It reads profiler counter slots (0109d014, 0109db08, 0109db34, 0109db14) out of the
profiler's array at instance+20h and accumulates them into `this+68h`, `+6Ch`, `+70h`, `+74h`
with a sample counter at `+78h`. A frame-time statistics accumulator.

**004c0170**, `__thiscall`, ECX = game, no arguments, unconditional. It takes the wall clock from
01090ab0 vtable +20h, which returns a pair of int64s divided on the x87 stack (ticks over
frequency), compares against the last stamp in 00e18b30, and does nothing until 3.0 seconds
(00d7a2b0) have passed. Past that it calls `XUserSetContext` and `XUserSetContextEx` (00a4d3f2,
00a4d41c) with context 8001h among others: a throttled Xbox LIVE presence refresh. This is the
second confirmation that 01090ab0 is the frame clock object.

**004db920**, `__thiscall`, ECX = game, no arguments, state 0Ch only. With mode 0 or the scene
flag at game+1EF0h+29Ch clear it polls the input backend 00f8bbf4 through 00a91020, latches the
result in backend+DDh, and on a rising edge with the menu (00425d10 +25Ch) and GUI (00f8abe8
+3E8h) both idle calls `BSP_InputManager_Update(0.0f)` - the input singleton with a zero delta,
which flushes device state without advancing hold timers. Otherwise, in mode 1, it counts the
game+18CCh slots again with a different pair of byte tests (+9h and +0Eh) and, when fewer than
two are active or game+624h is zero, builds a stack event object through 0075b430 and dispatches
it into the scene at game+1EF0h through 0076a9f0 and 007848f0. Both branches end in 004da6c0.
State 0Ch reads as a controller or profile wait screen.

## Mission completion poll, 004d7ea0

`__thiscall`, ECX = game, no arguments, `RET`, already named `BSP_Game_CheckMissionCompletion`,
sole caller OnMove inside the world tick. It returns at once unless game+5E8h is zero and
game+7188h is non-null, so it waits for an empty request queue. Then it ticks the world object at
game+19CCh three times with a literal zero delta through its vtable +Ch, with 00903670 between
the ticks, calls 004c3cb0(game), and shows the result GUI through 004f8a20 and 004f8970 on
00e18d48 using the float at result+8h and the record at result+14h. If result+21h is set it
enqueues request 0Fh and sets game+5ECh to 1, deferring the next drain. It ends with
004cd610(game, 1) and a tail jump to 004cc510 with ECX = **&game+7188h**, a pointer to the slot
rather than the object, which reads as a smart-pointer reset.

## Reconstruction status

| Address | Name recorded | State |
| --- | --- | --- |
| 004c6e30 | `BSP_Game_ScaleFrameDelta` | reconstructed, build-tested |
| 004e4430 | `BSP_Game_DrainStateRequestQueue` (existing) | reconstructed, build-tested, one focused test |
| 004d3ed0 | `BSP_Game_EnqueueStateRequest` (existing, not leased here) | reconstructed, build-tested |
| 004d20d0 | none (not leased here) | reconstructed, build-tested |
| 004d7ea0 | `BSP_Game_CheckMissionCompletion` (existing) | reconstructed, build-tested |
| 004e2200 | `BSP_Game_RunConsoleCommandQueue` | analyzed |
| 004c6b20 | `BSP_Game_UpdateCutscenePlayback` | analyzed |
| 0053c510 | `BSP_FrameStats_Accumulate` | analyzed |
| 004c0170 | `BSP_Game_UpdatePresenceContext` | analyzed |
| 004db920 | `BSP_Game_UpdateDeviceWaitScreen` | analyzed |
| 007713a0 | none (outside the packet) | identity when mission mode +F4h is zero; otherwise not analyzed |

Nothing here is ABI-compatible or game-validated. The reconstruction is a typed projection: the
queue uses `std::vector<std::unique_ptr<...>>` where the native uses a raw block map, and the
host interface replaces the native call sites.

## Uncertainties and what remains

- The three time-scale multipliers and the cinematic step are debug or cheat controls by shape,
  but the action indices 9, 10, 11 and 12 are not resolved to bindings. The input action table
  belongs to `game_frame_input_tick`.
- 00e0b6c8 ships 1.0f and no writer was looked for outside this packet, so the split-screen scale
  is currently an identity. If a writer exists, split-screen runs on a different time base.
- 00f876b0 disables the 0.25f clamp and also gates the network tick. Its writer is not identified.
- game+5ECh is cleared by 004c7ed0 and written by 004ddb90; neither was analyzed, so the full
  lifecycle of the drain suspension is not closed. A mission that requests the debrief (0Fh)
  latches the suspension, and only the state 11h arm was shown to clear it.
- The teardown arm 004e458a is summarised, not reconstructed: it belongs to the front-end and
  session packets. Its early-return sub-path at 004e48d7 sets state 8 and touches 00e198ac,
  00e198b4 and game+624h.
- game+1EE1h, game+624h, game+7184h and game+19C8h are carried as raw fields.
- The `frames` console command's own sub-dispatch (004e22de..004e23a7) was not decoded past the
  state test and the pop.
