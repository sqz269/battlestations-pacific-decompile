# Native VFS manager storage and lifetime packet map

Read-only discovery `orch2_vfs_owner_discovery_ao`, based on `b5e64d78`.
Addresses and names describe inferred behavior, not recovered symbols. The report
is [native_vfs_manager_packet_map.json](../reports/native_vfs_manager_packet_map.json).
No C++, ledger, Ghidra annotation, original executable, test or build change is
part of this discovery. Ghidra CLI batches verified `C:/Users/sqz269/bsp.gpr`,
program `/battlestationspacific.exe`; raw disassembly read the configured installed
PE at `I:/SteamLibrary/steamapps/common/Battlestations Pacific`.

## Decision and ready packet

**The five-address complete manager owner is not ready.** The minimal independent
implementation packet is `native_vfs_container_allocation6`: six complete native
allocation leaves, **243 bytes**. They accept no meaningful register or stack
input, return the allocated pointer in EAX and use plain RET. Their only direct
callee is BF681B, with one size argument and `ADD ESP,4` after the call.

| Address / inclusive end | Bytes | Allocation | Writes after allocation | Coverage |
| --- | ---: | ---: | --- | --- |
| BDA960-BDA979 | 26 | 0Ch | link +0 and +4 to allocation | complete |
| BDA980-BDA999 | 26 | 28h | link +0 and +4 to allocation | complete |
| BDABF0-BDAC26 | 55 | 24h | zero +0/+4/+8; byte +20=1, +21=0 | complete |
| BDABA0-BDABD6 | 55 | 20h | zero +0/+4/+8; byte +1C=1, +1D=0 | complete |
| 4C26B0-4C26E6 | 55 | 18h | zero +0/+4/+8; byte +14=1, +15=0 | complete |
| 7F82F0-7F8309 | 26 | 0Ch | link +0 and +4 to allocation | complete |

The list helpers test the allocated address before +0 and separately test
`allocation+4` before +4. Tree helpers independently test +0/+4/+8 addresses,
then unconditionally store their two bytes. Preserve that instruction schedule
if retaining native malformed-address behavior; do not replace it with an early
null return. Payload and padding bytes are untouched. In particular the tree
helpers initially produce **nonsentinel nodes**; BE1DC0 turns them into sentinels
by setting their sentinel byte to one and all three links to self.

Use new `include/bsp/native_vfs_container_allocation.hpp` and
`src/native_vfs_container_allocation.cpp`, plus matching docs/report. The primary
integrator owns the CMake source-list edit. Only the six addresses and the name /
reconstruction shards for their 004C, 007F and 00BD bands belong to this packet;
use `ledger add-*` to determine exact changed shard paths. Do not claim their
callers or BDA6F0/BE1DC0 merely because they consume these leaves.

Reuse `singleton_lifetime_allocate` with equal native and host sizes and its
malloc-paired `singleton_lifetime_free` boundary. Its current source body is the
established BF681B service: malloc, current source CRT new-handler retry, then
`std::bad_alloc`. This does not reconstruct original static CRT handler globals,
exception identity or fault-address identity. No new allocator interface is
needed. Raw allocation storage is appropriate; the typed lifetime manager is
not a substitute for native manager storage.

These are shared helpers. BDA960 also serves BDB3A0; BDA980 serves BDB3D0;
BDABF0 serves BDAFE0/BDB480/BE1940; BDABA0 serves BDAFA0/BDB450/BE1910 and
**BF4D30's physical-directory index construction**. 4C26B0 has 20 snapshot
callers spanning scene/profile/VFS code; 7F82F0 also serves 7F8960/7FEE20.
Their complete bodies consume no caller arguments, so these uses do not change
the allocation contract. Coordinate the shared BDABA0 producer with the later
physical-factory packet instead of implementing a duplicate.

## Actual owner storage and publication

Application initialization pushes A0h at 73D615, calls BF681B at 73D61A, and calls
BEDA60 with a nonnull allocation at 73D637. This corrects the older doc's
73D615 allocation-call attribution: that address is the size push. BEDA60 calls
BE1DC0, installs D68D04 and then obtains/registers BED990's physical factory.

BDA6F0 installs D683E4, obtains the lifetime manager at BDA71E, captures its
actual +10 critical-section pointer and enters it if nonnull. It increments
the captured section's +18 nesting word after EnterCriticalSection returns,
publishes `this` at BDA746, obtains the lifetime manager **again**, reloads the
current 109CEEC value, and registers that value at BDA75A. It then decrements
the captured nesting word and leaves the same section. BE1DC0 initializes the
remaining fields only after that base call returns.

BDA790 installs D683E4, repeats the captured-section protocol, obtains a second
current lifetime manager and unregisters the **current global 109CEEC**, not
unconditionally its ECX `this`, at BDA7F4. It clears the global only after
unregister returns, unlocks, then installs CE3818. If registration throws, base
construction does not clear the published pointer: the armed actions unlock
and reset the base profile. If unregister throws, the subsequent global clear
is skipped. These are meaningful reentrancy and exception boundaries.

| Manager offsets | Producer / initial state | Ownership / cleanup evidence |
| --- | --- | --- |
| +00 | D683E4 -> D685B4 -> startup D68D04 | Base reset ultimately CE3818 |
| +04/+08 | BE1DEC/1DEF writes zero | Meaning not established here |
| +0C/+10 | BE1DF6/1DF9 writes zero native string header | Pool-returned last before BDA790 |
| +14 | BE1DFC writes zero | Block-depth word from existing consumers |
| +18 | BE1DFF writes FFFFFFFF | Existing lookup status word |
| +1C; +20 | BE1E06 writes zero word; 1E09 zero byte | Meaning not established here |
| +24/+28/+2C | BE1E16/19/1C writes zero | No independent cleanup action identified; do not invent a container type |
| +30/+34/+38 | factory list; BDA960 0Ch head at +34; count +38=0 | Delete nodes and head, with no factory payload deletion in this destructor |
| +3C/+40/+44 | mount tree; BDABF0 24h head; sentinel +21 | Provider pointers at node+18 are deleted through current vtable+4 with argument 1 before tree cleanup |
| +48/+4C/+50 | zero native string-vector header | 427110(0), then BF6989 of current backing |
| +54/+58/+5C | BDABA0 20h tree; sentinel +1D | BE0C30 full-range erase, then free head; payload semantics remain a callee audit |
| +60/+64/+68 | BDA980 28h list head | Each node+8 receives BE1220; then node and head are freed |
| +6C/+70/+74 | 4C26B0 18h tree; sentinel +15 | 4D1A50 full-range erase, then free head |
| +78; +79 | zero byte; one byte | +78 is used by physical-date disabling; no immutable-value claim |
| +7C/+80/+84 | 7F82F0 0Ch list head | Nodes and head freed; payload not deleted by this path |
| +88/+8C/+90 | zero words | Application later stores callback addresses at +8C/+90 |
| +94/+98/+9C | zero vector header with 10h elements | BDEC70(0), then BF6989 of backing |

The allocator/comparator owner words at +30/+3C/+54/+60/+6C/+7C are not written
by BE1DC0. Neither are structure padding bytes such as +21..+23 or +7A..+7B.
Do not memset the whole A0h allocation and claim the original write schedule.

The seven live profile words are:

| Slot | D685B4 base | D68D04 startup |
| --- | --- | --- |
| +00 | BE25C0 | BEDAC0 |
| +04 | BDF310 | BDF310 |
| +08 | BDD440 | BDD440 |
| +0C | BDD520 | BDD520 |
| +10 | BDC8B0 | BDC8B0 |
| +14 | BD91F0 | BD91F0 |
| +18 | BDB040 | BDB040 |

Both deleting wrappers call BE1F60, conditionally free on `flags&1`, then return
the captured object in EAX, RET4. The pseudocode's `extraout_EAX` is incorrect:
BE25D8 explicitly reloads ESI. Current `native_vfs_runtime_bindings.cpp` accepts
only D685B4 for open/exists; accepting D68D04 is a separate integration change.
Do not replace the startup profile identity with the base identity.

## Complete destruction schedule and missing definitions

BE1F60's live stored body ends at **BE2028**, after BF6989. Installed bytes
continue through **BE21E2 RET**: missing body **BE2029-BE21E2 (442 bytes)**.
There is no new function boundary at BE2029. This discovery does not repair
Ghidra or claim those absent instructions passed live call-site verification.

The disk continuation establishes this order:

1. Install D685B4; if current 109CEE8 is nonnull, invoke its current vtable+0
   with argument 1. That object's concrete profile/ownership is still unread.
2. Iterate every mount and call each nonnull provider's current vtable+4(1).
   There is no node+1C ownership-byte test here. BD97E0 advances the iterator.
3. Destroy +94 vector, then +7C list, +6C tree, +60 list, +54 tree, +48 vector,
   +3C tree, +30 list, +0C string, and finally BDA790.

The complete normal-body dependencies include BF6713's returning invalid-iterator
handler boundary, BD97E0, BDEC70, BF6989, BF65AC, 4D1A50, BE1220, BE0C30,
427110, BE0D00, 419CC0, BD1510 and BDA790, plus the two virtual deletion calls.
Existing raw mount iteration, pooled string operations and native string-vector
operations can be reused. Do not reduce BDEC70 or the range-erase helpers to
their destructor-only zero/full-range branches while labelling the functions
complete.

| Unported dependency family | Direct contracts / remaining reads |
| --- | --- |
| BDEC70-BDECD4 | Native 10h-element vector resize. Calls BDCD10-BDCDEC on growth and BDB850-BDB8C5 when shrinking. Those two bodies remain unread. |
| BE0C30-BE0CF8 | 20h-node tree range erase. Full range calls BDF7A0-BDF7DC; partial range calls BD9860 and BDFD80-BDFFFC. BDFD80's saved `STL_xlen_throw` name does not establish its behavior. |
| BE0D00-BE0DC8 | 24h-node mount-tree range erase. Full range calls BDF7E0-BDF831; partial range calls BD97E0 and BE0080-BE0311. |
| 4D1A50-4D1B18 | 18h-node range erase. Full range calls 4CEC60; partial calls 4BE730-4BE792 and 4CF8A0-4CFB31. 4CEC60 is named in live Ghidra but this checkout does not expose an address-matched raw implementation; coordinate its current owner. |
| BE1220-BE12B9 | Destruct node payload: two pooled-string lists at payload+14 and +8, then pooled string at +0. Reuse actual 4D05E0 and string-pool operations; original EH still needs a focused audit. |
| BDB3C0 -> BDAED0 | Factory-list destruction thunk and actual list walk; BDAED0 stored body ends BDAF0B, after returning-free hazard. Full extent not determined in this pass. |
| 7F8770 -> 7F8310 | +7C-list destruction thunk and list walk; 7F8310 stored body ends 7F834B, same hazard. Full extent not determined here. |
| BE1D60 -> BE19E0 | +60-list destruction wrapper and list clear. BE19E0-BE1A1D remains unread. |
| BDB970-BDBA04 | Six constructor path-normalization exercises: 41E870, BEE390 and pool-return cleanup; not a no-op merely because output is discarded. Existing path/string operations are candidates for composition; BDB970's own register provenance and EH remain to audit. |
| BDA6F0 / BDA790 | Actual lifetime getter/manager +10 section ownership is still an integration boundary. Raw BD0C30/BCFCA0 already exist; `SingletonLifetimeDomain::get_manager_00415350` returns a semantic C++ owner and is not a native raw-pointer getter. |

Other observed missing Ghidra ranges are **BE16E4-BE16F3**, **BE1724-BE1733**,
**4D74C4-4D74D3**, **BE1D71-BE1D7C**, **BE0362-BE0366**,
**BE1259-BE12B9**, and **BE25D5-BE25D7**. These include required head/count
resets, subsequent member destruction and returning-free stack cleanup. The
existing 4D0FA0 implementation already documents its missing 4D0FB2-4D0FB6 tail;
reuse that actual implementation. A saved no-return `_free` annotation must not
silently delete the continuation from a reconstruction.

## Exception cleanup is part of the owner packet

BE1DC0 uses FuncInfo E0107C and unwind map E010A0; BE1F60 uses E010F0 and E01114.
Both maps have ten states chained to the preceding state. Raw handler code is
CC6910-CC6987 and CC6990-CC6A07. Constructor thunks use `[EBP-10]`; destructor
thunks use `[EBP-18]`. The state-to-action map is:

| State | Offset | Cleanup target |
| --- | --- | --- |
| 0 | +00 | BDA790 |
| 1 | +0C | 41DD20, reusable actual string-header destructor |
| 2 | +30 | BDB3C0 -> BDAED0 |
| 3 | +3C | BE16C0 -> BE0D00 then free/reset head/count |
| 4 | +48 | 4D0FA0, reusable actual string-vector destructor |
| 5 | +54 | BE1700 -> BE0C30 then free/reset head/count |
| 6 | +60 | BE1D60 -> BE19E0 then free/reset head |
| 7 | +6C | 4D74A0 -> 4D1A50 then free/reset head/count |
| 8 | +7C | 7F8770 -> 7F8310 |
| 9 | +94 | BE0350 -> BDEC70(0), then backing free |

BDA6F0/BDA790 have two-state maps E002EC/E00320. State 1 destroys the captured
eight-byte section guard through actual 411EE0; state 0 invokes 412430, which
only restores CE3818. Constructor base publication survives a throwing register
call as described above. A general C++ destructor that unregisters on every
constructor failure would change that behavior.

## Follow-up boundaries and acceptance

First implement the six allocation leaves and run the required MSVC Win32 build;
prefer existing allocation/native-instruction checks and one focused dirty-byte
check only if needed to protect the untouched-payload behavior. This discovery
itself requires no tests. Then size the independent full list/vector/tree
destruction families, including their non-destructor branches and EH, before
reconstructing BE1DC0/BE1F60/BE25C0 and the singleton base.

Keep actual physical factory BED990/BF4DF0/BF34D0/BF4D30/BB5590 separate, sharing
BDABA0 through the allocation packet. Keep factory/mount insertion
BE0660/BDB040/BE1890/BE1740/BE1330/BE0DD0/BDCE40/BDCFC0/BDEEC0 and FileStore
construction separate. BEDA60 and BEDAC0 belong to later startup composition;
their presence does not make those provider and insertion contracts complete.

The report distinguishes stored-function call-site rows (checked using
`verify_report_calls.py`) from raw continuation calls outside Ghidra's body.
Only the former can pass that check today. Static reading and packet readiness
are the results; reconstruction, build, fixture, ABI and game validation remain
unclaimed.

## Integration correction from docs/NATIVE_PHYSICAL_MEMORY_BINDING.md

The six allocation producers BDA960, BDA980, BDABF0, BDABA0, 4C26B0 and 7F82F0 are now reconstructed as actual raw-storage routines in docs/NATIVE_VFS_CONTAINER_ALLOCATION.md. Their independent destination checks, untouched bytes and null-allocation fault behavior are preserved. This resolves that allocation dependency only. Actual container construction/destruction, manager publication and cleanup, BE1F60 stored-tail coverage, factory and mount insertion, and startup-derived D68D04 dispatch remain separate dependencies.

Evidence: reports/native_ao_integration.json; reports/native_physical_memory_binding.json.
