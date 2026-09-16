# Canonical D3D9 texture2D pool process owner

`GameNativeTexturePoolProcess` owns the one source projection of actual 38h
storage `0108DB38` and its existing `D3D9Texture2DPool` companion. The companion
borrows `GameNativePhysicalPoolProcess::allocator_list_domain_00e188b4()`; no
second allocator-list head, renderer/device global, or texture-owner-services
bundle is constructed.

The process object is fully constructed before native startup registers an exit
callback. `initialize_once_00cd7b60` commits to one attempt before binding or
initialization, returns the original CRT registration status on repeat, and
rejects a repeat after a thrown attempt. The pool accessor rejects pre-start
use. A nonzero registration result retains the initialized native pool, matching
the wrapper's no-rollback behavior. Default C++ destruction is bookkeeping only;
it does not repeat native destruction.

## Original evidence and selected order

Saved Ghidra `/battlestationspacific.exe` and the configured executable agree on
the complete 22-byte initializer and 10-byte exit wrapper:

* `CD7B60..CD7B75`: `MOV ECX,0108DB38`; `CALL B3EE80`; `PUSH CE0CA0`;
  `CALL BF6FF5`; `POP ECX`; `RET`.
* `CE0CA0..CE0CA9`: `MOV ECX,0108DB38`; tail `JMP B3E430`.

The CRT table contains `CD7B60` at `CE3514`. The selected reconstructed order is
`CD7830` at `CE34EC` (particle model), `CD78B0` at `CE34F4` (particle parameter),
`CD7B60` at `CE3514` (texture2D), `CD7E40` at `CE3564` (mesh), `CD8250` at
`CE35A4` (section), and `CD82D0` at `CE35B0` (hierarchy); physical `CD9010`
registers later. `game_main` now calls the texture initializer between particle
parameter and mesh startup. This is a selected subset, not a claim that every
original CRT initializer is wired.

## Validation and limits

`python tools/ghidra_export.py verify-seeds` passed before the strict MSVC Win32
build. The build compiled with `/MD /W4 /WX /fp:strict`; all three existing
CTests passed.

The ignored `local/texture_pool_process_probe.cpp` links the built source library.
It verifies pre-start rejection, stable process/pool identity and registration
status, genuine texture allocation across two slabs, return and trim to zero,
then leaves one slab for real CRT cleanup. It starts model, parameter, texture,
mesh, section, hierarchy and physical pools in selected order. Interleaved real
`atexit` observers verify shutdown heads in order `physical -> hierarchy ->
section -> mesh -> texture -> parameter -> model`, ending with an empty shared
allocator list while process bookkeeping is still alive.

These checks execute the reconstructed source lifecycle and real host CRT exit
registry. They do not execute original image code, prove original ABI/FH3/SEH
equivalence, create renderer/device state, instantiate the texture-owner-service
bundle, launch the game, or validate rendering/gameplay.
