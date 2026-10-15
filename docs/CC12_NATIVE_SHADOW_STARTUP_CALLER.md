# Canonical shadow owner at application startup

The startup host now constructs and retains the existing canonical shadow target
at the original application ordering point. The optional GUI/POST probe borrows
that same completed application rather than creating a second owner. A real
windowed startup and shared drain pass on the combined Source build used for
runtime validation.

The machine-readable receipt is
[`cc12_native_shadow_startup_caller.json`](../reports/cc12_native_shadow_startup_caller.json).
It pins both compiled trees, the original caller bytes, all actual attempts and
the 393-payload local evidence archive. These are bounded Source/runtime claims,
not whole-game completion.

## Original caller and Source ownership

The complete 62 bytes at `0073DCCB..0073DD09` match the installed original PE and
the saved Ghidra program. The enclosing function remains
`0073D410 BSP_Application_Initialize`; no function was split or renamed for this
interior block. Its established name remains descriptive. The original block:

- Reads `F8899D` once, then uses `NEG/SBB/AND 800h/ADD 800h` to select 2048 or 4096.
- Saves the extent in ESI before `BF681B(2Ch)`, captures the returned allocation,
  and sets caller FH3 state `16h`.
- Calls `A8FE30` only for a nonnull allocation, with ECX as receiver and two equal
  stack dimensions. Shader preload at `73DD12` and input at `73DD8E` follow.

`GameStartupHost` retains `GameNativeShadowTargetApplication` before entering its
existing one-shot constructor. That application reads canonical settings `+1Dh`,
uses the renderer's actual singleton/string/texture/terminal context and sole
`F8BBF0` publication, and preserves the guarded null arm. There is no new native
reference, enable call, texture creation or replacement publication.

The host enters this block after window/device startup and the still-unimplemented
online checkpoint, before the pending preload boundary and actual input startup.
This does not implement shader preload, online IPC or the rest of `73D410`.

The GUI probe now requires a reference to the host's completed shadow application.
It validates and borrows that owner's target. Its parent metadata remains alive
through effect/sampler cleanup and the shared native drain. The host then verifies
the canonical shadow publication is null, closes metadata and finally destroys
the renderer's Source metadata. Interrupted construction or consumer graphs use
the existing process-retention exit policy; no guessed cleanup or replay occurs.

## Build and actual execution

Source commit `76befc721ea64de4c8fc95d1677716b7b5712918` passes the canonical MSVC
Win32 build and all three existing checks in 6.91 seconds. After importing the
current Main avoidance/cache reconstruction, combined Source
`19c2bad0eeb0923bd15040a32bd45796395205eb` passes the same checks in 6.78 seconds.
No new test suite was added. Each of those two compiled families freezes 161 artifacts,
including all 139 application objects, linked executable/map and full libraries.

Main subsequently added the separately reviewed avoidance owner/intersection
packet. Final integration Source `c05287f4c2347def7b34a423dc39b5673a2d6249` passes
the canonical build and the same three checks in 6.82 seconds. Its additional
build artifacts are sealed separately. All four shadow-binding Source files
match the runtime-validated tree; runtime proof remains at `19C2BAD0E`, and is
not promoted to execution coverage for that later avoidance code.

The first actual attempt had no private options file and selected fullscreen
defaults. It exited 4 at device creation, before the shadow block. Its log is
preserved. The unchanged basic D3D9 probe had succeeded under the then-current
session state; console-session mismatch alone did not predict device failure.

The next attempt used an exact copy of accepted windowed options in a fresh
private settings directory. Device creation and the actual startup shadow
constructor succeeded. The optional GUI probe logged the **same owner address**.
It then retained the full graph and exited 1 because the cached GuiFont sampler
entry consumed an unknown native `B-16` residue. The entry profile remained
unmatched and POST acquisition was not reached. No evidence gate was loosened.

Separate normal-startup invocations with optional probes disabled passed on both
compiled trees. The runtime-validated combined executable exited 0 with an 800x600 device,
two requested loop iterations, one presented frame, one skipped present and a
completed loop. The shadow constructor ran exactly once before input startup.
The genuine shared drain completed, the canonical publication-null precondition
passed, shadow metadata closed, and final renderer COM counts were device=0 and
API=0. These normal and interrupted paths remain separate runtime evidence.

The installed original executable retains its pinned SHA-256. Ghidra's previous
function name and complete comment text were preserved, the new bounded evidence
was appended under the shared write lock, the project was saved, the affected
export refreshed and the index rebuilt. Earlier failed families remain intact.

## Remaining fidelity boundary

This is an ordinary Source application binding. It does not recover the entire
application register/private-stack interface, caller FH3 cleanup or hardware
exception behavior. The original 4096/null branches are assembly/Source-reviewed,
not established by these nonnull 2048 runtime cases.

The cached material entry's complete module/code/IAT/caller qualification still
needs evidence before claiming a known `B-16` word. GuiFont/GuiFontBilinear and
POST cold/warm completion remain open. Native Text glyph buffers/sections/draw,
world drawing, complete frontend and gameplay validation also remain open; the
normal smoke test still reports 43 unimplemented host methods.
