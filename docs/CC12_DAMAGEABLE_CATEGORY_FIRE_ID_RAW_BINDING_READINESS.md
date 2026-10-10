# Category/FireID raw manager and terminal binding readiness

The saved-pointer borrow is implemented. The remaining gap is a compatible acquisition/destruction composition and a real terminal-release binding. Raw tree lookup, unique insertion, erasure, and the sixteen-byte manager lifecycle already exist. They do not need replacement containers or reconstruction in this packet. The full category/FireID fragment remains unimplemented and unadmitted.

This is a documentation-only audit at `e4f58a3ba20aeb00f5a90de979fa272364520514`, combining requested main `0fc0be6b158ec8b568d4cdb5992e0428656c1f0f` and borrow commit `2e4228e479322b90bb138f492af192426d226066`. Its authority is [the accepted capture review](CC12_DAMAGEABLE_CATEGORY_FIRE_ID_CAPTURE_PRIMARY_REVIEW.md) and [complete caller contract](CC12_DAMAGEABLE_CATEGORY_FIRE_ID_CAPTURE_CONTRACT.md). The report is [here](../reports/cc12_damageable_category_fire_id_raw_binding_readiness.json); local replay is `python local/category_fire_id_raw_binding_readiness_verify.py`. Later Root acceptance of the borrow and serial cell at main `f95dc28b7` is contextual notification, not a replacement for this frozen baseline.

## Existing providers and the precise mismatch

| Boundary | Current Source | Consequence |
|---|---|---|
| `004C1650`, `00870370`, `0086FE20` | `native_gameplay_effect_construction/destruction` use actual AA0/F87664 cells and raw 10h manager storage. `GameSingletonHost::probe_gameplay_effect_memory` uses that getter. | Reuse this domain. Manager+4 is the raw tree header; manager+8/+C are head/count. The allocator word stays opaque. |
| `0086B650`, `0086F930` | `native_effect_handle_acquisition` supplies actual signed lookup and unique insertion with 18h nodes. | Use these existing operations, checked iterators, allocation/error providers and weak payloads. |
| `0086E8A0` | `native_int_pointer_tree18_erase` supplies actual checked iterator erasure. | Definition destruction already has a genuine raw erase provider available. |
| `008700E0`, `00870CD0` | The only current overloads take `GameplayEffectManager` / `GameplayEffectAcquisitionContext`. That manager contains `optional<map<int32_t,void*>>`. | A raw 10h pointer cannot be cast to this type. No raw overload is currently linked. |
| `00870D00`, `00871440` | Actual 24h definition storage and full ordinary Source member cleanup exist, but their context still obtains the typed map manager. | Acquiring through a raw cache while deleting through this context would cross ownership domains. Destruction must use the same canonical raw cells/tree. |
| `0041DE40` | Captures current handle, decrements actual +4, calls current raw table slot0 on zero, and clears the original slot after success. | This provider requires an actually callable table; it cannot execute an identity DWORD or a separate host companion as the raw owner. |

The complete current Core symbol scan finds exactly one positive definition for each of 14 selected existing interfaces, including the typed lower/wrapper/destructor, raw getter/find/insert/erase, and companion operations. This proves linked availability and the absence of additional overloads under those exact names in this archive. Full Source searches establish the corresponding declared interfaces; no broader claim about arbitrary differently named code is inferred.

The older `NATIVE_EFFECT_HANDLE_ACQUISITION_ORCH4.md` statement that `00870CD0` is Source-absent is stale: the ordinary typed wrapper now exists. Its explanation of raw cache layouts remains consistent with the current code. Historical acquisition text about a required component-loader service is likewise qualified by today's concrete `load_gameplay_effect_components_00870400`; genuine current virtual/application bindings remain separate.

## Smallest proposed Source surfaces

These are interface contracts, not new declarations or implementations. They keep the current typed overloads intact and add no private publication/cache or substitute lifetime domain.

1. **Borrowed raw manager cells.** A small context containing only `void* volatile& actual_manager_publication_01090aa0` and `void* volatile& actual_effect_publication_00f87664`, or those same explicit reference arguments. It passes them directly to the existing raw getter. Do not save an owner globally or replace later dynamic getter operations with a captured cache.
2. **Raw ID lower and wrapper.** A `void* actual_raw_manager` lower overload, retaining `void*& out`, signed `int32_t id`, and by-value full `uint32_t flag`. Its smallest load context borrows existing `GameplayEffectNameIndexHost&` solely for `current_game_lua_1a0c()`, `NativeStringStorage&`, and `GameplayEffectComponentServices&`. No name-index state, setup, atexit registration, or by-name API is needed for the ID path. The wrapper adds the actual manager-cell binding and retains a distinct `const volatile uint32_t&` flag argument until after its unconditional getter.
3. **Matching raw definition destruction context.** Borrow the same manager cells, real `NativeStringStorage&`, and real `GameplayEffectComponentLifetime&`. Add raw-context overloads of the existing definition destructor/scalar wrapper. Reuse the existing 24h owner, member-cleanup states, raw find and raw erase; do not create another definition owner or retain the weak cache entry.
4. **Current-zero dispatch in the canonical lifetime domain.** Before a raw category caller can release definitions, supply an explicit operation receiving the actual captured owner *after* the caller's decrement reached zero. It must resolve current slot0, carry ECX=that owner with no flags argument, and only a verified current BD30E0 implementation may reload the current profile/slot4 and introduce flags1. The existing `NativeRefCountedDeleteCalls` interface covers that latter slot4 boundary only. It does not supply the missing slot0 dispatch or its concrete gameplay binding.

The first three surfaces can be specified using existing providers without designing a second container or new arbitrary callback. Their future ordinary Source implementations still require separate authorization and emitted-code review. A raw-manager overload reusing `GuiLua51Host` remains a registry-reference Source adaptation; it does not turn those refs into Native 14h tracked Lua objects or prove Native miss-path ABI/EH.

Reusing the concrete `00870400` body preserves the existing ordinary Source loader operation. It does not establish the actual definition's current virtual+8 binding. That current identity/call, like each component's current virtual+14 reader, must be established before claiming general Native virtual dispatch; a profile constant or previously observed slot value is insufficient. This remains an explicit provider boundary of the proposed small adapter.

## Ordering required of the raw composition

The wrapper captures the actual output address and signed ID, obtains the current manager even for ID<=0, then reads the actual full flag DWORD and calls the raw lower. It returns the captured output address regardless of the lower's returned value. The lower alone handles ID<=0 by storing null, without releasing the former output.

For positive IDs, use tree manager+4 and the actual signed lookup. A hit retains the found owner's actual +4 count. A miss resolves the current embedded game Lua state at +1A0C at that time, uses the existing genuine string/component services, then inserts `(ID, fresh actual owner)` uniquely. A reentrant duplicate preserves the existing mapped pointer; neither a new retain of that duplicate nor cleanup of the losing fresh allocation is invented. The selected node must remain valid. After Name-string, Name-ref, definition-ref and Effects-ref cleanup, reread that node's **current** mapped pointer for the output store. Do not cache its value across callbacks.

Raw definition destruction must arm existing state2 before manager/lookup work, call the dynamic raw getter, then read the owner's current ID, find that ID and pass the actual iterator to the genuine raw erase. Erase by ID even when its current weak value is another pointer. A missing ID must retain the existing raw checked-iterator/error path; skipping it or throwing a newly substituted standard exception is not equivalent. Preserve state1/name, state0/array, then base cleanup and their no-retry behavior. Scalar flags are read by the existing scalar contract, after the destructor.

These interfaces do not change the category caller's established publication/state sequence: category row+4 precedes field lookup; flag1 is written after lookup and before the integer getter; returned output is dereferenced before reading current row+24 and arming20; assignment publishes the new raw pointer before retain/release; normal cleanup captures/tests current S28 under20 then lowers19; only successful nonnull cleanup clears actual S28; null and throwing cleanup leave it unwritten. The outer Msh owner remains17 until its separate close.

## Companion and callable-slot prerequisites

`GameplayEffectDefinition` is actual 24h storage with the one actual atomic reference object started at +4 by its constructor. Its leading D0DA58 word is Native profile identity. The host `NativeGameplayEffectDefinitionReference` is a separate C++ object borrowing that same count; its callable vtable does not reside at the raw definition address. Neither substituting the companion in row+24/S28 nor copying its vptr into the raw owner is valid.

`GameplayDefinitionReferences::bind` requires the actual count to be **positive** and validates current D0DA58 slot0/slot4. It allocates metadata only on first bind, does not retain/reset the count, and requires all terminal releases of that owner to use the same domain. `definition_for` maps an existing companion to raw storage; there is no inverse raw-at-zero entry. `release_zero` is private and currently closes through the typed manager context.

If the existing companion domain is selected, the smallest additional operation is a pure lookup/retirement of an **already bound** raw owner whose count is zero, using the raw destruction context above and retiring the matching companion after actual destruction. It must not call `bind`, allocate metadata, decrement again, or touch the owner after free. A contextful handle-cleanup/zero-call site would have to select that operation explicitly; adding it does not make the existing context-free `0041DE40` magically callable.

Admission requires a concrete producer/setup protocol establishing prebinding while positive for every owner that can reach a terminal release: existing old row values, cache-hit values, newly allocated miss results, and values callbacks can install in S28/row storage. The fact that `bind` exists proves none of those coverage obligations. Inserting a first bind into lookup or a zero callback introduces allocation/failure behavior absent from the retained fragment and is not authorized by this audit. All releasing consumers must share the same domain so a raw deletion cannot leave its companion dangling. Context/table/strings/components must outlive these owners.

The existing companion's terminal interface is nonthrowing. Its use therefore requires its existing valid-cache/nonthrowing terminal prerequisites; it cannot silently replace the general raw virtual's potentially throwing Source boundary or erase the category caller's cleanup ordering. If actual callable raw tables are used instead, their addresses, current slot bodies and context access must be established explicitly; writing Native profile constants is not that proof.

Current virtual lookup must remain dynamic: resolve owner profile and slot0 after zero; if that implementation is BD30E0, reload the profile and slot4 for the scalar dispatch. The six-class `GameplayEffectPlainComponentLifetime` demonstrates a current borrowed-table contract for **components**, not a definition binding. The broader scalar dispatcher selects several bodies by profile identity and alone does not prove mutable current table-word parity. Remaining component, sound, texture and renderer services must be the application's genuine current implementations; this audit supplies none.

## Width and mode boundaries

| Storage/value | Required meaning |
|---|---|
| Actual S28 | Existing live `void*` object exposed as `void*&`; no initial null or raw-byte lifetime shortcut. |
| Pending acquisition flag | Distinct live unsigned 32-bit object, store1 after lookup, late volatile **full DWORD** wrapper load. |
| Lower flag predicate | Only its low byte controls the non-table rejection gate. `0x100` has low byte zero; full-word truth testing changes behavior. |
| Fire ID | Signed 32-bit result from the existing NUMBER-only integer-or-fallback0 Source getter. Preserve the signed ID<=0 lower branch. |
| Source CRT conversion mode | A distinct live `const bool&` object required by the current integer getter; it cannot alias the acquisition flag. |
| Native CRT mode | The original full DWORD mode remains a separate Native parity requirement. A byte alias or bool copy is not its replacement. |
| Intrusive count | The existing actual aligned +4 word; no companion-owned second count, bool normalization, first-bind initialization, or extra terminal decrement. |

## Evidence and next boundary

The freeze contains 183 full current Source/header snapshots with complete quoted-project-header closure, 205 baseline Git files, 47 exact members of the retained normal worker Core archive, its actual compile command records, and the complete 1,986-member symbol scan. All selected Source bytes equal borrow commit `2e4228e47`. Twenty-three units correspond to Source746 selected-object names, but none of these whole worker object hashes equal the Root objects; this packet does not inherit whole-object identity from those names. Archive membership/current Source are distinguished from instruction-level proof. The borrow packet's existing five-object comparison remains separately pinned evidence.

All 750 frozen Source746 pins remain unchanged. The retained worker build has two passing checks; Source746's three checks are prior context. This readiness packet ran no build, tests, Native probe, game execution, Ghidra query/mutation, or new Native byte read. The accepted 193-byte category/FireID slice is referenced through its frozen prior report, without expanding or re-decoding it.

The next bounded ordinary Source candidates are the raw ID context/lower/wrapper and the matching raw definition destruction overloads. Current-zero dispatch/prebinding remains a separately gated lifetime task. Full category/FireID assembly, application activation, Native register/FP/FH3/SEH equivalence and runtime validation remain held. No C++, adapters, tests, CMake or shared ledgers were changed here.
