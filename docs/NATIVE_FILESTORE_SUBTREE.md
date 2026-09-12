# Native FileStore subtree destruction and iterator advance

Addresses: `00BE5D70`, `00BE66C0`, `00BE6720`, `00BE4C30`, `00BE4E40`,
`00CC6C40`. Packet `orch2_filestore_subtree_ar` starts at `b44ac4fa`.
Names are descriptive hypotheses. The report is
[native_filestore_subtree.json](../reports/native_filestore_subtree.json).

The five actual-storage routines are reconstructed in
`src/native_filestore_subtree.cpp`, with the payload's string unwind action.
The interfaces add explicit services and do not claim original binary ABI.

| Entry | Final instruction / exclusive byte endpoint | Coverage | Original ABI |
| --- | --- | --- | --- |
| BE5D70 | BE5DEE / BE5DEF | complete normal body and one-state unwind | ECX actual 0Ch payload; RET; no semantic result |
| BE66C0 | BE670F RET4 / BE6712 | complete including raw 11-byte continuation | ECX tree; stack node; RET4 |
| BE6720 | BE675A RET4 / BE675D | complete including raw 11-byte continuation | ECX tree; stack node; RET4 |
| BE4C30 | BE4C92 / BE4C93 | complete | ECX actual 8-byte iterator; RET |
| BE4E40 | BE4EA2 / BE4EA3 | complete | ECX actual 8-byte iterator; RET |
| CC6C40 | CC6C43 JMP / CC6C48 | complete unwind action within payload guard | captured payload at EBP-10; tail to 41DD20 |

BE5D70 captures payload+8, arms state0, and uses real `InterlockedDecrement`
on captured owner+4. Only zero loads that owner's current vtable and current
slot0. BE5DB1 calls with ECX=captured owner, EDX=captured table and EAX=entry;
there are no pushed arguments. Returning dispatch clears the **current**
payload+8, even if a callback replaced it. A captured null skips that clear.
The normal tail captures current data+4, disarms state0, then releases using
current length+1 with DWORD wrap; string header fields remain untouched.

The EH handler at CC6C48 selects FuncInfo E0146C. The one-entry unwind map at
E01464 is `{previous=-1, action=CC6C40}`. Its eight bytes load the captured
payload from EBP-10 and tail-jump 41DD20. Consequently a throwing zero-reference
target leaves payload+8 as it currently stands but releases the current string.
It neither decrements again nor frees the enclosing node. The C++ guard covers
that source exception path. Existing `NativeStringStorage::release` is noexcept:
the native lazy pool getter's throwing behavior and original FH3/SEH exception
identity remain outside the interface's domain.

Both erasers recurse into current right+8, then capture left+0, destroy the
current payload, free the current node through the shared source CRT service,
and iterate the captured left. BE66C0 captures data+10 **before** left+0 and
loads length+C only for nonnull data. BE6720 delegates the resident payload+C
to BE5D70. They leave tree owner words/head/count unchanged and preserve the
sentinel. There is no node rollback on a throwing resident terminal.

Ghidra currently omits BE6701..BE670B and BE674C..BE6756 after the free calls.
Live bytes equal disk bytes. Each raw continuation is `ADD ESP,4; CMP byte
[ESI+19],0; MOV EDI,ESI; JE loop`. These are necessary loop instructions,
not new functions; `no_ghidra_function` is empty. No Ghidra mutation was made.

Both iterators implement the same successor behavior over the existing
`NativeFileStoreNameIterator` layout. A null tree calls returning BF6713 and
then reloads the current node. A nil current node tail-jumps BF6713 and returns
without retrying. Otherwise descend to the right subtree's leftmost node, or
climb parents while the **current iterator+4** equals parent+8. Each parent
climbed is written to iterator+4 before continuing. Owner+0 stays untouched.
CALL rows and tail transfers are recorded separately.

The layouts come from producers, including BE7FA0's sentinel self-links and
nil marker, BE55E0/BE5690's actual 1Ch allocations, BE63E0's three link stores,
payload+C copy, color+18 and nil+19 writes, and BE6250's actual length/data/
retained-owner copy. The source adds no alternative node or tree ownership.
All live incoming call contexts were inspected. The snapshot's extra BE63E0
edge to BE5D70 is stale: the live complete BE63E0 body contains no such CALL.
CC6DBB is a tail JMP despite the xref response calling it a CALL.

`NativeAdoptedSubstreamDispatch::source_zero_reference` is the exact current
CALL boundary. Existing concrete `NativeVfsRuntimeBindings` supports these
live-verified numeric profiles without replacing original table words:

| Current profile | slot0 | Actual next dispatch |
| --- | --- | --- |
| D642C0 | BD30E0 | reload current slot4 BB8F90, flags1 memory-stream deletion |
| D68DB0 | BD30E0 | reload current slot4 BF1240, flags1 adopted-substream deletion |
| D691B0 | BF55A0 | current slot4 flags0 destruction, then actual pool recycle |

The callable adapter preserves ECX and EDX for externally supplied actual ABI
entries. Other numeric profiles or zero-reference targets require their own
concrete binding. This packet does not reduce those targets to a no-op and does
not claim that arbitrary FileStore residents are all memory streams.

The strict Win32 build and both existing CTests pass; all 11 direct CALL rows
pass live verification. The focused probe passes 17 original/source comparisons
(three payload cases, four mutation-sensitive tree cases and ten iterator cases)
plus source-only throwing cleanup. Eighteen source/header/object/archive/probe/
command artifacts were frozen before execution and remained hash-identical after.
Validation and frozen artifact hashes are retained in the companion report.
The ignored differential probe uses copied original bytes with only explicit
dependency and import relocations, real Windows atomics, real native string
pool storage, and shared source CRT allocate/free. Original EH handlers are
not executed; the throwing cleanup check is source-only. Full owner startup,
arbitrary stack aliasing, concurrent mutation, original CRT exception identity,
game execution and native binary replacement remain unproved. No tracked tests
were added. The next dependencies are full BE6760/BE6A20 erase/rebalance,
BE7580/BE7690 range operations, BE7A70/BE7BB0 wrappers, and owner composition.
