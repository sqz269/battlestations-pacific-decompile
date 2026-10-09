# Raw World base constructor readiness

**Actual `004CB030` Source admission remains unready.** This read-only audit closes the constructor's bounded byte, array and construction-cleanup contracts, and identifies the concrete missing owner/table/lifetime bindings. It adds no Source, build, fixture, Ghidra mutation or reconstruction credit. World, full-game and startup readiness remain false.

## Actual owner and constructor

The owner is the same **0x4BC-byte allocation** established by the complete accepted `004DE610` caller (5,357 bytes / 1,370 instructions). Allocation `00BF55BE` and free `00BF6989` are complete five-byte jumps to `00BF681B` and `00BF65AC`; the underlying qualified host allocation boundary remains applicable. The caller memsets the allocation, passes the same pointer to `004CB030`, stores its identical EAX result in `Game+19CC`, then passes it as ECX to `009037F0`. There is no separate 4A8-byte base allocation.

The complete constructor is **126 bytes / 39 instructions**. ECX is the actual borrowed owner; no stack arguments are consumed and RET is plain. EAX returns the same owner. EBP/ESI/EDI are preserved. It installs `00CE7784`, zeros the three root DWORDs at +0C/+10/+14, then calls the actual vector constructor with **base World+18, count0x61=97, stride0x0C**, constructor004B7EC0 and destructor004C2D30. Thus the array ends at +4A4, and the final DWORD+4B8 ends at +4BC.

After the array succeeds it calls `004C3080` with ECX=World+4B0 (ignored by that leaf), stores its real sentinel pointer at +4B4, zeroes +4B8, and only then sets ready BYTE+4AC=1. It leaves +4/+8, +4A4/+4A8, iterator word+4B0 and padding untouched; their initial zeroes come from the caller's memset. The separate108-byte sentinel has only its first two self-links initialized; its other bytes are not zeroed by the World memset.

## Construction exceptions and array callbacks

Native `00C65651` is a complete10-byte/2-instruction handler stub: load FuncInfo00D8DE60 into EAX, then jump to `00BF6B43` (CxxFrameHandler3). Its complete36-byte FuncInfo has magic19930522, two states, unwind map00D8DE50, no try blocks and EHFlags1. The two physical unwind entries are:

| State | Next | Actual action |
|---:|---:|---|
| 0 | -1 | C65630, 11B/3: recover this, add0C, tail to4C2D30 |
| 1 | 0 | C6563B, 22B/8: destroy97 elements atthis+18 with stride0C and4C2D30 throughBF7C6E |

State0 is written before the array call; state1 is written before sentinel allocation. An element-construction exception first unwinds only completed array elements, then the constructor's state0 root. Sentinel allocation failure after all97 elements succeeded destroys all97 in reverse, then the root+0C, and propagates. No full World destructor is called by these actions.

The complete Native vector constructor77B/22 and its finally24B/8 preserve the completed-prefix count and call `__ArrayUnwind` on failure. The complete destructor iterator75B/20 and its finally24B/8 reverse the actual remaining range. `__ArrayUnwind` is94B/27 including all44 bytes omitted from the Ghidra listing: its filter recognizes exception codeE06D7363 and calls terminate00C07A75; its SEH handler/epilogue are retained. Three complete28-byte scope tables physically identify the filter/finally targets and cookie offsets. Full Native CxxFrameHandler3, terminate and SEH prologue/epilogue semantics are explicit external prerequisites, not newly reconstructed here. Installed Microsoft EH headers are schema references only.

Current `construct_native_game_array_00bf7cd1` supports exactly three other callback/stride pairs. World4B7EC0/4C2D30 is rejected before operation state changes. Its failure state retains progress for external diagnostic cleanup. Likewise `destroy_native_game_array_00bf7c6e` rejects destructor4C2D30 and does not provide the original unwind protocol. These complete Source bodies and their current physical1023B/259 and711B/210 functions are retained; their convenient interfaces do not close the World contract.

## Real providers and remaining ownership

| Dependency | Current evidence and limit |
|---|---|
| Header4B7EC0 | Actual13B/6 primitive and unique current archive member; initializes caller storage and allocates nothing. Its independent standalone gate remains reopened. |
| Callback4C2D30 / clear4BF8E0 | Actual5B tail and70B/29 clear, positive symbol-index edges to canonical free. Callback admission still depends on clear70 regate. |
| Sentinel4C3080 | Actual26B/11 plus58B/20 adapter, unchanged from the preceding complete worker build proof. Whole allocator/RTTI/GS/CRT origins are retained. Primary acceptance is a separate authority. |
| Current World table | Absent. The verified Native16-byte prefix points to4CB0B0,904390,9035D0 and904BF0. Raw9035D0 is MOV AL,1;RET, with no new function/name credit. Address-valued layout metadata is not a current function table. |
| Actual owner / caller | `WorldConstructHost::create_world` and `world_post_construct` remain pure virtual. The Source constructor returns a projection with sentinel1, not raw storage. No concrete production host implements that pair. |

Existing host storage is described precisely: `GameUnitsHost` owns97 coherent12-byte category headers and stable nodes in its own C++ member, explicitly outside a raw4BC World. `GameWorldHost` builds index vectors for the hierarchy chains. Real Game/Dyn owners elsewhere are separate and are not claimed absent. None supplies this owner, actual method table, post-initialization or destruction protocol.

Legacy `WORLD_DEFERRED_DESTROY.md` infers a fifth World slot from the next code pointer atCE7794. Current actual GameParticipant constructor/destructor Source independently installsCE7794 as its own vptr. This audit qualifies the four explicit prefix entries and leaves exact full table extent/ownership open; code-pointer adjacency alone does not admit another World method.

## Complete normal destruction and the partial-owner trap

The actual scalar deleting destructor4CB0B0 is30B/11, including the three bytes missing after its free call. It calls904C40, conditionally frees the same owner on flags bit0, returns that pointer and uses RET4.

The normal destructor904C40 is **522 bytes / 182 instructions, through RET904E49**. Ghidra currently stops its body at904E10 after a falsely nonreturning free and omits26 instruction starts overall. Read-only live memory and immutable PE bytes recover the complete continuation; the project was not repaired. The tail frees the matrix sentinel, clears+4B4, destroys all97 category headers throughBF7C6E and clears the root+0C before restoring the exception chain and returning. Padding follows at904E4A..4F and the next routine starts904E50.

Earlier phases set ready/global bytes to0, visit both hierarchy chains through real entity callbacks, run three zero-delta entity/matrix passes with global flushes, free the two hierarchy headers, release the refcounted pointer+4A8 through actual InterlockedDecrement and a dynamic terminal, then free matrix nodes and their sentinel. Exact sites and unclosed providers (926D90,874D00,922FD0, entity vslot+DC,904600, captured-object vslot0, destructor EH CA54CF and caller EH C671A0) are named in the report. No empty substitutes are admitted.

Crucially,904C40 **dereferences World+8 before any null guard**. The base constructor leaves +4/+8 at the caller's zero values; only subsequent9037F0 supplies the two actual headers. Therefore base construction alone is not a safely destructible full World. A generic full-World destructor is invalid cleanup for constructor failure or incomplete post-initialization. The original constructor's specific reverse-array/root cleanup must stay separate; post-init second-allocation failure and caller rollback remain their own unclosed contracts.

## Admission path and evidence

A future concrete implementation must receive actual fresh4BC storage, install a real current method table, invoke the admitted header primitive over the exact97 slots, preserve the two construction-cleanup states, call the real sentinel producer and publish its actual pointer/count/ready fields in order. Normal destruction and actual Game publication/post-initialization require their named real providers before full owner admission. No constructor Source is proposed for immediate registration from this audit.

The scoped audit of commit `add42746b7ec0b52862756d42866d5eb309c1217` and all five tracked follow-up outputs found no18-count/+390 mistake. The tracked files already specify97 headers at+18, stride0C; `GAME_WORLD_CONSTRUCT.md` also already appends the correction to its historical callback reversal. The initial packet brief alone required correction. No shared document was edited.

Evidence retains15 bounded Raw bodies (1,033 bytes /335 instructions), six complete data blocks, accepted whole-caller authority, whole current Source files, eight complete physical objects and their eight unique historical core members. Existing objects are historical artifacts, not a new compilation or fixture. Root/worker sentinel Source compares equal after newline normalization; LF/CRLF differences are not semantic edits.

Archive: `local/cc12_raw_world_base_constructor_readiness_evidence.zip`, 35,420,204 bytes, SHA-256 `ae9d6e3a35c7d4ca810a4b489f92118400f28e0ad50a73ff1d34d46c91777389`. Manifest SHA-256 `ee673e4b79e8d00501571cf62dd9c412201b1448af1c2db8587c3df235eaa439`. All 216 payload hashes and 217 ZIP entries/CRCs passed. Tracked summaries remain outside the archive to avoid a checksum cycle. The capture, physical-artifact and verification scripts passed on their first invocations. No Source, CMake, ledger, Ghidra-program mutation, build, test, probe or game execution occurred.
