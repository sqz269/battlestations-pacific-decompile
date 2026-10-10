# Particle manager early startup activation

The accepted raw `AF0B10` constructor now runs after input startup and before
locale construction. The old late Phase 8 placeholder is removed. This packet
changes only `src/game_hosts.cpp` and `include/bsp/game_hosts.hpp`, plus this
review and its report; the six accepted owner/deletion files remain unchanged.
The baseline is accepted main `a9db5b460`, including owner worker `7825e4e2a`.

The caller rejects a repeated phase before binding. It requires retained verified
data, prepares the accepted permanent particle domain and borrows its existing
context before allocating raw storage with
`singleton_lifetime_allocate({SingletonAllocationKind::object, 0x34, sizeof(NativeParticleModelManagerStorage)})`.
No manager getter, receiver initialization, publication repair, context reset or
additional destructor is introduced. The process domain's permanent claim still
rejects replacement-host adoption; a fresh host phase or null publication alone
does not establish valid first-bind history.

An allocation exception records `allocation_failed` and rethrows without a free
or constructor call. A nonnull allocation records `constructing` immediately
before `AF0B10`; its return is ignored. Constructor failure records `failed`
before freeing the captured allocation and rethrowing, preserving all callee
publication/registration effects. Success records `handed_over`, or null records
`skipped`, before the first log or locale allocation.

Four shutdown gates precede destructive work in the destructor, process exit,
application shutdown and singleton-manager destruction. Only `constructing` or
`failed` invokes the existing Source policy: best-effort logging/closure, a
stderr fallback if logging throws, then `std::_Exit(1)` without graph, context,
data, host or CRT cleanup. This is ordinary-C++ containment, not Native FH3,
SEH, hardware-fault or original exit equivalence.

## Compiled Source evidence

The Win32 Source phase is one byte at `GameStartupHost this+0x7C2`: unattempted=0,
allocating=1, allocation_failed=2, constructing=3, failed=4, skipped=5,
handed_over=6. The constructor initializes it to zero. These are new Source
layout facts, not recovered Native layout or calling-ABI claims.

All following offsets are relative to their complete COFF code sections:

| Source initializer evidence | Offset |
| --- | --- |
| Retry check before binder/context calls | `1C1D`, calls `1C3F` / `1C4A` |
| Allocation request kind=3, native bytes=34h, host bytes=34h | `1C67` / `1C71` / `1C7B` |
| Allocator result saved in ESI and `[EBP-1088]` | `1C8A` / `1C96` |
| `constructing` store; constructor receives captured ESI and borrowed context | `1CA6`; call `1CB2` |
| `handed_over`/`skipped` store before first construction log | `1CD9`; log `1CDF` |
| Constructor catch: `failed` before captured free | `312C`; free `3133` |
| Allocation catch: `allocation_failed`, then rethrow without free | `31B6`; rethrow `31BD` |

The Source initializer retains this in EDI / `[EBP-1084]` and the borrowed access
at `[EBP-1060]`. All four guard calls are present: destructor `D5`, process exit
`29`, application shutdown `2D`, singleton-manager destruction `2D`, each in its
own section. The complete helper accepts phase 3/4 and reaches `_Exit(1)`.
Complete objects, section/symbol/relocation indices, constructor/destructor,
guard bodies, and both related initializer code sections are retained.

## Provenance and validation

Before inputs come from the actual integrator build under
`battlestations-pacific-decompile-cc12_resume_integrator/build/win32`.
All 3,352 project inputs and four artifacts were independently checked against
`Source3352_frozen.json` and its retained bytes before copying. Compiler logs
identify the complete affected cohort: 32 `game_hosts.hpp` consumers, comprising
two Core objects and 30 App objects; 56 whole compiler logs; 920 affected project
inputs and four separately labeled Lua build headers. Both Core objects match
their whole archive members; all 30 before App objects match frozen Source3352
App pins. The complete archive contains 1,988 COFF members and three metadata
entries. None of the before objects came from an older worker/main-checkout build.

Only the two intended files differ after CRLF normalization. Seven pre-existing
checkout CRLF/LF variants are retained and listed separately; none is described
as byte-identical. All 32 cross-worktree objects differ as whole files. Raw
section comparisons retain exact anonymous-namespace symbol spellings and
checksum differences, so their difference flags are not behavioral-change
claims. Both full object versions and all 64 indices remain available. All 223
declared `.bss` sections with zero raw offsets in each cohort have no file-backed
payload; the COFF header is never substituted for their storage.

One normal Release Win32 build passed its two registered checks:
`reconstructed_math` and `tool_tests`. The initial preflight missed this older
worker's absent `local/seed_reference.hpp`. Root explicitly authorized copying
the existing verified 2,995-byte integrator header, reconfiguring unchanged CMake,
building only the existing native-math target and running
`native_math_differential` once; that separate check passed 1/1. It is not
relabeled as an original 3/3 build. Both exact seed copies and the remedy logs
are retained. The Core archive, game executable and map stayed byte-identical
through that remedy. No test, allocation API or CMake source was edited.

The ZIP-only replay passed with 6,999 retained payloads. A relocated copy is
also replayed. The bundle is 91,592,924 bytes, SHA-256
`d0545ea899baa4b85eb26ddcc91edec3dea1ade4023d9be87f1ace585ec90aff`.
From `local/cc12_particle_manager_startup_activation`, use
`python -B portable_replay.py evidence.zip`; keep `evidence_utils.py` beside it.
It reads only retained ZIP data and does not invoke Git, a compiler or an
executable. The separate post-commit checker is
`python -B check_current_git.py <worker-worktree> evidence.zip`; its full output
is recorded outside the ZIP after committing this packet. It checks all 3,352
committed blobs and current inputs independently of the portable replay.

No new Native query, Ghidra mutation, startup/game run, SDK/OS probe or new test
was performed. Existing mathematical/tool checks do not validate this startup
call's runtime, audio, gameplay, Native calling ABI or fault-delivery behavior.
The accepted caller cleanup remains linked through the earlier primary review.
Detailed receipts: [packet report](../reports/cc12_particle_manager_startup_activation.json).
