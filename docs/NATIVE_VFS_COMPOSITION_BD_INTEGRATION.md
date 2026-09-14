# Constructed native VFS composition

`GameNativeVfsRuntime` retains the real VFS factory, provider, mount, lookup,
stream and deletion contexts over the caller's existing publication cells and
raw singleton manager. It constructs the A0h manager with BEDA60, appends the
physical, FileStore and MPKG factories, mounts the three phase-2 loose paths,
then supports the later MPAK factory/registry/cache/lock tail. Package scanning
and search startup must run at their original positions between these phases.
The composer does not switch `GameVfsHost` or its external consumers yet.

The owner bundle must install its deletion bindings before any type or pool
getter. All publications, constants and contexts remain live through the same
singleton drain. The caller then retires this composer's matching deletion
bindings and VFS publication. Streams opened by `read` are released after both
ordinary reads and read exceptions. Construction cannot be retried after its
first attempt; partial native startup retains the original unwind boundaries.

`GameNativeVfsConstants` verifies the entire supported executable's size and
SHA-256 before extracting the 537-byte MPKG XOR key and one-byte MPAK empty
pattern. Their disk and live bytes were independently checked by the primary.
Both original spans are writable `.data`; these stable source arrays represent
verified initial values. They neither alias live globals nor enlarge the
separate fixed-address read-only mapper. The original key is not embedded in
source or this report.

The actual `+60` search registration module adds BE25E0, BE2600 and BE2310.
It uses 28h intrusive group nodes, pooled strings and nested value lists over
the constructed manager. Groups prepend, values append, equal counted length
and case-insensitive comparison suppress duplicates, and directories normalize
slashes and receive a trailing slash. The valid startup domain excludes the
original unsafe empty-directory indexing. The primary verified all 320 saved
instructions, 31 internal direct calls and the worker's 32 external startup
call sites. No saved-body repair was necessary.

All three established names and their native ABIs/evidence comments are saved
and exported. Ghidra inserts the ECX `this` argument implicitly; an intermediate
annotation duplicated the manager parameter. The final RET8/RET0C audit
corrected the declaration generator and all three signatures. Original and
intermediate values are retained in the annotation receipt; C++ source was
unaffected.

The combined manifested Win32 fixture constructs raw singleton/type/pool owners
and the manager/provider graph through the existing source. It observes three
then four factories and three mounts, verifies an installed 657-byte
`scripts/fundamentals.lua` read against disk, and populates two search groups.
Duplicate suppression, group prepend order and directory normalization pass.
The same manager drains the populated lists and registered owners; pool
publications clear and retained-memory counters return to zero. A second
fixture verifies both constant spans, pointer stability, wrong-size rejection
and same-size corrupt-image rejection. Neither fixture seeds a provider tree.

Repeated process launches also reproduced an unavailable required D10000 band:
an unrelated 36 KB committed private read/write allocation already occupied it.
The mapper correctly rejected the reservation. An earlier fixture let that
exception terminate the process; its top-level handler now reports the error
and the occupied region. The diagnostic source, executable and mixed
success/rejection log are retained separately. Successful graph execution is
conditional on all four required bands being free; reliable reservation during
production process startup remains unresolved. No unrelated allocation was
released or overwritten, and the mapper's rejection behavior is unchanged.

The exact combined source, core archive, executable, build, two CTests and both
fixture artifacts are pinned in `reports/native_vfs_composition_bd_validation.json`.
This proves bounded source composition, not original FH3/CRT identity, native
ABI compatibility, full package/search startup, production-host reachability,
archive member parity or gameplay. The separate +54 extension-prefix tree and
full 00738360 registration sequence await integration; actual package
enumeration and provider methods remain active dependencies.
