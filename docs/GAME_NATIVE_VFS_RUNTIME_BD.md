# Retained raw VFS composer (BD)

`GameNativeVfsRuntime` is a source composition over one actual A0h VFS owner.
Its caller supplies the raw owner allocation, the existing `01090AA0`
singleton publication/domain, `NativeVfsOwnerServices`, an initialized actual
physical-provider pool, type descriptors, retained-memory counters, returning
invalid-parameter callback, and the supported executable's verified
`GameNativeReadOnlyData`. It owns stable VFS-specific publication cells and
retains canonicalizer, lookup/open/logging, factory, registry, MPKG and MPAK
contexts. It never constructs a projected `VfsProviderManager` or aliases the
owner bundle's `0109CEEC` publication.

The MPKG key at `00E144F0` and MPAK null pattern at `00E17BF0` are outside
the `.rdata` map. The caller must supply both from separately verified,
retained source storage. Null is rejected before registration. The current
external-data evidence identifies these as initial snapshots of writable
`.data`, not proof that every possible live mutation is mirrored.

Call `construct_and_register_core()` once. It binds deletion contexts before
any getter, constructs the raw manager through `00BEDA60` (which appends the
physical factory), publishes the same `0109CEEC` cell, installs the two
established startup callbacks, then appends FileStore and MPKG factories.
`mount_phase2_loose_paths(root)` makes the three verified mounts in order:
root -> `.`, root -> `persistent_data`, and `filestore` -> `.`. The generic
`mount` takes the exact native priority/flags/device ID and creates/releases
two actual pooled string headers. `exists` and `read` likewise use pooled
headers; `read` decrements the returned stream's reference and dispatches
zero-reference deletion when it reaches zero, including after a read exception.
It returns the actual read count rather than silently padding a short read.

Call `register_archive_factory_tail(cached_load)` only after the later
package-scan/search phase. It appends MPAK, publishes the actual registry and
cached-load fields, and creates the source-compatible MPAK lock. Package scans,
search-path registration and the BE1480 prefix tree are separate source work;
direct loose-file reads do not establish full phase-2 startup. The factory
registration order among retained owners is physical, FileStore, MPKG, MPAK.

The caller must keep this composer, verified table image, raw owner services,
archive bytes and all borrowed publications alive through the one shared
`01090AA0` singleton drain. Startup failure after a getter may leave a
registered partial owner; keep the composer alive and drain the same domain
before destruction. The caller owns the A0h allocation until its registered
deletion. After the shared drain, call `retire_after_shared_drain()` to clear
this bundle's matching deletion bindings and `0109CEEC` publication before
destroying the composer. Original FH3/SEH, game startup reachability, package scanning and
search groups are not claimed by this source API.

One ignored manifested Win32 fixture used the installed executable and the
verified writable-data snapshot provider. It constructed the raw singleton,
type descriptors and physical-provider pool through existing source bodies;
there was no seeded provider tree. It observed factory counts 3 then 4, mount
count 3, `exists` true, and a 657-byte `scripts/fundamentals.lua` read exactly
matching the installed file. The same singleton manager drained the VFS,
streams, strings, batch and type-counter owners; retained-memory counters
returned to zero. This is fixture-tested source composition only. No archive
member, package scan, game process or production host was exercised.
