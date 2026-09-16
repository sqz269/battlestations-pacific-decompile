# Canonical mesh and draw-section process pools

`GameNativeResourcePoolProcess` now owns the distinct actual 38h storages for
mesh pool `0108FFF8` and draw-section pool `010901D4`, alongside the existing
hierarchy pool `0109022C`. Each companion uses the application's same `E188B4`
allocator-list domain. Construction binds metadata; native publication occurs
only when its explicit initializer runs.

## Original evidence and startup

The original CRT table selects `CD7830` at `CE34EC`, `CD78B0` at `CE34F4`,
`CD7E40` at `CE3564`, `CD8250` at `CE35A4`, and `CD82D0` at `CE35B0`.
`game_main` preserves this relative order for the selected reconstructed
initializers. This is not a claim that every original CRT initializer is wired.

`CD8250..CD8265` selects `ECX=010901D4`, calls `B85BB0`, pushes `CE0EC0`,
calls CRT `atexit` at `BF6FF5`, pops the argument, and returns its EAX status.
`CE0EC0..CE0EC9` selects the same storage and tail-jumps to `B858F0`.
The initializer had instructions and a table reference but no Ghidra function;
the integrator defined its verified 22-byte body and refreshed both exports.
The existing mesh wrappers `CD7E40`/`CE0E40` are reused.

Both process initializers permit one attempt, preserve the original registration
status, and reject access before the attempt returns. A thrown attempt cannot be
repeated. Failed `atexit` registration retains native initialization, as in the
original wrapper. No replacement exit callback or rollback is installed.
The actual storages, companions and allocator owner remain alive for their real
CRT callbacks. C++ bookkeeping destruction does not repeat native destruction.
All constructed payloads must be destroyed before those pool callbacks.

## Validation

`reports/native_geometry_pool_application_orch4.json` records full live/PE byte
matches for four wrappers (64 bytes) and five table entries (20 bytes), six
exact call/tail sites, artifact hashes, and the strict Win32 build at its source
commit. All three existing CTests passed.

The ignored `local/geometry_pool_process_probe.cpp` links the actual built
source library and creates the six canonical process pools. It verifies native
list links, distinct storage, guarded resource access, repeated initializer
identity, allocation/return, and the shared trim dispatch. Raw slots without
constructed payloads remain allocated for CRT exit. Observers confirm the real
callback order `physical -> hierarchy -> section -> mesh -> parameters -> model`,
with only the current owner removed, remaining slabs intact, and bookkeeping
still alive when each callback runs. The final shared list is empty.

These are compiled-source lifecycle checks. They do not execute original image
code, construct mesh payloads, exercise allocation/registration failures, prove
original ABI/FH3/SEH equivalence, create a game window, or validate rendering.
The texture owner-service bundle still needs its remaining concrete application
contexts before production construction.
