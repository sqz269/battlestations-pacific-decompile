# Actual Font storage constructor AD55C0

`construct_native_font_storage_00ad55c0` now composes the whole constructor over actual raw Font storage, four live argument cells, the genuine tree-head allocator, the raw name constructor and full tree cleanup. It is a new Source interface in the existing registered Core translation unit. It does not construct a semantic `FontDescriptor`, activate Text, or establish the complete raw Font load/registry/retirement graph.

The worker baseline is `9a3cf7ad4683981bd37cac01d789c3ef44a692e8`, merging published `ea33126f3ed53284f6f104ff6551977f048ffb6c` with accepted 151. A temporary exact-file refresh lease was released before the precise AD55C0/four-file lease. The sealed 146/151 families remain unchanged. Root150's manifest and canonical allocator object were verified before copying; the later worker build did not overwrite that baseline.

## Original body and live inputs

The unchanged installed Original and live verified BSP program agree on `[00AD55C0,00AD5691)`: 209 bytes, 71 physical instruction starts, SHA-256 `0efe2fd5d21694683d3f1535634ed7640d681cd8ac3a366dc1b574e6a4e50173`. Original ECX is the actual Font; four stack words are name-header pointer, scale bits, alpha bits and a word whose low BYTE is uppercase. It returns the SAME Font in EAX with `RET 10h`. The Source function uses an ordinary C++ ABI and required borrowed references to the four cells; it does not reproduce that stack/register ABI.

The complete direct calls are AD55E0→AD42C0, AD561E→41DD40 and AD5633→BF7680. No indirect call occurs in this constructor. The scalar values travel as bits, with no conversion or arithmetic. The Source interface does not snapshot the incoming words at entry or read a name header for preflight.

The observed order, including current reloads, is:

1. Call genuine AD42C0, publish its allocation at Font+04, then set that returned head's BYTE+15 to one.
2. Reload CURRENT Font+04 separately before each head self-link: head+04, head+00, then head+08. Store Font+08 count zero afterward.
3. Capture CURRENT name-pointer cell only now. Compare Font+0C with that pointer; arm the sole cleanup state before clearing the name header.
4. Construct the name through the established raw 00426060 equivalent of the inline name sequence, using SAME `GameNativeStringProcess::raw_context()`.
5. Read CURRENT scale DWORD bits, then CURRENT uppercase LOW BYTE; store scale at Font+18. Read CURRENT alpha DWORD bits; store WORD zero at Font+14, then alpha at Font+1C.
6. Store ten separate DWORD zeros at +20,+24,+28,+2C,+30,+34,+38,+3C,+40,+44. Finally store the captured uppercase BYTE at +48 and return the SAME Font.

The raw allocation is AC hex (172) bytes. This constructor writes +04..15, +18..48 as specified above. Font+00..03, +16..17 and +49..AB retain their preimage. In particular, +04 is the tree head, not a Font reference count; +08 is the tree count. Later glyph regions at +4C/+6C/+8C are not silently initialized here. There is no vtable publication, whole-object zeroing or new initialization counter.

## Actual lower services

AD42C0 is the existing `allocate_native_font_tree_head_00ad42c0`: genuine owning `singleton_lifetime_allocate(object,18h,18h)`, three independently guarded DWORD-zero effects and BYTE14=1/BYTE15=0. The constructor changes BYTE15 to one and forms the current self-links. Matching node/head destruction uses the SAME Source `singleton_lifetime_free` domain. Source SDK allocation is nonnull-or-throw; Native null/invalid-address behavior and private heap/new-handler/exception identity remain outside that admitted domain.

The raw `copy_construct_native_string_header_00426060(destination,source,NativeStringRawPoolContext&)` preserves the name-header identity and current operand schedule established in 151: compare pointers; zero destination length/data even on identity; otherwise capture source length, call real 41DD40 with preserve=1, reread current source length, then read current destination count, current source data and current destination data before copying. It does not replace the source header with early captured characters/length. Its existing Source zero-copy-count omission and overlap-capable `memmove` policy remain explicit; no Native zero-count fault or imported memcpy ABI claim is added.

The raw process context is required. It refers to the actual retained string-manager publication, pool publication and return-disable cell, and reaches genuine 00419CC0/BD1120/BD1510 services. The noexcept release projection returned by `.strings()` is not substituted. Raw getter/allocation/return exceptions may escape. An identity name abandons the cleared old buffer just as the recovered constructor sequence does; this packet adds no repair or name cleanup.

The existing `terrain_particles_cc13` raw-offset semantic helper and real `NativeFontResourceOwners` descriptor/data/image owners remain intact. Their earlier captured string projection or different owned representation is not cast into this raw object. This packet adds actual live operands and the proven full cleanup route without duplicating their algorithms or claiming they were absent.

## Full cleanup and exception domain

The Native physical EH chain is CB9478→FuncInfo DF0A94 (all 36 bytes), map DF0A8C: state 0→-1/action CB9470; the action loads the saved Font receiver and jumps to AD5360. State 0 is armed at AD560B after the pointer comparison and before name clearing. There is only tree cleanup. There is no name tidy, outer Font free, AD53A0 call, glyph/image release or rollback.

151 proved the complete AD5360→86FDE0 cleanup alias and its complete range/erase/recursive/rotation/iterator helpers: nine pairs, 1,299 bytes per side, 25 resolved named operand rows. Both iterator-error one-state FuncInfos/action maps were compared in full. Those frozen proofs and current unchanged Source inputs are carried forward; this packet does not infer catch/RTTI/FH3 compatibility from equal payloads or symbol names.

The Source action calls genuine `destroy_native_int_pointer_tree18_0086fde0(actual_font)` on the FONT BASE. Its complete full-current-range implementation includes accepted by-value iterator/owner validation, real increment/erase and the owning Source invalid-parameter/error transport. It reloads the current head after range erasure, frees it through the SAME SDK CRT, then clears current tree head/count. There is no empty-tree-only shortcut, stale originally allocated-head free or assumed payload release.

For ordinary Source C++ exceptions after name identity comparison, the private cleanup arm is consumed to -1 before the full tree action. The initial exception is rethrown after successful cleanup; a cleanup exception during that unwind terminates. No automatic name tidy, outer allocation owner, replay/reset/fresh retry or second cleanup is introduced. The caller retains the partial graph and metadata as required by the selected providers rather than obtaining a new ownership facade here.

The private volatile comparison slot/read deliberately makes the comparison occur before the private cleanup-state arm. The raw helper repeats the same pure comparison; no raw name-header read or provider call intervenes. That slot is Source metadata, not an alias of the Original private frame.

The optimized compiler FH3 state store is hoisted to +94, before some valid-domain raw self-link stores and before the Source private arm. The actual private comparison is +B4, its spill/read +B6/+BA, and the private arm +C0 before the raw name call +CA. This is not Original EH instruction-placement or hardware-fault cleanup proof. The admitted domain excludes faults in these raw accesses; there is no ordinary throwing call between the earlier allocator and the name helper. Native FH3 stack identity, register inputs, SEH, fault order, exception/catch/RTTI compatibility and game runtime behavior remain unclaimed.

## Required storage and lifetime correspondence

Before entry, the caller must independently provide aligned writable actual AC-byte Font storage and SAME owning SDK CRT/string-process authority. Every reached source header/backing and current tree/node must remain genuine and live until its selected service consumes it. A Source Font descriptor, an incomplete Font prefix or a copied header is not such storage. No blank-zero precondition replaces unwritten bytes.

The four argument-cell identities, view, process/context and private Source frame must remain stable and disjoint from the raw Font/tree/name regions. All reached metadata/backings and process publications are retained across allocation, raw getter, name-copy callbacks and exception cleanup. Providers may change live values/current fields only while preserving those admitted object/heap relationships; they may not retire the active receiver, cells or provider metadata. Access is externally serialized; volatile reads are not concurrency synchronization.

In particular, a name allocator/return callback can change current raw fields before cleanup. The full cleanup uses the current tree and its real nodes under the accepted lower contracts, rather than an automatic empty-head token. Caller graph retention and valid alias relationships are preconditions, not validations inferred from public fields. This packet adds no raw Font producer/loader/registry/current-Text composition or normal activation readiness.

## Compilation and preservation

The strict direct compile is MSVC Win32 C++17 `/MD /EHsc /O2 /Gy /W4 /WX /fp:strict /permissive-`. The complete constructor COMDAT is 356 bytes/116 physical starts including its catches and tail INT3 padding, with eight ordered symbolic relocations. The full generated handler is also frozen. The only semantic callees are the real owning allocator (the existing leaf is inlined), raw name helper and full tree destructor; security cookie, terminate, rethrow and FH3 machinery are Source compiler/runtime support.

The full main body, both catch transfers and handler were reviewed, not only normal stores. Catch +132 tests the private state; +13E consumes it, +149 calls 86FDE0, +159 rethrows. Cleanup escape reaches terminate at +12C. Scale/uppercase/alpha loads occur at +D5/+DA/+E4; no later scalar is cached across the name call. All eight symbolic relocations and the full emitted bytes are recorded in the machine report and private object audit.

`scripts/build.ps1` completed once after the C++ changes. The actual registered Core TU compiled, and all three existing checks passed in 8.35 seconds. No new test, behavior fixture, probe, copied-body execution or game validation was run. This proves compilation and existing-check compatibility, with static whole-body/operand/EH evidence for the new capability.

Both complete 146 Source files remain literal byte prefixes. Root150's immutable canonical old allocator is 107 bytes; the new canonical allocator has the SAME SHA-256 `8f85969c977a1f88d8d602304b9df19a5219ea7bf77d8abcf735c40bd364786f` and all three ordered symbolic relocations. The strict old/new allocator is separately 108 bytes with SHA-256 `1a6b55f0dcfec6a8c2b89f0138c68acd6b235258dc8f5adcd8d90017632fffdd` and its three relocations unchanged, including a fresh pre-edit strict comparison with sealed 146. No strict-versus-Core old-allocator equality is asserted.

Only owned AD55C0 naming evidence and a distinct raw-input reconstruction row are appended. The prior provisional name/comment and semantic reconstruction record are preserved; other library names and algorithms are unchanged. Ghidra annotation uses the write lock, prior-value history, save and forced refreshed exports. Current Original bytes are rechecked afterward.

The machine receipt is `reports/cc12_native_font_constructor_storage_155.json`. Exact Source prefixes, baseline Git blobs, actual lower-provider Source, reused 151 Native/EH/alias proofs, Root150 canonical baseline, strict/Core objects, full optimized listings, build/check logs and owned annotation/export/history are frozen in ignored `local/cc12_native_font_constructor_storage_155/`. Its final immutable `artifact_manifest.json` is pinned by the receipt; no sealed 146/151 artifact is regenerated.
