# Shared GuiGroup/Text Element completion review

Frozen Source baseline: `a299cdfc6e73814528b007f524d317dce02df0f8`. **No separate Element-completion metadata is required for the exact proposed closed Source extension, under the conditions below.** Existing Text/Group role states and cold cross-entry rejection can prove completion. This is static readiness advice, not an implemented/admitted Group owner or a generic no-throw claim.

## Actual provider proof

`game_native_gui_text_process.cpp:22-42` creates a local final `CounterHost`, whose sole counter override calls the same final `TypeIdCounterLifetime::get_006fac20`. The private Element adapter uses a direct reference-only lambda and the actual `LightTypeBootstrap`; it creates no generic host, Model, callback wrapper or counter substitute.

`light_type_bootstrap.cpp:22-44` distinguishes the important failure boundary. A null counter publication enters actual manager/critical-section, allocation and registration work. Real allocation can throw `bad_alloc` (`singleton_lifetime.cpp:51-64`), and the cold provider calls can escape before Element completes. Guard, name, parent, publication and counter effects stay sticky. A nonnull publication instead returns its first current global load at lines23-24: no allocation, manager getter, registration, lock, callback or destructor runs on that arm.

A normally completed cold Element has finished its actual counter access and ID update. Under the retained, serialized, non-retiring publication contract, that same counter remains nonnull and live afterward. The actual Text suffix (`gui_widgets_cc13.hpp:112-114`) contains only two plain parent-ID copies, the same warm counter getter and a `uint32_t` postincrement/publication. Its trivial locals and final enum assignment cannot add a C++ escape. The separately recovered Group suffix must remain only current parent-word capture/copies, the same warm getter, one current counter read and ordered counter/own-ID stores. There is therefore no supported ordinary C++ escape after successful Element but before Text/Group returned metadata.

This does not infer completion from a guard or a nonnull counter. Cold registration failure can leave both sticky before Element finishes. The induction starts from private loader-zero Element storage: only Text or Group may first enter it; either failure precedes completion and leaves the role entered/threw, or Element success necessarily proceeds to completed role metadata. A hot Element arm is admitted through that completed-role history. Layer cannot create another cold route because its existing API requires completed Text.

The actual application supplies one retained `TypeIdCounterLifetime` through `NativeVfsOwnerServices`, borrowing its actual `0109DB7C` and manager cells. `GameNativeTypeStorage` owns the same root/guard references; `GameVfsHost` returns those retained owners. Counter destruction clears the publication through the shared-drain `CFB6C4` dispatch. The proof excludes such drain, replacement or concurrent mutation during later role initialization. It makes no claim about an arbitrary `GuiTypeCounterHost`, Native ABI/FH3/SEH/fault transport or external interruption.

## Required implementation conditions

1. Bind `counter_/root_/root_guard_` once or compare all three identities before Native effects. Text must preserve a Group-first binding; metadata admission must not pre-read Native IDs or guards.
2. Preserve Text's completed-pool prerequisite. Group type startup requires neither Text completion nor either payload pool; Group-before-Text remains valid and creates no implicit pool startup.
3. For an already returned own role, preserve its same-domain checks and old prerequisites, then return before an unrelated failed-role veto. Reject own entered/failed replay. For a cold role, reject the other Text/Group role entered/threw before Native effects.
4. Mark the target role entered/threw before its first Native guard/name effect; mark returned only after the last successful Native publication. Keep all partial Native effects sticky, with no rollback, guard reset or retry.
5. Retain the same genuine counter object/publication and canonical root/guard domain through every role initialization, consumer and shared shutdown. Owner-address equality does not authorize a retired/reused domain.
6. Keep Layer's completed Text prerequisite, same-domain admission and existing getter gates. Group completion cannot admit Layer. Preserve the existing Element getter's Text gate; completed Text/Layer/Group access must not acquire a new global interrupted-state veto.
7. Append Group entered/threw to `native_operation_interrupted()` without altering its existing terms or adding callbacks/Native reads.
8. Keep the post-Element suffix closed: no new fallible validation, callback, allocation, logging, lock, registration or foreign provider before returned metadata.

Condition7 is required for existing failure custody. A Group-first failure followed by rejected cold Text can leave Text/Layer unattempted and the Text pool returned. The old predicate at `game_native_gui_text_process.cpp:167-169` would report false. The established catch at `game_hosts.cpp:3119-3130` would then rethrow through the sticky VFS/counter/root graph instead of taking its `_Exit` retention boundary. The same predicate also feeds the existing `game_main.cpp:1230-1252` startup catches. No new caller activation is established here.

## When separate Element metadata becomes necessary

If any actual post-Element C++ escape is introduced, another cold Element entry appears, or the warm publication/lifetime condition cannot be maintained, this proof no longer suffices. Re-review then requires private Source Element-completion metadata and an owned helper that records completion immediately after genuine `AA8200` returns, before later role work can throw. Such metadata must preserve Native ordering, sticky partial state and failed-role replay rejection; a Native guard must never be treated as completion.

The report retains 20 frozen Source/config pins and 26 bounded proof excerpts. The Group body's recovery and ordered suffix belong to the separate lineage-readiness packet; this review does not duplicate or credit it. No Source/CMake/shared metadata/Ghidra mutation, compilation, link, test, probe, target execution, runtime or activation work occurred. All Original/native/ABI/FH3/SEH/fault/runtime/activation credit is zero.
