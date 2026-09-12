# Native FileStore allocation and provider-base foundations

Addresses: `00BE55E0`, `00BE5690`, `00BB5590`, `00BB5380`.
Evidence: [native_filestore_foundations.json](../reports/native_filestore_foundations.json).
This packet implements two actual node allocation leaves and the provider base
shared by FileStore, MPKG and physical-directory constructors. The full FileStore
constructor, factory and tree/retained-payload destruction remain incomplete as
mapped in `NATIVE_FILESTORE_OWNER_PACKET_MAP.md`.

| Original range, inclusive | Bytes | Coverage | Original ABI | Source interface |
| --- | ---: | --- | --- | --- |
| BE55E0..BE5616 | 55 | complete | no consumed inputs; EAX pointer; RET | `allocate_native_file_store_node_00be55e0()` |
| BE5690..BE56C6 | 55 | complete | no consumed inputs; EAX pointer; RET | `allocate_native_file_store_node_00be5690()` |
| BB5590..BB5622 | 147 | complete in stated host domain | ECX actual provider, source-name header stack; EAX owner; RET4 | `construct_native_file_provider_base_00bb5590(owner, source_name, storage)` |
| BB5380..BB53E6 | 103 | complete in stated host domain | ECX actual provider; no stack args; RET | `destroy_native_file_provider_base_00bb5380(owner, storage)` |

All four owned bodies total 360 bytes and match live Ghidra memory and the
installed PE. The leaf declarations retain the original machine signature. The
base methods add an explicit C++ storage argument; they are not binary replacements.
Ghidra remained read-only. Proposed names and previous identities are recorded in
the ledger/report for the primary agent to annotate later.

## Actual allocation and storage

Both leaves call the existing `singleton_lifetime_allocate` with native and host
size `1Ch`. They preserve the independent tests of `p`, `p+4` and `p+8`, zero those
DWORDs, then write bytes `+18=1` and `+19=0`. The size push is balanced by ADD ESP,4.
The returned pointer stays in EAX. Payload `+0C..+17`, padding `+1A..+1B`, and caller
storage remain untouched. These leaves produce nonsentinel nodes; their callers
separately form sentinel links. No safe-null return, memset, alternate allocator,
tree object or registry is introduced. Release uses `singleton_lifetime_free`.

BB5590 writes CEB130, reference count `+4=1`, captures whether source is `this+8`,
writes D641A0, arms base cleanup, and zeroes both destination name words. An alias
therefore becomes empty without releasing its previous data. A distinct source
supplies its current length to the existing actual-header resize with preserve=1.
The constructor then reloads source length; if nonzero, it captures destination
length, source data, then destination data in that order. It copies that captured
count with overlap-safe `memmove`, matching original BF7680's backward-overlap
branch. Device `+10=FFFFFFFF` is written only after the copy succeeds. No other
owner field is initialized, and provider storage is not allocated or registered.

The existing actual-header policy omits a zero-byte native copy. The new outer
copy also preserves all field reads but omits the call if its captured count is
zero, avoiding undefined C++ null-pointer arguments. This is a stated host boundary,
not a claim to reproduce CRT instrumentation for zero-byte calls.

BB5380 first installs D641A0. The existing `destroy_native_string_header_0041dd20`
captures nonnull data `+0C`, then current length `+8`, and releases exactly
`length+1` with DWORD wrap. Cleanup restores CEB130 after return. It does not clear
stale name fields, modify count/device, release provider storage, or unregister the
provider. The original normal body and one-state unwind map were both inspected.

## Shared services and exception boundary

Pass the existing `ActualNativeStringPoolStorage`, bound to the application's
actual `01090AA8` publication slot, `01090AA4` return gate and canonical lifetime
domain with `NativeStringPoolLifetimeBinding`. Every allocation/release repeats
the actual pool getter; no pool pointer is cached by these base methods. The
existing bridge uses actual arena/ring storage, critical sections and lifetime
registration. The allocation leaves use the same source CRT service as the six
already integrated VFS leaves.

BB5590 FuncInfo DFDC90 / map DFDC88 has state0 -> -1 via CC43A0, which passes the
captured owner to BD30F0. BB5380 FuncInfo DFDC64 / map DFDC5C does the same through
CC4380. Both actions only write CEB130. Source C++ cleanup retains that one action;
a throwing constructor allocation leaves count=1 and zero name fields while the
device word remains untouched. It adds no cleanup of a partially created name.

Original static CRT handler state, exception object identity, FH3 stack encoding,
and SEH propagation are not reproduced. `NativeStringStorage::release` is already
noexcept: if its actual pool getter throws while lazily recreating the pool, the
existing bridge terminates. This packet does not claim native throwing-release
parity or convert that failure to a successful cleanup.

## Calls and cross-family evidence

| Site | Callee | Setup / cleanup evidence |
| --- | --- | --- |
| BE55E2 / BE5692 | BF681B | PUSH 1Ch; ADD ESP,4; EAX pointer |
| BB55E9 | 41DD40 | ECX destination header; PUSH preserve=1 then current source length; callee RET8 |
| BB55FE | BF7680 | Capture destination length, source data, destination data; three pushes; ADD ESP,0C |
| BB53BC | 419CC0 | No arguments consumed; the three already-pushed words belong to the next call; RET |
| BB53C3 | BD1510 | ECX returned pool; stack captured data, current length+1, unused=1; RET0C |
| BB53D2 | BD30F0 | ECX captured provider; state disarmed; write CEB130; RET |

All five constructor callers and all eight node-leaf callers were checked in the
AP discovery. BF4D54's physical-directory call forwards its first stack path and
incoming owner ECX, matching FileStore/MPKG use. The destructor's normal callers
and unwind tails likewise supply the actual provider, without additional stack
arguments. Generic pointer ownership was not inferred from the memory-stream-only
dispatch context. This packet does not alter any consumer runtime context.

## Validation and evidence preservation

The strict MSVC Win32 build passed with `/W4 /WX /fp:strict`; both existing CTests
passed and all eight seed bodies matched the installed PE. All 24 call-site rows
passed mechanical verification. No permanent tests were added. The ignored focused
fixture records these separate results:

- Both linked source leaves execute their real shared allocation/free service.
  All 110 emitted bytes match original instructions except the two CALL relocations.
- Private copies of original and linked leaf code redirect only their allocation
  CALL to a controlled fixture: exact dirty payload/padding/guard checks pass and
  all four forced-null copies fault on the write to address 4. Production entries,
  the archive and installed PE are untouched.
- Source cold construction creates the actual string pool, registers it once with
  the canonical lifetime manager, and returns storage through its actual ring.
- Five provider cases compare ten original/source snapshots: empty, ordinary name,
  self-alias header, source length changed during allocation, and overlapping copy.
  Snapshots compare all 44 surrounding owner bytes, pooled data and ring/bump state.
- One source-only injected allocation exception verifies the base-reset action.
  Original FH3 exception machinery is not executed by that check.
- Actual lifetime shutdown destroys the registered pool, clears publication and
  sets the return gate.

The original provider and string-resize instruction copies redirect only their
direct dependency CALLs to original BD30F0 bytes, the existing actual pool
getter/allocation/return source services, and host `memmove`. The overlap and
current-length cases mutate fixture input after a real allocation on both sides;
they do not substitute a fake pool. No exception is thrown through copied FH3
frames. These are bounded differential checks, not whole-game or original-CRT
execution.

`local/filestore_foundations_probe_provenance.json` was captured **after linking
and before executing** the retained fixture. It pins 24 source, object, archive,
probe and preparation artifacts. All 24 hashes were checked again after the
completed build and probe. The report embeds the essential hashes and the manifest
hash so the primary agent can freeze these exact artifacts before other builds.

## AQ parent integration, 2026-09-12

All four functions were reviewed, named, commented, saved and force-exported by the parent integrator. The 24 direct CALL rows passed live verification. The original 24-file before-execution manifest and 34 parent-frozen artifacts remain preserved. The combined strict Win32 build and both existing CTests passed; these checks do not establish game validation.
