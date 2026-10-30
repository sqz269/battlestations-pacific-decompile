# Raw DrawSection vertex-layout storage

Baseline is `a4bac101f` after merging `origin/main` at `0812812ea198274454e1fab0a5da8f9e7e716b64`. All four frozen Frame lifetime files from `d4e6aa48d20e55b6667a11d37a462c50ab2ee6e2` remain byte-identical Git blobs. This packet adds four new files only. It supplies the actual raw-backing seam required by SingleLine after both unmaps; it does not edit that caller or install an application adapter.

The only pre-existing `B865A0` implementation is `rebuild_native_mesh_section_vertex_layout_00b865a0(NativeMeshSectionStorage&, NativeRenderActualOwners&, void*, NativeMeshSectionLayoutServices&)` in `src/native_mesh_section.cpp`. That typed receiver/key/owner contract does not authenticate an arbitrary actual section pointer returned to the raw caller. The new functions use the same actual backing through reached raw DWORD accesses, without reinterpreting a receiver as a typed MeshSection. The typed body and its existing credit remain unchanged; this packet adds zero Original, helper, vendor, duplicate-body, ABI or runtime credit.

## Whole native evidence

| Entry | Inclusive end | Physical bytes | Decoded instructions | Whole PE SHA-256 |
| --- | --- | ---: | ---: | --- |
| `00B865A0` | `00B8664E` | 175 | 64 | `2bea4cdbde496711878d8baa0b57860646c0fd5f31e70ff142b833070f446f60` |
| `00B86550` | `00B8659E` | 79 | 36 | `b6ab5966ea8e3efed1f312889159258d0d75b2509e97aacb22719f0726efdfc1` |

Both whole assembly listings and saved pseudocode were read. Scope is 254 physical bytes/100 decoded instructions, with no x87 forms and no local FH3/SEH frame or Native unwind action. The jumped-over three-byte `LEA ECX,[ECX]` at `B8660D` was read from installed PE and live Ghidra. Native `B865A0` consumes ECX section and one stacked mesh DWORD and ends in `RET4`; `B86550` consumes ECX section and plain `RET`. Saved `undefined(void)`/extra-EDX prototypes do not supersede assembly.

The installed PE remains 12,223,752 bytes with SHA-256 `b682a82c52f81f957b2c70222077305a933f72481686c88843077f714b956dd6`. Four original raw saved exports, both whole body hashes, nine call encodings (two direct E8, three import-cell calls and four register calls), one padding span, and four known profile spans are pinned in [the report](../reports/cc16_native_mesh_section_layout_storage.json). PE import descriptors independently identify `CE221C` as `KERNEL32!InterlockedIncrement` and `CE2220` as `KERNEL32!InterlockedDecrement`. Their disk IAT words are import preimages, never host-callable pointers.

Before live batches, `bsp.py ghidra count` passed target verification for project `bsp`, program `/battlestationspacific.exe`, x86:LE:32:default and image base `00400000`; live and saved counts were both 77,632. Configured project is `C:/Users/sqz269/bsp.gpr`. No project mutation or new export was performed.

## Borrowed Source contract

`clear_native_mesh_section_vertex_streams_storage_00b86550(actualSection, context)` and `rebuild_native_mesh_section_vertex_layout_storage_00b865a0(actualSection, liveMeshDWORDhome, actualRawKeyHome, context, acquisitionOutput)` are ordinary Source ABIs. The captured section is the actual caller-selected section, including the current section loaded after SingleLine's two unmaps. No typed owner membership is inferred from its address.

The nonowning context borrows genuine current-table/other-target transport, the existing raw `NativeRefCountedDeleteCalls`, the existing hardware-layout constructor/factory context, the same renderer publication, and live callable reference-operation homes for `CE221C/CE2220`. Every reached call reloads its actual import target; no import target is cached at entry. Numeric profile/target words remain DATA selectors. The pure metadata lookup returns the SAME live table for a captured current profile without allocation, reentry, lifetime changes or entry snapshots. No numeric vtable word is cast to a function pointer.

The raw key address is supplied by the caller and retained through all callbacks and failure. It has descriptor home zero at `+00` and live count at `+10`, plus valid addressable adjacent backing for every actually reached raw read/write. The body allocates no key, fixed array or owner and initializes only the native count writer. Unwritten words, when reached by the genuine callee, need valid caller-provided ordinary DWORD backing/residue; this body does not synthesize zero fill. The optional acquisition output is the existing `B2F710` observation contract: explicitly nullable, empty on entry when supplied, and retained after failure. It observes the actual acquired result before insertion; it does not retain or clean it. Other-target transport must preserve its own genuine acquisition/custody epochs with prepared metadata.

## `B86550`: current-count release and clear

The initial signed current `section+4C` comparison selects either the loop or the final count reset. On each reached row, capture the current slot at `section+3C+4*row`. A null captured word skips release and slot clear. Otherwise load the current genuine `CE2220` callable and decrement the captured stream's actual `+4`. Only on zero capture that stream's current profile and current target `+00`, then dispatch that captured target on that captured receiver.

After a returning release, clear the SAME original slot even if a callback replaced it. Advance the independent DWORD row counter and slot address, then reload the section's current signed count for the continuation comparison. The count may shrink or grow; no four-stream cap, initialized-slot gate, fixed snapshot, semantic registry shortcut or extra retain is added. Reset `+4C` only after normal loop completion or the initial nonpositive arm. A throwing decrement/current-target call skips the failed slot's clear and the final reset; earlier effects remain.

Known `BD30E0` directly reuses `invoke_native_ref_counted_delete_00bd30e0`, which freshly captures current profile and selects current `+04` with flag one. Its required existing scalar provider must bind the real current scalar, not a nonthrowing canonical-reference terminal substituted for the raw call. The known logical vertex profile `D61D6C` names `BD30E0/B4BF10`; hardware layout `D62AF4` names `BD30E0/B60770`. Existing whole raw scalar bodies and their real contexts are available. Other current00 targets require the genuine supplied operation; there is no successful fallback.

## `B865A0`: exact layout and append epochs

Probe current `+50`. On a nonzero probe, call whole raw clear above FIRST. Only after it returns reload `+50` and capture the current layout. If nonnull, perform the same current-reference release schedule, then clear `+50` only after return. Recheck current `+50` and return if still nonzero. There is no rollback or cleanup around this path.

If current count `+4C` equals zero, only now read the live mesh DWORD home and directly call the whole unchecked raw `native_mesh_vertex_stream_unchecked_00b73260(mesh,0)`. Capture that returned stream. Reload genuine `CE221C`, then increment stream `+4` without a new null guard. AFTER the increment call reload current section count for the append destination, store the captured stream, then reload count AGAIN for the wrapping increment. If the append destination aliases `+4C`, the increment sees the word just stored. No typed `append` helper, capacity guard, default stream or prior count snapshot replaces these epochs.

## Raw key alias and descriptor loop

The initial signed section count comparison occurs BEFORE key-count `+10 = 0`; the comparison result is retained across that store, matching Native `CMP`, `MOV`, `JLE`. A nonpositive initial count skips descriptors and still reaches the renderer layout call with native count initialization complete.

The descriptor loop has its own wrapping row counter and raw stream-slot pointer. For each row capture the current stream, current profile, and current target `+24` at that call epoch. Known `B48CE0` directly reuses the exact raw getter returning actual stream `+68`; other targets require their genuine whole Source transport.

Only after the descriptor call returns read CURRENT key count, write the descriptor at `key + 4*count`, then read CURRENT key count AGAIN and add one. Descriptor index 4 is the count word itself: the descriptor write may replace the count before the increment. This is raw DWORD address arithmetic, with no C++ fixed-array indexing beyond an object, four-stream cap, separate count copy or new buffer. The independent loop row remains separate. After both key writes, advance row/slot and reload CURRENT signed section count for continuation. Physical private-stack aliases beyond the supplied raw backing are not claimed by this new ABI.

## Renderer and genuine reuse

Only after descriptor traversal capture the SAME current renderer home `F8D394`, that renderer's current profile, and its current `+40` target. Pass the SAME raw key address. Known `B2F710` directly reuses `get_or_create_native_hardware_layout_00b2f710` with the existing actual tree/pool/string/owner context; its native renderer ECX is unused. Other current40 targets require genuine whole Source transport on the captured renderer/profile/target. After the whole call returns, store the returned actual layout to captured section `+50`, including replacement of callback-written values or a genuine null result.

The exact reused seams are raw `B73260`, descriptor getter `B48CE0`, raw reference-delete dispatcher `BD30E0`, and raw hardware-layout factory `B2F710`. Existing genuine raw `B4BF10/B60770` scalar providers remain the finite known lifetime endpoints, not new reconstructions. No COM declaration, stream, renderer, layout, allocator or ownership registry is fabricated here.

The new loops add no stream-count domain cap. Existing known Source callees retain their established reached-input requirements: in particular the hardware creator's real declaration/element scratch and valid key domain must be proved by its retained provider context. This packet does not weaken that contract or add a new runtime count check. Arbitrary Native stack corruption, malformed storage or callbacks that invalidate still-reached backing are outside the proof.

## Remaining binding and proof cuts

Root's actual SingleLine adapter must borrow the selected actual section, a retained live mesh DWORD home, raw addressable key/backing and any acquisition observation; use the same renderer/layout/stream/scalar contexts and genuine import homes; and bind the current-table and actual other-target transports where reached. The scalar provider must use the same actual logical-vertex/hardware-layout domains and retirement metadata. All reached objects, raw slot words, tables, contexts, key/mesh homes, pools/tree and output metadata must remain valid through their Native access epochs and partial entered failures. No native retain or new allocation owner is introduced to establish this proof.

There is no local Native cleanup action to reproduce. The new functions add no `noexcept`, catch/termination, retry, rollback, destructor-time replay or clear-on-failure policy; completed releases, slot/count/key writes and acquired resources remain for explicit disposition. Existing reused helpers retain their previously established internal cleanup behavior. Both older typed bodies, SingleLine, CMake, shared application/view/registry/ledger and saved Ghidra state are unchanged.

Validation is static: whole assembly/PE/call/profile/import proof, Source epoch inspection, immutable Git LF input/output pins and whitespace. No compiler, build, tests, probe, native/game execution, process control, ABI/runtime comparison or activation was performed. Whole front-end readiness and guard flip remain false. Minimal next work is common integration/build and the genuine retained actual adapter/provider binding; no additional layout/helper recovery is needed for the evidenced known target set.
