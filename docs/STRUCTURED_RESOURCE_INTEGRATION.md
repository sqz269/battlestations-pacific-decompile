# Installed structured-resource metadata traversal

Addresses: 00bf0430, 00bea700, 00bea680, 00be9ed0, 00be9c40, 00be9df0,
00718f50, 00b93310, 00b7eb90, 00b936e0, 00b932e0, 00b7d160, 00b80720,
00b7f430, 00b88430, 00b80f10, 00501670, 005051a0, 00509fd0, 00b947a0.

The Win32 diagnostic can now traverse native MMOD records through the existing
mounted VFS and decode hierarchy metadata and Note text from installed files.
This advances the reader below the audited resource-manager dispatch. It does
not yet instantiate a complete mesh, resource manager, or running game.

## Implemented behavior

`structured_reader.cpp` retains the same stream and cursor, tracks declared
record sizes and child budgets, and distinguishes release from skip. Releasing
a child charges its declared payload to its parent without seeking over unread
bytes. Skip advances by the unread payload. Stable node addresses preserve the
borrowed reader/parent relationships. Counted strings preserve embedded NUL
bytes; the Note payload applies the native C-string prefix assignment.

`structured_hierarchy.cpp` decodes Parent, repeated Resource indices, Matrix,
Name, Flags, BoundingSphere and BoundingBox. Missing Matrix remains explicitly
absent because the audited native record leaves that storage uninitialized.
Sphere and Box share the resulting box fields, so the last relevant field wins.
The sphere-to-box calculation uses explicit x87 and float temporaries. Native
arithmetic rounds maxima before minima; the resulting box stores minima XYZ
followed by maxima XYZ. Exceptional x87 state and NaN behavior are unverified.

These are new C++ interfaces restricted to successful full reads. The host
rejects invalid budgets, incomplete backing data, unsupported depth and invalid
node lifetime use. Native partial reads can preserve stack-derived values and
unchecked accounting; that behavior is not replaced with a claim of equivalence.
Host allocation, containers and error returns are distinct from the original
object layouts, intrusive ownership and pool allocation.

## Installed-file checks

The existing executable accepts:

```powershell
./build/win32/Release/bsp_d3d9_probe.exe --structured-resource 'I:/SteamLibrary/steamapps/common/Battlestations Pacific'
```

This mode performs the following checks without requiring a graphics device:

| Installed file | Verified result | Explicitly skipped |
| --- | --- | --- |
| `models/clouds/cloud_10.mmod` | Control 12; four root records; exact six bounding-box float words; cursor/size 265; retained source released after reader destruction | CloudSystem payload |
| `models/misc/repulogepdarabok_004.mmod` | Control 14; Note `visp100-visp1.5`; one hierarchy Item; parent `FFFFFFFF`; indices 0,1,2; exact matrix including negative zero; explicit box overrides sphere; cursor/size 1303 | None; Mesh and GroupParams now decoded (see `MESH_RESOURCE_INTEGRATION.md`) |

One focused lifetime case releases a BoundingSphere child after one float: the
cursor stays at 42 despite 12 unread bytes, while the parent is charged the full
16-byte payload. No new test target or framework was added. The Win32 build and
both existing math CTests pass; the separate installed reader diagnostic passes.

The final full graphics diagnostic also passes, including both installed
metadata checks and the existing font cache, material binding and draw checks.
Font draws produce 74 and 900 visible pixels, none outside the expected bounds,
and restore render state. Earlier attempts reported zero D3D9 adapters and
`0x8876086c` (`D3DERR_INVALIDCALL`); the cause of that transient environment
failure was not established. No graphics workaround was introduced. This
diagnostic result does not constitute a running game or gameplay validation.

## Audited dependencies

Ten matched native audit packets establish the reader, resource-item ownership,
classification arrays, nonowning resource-name cache, queue work records and
synchronization, base/application/game parser registration, Note parser,
hierarchy primitives and Mesh parser entry. The resource-name tree stores raw
resource pointers without AddRef/release. The base resource releases primary
items; classification lists borrow their pointers. These contracts must guide
the eventual manager implementation rather than assuming every container owns.

The Mesh entry reveals a prefix DWORD and nine field handlers. These payload
readers and the declaration-name decoder are now implemented; native wrapper,
destructor and allocator contracts are audited. Current validation and remaining
material/GPU binding work are recorded in `MESH_RESOURCE_INTEGRATION.md`. Queue teardown does not establish a worker stop/join contract.
No synthetic completion, empty resource object or fake mesh parser was added.

Reviewed functions are named and commented in the existing `bsp` project with
prior comments preserved. Incorrect CRT-free no-return propagation was repaired
at bounded call sites, and seven truncated function definitions were recovered.
Fifteen missing function definitions were added from matched code. Final saved
readback, refreshed exports, source hashes and packet states are recorded in
`reports/structured_resource_validation.json` and its linked evidence reports.

## Registered parser and material follow-up

The selected model's Resource children now dispatch through the recovered
nonowning parser registry using actual Mesh, Note and GroupParams implementations.
The Mesh feeds a checked installed-material D3D9 draw with native compiler flags,
decode constants, building instance values and textures. Native root resource
object ownership, automatic scene/shadow updates and full game execution remain
separate. See `MESH_MATERIAL_INTEGRATION.md` and the mesh_material reports.
