# Native owned-string consumer readiness (CC12)

**Source 0 / ready Source 0.** Selected exactly one new consumer: `008F0340`, provisionally named `BSP_SceneProperty_CopyReferencePayload`. Its whole55-byte body is physically understood, and the admitted duplicate plus matching current free can bound a qualified normal domain over real payload storage. Root must first resolve the saved fall-through gap and register that qualified packet. Actual original-game reference-record ownership/destruction/private CRT/EH remains unclosed.

## Selection and scope

The bounded caller/ledger pass selected this unadmitted physical consumer of `00438E40`; its sole genuine direct caller is `008F0700`. Both were unleased before claim. The audit owns those two function addresses plus the caller's one type5 jump-table DWORD `008F07D8`, and only this document/report. Existing semantic `scene_property_bag_merge` code uses different interfaces and host methods; it does not admit the physical body. Type2 Source was freshly merged but not independently admitted in the frozen base; it receives no dependency credit. Type5 Source and external property-release audits are neither queried nor credited.

## Whole native body and saved-analysis gap

`[008F0340,008F0377)` is **55 bytes / 22 physical instructions**, SHA-256 `40611ad298e15a07908bead1b10f73434e3574529a027779139398e0f3edd3c6`. Original PE and fresh live bytes agree before/after. The saved function range ends at `008F0376`, but its listing contains only20 starts. The **10-byte gap `[008F035B,008F0365)`** immediately after `CALL 00BF6989` contains `83 C4 04` (`ADD ESP,4`) and `C7 46 04 00 00 00 00` (`MOV DWORD [ESI+4],0`). Current instruction-context reports no instruction at35B/35E; the read-only flow tool independently reports the gap. Those bytes fall through to the duplication path. The decompiler's early return after free is false.

Saved prototype is `undefined BSP_SceneProperty_CopyReferencePayload(void)`. An optional supported read-only flow-property query failed because script execution is disabled; its raw error/response is retained. The precise call-site override versus callee no-return annotation cause remains **unverified**. No settings were enabled and no mutation fallback occurred. A metadata-only recovery accepted the already complete byte/listing evidence without replaying successful queries.

Whole physical sequence: load source pointer from entry `[ESP+4]`; save EBX; snapshot source+4 text in EBX; save ESI/EDI; snapshot source+0 scalar in EDI; ESI=actual ECX destination; load/test destination+4; if nonzero push/free it, clean stack and clear destination+4; ECX=source text; duplicate; store scalar at destination+0; restore EDI; store actual copy at destination+4; restore ESI/EBX; `RET4`. There is **no literal phase/vtable/type constant** and no broader record store.

The only calls are `008F0356 -> 00BF6989` (CDECL old-pointer free), operand **[23,27)**, and `008F0367 -> 00438E40`, operand **[40,44)**. A future qualified Original/Source comparison would bind both to the same actual current free and admitted physical duplicate, preserving all other **47 bytes**. Original private `_free` is a named incomplete boundary, not executed or newly audited here.

## Actual ABI, return and ownership

Candidate interface: `char* __fastcall copy_native_reference_payload_008f0340(void* actual_destination_ecx, void* unused_edx, const void* actual_source_stack)`, without `noexcept`. Destination and source each expose two actual DWORDs (8 bytes). The one stack argument is source payload address, with `RET4`. EBX/EBP/ESI/EDI survive normal return. Full **EAX is the actual new copy or null**, not the payload receiver; the outer caller overwrites EAX with its own destination record.

Normal success writes exactly destination `[0,8)`: the snapshotted scalar then actual copied pointer. Nonnull old destination is freed and field+4 cleared before duplication. No other record field, ownership flag, phase word or kind is changed. On child failure, the old copy may already be freed/nulled while the scalar remains old; no strong failure or unwind guarantee is claimed.

The qualified current domain requires old destination+4 be null or a genuine sole-owned current allocation from the admitted duplicate, with a matching current canonical free. The source scalar is copied bit-for-bit and its nullable borrowed text stays readable through NUL with representable length+1/no wrap. Use genuine writable/readable payload storage disjoint from live frames, with source text disjoint from the old allocation. Simplest valid inputs use separate source/destination payloads and a separate text object. No semantic property class, fake vtable, provider callback, manufactured owned pointer or forced return value is needed.

The early snapshots do **not** make self-assignment safe: when source text equals a nonnull old destination allocation, free occurs before duplication reads it, causing use-after-free. Such self/shared ownership is excluded. The consumer frees only its old destination copy; caller owns the actual returned/new stored copy and must free it once through canonical free before payload disposal/reuse/lifetime end. Borrowed source ownership remains outside this function.

The current canonical implementation is `singleton_lifetime_free -> std::free`, with Source SHA-256 `97733266a44569114fca8142035375b728144e20658a6d914156b2a78655ec62`. Admitted physical duplicate Source is `48c86a1ad7763861d099d909a7a8b314b3258ee9b5508f702f093a64188946e0`, header `b101c630eba5844d1f5d21530aef1200c0969b0139fb72de1d3870e75814f477`. Frozen independent evidence establishes the current matching UCRT allocation/free and distinct VCRUNTIME memcpy components; fresh future executable/IAT/physical-file gates are still required for this candidate.

## Nested stack and flags

Let `T` point at the source-payload DWORD just before CALL. Entry=`T-4`; saved EBX/ESI/EDI=`T-8/T-12/T-16`. Optional old-free argument=`T-20`, free entry=`T-24`, and cleanup restores `T-16`. Duplicate entry=`T-20`; its saved EBX/ESI/EDI=`T-24/T-28/T-32`, size argument=`T-36`. Copy destination/source/length=`T-48/T-44/T-40`, copy entry=`T-52`, copy return=**`T-48`**, final `ADD ESP,16`=**`T-32`**. Child returns to `T-16`, outer `RET4` yields **`T+4`**.

Post-child instructions preserve arithmetic flags. Null text gives EAX0/ECX0 and mask `0x8C5` value `0x44`; AF is undefined/excluded. If old was nonnull, EDX may already have been changed by free, so do not assert incoming EDX even for null text. If old was null, no free intervenes and null-child EDX preserves incoming bits. Nonnull text flags under `0x8D5`, including AF, derive from **`ADD32(T-48,16)`**; ECX/EDX are volatile. DF0 is required for the current CRT; no whole EFLAGS/FP/MXCSR/segment claim.

## One genuine direct caller

Only the destination-type5 route of `008F0700` is admitted as a **witness**, not whole caller Source. Entry `[008F0700,008F0716)` is22B6, SHA-256 `001d543beac6ec2c944f3e52163e52d01ffc9cc1e0fbb4f73e8c188a3d28c8f0`. It saves destination ECX in ESI, reads destination+4, performs unsigned <=0xB check, then jumps via tablebase `008F07C4`. Cell `008F07D8` for code5 contains `008F0766`, before/after live/PE SHA-256 `e2d33cca6db7721a23284ec20175410a9014599dcc62a6df45cbeba9e0354cf0`.

Arm `[008F0766,008F077C)` is22B8, SHA-256 `ce28bf0d86601264cf86037fdd3f4c5a210490fcb14a3a00726dbd0360f5d82d`: load actual source record from `[ESP+8]`, add18h and push, form ECX=dest+18h, CALL at0771, then EAX=dest/POP ESI/RET4. The source's type is not checked in this arm. Only record+18h/+1Ch is copied; destination kind+08h and phase remain untouched. Native record construction, old pointer provenance and eventual destruction remain prerequisites outside this bounded witness.

## Conditional next packet and boundary

Proposed disabled packet `cc12_native_reference_payload_copy` owns only this55-byte candidate and four future Source/doc/report files listed in the JSON. Root should inspect/repair the saved35B/35E fall-through using locked supported tooling with original values retained, then register the genuine current-owned payload contract. No Source credit is conferred now.

A future fresh four-TU fixture should compile candidate + unchanged duplicate + canonical services + new ignored probe, derive complete COFF/map/code spans, and gate actual matching malloc/free/_callnewh UCRT plus distinct memcpy VCRUNTIME identity before zero entries and after. Bind Original's two operand ranges to those same actual services; never execute the original private free. Minimal coverage is three raw Original/Source pairs (oldnull/textnull, realold/textnull, realold/textnonnull) plus ordinary Source oldnull/textnonnull: **7 entries**, four real old-copy setups/frees and three new-copy observations/frees. Old copies must come from real admitted duplication, with no freed reads or required allocator-address equality.

This bounds a useful **qualified payload operation**, not original-game property lifetime. The native type5 producer `008EF2B0`, actual owner/destructor/EH and private heap provenance remain named incomplete dependencies. Current free cannot accept arbitrary original-game pointers. No full caller, constructor, release body, broad class or game admission is claimed. The explicit graph has **11 nodes + 11 edges =22**, including named incomplete boundaries, under the24 limit.

## Provenance and verification

Fresh private worktree base `3b2b12b14b0abd914f71c75ef46ad5e3d0e95876`. Family `J:\PROG\battlestations-pacific-decompile-cc12_native_string_consumer_readiness\local\cc12_native_string_consumer_readiness20261008a`. Four live/PE spans and saved starts are bookended;19 explicit inputs and991 older immutable artifacts retain their original hashes/associations. Older manifests and metadata remain historical; no accepted recipe/helper/process was rerun or old external association repinned. Every Ghidra batch verified the existing BSP project/program. Zero Source changes, builds, probes, native executions, Ghidra mutations or settings changes. The final family manifest covers the complete actual inventory, including the optional-query failure and metadata-only recovery.

## Root flow repair and registration

Root verified the immutable64-file readiness and23-file repair-proposal families, plus1055 older pins. Locked tool cleared the observed CALL_RETURN at008F0356 and decoded10 bytes, restoring the complete22-start listing. Name, prototype header and complete documentation stayed exact; derived CFG edge count changed2 to3 as expected. The cached signature fingerprint count20 is historical; physical listing and refreshed export supply the complete22 instructions. Saved project and forced export verified, whole live55 bytes match installed PE. The raw current-owned payload packet is now registered; Source0/privateCRT/class/game remain unadmitted.
