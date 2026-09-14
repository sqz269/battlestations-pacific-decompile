# Retained native VFS consumer operations BJ

`GameNativeVfsRuntime` now provides the operations required by real game consumers
through its existing raw manager. It does not construct `VfsMountContext`, copy a
mount tree, or replace original numeric code identities with executable pointers.

| API | Recovered operation and original ABI |
| --- | --- |
| `resolve_existing(std::string&)` | BDF4C0: ECX actual manager, mutable8h name stack, RET4, AL Boolean. Full normalization, ordered candidates and successful-name logging. |
| `direct_resolve(const std::string&, std::string&)` | BDD6E0..BDD847, 360 bytes: ECX manager, input8h/output8h stack, RET8, AL0/1. Normalized copy, D683F4/BDBC70 mount traversal, output assignment only on success. |
| `enumerate(directory, extension, flags)` | BDD990: ECX manager, directory/extension/flags/output-list stack, RET10h. Existing BE1130 dispatcher and actual0Ch intrusive result list. |
| `file_date(path)` | BDD340: ECX manager, output/name stack, RET8, EAX output. Returns the five original DWORDs, including all-zero results. |
| `read_all(path, flags)` | BDF310: ECX manager, name/flags stack, RET8, EAX stream. Complete flags pass unchanged; the existing one-argument API forwards mode2. |

The newly public BDD6E0 entry reuses the complete private `direct_resolution`
helper in `native_vfs_name_resolution.cpp`. That helper and its BDF4C0 caller are
unchanged. The full exported assembly was checked because the decompiler omits
the captured ECX manager. The sequence preserves input/output aliasing and leaves
output untouched on normal false. No second direct-resolution algorithm was added.

Name resolution borrows `mpak_runtime.device_context()` and the existing logging
context. Its device, lookup, provider, manager publication and string domain are
the already bound runtime graph. The date context borrows `owners.physical()` and
the verified readable D683B0 profile. Enumeration uses the existing concrete
physical/FileStore/MPKG/MPAK dispatcher, native sentinel allocation, native pooled
strings, native duplicate filtering and native list destruction before returning
host strings. Original vtable entries remain numeric identities consumed by source
dispatch; unsupported provider targets propagate exceptions.

Each resolution publishes its native caller headers and acquired frame into the
runtime before calling recovered source. Normal true and false complete the frame;
BDF4C0 copies the final mutable name back for either result. Direct resolution
copies back only on true. An interrupted frame remains retained and prevents a
replacement resolution. `has_failed_name_resolution()` and
`name_resolution_failure_site()` expose that state to the outer owner. These
diagnostics cover both public resolution operations. An interrupted runtime and
all its borrowed inputs must remain alive through process exit: the established
acquired-frame destructor terminates on an incomplete/failed frame. This packet
does not invent a successful teardown for that case or turn a missing provider
implementation into false.

Full-file reads retain the captured stream table and the single caller-reference
release. The prior Win32 size limits, partial-read retry and zero-progress/invalid
count checks remain unchanged. Mode32h is not rejected: recovered BF52A0 uses
`flags & 1` for access and `flags & 0x0e` for disposition, which admits its physical
read-existing path. Other provider flag semantics remain in their actual dispatch.

Validation on 2026-09-14 UTC used a copied and extended existing BH fixture in
ignored `local/native_vfs_handoff_bh_probe.cpp`, with its runner in the same folder.
Both changed translation units and the fixture compiled with MSVC Win32,
`/std:c++20 /W4 /WX /O2 /MD /fp:strict`; the manifested probe linked against the
primary harness's existing game objects/core. No permanent tests or CMake changes
were added. `bsp.py ghidra ensure --status` verified the configured BSP target;
analysis used saved BSP exports. No Ghidra mutation or ledger edit was performed.

The actual graph fixture passed these bounded checks:

- Full startup phase2 retains three mounts, six search groups and thirteen aliases.
- `SCRIPTS\\FUNDAMENTALS.LUA` resolves to `scripts/fundamentals.lua`; an uppercase
  missing name returns false with normalized lowercase spelling and no failed frame.
- Direct missing lookup preserves the old output; direct input/output aliasing
  resolves the real fundamentals file.
- Raw `scripts`/`lua` enumeration returns that member; flags100h gives the same
  list as flags0, matching the recovered low-byte visitor capture.
- Existing-file metadata is nonzero and missing-file metadata is five zero words.
- `read_all("shaderfx/terrain/tree.shfx", 0x32)` matches all 1,387 physical bytes;
  candidate `TREE.SHFX` resolves to that installed descriptor.
- Repeated/missing reads, zero retained-memory counters, the real shared singleton
  drain, and singleton-host destruction before runtime destruction all still pass.

The fixture proves normal success/missing-name cleanup on the installed loose-file
graph. It does not exercise an interrupted provider frame, nonzero duplicate count,
archive member reads, the production consumer migration, original binary ABI,
native FH3/SEH equivalence, or gameplay. Full production build/validation and any
Ghidra annotation are integration work. Exact hashes and local evidence paths are
recorded in `reports/game_native_vfs_operations_bj.json`.
