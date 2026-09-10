# Game front-end states 1, 2 and 4 (packet `game_frontend_states`)

Addresses: 00685170, 0068d850, 004f8830, 004f71f0, 004b6e50, 00425d10, 0054e440, 004f7180,
004f71a0, 004f71d0, 004e4000, 004d7920, 0068d8a0, 0068d8d0, 00685070, 00685300, 004d7f90,
0068d760

Scope: the front-end branch of `BSP_Game_OnMove` (004e4a40), listing range 004e4b9d..004e4d2c.
`BSP_Game_OnMove` itself was read only. Everything below comes from the disk listing
(`bsp.py disasm-raw`), not from the decompiler, because the stored Ghidra body for OnMove is
eight bytes. Ghidra was not modified.

## What game state 4 is

`docs/APP_INIT_GAME_ENTRY.md` names 1 (logo sequence) and 2 (title screen) but leaves 4 blank,
and no instruction anywhere in `.text` writes the immediate 4 into `game+5D4h`. The value only
reaches the field through `BSP_Game_DrainStateRequestQueue` (004e4430), which stores the dequeued
request at 004e449e and then dispatches on it; the case for 4 at 004e44f3 calls **004e4000**.

004e4000 destroys the title-screen singleton at `00E198C8` through its virtual `+0h`, nulls the
global, and loads the front-end resource sets. Its string immediates settle the question:

| String | Address |
| --- | --- |
| `GILoading::SLM_LOAD_FRONTEND` | 00CE8254 |
| `GILoading::SLM_LOAD_FRONTEND_RETURN` | 00CE8274 |
| `Effects before mainmenu` | 00CE8298 |
| `Sounds before mainmenu` | 00CE82B0 |
| `Textures before mainmenu` | 00CE82C8 |

The label is chosen at 004e407f..004e40ab: a nonzero `game+719Ch` (the mission-init once guard)
selects `_RETURN`, a zero selects the plain form. So **state 4 is the front-end / main-menu shell**,
entered either from the title screen on a cold boot or on the way back from a mission. It is a
different thing from state 2, which is the title screen proper. `BSP_Game_ProcessWindowCloseRequest`
(004ca2f0) treats 1, 2 and 4 alike, which is why `is_front_end_game_state` in
`include/bsp/app_frame.hpp` is correct as written.

State 4 receives **no per-state update** in OnMove. The compare at 004e4bfb falls straight to the
shared tail, so the front-end shell is driven entirely by the screen registry.

## Dispatch structure

Entered at 004e4b9d with `eax = game+5D4h`; `ebx = 1` and `ebp = 0` throughout the branch.

1. **State test** 004e4ba3..004e4baf. `1`, `2` or `4` enter the branch; anything else jumps to the
   fallback at 004e4cdd.
2. **Render-queue open** 004e4bb5..004e4bdb, guarded by `game+34h == 0`.
   `BSP_RenderCommandQueue_GetSingleton` then `BSP_RenderCommandQueue_GetRetentionControl`; a
   **nonzero** retention control jumps to 004e4ccb, which skips the per-state update *and* the
   render, not just the render. Otherwise the renderer at `00F8D394` runs vtable `+0Ch` and
   `game+34h` becomes 1.
3. **State re-read** 004e4bde. The state is fetched from `game+5D4h` a second time, after the
   renderer virtual.
4. **State 1** 004e4be8. `BSP_LogoSequence_PollSkip(00E198A4)`, then the shared tail.
5. **State 2** 004e4c01..004e4c96.
   - If `00E198C8` is null, `004db220(game)` runs and the global is re-read. 004db220 is
     `004db190` + `004cccc0` + `game+2180h = 0` + a tail jump to `GGame::OnInitTitle` (004c9a70),
     which is what builds the 0x44 byte title object.
   - If it is now non-null, `BSP_TitleScreen_Update(title, rawDelta)`.
   - The GUI branch below runs whether or not the title object existed.
6. **GUI branch** 004e4c29..004e4c96, on `*(char *)(*(00F8ABE8) + 0x3E8)`:
   - **Nonzero**: `BSP_GuiManager_GetOrCreate()` then `00aa0e50(gui, 0)`, then the accessor again
     then `00aa0e00(gui, 0)`. Straight to the shared tail.
   - **Zero**: `00aa0e00(gui, 1)`, then the menu-command screen:
     `BSP_MenuCommandScreen_GetSingleton()` four times over; if `+5h` (active) is zero, nothing;
     else if `+4h` (wanted) is set, `BSP_FrontEndScreen_Update(cmd, game+21F0h)`; else
     `BSP_FrontEndScreen_Close(cmd)`.
7. **Shared tail** 004e4c9b..004e4cc9. `BSP_FrontEndScreens_Pump(rawDelta)`,
   `BSP_GuiManager_GetOrCreate()` then `BSP_GuiManager_Update(gui, rawDelta, 0)`,
   `BSP_Game_Render(game)`, `BSP_Game_FinishRenderFrame(game)`.
8. **Pending check** 004e4ccb. Reads `*(00E188A8) + 5E8h`, the request count, through the global
   mirror rather than through ESI. Zero returns; nonzero falls into the fallback.
9. **Fallback and drain** 004e4cdd..004e4d0d. The same render-queue open (retention result
   discarded this time), then `BSP_Game_DrainStateRequestQueue(game)` unless `game+5ECh` is set.
10. **Post-drain state test** 004e4d12..004e4d2c. Three separate compares against 1, 2 and 4 that
    share one epilogue. If the drain left a front-end state the call returns; otherwise the same
    OnMove call runs on into the timing block at 004e4d32 **as the new state**.

Only the argument attribution in step 6 needed a decision. `BSP_GuiManager_GetOrCreate` is called
with no push at 004e4ca8 in the shared tail, so the `push ebp` / `push ebx` at 004e4c38, 004e4c45
and 004e4c54 are arguments to `00aa0e50` and `00aa0e00`, not to the accessor. The independent site
at 004c6da5 has the same shape with a computed boolean. The map note in `docs/GAME_ON_MOVE_MAP.md`
that reads these as `BSP_GuiManager_GetOrCreate(1)` is superseded.

## The `*(00F8ABE8) + 3E8h` GUI branch

Three sites read this byte and all three make it a suppression flag:

| Site | Effect when the byte is nonzero |
| --- | --- |
| 004e4c2f | the state 2 branch clears the GUI screens and disables the GUI |
| 004c6d96 | the GUI-enable argument computed above is forced to false |
| 004ca327 | `BSP_Game_ProcessWindowCloseRequest` drops the request entirely |

`00F8ABE8` is filled at 00a3f586, inside 00a3f530 in the online / system-manager segment (segment
71, keywords `xenonsystemmanager`, `mnetworkclientxlive`, `changestate`), and
`BSP_Game_RequestReturnToTitle` tail jumps to 00A3E6A0 with it in ECX. **Uncertain**: the only
writer of the byte found by a full `.text` scan for `mov byte ptr [reg+3E8h], imm8` is 009e04b0,
which clears `+3E8h`, `+3E9h` and `+3EAh` together but in a segment with no established link to
this object. The owner of the flag and what sets it are not established; only its three read
sites are. Treat "front-end GUI suspended while a system-level operation is up" as provisional.

## The front-end screen hierarchy

Constructor 004f7180 stores vtable `00CEAE54` and clears two bytes: `+4h` is the requested state
and `+5h` is the state the pump has applied. Slot `+0h` of that vtable is `__purecall`, and
`BSP_FrontEndScreen_Register` (004f71d0) calls it with no arguments and uses EAX as the index into
a fixed pointer array, so each leaf type supplies its own screen id.

| Vtable slot | Target in the base | Role |
| --- | --- | --- |
| +00h | `__purecall` | screen id, used as the registry index |
| +10h | 004f71d0 | register |
| +18h | 004f75a0 | enter |
| +1Ch | 004f75b0 | exit |
| +20h | 004f75c0 | update(float) |
| +24h | 004f75d0 | fills the list 004f83b0 walks |

The registry is the pointer array `00E18B60..00E18CDB`, **95 slots**; `00E18CDC` is a separate byte
that the pump clears at 004f8881 and that OnMove clears again at 004e544c and 004e5481. The
destructor 004f71a0 scans the same range and nulls every slot holding `this`.

`BSP_MenuCommandScreen_GetSingleton` (00425d10) returns a member of this hierarchy: its constructor
00533120 chains 004f7180 before installing two further vtables at `+8h` and `+0Ch`, which is also
why the singleton lifetime manager is registered with `instance+0Ch`. Its string table
(`globals.back`, `globals.accept`, `globals.yes`, `globals.no`, `globals.exitgame`,
`globals.exittomenu`, `globals.restartmission`, and the `globals.dialog_*` forms) identifies it as
the shared menu command and confirmation-dialog screen. Fields observed: `+4h` wanted, `+5h`
active, `+25Ch` modal-dialog flag, `+188h` and `+218h` polled by the OnMove pause gate.

## The shared tail versus the blocking-screen render

Both tails call `BSP_GuiManager_GetOrCreate`, `BSP_GuiManager_Update(delta, 0)`, `BSP_Game_Render`
and `BSP_Game_FinishRenderFrame` with the raw delta. Three differences:

- The blocking path reaches the renderer through `BSP_Game_TryBeginRenderFrame` (004c6c30) at
  004e4b4e. The front-end path open-codes the equivalent at 004e4bba against the global renderer
  `00F8D394` and the queue singleton, and sets `game+34h` itself.
- On a retained queue the blocking path simply skips the render and continues; the front-end path
  skips the per-state update as well and lands on the pending-request check.
- The front-end path runs `BSP_FrontEndScreens_Pump` before the GUI update. The blocking path does
  not touch the screen registry at all.
- The blocking path returns unconditionally at 004e4b8c. The front-end path only returns when the
  request ring is empty or when the drain kept a front-end state.

## Transitions out of each state

**State 1 to state 2.** `BSP_LogoSequence_PollSkip` computes `elapsed` as
`BSP_Timestamp_Subtract(now, this+68h)` reduced to seconds by `fild[eax] / fild[eax+8]`, compares
it against the float at `this+78h`, and requires **both** `elapsed > duration` **and** the
edge-triggered input query `004d92b0(game, 0x4A)` before tail-jumping to
`BSP_LogoSequence_AdvanceOrFinish`. That routine advances the index at `this+60h` through the
8-byte-element vector at `this+44h..+48h`; when the list is exhausted it runs the object's virtual
`+0h` with 1 and then `BSP_Game_OnInitOnce(game, 0)`, which re-runs `GGame::OnInitTitle` and writes
`game+5D4h = 2` directly. The logo table comes from `Scripts/datatables/Logos.lua` through
`BSP_LogoSequence_LoadTable` (fields `delay`, `(platform)`, `(region)`, `Szplessek`).

**Uncertain**: because the two conditions are ANDed, this body cannot advance the sequence on time
alone. The automatic advance is not in it. The logo object is the 0x80 byte allocation made at
004e57bc and it is not shown to be a member of the 00CEAE54 hierarchy, so where the timed advance
lives is unresolved.

**State 2 to state 4.** `BSP_Game_RequestState` (004d7920) is a one-line wrapper over
`BSP_Game_EnqueueStateRequest`. Of its 28 call sites, exactly two push 4: 0068d8bc in
`BSP_TitleScreen_Skip` and 0068d90e in `BSP_TitleScreen_Advance`, which is slot `+4h` of the title
vtable `00CF7A98`. Both are guarded by `game+5E8h == 0` (nothing already queued) and both then
clear `game+5ECh`. `GGame::OnInitTitle` calls the virtual normally and calls
`BSP_TitleScreen_Skip` instead when `skipTitle` is set.

**Into state 4 from a mission.** OnMove's state 0x11 branch enqueues 4 at 004e5074, guarded by
`game+7184h == 0` and an empty ring, and it drives `game+5ECh` on the way. So state 17 hands off to
the front-end shell, which then picks the `_RETURN` loading label because `game+719Ch` is already
set. The drain holds a second copy of the same enqueue block at 004e478b. **Uncertain**: that copy
is *not* reached from the state-17 store at 004e4778, which sets `game+5ECh = 1` and jumps past it
to 004e47bd; the predecessor of 004e478b was not identified.

**Out of state 4.** `BSP_Game_PollPlatformSessionEvents` at 004db311 is the only compare against 4
outside OnMove: in state 4 it calls `BSP_Game_RequestReturnToTitle`, whereas every other state gets
the inline `GGame::OnInitTitle` path. Menu screens leave state 4 through `BSP_Game_RequestState`
with other values (9, 11, 16, 22 and others appear at the 28 call sites).

## 0054e440 is not on the front-end path

The packet listed it, and the map filed it under "front-end button segment", but its only OnMove
call site is 004e522d, inside the in-mission pause branch of the simulation gate, with
`ECX = *(00E198C4 + 0xA8)` and guarded by that object's byte `+8h`. It clears its own `+8h`/`+9h`,
zeroes a float at `+54h`, stops two force-feedback effects, then reads the current hint id from the
award tracker and advances it one step: `BASICPLANE` to `BASICPLANE2` (a raw `strcmp` plus a fresh
0xB byte buffer), then through the native-string comparison 00425850 `BASICPLANE2` to
`BASICPLANE3`, `BASICSHIP` to `BASICSHIP2`, `BASICSUB` to `BASICSUB2`. It hands the result to
00690fd0, whose body holds `hints.missionhint` and the HintSystem Lua strings. It re-shows the next
basic-training hint when the pause menu opens.

## Calling conventions and RET sizes

| Address | Convention | Stack args | RET | ECX |
| --- | --- | --- | --- | --- |
| 00685170 | `__thiscall` | none | 0 | 00E198A4, the logo object |
| 0068d850 | `__thiscall` | float raw delta | 4 | 00E198C8, the title object |
| 004f8830 | `__stdcall` | float raw delta | 4 | unused |
| 004f71f0 | `__thiscall` | float seconds | 4 | any front-end screen |
| 004b6e50 | `__thiscall` | none | 0 | any front-end screen |
| 00425d10 | `__cdecl` | none | 0 | unused |
| 0054e440 | `__thiscall` | none | 0 | `*(00E198C4 + 0xA8)` |
| 004e4000 | `__thiscall` | none | 0 | the game object |
| 004d7920 | `__thiscall` | int state | 4 | the game object |
| 0068d8d0 | `__thiscall` | none | 0 | the title object |

## Reconstruction

`include/bsp/frontend_states.hpp` and `src/frontend_states.cpp`.

- `run_front_end_state_frame` is the dispatch of 004e4b9d..004e4d2c over `FrontEndFrameHost`, one
  method per native call site, in the style of `run_application_frame`. It returns
  `FrontEndFrameOutcome::ContinueToSimulation` for the fall-through at 004e4d32.
- `run_front_end_screen_pump` is a real reconstruction of 004f8830, including the modal-dialog
  guard and all three passes with their differing commit order.
- `close_front_end_screen` and `close_menu_command_screen` are 004b6e50.
- `GameFrontEndState` carries the recovered meanings; `is_front_end_game_state` is reused from
  `bsp/app_frame.hpp` rather than redeclared.
- `FrontEndScreen`, `FrontEndScreenTable`, `MenuCommandScreen`, `TitleScreen`,
  `GameFrontEndFrameState` and `FrontEndWorld` carry the recovered offsets as comments.

No new tests. The routines are control flow over an injected host with no numeric behaviour, and
the existing `reconstructed_math` suite covers the arithmetic they touch (none).

## State reached per routine

| Address | State |
| --- | --- |
| 004e4b9d..004e4d2c dispatch | analyzed, reconstructed, build-tested |
| 004f8830 | analyzed, reconstructed, build-tested |
| 004b6e50 | analyzed, reconstructed, build-tested |
| 004f71f0 | analyzed (one virtual dispatch, modelled as a host call) |
| 00425d10 | analyzed (singleton pattern and object identity; not reconstructed) |
| 00685170 | analyzed (skip poll only; the timed advance is unresolved) |
| 0068d850 | analyzed (two forwarding calls) |
| 0054e440 | analyzed (out of the front-end path; not reconstructed) |
| 004f7180, 004f71a0, 004f71d0 | analyzed |
| 004e4000 | analyzed to the point that names state 4; the body past 004e4130 was not read |
| 004d7920, 0068d8a0, 0068d8d0, 004d7f90, 0068d760 | analyzed |
| 00685070 | analyzed |
| 00685300 | partially analyzed: the Lua table load only |

None of this is ABI-compatible or game-validated.

## What remains

- The timed advance of the logo sequence, and the entry point that calls
  `BSP_LogoSequence_LoadTable` (the index records no callers for it).
- The owner and the writer of `*(00F8ABE8) + 3E8h`.
- The identity of `00F8BBF4`, updated by `BSP_TitleScreen_Update` through vtable `+4h`; it is
  built at 00a908f6.
- The screen ids: the registry is indexed by a per-leaf virtual, so no id-to-screen table was
  recovered and the 95 slots are unnamed.
- The body of 004e4000 past its loading-label setup, which is where the front-end shell's screens
  are actually created.
- `00aa0e00` and `00aa0e50` are external contracts owned by `game_blocking_screen`; only their
  argument attribution was settled here.

## Correction from docs/GAME_FRONTEND_ENTRY.md

Game state 4 never survives `BSP_Game_EnterFrontEndShell`: `GGame::OnInit` stores `game+5D4h = 3` at `004e3ac2`, the gate at `004e4151` compares against that 3, and `004e4279` writes 5. So 4 is a drain request value only and the main-menu shell rests at state 5 (listed as "no call" in the drain table); the gate fails only when the platform session poll moved the state off 3, the sign-out abort path.
