# Whole actual preset-tree hinted insertion, 006A15E0

`insert_hint_native_input_preset_tree_006a15e0` covers the complete 508-byte,
210-instruction ordinary entry, not the older index-contract projection. The
established `STL_Tree_InsertHint_StrKeyCI_N2C_006a15e0` name stays unchanged;
the descriptive C++ name is a hypothesis. Existing AV/typed interfaces, comments
and prior records retain their separate contracts. Map-index 006A1E70 and its
unwind action are outside this packet.

Native uses ECX tree, unconsumed EDX and four stacked DWORDs (output pointer,
by-value hint owner/node and pair pointer), RET10h and EAX output. Source adds
the same borrowed RawCtx, actual 0109EEA4 feature reference, actual 0109DE1C
locale-change reference and existing `NativeCrtStricmpCallees`: eight stack
DWORDs/RET20h. It uses the published real eight-byte iterator and twelve-byte
unique result carriers. This is a new C++ interface, not a binary ABI claim.

The ordered behavior is:

1. Read count+8 before hint fields. Count0 reads private pair, CURRENT head+4, private output, then FBC0(direction1,head); childEAX ignored, retained output returned.
2. Nonempty CURRENT initialhead+4 then initialhead.left+0 precede original hintOWNER. Zero/mismatching owner invokes actual SDK; original hintNODE is read AFTER a returning diagnostic. Original owner/node remain distinct from mutable byvalue hint.
3. OriginalNODE==initialleft: less(pair,node+C); true inserts direction1 at originalnode and returns retained output. False reaches unique fallback.
4. Otherwise capture second CURRENT head BEFORE the second cached-owner diagnostic. Compare retained node/head BEFORE privatepair read; Source uses a private volatile predicate to preserve the compare/pair order, with spill/frame/flags differences qualified.
5. End arm: CURRENT head.rightmost read for less(rightmost+C,pair). On true reread CURRENT head.rightmost AFTER comparator and insert direction0; retained output returned. False reaches fallback.
6. Middle lesser arm: less(pair,originalnode+C); on true write mutable hintOWNER then NODE, actual predecessor mutates it. Read current predecessor for less(pred+C,pair). On true reread CURRENT predecessor after comparator; test pred.right.nil29. Nil inserts direction0 at currentpred, otherwise direction1 at originalnode. Failed predecessor ordering CONTINUES to greater comparison.
7. Greater arm: less(originalnode+C,pair). On true capture CURRENT head, reset mutable hintOWNER then NODE, initialize private end NODE before OWNER, actual successor then equality on real8B carriers. Read CURRENT successor AFTER equality even when equality is true; retain it across a reached less(pair,successor+C). If accepted, CURRENT originalnode.right.nil29 chooses direction0/originalnode or direction1/SAVEDsuccessor.
8. All seven FBC0 fast paths ignore childEAX and return their retained actual output. No extra private flag-upper-word input, entry-preimage promise or aggregate iterator snapshot is added.
9. Fallback passes real uninitialized12B private result to whole unique with same actual context/cells/callees. Consume actual returned pointer: load childOWNER -> read CURRENT private output -> store OWNER -> freshly read childNODE -> store NODE. Ignore insertedBYTE/three padding bytes. Aliasing and transitive child/provider writes are inherited.
10. No local catch, cleanup, key release, owner/default provider, guard, retry, revalidation, repair, rollback or noexcept; ordinary CurrentCpp child exceptions and diagnostic progress propagate.

All services are the actual published ordinary entries. Seven direct FBC0
paths use the same RawCtx/feature input. Both ordering forms use the supplied
443D00 comparator with the same actual locale cell and existing provided CRT
callees, never the older CurrentCRT two-argument comparator. Whole unique
1ab0ef5 is published in the checkout baseline cd8a0a377; earlier frozen
readiness correctly recorded its then-pending gate. No callback, host map,
default provider, global or allocator model is introduced.

The nineteen owned Native transfers are:

- `006A15FF CALL 0069FBC0` -> `bsp::insert_native_input_preset_tree_node_0069fbc0`.
- `006A1620 CALL 00BF6713` -> `bsp::invoke_native_invalid_parameter_00bf6713`.
- `006A1639 CALL 00443D00` -> `bsp::less_native_string_headers_with_callees_00443d00`.
- `006A1651 CALL 0069FBC0` -> `bsp::insert_native_input_preset_tree_node_0069fbc0`.
- `006A166D CALL 00BF6713` -> `bsp::invoke_native_invalid_parameter_00bf6713`.
- `006A1687 CALL 00443D00` -> `bsp::less_native_string_headers_with_callees_00443d00`.
- `006A16A5 CALL 0069FBC0` -> `bsp::insert_native_input_preset_tree_node_0069fbc0`.
- `006A16BD CALL 00443D00` -> `bsp::less_native_string_headers_with_callees_00443d00`.
- `006A16D2 CALL 0069AEC0` -> `bsp::decrement_native_input_preset_tree_iterator_0069aec0`.
- `006A16E2 CALL 00443D00` -> `bsp::less_native_string_headers_with_callees_00443d00`.
- `006A1703 CALL 0069FBC0` -> `bsp::insert_native_input_preset_tree_node_0069fbc0`.
- `006A1718 CALL 0069FBC0` -> `bsp::insert_native_input_preset_tree_node_0069fbc0`.
- `006A1730 CALL 00443D00` -> `bsp::less_native_string_headers_with_callees_00443d00`.
- `006A1750 CALL 0069C0F0` -> `bsp::increment_native_input_preset_tree_iterator_0069c0f0`.
- `006A175E CALL 00699EF0` -> `bsp::equal_native_input_preset_tree_iterators_00699ef0`.
- `006A1772 CALL 00443D00` -> `bsp::less_native_string_headers_with_callees_00443d00`.
- `006A178F CALL 0069FBC0` -> `bsp::insert_native_input_preset_tree_node_0069fbc0`.
- `006A17A4 CALL 0069FBC0` -> `bsp::insert_native_input_preset_tree_node_0069fbc0`.
- `006A17BD CALL 006A1050` -> `bsp::insert_unique_native_input_preset_tree_006a1050`.

Fresh typed whole-body proof verifies C:/Users/sqz269/bsp.gpr and
/battlestationspacific.exe. All 508 installed PE bytes equal live bytes and the
accepted readiness; all 210 instruction starts and 35 call/branch flows match.
There are eight Native RET10h instructions, nineteen calls, no gap, local EH,
catch or cleanup action. Identical-byte CFG proof covers all reachable starts
and balanced native stack joins. Every original instruction was reviewed in
the accepted worker and independent Root readiness.

The normal MSVC Win32 Release `./scripts/build.ps1` passes all three existing
suites in 8.14 seconds. These are existing checks, not new hinted-entry body or
Original runtime tests. The known unrelated duplicate spawn symbol LNK4006
warning remains. No tests, probes, clean/alternate builds or Native executions
were added. Before building, 22 previously cited local objects were archived by
hash, along with both exact original/alternate unique objects from the other
worker. Their cited source paths were not modified.

The complete actual CMake object has 771 code bytes / 316 instructions in seven
sections: public 698/278 plus six helpers 73/38. Every instruction, section and
relocation was inspected. There are nineteen logical Source/Native calls and
eight logical exits; compiler sharing of predecessor-original-left with begin
insertion produces eighteen actual child REL32 calls and seven physical RET20h
instructions. Separately there are seven compiler GS-check REL32 calls and one
security-cookie DIR32 relocation (26 relocations total). No local C++ EH or
rollback is emitted. Directives are 47 bytes, debug metadata 216 and checksums
80; none has relocations. No BSS, weak compiler datum, embedded table or padding
is present. Source GS checks and a private volatile end predicate are qualified
compiler differences, not Original byte or fault identity.

Actual code preserves current successor load after equality even on the true
arm, the later comparator retention, failed predecessor continuation and the
fallback owner-read/output-read/owner-store/fresh-node-read/node-store sequence.
All seven fast paths return retained output instead of child EAX. Source uses
private argument locals/preloads and canonical dead EDX values; Original stack
aliases, flags/registers, private frames and fault timing remain unproved.

The caller must supply the same actual RawCtx, feature cell, locale-change cell
and existing CRT callees. The provided CRT domain retains its locale/TLS service
obligations. Owning-error transport, current CRT allocation and SDK diagnostics
that return, throw or terminate remain the published child contracts. Storage
must admit raw Win32 volatile DWORD/BYTE accesses and finite child walks.

This body directly writes ordered owner/node output words. Child/provider writes
and overlapping aliases remain inherited, so no whole-call output or padding
entry-preimage guarantee follows. There is no local cleanup, repair or rollback.
Original private frames, register/flag state, partial faults, FH3/SEH/RTTI,
heap/locale/TLS/DF ownership, runtime and game equivalence remain unproved.
Build and static evidence do not establish Original ABI or fixture/game behavior.

The report freezes 30 current published provider LF fingerprints, native and
actual COFF evidence, build log and preservation manifests. Root preliminary review passed sixteen checks with no findings, including
Source/header and all emitted instructions. Its cited receipt is immutable;
final frozen-pin acceptance remains pending in a distinct future Root seal.
