# Application frame

Addresses: 00737a50, 004c1dd0, 004c43c0, 004fde20, 00be34d0, 00be3640, 00be3660, 00be3260

The application frame is the body behind application vtable `00cfeab0` slot +10h. The platform
frame wrapper `00bece70` loads the application from platform+48h, calls the slot, then calls
`00becb20(false)`. The frame is 371 bytes, `00737a50..00737bc2` inclusive, and every byte in the
spans listed above was compared against the disk image before analysis
(`reports/app_run_frame_audit.json`).

The frame does not pump messages, poll devices, kick rendering, present, throttle or sleep, and it
does not handle focus or pause. It sequences a profiled scope, one input edge test, the frame clock,
the game update, exit propagation, and two loader pumps. The message pump lives in the platform loop
(`docs/PLATFORM_LOOP.md`); simulation and rendering are reached through the game update
(`docs/GAME_FRAME.md`).

## Recovered sequence

| Address | Step |
| --- | --- |
| 00737a6c | One-time init of the profiler bar colour, guarded by bit 0 of `00e1ae98` |
| 00737a98 | `EDI` = PERF_APP_UPDATE slot index from `0109d014` |
| 00737a9e | Profiler singleton `004c1dd0`; `instance+24h` colour array slot = `00e1ae94` |
| 00737ab4 | Profiler singleton again, then begin counter `00be3640(slot)`; trylevel becomes 0 |
| 00737ac0 | Clear application+19h |
| 00737acc | Read game state `*(00e188a8)+5D4h` |
| 00737ae7 | Non-mission states only: `004c43c0(0Eh)`; a true result sets application+19h |
| 00737af6 | Frame clock `01090ab0` virtual +8h, `00bedc30`, advances the clock |
| 00737b03 | Frame clock virtual +1Ch, `00bee070`, returns the interval timestamp at clock+40h |
| 00737b10 | `FILD`/`FILD`/`FDIVP` on ticks and frequency, spilled to float32 |
| 00737b26 | `004e4a40` game update with `ECX = *(application+14h)` and the float on the stack |
| 00737b2b | Clear application+18h |
| 00737b33 | Re-read the game state; both branches then run the same exit test |
| 00737b4e | Sticky exit: nonzero `00e1ae75` stores 1 into platform `0109cf04`+181h |
| 00737b79 | `00bdb0b0` VFS provider tick with `ECX = 0109ceec` |
| 00737b84 | Loading queue singleton `004fde20`, then `00509190` update |
| 00737b90 | Trylevel back to -1, then end counter `00be3660(slot)` |
| 00737bac | End of profiler frame `00be34d0`; the returned wrap flag is discarded |

The whole body runs inside one SEH frame with handler `00c86228`. Trylevel is 0 only between the
begin and end counter calls, so the counter close is a scope guard rather than ordinary cleanup.
`00737ab0` writes the slot index into a local that no later instruction reads; the decompiler
folds that dead local into the saved exception-list slot and emits `ExceptionList = pvVar1`, which
is wrong. The assembly restores the exception list from `[ESP+14h]` at `00737bb1`.

## Calling conventions and RET sizes

| Address | Convention | RET |
| --- | --- | --- |
| 00737a50 | `__thiscall`, ECX = application, no stack arguments, void | `RET` |
| 004c1dd0 | no arguments, returns the profiler singleton in EAX | `RET` |
| 004c43c0 | one stack argument, caller-cleaned, returns AL | `RET` |
| 004fde20 | no arguments, returns the loading queue singleton in EAX | `RET` |
| 00be3640 | `__thiscall`, ECX = profiler, one stack argument | `RET 4` |
| 00be3660 | `__thiscall`, ECX = profiler, one stack argument | `RET 4` |
| 00be34d0 | `__thiscall`, ECX = profiler, returns int in EAX | `RET` |
| 00be3260 | `__thiscall`, ECX = counter record, no stack arguments | `RET` |

Ghidra types `00be3640` as a plain function whose `param_1` is the stack slot, which hides the
`ECX` profiler pointer. The call sites at `00737ab4` and `00737b98` load `ECX` from the singleton
result, and both callees clean four bytes, so both are `__thiscall`.

## Profiler subsystem

`004c1dd0` is the double-checked singleton accessor for the performance profiler: it takes the
singleton-lifetime critical section, allocates 30h bytes, constructs through `00be3820`, registers
with `00bd0c30`, and caches the pointer in `0109db50`.

Counter records are 28h bytes in the array at `profiler+14h`. The static initialiser at
`00cd8afc..00cd8b20` shows that array is `0109d018`: it forms `0109d018 + 28h*n`, passes the
string `PERF_APP_UPDATE` at `00d686a8` and the value 0Fh to `00408720`, stores `n` into
`0109d014` and increments the static counter count `00e15118`. The next string is
`PERF_GAME_ONMOVE`, so the counter names are a contiguous table.

| Offset | Meaning | Evidence |
| --- | --- | --- |
| +00h | int64 ticks accumulated this frame | added at `00be36a1`/`00be36a9`, zeroed in `00be34d0` |
| +08h | int64 start of the current sample | written at `00be3292`, subtracted at `00be3699` |
| +10h | int64 first entry this frame | written at `00be329a` only when +00h is zero |
| +18h | int64 last exit this frame | written at `00be36a3`/`00be36a6` |
| +20h | LONG active depth | interlocked add of 1 at `00be3270`, decrement at `00be3679` |
| +24h | int hit count this frame | incremented at `00be3266`, zeroed in `00be34d0` |

`00be3640` converts a slot index to a record and forwards to `00be3260`, which increments the hit
count, increments the depth, and on the outermost entry only samples `QueryPerformanceCounter`
into +08h, additionally into +10h when the accumulator is still zero. `00be3660` decrements the
depth and, on the outermost exit, samples the counter again, adds the 64-bit delta into +00h and
stores the end timestamp into +18h. Nested samples therefore accumulate once per outermost pair.

`00be34d0` closes the frame. For every slot from 1 to `0109cf18 + 00e15118` it copies the hit count
into the display record at `profiler+28h` (stride 1Ch, count at +14h), converts the accumulated
ticks to a float through the scale in `0109db48` and writes it both into the history ring at
`profiler+18h` and into the current-value array at `profiler+20h`, then zeroes the record's hit
count, depth and accumulator. It also writes normalised start and end offsets into `profiler+1Ch`
and the display record at +18h, each computed relative to the PERF_APP_UPDATE record's own
timestamps, divided by the scale at `profiler+4` and biased by `00d7a270`. The ring holds 14h
frames: the index at `profiler+0Ch` advances modulo 14h and the return value is the pre-wrap index
divided by 14h, so it is 1 on the frame that wraps. `0109cf18`, the count of slots registered
during the frame, is cleared here; `00e15118` counts the statically registered ones.

## Input edge test

`004c43c0` reads the singleton from `004bec00` (cached in `00f8bbf8`, constructed by `00a93da0`),
indexes `instance+4` with a 30h stride, and returns 1 when +28h is nonzero, +24h is greater than
zero, and either +20h is zero or +1Ch is not greater than zero. The +1Ch/+20h pair mirrors the
+24h/+28h pair, so the test is a rising edge: held now with positive hold time, and not held with
positive hold time before. Thirty-two call sites pass different constant indices; index 0Eh is not
identified.

## Application object fields

Constructor `00737970` installs vtable `00cfeab0`, zeroes +08h, +0Ch, +10h and +14h, stores 1 at
+18h and 0 at +19h and +1Ah. Slot +14h holds the game object used as `ECX` for the update.

`+18h` starts true and is cleared after the first update, so it reads as a first-update flag; no
other writer has been found, so a re-arm path cannot be ruled out. `+19h` is cleared at the top of
each frame and set from the input edge before the update, so it is a one-frame request latch that
the update consumes. Neither consumer is recovered, because the game update body is not analysable
(see below).

## Exit propagation

`00e1ae75` is set by the window-close policy in `004ca2f0` and by the confirmation callback
`004bbc50` (`docs/WINDOW_CLOSE.md`). The frame copies it into platform `0109cf04`+181h, which the
message loop tests to set finished+43h (`docs/PLATFORM_LOOP.md`). The copy is one-directional: a
false global never clears an already-set +181h. The frame returns no value, so the loop's exit
decision rests entirely on that byte.

## Callers and callees

Caller: application vtable `00cfeab0` slot +10h, reached from `00bece70`. Ghidra records no direct
call reference because the dispatch is virtual.

Callees in frame order: `004c1dd0`, `00be3640`, `004c43c0`, clock virtual +8h `00bedc30`, clock
virtual +1Ch `00bee070`, `004e4a40`, `00bdb0b0`, `004fde20`, `00509190`, `00be3660`, `00be34d0`.

## Uncertainties and what remains

- Input action 0Eh has no recovered name. The action table built by `00a93da0` was not opened.
- Game states 1, 2 and 4 are still unnamed; they are treated here only as the mission set, which is
  the same set the close policy uses.
- The scale at `0109db48` and the bias at `00d7a270` are read as doubles and not otherwise
  identified, so the profiler's output unit is unproven. The 0Fh argument passed with each counter
  name is unexplained.
- `00408720`, the counter registration routine, and `00be3820`, the profiler constructor, were not
  opened.
- `004e4a40` still has an eight-byte stored Ghidra body, so its callees, its use of application+18h
  and +19h, and the render and present phases inside it could not be traced from this packet.
- The SEH handler `00c86228` was not disassembled; the unwind action is inferred from the trylevel
  transitions.
- Whether anything re-arms application+18h after the first frame is unresolved.

## State reached

| Address | State |
| --- | --- |
| 00737a50 | reconstructed, build-tested |
| 004c1dd0 | analyzed |
| 004c43c0 | analyzed |
| 004fde20 | analyzed |
| 00be3640 | analyzed |
| 00be3660 | analyzed |
| 00be34d0 | analyzed |
| 00be3260 | analyzed |

`bsp::run_application_frame` in `include/bsp/app_frame.hpp` and `src/app_frame.cpp` projects the
frame order onto an injected `ApplicationFrameHost`, one method per native call site, and reuses
`timestamp_seconds_x87` so the interval conversion keeps the native `FILD`/`FILD`/`FDIVP` result.
It is a semantic projection, not an ABI-compatible replacement: the native routine takes the
application in `ECX`, returns nothing, and reads its subsystems from globals. No profiler, input,
clock, VFS or loading-queue implementation is supplied and none of the host methods have defaults.
No test was added; the existing suite still passes.
