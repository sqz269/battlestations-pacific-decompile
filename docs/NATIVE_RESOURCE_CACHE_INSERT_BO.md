# Actual resource-cache unique insertion BO

The packet adds complete actual-storage source for both insertion routines in
[native_resource_cache_insert.cpp](../src/native_resource_cache_insert.cpp).
Worker base is `44eed0a86fdaa52e82326cbfb58154b28ccd588a`; the committed
resource-loading frontier and integrated node/lookup/link contracts were
checked before composition. The [report](../reports/native_resource_cache_insert_bo.json)
contains complete native spans, exact call classifications, FH3 and build evidence.

| Routine / full span | Original ABI | Coverage |
|---|---|---|
| `B7FF80..B8016B`, 492 bytes | ECX tree; stack output,insert-left,parent,pair; EAX output; RET10h | Complete source |
| `B803B0..B804C3`, 276 bytes | ECX tree; stack result,pair; EAX result; RET8 | Complete source |

Actual tree storage remains allocator word+0, head+4 and count+8. A head's
links are minimum/root/maximum at+0/+4/+8. Ordinary 1Ch nodes have the existing
left/parent/right links, owning key+C/+10, borrowed raw resource+14, color+18
and nil+19. The input is the actual 0Ch key/resource pair. These bodies neither
retain nor release mapped resources and introduce no tree projection.

## Count, allocation and balancing

`B7FF9C` rejects initial **unsigned count >=15555554h** before reading head
or allocating a node. This is a direct count limit, not one of the alias-list
growth wrappers' subtraction predicates. Passing the check captures head once
for both child arguments, preserves incoming parent, and calls actual
`B7F6A0` with color0. There is no null-allocation guard after that dependency.

After construction, `B8000B` reads current head **before** the DWORD increment
at `B80013`. The increment reads current count and wraps; there is no second
limit check if allocation changed it. Parent comparison uses that captured
postallocation head. Root insertion writes head.root, reloads current head
for minimum, then reloads it again for maximum. Nonroot insertion writes the
captured parent's selected child first, then reads current head for the
minimum/maximum update. Only the low insert-left argument byte is tested.

Balancing keeps the inserted node for the eventual result and a separate
working node. It preserves native current-parent/grandparent reads across
recoloring, including repeated reads through the working node's parent slot.
It composes the existing actual right/left rotations and their current-link
schedules. `B800F6..B80132` is an inlined left rotation with that same schedule;
its source reuse is recorded as inline equivalence, not a native CALL.
The final current root is blackened, then output.node+4 is written before
output.owner+0. No inserted-node rollback is armed in this routine.

## Unique-key selection and outputs

`B803B0` captures the initial head/root and descends current links. Stored
length0 sorts as empty regardless of data; otherwise the comparison uses
current C strings through `_stricmp`, with no stored-length tie-break. The
inlined comparison reads right data before left data and exactly matches the
existing actual `443D00` source used by cache lookup.

The last descent node remains the insertion parent. A separate actual8h
iterator is written node-first, owner-second. When the next insertion is
left of the current minimum, insertion happens immediately. Other left
positions decrement that iterator through actual `B7CDF0`, preserving its
returning invalid-parameter service. The candidate/predecessor is compared
against the input again. Insertion still uses the captured original parent
and original descent-side byte.

An equivalent key returns the existing candidate and inserted0, leaving its
raw resource pointer and all reference counts unchanged. A successful insert
returns the new node and inserted1. Both result paths store node+4, byte+8,
then owner+0, retaining captured fields across output aliases. Other result
bytes remain untouched. The equivalent path reads the current local iterator
owner; it does not replace that owner with a new projection or snapshot.

## Owning length-error payload and FH3

The native error branch initializes only capacity15, length0 and the first
inline byte of its legacy SBO temporary, then assigns exactly19 bytes from
`CE47BC`: `map/set<T> too long`. State0 is armed after assignment returns.
It constructs the `411700` logic-error owner, writes profile `D69260`, and
throws with `D83F98`. Those constructor/profile/ThrowInfo identities match
the existing `4CE780` length-error path, so its owning
`NativeAliasListLengthError` transport is reused with the recovered message.

Handler `CC2158..CC2161` selects FuncInfo `DFB398`: magic `19930522`,
maxState1, unwind map `DFB390`, no try blocks, flags1. The only map entry is
`state0 -> -1, CC2150`. Its complete 8-byte action executes
`LEA ECX,[EBP-50h]; JMP 4072D0`, destroying the completed message. There is
no node, key or tree rollback action. Failed message assignment is outside
this completed-temporary state; the existing string/exception dependencies
retain their own cleanup rules.

The source arms its message guard only after assignment and throws the
existing owning payload. Constructor failure inside `B7F6A0` retains that
integrated helper's captured-outer free/rethrow path. It never reaches this
routine's count/link/output stores. Resource ownership remains unchanged.

## Native transfers

| Site | Target | Classification / argument evidence |
|---|---|---|
| B7FFC3 | 408720 | CALL; message pointer,count19; callee RET8 |
| B7FFD5 | 411700 | CALL; ECX exception owner,stack SBO source; RET4 |
| B7FFEC | BF6885 | CALL; payload and D83F98; native throw |
| B80004 | B7F6A0 | CALL; initial head,parent,initial head,pair,color0; RET14h |
| B80095 | B7D5F0 | CALL; ECX tree,stack working node; RET4 |
| B800B3 | B7CBD0 | CALL; ECX tree,stack current grandparent; RET4 |
| B800E1 | B7CBD0 | CALL; ECX tree,stack working node; RET4 |
| B803F6 | BF7FBF | CALL; left/right C strings; B803FB ADD ESP,8 |
| B80438 | B7FF80 | CALL; iterator,1,parent,pair; RET10h |
| B8045F | B7CDF0 | CALL; ECX actual local iterator; no stack arguments |
| B8046F | 443D00 | CALL; candidate key,input key; RET8 |
| B80486 | B7FF80 | CALL; iterator,captured side,parent,pair; RET10h |
| CC2153 | 4072D0 | Tail JMP; ECX completed SBO temporary |
| CC215D | BF6B43 | Decoded-only tail JMP; EAX FuncInfo DFB398 |

The whole report verifier checks all13 supported CALL/tail-JMP rows. The
last transfer is retained separately: `CC2158` has no current Ghidra function.
Its 10 bytes and prologue reference are verified; a later primary annotation
batch may create the handler under its write lock. This worker applies no
Ghidra mutation, new name or shared metadata change. Live xrefs and argument
setup confirm the two InsertAt calls above and the unique insertion caller
`B80975` in the complete resource-loading body.

## Verification and remaining boundaries

Every live query used `bsp.py ghidra`, which verifies project `bsp`, program
`/battlestationspacific.exe`, x86 language and base `00400000`; the configured
project file is `C:/Users/sqz269/bsp.gpr`. Autostart was disabled. Eight full
body/support spans,866 bytes, match fresh live Ghidra and the installed PE.

The changed TU and ignored fixture passed MSVC Win32 `/std:c++17 /EHsc /O2
/MD /W4 /WX /fp:strict`. The newly compiled insertion object was explicitly
linked against a frozen copy of the primary BN library from
`J:/PROG/battlestations-pacific-decompile-orch4-20260910/build/win32/Release/bsp_core.lib`,
source commit `9673354d1a985c46c83a8226caa7a54e0290328b`, SHA256
`d9947ae6d8e55c2903641262080c16b543df87eb1519df37d932b1824a9030f0`.
Source/copy/source hashes agreed; all11 checked dependency source blobs match that
frozen commit. No main/primary build was run by this worker.

One actual-storage fixture passed successful/equivalent/zero-length keys,
both rotation families, red-black invariants after each insert, min/max/count,
output/source aliasing and preserved result padding. It also passed a current
head/count replacement during key allocation, including DWORD count wrap;
an owning copy of the exact length-error message surviving temporary unwind;
and node-construction failure freeing the captured outer node exactly once
without insertion count/link/result stores or resource reference changes.
Only the probe's own CRT imports/new handler are temporarily instrumented for
allocation observations/failure; they are restored before exit. The manifest
is embedded. No permanent test or production callback is introduced.

These checks establish the bounded source storage schedules and declared
host services. Original register/stack spill identity, hardware SEH/FH3,
legacy exception ABI, arbitrary invalid/overlapping tree layouts, concurrent
mutation, production integration and gameplay remain unproved. The fixture's
controlled field replacement is scheduling evidence, not a concurrency claim.
