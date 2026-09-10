# Concrete Windows platform and message loop

## Resolved ownership and vtables

The singleton `0109cf04` is installed by `00be2960` and cleared by `00be2a00` under the
singleton manager's lock. Constructor chain:

`00becda0` (Windows platform) → `00be2ac0` (window configuration base) → `00be2960` (singleton base).

The final vtable is `00d68cc4`. Verified slots:

| Offset | Target | Observed role |
| --- | --- | --- |
| `04` | `00becee0` | Create/configure window and initialize rendering device |
| `08` | `00bebf70` | Restore power scheme, disable frames, post quit |
| `0c` | `00bebfa0` | Apply fullscreen/windowed mode |
| `20` | `00bece70` | Application frame, then cursor/focus update |
| `24` | `00bec1a0` | Main message/frame loop |
| `28` | `00bed3b0` | Instance window-message handler |

Seven table targets lacked function definitions in the saved project: `00becee0`, `00bec140`,
`00bec150`, `00bec180`, `00bec190`, `00bed3b0`, and `00beca30`. Created them through Ghidra
using the observed executable addresses and disassembly. The four tiny entries at 140/150/180/190
have no-op/constant behavior but their semantic roles remain unnamed. No game binaries changed.

The application vtable `00cfeab0` slot +10h points to another previously undefined function,
`00737a50`, now recovered and named `BSP_Application_RunFrame`. Its object is passed through the
Windows platform's field at +48h. The total inventory grew by eight functions to 62,080 internal
and 62,522 including externals. Ghidra now has 36 project-specific descriptive names.

## Ported loop

`platform_run_loop_00bec1a0` preserves these assembly observations:

1. A do-while loop executes at least one iteration even if exit was already requested.
2. Each iteration calls `PeekMessageA(NULL, 0, 0, PM_REMOVE)` once.
3. A message goes through `XLivePreTranslateMessage`; only an unconsumed message reaches
   `TranslateMessage` followed by `DispatchMessageA`.
4. An empty queue invokes frame slot +20h only if byte +42h is nonzero.
5. Exit byte +181h is checked after message/frame processing; on exit byte +43h becomes one.

There is no explicit WM_QUIT branch, queue-draining inner loop, sleep, or idle wait. The C++ port
does not introduce those changes. `PlatformLoopState` projects the three flags into explicit
fields; it is not the original Windows-platform object layout. Required callback interfaces
represent the still-unported XLive pretranslation and application frame boundary. They have no
default implementations. Probe callbacks exercise the integration contract and are not game stubs.

The original frame wrapper `00bece70` calls application virtual +10h and then `00becb20(false)`.
That latter method manages cursor/focus transitions and invokes other input/UI helpers; it is
not a renderer-present function. `00737a50` updates time and other systems, then copies the
application-wide exit request at `00e1ae75` to platform byte +181h. It is named but not ported.

## Window lifecycle findings, still unported

`00becee0` registers a CS_BYTEALIGNWINDOW (0x2000, stored at 00becf36) class with 24 extra window bytes, thunk `00bec3b0`, and an
arrow cursor. It creates a captioned window, then applies captioned or topmost-popup styles,
records dimensions and aspect data, shows the window, and calls the renderer singleton at
`00f8d394` virtual +4. It subsequently initializes timing and modifies power/screensaver settings.
The probe does not execute this function or those settings changes.

`00bed3b0` stores the object in window-extra offset zero during WM_CREATE. It handles activation,
resize, input, and sizing-loop frame suppression; WM_CLOSE sets close-request byte +180h and
returns zero. That byte is distinct from loop exit +181h. The path from +180h to global
`00e1ae75` is now established in `WINDOW_CLOSE.md`, including confirmation UI
and a request-suppression gate. Posting WM_QUIT alone cannot be assumed to end the ported loop.

Window creation and frame callbacks have many renderer/audio/input/UI dependencies. The current
library does not invent those dependencies to make a fake game executable link.

## Validation

Win32 Release build and both existing CTest cases pass. Added no automated test cases.
`bsp_platform_probe.exe` runs the actual Windows queue: the first empty-queue frame posts a
private thread message; the pretranslation callback consumes it and sets exit. Output reports
one frame, one consumed message, and loop-finished=true. This is a controlled probe, not proof
of XLive integration, visible rendering, gameplay, or full window shutdown.

Next: integrate the close-request policy in `WINDOW_CLOSE.md`, recover its UI
dependencies and finish native window/renderer initialization. Keep the
existing game and its saved analysis intact except for recorded, evidence-backed Ghidra edits.
