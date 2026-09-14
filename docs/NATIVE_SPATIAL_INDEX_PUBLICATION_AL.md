# Raw spatial-index construction and publication

Addresses: `0042D450`, `0042E630`.

The complete 62-byte constructor and 174-byte getter now have source bodies in
`src/native_spatial_index_publication.cpp`. They use actual raw storage and the
existing raw singleton manager, allocator, registration and guard providers.
This finishes a dependency of the pending unit-part attachment path. It does
not admit a spatial index into the executable's current owner domain.

## Storage and publication order

`0042D450` consumes ECX, returns that pointer in EAX and uses plain RET. It writes
profile `CE3CEC` at zero and zero at `+4`, calls memset on the `15F90h` bytes at
`+84`, then clears the count at `+80` and root at `+16014`. The allocation is
`16018h` bytes. All thirty loose slots at `+8..+7C` remain untouched. No typed
layout initialization or callable replacement vtable is inferred.

`0042E630` takes no native arguments, returns EAX and uses plain RET. Its source
interface borrows distinct stable references to the actual mutable `01090AA0`
manager and `F8A0D8` spatial-index publication cells. The fast path captures
and returns the existing index without looking up a manager.

The slow path looks up the raw manager through `00415350`, captures its section
at `+10`, enters that Win32 critical section if nonnull and increments the
physical DWORD at section `+18`. It then rechecks the index publication. If
absent, it allocates `16018h` through the canonical allocator and constructs the
returned allocation if nonnull. It publishes the result before looking up the
manager again, rereads the index publication and registers that captured value
through the complete raw `BD0C30` provider. The original captured section is
decremented and released even when the second lookup returns another manager.
The slow return reloads publication after LeaveCriticalSection.

The getter's FH3 handler `C5EB28` points to FuncInfo `D851D8`. Its sole unwind
map row at `D851D0` moves state zero to minus one through `C5EB20`, which passes
guard `EBP-14` to `411EE0`. Guard cleanup begins after Enter and the increment.
There is no allocation cleanup state or rollback of publication. The source
expresses this with catch, canonical guard cleanup and rethrow, including the
normal decrement/Leave inside the guarded region. The allocator's native null
branch is retained; the actual source allocator normally retries or throws.

## Evidence and verification

`reports/native_spatial_index_publication_al.json` records all six direct call
sites, both imported critical-section calls, and four fresh Ghidra/disk byte
comparisons: 236 complete function bytes and 62 exception-evidence bytes. The
existing names remain descriptive hypotheses; annotation updates preserve prior
comments and clarify the exact constructor writes and exception state.

An ignored MSVC Win32 probe compares the original relocated constructor and
getter with the actual archive implementations. The constructor comparison
covers every allocation byte plus both canaries, including the untouched loose
slots. Seven paired getter scenarios cover prepublication, a null section, an
actual tracked section, canonical creation of a missing raw manager, publication
while waiting for the section, replacement of the manager while waiting, and a
returning SDK validation handler that changes index publication during
registration. The two blocked cases observe actual critical-section contention
before mutating publication. No shadow manager or callback allocator is used by
the rebuilt getter.

A source-only SDK validation exception verifies that registration failure
retains the constructed publication and balances the captured lock counter.
Original EH is not executed. The original reference routes the unchanged
manager and registration dependencies to their canonical source providers,
allocation to the same source CRT, and memset/locking to SDK services. This is
bounded caller equivalence under those shared service contracts, not an
independent proof of every dependency. The getter results normalize allocation
addresses and compare defined fields and pointer relationships; the standalone
constructor retains separate complete byte images.

The compiled object is inspected for constructor write order, volatile
publication reloads, both manager lookups, raw registration, captured-section
release and guard cleanup/rethrow. No repository tests were added. Native
FH3/SEH identity, hardware faults, an allocator returning null, the index's
deleting destructor, attachment, executable admission and gameplay parity
remain unproved. Probe cleanup reclaims fixture allocations directly and does
not claim native owner destruction.

## Integrated validation

Commit `9f0e2bd05d9614c51f1ef863e25b4e5cbb6db0da` passes the Win32 build and both existing CTests. All eight paired original-byte cases and source registration-exception cleanup pass against that exact library. The 120-frame USN01 compatibility run passes finite-trajectory, stationary Airfield2, avoidance, generic-tick, participant, world-list and observer/pending-owner checks. The preserved executable SHA-256 is `fee6f2e5ae388b344a68d0d8f447a86257857dca32a532d52083a095cc1bfa27`. An immutable manifest retains original reference bytes, separate raw constructor images, normalized getter results, linked objects, compilation dependencies and mission artifacts. Spatial admission and gameplay parity remain unproved.

After merging separately published work, combined commit `334f84e3f2e3dbe1bec62943ea0ff17535dfe3ea` passes the build, both CTests and the same 120-frame compatibility checks. Its executable SHA-256 is `d6c92a8aa8087faf7c7eb7b6b06bcf2d88c62ea3a35a1891d94146c7b50241a5`. All 12 production objects linked into the spatial constructor/getter fixture remain byte-identical to the sealed proof. The additional integration manifest is retained separately.
