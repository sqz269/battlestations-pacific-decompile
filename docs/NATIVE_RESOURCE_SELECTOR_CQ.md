# Production resource selectors and application type startup

Addresses: 00ccec00, 00ccf740, 00ccf980.

The application now initializes the recovered game-resource selectors and mesh
types in its retained type-storage domain before the VFS stream types. These
use the same counter, root and scene descriptors. Classification consumes real
initialized selector cells; the earlier fixture selector IDs are removed from
this packet's probe.

## Production families

| Initializer | Guard | Descriptor | Name | CRT cell |
| --- | --- | --- | --- | --- |
| CCEC00 | E19A94 | E19A98 | MConvexObject at CFB6C8 | CE2E04 |
| CCF740 | E19B51 | E19B64 | MAux at CFD880 | CE2E44 |
| CCF980 | E19BD4 | E19BE4 | MGeomMesh at CFDBBC | CE2E80 |

Each complete body is 79 bytes with no ordinary arguments and plain RET.
Each descriptor is `[own ID, scene ID, root ID, name address]`. When its guard
is zero, the initializer sets the guard and name, calls B869C0 on the SAME
scene descriptor, captures both parent IDs before either destination store,
calls the shared 6FAC20 counter getter, increments the current counter DWORD
with unsigned wrap and publishes the captured old value. It does not reset
guards or undo earlier writes when a call throws.

All 237 code bytes, 48 instruction owners, six direct call sites, literal names
and CRT cells match saved Ghidra and the installed PE. The three previously
missing function entries are defined and saved. Before-state comments and
documentation are retained; three names, ABI descriptions and source records
are recorded and exports refreshed. Every batch verified project `bsp`,
`C:/Users/sqz269/bsp.gpr`, program `/battlestationspacific.exe`.

The three selectors are scene-derived families distinct from
`cRenderMeshResource`, `cSkinedMeshResource` and `cMatrixIndexedMeshResource`.
With unique IDs from the common counter, render-mesh items match none of the
three selectors in 71BB40. They remain in the primary item array, with no entry
in the borrowed lists at +64, +54 or +44. In particular, the string `MGeomMesh`
does not establish that it is the render-mesh resource family.

## Application wiring

`GameNativeTypeStorage` now owns stable loader-zero guards and descriptor
arrays for scene, mesh variants and selectors. It exposes borrowed views for
future native resource consumers. `initialize_resource_types` validates the
common root domain and executes CCEC00/CCF740/CCF980, then
CD8690/CD86F0/CD87B0 in their verified relative CRT order.

`GameNativeVfsApplication::initialize_core` binds singleton deletion first,
then calls the new resource-type initialization before its existing memory
type, process pool and physical type schedule. This uses the application's
existing `owner_services.types()` counter and common bootstrap. There is no
second registry/counter, implicit reset or file-type ID constant. The same
retained storage survives through the shared singleton drain.

This schedules the recovered subset; it does not execute every native CRT
entry or claim the original process's absolute numeric IDs. Source interfaces
remain Win32 reconstructions rather than binary ABI replacements. No CMake
registration or shared registry file changed.

## Validation

The tracked MSVC Win32 build and both existing CTests pass. The extended local
mesh probe initializes the actual application-owned storage using a cold shared
counter/manager and the existing root/scene routines. It checks selector names,
inheritance, unique IDs, repeated-guard behavior and continuation through
memory/file/physical type publication. It does not supply invented selector
values or substitute predicate callbacks.

All three full mesh parsers execute their nine fields and two complete subsets
with actual D3D9 buffers/layouts on the RTX5090. Each item enters an actual
71B810 game resource and 71BB40 with the real initialized selector cells. The
primary array has one item; all three classification arrays remain empty.
Item and mesh reference counts remain one. Actual profile deletion retires all
57 canonical geometry owners. The counter unregisters/deletes and its manager
drains/frees; all pools, caches, tree/list storage and D3D9 references clear.

A one-frame run of the current `bsp_game.exe` exits zero with explicit private
GFWL overrides and isolated 640x480 windowed settings. It reaches real VFS
mount/scan setup, phase-5 settings, the application loop and shutdown. Its
receipt records the exact executable and private runtime hashes and command.
The installed game is input; no installed DLL or executable was changed.

The default game-directory `xlive.dll` causes a load-time access violation on
both this executable and the frozen CP executable. A child-only debugger
records a write fault in that DLL at RVA319049, targeting 10640F5E beyond the
rebuilt image, with map-resolved stack candidates in `XLiveLibrary` loading.
The existing `--xlive-dll` and `--xlive-dependency` overrides with the already
verified private runtime avoid that fault. This packet changes neither loader
defaults nor the installed DLL. Raw stack candidates are diagnostic evidence,
not an unwound call stack.

## Limits and next work

The successful process is a startup smoke check. Its own trace still labels
frontend GUI/game rendering, loading, sign-in and part of shutdown
`UNIMPLEMENTED`. That is evidence against claiming full game behavior or
gameplay validation. Replace those paths with actual reconstructed owner/domain
compositions as their dependencies become available.

Initializer execution here is source execution plus full listing/PE evidence,
not a new original-machine-code initializer oracle. Partial-publication faults,
aliases and concurrent guard writes are not newly injected. The mesh probe
still uses the documented explicit hot-effect zero-pass preimage and an actual
descriptor named `generic`; cold shader/compiler paths remain unexecuted.
There is no full CRT-order, native ABI/FH3/SEH, complete shutdown, scene-rendering
or gameplay proof.

Future resource consumers must borrow `types().mesh_resource_types()` and
`types().resource_selectors()` from the same retained application. Changes are
published on the agent branch; moving main remains unreviewed.

Reports: [source and byte evidence](../reports/native_resource_selector_cq.json),
[definitions](../reports/native_resource_selector_flow_cq.json),
[annotations](../reports/native_resource_selector_annotations_cq.json),
[integration receipts](../reports/native_resource_selector_integration_cq.json).
Frozen local evidence: `local/native_resource_selector_cq/registered/`.
