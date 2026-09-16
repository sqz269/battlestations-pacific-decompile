# Canonical D3D9 surface pool process owner

`GameNativeSurfacePoolProcess` owns the one source projection of actual 38h
storage `0108DB00` and its existing `D3D9SurfacePool` companion. The companion
borrows `GameNativePhysicalPoolProcess::allocator_list_domain_00e188b4()`; no
second allocator-list head, renderer/device state, COM surface, or texture-owner
service bundle is constructed.

The process object is fully constructed before native startup registers an exit
callback. `initialize_once_00cd7b40` commits to one attempt before binding or
initialization, returns the original CRT registration status on repeat, and
rejects a repeat after a thrown attempt. The accessor rejects pre-start use. A
nonzero registration result retains the initialized native pool, matching the
wrapper's no-rollback behavior. Default C++ destruction is bookkeeping only and
does not repeat native destruction.

## Original evidence and selected order

Saved Ghidra `/battlestationspacific.exe` and the configured executable agree on
the complete 22-byte initializer and 10-byte exit wrapper:

* `CD7B40..CD7B55`: `MOV ECX,0108DB00`; `CALL B3EC60`; `PUSH CE0C90`;
  `CALL BF6FF5`; `POP ECX`; `RET`.
* `CE0C90..CE0C99`: `MOV ECX,0108DB00`; tail `JMP B3E2B0`.

The CRT table contains `CD7B40` at `CE3510`, immediately before texture2D
`CD7B60` at `CE3514`. The selected reconstructed order is `CD7830` at `CE34EC`
(particle model), `CD78B0` at `CE34F4` (particle parameter), `CD7B40` at
`CE3510` (surface), `CD7B60` at `CE3514` (texture2D), `CD7E40` at `CE3564`
(mesh), `CD8250` at `CE35A4` (section), and `CD82D0` at `CE35B0` (hierarchy);
physical `CD9010` registers later. `game_main` now preserves this order. This is
a selected subset, not a claim that every original CRT initializer is wired.

## Validation and limits

`python tools/ghidra_export.py verify-seeds` passed before the strict MSVC Win32
build. The new source compiled with `/MD /W4 /WX /fp:strict`; all three existing
CTests passed.

The ignored `local/surface_pool_process_probe.cpp` is an R28 copy of the R27
genuine lifecycle probe and links the built source library. It verifies surface
pre-start rejection, stable process/pool identity and registration status,
genuine allocation across two slabs, return and trim to zero, then leaves one
surface slab for real CRT cleanup. The copied texture exercise is also retained.
It starts model, parameter, surface, texture, mesh, section, hierarchy and
physical pools in selected order. Interleaved real `atexit` observers verify
shutdown heads in order `physical -> hierarchy -> section -> mesh -> texture ->
surface -> parameter -> model`, ending with an empty shared allocator list while
process bookkeeping remains alive.

These checks execute the reconstructed source lifecycle and real host CRT exit
registry. They do not execute original image code, prove original ABI/FH3/SEH
equivalence, construct surface or texture payloads, create COM or renderer/device
state, launch the game, or validate rendering/gameplay.
