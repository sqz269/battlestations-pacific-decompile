# The application frame's game-state read, profiler pair and pretranslation

Addresses: 00737acc, 00737b33, 004e449e, 004c1dd0, 00be3640, 00be3260, 00be3660, 00be34d0,
00be3820, 00bec1a0, 00bec1d8, 00bec20a, 00c2f1d2, 004f7570, 004f7580, 004f7590

Packet `cc_frontend_states`, read-only analysis. Everything below was read from the stored Ghidra
listing or from `bsp.py disasm-raw` against the disk image. Ghidra was not modified.

Scope: the three application-frame host methods in `docs/GAME_EXECUTABLE.md` that had no
reconstruction behind them, so the executable can make them concrete. The frame itself
(`docs/APP_RUN_FRAME.md`, `src/app_frame.cpp`) and the game update spine
(`docs/GAME_FRAME_CONTROL.md`) are unchanged.

## `ApplicationFrameHost::game_state` is a field read, not a call

`00e188a8` holds four zero bytes in the image and is the **GGame singleton pointer**, not a vtable.
`5D4h` is a field inside the object it points at. The frame loads it at `00737acc` and again at
`00737b33`; both are `MOV EAX,[00e188a8]` followed by a load at `+5D4h`. There is no function
pointer to resolve. `00e18e7c`, which is `00e188a8 + 5d4h` read as if the base were a table, also
holds four zero bytes and belongs to no structure this packet could identify; the nearest named
cell is `00e18e6c`, the unrelated menu-command screen singleton cache.

That is why the 60 frame run counts 120 calls: one field, read twice per frame. The second read is
dead in the native body, because both of its branches run the same exit test at `00737b3d`.

Writers of the field, which is the whole set a host must serve:

| Address | Writes | Route |
| --- | --- | --- |
| `004e3ac2` | 3 | `GGame::OnInit`, the front-end init state |
| `004c9a70` | 2 | `GGame::OnInitTitle`, the title screen |
| `004e449e` | the dequeued request | `BSP_Game_DrainStateRequestQueue`, before its dispatch |
| `004e4279` | 5 | the front-end shell, after `004e4000` finishes |

`bsp::GameStateSlot` in `include/bsp/app_frame_game_state.hpp` is that field plus the two queue
fields the front-end branch reads next to it (`+5E8h` count, `+5ECh` hold). The value meanings are
not re-enumerated: `bsp::GameFrontEndState`, `bsp::GameStateId`, `bsp::kGameStateFrontEndInit` and
`bsp::kGameStateFrontEndShellReady` already carry them.

## The profiler counter pair

`004c1dd0` is the singleton accessor and carries no per-frame logic. The behaviour is in the three
routines it fronts, and all three are now reconstructed.

`00be3820`, the constructor, settles the layout. It sizes everything from one capacity,
`00e15118 + 32h` (`00be384e`), and makes six allocations:

| Field | Contents | Evidence |
| --- | --- | --- |
| `+04h` | float `0.1`, from `00d7a2f0` | `00be3925` |
| `+08h` | float `0.02222222`, from `00d68680` | `00be3932` |
| `+0Ch` | int ring index, zeroed | `00be394a` |
| `+14h` | counter records, capacity * `28h` | `00be3893` |
| `+18h` | float ring, capacity * `14h` | `00be38b9`, sized at `00be3896` |
| `+1Ch` | second float ring, same shape | `00be38d4` |
| `+20h` | float per slot | `00be38ef` |
| `+24h` | dword colour per slot, all `FFFFFF0Fh` | `00be390a`, filled at `00be3965` |
| `+28h` | display records, capacity * `1Ch` | `00be393f` |

The `14h` element run per slot is the history depth, and `00be362e` divides the advanced ring index
by the same `14h`, so the ring holds twenty frames per counter.

`00be3640(slot)` and `00be3660(slot)` both skip slot 0 (`TEST EAX,EAX` at `00be3644` and
`00be3667`) and both reach `records + slot * 28h` through `LEA EAX,[EAX+EAX*4]` then
`LEA reg,[reg+EAX*8]`.

- **Begin**, `00be3260`. Raises the hit count unconditionally, then
  `InterlockedExchangeAdd(&depth, 1)` through `00ce22c4`. The **previous** value decides whether
  this is the outermost entry. On the outermost entry it tests the 64 bit accumulator as two dwords
  (`00be3285`, `MOV EDX,[ESI]` / `OR EDX,[ESI+4]`) **before** storing the sample, then samples
  `QueryPerformanceCounter` through `00ce2270` into `+08h` and, only when the accumulator was zero,
  into `+10h` as well.
- **End**, `00be3660`. `InterlockedDecrement(&depth)` through `00ce2220`; the **new** value decides
  the outermost exit. It then samples the counter, adds the 64 bit delta into `+00h` with one
  `SUB`/`SBB` and `ADD`/`ADC` pair, and stores the end timestamp into `+18h`.

Nested pairs therefore accumulate once, around the outermost pair, and the hit count counts every
entry including nested ones.

`00be34d0` closes the frame over slots `1 .. registered - 1`, where `registered` is
`00e15118 + 0109cf18`. Per slot it copies the hit count into the display record, converts the
accumulated ticks to a float through the scale at `0109db48`, writes it into the history ring and
mirrors it into the per-slot current value, zeroes the hit count, the depth and the accumulator,
and finally writes two normalised offsets: the record's first entry and its last exit, each
measured from the `PERF_APP_UPDATE` record's own first entry, divided by the tick scale, divided
again by `profiler+4h` and biased by the double at `00d7a270`. Both timestamps survive the clear,
which is what makes that ordering safe. Afterwards it clears `0109cf18`, advances the ring index
modulo `14h` and returns the quotient.

**Correction to `docs/APP_RUN_FRAME.md`.** That doc reads the return value as "the pre-wrap index
divided by 14h". The listing at `00be3627` adds one first: `EAX = ring_index + 1`, `IDIV 14h`, the
remainder becomes the new index and the quotient is returned. The conclusion, 1 on the wrapping
frame, is unchanged.

**Uncertain.** The two interlocked import slots carry no symbol in the stored listing.
`InterlockedExchangeAdd` for `00ce22c4` and `InterlockedDecrement` for `00ce2220` are inferred from
the pushed argument count and from which value the caller tests, not from a name.
`0109db48` is zero in the image and filled at run time; its unit is unproven, so the converted
values are "ticks over an unknown scale", not proven seconds. The colour dword's channel order is
unproven: the default is `FFFFFF0Fh` and the application frame's own colour at `00e1ae94` is
`FF000000h`, which is consistent with several orders.

**Modelling caveat.** The native conversions run on the x87 stack, so each divide happens at 80 bit
precision and rounds once on the store to float32. The reconstruction divides in `double` and casts
once, which can differ in the last bit.

## Pretranslation: the host table cites the wrong instruction

`00bec20a` is `MOV byte ptr [ESI+43h],1`, the loop-finished store at the exit of
`BSP_Win32Platform_RunLoop`. It is reached once per process. The pretranslation is the call at
**`00bec1d8`**, which enters the thunk `00c2f1d2`, itself a `JMP dword ptr [00ce25dc]` to
`XLivePreTranslateMessage` (ordinal 5030, `docs/XLIVE_NOTIFICATIONS.md`). The 8 calls the 60 frame
run recorded are 8 messages, which matches the call inside the `PeekMessageA` success arm and not
the once-per-process store.

`docs/GAME_EXECUTABLE.md` lists `PlatformLoopCallbacks::pretranslate` against `00bec20a`. The
corrected site is `00bec1d8`. That doc is owned by another packet, so the correction is recorded
here rather than applied there.

The loop's disposition rule, from `00bec1dd`: a nonzero result skips both `TranslateMessage`
(`00ce233c`) and `DispatchMessageA` (`00ce2338`) entirely, so a consumed message never reaches the
window procedure. With no XLive library bound there is no consumer and every message must be
dispatched, which is what `bsp::platform_pretranslate_consumes_00bec1d8` states.

## Host methods the executable must implement, in call order

Everything below is per frame unless noted. The addresses are the native call sites.

| Order | Host method | Native site | Now available |
| --- | --- | --- | --- |
| 1 | `ApplicationFrameHost::profiler_set_frame_slot_color` | `00737a9e` | `bsp::profiler_set_slot_color` with `bsp::frame_marker_color_00737a6c` |
| 2 | `ApplicationFrameHost::profiler_begin_frame_slot` | `00737ab4` | `bsp::profiler_begin_frame_slot_00be3640` |
| 3 | `ApplicationFrameHost::game_state` | `00737acc` | `bsp::read_game_state_00737acc` over a `bsp::GameStateSlot` |
| 4 | `ApplicationFrameHost::game_on_move` | `00737b26` | `bsp::run_game_frame_control` then `bsp::run_front_end_state_frame`; see `docs/FRONTEND_STATE_MACHINE.md` |
| 5 | `ApplicationFrameHost::game_state` again | `00737b33` | same slot; the result is discarded |
| 6 | `ApplicationFrameHost::profiler_end_frame_slot` | `00737b98` | `bsp::profiler_end_frame_slot_00be3660` |
| 7 | `ApplicationFrameHost::profiler_end_frame` | `00737bac` | `bsp::profiler_end_frame_00be34d0` |
| — | `PlatformLoopCallbacks::pretranslate` | `00bec1d8` | `bsp::platform_pretranslate_consumes_00bec1d8`, per message |

The profiler host also needs one method of its own, `bsp::ProfilerClockHost::query_performance_counter`
(`00ce2270`), because the native samples the counter only on the outermost entry and exit; calling
it eagerly would change the observable call count.

The drain writes the state field through `bsp::apply_drained_game_state` at `004e449e`, so the same
`GameStateSlot` the frame reads is the one `bsp::drain_state_requests_004e4430` advances.

## Reconstruction

`include/bsp/app_frame_game_state.hpp` and `src/app_frame_game_state.cpp`.

No new tests. The profiler routines are integer bookkeeping over caller-owned arrays with no
numeric behaviour the existing `reconstructed_math` suite does not already cover, and the
pretranslation rule is one boolean.

## State reached per routine

| Address | State |
| --- | --- |
| 00737acc, 00737b33 | analyzed; the read is modelled, the frame itself is unchanged |
| 00be3260, 00be3640, 00be3660 | analyzed, reconstructed, build-tested |
| 00be34d0 | analyzed, reconstructed, build-tested; the display record's other fields are unidentified |
| 00be3820 | analyzed for the layout only; the tail past 00be3983 was not read |
| 00bec1d8, 00bec20a | analyzed; the disposition rule is reconstructed |
| 004c1dd0 | analyzed previously; unchanged here |

None of this is ABI-compatible or game-validated.

## What remains

- The unit of `0109db48` and therefore the profiler's output unit.
- Fields `+00h..+13h` of the `1Ch` byte display record.
- The tail of `00be3820` past the colour fill, which reads `0109db08`.
- `00408720`, the counter registration routine, and the meaning of the `0Fh` it is passed with each
  counter name.
- Whether `profiler+8h`, the `1/45` constant, is read anywhere.
