# Actual renderer constructor (R63)

The full B32410 renderer constructor now builds and runs using the actual shared
singleton manager, string pool, VFS/Lua services and D3D9 factory. This advances
the renderer owner needed by application initialization. The application's
milestone device/frame path has not yet been replaced.

## Implementation

The reviewed 1256-byte constructor is recovered from `00a12b5d0`. It preserves
ordered member initialization, actual publications, 29-state parent cleanup,
caller scratch preimages, and native retained failures. Its children are the
real Lua owner, shader-state definitions, system constants and control worker.
Successful children receive no added rollback if a later parent operation fails.

Two historical dependencies were missing from main. Shader-state lifetime now
accepts the same actual manager through `SoundLifetimeAccess`. A separate raw
system-constant source implements the constructor's 106-state cleanup using the
same mutable manager/pool publications. Existing interfaces remain available.
The complete record copy/default bodies are shared by both string interfaces.
Full B5BE10 resize handles growth and shrink; B5BF50 calls it before freeing the
current base. No constructor-only resize-to-zero substitute remains.

| Native entries | Scope |
| --- | --- |
| B32410 | Complete actual renderer construction and member unwind schedule |
| B56610 / B566B0 / B56750 | Shader-state base publication, destruction and scalar deletion |
| B585A0 / B58320 / B59E50 | All 62 shader-state definitions and their lifetime |
| B5B9E0 / B5BA80 | Actual system-registry base publication/destruction |
| B5BBC0 / B38310 / B5BB40 | Complete system-record construction/copy/default initialization |
| B5BD10 / B5BE10 / B5BED0 / B5BF50 | Complete raw reserve/resize/append/array destruction |
| B5BF70 | All 52 system constants, original order and recovered cleanup states |

Names remain descriptive hypotheses. The report records original ABIs,
provenance, state maps, call sites and uncertainty for every entry.

## Validation

- Strict MSVC Win32 `/MD /W4 /WX /fp:strict` build and three existing CTests pass.
- Fresh live Ghidra/installed-PE match covers 286 spans, 21,896 bytes and all
  17 function bodies. The stale B58320 export was refreshed; no listing repair
  was necessary. B5BDAD remains unreferenced alignment skipped by a jump.
- A copied full native B32410 calls copied full B585A0 and B5BF70 constructors.
  The source lane executes the complete C++ implementations. Both use real
  D3D9 on the NVIDIA GeForce RTX 5090, actual VFS-loaded fundamentals, real Lua,
  pooled strings, allocations, registrations, events and the control thread.
- All 52 system-record fields and names agree with the native literal table.
  The full normalized renderer, deep arrays, and all 62 shader-state names and
  values compare equal across 9,591 bytes. Present parameters retain preimages.
- Real control threads are observed alive and joined through their genuine
  destructor. Fixture-owned cleanup invokes real child/base providers, then
  drains the application's shared VFS, pool and manager. It is not the native
  full renderer destructor or its production shutdown binding.
- Seven focused checks pass in three fresh processes. No permanent tests added.

The initial module audit hit a stale Toolhelp DLL address after D3D9 unloading.
The fault address identified the probe's `modules` function. The audit now holds
a same-base module reference while reading memory and resolving its file.
Forty retained modules are verified I386; expired WINMM/nvd3dum snapshot entries
are recorded rather than treated as live captures. This was an audit defect;
no constructor source change was needed for it.

## Remaining work

Bind this actual owner into production application initialization and shutdown,
then complete device/frame execution. The native constructor leaves the control
worker at run0; this probe does not enable it. Active clock/frame providers and
declaration loading remain outside the fixture. Original FH3/SEH, failure
injection, unsuccessful COM-output preimages, full renderer destruction, visual
parity and gameplay are not proved.

See `reports/native_renderer_constructor_r63.json` for the exact evidence and
immutable source/build/probe archives.
