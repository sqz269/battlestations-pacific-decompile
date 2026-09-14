# Populated raw VFS and Lua source composition (BC)

This packet adapts the existing ignored
`local/game_native_readonly_data_az_probe.cpp` fixture to use
`NativeVfsOwnerServices` and one actual source `01090AA0` manager publication.
The adapted files are `local/raw_vfs_lua_bc_probe.cpp` and
`local/run_raw_vfs_lua_bc_probe.ps1`; both remain ignored. The bundle receives
references to the fixture's existing publication cells, not shadow cells. Its
string, physical-stream, render-batch, and type-counter contexts all borrow
that same manager. `NativeLuaFundamentalsContext` borrows the same raw manager
and actual string storage. The prior `SingletonLifetimeDomain`, its callback
chain, and semantic-domain shutdown are absent from the adapted fixture.

Before registration, the fixture installs deletion bindings for actual string
pool/gate, physical-stream pool, render-batch pool and lock, type counter, and
Lua fundamentals. It deliberately creates the render-batch pool to exercise
its deletion profile. The manager has six distinct registered owners before
drain; raw `00BD0400` consumes all six while the bundle, Lua context, borrowed
cells, and `GameNativeReadOnlyData` still live. The manager vector is empty and
all six publications clear afterward. Retained-memory object/requested
counters return to zero; type-descriptor guards stay initialized.

The original populated fixture's other source observations remain: the
hash-verified installed executable supplies 196608 exact read-only bytes in
48 non-executable pages; real Win32 HANDLE reads load installed shader files;
Lua executes nested `DoFile` with `MCPP` order; FileStore converts and retains
memory, physical, and adopted streams; cursor positions, reference counts,
file-handle closure and zero retained-memory counters are checked. The
adaptation does not introduce an unresolved-call stub or a new provider.

The fixture **seeds** its VFS manager, physical-provider and FileStore tree
records in local arena storage. It does not exercise construction, search-path
registration, index population or production startup. The fixed read-only
span mapper is source memory support, not an original game process. The
Win32 build and this fixture prove source composition and reached behavior;
they do not prove original binary ABI, native game reachability or a complete
VFS manager/provider tree. See `reports/raw_vfs_lua_composition_bc.json` for
the source and artifact hashes and exact result.
