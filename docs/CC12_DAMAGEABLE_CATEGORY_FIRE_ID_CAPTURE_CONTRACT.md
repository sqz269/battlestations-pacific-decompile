# Damageable category and FireEfx ID: capture contract

A guarded borrow of the same active Msh owner's saved pointer, plus genuine typed S28 output and pending DWORD flag objects, closes the local capture and C++ object-lifetime gaps. It is sufficient to specify a bounded ordinary Source implementation using the existing providers. It is **not sufficient by itself** to bind that implementation to the application's raw manager, current virtual tables, component services, or Native numeric ABI. Those remain explicit prerequisites, not missing behavior to replace with a private manager, facade, callback, or default.

This packet is documentation and evidence only. It changes no C++, builds nothing, and admits no new fragment or complete function. Root must review this contract before authorizing the proposed owner method or field implementation.

## Scope and evidence

The only decoded Native slice is retained `[0087CF70,0087D031)`, 193 bytes and 59 instructions, SHA-256 `77eb783a466265193f4e5eee8cf1bcd3b370f9abe3c2834e6d2babfefe6b9b90`. It contains category publication, the FireEfx ID lookup/acquisition, assignment, and normal temporary cleanup. Its ten calls, seven conditional branches, and four state stores are covered below. The following by-name fallback beginning D031, Index before CF70, iterator operations, outer Msh release, and application caller are excluded. Complete containing Source files may include other routines; their inventory does not admit those routines to this fragment.

The inherited Native instruction/state facts come from the accepted inner-field readiness receipt. No new Native instruction, data, callee, handler, PE, or Ghidra window was opened. Current implementation/interface evidence comes from baseline `01db1d6577d88f66713f04e2125a8e357e6433ab`.

## Proposed saved-capture borrow

The current Msh owner stores `saved_data_`, `saved_length_`, and private `state_`; it currently exposes only `open()` and `close()`. Its successful open captures the raw header's data before the excluded fields. Index callbacks can change the current raw S18 header. CF70 nevertheless uses the original saved EBX pointer, not the header's current data. The pointed-to bytes can still change and must be read at category time; copying the category text during Msh open or before Index is also incorrect.

A narrowly scoped future owner method can have this contract:

```cpp
bool try_borrow_saved_category_data(
    const NativeDamageableClassSectionMshFieldFragmentScratch& expected,
    const char*& output) const noexcept;
```

It succeeds only when `state_ == 17` and `&expected == &scratch_`. On success it writes only `static_cast<const char*>(saved_data_)` to the disjoint output object, including a legitimate null capture. On failure it returns false without modifying that output. It must not read S18, `saved_length_`, the pool, or Lua; copy text; retain/release; allocate; change state; or close the owner. Failure is an invalid Source binding/precondition, not a null-category default. The field caller must require a successful borrow before executing any fragment operation.

This check excludes unopened, opening, closed, and wrong-scratch owners. It does not prove successful enclosing control flow, buffer lifetime, NUL termination, or arbitrary callback validity. The same successful owner/scratch and live actual S2C must be supplied after Index has restored17. The owner and scratch bindings/private state must remain stable and disjoint from callback-writable storage; reentry is excluded. The original saved buffer must remain readable through category comparison, even if the current S18 header has changed. A null saved capture selects the genuine borrowed `00F878E0` empty-name span; it does not authorize a replacement string literal.

## Concrete caller storage and providers

The next standalone fragment must borrow:

- The same live Msh owner/scratch, actual S2C Lua row, fresh aligned 14h S6C storage, and the previously selected live writable raw 30h row. It must not reselect or repair the row after callbacks.
- Actual `00E08138` category pointer cells with the existing volatile-cell interface, genuine `00F878E0` substitute bytes, and the actual `00CFE79C` FireEfx key storage. Stable address bindings and provider string/table validity are required.
- A live **`void*` object at actual S28**, exposed as `void*&`, and a distinct live **32-bit unsigned flag object** at the pending CF96 argument location (S-4 in the retained stack schedule). The flag must permit full-width writes and the wrapper's volatile late read. Borrowing raw bytes or applying `reinterpret_cast` does not start either C++ lifetime. Do not value-initialize S28 to null or add an early output clear.
- The existing `native_lua_integer_or_00b66380` provider's distinct live `const bool&` conversion-mode binding, and a genuine compatible `GameplayEffectAcquisitionContext` with its required live dependencies. Neither the pending acquisition flag nor a byte alias of a Native DWORD is a substitute for that C++ bool object.
- Valid aligned actual intrusive reference words and genuine callable current slot0 implementations for every old/temporary owner that can reach zero. The retained call has ECX=the actual owner and no stack flags. Native profile identity words alone do not provide callable rebuilt virtual tables.

The typed flag object is assigned full DWORD `1` **after successful FireEfx lookup and before B66380**. This models CF96's pending argument; it is not supplied to the wrapper as a copied literal. The separate integer fallback is zero. The wrapper captures the actual S28 address and ID, calls the canonical manager getter unconditionally, then reads the complete current flag word and calls the lower body. Callback writes to that flag before the post-getter load must remain visible. In particular, a nonzero high byte with low byte zero must reach the lower provider unchanged. ID zero does not bypass the manager getter and no output is cleared before it.

The existing B66380 Source requires its actual exact-number predicate, returning fallback0 for a non-number and calling the direct integer getter for a number. Its ordinary implementation currently uses a live C++ bool mode and the existing float narrowing/conversion helper. The Source bool interface is not proof of the original full-DWORD conversion-mode semantics. A future requirement to accept actual Native `0109EEA4` needs a separately authorized provider/interface change; this fragment may not silently synthesize that conversion or reuse its acquisition flag. Floating-point precision/status/trap and ordinary provider-spill differences remain held.

## Required sequence and callback effects

| Retained site | Required operation |
| --- | --- |
| CF70..CF7B | Select saved pointer or actual empty span; call genuine `native_mesh_category_from_name_007149d0` with the actual category table. |
| CF8E, before CF91 | Publish category EAX bits at actual row+4 before FireEfx lookup. The original argument preparation is interleaved; category publication cannot move after lookup. |
| CF91..CFA4 | Real protected named lookup constructs S6C. Write pending flag1, prepare fallback0, arm19 only after lookup, then call genuine B66380. |
| CFAF..CFBB | Call genuine `acquire_gameplay_effect_by_id_00870cd0` on actual typed S28/flag/context. Dereference its returned captured output address, then read current row+24, compare, and only then arm20. |
| CFC3..CFEE | Equality skips the row assignment and both assignment reference operations. Otherwise publish new row+24 first, retain nonnull new owner at+4, then release the captured old owner; on zero call its current slot0. |
| CFF0..D018 | Read and test **current** S28 while20, then lower19. For a captured nonnull temporary, decrement it, dispatch its current slot0 on zero, and clear actual S28 only after successful release. A captured null performs no slot write. |
| D024..D02C | Lower17 before the real S6C destructor. Leave the outer Msh owner/captures and iterator ownership unchanged. |

The acquired pointer is captured from the returned output address, not assumed from an earlier S28 value or the lower function's return. The old row pointer is read after acquisition callbacks. Once row+24 is published, release callbacks may change it; no rollback or final republish is added. Normal S28 cleanup reloads the current slot after those callbacks. If its captured pointer is nonnull, a successful callback that replaces S28 is followed by clearing that same actual slot; a throwing callback leaves it unchanged and is not retried.

The category provider performs its existing ordered volatile category-cell reads and uses the genuine `compare_insensitive_00438e10`/host `_stricmp`. Preserve the terminator-driven scan and its reloads; introduce no fixed category count, copied table, case-folding policy, or locale default. Original CRT locale and asynchronous mutation equivalence remain held.

## Ordinary exceptional ownership

| Failure point | Armed cleanup and preserved effects |
| --- | --- |
| Category or FireEfx lookup | Still17; no new field cleanup. Any completed category publication remains. Outer Msh cleanup belongs to its existing owner. |
| B66380 or ID wrapper before successful return | State19 owns S6C only. Do not clean or preclear S28 on a partially completed acquisition; the retained code has not armed20. |
| Row assignment/reference operations after CFBB | State20 cleanup lowers19 and uses genuine current-S28 handle release, then state19 lowers17 and destroys S6C. No row rollback. |
| Normal temporary release after CFF6 | Already19; never retry that temporary. State19 still cleans S6C on propagation. |
| Normal S6C destructor after D024 | Already17; never retry S6C. |

The retained EH edges are 20->19 through C969C5/41DE40 on S28 and 19->17 through C969BA/B67700 on S6C. The existing raw 41DE40 body is suitable for current-slot exceptional release. Its normal call cannot replace CFF0's explicit capture/test-before-lower ordering. Secondary cleanup failure follows ordinary C++ `noexcept` termination; Native FH3/SEH/longjmp, fault timing, private-frame aliases, and double-exception equivalence remain held. A future compiled fragment must prove the guard, publication, arming, lowering, and no-retry order in emitted Release code.

## Existing implementations and the remaining binding boundary

All direct provider bodies are present. The missing local capture API is small and does not require new Native analysis. Existing protected Lua lookup/destruction, category comparison, integer-or, the full ID wrapper, and its lower acquisition body can be reused within their explicit Source contracts.

The lower ID body uses `GameplayEffectManager::definitions`, an `optional<std::map<int32_t, void*>>` projection. It handles nonpositive IDs, cache hits, current game+1A0C Lua misses, component construction, weak insertion, and final post-cleanup cache-value reload. Its selected path does not require the excluded by-name lookup; the context's name-index host base supplies `current_game_lua_1a0c`, not evidence that by-name setup is complete.

The raw 10h manager getter, constructor, and destructor **already exist** in `native_gameplay_effect_construction.cpp` and `native_gameplay_effect_destruction.cpp`. `GameSingletonHost::probe_gameplay_effect_memory` uses that raw getter/publication. The current ID wrapper instead calls the typed-map getter, and the complete bounded search finds only the typed `008700E0`/`00870CD0` interfaces. No raw-compatible lower acquisition bridge is established. Casting the application's raw 10h manager into the typed manager, supplying a private second cache/publication, or replacing the canonical lifetime domain would not close this gap.

Definition storage is already actual 24h storage, including its actual reference word. Its constructor writes profile identity D0DA58; that identity is not a callable Source virtual table. `GameplayDefinitionReferences` exists as a guarded companion domain, requires binding while the actual count is positive, and requires all terminal releases to use that domain. It does not expose an arbitrary raw-owner slot0 entry that this fragment can call at count zero. Replacing row+24 with a host companion, binding on terminal release, hardcoding scalar deletion(flags1), or inventing a callback would change the contract.

The concrete component dispatcher supplies established scalar/Sound implementations and still requires the genuine remaining services, current renderer/texture/sound ownership where applicable, and component terminal dispatch. The actual application still records an unimplemented effect-acquisition composition in `SceneContents::acquire_effect`. Generic global-subsystem use of an acquisition context is not a Damageable binding. These current Source frontiers keep unrestricted standalone execution and production integration conditional even after the capture method and typed objects are provided.

## Frozen provenance and next gate

Source735 context is frozen from the accepted Index primary receipt: 735 inputs, 106 selected Core objects, five App objects, 296 positive selected Core definitions, and 739 frozen input/artifact pins. All 739 payloads (90,212,807 bytes) were verified and copied into this packet's local baseline before the audit. The complete frozen Core archive is 73,930,550 bytes, SHA-256 `4f7a0a08397a26e2dbee7a29d5b31ca3d39597caa7a3559470f5393114b65cc4`. Root's 07:04:58..07:05:14 UTC normal build and three passed checks are inherited context only.

The audit freezes 151 complete selected Source/header files, including 30 full implementation units and all their quoted project headers, plus five whole application-frontier files. The original 87 readiness Source files remain byte-identical. Each selected implementation's archived member was independently located, pinned, and copied from the frozen Core archive. Eighteen members have exact matching selected whole-object evidence in Root's Source735 receipt; the remaining twelve have archive membership and current Git Source provenance, not new whole-COFF or source-to-object equivalence credit. No archived instruction body was decoded. This inventory includes explicit alternative/binding interfaces; it does not claim closure of arbitrary virtual/runtime behavior.

The report contains current and baseline Git pins, Source snapshots, bounded evidence reads/searches, archive-member provenance, the retained193-byte decode, and a small replay verifier at `local/category_fire_id_capture_contract_verify.py`. Only this document and the report are committed.

The next authorized Source packet can implement the guarded owner borrow and the exact bounded field sequence, borrowing the genuine typed Source dependencies and retaining the above qualifications. It must inspect actual Release emission and ordinary cleanup ordering before Source admission. If the next objective instead requires the application's raw manager/virtual binding or the Native DWORD conversion-mode interface, those are separate concrete dependency packets. No Source count, full-parent/741-byte composition, original ABI, runtime, or game-validation credit is added here.
