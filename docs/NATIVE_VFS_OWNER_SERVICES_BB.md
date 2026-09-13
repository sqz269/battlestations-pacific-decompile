# Raw VFS-adjacent owner services (BB)

`NativeVfsOwnerServices` is a retained source bundle for existing string,
physical-stream, render-batch and type-counter contexts. Its constructor
requires `NativeVfsPublicationCells`: references to the application's
**existing** actual publication cells. It creates no shadow manager, VFS
manager, provider tree, stream factory, or fixed-address global. The caller
supplies the `01090AA0` manager cell, immutable D5E5AC table, and existing
returning-invalid-parameter boundary. Source references and the bundle must
outlive all registered-owner deletion during manager drain.

The bundle constructs `SoundLifetimeAccess` once from the borrowed manager
cell. `ActualNativeStringPoolStorage`, `NativePhysicalFileDateContext`,
`NativeRenderBatchLifetime`, `NativePhysicalStreamOpenContext`, and
`TypeIdCounterLifetime` all retain the same publication and their exact
dependent references. `bind_deletion` installs four existing deletion
bindings: actual string-pool publication/gate, physical-stream pool, render
batch pool/lock lifetime, and type-counter lifetime. It leaves unrelated
bindings untouched. It does not call any singleton getter, allocate an owner,
or modify a publication during construction.

The caller must install these bindings before any of those owner profiles can
register, drain the raw manager while the bundle and all cells live, then
destroy the bundle. The bundle does not perform drain in its destructor. The
VFS manager publication is carried for physical path/date methods; no VFS
manager owner is created or deleted here. `NativeVfsManagerLifetimeContext`
and provider/index population remain separate application bindings.

This module has only a new C++ source ABI and no reconstructed native address
or Ghidra mutation. The reached native getter/deletion bodies are implemented
in their existing modules; this packet verifies source ownership composition
and registered-owner drain only. Neither original game execution nor binary
ABI parity follows from its fixture. The ignored Win32 fixture compiled this
new translation unit explicitly against the integrated core and observed five
distinct raw registrations and five owner deletions, with all five publications
cleared. This packet's four-file lease excludes CMake registration; the new
translation unit is not yet included in `bsp_core` and must be registered by
the integrator before a production source caller uses it. See
`reports/native_vfs_owner_services_bb.json` for exact scope and validation.
