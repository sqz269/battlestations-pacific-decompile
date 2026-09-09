# Physical provider flags and pending FileStore requests

Implementation follow-up: the physical/FileStore adapter now accepts the two
observed modes explicitly, and the native five-script startup policy passes an
installed-source cache round trip. See [STARTUP_SCRIPT_PRELOAD.md](STARTUP_SCRIPT_PRELOAD.md).
The audit below records the earlier analysis and its then-current interface.

For the inspected **synchronous physical and memory-backed FileStore route**, native flags `0x32` and `2` have the same read-open behavior. The current host physical/cache adapter can therefore serve the five script preloads identified in [VFS_PRELOAD_BOUNDARY.md](VFS_PRELOAD_BOUNDARY.md), within its existing supported read-only domain. This does not establish equivalent flags for MPKG, pending/overlapped requests, native failures or arbitrary flag values.

No universal meaning was recovered for bits `0x10` and `0x20`: neither affects this particular synchronous route. Explicitly recognize the two observed modes at the policy boundary and preserve the original requested flags; do not silently mask arbitrary modes or present the current flags-2 API as a complete native open interface. Native font preload selection remains separate from these five script requests.

This read-only lane verified project `bsp`, program `/battlestationspacific.exe`, language `x86:LE:32:default` and image base `00400000` before every live batch. Six primary routines were inspected. The parent authorized `00bdda10` and `00bdc100` as specific extensions to the physical/pending code ranges. No Ghidra metadata, C++, tests or shared integration files were changed. Evidence and hashes are in [vfs_provider_flags_audit.json](../reports/vfs_provider_flags_audit.json); ignored exports are under `exports/bsp/parallel_vfs_flags/`.

After reviewing this lane, the primary applied the five proposed annotations,
preserved previous comments and the existing physical-stream/thunk names, saved
the project and refreshed all affected standard exports. Four formerly unnamed
functions received descriptive names. The application log is
`local/ghidra-annotations-20260909T211216Z.json`; the read-only audit remains a
record of what the worker did. This integration added no VFS implementation.

## Synchronous physical path

The current path is manager open `00bdf310` -> mount callback `00bda690` -> physical provider `00bf4ba0` -> thunk `00bf5590` -> physical stream open `00bf52a0`.

- Cached, previously byte-audited manager/callback assembly shows the original flags passed to provider virtual `+8` unchanged. Their own mode checks use bit 0. Name normalization, alias selection and mount traversal do not consume the extra `0x30` bits.
- Freshly checked provider table `00d69168` has `+8 = 00bf4ba0`. The complete 199-byte provider body takes **ECX provider; suffix and flags on stack; EAX stream/null; RET 8**. It constructs a physical path, forwards the original flags at `00bf4bde..00bf4bee`, checks stream validity and releases a failed result. The allocator and path-builder calls receive no flags.
- `00bf5590` is exactly **`JMP 00bf52a0`**, five bytes `e90bfdffff`. It neither transforms flags nor adds another wrapper object. Its Ghidra name already follows the target; no new name is proposed.
- The complete 589-byte `00bf52a0` body takes **ECX stream; path and flags on stack; RET 8** at `00bf54ea`. It derives desired access from bit 0, share mode from `~flags & 1`, and creation disposition from `flags & 0xE`. The extra bits are not retained in the stream object or consulted later in the inspected body.

For both observed inputs, mask `0xE` produces index 2. Fresh native jump-table bytes at `00bf54f0` direct index 2 to `00bf530c`, which overwrites the disposition argument with 3 before `CreateFileA`:

| Native flag input | Desired access | Share mode | Creation disposition | Attributes | Security/template |
|---:|---|---|---|---:|---|
| `2` | `0x80000000` / `GENERIC_READ` | `1` / `FILE_SHARE_READ` | `3` / `OPEN_EXISTING` | `0` | null / null |
| `0x32` | `0x80000000` / `GENERIC_READ` | `1` / `FILE_SHARE_READ` | `3` / `OPEN_EXISTING` | `0` | null / null |

The relevant native instructions are `00bf52b8..00bf52e7`, `00bf530c..00bf5346`. Both inputs also follow the same native success/failure branches: they request read access, so the write-only directory-creation retry is not selected. The body caches file size on a valid handle and zeroes the cursor words. This proves agreement between the two **native inputs**, not equivalence between native errors and the host's guarded error handling.

Do not generalize the masking rule to arbitrary invalid mode masks. When `flags & 0xE` is outside the handled cases, the body can retain the original flags word as the disposition argument rather than replacing it with a valid Windows constant.

## What can be reused by the host

`PhysicalFile::open_read_only_00bf52a0_fragment` already issues the same Windows open tuple. Existing `cache_resource_00be7ab0_fragment` buffers the opened physical source, obtains/reset-clones its memory backing and performs first-insertion-wins FileStore population. Cached native `00be5fa0` rejects bit 0 and otherwise does not use `0x10`/`0x20`, so an already-populated memory-backed FileStore also treats `2` and `0x32` alike. Conversion `00bef750` receives the resulting stream, not flags.

A minimal integration can reuse that implementation for exact flags `{2, 0x32}` when every selected provider is one of these supported physical/FileStore adapters. The original five-request startup sequence should still record/request `0x32`; a narrow adapter can document why it uses the existing read-only primitive. Provider lists containing other provider types need their own flag contract. Public APIs currently advertise flags `2` only; accepting the second mode is a deliberate interface/domain change, not something already implemented by this audit.

Existing differences remain: host provider ownership and early buffering, incomplete-read rejection, explicit errors instead of native diagnostic callbacks, and other lifecycle behavior. The flags finding does not require inventing telemetry, an async scheduler or a native font preload list. No preload integration or runtime claim is made here.

## Pending submission is a separate interface

The diagnostic literals identify `00be7cd0` as `cFileStore::RequestFile`. Its actual ABI is **ECX store; name and completion callback on stack; AL boolean; RET 8**. The decompiler omits the callback argument, which assembly references at `00be7e58`. There is **no public flags argument** in this routine.

The complete 672-byte body establishes this sequence:

1. Copy/normalize a local name via `00bee780`, then call `00bdf4c0` to resolve it (`00be7cfe..00be7d1c`). Resolution failure logs and returns AL 0.
2. Query the resident tree at store `+14h` (`00be7d8b..00be7db8`). An existing entry logs and returns AL 1; it does not submit work or invoke the newly supplied callback in this body.
3. Query the pending tree at store `+20h` (`00be7e12..00be7e53`). An existing pending entry likewise returns AL 1 without adding another callback here.
4. Build/insert a name/callback pending record through `00be6120`, `00be62e0`, `00be7460` (`00be7e58..00be7e8b`). This occurs **before** dispatch.
5. Call `00bdda10(local resolved name, original name, 00be7b20, 2)` at `00be7eef`. The last argument is a literal **`PUSH 2`** at `00be7ee2`, independent of synchronous preload flags `0x32`.
6. Dispatch acceptance returns AL 1. Rejection logs, calls cleanup `00be79c0` using the secondary-tree receiver and local name, calls manager helper `00bd9e30(-1)`, then returns AL 0 (`00be7ef4..00be7f6d`). The cleanup/error helper internals were not expanded here; do not claim a complete rollback/lifetime implementation from this call sequence alone.

The prior audit establishes completion `00be78b0` and adapter `00be7b20`: pending removal, stream insertion and callback invocation belong to that completion route. A successful `RequestFile` return therefore cannot be treated as a general assertion that a new entry is already populated. Duplicate-resident, duplicate-pending and accepted-submission cases all return true.

Live `00be7cd0` references are `00509297`, `005092cb`, `0062d1e3`, `00ab5fa8` and `0058c9cd`, in four caller bodies. Those callers were not expanded in this packet; their asset choices and outer triggers remain unverified.

## Dispatcher and visitor boundary

The complete 135-byte `00bdda10` logs `cFileSystem::OpenFileOverlapped entry`. ABI: **ECX manager; four stack arguments; AL visitor acceptance; RET 0x10**. It forwards all four arguments unchanged into constructor `00bdc100`, traverses mounts through `00bdd0a0` using its **second** name argument, reads visitor success byte `+14h`, then destroys the visitor. This second-argument traversal is explicit assembly at `00bdda3f`, `00bdda5c`; do not silently replace it with the resolved first argument while implementing the interface.

The complete 195-byte `00bdc100` takes **ECX visitor; first name, second name, callback, flags on stack; EAX visitor; RET 0x10**. Its field assignments are:

| Visitor offset | Value |
|---|---|
| `+0` | Table `00d68478` |
| `+4/+8` | Copy of first name |
| `+0C/+10` | Copy of second name |
| `+14` byte | Acceptance initialized false |
| `+18` | Flags copied unchanged |
| `+1C` | Callback pointer |

Fresh table bytes give visit `+4 = 00bdc1e0` and acceptance query `+8 = 00bdc1d0`. Those two bodies, destructor/ownership, the provider's overlapped method and callback timing were **not** expanded. They form the next focused async contract boundary. The observed front half carries flags `2` unchanged; it neither explains the general meaning of `0x10`/`0x20` nor proves that the eventual async provider call treats all modes identically.

## Evidence boundary

All six complete primary code spans, the physical disposition jump table, three relevant vtable spans and seven diagnostic strings matched current disk and saved Ghidra bytes after target checks. Raw assembly was used for register/stack inputs and jump-table interpretation. Existing cached exports support the adjacent synchronous manager, callback, FileStore and memory-conversion steps. No native execution, build or fixture tests were performed. Names are descriptive reconstruction interpretations except where an existing symbol or diagnostic text is explicitly identified.
