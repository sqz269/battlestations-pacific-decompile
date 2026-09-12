# Native FileStore owner packet map

Read-only discovery `orch2_filestore_owner_ap`, based on `116987ab`.
Owned addresses: `BE7FA0`, `BB5590`, `BE55E0`, `BE5690`, `BE80B0`, `BE8090`,
`BE7BF0`. Report: [native_filestore_owner_packet_map.json](../reports/native_filestore_owner_packet_map.json).
All live analysis used the verified `bsp` project and `/battlestationspacific.exe`;
disk disassembly read the configured installed PE. No source, tests, Ghidra
definitions, names, comments, or installation files changed. Existing names remain
hypotheses; a name or semantic reconstruction fragment is not a complete raw owner.

## Ready packet and remaining owner work

The smallest source-ready packet is **BE55E0 + BE5690**, two complete 55-byte
allocation leaves, 110 bytes total. Each allocates `1Ch` through the existing
`singleton_lifetime_allocate` service, tests addresses `p`, `p+4`, `p+8` separately
before zeroing those DWORDs, then writes byte `+18=1` and `+19=0`. EAX retains the
pointer; there are no consumed incoming register or stack arguments; plain RET.
The one size push is balanced by ADD ESP,4. Payload `+0C..+17` and padding `+1A..+1B`
are untouched. These are initially nonsentinel nodes: the caller converts each
head to a sentinel and links it to itself. A null return must not become a new
safe early return; later native address tests/stores still execute.

Use new `native_filestore_container_allocation.hpp/.cpp` and the existing shared
allocation/free boundary, following `native_vfs_container_allocation.cpp`; do not
duplicate the six already integrated VFS allocation leaves. Claim only these two
addresses and the new module files. All eight direct call sites were inspected:
BE8030/BE58E3/BE5AD3/BE7F73 use BE55E0; BE805A/BE5953/BE5B33/BE7B83 use BE5690.
Their incoming ECX values do not change the leaf contract.

**The complete FileStore owner is not source-ready.** Its resident clear loop,
both full tree destruction paths, both partial range branches, retained payload
deletion and EH cleanup wrappers are not implemented as address-matched raw
functions in this checkout. Keep them explicit dependencies. A subsequent shared
provider-base packet can pair BB5590 with BB5380 (250 bytes) using existing actual
string APIs, with the pool-getter/throwing-release boundary stated below. Do not
claim that pair completes either FileStore or physical-directory ownership.

## Owned routine coverage and native ABI

| Routine | Inclusive end / bytes | Coverage | Native contract |
| --- | --- | --- | --- |
| BE7FA0 | BE808D / 238 | complete normal body and four-state unwind map inspected | ECX raw 2Ch owner; EAX owner; RET |
| BB5590 | BB5622 / 147 | complete normal body and one-state unwind map inspected | ECX raw provider base; stack source 8-byte name header; EAX owner; RET4 |
| BE55E0 | BE5616 / 55 | complete | no inputs; EAX 1Ch allocation; RET |
| BE5690 | BE56C6 / 55 | complete | no inputs; EAX 1Ch allocation; RET |
| BE80B0 | BE8118 / 105 | complete normal body and allocation unwind action inspected | ECX actual factory; EAX cached/new provider; RET |
| BE8090 | BE80AD / 30 | complete with raw three-byte gap | ECX owner; stack flags; EAX original owner; RET4 |
| BE7BF0 | BE7CA3 / 180 | complete normal bytes and three-state map inspected; dependency closure incomplete | ECX actual FileStore; no stack arguments; RET |

The seven full ranges total 810 bytes and match current Ghidra memory and the
installed PE. BE7BF0's stored Ghidra body ends at BE7C49: **BE7C4A..BE7CA3** is
the 90-byte missing continuation, not a separate routine. BE8090 contains an
undisassembled ADD ESP,4 at **BE80A5..BE80A7**; its EAX is reloaded from saved ESI
at BE80A8, correcting pseudocode's `extraout_EAX` return after free.

## Storage established by producers

| Owner field | Producer and retained behavior |
| --- | --- |
| +00 | BB5590 writes CEB130, then D641A0; BE7FA0 later writes D689E8 |
| +04 | BB5590 writes intrusive count 1; constructor does not publish or register this owner |
| +08/+0C | BB5590 zeroes length/data, then copies the supplied actual header; no std::string or side registry |
| +10 | BB5590 writes FFFFFFFF only after string copy succeeds |
| +14/+18/+1C | Resident tree owner word/head/count; +14 untouched, +18 receives BE55E0 allocation, +1C zero |
| +20/+24/+28 | Pending-key tree owner word/head/count; +20 untouched, +24 receives BE5690 allocation, +28 zero |

The constructor first initializes a stack string header to zero and calls
41DD40(0,1). It captures the temporary data pointer in ESI; the optional copy from
CE3A0C includes the terminator. BB5590 receives that header, then the caller returns
the captured temporary pointer using its current length+1. Native getter 419CC0
consumes no arguments: the three words already pushed belong to BD1510, whose RET0C
cleans them. Do not assign those three arguments to the getter.

Each tree then receives the allocated head, byte head+19=1, head+4=head,
head+0=head, head+8=head, and count=0, with current head reloads between writes.
The constructor does not memset 2Ch or initialize the two owner words or padding.
D689E8 has zero-reference invoker BD30E0 and deleting slot+4 BE8090; the borrowed
open path is BE5FA0 at +8. Other provider slots are outside this packet.

BB5590 compares destination `this+8` with the source header, but zeroes destination
words before taking that branch. Self-alias construction therefore leaves an empty
header without releasing its former buffer. Otherwise it calls 41DD40 with the
source length and preserve=1, reloads source length after resize, and if nonzero
copies the current destination length from current source data to current target
data. BF7680's backward-overlap branch is real; retain its overlap-safe copy domain
instead of assuming arbitrary overlapping buffers satisfy standard `memcpy`.

All five BB5590 call sites agree: BE8000 supplies the temporary empty header;
BB8264, BB9CD3, BBB5D2 and **BF4D54** forward their first incoming stack path and
retain incoming ECX. The physical worker owns BF4D30 and consumes BB5590 externally.
No physical-worker address is annotated or leased here.

## Current publication and normal destruction

BE80B0 returns its first nonzero load of actual factory+8. Otherwise it allocates
2Ch, arms raw-allocation cleanup, invokes BE7FA0 with ECX equal to the allocation,
and only then stores the returned pointer to factory+8. It adds no retain, lock,
singleton registry, or rollback write to that field. Constructor failure leaves
the field unmodified while its allocation unwind action frees the raw block.
The factory's own creation/destruction and startup binding remain separate work.

The existing `file_store_00be80b0_fragment` uses a `shared_ptr<FileStore>` and a
separate semantic provider identity. It is not the actual factory+8 or 2Ch owner.
The current `native_filestore_open` code does consume actual tree+14 and current
stream/type storage, but explicitly leaves tree population and FileStore lifetime
external. `native_retained_memory_owners` supports D642C0/D15AD8 owners only; the
node payload's current virtual deletion domain must be verified before reusing
that dispatch for all FileStore residents.

BE7BF0 installs D689E8, calls BE72C0 to clear residents, then destroys pending
tree+20, frees its current sentinel, clears its head/count, destroys resident
tree+14, frees/clears its head/count, and finally calls BB5380. Both range calls
pass output pointer plus two complete `{tree,node}` iterators, **five DWORD stack
arguments**, matching callee RET14h. No physical cancellation or callback execution
is established by this cleanup.

BE72C0 repeatedly rereads resident count+1C and the leftmost current node. It
validates with returning BF6713, reads resident node+14's refcount, prepares the
warning arguments if count differs from 1, and invokes BE6760. The warning callee
4254B0 is actually a one-byte RET: no emitted diagnostic is established. The
warning call's ADD ESP,0C confirms its three arguments; caller-side loads remain.

## Exception cleanup and incomplete dependency inventory

| Routine / FuncInfo / unwind map | State -> previous state: action |
| --- | --- |
| BB5590 / DFDC90 / DFDC88 | 0 -> -1: CC43A0 tail-jumps BD30F0 with captured owner; only restore CEB130 |
| BB5380 / DFDC64 / DFDC5C | 0 -> -1: CC4380 tail-jumps BD30F0; string release does not clear stale header words |
| BE7FA0 / E017B8 / E01798 | 0 -> -1: destroy temporary header (CC6E50 -> 41DD20); 1 -> 0: base (CC6E58 -> BB5380); 2 -> -1: base; 3 -> 2: tree+14 (CC6E60 -> BE7BB0) |
| BE7BF0 / E01738 / E01720 | 0 -> -1: base (CC6DF0 -> BB5380); 1 -> 0: tree+14 (CC6DF8 -> BE7BB0); 2 -> 1: tree+20 (CC6E03 -> BE7A70) |
| BE80B0 / E017E4 / E017DC | 0 -> -1: CC6E80 frees captured raw allocation; POP ECX/RET continue at CC6E89..CC6E8A |

BE7FA0 arms state0 before BB5590, writes state2 after that returns and state3
after the first tree is complete. The table's state1 exists but is not assigned
on this normal path. BE7BF0 arms state2 before resident clearing, then state1
before pending-tree destruction, state0 before resident-tree destruction and -1
before base destruction. Preserve those boundaries; do not repeat an already
active tree destructor when it throws.

| Dependency | Body/read status and next requirement |
| --- | --- |
| BB5380 | Full 103-byte normal body and EH map inspected; no address-matched source here. Reuse actual string destruction, then base profile reset. |
| BE7580 / BE7690 | Full 201-byte normal bodies inspected, unimplemented. Full-range branch calls BE66C0 / BE6720; partial branch needs BE4E40+BE6A20 / BE4C30+BE6760. Do not port only full-range branch under a complete label. |
| BE66C0 / BE6720 | Full 82 / 61-byte flows read using raw gap repair evidence. Recurse right, capture left, destroy payload/free current, then iterate left. Missing BE6701..BE670B / BE674C..BE6756 encode the loop, not optional cleanup. |
| BE7A70 / BE7BB0 | Full 52-byte cleanup wrappers; Ghidra stops after free. Missing BE7A94..BE7AA3 / BE7BD4..BE7BE3 clear current head/count, restore stack and return. |
| BE5D70 | Full 127-byte normal body inspected; EH map still unread. Captures payload+8 owner, InterlockedDecrement, current vtable+0 at zero, clears current payload+8 after return, then releases payload string. Current-profile and unwind closure remain. |
| BE6760 / BE6A20 | Named only as native addresses here; whole bodies not read. Current stored spans 637 / 658 bytes. No source implementation found; single-node mutation/rebalance contracts must be established directly. |
| BE4C30 / BE4E40 | Unread 99-byte iterator bodies; do not invent their validation or mutation contract. |
| 41DD40 / 41DD20 | Complete existing actual-header APIs. `ActualNativeStringPoolStorage` repeats the actual getter for every operation and shares canonical lifetime/publication/gate storage. Its existing noexcept release restricts throwing lazy-getter behavior; original EH/SEH parity is not established. |
| BF681B / BF65AC | Existing source CRT allocation/free boundary; original CRT handler/exception identity remains distinct. |

The report retains every direct call in the seven owned normal bodies, the nine
listed cleanup dependency bodies, incoming allocator/base sites and the factory unwind action.
Tail JMP actions are recorded separately as transfers. Three calls in BE7BF0's
missing continuation cannot pass live function-membership verification before a
primary repair. The mechanical check examined 66 direct rows: 63 passed and those
three failed; two indirect call rows were retained and explicitly not checked.
This discovery adds no tests and makes no runtime/startup claim.

## AQ parent integration, 2026-09-12

Correction from docs/NATIVE_FILESTORE_FOUNDATIONS.md and docs/NATIVE_ADOPTED_SUBSTREAM.md: the two FileStore node allocators, provider-base construction/destruction, and adopted-substream body/dispatch are reconstructed with bounded native evidence. Full owner tree operations, pending-tree teardown and the native canonicalizer remain open. The discovery report still has three expected BE7BF0 tail CALL coverage failures; the passing source-packet checks do not erase that discovery limitation.
