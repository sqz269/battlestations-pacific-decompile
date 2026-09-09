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
| 004ca1f0, finish render frame | Require phase 2 and queue helper returning 0; choose renderer +14h or +10h according to global 00e188ad; set phase 0 | 004c9a00 argument construction; +14h now confirmed to pass zero to the same end-frame body |

Renderer +10h is the previously recovered thunk to 00b2d8e0 EndFrameAndPresent.
The +14h path is now confirmed to call that same body with a zero argument. BeginFrame always
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


## Command queue gate and default completion

Renderer +14h is 00b2f4a0: PUSH 0, CALL 00b2d8e0, RET. Both completion slots
therefore use EndFrameAndPresent; the difference is the supplied optional argument.
Neither slot is established as a no-Present path.

00b1bf90 reads queue+20h. It is not an emptiness check: queued count is +18h.
Execution at 00b1ebe0 uses +20h as a retention control. It walks the pointer array
at +14h, retains command context +28h as current context +30h, invokes the command's
virtual slot 0, and releases the current context. With control zero, each command
is destroyed via 00b1ddd0 and freed, and the array count is cleared after execution.
With control nonzero, commands remain queued for reuse. The field's writers and
possible nesting semantics still need tracing.

Ghidra pseudocode incorrectly returns at the free call in the zero-control path.
Raw bytes at 00b1ec78 instead clean the stack and proceed to increment the index,
compare the live count and continue. The complete 187-byte executor range matches
the original binary. See `reports/render_queue_evidence.json`.

The queue singleton getter 004c11f0 allocates 34h bytes via constructor 00b1f280,
using an optional manager critical section and lifetime registration. Game helper
004c6c30 returns success immediately for nonzero game phase. For phase zero it
blocks on nonzero queue control; otherwise it invokes BeginFrame and sets phase 1.
Command object types, their destructor dependencies, queue-control writers and
singleton manager lifecycle remain unported. No C++ or runtime validation was added
in this investigation.


## Queue mode transitions

00b1c460 establishes +20h as a mode: only transition 2 -> 0 calls renderer +11Ch
before storing the new value. Other transitions store it directly. 00b1c4b0
calls +11Ch when old mode is 2, sets zero, then invokes BeginFrame and default
EndFrame. It does not directly empty the command array.

Renderer +11Ch resolves to 00b28a90. It calls 00b33bf0 on worker +1970h when
nonnull, sets optional synchronization off, clears pipeline bindings through
00b26920, and sleeps 100 ms. Worker helper 00b33bf0 clears worker byte +4h and
tail-calls virtual +8h on its interface at +10h. Whether that call signals, waits,
or performs another operation remains unverified. Do not treat it as a proven
thread join or port mode 2 -> 0 as a plain assignment.

Synchronization setter 00b33aa0 now has a C++ semantic port. It updates enabled
and observed-enabled together, preserving nesting and lock state. The existing
D3D9 probe uses it to enable guarded state updates; build, existing CTest targets,
and device/pixel probes pass. This does not validate concurrent mode switching or
worker shutdown. No new test case or target was added. Evidence is retained in
`reports/queue_mode_evidence.json` and `reports/queue_mode_probe.txt`.

Worker virtual +8h is now resolved as an infinite event wait. See
`RENDER_WORKER.md` for the idle acknowledgment versus thread-join distinction.
