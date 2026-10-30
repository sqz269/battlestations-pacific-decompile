# CC16 Glyph constructor failure-origin site analysis

Analysis/design only. No production or shared-protocol file changed. The proposed direct reuse of8f9's post-constructor five-argument AB8530 inside raw93 is invalid across constructor phases. Root corrected the task to this bounded two-file analysis; a Source packet requires a separately approved constructor-specific cut. All positive Native permits, whole Glyph/changed96 behavior, ABI and activation remain gated.

## Three distinct Source boundaries

| Actual operation | Existing Source API / site | Required phase |
| --- | --- | --- |
| Glyph constructor provider | `NativeGuiTextGlyphQuadStorageCalls::construct_00ab9650(allocation, GlyphAcquired&)`, GlyphCPP418 / Native `AB9D4F` | Raw allocated Text, Native Glyph state0; no completed raw93/raw122 yet |
| Actual raw93 constructor to constructor-phase85 | `construct_native_gui_text_default_storage_00ab9650` raw-Model overload, defaultCPP387..394; template call at371 / Native `AB98D8` -> `ensure_native_gui_text_sections_storage_00ab8530(void*, RawModelSectionFrame&, RawModelSectionContext&, RawModelSectionAcquired&)` | In-flight DEFAULT constructor; ctor85 creates/publishes mandatory internal Model188 and its independent graph |
| Later Glyph child sections call | `Calls::sections_00ab8530(child,a)`, GlyphCPP433 / Native `AB9D93` | After constructor normal disarm, child append and scene-node setter; potentially post-constructor only if genuine raw122/current-primary affinity is established |

The exact opaque interface name is **Calls**, not Bindings. Scoped current Source search finds only its declaration/destructor/context member, with no derived implementation. The semantic `engine_seg11_35_66_cc13.cpp` constructor projection is not this raw provider.

8f9's `ensure_native_gui_text_post_constructor_sections_storage_00ab8530` takes a qualified `NativeGuiTextRawModelReference&`, requires completed genuine raw93/bound raw122 and current-nonnull188/main-path affinity. These cannot be supplied while raw93 is constructing itself. No cast of the destination, invented reference, replayed constructor or default graph closes that gap. The later AB9D93 call is a different possible consumer, and phase alone does not satisfy8f9's strict primary/publication contract.

## Exact Glyph state0/home schedule

Native `AB9D38` allocates from the genuine Text pool; `AB9D3D` stores allocation into actual home7. After TEST, `AB9D43` arms state0; `AB9D4B` skips the constructor only on nullable allocation. `AB9D4F` invokes AB9650 and `AB9D54` captures its returned child in ESI. `AB9D68` disarms to -1 before `AB9D6C` stores child into SAME home7 and `AB9D70` appends the ADDRESS of that live home. Source411..425 follows that order; no additional null/count policy is proposed.

EH map `DEEEE4` state0 parents -1 and selects `CB80B0` bytes `8b4d20e938f6dfff`: read CURRENT `[EBP+20]`, then tail AB76F0. Source catch492..495 likewise reads CURRENT `homes[7]`. The allocation snapshot is not the cleanup argument. Constructor/provider callbacks can change that mutable home; a future guard must not compare the value to the captured receiver or substitute `captured_allocation`.

Current Glyph catch482..505 records diagnostic `current_exception`, consumes the public native_eh_state, and permits state0 AB76F0 for every constructor-provider C++ escape. This does not distinguish raw93/85 added admission, completed-constructor bind failure or unclassified provider error. The new current-cause receipt must be captured before that diagnostic snapshot. Unknown/host must retain allocation/Text/Model188/reservation/slot/provider/homes backing without reading a stale cleanup operand or running AB76F0. States1/2 are separate remaining origin gates; this analysis does not silently certify or repair them.

## Real added host/bind sites and nested cut

Raw93 `preflight` at defaultCPP126..199 performs fresh record/extent/canonical-provider admission before base effects. `raw_base_admission`107..124 and `before_base`202..206 reauthenticate Source cleanup domains. `completed`264..273 runs that added base admission and real internal188 graph checks AFTER ctor85 returns. These are actual extra Source operations; private host evidence must come from their throwing catch, never from the public Failure enum.

Ctor85 has additional preflight/default-branch/source graph admission, material checks/registration375..378, and shadow-ready/association checks. The real internal Model binding is `a.model.reference.emplace(...)` at440..442 AFTER publication of actual188 at538 and normal Native state disarm540. Its current Model reference constructor can allocate/validate/bind metadata; completion/entered markers do not prove Native exception origin. Mixed model/mesh/material constructors remain unknown without a genuine leaf. Known host sites need private direct capture; existing diagnostic inference from public constructor/material progress is not a substitute.

Nested cleanup also needs the constructor-phase Source path: storage_constructCPP573..576 calls cleanup for any escape not marked public host_metadata; defaultCPP278..290 similarly admits unwind via public markers. Default unwind209..246 projects seven Native state actions and retains pre-existing termination qualifications. A Glyph-only slot guard cannot undo an earlier incorrectly authorized nested cleanup. The future USED negative constructor paths must preserve old entry APIs while denying Native unwind on host/unknown with retained whole graphs. No new termination, cleanup, cancellation, rollback or retry policy is authorized.

A distinct PRIVATE negative-only **modeled Native-body-return boundary** is needed immediately after actual ctor85 returns at defaultCPP371 and BEFORE completed()/later Text binding. At that point the recovered raw93 Native body has no further modeled call/effect before its return. This notification only revokes constructor-failure eligibility; it does not grant raw93 Source completion, raw122 birth, ownership, Native origin or permission. Existing a.complete is set after extra checks and cannot represent this earlier boundary. Do not generalize that placement to before ctor85's own required DEFAULT-path admission.

## Minimal shared protocol / used-overload cut

HeaderArm represents an actual eight-byte header lifetime and exposes header/state permission; it must not masquerade as a constructor slot arm. Proposed dedicated `NativeGuiTextFailureConstructorSlotArm` records owning Glyph invocation, actual home7 ADDRESS, captured constructor receiver, expected real callee record/entry `AB9650`, actual caller edge/site `AB9D4F`, current SDK cause and stage. It arms at the Native state0 epoch and disarms on normal region exit, including nullable allocation without a constructor. It never snapshots the cleanup value. A private consume checks exact current cause/edge/callee/supplier/ordinary stage and once-per-arm state BEFORE the code reads CURRENT home7 for the actual action. Wrong/stale/sibling/replaced/secondary cause, body-returned, host/unknown or no issuer denies permission. No new public flag, abstract issuer or default Native path is allowed;8f9 currently has no issuer.

Preferred candidate signatures from the independent audit are:

```cpp
Calls::construct_00ab9650(void* allocation, GlyphAcquired&,
                         NativeGuiTextFailureConstructorCall&);
construct_native_gui_text_default_storage_00ab9650(void*, RawDefaultFrame&,
    RawDefaultContext&, RawDefaultAcquired&, NativeGuiTextFailureConstructorCall&);
ensure_native_gui_text_sections_storage_00ab8530(void*, RawSectionFrame&,
    RawSectionContext&, RawSectionAcquired&, const NativeGuiTextFailureOriginRuntime&);
```

These are design signatures, not implemented APIs. The Glyph-created noncopyable ConstructorCall envelope borrows SAME qualified runtime, distinct SlotArm, home ADDRESS, actual receiver and caller/callee edge. Actual raw93 and ctor85 own private Origins; the actual raw93 body alone may notify body_returned() before completed(). Added bind/preparation stays host/unknown, not a Native constructor escape. Old APIs retain their existing qualifications. Real actual providers must supply genuine records; no derived raw Glyph producer currently closes this path.

The minimum candidate Source ownership is EIGHT existing files: Glyph quad pair, default_storage raw93 pair, storage_construct constructor85 pair and failure_origin protocol pair. AB98F0/AB9650/constructor AB8530 need explicit future leases; postconstructor main_section files need no change merely to force reuse. Model/Text reserved-birth APIs remove insertion/rehash only and remain extra host admissions; no new Native retain or nonthrow promise follows. All independent Model188, Text/slot, reservation and recursive frames survive failed admission.

## Proposed inverse and validation boundary

No inverse implementation is claimed because no Source changed. The exact baseline Source blobs and fixed Native spans/actions are pinned in the report. A future patch must invert Glyph changes limited to constructor envelope/state0 guard/pre-diagnostic capture, preserving all22 asm bodies,78 x87 forms,10 mutable homes, normal disarm, later calls and CURRENT-home7 action. Raw93 inversion must preserve every Native store/constant read (including FC=1 at defaultCPP319), AA9390, actual ctor85 call and ownership schedule. Constructor85 inversion must preserve model/name/mesh/material/current-publication operations; only private capture/forwarding/qualified cleanup seams may change. Shared protocol changes must be isolated as the distinct slot/envelope API.

Root must review/authorize that concrete cut before edits or builds. No tests, probe, game, Ghidra query/write, CMake, ledger, process-control or production change occurred. Frozen8f9's eight files and all input Source remain byte-identical. No positive Native/ABI/runtime/body/activation credit is granted.
