# Game VFS process lifetime integration boundary (BH)

This source audit follows the published raw phase-2 graph and examines the
remaining executable integration. It claims no new native body or CALL site.
The inspected source revision and exact input hashes are recorded in
`reports/game_vfs_process_lifetime_bh.json`.

`GameSingletonHost` already owns one actual `01090AA0` publication and the
`NativeSingletonDeletionBindings` used by the application's raw drain. Sound,
input, online and observer owners attach retained contexts to that binding.
`GameVfsHost`, however, still constructs a projected `VfsProviderManager` with
four factory-token identities. Migrating VFS must borrow the existing lifetime
publication and deletion binding; a second raw lifetime manager would split
the application.

## Required owner and shutdown changes

The raw graph fixture creates real string/batch/stream/type-counter services
and physical-provider pool storage, but its local stack declarations are a
fixture lifetime. They cannot be copied into a short-lived production VFS
wrapper without examining the original process ownership.

- `NativeVfsPublicationCells` deliberately borrows stable application cells.
  It does not supply those cells or initialize a private publication domain.
  Production needs retained string-pool, batch-pool/lock, stream-pool and
  type-counter storage attached to the existing application lifetime.
- `NativeStreamTypeIds` requires the same counter and root guard/descriptor
  used by shared types. Its storage includes actual file, memory and physical
  guards/descriptors. A VFS-private copy would cease to be the application's
  shared type state when another consumer begins using the native types.
- The physical-provider pool has a **separate process/CRT lifetime**. Its
  existing source contract binds one `0109DBF0` owner and the same `E188B4`
  allocator domain, initializes it through `CD9010`, registers `CE10F0`, and
  retains both until real CRT exit. The fixture directly calls `BF3250` and
  `BF33A0`; that proves operational pool behavior, not process registration.
  No call to those static initialization bindings or `NativeStreamTypeIds`
  initialization was found in the inspected `src/game*.cpp` files. This is a
  bounded source search, not a linker-wide absence claim.
- The current application destructor calls `singletons_->shutdown()` while
  VFS, script and settings owners remain alive, then deletes `singletons_`
  **before** deleting `vfs_`. The normal `008F8449` callback also drains through
  `GameSingletonHost`. A native VFS retirement hook must run immediately after
  that shared drain while the deletion table/publication references remain
  valid. Calling `retire_after_shared_drain()` only from the later VFS
  destructor would access a destroyed singleton host.
- Registration and partial startup must leave all bound VFS contexts alive
  until the same drain. `GameNativeVfsRuntime` explicitly disallows retrying
  core construction and instructs callers to retain it if startup throws.
  Default `unique_ptr` unwinding during a registering constructor does not
  satisfy that requirement. Separating retention from initialization is
  necessary for both the normal path and failed startup.

## Coordinated migration sequence

First attach the process-lived pool/type/owner storage to a stable application
lifetime and provide the reviewed deletion-binding attachment and post-drain
retirement hook. Keep the CRT pool and allocator domain alive until their
actual exit callback. The reservation bootstrap packet supplies the earlier
numeric-data ownership handoff; data and constants must remain readable for
all raw consumers through the relevant drain.

Then replace `GameVfsHost`'s projected manager/factory tokens and phase-2
callbacks with the actual retained graph. Move every `manager()->context()`
consumer together: script/settings setup in `game_hosts.cpp`, locale in
`game_hosts_lua.cpp`, and the font, frontend, init-tail, mission and scene-content
readers. The BH full-file read adapter supplies owned bytes with actual stream
length/read/release semantics; it does not itself migrate these consumers.
Locale/Lua APIs that require `VfsMountContext` need concrete raw services,
not a cast from the A0h native manager. Phase-6 resource-manager/parser
ownership remains a separate dependency.

Acceptance must exercise the real executable startup and both normal and
exceptional shutdown, including the single raw drain before referenced
services die and the later process-pool exit. Existing phase-2 probes and
static source agreement do not prove those paths or gameplay.
