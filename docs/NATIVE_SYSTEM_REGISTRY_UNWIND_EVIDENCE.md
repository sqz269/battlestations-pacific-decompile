# System-registry constructor unwind evidence

This packet recovers the inner FH3 cleanup of `B5BF70` needed before renderer constructor `B32410` state 27 can free its 10h allocation. It is evidence and a source-change plan only: no production edit, Ghidra mutation, build, test or game validation.

Pinned base: `0a7c5c7035e01af98bc9136db86c5817a9dca690`. The full 8,076-byte constructor, its 106-state map, all its funclets, six related callee maps and bounded dependency bodies were captured immutably. **20 spans / 12,708 bytes** matched the saved Ghidra program and installed PE through verified `bsp.py` queries. Analysis target: `C:/Users/sqz269/bsp.gpr`, `/battlestationspacific.exe`.

## Exact constructor frame and map

`B5BF70..B5DEFB` has 2,739 instructions. Its prologue saves EBP, aligns ESP down to eight bytes, installs handler `CC1113`, and later uses body EBP for the actual owner, then owner+4. Handler CC1113 loads FuncInfo `DF9AB4` and jumps to `BF6B43`. FuncInfo words are `19930522, 6A, DF9AD8, 0, 0, 0, 0, 0, 1`: 106 unwind entries, no try blocks or IP map.

Let A be `align_down(entry_ESP-4,8)`, the FH3 funclet EBP establisher value. Steady body ESP is A-50h. The ordinary body EBP register is not this funclet base.

| Value | FH3 frame location | Steady body location |
|---|---|---|
| Captured owner pointer | `[A-3Ch]` | `[ESP+14h]` |
| Local name header (length/data) | A-38h | ESP+18h |
| Local 20h record | A-30h | ESP+20h |
| Record string header | A-1Ch | ESP+34h |
| FH3 state | `[A-4]` | `[ESP+4Ch]` |

| State | Exact action | ECX | Next |
|---:|---|---|---:|
| 0 | CC0DC0 -> B5BA80 | captured owner | -1 |
| 1 | CC0DC8 -> B5BF50 | captured owner+4 | 0 |
| 2+2n, n=0..51 | CC0DD3+10h*n -> 41DD20 | address of CURRENT local name header A-38h | 1 |
| 3+2n, n=0..51 | CC0DDB+10h*n -> B34CC0 | address of local record A-30h | 2+2n |

Each funclet loads or computes ECX and tail-jumps. A record-state unwind is therefore `temporary record -> current local name -> current published array -> base -> -1`; a name-state unwind omits the record. It never walks previous literals' temporary states. Each previous completed literal is represented only by the array's current published count. JSON retains all 106 exact pairs, bytes, instructions, receiver expressions and chain orders.

Outer state is -1 while calling B5B9E0 at `B5BF98`. Only after the base returns does `B5BFA9` set 0. Array data/count/capacity are zeroed at `B5BFAD/B5BFB0/B5BFB3`; `B5BFC2` sets 1 before the first name resize. Normal completion leaves state 1, unlinks FS at `B5DEEE`, restores the frame and returns at `B5DEFB`; it has no explicit final -1 write. A successfully completed exceptional unwind reaches -1 before control propagates to the parent.

## Temporary ownership and normal disarming

For each literal, the native code resets the local name header and calls 41DD40 while only state 1 is armed. It captures data in EDI and copies the literal. The even name state is armed **after** that work and immediately before B5BBC0. The odd record state is armed **after B5BBC0 returns**, immediately before B5BED0. Before normal record release the code lowers to the even state; before normal name release it lowers to state 1. Cleanup must not be rearmed merely because a normal release has not returned.

Normal record release reads the current record string header. Normal name release uses captured EDI with the **current** name length+1. In contrast, name unwind calls 41DD20 on the **current header**, reading both its current pointer and length. Released headers are left stale; no pointer clearing is added.

The source currently sets `name_live` before resizing the name and `record_live` before calling B5BBC0, then clears them only after release returns. These booleans cannot be used as native EH states. All 210 native state writes and all 52 literal sites are in the JSON. First and last literal anchors:

| Literal | Name resize | Name arm / record ctor | Record arm / append | Record disarm | Name disarm |
|---|---|---|---|---|---|
| 0 cScreenToTextureMat | 00B5BFCE | 00B5C000 / 00B5C005 | 00B5C00D / 00B5C012 | 00B5C01D | 00B5C03C |
| 51 cMatDiffColor | 00B5DE5B | 00B5DE8D / 00B5DE92 | 00B5DE9A / 00B5DE9F | 00B5DEAA | 00B5DEC9 |

## Callee cleanup that determines the outer result

| Callee | Handler / FuncInfo / map | States and exact actions |
|---|---|---|
| B5B9E0 base constructor | CC0CA0 / DF9954 / DF9944 | 1 -> CC0C98 -> 411EE0(local guard) -> 0; 0 -> CC0C90 -> 412430(owner) -> -1 |
| B5BA80 base destructor | CC0CC0 / DF9988 / DF9978 | 1 -> CC0CB8 -> 411EE0(local guard) -> 0; 0 -> CC0CB0 -> 412430(owner) -> -1 |
| B5BBC0 temporary-record constructor | CC0CFB / DF99E0 / DF99D8 | 0 -> CC0CF0 -> 41DD20(captured record+14h) -> -1 |
| B5BD10 reserve | CC0D47 / DF9A38 / DF9A30 | 0 -> CC0D30 -> 401130(current destination, captured fresh base+completed-count*20h) -> -1 |
| B5BE10 resize | CC0D79 / DF9A64 / DF9A5C | 0 -> CC0D60 -> 401130(current destination, CURRENT array data+requested-count*20h) -> -1 |
| B5BED0 append | CC0DAC / DF9A90 / DF9A88 | 0 -> CC0D90 -> 401130(captured destination, CURRENT array data+CURRENT count*20h) -> -1 |

**401130 is exactly one RET instruction.** These placement-cleanup funclets compute and push two arguments, call it, remove the arguments and return. They do not free allocations or destroy records. B38310's full 114-byte record-copy body has no local FH3 frame.

B5BBC0 initializes its record string at +14/+18, captures the record at steady ESP+10h, and arms its own state 0 at `B5BBF2` before the string copy. Its funclet uses `[FH3 EBP-10h]+14h`. Failure during this constructor cleans that partial string **inside the callee**; the outer B5BF70 still has only the name state armed. The current source B5BBC0 constructor lacks this explicit child cleanup.

B34CC0 is a 30-byte record destructor: read CURRENT record+18, and if nonnull release it via 419CC0/BD1510 using CURRENT record+14 plus one. 41DD20 performs the equivalent 29-byte operation on a direct string header. Both leave their header words unchanged.

B5BF50 is a 23-byte array destructor: call B5BE10(header,0), reload CURRENT header data, free it through BF6989 and return. B5BE10 lowers CURRENT count before each reverse row-string release (`B5BE80`), rereads CURRENT data/count, and publishes requested count at `B5BEB4`. Thus successful cleanup leaves count zero and freed but stale data/capacity words. For the constructor's valid nonnegative array domain, resize zero does not enter growth. A fresh operation admission or allocation of host continuation metadata is not part of this destructor.

## Publication, registration and owner allocation

B5B9E0 arms its own root cleanup at `B5BA00`, then writes profile D626F4 at `B5BA08`. It captures the first manager's section, enters/increments it, arms state 1 at `B5BA31`, publishes the actual owner at 0108FE94 (`B5BA36`), gets the manager again, reloads CURRENT publication and registers it at `B5BA4A`. Native failure cleanup releases the captured guard and writes root profile CE3818. It does **not** unregister or clear publication. A failure after publication can therefore leave publication/registration side effects before the parent frees the failed allocation. Outer B5BF70 is still state -1, so it must not call B5BA80 for a base that never returned.

Once the base has completed, outer state 0 uses B5BA80. That destructor writes D626F4, obtains/captures the first manager section, then obtains the manager again and unregisters CURRENT 0108FE94 at `B5BAE4`. It clears that publication at `B5BAE9`, leaves the originally captured section and writes CE3818 at `B5BB06`. Its own failure handlers release an armed guard and stamp CE3818; they do not invent a missing unregister/clear. Current source CapturedSoundLifetimeSection provides section cleanup, but `base_construct`/`base_destroy` lack the unconditional root-profile failure cleanup. Existing actual 411EE0/412430 providers used by renderer-base lifetime can be reused.

No B5BF70 map action frees the registry owner. B5BF50 frees only backing-array storage. After the complete inner chain reaches -1, parent state 27 calls BF65AC on its captured 10h allocation through CBDED9, then continues renderer states 24..0. The final owner profile is CE3818 if base cleanup completes. Publication cleanup uses CURRENT publication; it does not prove the captured owner has no remaining native registration if reentry changed the global. No extra captured-owner unregister is authorized by this evidence.

## Failure allocation dispositions

| Failure point | Native disposition before parent allocation free |
|---|---|
| Initial name allocation/copy, before even state arm | No outer name cleanup. Clean current published array and completed base. 41DD40 has no own FH3 frame; do not invent cleanup for acquired but unpublished name storage. |
| B5BBC0 string copy | Callee cleans current partial record string; outer cleans name, current array and base. No second full record destruction. |
| Reserve allocation BF55BE at B5BD50 | Reserve state-1; no returned fresh allocation to free. Outer temporary record/name/current array/base cleanup follows. |
| Reserve copy B5BD8D | Reserve state0 calls no-op401130. Fresh unpublished buffer, completed copies and any partial string can remain unreleased. Outer cleanup operates on the CURRENT published old array. |
| Append copy B5BF24 | Append state0 calls no-op401130; count increments only after return at B5BF29. Outer reverse cleanup excludes the uncounted partial row, then frees current array backing storage; separately acquired partial-row string data can remain unreleased. |
| Normal record or name release after disarm | Do not repeat that temporary cleanup. Follow the lowered even state or state1. Current host string-release noexcept limits still apply. |

These are instruction-derived dispositions. They were not fault-injected in this evidence packet. A completed native unwind is **not** proof that every acquisition was freed. `NativeCompiledShaderArrayOperation` currently retains unpublished/current-record pointers and terminates on failed destruction. Freeing those acquisitions, clearing them to satisfy diagnostic retirement, or using `acknowledge_diagnostic_cleanup` as production recovery would change the native failure behavior.

## Minimal source change plan

1. Complete the small child cleanup contracts first: B5BBC0 current-string failure cleanup; B5B9E0/B5BA80 root/guard failure cleanup with their different publication rules; and B5BF50 current-array cleanup without public binding admission or fresh host metadata allocation. Reuse existing actual string, guard, singleton and CRT providers; retain their documented failure domains.
2. Add a separate, bounded raw B5BF70 constructor entry/overload using a fixed per-call frame: actual owner, local name, local 20h record, captured normal-name pointer and explicit native state. Use the 52 recorded arm/disarm intervals and the exact record/name/array/base cleanup chain. Do not reuse the current early `name_live`/`record_live` flags as native states.
3. Its reserve/append path must retain the no-op placement-cleanup behavior. Any fresh uncommitted buffers or uncounted rows left unreleased by native must not gain automatic cleanup. Diagnostic leak records may record addresses, but must not own or later dereference the freed parent/header. Avoid host heap allocation solely for continuation metadata at native call sites.
4. Keep existing retained-operation APIs for their current callers until the new raw path is proved. Route parent B32410 to the new path only after its child cleanup and exception propagation are complete. The raw path must not enter the existing retained Operation protocol: it provides a new proved lifetime contract whose unwind finishes before parent free. This is not a wrapper that clears/bypasses a failed guard around the current throwing implementation. Root owns that later parent integration.

The concrete acceptance check for later implementation is a focused comparison covering base-before/after-publication failure, temporary-record construction failure, reserve allocation versus mid-copy failure, and append-copy failure. It must compare cleanup ordering, CURRENT versus captured pointers, row count/publication, native unfreed allocations, then exactly one parent free. This packet adds no test or implementation.

Calling the existing public registry destructor cannot implement this plan: it calls binding.begin while the guard is already held; its internal path adds a D62A3C write and fresh ArrayOperation allocation. Calling a whole-registry destructor also mishandles the outer state-1/base-construction case and partial temporary cleanup. Clearing guard/phase, replaying work, adding rollback, or declaring leaked acquisitions cleaned are explicitly excluded.

## Evidence and limits

JSON includes all native maps, funclet bytes and instructions, 210 body state-write IPs, 52 literal names checked against the PE string pool, each normal release site, six subordinate maps, exact failure cases, source gaps and the source-change plan. Immutable manifests:

- `J:\PROG\battlestations-pacific-decompile-orch5-native-system-registry-unwind\local\output\system_registry_unwind_inputs\manifest.json`; SHA-256 `13a3ba99b3ec904f4f4813675ec7bc030117fa77b4a4abc114f68745976c61ef`.
- `J:\PROG\battlestations-pacific-decompile-orch5-native-system-registry-unwind\local\output\system_registry_unwind_dependencies\manifest.json`; SHA-256 `5062a212d170479c5a471df1213dbf7e1b55a51a04676ebad7e063e764052e7b`.
- `J:\PROG\battlestations-pacific-decompile-orch5-native-system-registry-unwind\local\output\system_registry_literal_sites\manifest.json`; SHA-256 `3154c44908989fd66805209d705e0d2b1c14812f690d7056d0210bcda8431a63`.

Source snapshots are pinned to the stated commit. Native inputs were read through wrappers that verify the expected project/program. Only the two evidence files are committed. No additional entrypoint was claimed and no saved analysis was changed.

Native exception-object/SEH dispatch, faults in arbitrary providers and second exceptions during cleanup remain outside this evidence-only result. The ordered chains describe successful progression through each cleanup; they do not establish that a parent free occurs after a cleanup itself fails. Current readable-storage and noexcept string-release source boundaries remain explicit. Native byte equality is not a runnable-game or original-FH3 compatibility claim.

## Integrated validation at 87dad090

This remains a static unwind map and source-change plan. Root checked all 106 state/action pairs, their native funclet bytes and all 52 literal names against the installed PE; the complete captured map/body spans also match fresh Ghidra bytes. No registry constructor implementation or native FH3 execution is claimed by this evidence packet.

The combined checkpoint retains 3413 immutable artifacts at `local/checkpoints/87dad090/native-renderer-lifetime-followups/validation.json` (SHA-256 `930898fb5129ef943689981be1fcdd48913c18e9154efa35665c19f74398bfc3`). It records 13 new entry bodies (713 native bytes), the separate capability ownership overload, 14 saved/read-back annotations, seven completed returning tails, 61 passing call rows and 45 live/PE spans totaling 18,479 bytes including reused/evidence spans. The B22530 RET4 starts at B2258C and ends at B2258E; stored-body ends and return-instruction addresses are recorded separately. Worker evidence remains preserved. Full renderer lifetime, application adoption, original exception identity and gameplay remain incomplete.
