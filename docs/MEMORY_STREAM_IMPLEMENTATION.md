# Typed memory stream implementation

`include/bsp/memory_stream.hpp` and `src/memory_stream.cpp` reconstruct shared
backing, independent cursors, the read/seek/getter behavior, and the normal
positive-size physical-file conversion used by native texture loading.
They use the original bodies documented in
[MEMORY_STREAM_HANDOFF.md](MEMORY_STREAM_HANDOFF.md), including the corrected
backing destructor range. The saved `bsp` project and
`/battlestationspacific.exe` were verified again before implementation evidence
reads. This task introduced no tests or Ghidra changes; integration/build remain
with the primary agent.

## Ownership and representation

`MemoryStream` holds a `shared_ptr` to a backing object and its own DWORD cursor
offset. The backing owns a `new uint8_t[count]` allocation **without value
initialization**, its requested DWORD length, and a host-only initialized-prefix
length. Bytes are not zero-filled. This reflects the native separately allocated
data and length at backing `+8/+0Ch`; it does not use a resized vector whose
zero filling would change short-read behavior.

Native `00bef6d0` (ECX backing, EAX new wrapper, RET) retains an intrusive
backing pointer, sets a cursor to the backing base, and stores end=base+length.
`clone_reset_00bef6d0` creates a new typed wrapper sharing that backing with
cursor zero. It leaves the source cursor unchanged. Sharing wrapper identity
requires an outer `shared_ptr<MemoryStream>`: retaining a logical texture's
source must keep that exact wrapper, rather than implicitly cloning it. A
separate clone is appropriate for the native conversion's memory-stream branch.

The typed class is movable but not copyable, so cursor duplication is explicit.
The last backing owner releases the bytes. Native destructor ordering
(`00bef9c0 -> backing final release -> 008d4470`) is represented through C++
ownership, but original vtables, layout, Interlocked counts, allocation/free
instrumentation, and globals `0109db98/0109db9c` are not reproduced. No fake
native counters or allocator stubs are introduced. The typed backing does not
outlive its last wrapper merely because a caller saved its raw data pointer.

## Read, seek and data contracts

The supported extent is 1 through INT32_MAX bytes. With this precondition,
native pointer-difference getters can be represented as signed DWORD offsets
without dependence on an actual 32-bit allocation address.

- `size_00bef600` and `position_00bef580` return signed-32-bit offsets widened
  to int64, matching SUB/CDQ for supported states. Native ABI is ECX wrapper,
  EDX:EAX result, plain RET. A missing typed backing returns zero as a host
  empty-state convenience; native getters dereference their input.
- `data_00bef610` returns backing base independent of the cursor. Native ABI
  is ECX wrapper, EAX pointer, plain RET. The typed missing-backing result is
  null. The pointer is not proof that all reported bytes are initialized.
- `seek_00bef540` ignores the offset's high DWORD and performs unsigned DWORD
  addition using start/current/end for origins 0/1/everything else. It preserves
  two's-complement low-bit negative offsets and modulo-32-bit arithmetic. It
  rejects a resulting offset outside [0,length] without changing the cursor.
  That rejection is an explicit host boundary: native ECX/three stack DWORDs,
  RET 0Ch, stores the unchecked pointer and has no BOOL success contract.
- `read_00bef590` clamps requested count with unsigned remaining length,
  specializes two/four-byte transfers using a local word load then store, and
  uses memcpy for other positive counts. It advances by the clamped count and
  writes the optional actual count. Native ABI is ECX wrapper, stack destination,
  requested DWORD, optional count pointer, RET 0Ch; EAX is not a count contract.
  The typed bool explicitly rejects missing backing, null positive-count
  destination or reading beyond the initialized prefix, leaving destination,
  cursor and actual output unchanged. Native has no such safety checks.

Zero-byte reads perform no memcpy and report zero, including at EOF. Native
calls memcpy with zero for that case; the typed no-op avoids invalid-pointer
questions without filling or consuming anything. Nonzero general memcpy ranges
must not overlap the source; no memmove semantics are claimed. The optional
count pointer must refer to separate caller output storage, not backing bytes.

## Physical source conversion

`memory_stream_from_physical_00bef750_fragment(PhysicalFile&, MemoryStream&,
DWORD&)` implements the physical-stream branch only. It verifies a valid host
handle, seeks to zero, obtains cached physical size, and accepts only the
positive signed-DWORD domain. It allocates uninitialized backing, then calls
the physical reader exactly once for the requested length. The output wrapper
has cursor zero. Source handle ownership remains with the caller, matching
the fact that `00bef750` itself does not release its input; texture loader
`00b2c2d0` releases that original stream separately after conversion.

The function preserves requested backing length independently of actual read
count. **A successful short read returns true** and creates a backing whose
tail remains uninitialized. It neither retries nor shrinks the backing, and
does not substitute zero bytes. The host records actual count in
`initialized_size()` so downstream code can establish whether it is safe to
inspect a range. `fully_initialized()` compares that prefix against requested
length and is false for a missing backing. These predicates are host metadata,
not recovered native fields or a native short-read rejection.

Consumers such as D3DX that inspect the full extent must check
`fully_initialized()` first and report unsupported incomplete input explicitly.
They must not pass uninitialized tails to a decoder and then treat arbitrary
results as behavioral evidence. The native conversion and texture caller lack
that guard, as documented in
[TEXTURE_STREAM_LIFETIME.md](TEXTURE_STREAM_LIFETIME.md).

A physical read failure also publishes the allocated output and actual-prefix
metadata, while returning false and its Win32 error. The native error callback
is still outside the host interface. Failures before the read leave the previous
output wrapper untouched, although the initial seek may already have moved
the input's position. Invalid handle reports ERROR_INVALID_HANDLE; zero length
reports ERROR_INVALID_DATA; length above INT32_MAX reports ERROR_FILE_TOO_LARGE;
allocation failure reports ERROR_NOT_ENOUGH_MEMORY. These explicit host errors
do not model native allocation faults, empty backing allocation or dangerous
high-DWORD truncation paths.

## Integration and evidence boundary

The existing DDS probe now converts an opened `PhysicalFile` into a
`shared_ptr<MemoryStream>`-owned wrapper and closes the file. It verifies a
four-byte read, a cloned cursor reset, ignored high offset DWORD and a two-byte
read. The original wrapper cursor remains independent. It retains that same
wrapper in `D3D9RetainedTexture2D`, creates a texture, releases the local stream
and clone, drops the first COM texture, then recreates using only the retained
source. The complete DXT1 payload matches the installed asset after recreation,
and the existing atlas checks pass. Final source release expires a weak wrapper
reference. These checks extend the existing probe; no new test target was added.

The typed texture owner projects assignment `00b23640` and source-before-COM
cleanup from `00b3f2e0`, with `shared_ptr` replacing native intrusive references.
Assignment skips identical wrapper pointers and retains the new wrapper before
releasing the old one. The native helper uses ECX destination slot and EDX
source slot, returning the destination in EAX with plain RET. Its original
59-byte body was checked against the installed image. The texture destructor
has renderer notifications, registrations, cached surfaces and base cleanup
outside this typed owner. `release_com()` is explicitly host orchestration,
not a recovered complete texture reset callback. Disk reload is not implemented.

Both existing CTests and the MSVC Win32 build pass. The full D3D9 probe log is
`reports/memory_stream_texture_probe.txt`; fresh disk/live byte hashes are in
`reports/memory_stream_audit.json`. Short-read, allocation-failure and invalid
seek behavior were inspected in source, not exercised by this installed-asset
run. Native ABI compatibility and gameplay remain unproven.

Three previously unclassified bodies (`00bef540`, `00bef580`, `00bef600`) were
created in Ghidra after exact-byte verification and matching dry-run body sizes
of 61, 11 and 11 bytes. Descriptive names/comments were applied with previous
values preserved, the project saved, and the inventory/exports refreshed.

This unit establishes a concrete physical-file-to-shared-backing dependency.
It does not implement mount/alias/archive resolution, native texture-quality
policy, cached resource identity, error callbacks, or a runnable game. The
original ABI and full-body SHA-256 evidence for `00bef540`, `00bef580`,
`00bef590`, `00bef600`, `00bef610`, `00bef6d0`, `00bef750`, `008d43c0`,
`008d4470`, `00bb8f90`, and `00bef9c0` are recorded in
[MEMORY_STREAM_HANDOFF.md](MEMORY_STREAM_HANDOFF.md). The implementation remains
a typed reconstruction with explicit supported-domain boundaries, not a
drop-in binary replacement.
