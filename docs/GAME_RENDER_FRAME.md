# Game render frame (004ca440 and its begin/finish pair)

Addresses: 004ca440, 004c6c30, 004ca1f0, 004bfbf0, 004c0960, 004c0a30, 004c1130, 00503510,
00535430, 0059d7b0, 00a8f3b0

Packet `game_render_frame`. The eleven leased addresses are `BSP_Game_Render` and the two
routines that bracket it, plus the eight direct callees this packet owns.
`docs/GAME_BLOCKING_SCREEN.md` (rows 004e4b4c, 004e4b6c, 004e4b73) and
`docs/GAME_RENDER_TAIL.md` (rows 004e54bc, 004e54eb) are the two callers; this document
replaces their one-line summaries of the render entry with recovered behaviour. The render
command queue (00b1ebe0, 00b1ef60, 00b0d190), the light and shadow objects (00b7aab0), the
renderer end-frame body (00b2d8e0) and the GUI manager are external contracts here and are
named, not analysed.

## Frame phase, game+34h

One `int`, three values, three plain stores. No interlocked access anywhere.

| Phase | Set by | Meaning |
| --- | --- | --- |
| 0 | 004ca2c9 | idle; only 004c6c30 leaves this state |
| 1 | 004c6c5f | renderer BeginFrame has run |
| 2 | 004ca46b | callbacks issued, the frame awaits its present |

`004c6c30` returns 1 for **any** non-idle phase without touching the renderer (004c6c36), which
is why the in-mission frame may call it early and `BSP_Game_Render` still runs its own gate.
`BSP_Game_Render` returns immediately unless the phase is 1, and advances to 2 *before* the
first callback, so a re-entrant call cannot render twice.

## The begin/finish pair reaches D3D9, not a lost-device test

The renderer singleton is `00f8d394`, whose primary vtable is `00d5f0a8`
(`docs/APP_INIT_RENDERER.md` row 4c). Reading that table settles the question:

| Slot | Target | Reached |
| --- | --- | --- |
| +0Ch | `00b2b200` `BSP_D3D9Renderer_BeginFrame` | device vtable +A4h, `IDirect3DDevice9::BeginScene` |
| +10h | `00b2f4b0` | `JMP 00b2d8e0` `EndFrameAndPresent`, one argument |
| +14h | `00b2f4a0` | `PUSH 0; CALL 00b2d8e0`, the same body with a zero argument |

So `004c6c30` is **begin-scene** and `004ca1f0` is **end-scene plus present**. Neither is a lost
device test: `00b2b200` tests the renderer's own inhibit counter at +1D90h and its lost flag at
+1D8Ah before calling BeginScene, and always returns 1 regardless.

## What the +10h argument is: screenshot capture

`004ca1f0` picks +14h or +10h on the byte `00e188ad`. The +10h argument comes from `004c9a00`,
which formats `"%s%04d\scr%04d.png"` from a base directory, a sequence number and an image
counter, and returns its own incoming argument unchanged. `00e188ad` is therefore
**screenshot capture enabled**, and the whole cluster falls out:

| Global | Role |
| --- | --- |
| `00e188ad` | capture enabled; toggled in `BSP_Game_Render`, read in `BSP_Game_FinishRenderFrame` |
| `00e08210` | sequence, the `%04d` directory |
| `00e188b8` | image counter inside that directory, post-incremented by 004c9a00 |
| `00e1899c` | base directory override; `00e18b1c` is the fallback buffer |

`004bfbf0` opens a new sequence: it increments `00e08210` **before** its first probe, formats
`"%s%04d"` and calls `00bf94c6`, looping while that returns non-zero, then stores the zero into
the image counter. The CRT helper `00bf94c6` is unidentified; `_mkdir`, which returns 0 on
success and non-zero when the directory exists, fits the use exactly, but that reading is
**provisional**. Both routines format into a 512-byte stack buffer through `004b7f70` with no
length check on the base directory.

## BSP_Game_Render, 004ca440

`__thiscall(game)`, no stack arguments, plain `RET`, void. The whole body is inside an SEH frame
whose handler is `00c6557e`; that unwind path is not modelled. Order of the body:

| Site | Call | Gate |
| --- | --- | --- |
| 004ca45b | `if (game+34h != 1) return` | - |
| 004ca46b | `game+34h = 2` | - |
| 004ca472 | `(*00f88c20)->vtbl+0Ch (float game+21F0h)` | - |
| 004ca483 | `(*00f88c20)->vtbl+4h ()` | - |
| 004ca490 | one-shot: `0041e870("GGame::Render")`, `00e18b38 = 0`, release the temp | bit 0 of `00e18b3c` |
| 004ca4e7 | the view branch, below | - |
| 004ca5d9 | screenshot toggle, `004bfbf0` on the rising half | not a front-end state, action 1Dh |
| 004ca64a | `00503510`, ECX `[00e198ac+70h]` | that pointer non-null and its `+5` byte set |
| 004ca658 | `004c12b0` then `00aa45a0`, the GUI draw | - |
| 004ca66e | `0078a430`, ECX `00f871b4` | `00f871b4 != 0` |
| 004ca673 | `004c11f0` then **`00b1ebe0`**, the command queue | - |
| 004ca67f | `004c14c0`, result discarded | - |
| 004ca68d | `004c1dd0` then `00be3bf0`, the profiler overlay | `game+2191h` |
| 004ca699 | second toggle, target `00e18b35` | not a front-end state, action 29h |
| 004ca6ee | `00b7aab0` then `00a8ac00` | `game+19F8h` and a non-null shadow owner |
| 004ca713 | `(*(game+19F0h + A8h))->vtbl+8h ()` | `game+19F0h` |
| 004ca72c | `00b0d190`, ECX `00f8d39c` | - |
| 004ca737 | `004c11f0` then `00b1ef60(game+19FCh)` | - |

The float at `game+21F0h` is loaded with `FLD` at 004ca465, **before** the phase store, and
passed as a stack float with `FSTP [ESP]`. It is the dilated delta `004c6e30` produced
(`docs/GAME_FRAME_CONTROL.md`). The Ghidra pseudocode drops most of these arguments because the
call sites set ECX only; every `this` above is read from the assembly.

`game+19FCh` is the render camera (`docs/GAME_RENDER_TAIL.md` line 132) and `game+19F8h` is the
light whose `+174h` is the shadow-map owner (`00b7aab0`).

### The scene-versus-interface branch, 004ca4e7..004ca5ce

```
if (!(game+1FE4h == 1 && game+2058h && game+5D4h == 0Dh)   // 004ca4e7
    && [00e198c4] != 0                                     // 004ca506
    && game+7184h == 0) {                                  // 004ca514
    switch ([00e08310]) {                                  // 004ca521
    case 1: shadow view; enqueue two jobs; dispatch;  goto after;   // skips 00735b50
    case 2: 0059d7b0([00e198c4]+54h); break;
    case 3: 00535430([00e198c4]+5Ch); break;
    }
}
00735b50();  // 004ca5ce, ECX = [00e1ae90]
after:
```

`00e198c4` is the in-mission interface manager (`docs/GAME_SIMULATION_GATE.md`). `00e08310` is
the **highest active interface level**, latched by `004f7620` over the five vectors at
00e18cfc..00e18d3c. What the levels rank is not established in the ledger, and this packet does
not settle it either; what it adds is that the render entry dispatches on exactly levels 1, 2
and 3, and that level 1 is the level that runs the 3D scene.

`game+1FE4h` is the local-player mode and `game+2058h` a companion byte, so the first
conjunction reads as **split-screen inside the mission state suppresses the interface block**.
That naming is a hypothesis; the loads are not.

Level 1 is the only path that jumps over `00735b50` (004ca5b0). Levels 2 and 3 and every skipped
path fall through to it.

### The frame job pool, 004c1130

`004c1130` is a double-checked lazy singleton over `0109cf08`, 0x138A8 bytes, constructed by
`004bfa40`. Its base subobject at +4 is built by `00be3040` with a thread count of -1, which
`00be4800` resolves. Layout of that base:

| Offset (from pool+4) | Field |
| --- | --- |
| +4h | worker thread handle array |
| +10h | worker object array |
| +14h | worker count |
| +18h | running flag |
| +20h | job count |
| +24h + i*8 | job slot: object pointer, then a one-word argument |

The array is 10000 slots, zeroed by the constructor and **not bounds checked** by the enqueue.

- `00be3020`, vtable +4h: `RET 8`. Stores the pair at `+24h + count*8` and increments the count.
- `00be3150`, vtable +8h: `RET 4`. With a zero argument it calls `worker[i]->vtbl+4h` for each
  worker, drains the jobs on the calling thread through `00be2fa0`, then clears the running flag.
  With a non-zero argument it calls `ResumeThread` on the handles instead.
- `00be2fa0`, the drain: `InterlockedDecrement` on the count, then index the slot array with the
  value that returned, calling `(*(*job))(arg)` with the job in ECX and clearing the slot. **Jobs
  run last in first out.** After the drain it calls `worker[i]->vtbl+8h`.

`BSP_Game_Render` enqueues `004c0960()` then `004c0a30()`, both with a zero argument, then
dispatches with zero. So the camera update runs before the world view.

Both enqueued objects are eight-byte singletons with two vtables. Their constructors write the
primary vptr at +0 and the singleton-lifetime vptr at +4, and the getters return the **primary**
pointer (004c0a19 returns `00e18ad8`), which is what the pool stores and calls.

| Singleton | Global | Primary vftable | Slot 0 | Lifetime vftable | Slot 0 |
| --- | --- | --- | --- | --- | --- |
| 004c0960 | 00e18ad8 | 00ce753c | `004bbd00` | 00ce7538 | `004bd080` -> `004c10b0` deleting dtor |
| 004c0a30 | 00e18adc | 00ce7544 | `004b4820` | 00ce7540 | `004bd0b0` -> `004c10f0` deleting dtor |

`004bd080` and `004bd0b0` are `sub ecx,4; jmp` adjustor thunks; Ghidra has no functions at
either, nor at `004b4820`.

- **Job A, `004bbd00`**: builds a 4x4 identity on the stack, sets ECX to `[00e198c4]`, calls
  `0068a670(manager, &matrix, &out)` and hands the result to `(*00f8bbd8)->vtbl+4h`.
- **Job B, `004b4820`**: `mov ecx,[00e198c4]; call 0068a0d0; ret 4`. `0068a0d0` runs
  `0078cff0(game+19FCh, 00f876a4)` and `00b101c0(game+19ECh, game+19FCh)`, so it is the
  camera-and-global-time update.
- `00735b50` is job A's body with a null guard added on `00e198c4`. Ghidra types it
  `FUN_00735b50(void)`; the ECX the call site loads from `00e1ae90`, the application singleton,
  is not read by the body. Treat that load as vestigial.

So level 1 defers the world view to the job pool and adds the camera job; every other path runs
the world view inline and never runs the camera job.

### The two ortho interface views

Both take the interface manager's subobject in ECX, both `RET`.

- `0059d7b0` (manager+54h) builds a look-at from `sub+54Ch` and `sub+550h` on the ground plane
  through `00b63f10`, sets an orthographic projection from `sub+30h` and `sub+34h` through
  `00aa2020` (`BSP_Camera_SetProjectionMatrix`), computes a world-space extent pair scaled by
  `00d7a280` and `00d7a308`, calls `00b7aab0` and `00a8e860` with it, and tail-jumps to
  `0093d250`.
- `00535430` (manager+5Ch) is gated on `sub+4`, reads a source object through
  `[sub+138h]+14h`, pushes two vectors through virtual +30h on `sub+178h` and `sub+174h`, and
  reaches the same `00aa2020`, `00b63f10` and `0093d250`.

Both are top-down orthographic map views. `0093d250` submits through the command queue
(`004c11f0` then `00b1f4d0`).

### The two debug toggles

Both inline the `004c43c0` rising-edge test against the input singleton's record array at
`instance+4h`, whose stride is 30h (`docs/GAME_FRAME_CONTROL.md`), and both are suppressed on
game states 1, 2 and 4 (`bsp::is_front_end_game_state`).

| Site | Inline offset | Action | Target | Effect |
| --- | --- | --- | --- | --- |
| 004ca5ee | +570h | 1Dh | `00e188ad` | on the rising half only, `004bfbf0` opens a sequence |
| 004ca6ae | +7B0h | 29h | `00e18b35` | none |

The `SETZ`/`CMP AL,CL`/`JZ` sequence at 004ca61e is the MSVC expansion of `b = !b`; the branch
can only be taken when the byte holds neither 0 nor 1, so it is unreachable in practice.
`00e18b35` is read and written **only** inside `BSP_Game_Render` (Ghidra xrefs: 004ca6de read,
004ca6e8 write), so action 29h currently changes nothing observable.

## BSP_Game_FinishRenderFrame, 004ca1f0

`__thiscall(game)`, no stack arguments, plain `RET`, void. SEH handler `00c65530`.

```
slot = [0109db34];
profiler = 004c1dd0();  *(profiler+24h + slot*4) = FFFFFFFFh;   // 004ca20d
004c1dd0(); 00be3640(slot);                                     // open the counter
if (game+34h == 2 && 00b1bf90() == 0) {                         // 004ca236
    if (!00e188ad) renderer->vtbl+14h();                        // EndFrameAndPresent(0)
    else           renderer->vtbl+10h(004c9a00(...));           // EndFrameAndPresent(path)
    game+34h = 0;
}
004c1dd0(); 00be3660(slot);                                     // close the counter
```

The counter is opened and closed **outside** the phase gate, so a frame that never rendered
still pays for it. A retained queue leaves the phase at 2 rather than returning to idle.

## The remaining owned callee: 00503510

`__thiscall`, ECX = `[00e198ac+70h]`, `RET`. The gate is that pointer plus its `+5` byte, the
same "sub-object is active" pattern `docs/GAME_SESSION_POLLS.md` records for the interface
managers. The body is 0xC18 bytes, references `ColourRemap.tga` and `"Rotor %d=0x%x"`, walks
4x4 matrices through `004134f0` and `00413920`, and reaches `00a8f3b0`, the mesh binders at
00b0e9b0..00b0fe10 and `0078cff0`. It is the front-end 3D preview draw (a model with animated
rotors and a colour-remap texture). The interior is not reconstructed.

`00a8f3b0`, `__thiscall(shadowOwner, camera)`, `RET 4`, builds identity matrices, normalises a
direction through `00419510`, forms a view through `00b63f10`, inverts it through `00b63b30`
and refreshes a world matrix through `00b6db70`. It is the shadow-map view update from the
render camera. `00a8ac00`, its end-of-frame counterpart, has a trivial body in this image.

## Calling conventions and RET sizes

Every RET size below is read from the last instruction of the stored body; the `this` pointers
are read from the ECX the call site sets.

| Address | ABI | RET |
| --- | --- | --- |
| 004ca440 | `__thiscall(game)`, void | `RET` |
| 004c6c30 | `__thiscall(game)` -> bool in EAX | `RET` |
| 004ca1f0 | `__thiscall(game)`, void | `RET` |
| 004bfbf0 | `__cdecl(void)`, void | `RET` |
| 004c0960 | `__cdecl(void)` -> `00e18ad8` | `RET` |
| 004c0a30 | `__cdecl(void)` -> `00e18adc` | `RET` |
| 004c1130 | `__cdecl(void)` -> `0109cf08` | `RET` |
| 00503510 | `__thiscall([00e198ac+70h])`, void | `RET` |
| 00535430 | `__thiscall([00e198c4+5Ch])`, void | `RET` |
| 0059d7b0 | `__thiscall([00e198c4+54h])`, void | tail `JMP 0093d250` |
| 00a8f3b0 | `__thiscall(owner, camera)`, void | `RET 4` |
| 00be3020 | `__thiscall(pool+4, job, arg)` | `RET 8` |
| 00be3150 | `__thiscall(pool+4, char)` | `RET 4` |

## Reconstruction

`include/bsp/game_render_frame.hpp` and `src/game_render_frame.cpp`. The branch decisions are
pure functions (`select_view_path_004ca4e7`, `runs_inline_world_view`, `frame_job_drain_order`,
`toggle_debug_flag`, the two path formatters) and the callback order is a sequence routine over
`GameRenderFrameHost`, one virtual per native call site, in the style of
`bsp::run_application_frame`. Reused rather than duplicated: `bsp::is_front_end_game_state`
(`bsp/app_frame.hpp`) for the {1, 2, 4} suppression and `bsp::GameStateId::kInMission`
(`bsp/game_frame_control.hpp`) for the 0Dh test. Render-queue types are referenced by name only;
the submission point is `bsp::execute_render_command_queue_00b1ebe0` in
`bsp/render_command_queue.hpp`. The three routines are the bodies behind host methods other
packets already declare: `BlockingScreenHost::try_begin_render_frame` and `::finish_render_frame`,
`RenderTailHost::game_render`, and `FrontEndStateHost::game_render`.

The one added test case pins the LIFO drain order, because a FIFO reading of the pool would
silently reverse the frame's two jobs.

## Function states

| Address | State |
| --- | --- |
| 004ca440, 004c6c30, 004ca1f0 | reconstructed, build-tested |
| 004bfbf0, 004c9a00 | reconstructed, build-tested |
| 004c0960, 004c0a30, 004c1130 | analysed (singleton getters, not reconstructed) |
| 00be3020, 00be3150, 00be2fa0 | analysed; the drain order is reconstructed |
| 00503510, 00535430, 0059d7b0, 00a8f3b0 | analysed, exported, interiors not reconstructed |
| 004bbd00, 004b4820, 0068a0d0, 00735b50 | analysed to their call graph only |

## Uncertainties

1. **What the interface levels rank.** `00e08310` selects between three render paths; the ledger
   and `docs/GAME_SIMULATION_GATE.md` both stop short of naming the levels. The mapping level 1
   -> 3D scene is established by the branch bodies; the names of levels 2 and 3 are not.
2. **`00bf94c6`** is unidentified. The `_mkdir` reading is provisional.
3. **`00f8bbd8`**, the target of job A's `vtbl+4h`, is not identified. It is written by
   `00a7b230` and read by `004da780`.
4. **`0078a430`** and its owner `00f871b4` are characterised only by their callees
   (`BSP_NativeString_FromUnsigned`, the string pool, `00860030`). "Debug overlay" is a guess.
5. **`00735b50`'s ECX.** The call site loads the application singleton; Ghidra's body does not
   read it. Either the decompiler dropped a use or the load is vestigial.
6. **`game+2058h`** is read only here in this packet's window; "split view companion byte" comes
   from its conjunction with `game+1FE4h == 1`, not from a writer.

## Functions with no Ghidra function

The orchestrator must define these before any name can be applied:

- `004b4820`, job B's primary virtual: `mov ecx,[00e198c4]; call 0068a0d0; ret 4`.
- `004bd080` and `004bd0b0`, the two adjustor thunks (`sub ecx,4; jmp`).
- `00be3020`, the job enqueue, `RET 8`. Ghidra's enclosing candidate is `00be2fa0`.

## Follow-up packets

| Packet | Addresses | Files | Contract |
| --- | --- | --- | --- |
| `frame_job_pool` | 004c1130, 004bfa40, 00be3040, 00be3020, 00be3150, 00be2fa0, 00be2ea0, 00be2c70, 00be4800 | docs/FRAME_JOB_POOL.md, include/bsp/frame_job_pool.hpp, src/frame_job_pool.cpp | The 0x138A8-byte worker pool: worker construction, the LIFO drain, the wake and idle virtuals, and the thread-count probe. |
| `interface_ortho_views` | 0059d7b0, 00535430, 00aa2020, 00b63f10, 0093d250, 00a8e860 | docs/INTERFACE_ORTHO_VIEWS.md, include/bsp/interface_ortho_views.hpp, src/interface_ortho_views.cpp | The two top-down map views and the ortho projection and look-at they share. |
| `screenshot_capture` | 004bfbf0, 004c9a00, 004b7f70, 00bf94c6, and the 00b2d8e0 capture argument | docs/SCREENSHOT_CAPTURE.md, include/bsp/screenshot_capture.hpp, src/screenshot_capture.cpp | Where the +10h path writes the PNG and what the sequence directory probe actually is. |
| `front_end_preview_draw` | 00503510, 00502480, 00b0e9b0, 00b0fd70, 00b0fdc0, 00b0fe10 | docs/FRONT_END_PREVIEW_DRAW.md, include/bsp/front_end_preview.hpp, src/front_end_preview.cpp | The interface model preview: rotor bone animation, the colour-remap texture and the mesh binder calls. |
| `shadow_map_view` | 00a8f3b0, 00a8ac00, 00a8e860, 00b7aab0, 00b63b30 | docs/SHADOW_MAP_VIEW.md, include/bsp/shadow_map_view.hpp, src/shadow_map_view.cpp | Building the shadow-map view and its extents from the render camera. |

## What remains

The three routines in this packet are complete. Their callees' interiors are not: `00503510`,
`00535430`, `0059d7b0` and `00a8f3b0` are exported and characterised but not ported, and the job
pool's worker loop is analysed only as far as the drain. Nothing here is ABI compatible or
game validated; the reconstruction is build-tested only.
