# Property record candidate 008EF1B0: blocked string-copy readiness

**Zero ready Source packets.** `FUN_008ef1b0` is a complete **60-byte / 20-instruction** initializer at `[008EF1B0,008EF1EC)` that stores tag `2` and calls a string-duplication service. It is not a call-free integer or scalar storage leaf. No Source/header/config/CMake changes, compilation, fixture, native execution, Ghidra mutation or admission credit were added by this audit.

The descriptive interpretation is a hypothesis supported by actual instructions and one caller arm. The saved `undefined FUN_008ef1b0(void)` prototype and decompiler's unary fastcall signature omit the physical stacked argument. They were left unchanged.

## Whole entry and physical contract

All 60 live bytes equal the installed PE, SHA-256 `be944adca54cbd0702bc89a12e88ab0c8ac84f306bb8c14188266b8f70ae5a04`. All 20 saved instruction starts match a complete linear decode, with zero interior or tail gaps. The installed executable SHA-256 is `b682a82c52f81f957b2c70222077305a933f72481686c88843077f714b956dd6`. The supported read-only gateway verified `C:/Users/sqz269/bsp.gpr`, `/battlestationspacific.exe`; all inspected live-byte spans matched again afterward.

ECX supplies the actual destination. The original entry stack word at `ESP+4` is a source string pointer: after `PUSH ESI`, the function loads that word from `[ESP+8]` into ECX for `CALL 00438E40` at `008EF1D6`. Incoming EDX is unused locally. Normal return places the actual destination in EAX and uses `RET 4`.

The body requires at least 56 writable bytes and performs these stores in order:

| Order | Destination offset | Width | Value |
| --- | --- | --- | --- |
| Before duplicate | `00` | DWORD | Literal `00CE89D4` |
| Before duplicate | `04` | DWORD | `2` |
| Before duplicate | `18`, `1C`, `20`, `24`, `30` | DWORD each | `0`, in that order |
| After normal duplicate return | `34` | DWORD | `0` |
| After normal duplicate return | `0C` | DWORD | Actual returned duplicate pointer, or null on the native null-source path |
| After normal duplicate return | `2C` | BYTE | `1` |

Normal return writes 37 bytes and preserves 19 within the minimum 56-byte storage: `[08,0C)`, `[10,18)`, `[28,2C)` and `[2D,30)`. It does not write a scalar payload at `+08`. The post-call stores are not established on an allocation/exception failure path.

The candidate explicitly saves/restores ESI and EDI. EBX/EBP are untouched locally; downstream nonvolatile preservation remains a normal-call dependency. ECX/EDX are provider-defined volatile outputs for a nonnull string. On the native null-source path, the inspected duplicate helper leaves ECX zero and incoming EDX unchanged. No universal zero/preservation contract is inferred.

The candidate performs no arithmetic after duplicate returns. Null-source duplication ends with `XOR EAX,EAX`, so CF/OF/SF are clear, ZF/PF set and AF undefined. Nonnull normal duplication ends with `ADD ESP,10h` after allocation and copying, so that actual stack addition supplies the final arithmetic flags. The candidate's earlier `XOR EDI,EDI` is not a final-flags guarantee. FP/DF/segment and exception equivalence remain unadmitted.

## The actual duplication boundary

`00438E40` is whole **57 bytes / 30 instructions**, SHA-256 `28510a4c8a423997dfca7d8e257c1b250a7bb9e79cce177fc9c501e7b64735c4`, with no saved-listing gaps. Its physical contract is source pointer in ECX, no stack arguments, and `RET 0`. Null source returns null. Nonnull source is scanned through its terminator; the helper requests `strlen+1`, copies that many bytes and returns the actual allocation.

The apparent allocation-target discrepancy is resolved: `CALL 00BF55BE` at `00438E61` reaches a verified five-byte `JMP 00BF681B`. The ledger's direct target and the header/decompiler's terminal target describe two levels of the same path. The second direct call, at `00438E6B`, targets `_memcpy` at `00BF7680`. The terminal allocator and memcpy bodies, private heap and EH behavior were not opened as a new closure.

Current `src/native_string.cpp` implements `duplicate_00438e40` using `std::strlen+1`, `std::malloc`, immediate `std::bad_alloc` on a null allocation and `std::memcpy`. Its separate release helper uses `std::free`. The header declares a plain C++ API without an explicit fastcall attribute. A reconstructed/backfilled ledger label does not prove the physical ECX/RET0 call binding or a matched native allocation/copy/release domain. The header's historical `00BF9DC8` release association is retained as an unverified boundary, not asserted as a newly inspected native call edge.

Before this constructor can become a Source packet, a genuine physical duplication provider and its matching owned-buffer release relationship need separate admission. That admission must explicitly bound current versus original allocation/retry/EH behavior and establish real caller/provider identity. This audit does not invent an adapter, default result, null-only fragment or callback to manufacture readiness.

## Phase identity and one caller witness

The actual data cell at `00CE89D4` contains `004E6730`; the candidate stores the literal cell address without making a virtual call. Those original identity bits do not establish host vtable substitution, native class dispatch, ownership or destructor behavior.

Only one genuine Clone arm was inspected: `[008F5083,008F50B1)`, **46 bytes / 13 instructions**, containing `CALL 008EF1B0` at `008F5099`. It moves `[ESI+0C]` into EDX, pushes that actual word, and sets ECX from destination EAX. No numeric conversion occurs in the inspected arm. After return it copies `[ESI+34]` into `[EAX+34]` and restores caller state including the FS frame. Earlier destination allocation, source ownership, the rest of Clone and all other arms remain uninspected.

The bounded native evidence graph is **9 nodes + 7 edges = 16/24**. It includes named incomplete terminal allocation, memcpy, destructor/dispatch, whole-Clone and historical release boundaries. The current Source API comparison is separate evidence, not an established replacement edge. No live query targeted active `008EF170` or `004845A0`.

## Immutable receipt

Family: `J:/PROG/battlestations-pacific-decompile-cc12_property_record_scalar_readiness/local/cc12_property_record_scalar_readiness20261008a`, frozen at `ccdd7afbe1a4381089100af74aa51cdc33ab2637`.

The manifest covers all **43 actual family artifacts**, excluding only its own exact output path. It includes the initial orientation results, initial old metadata record, both exact historical byte/float report copies, all captures, helpers and receipts. Old embedded Source/hash associations remain frozen; none were repinned against subsequently changed main. All current inputs, historical copies and five native spans passed before/after checks. No old successful helper, recipe or body was replayed or changed.

Proposal SHA-256: `ef7cbed7ee3343bf635302af69a55770698302cfa36faaf1a6c2546613ab0f81`. Receipt: `5d1ee0094665bde43286688cee541bbdddac974abf29275ff51a8f9cc23802db`. Manifest: `3b08327763a8dd2b35078fbcaa1052501cfc1cbf7790b272a3033542de00fa1b`.

No future four-file Source/API/fixture packet is proposed as ready. Root owns any subsequent provider admission, registration and integration. Native class/parent/world ownership, original private CRT/EH, startup and gameplay remain unestablished.
