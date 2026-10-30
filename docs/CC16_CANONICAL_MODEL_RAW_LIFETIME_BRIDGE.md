# Canonical Model raw lifetime bridge

The canonical Model lifetime blocker is an **implementable Source transport repair**, not a missing Native body. Keep the existing `NativeModelOwner`, `NativeModelReference`, actual `+04` count and renderer registry. Bind a genuine raw lifetime context immutably when preparing a Page-owned Model; route normal raw current18/current54 through explicit throwing entries; select full raw Model/node destruction at the existing canonical observed-zero terminal. No second Model reference, registry, count, Native recovery or empty-input restriction is needed.

Frozen Source is Main `c889198e6609e03fb2ed5f4593ce19b7d4a64523`. This clean worktree merged that metadata-only Main into the prior owner-contract branch, producing parent `69f145d577e3b178a50979e79e71bad40fd07bfd`. Production Source is unchanged from the prior `5dc9e9a0` audit. The prior owner contract `8eea91c3a8dfe9a47c33c8eeb9f8147155a31447` remains a frozen input. This document supersedes its *unresolved canonical terminal* finding with the concrete repair below; it does not implement that repair or admit populated Page execution.

## Existing complete bodies and the precise seam

The existing raw bodies already perform scene attach/remove B6ED80/B6EE10, tree unlink/release B6DFA0/B6F310, Model destructor/scalar B750C0/B75290, geometry-member cleanup B74F20 and raw node destruction B6F440. BD30E0/BD30F0 and same-pool B74750 are also present. Their concrete Source interfaces retain current imports, native load/callback order, real raw hierarchy and point-light/string operations, and explicit failure diagnostics. No Ghidra batch or new export is required.

The current canonical companion is nevertheless routed incorrectly for a raw Page scene: `native_model_owner.cpp:333–338` calls legacy B6EE10; zero terminal 340–351 calls the legacy owned scalar/destructor (251–275); `finish_node` 102–116 selects only the *name* pool and still calls legacy node destruction. The latter reads actual `+170` as `SceneResource*` (`native_node_destruction.cpp:252–254`). Raw name mode therefore cannot choose a raw scene lifetime implicitly.

The raw Model storage primitives accept raw actual storage/context/frame/acquired independently of `NativeModelStorageReference` (`native_model_storage_lifetime.hpp:55–82`). Reuse those primitive bodies with the existing canonical owner. Do not instantiate the separate storage reference or its DEFAULT private dispatcher, whose hierarchy/scene/retained/point-light restrictions are irrelevant to this populated domain.

## Immutable binding and exact new interfaces

The names in this section are proposed host C++ APIs, not recovered symbols. Add these declarations to `native_model_owner.hpp`; include/forward the existing raw lifetime types without a cycle:

```cpp
struct NativeModelRawHostLifetime {
    bool typed_storage_ended{}, failed{};
    std::uint32_t stopped_site{};
    std::exception_ptr exception;
};
struct NativeModelRawLifetimeBinding {
    NativeModelStorageContext& storage;
    NativeModelStorageLifetimeFrame& terminal_frame;
    NativeModelStorageLifetimeAcquired& terminal;
    NativeModelRawHostLifetime& host;
};
// Existing three-argument preparation remains the genuine legacy selection.
NativeModelOwner(void* actual, std::size_t bytes, NativeModelEnvironment&);
// NEW, required nonnull raw lifetime binding before any native construction.
NativeModelOwner(void* actual, std::size_t bytes, NativeModelEnvironment&,
    NativeModelRawLifetimeBinding&);
// Host field in owner: NativeModelRawLifetimeBinding* const raw_lifetime;

// NEW members of the SAME NativeModelReference; deliberately not noexcept.
void release_raw_virtual18_00b6f310(std::uint32_t captured_target,
    NativeNodeTreeRetirementFrame&, NativeNodeTreeRetirementAcquired&);
void remove_raw_scene_virtual54_00b6ee10(std::uint32_t captured_target,
    NativeNodeSceneAttachmentFrame&, NativeNodeSceneAttachmentAcquired&);
```

The actual shared Model domain's sole stable `ModelRecord` owns the terminal frame/acquired/host state and this binding; the binding borrows its Page client's stable `NativeModelStorageContext`. The client context is initialized before the raw `prepare_model` callback uses the four-argument constructor. The raw storage context borrows the **actual Page raw node lifetime provider**, not the legacy `SceneAttachmentRuntime`: `NativeModelStorageContext{page.group_node, same_process_Model_pool, renderer.node_constants(), same_D7A208_cell}`. Its scene/tree/world/type/point-light dispatch extends the genuine Page/renderer domain to canonical child Models. Page and renderer/VFS/mapped data outlive this record and every entered call. No native construction occurs merely from composing these metadata views.

Preflight before preparing the actual slot must prove: exact same pool; same canonical registry in environment/geometry/raw scene/raw tree contexts; raw node strings are the same persistent `NativeStringRawPoolContext` object; the same point-light runtime/allocation bindings; identical CE4ADC/CE4970/D7A24C/D7A208 constant cells; genuine mapped Model/base tables; and the same current increment/decrement cells in raw scene/tree contexts. `actual_names` must be null for this Page binding. Do not inspect unconstructed native slot fields during preparation. These identity requirements extend, rather than replace, existing AAACD4 raw preflight.

The binding is immutable through failure disposition and terminal disposal. The existing three-argument constructor explicitly has `raw_lifetime == nullptr`; its original lifetime path remains selected for current Mission/legacy clients, including Mission's born-raw **name** environment. Do not infer lifetime selection from `actual_names`, profile names, a GUI enum or a later scene-null test. Do not retarget an already prepared/live owner. The ordinary instance environment stays typed as before.

## Throwing normal current18/current54

The Page raw tree dispatcher authenticates `actual -> same canonical NativeModelReference -> same ModelOwner/binding` in the renderer registry, validates the captured target/current mapped profile and passes a fresh, disjoint, persistent invocation. For current18 the target is B6F310; for current54 it is B6EE10. The methods above capture the actual numeric identity and the Page-owned `storage.node.trees` or `.scenes` reference **before** calling the existing whole raw body. They then call `release_native_node_tree_00b6f310` or `remove_native_node_scene_storage_00b6ee10` directly. They neither invoke a legacy virtual nor add a decrement, scene preclear, progress store or post-call actual read.

The current54 frame retains its real incoming scene/recursion DWORD homes. The raw body preserves its expected-before-initial170 capture, current170 reload, clear-before-release and late child recursion. The same Page scene provider can directly use the existing B6ED80 body for current50 with its own call frame; no additional Model ownership wrapper is required.

Normal raw18/raw54 have **no catch that terminates**, no `noexcept`, and no post-body access to `this`, its binding or Model record. A nested zero terminal can dispose that companion. The calling Page/tree/scene provider owns the persistent call record and catches into that still-live Page invocation on an exception, recording the stopped site and exception and retaining the complete partial graph. It may inspect only its diagnostic frame after return. Diagnostic reference pointers become opaque identities after retirement; they are not a reason to retain the native count or dereference a deleted companion.

Both the current Model D62DE8 phase and genuine live D62C88 base-destruction prefix can be admitted with their real tables/targets. Base-prefix raw calls remain legal while B6F440 is active and before its typed extent is ended; a retired/ended payload rejects before native reads. Do not copy the DEFAULT storage dispatcher's positive-count/empty-state policy onto these normal bodies. The original raw B6F310 byte44 gate and actual current counts determine effects; only a terminal requires observed zero.

`RenderCommandReference::release_zero_references`, `RenderCommandModelLifetime::release_model_virtual18` and `GeneratedModelNodeLifetime::remove_scene_virtual54` remain their existing `noexcept` interfaces. **Raw Page normal lifecycle calls must not enter the latter two.** Change every reached Page/root/tree/scene normal caller to the explicit raw entries. Legacy normal virtuals and legacy SceneNodeAttachment callbacks stay valid only on owners prepared without the raw lifetime binding; add an explicit wrong-transport contract guard so a raw owner never silently falls back into legacy scene destruction. Such a guard diagnoses a Source integration violation before legacy effects, not a policy rejecting a native populated graph. Do not make raw callback failures appear supported by catching them inside those `noexcept` normal virtuals. Any additional caller that hands a raw Page Model to a legacy queue/host normal-lifetime interface must also be rerouted before that caller is admitted.

## Canonical observed-zero terminal and typed extent end

Keep the existing canonical `release_zero_references() noexcept` boundary. The raw branch verifies the same actual `+04` alias is still zero, the same canonical registry entry, live owner and genuine current0 BD30E0. Use existing `invoke_native_ref_counted_delete_00bd30e0(actual, calls)`; its actual CURRENT profile selects slot04 B75290. A small used `NativeRefCountedDeleteCalls` adapter validates this same owner/target, seeds the record's persistent flags DWORD with the actual flags1 argument, and invokes the raw scalar transport. It is a synchronous dispatch object, not a new native owner or address-only wrapper.

Full raw B750C0 already performs the correct single import epoch for retained174 and freshly reloaded geometry180, fresh B74F20 import on state1 unwind, and exactly one reached raw B6F440 (`native_model_storage_lifetime.cpp:122–185`). Do not follow it with `finish_node`, legacy node destruction, another B6F440 or another name/array cleanup.

There is one necessary host bridge inside the existing scalar schedule: raw primitives stamp/destroy raw payload but do not end the `NativeNodeStorage` and `NativeModelTailStorage` C++ extents originally created by `NativeModelOwner::prepare_storage`. Legacy `destroy_reference_base` ends the typed node; raw B6F440's AA6E10/BD30F0 leaf only stamps (`native_node_destruction.cpp:53–56`; raw-base CPP 189–193; widget-base CPP 29–33). The canonical owner must end those existing typed extents and forget its existing external scene binding **after raw base destruction and before physical pool return**.

Add an actually consumed completion overload to the existing raw Model lifetime module:

```cpp
struct NativeModelStorageHostCompletion {
    void* context;
    void (*end_typed_storage)(void*) noexcept;
};
void destroy_native_model_storage_00b750c0(void* actual,
    NativeModelStorageLifetimeFrame&, NativeModelStorageContext&,
    NativeModelStorageLifetimeAcquired&, const NativeModelStorageHostCompletion&);
void* delete_native_model_storage_00b75290(void* actual,
    const volatile std::uint32_t& actual_flags_argument,
    NativeModelStorageLifetimeFrame&, NativeModelStorageContext&,
    NativeModelStorageLifetimeAcquired&, const NativeModelStorageHostCompletion&);
```

Preserve the existing overloads with no completion for their actual raw-storage callers. Factor the implementation around the same existing `destroy_model` body, not a copy of it. The new completion runs once immediately after full B750C0 returns, before B75290 rereads the flags LOW BYTE and calls the same B74750 pool. It performs no native cleanup, callback, allocation, count or profile store: end the typed node extent, invoke the existing tail/scene-metadata end (`end_tail`), and mark `typed_storage_ended`. The node/tail destructors must remain trivial metadata extent ends; preserve their original byte preimages and do not introduce RAII string release. External `SceneAttachmentRuntime::forget_destroyed_binding` inspects companion metadata only (scene-attachment CPP 342–348).

For an escaping raw destructor exception, the same wrapper runs that host extent-end once **only if raw node destruction was entered**, after the existing raw cleanup projection returns/throws; it then rethrows the original exception. If raw admission fails before that point, keep the typed extent and partial record as it stands. Never rerun raw cleanup to force completion. Existing second-exception-during-cleanup termination remains qualified exactly as in the recovered Source bodies. The completion itself is nonthrowing and cannot replay native operations.

In `native_model_owner.cpp`, make raw-bound `destroy_native_model_00b750c0` select the completion overload instead of legacy `finish_node`. Add the used borrowed-flags helper `delete_native_model_raw_00b75290(NativeModelOwner&, const volatile std::uint32_t&)` for the canonical scalar adapter; it selects the full raw B75290 completion overload. The existing value-flags owned scalar entry may delegate to this path for a raw-bound owner by seeding its persistent argument home; unbound owners keep the original implementation. Do not add a const-reference overload with the same value-flags name, which would make ordinary C++ calls ambiguous.

After successful scalar destruction/return, unbind the existing `GeneratedModelLifetimeRuntime` association, mark this same reference retired, then invoke its existing disposal callback to unbind the canonical registry exactly once and retire the sole host record. Preserve the prior nonowning Mission notification order where applicable. No access to actual storage follows return to the pool; generated-lifetime/registry unbind are identity-only operations (generated-lifetime CPP 117–120; render-context CPP 132–138). The scene metadata/typed extent end occurred before the pool return. Capture runtime/disposal pointers before calling into the native destructor so the method never rereads a disposed owner afterward.

If CURRENT flags becomes zero, full B75290 deliberately does not return the physical slot. Retire the canonical/native payload identity as required but preserve the original physical-slot custody in the **same** dead/retired record; do not erase the last slot receipt or fabricate a pool return. A legitimate flags0 caller must retain/provide its subsequent physical disposition; absent that disposition the owner reports retention. This preserves the existing raw scalar contract instead of imposing a flags1-only accepted-input rule.

Canonical-zero failures are the existing **explicit fatal terminal boundary**, not recoverable normal18/raw54 failures. Capture stopped-site/acquired/exception state in the still-retained terminal record, mark its failure and inhibit any disposal/drain; then honor the existing `noexcept` terminal termination. Do not retire/unbind/erase the record on that failed path or claim original FH3/SEH recovery. No new catch-and-terminate is permitted around normal raw lifecycle calls. Native faults, unmasked FP and a second C++ exception during recovered cleanup remain separate, unproved failure domains.

## Reentry, frames and failure custody

One Model record has one fresh terminal frame/acquired for its single actual scalar destruction. That frame is distinct from every Page current18/current54/parenting invocation and from every recursive node/scene call. Normal recursive or same-Model reentry uses another prepared Page invocation; it is not rejected by a per-Model/global busy flag. The immutable contexts may be shared, the mutable frames/acquisitions may not. The Page provider prepares honest backing for its actual retained graph/reentry frontier before cleanup; obtaining a recursive frame performs no entry-time allocation or native callback. Exhaustion stops and retains preceding effects; it never becomes a successful skip or a childless input policy.

A second zero terminal while the first is destroying, a replayed acquired frame or an ended payload is an identity/lifetime violation. Do not reset terminal diagnostics to make it replayable. Reentry through current54 during the genuine D62C88 base prefix uses a fresh Page frame until that prefix ends. No raw method may retain a scope guard that dereferences the companion after a terminal callback can dispose it. Source backing and pool slots must not be recycled before external quiescence of every active raw call.

Normal failures propagate to the existing Page outer retained-failure disposition. Preserve all real stores, decrements, registry mutations, arrays, scene credits, native creators and frame homes. The provider records failure in Page/client-owned state that survives possible companion disposal; the shared Model domain sees that failed client and cannot drain. A terminal failure instead remains inside the existing canonical fatal boundary with its own record available until termination. A healthy client can be dropped only after actual child/root retirement and no retained calls/physical-slot obligations; no blanket service sweep releases Models independently of child `+4C`.

## Connected Source changes and actual callers

The canonical bridge itself needs exactly four existing production files: `include/bsp/native_model_owner.hpp`, `src/native_model_owner.cpp`, `include/bsp/native_model_storage_lifetime.hpp`, and `src/native_model_storage_lifetime.cpp`. The raw node/tree/scene bodies, base interfaces, separate storage reference, CMake, ledgers and Ghidra do not require changes for this transport.

It must land with the actual eight-file shared-owner/consumer migration in [the frontend Model domain contract](CC16_NATIVE_FRONTEND_WIDGET_MODEL_DOMAIN.md), yielding a twelve-file ownership/bridge/consumer lower bound before any other Page supplier edits: new common-domain header/INC, renderer header/CPP, Mission INC, Page header/CPP and `game_hosts.cpp`, plus the four bridge files. This is not permission to land another unused interface.

The actual shared-domain `prepare_model` callback constructs each Page record with the real binding before B75030; Mission uses the original constructor. `game_hosts.cpp:3166–3172` supplies that renderer domain to the actual Page constructor. At `read_properties:644–649`, the real AAACD4 consumer uses the same scratch and raw Model client. The Page normal widget lifetime bridge routes both AA8320's primary and AA9730's late-primary current `+4C` through whole raw B6DFA0; Page `GroupTrees::invoke_virtual18` and recursive Model tree callbacks select the new throwing raw18 entry. The Page raw `Roots::invoke_child`/scene dispatcher selects the raw54 entry for canonical Model B6EE10 and retains real recursion homes. Its current0 dispatcher validates SAME observed zero and invokes the existing canonical terminal. No Model producer/drain callback clears widget `+4C` independently; the widget caller clears its ORIGINAL field only after normal return.

The separate [raw parenting design](CC16_NATIVE_GUI_PAGE_RAW_PARENTING_DESIGN.md) is sealed at `dfa633c235398dd2ebbc5d561e0f9d8f9853dff2`. The [populated Page reader/lifetime contract](CC16_NATIVE_GUI_PAGE_POPULATED_READER_LIFETIME.md) owns the actual Page/root/widget/timed suppliers and exception recording. This canonical bridge now has no missing recovered body/address and needs no reconstruction packet. Its implementation remains blocked on that complete connected supplier/timed closure and admission review, not on a new Native leaf. Page guards stay false; this pass changes only its two assigned evidence files and performs no Source mutation, Ghidra work, build/test/probe/process/Game action or push. ABI, FH3/SEH, runtime, gameplay and activation credit is zero.
