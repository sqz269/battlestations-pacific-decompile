# CC16 Glyph constructor-slot failure-origin audit

This is an evidence-only design audit at `15310a621205977cfa25402aa14646b7e72e8a10`, with the separate sealed host slice `8f9f9942bfc3bd8f23d385f6269d01375b72511b` read directly from immutable Git blobs. No production code, old evidence, ledger, CMake, Ghidra state, build or executable changed. Source34/getter/caller receipts remain historical and unchanged. The audit grants zero Native/body/helper/vendor/ABI/runtime/activation credit.

The proposed postconstructor substitution inside raw93 is invalid. The finite next cut is the Glyph/default/constructor85/failure-origin header/source pairs. It must carry a distinct constructor-slot arm and an earlier, negative-only Native-body-return boundary through the **actual** raw93 route. No positive Native-origin issuer exists, and no concrete implementation of `NativeGuiTextGlyphQuadStorageCalls` was found in the scoped source search. This is not production readiness or permission to return a slot.

## Actual phases and evidence

| Phase | Actual Source and Native boundary | Consequence |
|---|---|---|
| Allocate | Glyph CPP411 / `AB9D38` calls `AB78A0`; CPP413 / `AB9D3D` writes allocation into home7 | Allocation failure precedes slot state0. |
| Constructor slot | CPP414 sets state0; Native `AB9D43` stores state0 after the home write; nonnull receiver calls raw93 at CPP418 / `AB9D4F` | The slot arm belongs to this precise call and home **address**. Null allocation skips the constructor and reaches normal disarm. |
| In-flight raw93 | default CPP387..394 selects the raw-Model overload; template CPP371 / Native `AB98D8` calls constructor-phase `ensure_native_gui_text_sections_storage_00ab8530` | This is `native_gui_text_storage_construct`, not the postconstructor main-section API. |
| Native body returned | Native raw93 has only its epilogue after `AB8530` returns; Source CPP372 runs `completed(raw,c,a)` before returning | The new private body boundary must revoke constructor-failure eligibility immediately after CPP371 returns and **before** CPP372. Whole Source normal completion is later and is a different fact. |
| Source normal constructor return | Glyph CPP421 disarms state0, CPP422 writes returned child; Native `AB9D68` disarms before `AB9D6C` child store and `AB9D70` append | Preserve disarm-before-home-replacement and pass the actual home address to append. |
| Later section use | Glyph CPP433 / `AB9D93`, after append and scene-node work | A different postconstructor use. Any eventual 8f9 connection needs the actual completed/bound raw122 reference and exact primary affinity; it cannot repair the earlier in-flight call. |

Glyph FuncInfo `DEEEFC` maps state0 through `DEEEE4` to `CB80B0`. Its bytes `8B4D20 E938F6DFFF` load **CURRENT** `[EBP+20]` then tail-jump to `AB76F0`; the source projection uses CURRENT `homes[7]`. Receiver captured at entry is corroborating identity, never the return target. A callback may change the home; substituting `captured_allocation`, comparing-and-clearing, or requiring the home value to equal that capture changes the action.

The installed Original PE is unchanged (`b682a82c52f81f957b2c70222077305a933f72481686c88843077f714b956dd6`, 12,223,752 bytes). The seven bounded PE ranges in the report match the existing Glyph/93/85 EH evidence. Raw93's seven cleanup states and constructor85's six states describe their own resource actions; they do not prove exception origin or grant a caller slot action. Existing constructor cleanup/secondary transport qualifications, including old Source termination/noexcept projections, are retained without new endorsement or policy. No vendor FH3 body is reconstructed here.

## Required private metadata cut

Proposed names below are a finite implementation interface, not implemented Source or Native capabilities:

```cpp
class NativeGuiTextFailureConstructorSlotArm; // distinct from HeaderArm
class NativeGuiTextFailureConstructorCall;    // retained, noncopyable envelope
```

`ConstructorSlotArm` binds one retained Glyph invocation, the SAME qualified Source runtime, actual `volatile uint32_t*` home7 address, captured constructor receiver, callee entry `AB9650`, actual supplier/invocation identity, caller entry/site `AB98F0/AB9D4F`, exact operation edge and ordinary-primary stage. Its arm is metadata only. The Glyph invocation owns it; each nested/reached invocation needs distinct prepared records. Neither an arbitrary provider nor public failure/phase fields may mint permission.

`ConstructorCall` is the noncopyable Glyph-created bridge to that arm and the actual raw93 route. Its body-return notification is private to the genuine raw93 entry and only **revokes** eligibility. It does not set Native origin, settle ownership, complete an Origin, add a reference, or grant normal-return credit. It runs immediately after ctor85 returns and before `completed()`, reference binding, registry publication or any added postconstructor work. The call must not create a call-local owner or discard failed metadata. Old four-argument constructor entries remain unchanged and receive no protocol upgrade by inference.

The explicit proposed Source entries are:

```cpp
virtual void* construct_00ab9650(void* actual_allocation,
    NativeGuiTextGlyphQuadStorageAcquired&,
    NativeGuiTextFailureConstructorCall&) = 0;

void* construct_native_gui_text_default_storage_00ab9650(
    void* actual_destination, NativeGuiTextRawModelDefaultStorageFrame&,
    NativeGuiTextRawModelDefaultStorageContext&,
    NativeGuiTextRawModelDefaultStorageAcquired&,
    NativeGuiTextFailureConstructorCall&);

void ensure_native_gui_text_sections_storage_00ab8530(
    void* actual_text, NativeGuiTextRawModelSectionStorageFrame&,
    NativeGuiTextRawModelSectionStorageContext&,
    NativeGuiTextRawModelSectionStorageAcquired&,
    const NativeGuiTextFailureOriginRuntime&);
```

Append the same borrowed runtime and retained Origin/slot/call metadata to the real Glyph/constructor graph; preserve existing raw scratch/homes/field layouts and Native operand order. Raw93 and ctor85 keep distinct invocation Origins and forward exact current-cause/child-edge identity. Wrap only concrete added Source checks, preparation, admissions and reference/registry publication as host. Mixed constructors, normal return, provider or cleanup operations stay unclassified without an actual issuer. Raw93 `completed()` performs `raw_base_admission` and graph checks; raw85 `prepare_model`, `bind_model`, `shadow_ready` and DEFAULT checks are also actual Source metadata sites, not Native permission. Public `Failure::provider_call` and `Failure::host_metadata` remain diagnostics, not cause authentication.

At state0 catch, authenticate the current supported-catch cause and exact caller/child edge first. A private predicate may consume the slot arm **once before reading CURRENT home7**, only if its invocation/runtime/receiver/callee/site/edge/ordinary stage and still-in-flight state match a genuinely proven Native primary. Only then read the current home and reach `AB76F0` without an intervening resolver/provider callback. With 8f9's absent issuer the predicate must stay false: host and unclassified escapes retain the slot and complete reached graph. No public Native bit, caller-provided boolean, HeaderArm cast or successful fallback is acceptable.

If an authorized action itself throws, its current cause/edge is cleanup-secondary; it never inherits the trigger's permission or replays the action. Consume before action and retain both histories. Exact Original FH3 secondary identity/transport remains unproved. This design adds no cleanup, retry, rollback, termination or exception-suppression policy.

## Remaining proof and scope

`NativeGuiTextFailureHeaderArm` is header-resource-specific: its predicate compares an actual header and header lifetime/state. It cannot stand in for a mutable constructor home or this earlier epoch. Its 8f9 issuer is intentionally absent. The runtime constructor's enum is preparation, not runtime verification; SDK active-object identity requires the qualified own-runtime active catch/PTD/domain and retained service. `current_exception()` is diagnostic only, may copy/substitute, and never creates Native origin. Committed host/unclassified denials stay sticky; secondary/replacement/sibling/reentry cannot upgrade them.

The eight future Source files are `include/bsp/native_gui_text_glyph_quad_storage.hpp`, `src/native_gui_text_glyph_quad_storage.cpp`, `include/bsp/native_gui_text_default_storage.hpp`, `src/native_gui_text_default_storage.cpp`, `include/bsp/native_gui_text_storage_construct.hpp`, `src/native_gui_text_storage_construct.cpp`, `include/bsp/native_gui_text_failure_origin.hpp`, and `src/native_gui_text_failure_origin.cpp`. They require a fresh explicit lease/authorization. No postconstructor/changed96 Source edit is implied by this audit.

Before activation, the actual constructor binding must invoke the new raw93 entry with the SAME envelope, contexts, receiver and retained records. Providers, pools, actual Text/Model/mesh/material objects, residual slots, home backing and all nested records must survive failure and any ended widget lifetime. No destructor may infer disposal from these diagnostics. The genuine Native issuer, caller retention/disposal contract, supported Source-runtime integration and Original ABI/FH3/SEH/private-alias/game proof remain open. Existing DEFAULT-domain constraints are inherited, not expanded or newly narrowed. No production edits or new execution were performed.
