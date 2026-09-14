# Native VFS data handoff and application lifetime integration (BH/BI)

The raw VFS composition now accepts reservations made in its own suspended
child, reads complete files, and retires through the real application singleton
host. The combined manifested Win32 fixture uses the new process physical pool
and shared type storage, builds all three loose mounts and six search groups,
reads the installed 657-byte `scripts/fundamentals.lua`, and drains ten raw
singleton registrations before destroying the singleton host ahead of VFS.
`reports/native_vfs_handoff_bh_validation.json` pins the exact source, archive,
game objects, probes, native evidence, and installed inputs.

The bootstrap transfers only its own exact 64-KB reservations through a
single-use capability. The mapper still checks the complete supported PE hash
and exposes only selected read-only data pages. Process exit and timeout are
ordered against the mapper acknowledgement. The ordinary constructor continues
to reject occupied bands. This is verified in controlled rebuilt child probes;
the production `bsp_game` entrypoint has not yet adopted the bootstrap.

`GameNativePhysicalPoolProcess` retains the physical pool and common allocator
list through its native CRT cleanup. `GameNativeTypeStorage` supplies stable
descriptor and guard views while borrowing the existing shared counter for
explicit initialization. The combined fixture runs memory type, physical pool,
then physical type initialization in that order. Application admission still
needs to attach these owners and publication cells to the production startup.

`read_all()` retains the captured stream table, handles positive partial reads,
and releases its stream once. Existing capacity reads use the same guard. The
combined missing-member check exposed a call to numeric `00530620` in the open
failure path; the explicit source binding now dispatches that identity to the
already reconstructed one-byte RET callback. Unknown identities remain errors.
See `docs/NATIVE_VFS_OPEN_FAILURE_BH.md` for the native site and observed failure.

The exact byte constructor and append replace the physical separator's generic
C-string approximation. Their complete 122/147-byte native bodies contain
45/51 instructions and ten direct calls; the physical adoption adds one call.
Ghidra now holds reviewed names and ECX/RET4 signatures with prior comments and
bodies preserved. Direct append uses the overlap-safe behavior visible in
`BF7680`; the existing generic resize's overlap limits remain unchanged.

The current `GameVfsHost` still uses its projected manager. Migrating its Lua,
font, frontend, scene and mission consumers, admitting process owners and the
bootstrap in `game_main`, authentic archive payload validation, original FH3
and binary ABI compatibility, and gameplay validation remain open. The original
game executable was read as evidence and was not run or modified. The successful
fixture is a rebuilt source composition, not a game startup or gameplay result.
