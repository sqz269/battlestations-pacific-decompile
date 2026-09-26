# Actual procedural factory table lifetime (CC10)

This implements the qualified normal lifetime of an already live 10h table
and its actual 4B factories. BBC4E0/BBC500 (25B each), BBC520 (144B), BBC5D0
(29B), BBC890/BBC8E0 (31B each) total **285 bytes**. Compiler evidence
CC4B20/28/33/3E contributes another **40 bytes**. The code exposes source C++
interfaces, not original register/private-stack ABI or native FH3/SEH transport.

The producer evidence remains external: BBCB56 pushes10h before BF681B at
BBCB58; BBCB71 calls BBC900 and BBCB76 publishes its result at01090900.
BBC900 separately allocates four4B factories, stamps final D644E8 for slot0
and D644F0 for slots4/8/C, then publishes each completed owner. This packet
does not implement either constructor, allocate process publication storage,
register an atexit callback, construct a resource34h, or invoke startup.

## Current storage and providers

The table is four native DWORD pointers with no profile or count. Each reached
factory is actual4B profile storage, freed through the same genuine CRT domain.
The source borrows the existing `NativeResourceRegistryLookupContext` profile
views; it creates no second registry, copied profiles, ownership map or credits.

| Current factory profile | Required current profile+0 | Source provider |
| --- | --- | --- |
| D64470 | BBC650 | Existing caustics factory scalar |
| D644AC | BBC7A0 | Existing shore-wave factory scalar |
| D644E8 | BBC890 | New final caustics scalar wrapper |
| D644F0 | BBC8E0 | New final shore-wave scalar wrapper |

BBC890/BBC8E0 reuse the exact reader scalar effects: test the current flags
LOW byte bit0 **before** the D64468 base stamp, free only under that captured
test, and return captured pointer bits without a post-free payload read.
The base D64468/BBC460 domain is explicitly unsupported; an unknown profile
or mismatched current scalar target fails at the reached source binding site.

Each slot helper captures its slot address, reads the current owner, and skips
null without writing it. For nonnull owners it reads current profile then
current target0, validates the qualified pair, writes1 into the borrowed flags
cell, and invokes the genuine scalar. Only after successful return does it
clear the **captured slot address**. A replacement written into that slot during
the call is cleared, without disposing the replacement. Future slots are not
read early. Flags are seeded after target capture, not before profile lookup.

## Table order and failure residuals

BBC520 seeds the mutable cleanup-self cell twice at the native PUSH ECX/MOV
sites. Its normal path retains the captured table, reading slots C/8/4/0 in
that order and setting states2/1/0/-1 after each owner read, before dispatch.
The normal calls reuse one actual flags argument cell (native entry S-1C).

The EH map atDFE954 and handler info atDFE96C establish state2 -> self+8,
state1 -> self+4, state0 -> self+0. Each source cleanup action consumes its
state **before** reloading current cleanup-self, then invokes BBC500/BBC4E0
with its explicit caller-owned helper frame. A change to cleanup-self can
redirect later actions. Normal destruction continues using its captured table.
No native FH3 private-stack alias relation is invented for the helper frames.

A throwing slot is not cleared or disposed again. If cleanup succeeds, the
source rethrows the original source exception. A second cleanup failure stops
immediately, retaining the consumed state, reached slot, captured targets and
completed effects; it does not retry or mark the table settled. These source
validation/injection exceptions are separate from native fault/exception
transport and its termination rules.

BBC5D0 reads01090900 once. It destroys and then frees that captured nonnull
table only after normal child return. It neither reloads nor clears publication.
A publication replacement is preserved while the captured old table is freed.
A child failure prevents the table free. Normal native behavior can leave a
dangling publication; this API does not turn a subsequent nonnull value into a
new admitted lifetime.

All acquired trees must be fresh, persistent and disjoint from payload/cells.
Replay fails before native work. Frames/profile views/acquisitions must outlive
the operation and retained failures; no metadata destructor frees native owners.
The global wrapper additionally requires CRT-freeable table backing. Duplicate
owners, reentrant double destruction, stale pointers and unsupported profiles
are caller-boundary failures, not repaired through synthetic registrations.

Registry nodes borrow factory bits at+14 without retaining them. Quiesce all
lookups before disposing factories. Genuine later registry erase can release
names/nodes without dereferencing those borrowed values. This packet establishes
no application-wide shutdown ordering or factory/resource admission.

## Validation and exact limits

- Strict MSVC Win32 build and all three existing CTests passed. No tracked
  tests or application/game/original-process launch was added.
- Every proposed native/compiler body has full live/PE-equal bytes, exact
  boundaries and complete local decodes. All 14 call/transfer rows are reviewed;
  the live checker passed eight direct callee rows, with containment deferred
  for the three undefined bodies. Six current calls remain explicitly indirect. Missing function
  definitions are listed with inclusive ends and last-instruction lengths for
  primary annotation; the worker performed no Ghidra mutations.
- The uninstrumented linked source passed four-profile normal disposal,
  reverse slot order, captured global free/no clear, replay rejection and both
  flags0 scalars. A source-interface alias case verified target capture before
  the flags write; it is not a native stack/profile-alias claim.
- An ignored renamed translation unit inserts narrowly documented observation
  hooks while delegating actual reader/CRT providers. Post-return mutation
  checks passed: replacement-slot clearing, late future-slot loading, normal
  captured-table use after cleanup-self mutation, and publication replacement
  with captured old-table free. No hook was added to production source.
- Source-only pre-provider failure checks passed current cleanup-self reloads
  and stop-on-second-failure residuals. An uninstrumented reached base-profile
  rejection also preserved its throwing slot/table while genuinely disposing
  siblings. Failed operations and backing remain allocated through explicit
  diagnostic `ExitProcess(0)`; that is not failed-frame/native-EH cleanup proof.

The initial normal fixture exitedC0000409 without buffered output; its cause
remains unproved. A separate diagnostic revision flushed stage output and
reported that requested00BB0000 was already MEM_RESERVE/MEM_PRIVATE, allocation
base00BA0000. VirtualAlloc failed487. No occupied memory was replaced, no larger
reservation was attempted, and **copied-native normal execution is inconclusive**.
The diagnostic continued the independent source mutation check and returned2.
This observed refusal does not retroactively identify the first failure.

Both fixture versions, exact source/instrumentation differences, objects,
libraries, executable identities and receipts are retained. COFF/PE comparison
pins full emitted sections and relocation maps; linked code identity is not a
recursive CRT graph or original ABI claim. The quiet first attempt's actual
ASLR module base was not captured; the diagnostic recorded00570000.

See [the report](../reports/native_procedural_factory_table_storage_cc10.json)
for hashes, native rows, compiler states, fixture exits and the frozen evidence
index. The archive includes the approved 79-file readiness packet, both fixture
versions, and frozen build inputs. All 1,187 prior indexed artifacts were checked
against their existing indexes and remain preserved separately.
