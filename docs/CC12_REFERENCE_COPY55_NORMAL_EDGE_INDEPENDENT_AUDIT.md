# ReferenceCOPY55 normal-edge independent audit (Source0)

The sealed Root `local/ref55p3` fixture has **no missing retained normal TU body or actual referenced local normal helper**. This independent raw audit confirms all **24 normal TU bodies and 12 normal helpers** in the unchanged **38 packed gate spans**. The existing **Source1 qualification for the recorded normal-success domain remains unchanged**; this audit adds **Source0, ready0 and registration0**.

The earlier metadata-only audit identified six omitted cold std bodies. Reading the saved four COFF objects, linker map, PE bytes and packed gate independently confirms those six and adds the transitive cold delete/std helper omissions described below. Cold-only omissions do not establish a normal-path defect.

## Raw body and relocation evidence

| Object | Code definitions | Retained physical bodies | Unmapped definitions |
|---|---:|---:|---:|
| consumer.obj | 1 | 1 | 0 |
| duplicate.obj | 2 | 2 | 0 |
| canonical.obj | 67 | 9 | 58 |
| probe.obj | 32 | 19 | 13 |
| Total | 102 | 31 | 71 |

All 33 executable TU map rows resolve: **31 physical bodies and two weak aliases**. The parser considers executable type32 functions and code-only type0/EH entries; this actual link has no additional mapped code-only handler. Six storage-class6 `$LN` symbols are interior labels. The 71 unmapped definitions are not assigned invented live addresses.

The independent raw reconstruction checks **271 per-symbol relocation operands, corresponding to 265 unique physical operands**. Each weak alias contributes three additional symbol checks over the already checked physical body. Both `??_Eexception` and `??_Ebad_alloc` have actual raw characteristic **1 = NOLIBRARY**, not characteristic3. Their auxiliary TagIndex chains select defined `??_G` fallbacks at `34001190` and `34001160`; each complete 45-byte body, all three relocations, same map address and all remaining bytes agree. Cycles, auxiliary-record indices, unsupported modes and unresolved ownership are rejected.

Every packed span equals the saved PE bytes, recorded full extent, instruction stream and SHA256. `_main` is **5,279 bytes / 1,295 instructions / 163 relocations**, fully gated. Normal retained TU bodies account for 249 physical relocations; all 25 gated TU bodies account for 251. No normal body or helper is accepted by a prefix match.

The normal local helpers are the cookie14 and stack43 bodies, memcpy/memset thunks, K32GetMappedFileNameW and all seven referenced BCrypt thunks. Every thunk is the complete six-byte `FF25` transfer through its actual saved PE import slot. All 52 direct TU IAT operand references across 27 distinct import names are also recorded. Register-indirect allocator calls retain their actual malloc/_callnewh IAT loads. External DLL implementations and CRT startup remain outside this body inventory.

## Cold omissions and failure frontiers

| Ungated local body | Address | Bytes |
|---|---|---:|
| exception copy constructor | 34001110 | 42 |
| exception::what | 34001230 | 14 |
| exception scalar deleting destructor | 34001190 | 45 |
| bad_alloc destructor | 34001140 | 17 |
| bad_alloc copy constructor | 340010C0 | 48 |
| bad_alloc scalar deleting destructor | 34001160 | 45 |
| sized operator delete | 340037E0 | 16 |
| operator delete | 34003B40 | 5 |
| free import thunk | 3400453C | 6 |
| std exception destroy thunk | 34004518 | 6 |
| std exception copy thunk | 34004512 | 6 |

The six TU bodies total 211 bytes; the five additional local helpers total 39. The actual cold delete chain is **sized delete16 -> delete5 -> free6**. The six omitted TU bodies have incoming object references only from non-code vtable/EH/debug metadata. This is distinct from a missing normal execution edge.

The complete allocator body separates successful allocation at `340011DA JNE340011FA` through `34001200 RET` from the failure branch `340011EC JE34001201`. That branch invokes the already gated bad_alloc default constructor and the already gated local CxxThrowException thunk. The DLL throw/EH/OOM destinations, unwind and cold destruction remain unadmitted.

The gated stack helper has its real 43-byte CFG: an earlier RET at 34003841 and a final internal JMP at 34003849 back to 34003834. It is not truncated at the earlier RET and is not required to end in RET. The complete cookie14 helper returns normally at 340037D8 and has a failure tail at 340037D9 to `___report_gsfailure` at 34003B30. The next-map-owned 16-byte window contains `MOV ECX,2; INT29; RET` (8 bytes), then eight CC padding bytes. This named cold failure sequence is ungated; its interrupt/failure behavior remains unadmitted and is not treated as an ordinary successful helper CFG.

## Recorded Source and ownership domain

The consumer at original 008F0340 remains **55 bytes / 22 instructions**, uniquely matched in the saved PE at 34001000. Only CALL operands at 23 and 40 differ; all 47 other bytes match the sealed Original. The input pointer is snapshotted at 34001005 and input tag at 3400100A, before old canonical free at 34001016. Current duplicate57/30 is called at 34001027. Whole EAX is the new owned copy or NULL, then is stored into the destination pointer; it is not a Boolean or destination-root return.

The actual Win32 fastcall boundary has ECX destination, one stack source pointer and `RET4`. The full raw caller140/52 and ordinary caller18/5 are gated. Six archived 80-byte captures are independently re-encoded from their recorded DWORDs and checked: same paired Original/Source frame, actual argument slot/value, ESP cleanup, preserved nonvolatile registers, whole EAX result and defined arithmetic flags. Nullable XOR flags exclude undefined AF; nonnull ADD flags are recomputed from the recorded stack address. Only old-null/new-null claims incoming EDX preservation. Input DF0 through CRT is the qualified domain; no blanket FP or ES claim is added.

Archived execution consists of **three raw Original calls, three raw Source calls and one ordinary Source call**. Counts agree: zero target/setup entries before provider gates, four real current-duplicate old setups, four old frees, three returned allocations, three result frees and seven total canonical frees. The old 8-byte NUL-terminated setup and new 9-byte copy are preserved; copied child bytes are `D9 34 8E C7 5A F1 26 B3 00`.

The four actual 48-byte source/destination/borrowed blocks are pairwise disjoint; current owned old/new allocations are disjoint from all four. The saved full before/after destination48 and source48 bytes preserve all D2 guards and expected payload words. Old allocations are never read after their target calls, and old/new address reuse is permitted. Full borrowed-buffer stability was checked by the original gated probe, but those two complete borrowed48 buffers are not separately serialized. This audit does not invent a broader alias domain or a new memory sample.

All four archived provider records agree with the packed gate and static records on I386, IAT/export/base+RVA, NT mapped/opened paths, file identity and full SHA256. Three 32-byte prefixes agree literally. `_callnewh` differs only in its saved FF15 absolute operand at 24, consistent with the recorded relocation; the implied preferred base is an inference from those archived values, not a new provider-header query. The accepted process's physical bookend evidence is preserved; **no new live provider attestation occurred**.

## Preservation and audit limits

Before/after hashes match for **22,442 unique immutable artifacts** across 19 explicitly sealed families. These include ReferenceCOPY55 Root101 and prior18,016, latest callback Root419 with base21,343 plus468 additional records (deduplicated21,683), corrected Type7/8/9 drafts17/19/19, and the older 13-file peer audit. Current Main reports are copied as historical metadata; no live Main report, ledger, config, documentation or CMake path is added to strict old pins.

An initial collection check confused the callback's combined 21,683 total with its base prior count 21,343. The stop is preserved in `preservation_attempt1_stop.json`; the corrected collection verifies the additional inventory independently. Both new raw/observation parsers pass. A later inline metadata writer stopped at Python parsing because it used the reserved keyword `return` as a dictionary keyword; no statement ran or output was created. Its stop is preserved, and the separate new writer uses `return_value`.

Five uninitialized COFF sections have zero raw-file pointer and zero actual raw bytes; the old COFF JSON included 24 total header-derived bytes there. That discrepancy is confined to uninitialized data and adds no normal code-gate gap.

Only this document and its companion report are tracked changes. No C++/HPP, CMake, shared metadata, ledger, saved analysis, credit or registry was changed. No compiler, Native/provider/target operation, accepted-helper import/replay, Ghidra analysis/mutation or test was run. This audit supplies raw artifact and recorded-observation consistency evidence; it does not supply new execution, universal ABI compatibility or game validation. Root decides whether a future correction should widen the cold gate scope.

The companion report is `reports/cc12_reference_copy55_normal_edge_independent_audit.json`. Raw proofs, parser sources/logs, preserved stops, before/after preservation and the final recursive seal are in ignored `local/ref55edge_audit20261008a` in the named audit worktree.
