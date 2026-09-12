# Native VFS manager storage lifetime

Addresses: 00be1dc0, 00be1f60, 00be25c0.

The three complete installed bodies now have C++ implementations in
`src/native_vfs_manager_lifetime.cpp`. They construct and destroy the actual A0h
manager storage, borrow the application publication cells and lifetime manager,
and use the reconstructed canonical string pool and container operations.
Names are descriptive hypotheses, not recovered symbols. These source APIs are
not original-ABI replacements or evidence of a running game.

## Native evidence and range limits

The target is `C:/Users/sqz269/bsp.gpr`, `/battlestationspacific.exe`; the installed
PE SHA256 is `b682a82c52f81f957b2c70222077305a933f72481686c88843077f714b956dd6`.
Live Ghidra bytes matched all 28 captured spans, 1668 bytes total: three bodies
(1084 bytes), 20 cleanup funclets, two handlers, both EH tables, the profile word
and six path literals. Captures and complete raw transfer lists are retained in
`local/native-manager-at-evidence`; hashes and contracts are in the report.

| Entry | Complete logical interval, end exclusive | Native ABI |
| --- | --- | --- |
| BE1DC0 | BE1DC0..BE1F5B, 411 bytes | ECX owner; RET0; EAX owner |
| BE1F60 | BE1F60..BE21E3, 643 bytes | ECX owner; RET0; no semantic result |
| BE25C0 | BE25C0..BE25DE, 30 bytes | ECX owner; stack flags; RET4; EAX original owner |

Ghidra originally stored the destructor only through BE2028. The 442 bytes
BE2029..BE21E3 contain most member teardown and are supported by installed/live
bytes and linear assembly. Decoding them and clearing verified CRT call overrides
does not extend the stored function range. The report qualifies these sites as
raw logical-tail evidence instead of attributing them to a stored Ghidra body.
The deleting wrapper also had a three-byte internal ADD ESP,4 listing gap.

## Construction and member ownership

Construction first calls BDA6F0, which publishes through actual 0109CEEC and the
existing lifetime manager. It then writes D685B4 and initializes the members below.
Opaque allocator/comparator DWORDs at 30,3C,54,60,6C,7C and padding remain untouched.

| Owner offset | Storage and initialization |
| --- | --- |
| 0C | Native string header; length/data zero |
| 18 | FFFFFFFF; 1C zero; byte20 zero |
| 24 | Three zero DWORDs |
| 30 | Plain list; BDA960 allocates a 0Ch sentinel; head34/count38 |
| 3C | Mount tree; BDABF0 allocates 24h sentinel, nil byte21; head40/count44 |
| 48 | Native string vector; three zero DWORDs |
| 54 | Physical index tree; BDABA0 allocates 20h sentinel, nil byte1D; head58/count5C |
| 60 | Nested request list; BDA980 allocates 28h sentinel; head64/count68 |
| 6C | String tree; 4C26B0 allocates 18h sentinel, nil byte15; head70/count74 |
| 78 | Bytes78=0 and79=1 |
| 7C | Plain list; 7F82F0 allocates 0Ch sentinel; head80/count84 |
| 88 | Zero DWORDs88,8C,90 |
| 94 | Native string-pair vector; three zero DWORDs |

The tree sentinel becomes nil and links left/parent/right to itself, reloading
the current owner head before each link store. All six allocation leaves are
actual implementations. After the final vector is initialized, state9 is armed
and BDB970 runs six times with original values at D68464,D68454,D68444,D68438,
D68414,D68400. The context must bind these values and the application's actual
canonicalizer; their allocation, folding and release calls remain observable.

## Destruction and dispatch

The destructor writes D685B4, captures current 0109CEE8, and, when nonnull, reads
that captured owner's current vtable slot0 and calls it with flags1. It then walks
the mount tree. Each nonnull node+18 provider is sent its current vtable slot4
with flags1; node+1C does not gate ownership. The iterator captures its end before
a returning validation callback and reloads its owner only after BD97E0 advances.

The required virtual-call interface receives the exact captured numeric target,
owner and flags. Application bindings must invoke that target; they may not
re-read the publication, infer a replacement owner or ignore the operation.
Actual file-log deleting wrappers and physical-provider BF4DD0 are available as
separate reconstructed callees. Other provider classes still need their concrete
application bindings; this module does not claim their ownership code complete.

Normal teardown then destroys pair-vector94, plain-list7C, string-tree6C,
request-list60, physical-index54, string-vector48, mount-tree3C, plain-list30,
string0C, and base BDA790. Vector backing headers retain their native dangling
values after free. Tree heads/counts and list heads receive the native zero stores.
Plain-list payload pointers are not destroyed. Request nodes destroy their nested
payload at+8 before their allocation is freed. Current-head reloads and the
comparison before count reset are preserved, including native inline list code.

BE25C0 always runs the destructor, then frees the original owner only when bit0
of flags is set. It returns that original pointer even after free.

## Cleanup evidence and limits

Ctor FuncInfo E0107C uses map E010A0 and funclets CC6910..CC6970; destructor
FuncInfo E010F0 uses map E01114 and funclets CC6990..CC69F0. Both maps contain ten
states chained to their predecessor. In descending state order, cleanup destroys
pair94, list7C, string-tree6C, request60, index54, vector48, mount3C, list30,
string0C and base. Normal inline list teardown deliberately leaves state8 or
state2 armed, matching the assembly; no extra state or rollback was invented.

The source reproduces this chain for ordinary C++ unwinding through the supplied
services. Original MSVC FH3/SEH handler identity, mutable EH-frame spill aliases,
throwing native pool getter behavior and simultaneous cleanup exceptions are
unvalidated. Actual pooled release uses the existing noexcept bridge.

Strict MSVC Win32 compilation and both existing CTests pass with the three worker
dependencies integrated. The focused manager fixture passes five native/source
comparisons and one source-only state9 cleanup case (12605 checks). Cases cover
empty and populated storage, nested request payloads, both file-log targets
7376A0/737CC0, physical provider BF4DD0, direct destruction, and deleting flags
0,80000000,80000001. Complete normalized owner bytes, actual string-pool returns,
and allocation/free/virtual/lowercase event order agree. The caller retains its
allocation after the deliberate source-only lowercase exception and all six
heads and the base publication are cleaned up.

Owner snapshots after destruction cover the cases that retain the allocation.
The freeing case checks the returned original pointer and free flag, and skips
reading the released owner. This qualifies the shared comparison labels in the
sealed cases file; no read of freed storage is used as evidence.

All 1084 original manager body bytes remain unchanged at a uniform 30000000
shift, with all 39 direct calls and 55 relative transfers checked. External
callee entry bridges call the actual reconstructed services. Original vtable
values and path bytes come from exact installed PE pages. A dedicated launcher
reserves five required 64KiB pages in its suspended probe before CRT startup;
the probe verifies allocation ownership markers before loading those pages.
This solves the test-process address collisions without changing native body
instructions or table values. Prior failed attempts are retained separately.

The passing attempt pins 192 physical inputs, including 38 linked object files
matched to the archived library, probe/launcher source and binaries. All input
hashes remain unchanged after execution. Original FH3/SEH is unexecuted; the
cleanup exception case runs source only. This is an isolated instruction/source
composition with explicit service bridges, not original game execution.

## Follow-up packets

Recover complete derived manager BEDA60/BEDAC0 and its installed startup binding,
then compose the application VFS bring-up with full concrete provider ownership.
FileStore/MPKG/MPAK provider virtual bindings and game startup remain separate
requirements. No game executable launch or gameplay validation is claimed here.
