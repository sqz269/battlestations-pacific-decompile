# Startup script cache population

Addresses: `0073d410`, `00be7ab0`, `00bdf310`, `00be5fa0`

The five script requests in application initializer `0073d410` now run through
the reconstructed mounted-stream/FileStore path. `resource_preload.hpp/.cpp`
preserves their exact names, order, call-site addresses and flags `0x32`.
This fragment runs after the caller supplies mounts and a FileStore; it does
not reconstruct the rest of application initialization or execute the scripts.

The installed-source probe completes all five requests, opening five physical
streams with flags `0x32`. It then reopens all five from the same FileStore with
the same flags and compares their full contents with independent disk reads:
140,625 bytes agree. The subsequent font path uses that mounted context and
adds its three explicitly selected diagnostic resources. Font loading still
records three cache opens and zero physical opens, with eight total cache
entries; the existing A draw retains 74 lit pixels and restored device state.

## Native policy and ABI

| Order | Call instruction | Resource | Flags |
|---:|---|---|---:|
| 1 | `0073e239` | `scripts/datatables/inputs.lua` | `0x32` |
| 2 | `0073e2b7` | `scripts/datatables/keyboardsetup.lua` | `0x32` |
| 3 | `0073e335` | `scripts/datatables/controllerinputnames.lua` | `0x32` |
| 4 | `0073e3b3` | `scripts/datatables/controlpresets.lua` | `0x32` |
| 5 | `0073e431` | `scripts/datatables/scoring.lua` | `0x32` |

These are a fragment of the ECX application initializer, not five standalone
native functions. Each native call obtains the FileStore factory/store and calls
`00be7ab0`: ECX store, name and flags on the stack, RET 8. The routine opens the
supplied name before duplicate detection, obtains a memory view, inserts under
the supplied key, and releases temporaries. The application caller does not
check a semantic success return. The host accepts explicit store/mount references
and returns a guarded status; it has no native application or intrusive ABI.

Caller/ABI evidence is in [VFS_PRELOAD_BOUNDARY.md](VFS_PRELOAD_BOUNDARY.md).
Physical and FileStore flag evidence is in [VFS_PROVIDER_FLAGS.md](VFS_PROVIDER_FLAGS.md).
The separate menu audio sequence remains unimplemented and is not added to this
startup batch. No native font preload choice is inferred from these scripts.

## Flags and provider behavior

`open_resource_memory_00bdf310_fragment` accepts the exact observed modes `2`
and `0x32`; its default remains `2`. It rejects other modes before traversal.
`VfsMount::open_read_only` now receives the original flags as an explicit
argument. Normalization, the single alias replacement, mount ordering and
stop-after-first-underlying-open behavior remain the recovered contract.
Each provider callback must establish support for the passed mode.

The physical adapter explicitly accepts those two values. Native
`00bf4ba0 -> 00bf5590 -> 00bf52a0` sends both to identical `CreateFileA`
arguments: GENERIC_READ, FILE_SHARE_READ, OPEN_EXISTING and zero attributes.
It therefore uses the existing read-only primitive for either observed mode.
The FileStore adapter passes the original flags to `open_00be5fa0`, whose
inspected memory-backed path rejects bit 0 and ignores the extra `0x30` bits.
This is not a general flag mask or a claim about MPKG/overlapped providers.

`cache_resource_00be7ab0_fragment` now requires explicit flags and forwards
them to mounted opening. The startup fragment never invokes candidate search
or substitutes resolved names. The FileStore key remains the originally
supplied name even if manager opening uses an alias. Existing first-insertion
ownership and early-buffering boundaries remain in
[MOUNTED_RESOURCE_STREAMS.md](MOUNTED_RESOURCE_STREAMS.md).

## Failure and validation boundaries

The batch stops on the first unsupported/unavailable/incomplete source, returns
the existing error, and reports the number of successful requests so far.
Earlier cache insertions remain. Successful duplicate requests count as completed;
the population primitive still opens before checking for duplicates. This is an
explicit host failure policy, not emulation of native diagnostic/null-source
behavior. Memory allocation exceptions propagate, as in the existing adapters.

MSVC Win32 compilation, both existing CTests and the full D3D9 probe pass.
One installed-source scenario was added to the existing probe; no new test
target/framework or broad failure suite was added. Evidence is recorded in
`reports/startup_script_preload_validation.json` and
`reports/startup_script_preload_probe.txt`. Runtime checks establish the exercised
physical-to-cache path, not script execution, original startup timing, native
errors, async completion, game ABI or gameplay equivalence.
