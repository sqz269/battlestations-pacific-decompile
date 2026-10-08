# Property replacement release child readiness

Whole `008F0DE0` is **NOT_READY** for Source. Its first missing whole callee is `008F0640`, the actual per-type record release. The new raw storage, string duplicate and array release leaves do not establish the nested type-6 destruction chain. No Source implementation is proposed from this child alone.

This read-only packet owns only `008F0DE0` and this document/report. It began from the clean property worktree, merged current main `d56acc6aa8a4d4698931b03c5c8bd83a1b3daa78` (worker merge `8ad4e14f1968d55a747722b0a7d58e1107b2837c`), and acquired an eight-hour lease. The publication/lookup Source stream and its pending Root review remain untouched. No new native body other than the selected child was expanded; existing release/producer evidence was reused and rehashed.

## Complete native child

The entire `[008F0DE0,008F0DEB)` body is 11 bytes / two instructions, with installed offset `5180896` and SHA-256 `4abd74aaaada8f496d594975c578825bfab9804569fe66790cbdd27d80560e5b`. Complete live bytes, listing and saved pseudocode agree with the installed PE:

```text
008F0DE0 C7 01 D4 89 CE 00  MOV DWORD PTR [ECX],00CE89D4
008F0DE6 E9 55 F8 FF FF     JMP 008F0640
```

ECX is the same actual live writable `38h` record. This child does not adjust the receiver, allocate, read the type, consume stack arguments, save registers or return locally. It writes only the profile DWORD at `+0`, then tail-transfers the unchanged receiver, stack and original return address. MOV/JMP themselves preserve registers, flags, DF and floating-point state; the tail callee and any nested destruction retain their own effects. EAX is unspecified by the whole release contract. The stored `undefined CG_adjustor_thunk(void)` prototype and adjustor role need Root annotation review. No listing repair or x87 recovery is required for these two instructions.

Actual profile DATA `00CE89D4` contains slot0 `004E6730`. This is native phase identity, not an invented Source vtable or callable Source global. The raw storage leaves below preserve that literal while expressly excluding dispatch. A caller must supply a valid owning record preimage and permit its destruction-phase transition; fresh unowned raw storage is insufficient.

All three incoming transfers are pinned: `004E6733` and `004EE421` are E8 calls; `00C67F33` is an E9 tail JMP. Live xrefs classify the last as `UNCONDITIONAL_CALL`; Root should review whether an intentional call-return flow override exists. The actual tail encoding is retained, and no unwind body was expanded or modified. The selected child's sole transfer is the E9 tail at `008F0DE6`. Four explicit native transfer rows are checked against complete live containing listings.

## What current providers close

| Current genuine provider | Closed domain | Remaining limit |
| --- | --- | --- |
| Type1 `008EF170`, 53B/15 | Integrated physical supplied-storage leaf; exact uint32 payload bits, tag1, phase and partial writes; no calls | Fresh unowned storage; caller conversion, record allocation/owner and class destruction unbound |
| Type4 `008EF230`, 56B/16 | Integrated raw two-DWORD storage, tag4, declaration identity `+28` and value `+C`; no calls | Declaration/enum ownership and whole record lifetime unbound |
| Raw duplicate `00438E40`, 57B/30 | Current ECX/RET0 Source exists; sealed fresh component qualifies null or successful current-heap n+1 allocation and real memcpy including NUL | External main integrator still holds its lease and independent registration/full-main gates; whole type2 record producer/lifetime, private CRT/EH/failure remain unbound |
| Array release `008F03F0`, 39B/13 | Integrated actual `record+20h` pointer/byte-size header release with genuine current-heap free and exact clear ordering | No surrounding record/type dispatcher, array producer or nested lifetime closure |
| Canonical current heap | Unchanged actual malloc/free Source, full production allocator90B/37 and free6B/1 retained | Historical heap, class scalar flags, failure/unwind and recursive owning release unbound |

Seven complete current production COFF bodies and ordered relocations are retained with their whole objects. Type1/type4 bodies equal their complete installed bytes; array release and raw duplicate preserve all nonrelocation bytes. Raw duplicate calls the genuine concrete 58B allocation adapter and `_memcpy`; the adapter calls actual canonical allocation and its named cookie helper. Canonical allocation imports malloc and names the existing new-handler/bad_alloc/throw path; those cold helpers are explicit, unexpanded dependencies outside the normal successful contract used here. Canonical free tail-imports free. Actual I386 UCRT/VCRUNTIME DLLs and selected export identities are copied for static provider identity, without executing a provider or replaying an old fixture.

Historical release audit text saying raw array release or physical duplicate is absent is superseded by current Source, current primary integration fields and retained production objects. Historical worker pending statuses for type1/type4/array are likewise qualified by their current primary admission fields. The external raw duplicate packet's remaining gates are kept explicit.

## Exact unresolved whole lifetime

The prior sealed whole `008F0640` remains 158B/63, SHA-256 `a04fa16e42a48885b025ec9be4ec3683b50db6cac380047a8ae0db44cc590026`; its installed bytes were rehashed without expanding it again. It receives the same record in ECX, no stack arguments, preserves ESI/EDI and uses plain RET. It dispatches actual `record+4` through the pinned selector/target DATA.

Type2 optionally frees owned `+C`; type5 optionally frees owned target `+1C` and clears `+18`; types8..B call the now-genuine array release on `+20h`. Type6 loads the actual nested payload `+C`, obtains its real profile slot0, pushes1 and calls it, then clears `+C` after return. The shared tail resets `+4,+C,+28,+8`; it retains profile, owner `+30`, ordinal `+34` and all unlisted fields. It never frees declaration identity `+28`. Type1/type4 raw construction does not establish owning release for other tags, and null-only or nonowning cleanup cannot qualify the whole dispatcher.

The actual prior clone/constructor evidence binds a successfully allocated nested `114h` bag at record `+C` to profile `00D16504`, whose slot0 is `008F59E0`, with actual record backlink at bag `+110`. This establishes that producer-specific binding, not universal dynamic dispatch or a fabricated bag. The already audited scalar entry unconditionally needs whole `008F5410` ordinary bag destruction; that needs actual `008F3F30` owning-record map clear. These raw Source lifetime providers remain absent. Their recursive record scalar/value, raw key buffer and same node/page pool ownership must close before a whole cleanup claim. Current raw bag/map construction, lookup, pool bookends and independent new-node publication do not provide that destruction chain.

For replacement, actual `008F28F0` must finish old-record profile slot0 destruction with flags1 before clearing/publishing mapped `+8`. This audit supplies no opaque old-record callback, scalar wrapper/free, cleanup or whole insertion implementation. The exact first missing provider for the selected child is `008F0640`; a separately leased followup can examine the known nested owning-map lifetime branch. There is no independent ready 11B Source service while its whole tail release is missing.

## Retained evidence and checks

Evidence is `local/cc12_property_replacement_release_child_readiness/`. Every live batch used `bsp.py`'s verified client for `C:/Users/sqz269/bsp.gpr`, `/battlestationspacific.exe`, x86/32, image base `00400000`. Original installation and saved analysis were preserved. The full installed PE is identity-pinned; 11 selected whole/provider/data/caller spans have physical byte copies. Thirty-three selected current Source/header/recipe/audit/whole-object/CRT inputs have per-row physical copy paths, with zero identity-only candidates. `inputs_and_native_evidence.json`, `artifact_manifest.json`, complete `COFF/` bodies/listings and the call checker/final recheck receipts permit independent review. Query-shape/parser failures and the first caller-opcode assertion failure are retained.

No Source, build/link, runtime/native/SDK/game execution, new tests, historical fixture replay, CMake, registry, ledger, shared export or Ghidra mutation occurred. Static byte/ABI/provider evidence does not establish complete owning class ABI, application startup wiring or gameplay.

Final recheck: all 33 selected physical inputs and copied bytes, the whole installed PE identity and the selected live 11-byte body were unchanged. All four native transfer rows passed. The manifest seals 77 artifacts. Root independently observed the eight-byte/two-instruction unwind funclet and separately retained the unavailable flow-properties response; the E9 encoding is authoritative and the call edge is not asserted to be a listing defect.

Primary review rehashed77 artifacts/33 physical inputs, all11 Original body/data/caller spans, seven complete current provider bodies and unique current archive members. Historical raw-duplicate pending admission is superseded by its later primary admission on main; this does not close type6 recursive ownership. Root corrects the misleading adjustor-thunk name to provisional `BSP_PropertyRecord_PublishPhaseThenRelease_Provisional`, preserving prior name/comments and prototype limitations. E9 at00C67F33 remains a tail transfer; unavailable flow-property evidence does not justify changing the genuine EH function or its edge metadata. Receipt: `local/cc12_append_release_child_primary_review/release_child.json`. Zero Source/Original-function count increment; no new build/runtime/tests.
