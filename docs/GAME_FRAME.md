# Game update and rendering dependency map

The application frame invokes 004e4a40 with the game object in ECX and a floating
delta on the stack. Profiler strings identify it as `GGame::OnMove`. It calls the
close policy before branching through game-state-specific simulation and rendering.
Game state +5D4h and rendering phase +34h are separate fields.

The rendering sequence established so far is:

| Routine | Gate and transition | Remaining dependencies |
| --- | --- | --- |
| 004e4a40, game update | When phase is 0 and 00b1bf90 returns 0, invoke renderer +Ch, then phase = 1 | State-specific simulation, input, command queue |
| 00b2b200, renderer BeginFrame | If renderer active +1998h is 0, set 1; decrement nonzero inhibit; process reset; call device BeginScene only when inhibit is 0 and device is not lost | Full reset processor and optional 00b15090 callback |
| 004ca440, game render | Require phase 1, set phase 2 before invoking render callbacks | Scene, UI, render singleton callbacks, command submission |
| 004ca1f0, finish render frame | Require phase 2 and queue helper returning 0; choose renderer +14h or +10h according to global 00e188ad; set phase 0 | Alternate +14h completion path and 004c9a00 argument construction |

Renderer +10h is the previously recovered thunk to 00b2d8e0 EndFrameAndPresent.
Do not assume the alternate completion path is interchangeable. BeginFrame always
returns 1, including when already active; native HRESULTs are ignored. A future
port must preserve reset processing before BeginScene and the ordering of buffer
rewind, debug rendering, EndScene and Present described in `BUFFER_LIFETIME.md`.

The game render function has the profiler label `GGame::Render`. It initially
invokes singleton 00f88c20 virtual +Ch with game delta +21F0h, then virtual +4.
Further rendering depends on several scene and UI states, and submits command
work through 00b1ebe0. This map does not provide a runnable substitute for those
dependencies.

## Analysis boundary

004e4a40 still has an eight-byte stored Ghidra body. A backup of its original
generated metadata was retained locally before recreating the function. Ordinary
disassembly/recreation did not fix the body; clearing a suspected flow override
reported NONE before and after. No binary bytes or bridge configuration changed.
Disassembling the entry continuation exposed the close call's reference, and the
decompiler traverses the larger update routine, but its stored assembly listing
remains incomplete.

A separate Capstone 5.0.7 x86-32 traversal of original bytes in
004e4a40..004e553F followed direct branches and assumed calls return. It reached
745 instructions, 152 direct/indirect call sites and two returns, with no
unresolved branch targets inside this bounded traversal. Calls themselves remain
unresolved dependencies. The range was checked byte-for-byte against Ghidra.
`reports/game_update_flow.json` records the bounds, hash, call sites and limitations;
the full audited listing stays in ignored exports. This does not repair the stored
Ghidra body or prove semantic completeness.

`reports/game_frame_evidence.json` records assembly and byte parity for the three
rendering routines. Four names and evidence comments were saved to Ghidra. No C++
changed and no new tests were added. Next, trace the alternate finish path and
command queue contract before integrating a complete frame lifecycle.
